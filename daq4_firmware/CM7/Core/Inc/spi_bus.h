/**
  * @file    spi_bus.h
  * @brief   The DAQ's two SPI buses, one ADC on each, with software chip selects.
  */
#ifndef SPI_BUS_H
#define SPI_BUS_H

#include "stm32h7xx_hal.h"

typedef enum
{
  SPI_BUS_DEV_TC_ADC = 0, /* thermocouple ADS7066, SPI1 */
  SPI_BUS_DEV_PT_ADC,     /* pressure transducer / load cell ADS7066, SPI2 */
  SPI_BUS_DEV_COUNT
} spi_bus_dev_t;

/* Call after MX_GPIO_Init(): it takes the chip select pins back from the SPI peripheral. */
HAL_StatusTypeDef spi_bus_init(void);

/* Full-duplex transfer of len bytes with the device selected for the whole transfer. */
HAL_StatusTypeDef spi_bus_transfer(spi_bus_dev_t dev, const uint8_t *tx, uint8_t *rx, uint16_t len);

#endif /* SPI_BUS_H */
