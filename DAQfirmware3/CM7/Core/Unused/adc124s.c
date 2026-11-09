/**
  * @file    adc124s.c
  * @brief   TI ADC124S 4-channel, 12-bit ADC driver.
  *
  * Each 16-clock frame returns the channel selected by the frame before it, so
  * the first frame of a transfer only selects a channel and its result is dropped.
  */
#include "adc124s.h"

#define ADC124S_FRAME_BYTES   2U
#define ADC124S_SELECT(ch)    ((uint8_t)((ch) << 3)) /* ADD2:0 are bits 5:3 of the first byte */
#define ADC124S_RESULT(buf)   ((uint16_t)((((uint16_t)(buf)[0] << 8) | (buf)[1]) & 0x0FFFU))

HAL_StatusTypeDef adc124s_read_channel(spi_bus_dev_t dev, uint8_t channel, uint16_t *raw)
{
  const uint8_t tx[2 * ADC124S_FRAME_BYTES] = {ADC124S_SELECT(channel), 0x00U,
                                               ADC124S_SELECT(channel), 0x00U};
  uint8_t rx[2 * ADC124S_FRAME_BYTES] = {0};
  HAL_StatusTypeDef status;

  if (channel >= ADC124S_NUM_CHANNELS)
  {
    return HAL_ERROR;
  }
  status = spi_bus_transfer(dev, tx, rx, sizeof(tx));
  if (status == HAL_OK)
  {
    *raw = ADC124S_RESULT(&rx[ADC124S_FRAME_BYTES]);
  }
  return status;
}

HAL_StatusTypeDef adc124s_read_all(spi_bus_dev_t dev, uint16_t raw[ADC124S_NUM_CHANNELS])
{
  const uint8_t tx[(ADC124S_NUM_CHANNELS + 1U) * ADC124S_FRAME_BYTES] = {
    ADC124S_SELECT(0), 0x00U, ADC124S_SELECT(1), 0x00U, ADC124S_SELECT(2), 0x00U,
    ADC124S_SELECT(3), 0x00U, ADC124S_SELECT(0), 0x00U};
  uint8_t rx[(ADC124S_NUM_CHANNELS + 1U) * ADC124S_FRAME_BYTES] = {0};
  HAL_StatusTypeDef status = spi_bus_transfer(dev, tx, rx, sizeof(tx));

  if (status == HAL_OK)
  {
    for (uint32_t ch = 0; ch < ADC124S_NUM_CHANNELS; ch++)
    {
      raw[ch] = ADC124S_RESULT(&rx[(ch + 1U) * ADC124S_FRAME_BYTES]);
    }
  }
  return status;
}
