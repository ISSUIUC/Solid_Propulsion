/**
  * @file    adc124s.h
  * @brief   TI ADC124S 4-channel, 12-bit ADC driver.
  */
#ifndef ADC124S_H
#define ADC124S_H

#include "spi_bus.h"

#define ADC124S_NUM_CHANNELS 4U

/* Reads one channel (0-3) as a raw 0-4095 count. Allow any mux in front of it to settle first. */
HAL_StatusTypeDef adc124s_read_channel(spi_bus_dev_t dev, uint8_t channel, uint16_t *raw);

/* Reads channels 0-3 in order, as raw 0-4095 counts. */
HAL_StatusTypeDef adc124s_read_all(spi_bus_dev_t dev, uint16_t raw[ADC124S_NUM_CHANNELS]);

#endif /* ADC124S_H */
