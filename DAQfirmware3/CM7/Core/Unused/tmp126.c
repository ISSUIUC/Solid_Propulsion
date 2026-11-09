/**
  * @file    tmp126.c
  * @brief   TI TMP126 temperature sensor driver.
  */
#include "tmp126.h"

#define TMP126_CMD_READ_MSB   0x01U /* Bit 8 of the command word is R/W */
#define TMP126_REG_TEMP       0x00U
#define TMP126_DEG_C_PER_LSB  0.03125f

HAL_StatusTypeDef tmp126_read_temp(spi_bus_dev_t dev, float *temp_c)
{
  const uint8_t tx[4] = {TMP126_CMD_READ_MSB, TMP126_REG_TEMP, 0x00U, 0x00U};
  uint8_t rx[4] = {0};
  HAL_StatusTypeDef status = spi_bus_transfer(dev, tx, rx, sizeof(tx));

  if (status == HAL_OK)
  {
    /* 14-bit two's complement in bits 15:2; the low bits are not temperature */
    int16_t raw = (int16_t)(((uint16_t)rx[2] << 8) | rx[3]);
    *temp_c = (float)(raw >> 2) * TMP126_DEG_C_PER_LSB;
  }
  return status;
}
