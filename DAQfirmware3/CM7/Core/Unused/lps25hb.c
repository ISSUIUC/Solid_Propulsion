/**
  * @file    lps25hb.c
  * @brief   ST LPS25HB barometer driver.
  */
#include "lps25hb.h"

#define LPS25HB_SPI_READ        0x80U
#define LPS25HB_SPI_AUTO_INC    0x40U

#define LPS25HB_REG_WHO_AM_I    0x0FU
#define LPS25HB_REG_CTRL_REG1   0x20U
#define LPS25HB_REG_PRESS_OUT   0x28U /* XL, L, H, then TEMP_OUT L, H */

#define LPS25HB_WHO_AM_I_VALUE  0xBDU
#define LPS25HB_CTRL1_PD        0x80U
#define LPS25HB_CTRL1_ODR_25HZ  0x40U
#define LPS25HB_CTRL1_BDU       0x04U

HAL_StatusTypeDef lps25hb_init(spi_bus_dev_t dev)
{
  const uint8_t id_tx[2] = {LPS25HB_SPI_READ | LPS25HB_REG_WHO_AM_I, 0x00U};
  const uint8_t ctrl_tx[2] = {LPS25HB_REG_CTRL_REG1,
                              LPS25HB_CTRL1_PD | LPS25HB_CTRL1_ODR_25HZ | LPS25HB_CTRL1_BDU};
  uint8_t rx[2] = {0};
  HAL_StatusTypeDef status = spi_bus_transfer(dev, id_tx, rx, sizeof(id_tx));

  if (status != HAL_OK)
  {
    return status;
  }
  if (rx[1] != LPS25HB_WHO_AM_I_VALUE)
  {
    return HAL_ERROR;
  }
  return spi_bus_transfer(dev, ctrl_tx, rx, sizeof(ctrl_tx));
}

HAL_StatusTypeDef lps25hb_read(spi_bus_dev_t dev, float *pressure_hpa, float *temp_c)
{
  const uint8_t tx[6] = {LPS25HB_SPI_READ | LPS25HB_SPI_AUTO_INC | LPS25HB_REG_PRESS_OUT};
  uint8_t rx[6] = {0};
  HAL_StatusTypeDef status = spi_bus_transfer(dev, tx, rx, sizeof(tx));

  if (status == HAL_OK)
  {
    uint32_t press = ((uint32_t)rx[3] << 16) | ((uint32_t)rx[2] << 8) | rx[1];
    int16_t temp = (int16_t)(((uint16_t)rx[5] << 8) | rx[4]);

    if (press & 0x800000U)
    {
      press |= 0xFF000000U;
    }
    *pressure_hpa = (float)(int32_t)press / 4096.0f;
    *temp_c = 42.5f + (float)temp / 480.0f;
  }
  return status;
}
