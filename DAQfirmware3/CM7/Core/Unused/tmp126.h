/**
  * @file    tmp126.h
  * @brief   TI TMP126 temperature sensor driver.
  */
#ifndef TMP126_H
#define TMP126_H

#include "spi_bus.h"

/* Reads the temperature result register, in degrees C. */
HAL_StatusTypeDef tmp126_read_temp(spi_bus_dev_t dev, float *temp_c);

#endif /* TMP126_H */
