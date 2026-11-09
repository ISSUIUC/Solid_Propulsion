/**
  * @file    spi_bus.c
  * @brief   The DAQ's two SPI buses, one ADC on each, with software chip selects.
  */
#include "spi_bus.h"
#include "daq_board.h"

#define SPI_BUS_TIMEOUT_MS 2U

/* The kernel clock is 64 MHz (HSI via CKPER, see HAL_SPI_MspInit), so /4 gives 16 MHz.
   In SPI mode 0 the ADS7066 changes SDO on the falling edge and the MCU samples on
   the next rising edge, half a clock later. The ADS7066 needs up to 16 ns
   (tD_CKDO). At 16 MHz half a clock is 31 ns. At 32 MHz it would be 15.6 ns, which is too short. */
#define SPI_BUS_PRESCALER SPI_BAUDRATEPRESCALER_4

typedef struct
{
  SPI_TypeDef *instance;
  GPIO_TypeDef *cs_port;
  uint16_t cs_pin;
} spi_bus_dev_cfg_t;

static const spi_bus_dev_cfg_t dev_cfg[SPI_BUS_DEV_COUNT] = {
  [SPI_BUS_DEV_TC_ADC] = {DAQ_SPI_TC_INSTANCE, DAQ_CS_TC_PORT, DAQ_CS_TC_PIN},
  [SPI_BUS_DEV_PT_ADC] = {DAQ_SPI_PT_INSTANCE, DAQ_CS_PT_PORT, DAQ_CS_PT_PIN},
};

static SPI_HandleTypeDef hspi[SPI_BUS_DEV_COUNT];

static HAL_StatusTypeDef spi_bus_init_one(SPI_HandleTypeDef *h, SPI_TypeDef *instance)
{
  h->Instance = instance;
  h->Init.Mode = SPI_MODE_MASTER;
  h->Init.Direction = SPI_DIRECTION_2LINES;
  h->Init.DataSize = SPI_DATASIZE_8BIT;
  h->Init.CLKPolarity = SPI_POLARITY_LOW; /* mode 0, the ADS7066 power-up default */
  h->Init.CLKPhase = SPI_PHASE_1EDGE;
  h->Init.NSS = SPI_NSS_SOFT;
  h->Init.BaudRatePrescaler = SPI_BUS_PRESCALER;
  h->Init.FirstBit = SPI_FIRSTBIT_MSB;
  h->Init.TIMode = SPI_TIMODE_DISABLE;
  h->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  h->Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  h->Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  h->Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  h->Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  h->Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  h->Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  /* HAL disables SPI between transfers; without this SCK floats while it is off */
  h->Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
  h->Init.IOSwap = SPI_IO_SWAP_DISABLE;

  return HAL_SPI_Init(h);
}

HAL_StatusTypeDef spi_bus_init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  DAQ_BOARD_GPIO_CLK_ENABLE();

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  for (int i = 0; i < SPI_BUS_DEV_COUNT; i++)
  {
    /* Deselect before the pin becomes an output so no device sees a glitch */
    HAL_GPIO_WritePin(dev_cfg[i].cs_port, dev_cfg[i].cs_pin, GPIO_PIN_SET);
    GPIO_InitStruct.Pin = dev_cfg[i].cs_pin;
    HAL_GPIO_Init(dev_cfg[i].cs_port, &GPIO_InitStruct);
  }

  for (int i = 0; i < SPI_BUS_DEV_COUNT; i++)
  {
    HAL_StatusTypeDef status = spi_bus_init_one(&hspi[i], dev_cfg[i].instance);
    if (status != HAL_OK)
    {
      return status;
    }
  }
  return HAL_OK;
}

HAL_StatusTypeDef spi_bus_transfer(spi_bus_dev_t dev, const uint8_t *tx, uint8_t *rx, uint16_t len)
{
  const spi_bus_dev_cfg_t *cfg;
  HAL_StatusTypeDef status;

  if ((unsigned)dev >= SPI_BUS_DEV_COUNT)
  {
    return HAL_ERROR;
  }
  cfg = &dev_cfg[dev];

  HAL_GPIO_WritePin(cfg->cs_port, cfg->cs_pin, GPIO_PIN_RESET);
  status = HAL_SPI_TransmitReceive(&hspi[dev], tx, rx, len, SPI_BUS_TIMEOUT_MS);
  HAL_GPIO_WritePin(cfg->cs_port, cfg->cs_pin, GPIO_PIN_SET);

  return status;
}
