/**
  * @file    ads7066.c
  * @brief   TI ADS7066 8-channel, 16-bit, 250 kSPS SAR ADC driver.
  *
  * Register map, opcodes and timing are from the TI datasheet SBAS928C. Section
  * numbers below refer to it. Every SPI frame is 24 bits, and every CS rising edge
  * starts a conversion whose result comes out in the next frame (section 7.4).
  */
#include "ads7066.h"
#include "daq_timing.h"

/* Opcodes, Table 7-6 */
#define OP_NOP     0x00U
#define OP_RD_REG  0x10U
#define OP_WR_REG  0x08U

/* Registers, Table 7-9 */
#define REG_SYSTEM_STATUS    0x00U
#define REG_GENERAL_CFG      0x01U
#define REG_DATA_CFG         0x02U
#define REG_OSR_CFG          0x03U
#define REG_PIN_CFG          0x05U
#define REG_SEQUENCE_CFG     0x10U
#define REG_AUTO_SEQ_CH_SEL  0x12U

/* SYSTEM_STATUS bits, Table 7-11. Reset value 0x81 */
#define STATUS_RSVD          0x80U /* always reads 1 */
#define STATUS_CRCERR_IN     0x02U
#define STATUS_BOR           0x01U
#define STATUS_AFTER_RESET   0x81U

/* GENERAL_CFG bits, Table 7-12 */
#define GEN_REF_EN           0x80U
#define GEN_CAL              0x02U
#define GEN_RST              0x01U

/* DATA_CFG bits, Table 7-13. CPOL_CPHA stays 00b (SPI mode 0) */
#define DATA_FIX_PAT         0x80U /* ADC data reads return 0xA5A5 */
#define DATA_APPEND_CHID     0x10U /* APPEND_STATUS = 01b */
#define FIX_PAT_CODE         0xA5A5U

/* SEQUENCE_CFG bits, Table 7-21 */
#define SEQ_START            0x10U
#define SEQ_MODE_AUTO        0x01U

#define FRAME_BYTES          3U

/* Wait after every frame so CS rising edges are at least tCYCLE (1 / 250 kSPS = 4 us)
   apart: tCONV (3.2 us) must finish before the result is clocked out, and the next
   sample needs tACQ (0.8 us). Section 6.6/6.7. */
#define T_CYCLE_US           4U
#define T_RST_MS             5U  /* RST to reset complete, max (section 6.7) */
#define T_REF_SETTLE_MS      10U /* not specified; allows for the REF capacitor (up to 10 uF) */
#define T_CAL_TIMEOUT_MS     10U /* not specified; CAL clears itself when done */

static HAL_StatusTypeDef frame(spi_bus_dev_t dev, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t rx[FRAME_BYTES])
{
  const uint8_t tx[FRAME_BYTES] = {b0, b1, b2};
  uint8_t dummy[FRAME_BYTES];
  HAL_StatusTypeDef status;

  status = spi_bus_transfer(dev, tx, (rx != NULL) ? rx : dummy, FRAME_BYTES);
  daq_timing_delay_us(T_CYCLE_US);
  return status;
}

HAL_StatusTypeDef ads7066_write_reg(spi_bus_dev_t dev, uint8_t addr, uint8_t value)
{
  return frame(dev, OP_WR_REG, addr, value, NULL); /* Figure 7-9 */
}

HAL_StatusTypeDef ads7066_read_reg(spi_bus_dev_t dev, uint8_t addr, uint8_t *value)
{
  uint8_t rx[FRAME_BYTES];
  HAL_StatusTypeDef status;

  /* Figure 7-10: the command frame, then the register in the first byte of the next frame */
  status = frame(dev, OP_RD_REG, addr, 0x00U, NULL);
  if (status == HAL_OK)
  {
    status = frame(dev, OP_NOP, 0x00U, 0x00U, rx);
  }
  if (status == HAL_OK)
  {
    *value = rx[0];
  }
  return status;
}

static HAL_StatusTypeDef write_verify(spi_bus_dev_t dev, uint8_t addr, uint8_t value)
{
  uint8_t readback = 0U;
  HAL_StatusTypeDef status = ads7066_write_reg(dev, addr, value);

  if (status == HAL_OK)
  {
    status = ads7066_read_reg(dev, addr, &readback);
  }
  if ((status == HAL_OK) && (readback != value))
  {
    status = HAL_ERROR;
  }
  return status;
}

/* Checks the data path with the fixed 0xA5A5 test pattern (DATA_CFG.FIX_PAT). */
static HAL_StatusTypeDef check_fixed_pattern(spi_bus_dev_t dev)
{
  uint8_t rx[FRAME_BYTES];
  HAL_StatusTypeDef status = ads7066_write_reg(dev, REG_DATA_CFG, DATA_FIX_PAT);

  if (status == HAL_OK)
  {
    status = frame(dev, OP_NOP, 0x00U, 0x00U, NULL); /* settle on a data frame */
  }
  if (status == HAL_OK)
  {
    status = frame(dev, OP_NOP, 0x00U, 0x00U, rx);
  }
  if ((status == HAL_OK) && ((((uint16_t)rx[0] << 8) | rx[1]) != FIX_PAT_CODE))
  {
    status = HAL_ERROR;
  }
  return status;
}

