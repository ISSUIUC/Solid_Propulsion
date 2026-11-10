/**
  * @file    ads7066.h
  * @brief   TI ADS7066 8-channel, 16-bit, 250 kSPS SAR ADC driver.
  *
  * Written from the TI datasheet, SBAS928C (revised September 2023).
  * The ADC runs in auto-sequence mode over all 8 channels, with the channel ID
  * appended to each result. Every result is checked against the channel it is
  * stored under. CRC and averaging are off.
  */
#ifndef ADS7066_H
#define ADS7066_H

#include <stdbool.h>
#include "spi_bus.h"

#define ADS7066_NUM_CHANNELS 8U
#define ADS7066_FULL_SCALE   65536.0f

typedef struct
{
  bool internal_ref; /* true: enable the 2.5 V internal reference (REF pin has only a capacitor) */
} ads7066_config_t;

/* Resets the ADC and checks it answers. Then calibrates its offset and starts sequencing
   AIN0-AIN7. Takes about 15 ms. */
HAL_StatusTypeDef ads7066_init(spi_bus_dev_t dev, const ads7066_config_t *cfg);

/* Converts all 8 channels and returns raw 0-65535 counts, indexed by channel.
   Takes about 60 us at 16 MHz SCLK. raw is unchanged on failure. */
HAL_StatusTypeDef ads7066_read_all(spi_bus_dev_t dev, uint16_t raw[ADS7066_NUM_CHANNELS]);

HAL_StatusTypeDef ads7066_read_reg(spi_bus_dev_t dev, uint8_t addr, uint8_t *value);
HAL_StatusTypeDef ads7066_write_reg(spi_bus_dev_t dev, uint8_t addr, uint8_t value);

/* Input voltage for a raw count, with the 1x VREF input range set by ads7066_init(). */
static inline float ads7066_to_volts(uint16_t raw, float vref)
{
  return (float)raw * vref / ADS7066_FULL_SCALE;
}

#endif /* ADS7066_H */
