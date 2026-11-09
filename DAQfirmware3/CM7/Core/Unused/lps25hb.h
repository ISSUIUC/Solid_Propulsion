/**
  * @file    lps25hb.h
  * @brief   ST LPS25HB barometer driver.
  */
#ifndef LPS25HB_H
#define LPS25HB_H

#include "spi_bus.h"

/* Checks WHO_AM_I (HAL_ERROR on mismatch), then starts continuous 25 Hz conversion. */
HAL_StatusTypeDef lps25hb_init(spi_bus_dev_t dev);

HAL_StatusTypeDef lps25hb_read(spi_bus_dev_t dev, float *pressure_hpa, float *temp_c);

#endif /* LPS25HB_H */