static HAL_StatusTypeDef calibrate_offset(spi_bus_dev_t dev, uint8_t general_cfg)
{
  const uint32_t start = HAL_GetTick();
  uint8_t value = 0U;
  HAL_StatusTypeDef status = ads7066_write_reg(dev, REG_GENERAL_CFG, (uint8_t)(general_cfg | GEN_CAL));

  while (status == HAL_OK)
  {
    status = ads7066_read_reg(dev, REG_GENERAL_CFG, &value);
    if ((status == HAL_OK) && ((value & GEN_CAL) == 0U))
    {
      break;
    }
    if ((HAL_GetTick() - start) > T_CAL_TIMEOUT_MS)
    {
      status = HAL_TIMEOUT;
    }
  }
  return status;
}

HAL_StatusTypeDef ads7066_init(spi_bus_dev_t dev, const ads7066_config_t *cfg)
{
  const uint8_t general_cfg = cfg->internal_ref ? GEN_REF_EN : 0x00U; /* RANGE = 0: 0 V to VREF */
  uint8_t value = 0U;
  HAL_StatusTypeDef status;

  status = ads7066_write_reg(dev, REG_GENERAL_CFG, GEN_RST);
  if (status != HAL_OK)
  {
    return status;
  }
  HAL_Delay(T_RST_MS + 1U);

  /* Presence check: after a reset SYSTEM_STATUS is 0x81. A floating or shorted MISO
     reads 0xFF or 0x00, and CRCERR_FUSE (bit 2) set means the part did not load its configuration. */
  status = ads7066_read_reg(dev, REG_SYSTEM_STATUS, &value);
  if (status != HAL_OK)
  {
    return status;
  }
  if ((value & (uint8_t)~STATUS_CRCERR_IN) != STATUS_AFTER_RESET)
  {
    return HAL_ERROR;
  }
  status = ads7066_write_reg(dev, REG_SYSTEM_STATUS, STATUS_BOR | STATUS_CRCERR_IN); /* write 1 to clear */

  if (status == HAL_OK)
  {
    status = check_fixed_pattern(dev);
  }
  if (status == HAL_OK)
  {
    status = write_verify(dev, REG_DATA_CFG, DATA_APPEND_CHID);
  }
  if (status == HAL_OK)
  {
    status = write_verify(dev, REG_GENERAL_CFG, general_cfg);
  }
  if ((status == HAL_OK) && cfg->internal_ref)
  {
    HAL_Delay(T_REF_SETTLE_MS);
  }
  if (status == HAL_OK)
  {
    status = calibrate_offset(dev, general_cfg); /* section 7.3.4, after the reference is up */
  }
  if (status == HAL_OK)
  {
    status = write_verify(dev, REG_PIN_CFG, 0x00U); /* all 8 pins analog inputs */
  }
  if (status == HAL_OK)
  {
    status = write_verify(dev, REG_OSR_CFG, 0x00U); /* no averaging: 16-bit results */
  }
  if (status == HAL_OK)
  {
    status = write_verify(dev, REG_AUTO_SEQ_CH_SEL, 0xFFU); /* AIN0-AIN7 */
  }
  if (status == HAL_OK)
  {
    status = write_verify(dev, REG_SEQUENCE_CFG, SEQ_START | SEQ_MODE_AUTO); /* section 7.4.4 */
  }
  return status;
}

HAL_StatusTypeDef ads7066_read_all(spi_bus_dev_t dev, uint16_t raw[ADS7066_NUM_CHANNELS])
{
  uint16_t result[ADS7066_NUM_CHANNELS];
  uint8_t rx[FRAME_BYTES];
  uint32_t seen = 0U;
  HAL_StatusTypeDef status;

  /* This frame's output is the conversion started at the end of the previous call,
     possibly a whole sample period ago, so it is discarded. */
  status = frame(dev, OP_NOP, 0x00U, 0x00U, NULL);

  /* The next 8 frames return 8 back-to-back conversions. The sequencer steps through
     all 8 channels, so each appears once, in rotating order. Output format, Table 7-4:
     {result[15:0], CHID[3:0], 0000b} */
  for (uint32_t i = 0U; (i < ADS7066_NUM_CHANNELS) && (status == HAL_OK); i++)
  {
    status = frame(dev, OP_NOP, 0x00U, 0x00U, rx);
    if (status == HAL_OK)
    {
      const uint32_t ch = rx[2] >> 4;
      if ((ch >= ADS7066_NUM_CHANNELS) || ((rx[2] & 0x0FU) != 0U) || ((seen & (1UL << ch)) != 0U))
      {
        status = HAL_ERROR;
      }
      else
      {
        result[ch] = (uint16_t)(((uint16_t)rx[0] << 8) | rx[1]);
        seen |= 1UL << ch;
      }
    }
  }

  if (status == HAL_OK)
  {
    for (uint32_t ch = 0U; ch < ADS7066_NUM_CHANNELS; ch++)
    {
      raw[ch] = result[ch];
    }
  }
  return status;
}
