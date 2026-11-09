/**
  * @file    daq_board.h
  * @brief   Board wiring for the DAQ: two SPI buses, one TI ADS7066 ADC on each.
  *
  *   Thermocouple ADC (8 thermocouple amps)          -> SPI1
  *   Pressure transducer / load cell ADC ("ADC 2")   -> SPI2
  *
  * TODO(pin map): the board pin map is NOT decided yet. Every pin below is a
  * placeholder. SPI1 on PA4-PA7 matches DAQfirmware3.ioc; the SPI2 pins are
  * common SPI2 pins on the STM32H745IIT6 (LQFP176), chosen only so the code
  * compiles. When the schematic is done:
  *   1. Set every port/pin/AF below, and check each AF in the STM32H745 datasheet
  *      alternate function table.
  *   2. Update DAQ_BOARD_GPIO_CLK_ENABLE() to cover every port used.
  *   3. If a bus moves to another SPI, change its INSTANCE and CLK_ENABLE. SPI4/5/6
  *      also need their own kernel clock source in HAL_SPI_MspInit (stm32h7xx_hal_msp.c).
  *   4. Uncomment DAQ_BOARD_PINS_CONFIRMED.
  * Until then the build stops on purpose, because these pins are driven as outputs.
  */
#ifndef DAQ_BOARD_H
#define DAQ_BOARD_H

#include "stm32h7xx_hal.h"

/* #define DAQ_BOARD_PINS_CONFIRMED */

#ifndef DAQ_BOARD_PINS_CONFIRMED
#error "Pin map not decided: set the pins in daq_board.h, then define DAQ_BOARD_PINS_CONFIRMED"
#endif

/* ---- Bus A: thermocouple ADC (ADS7066) --------------------------------------
   TODO(pin map): placeholder, taken from DAQfirmware3.ioc (daq4_firmware.ioc has no SPI yet) */
#define DAQ_SPI_TC_INSTANCE   SPI1
#define DAQ_SPI_TC_CLK_ENABLE() __HAL_RCC_SPI1_CLK_ENABLE()
#define DAQ_SPI_TC_AF         GPIO_AF5_SPI1
#define DAQ_SPI_TC_SCK_PORT   GPIOA
#define DAQ_SPI_TC_SCK_PIN    GPIO_PIN_5
#define DAQ_SPI_TC_MISO_PORT  GPIOA
#define DAQ_SPI_TC_MISO_PIN   GPIO_PIN_6
#define DAQ_SPI_TC_MOSI_PORT  GPIOA
#define DAQ_SPI_TC_MOSI_PIN   GPIO_PIN_7
#define DAQ_CS_TC_PORT        GPIOA /* software chip select, active low */
#define DAQ_CS_TC_PIN         GPIO_PIN_4

/* ---- Bus B: pressure transducer / load cell ADC (ADS7066) -------------------
   TODO(pin map): placeholder, not from any schematic */
#define DAQ_SPI_PT_INSTANCE   SPI2
#define DAQ_SPI_PT_CLK_ENABLE() __HAL_RCC_SPI2_CLK_ENABLE()
#define DAQ_SPI_PT_AF         GPIO_AF5_SPI2
#define DAQ_SPI_PT_SCK_PORT   GPIOB
#define DAQ_SPI_PT_SCK_PIN    GPIO_PIN_13
#define DAQ_SPI_PT_MISO_PORT  GPIOB
#define DAQ_SPI_PT_MISO_PIN   GPIO_PIN_14
#define DAQ_SPI_PT_MOSI_PORT  GPIOB
#define DAQ_SPI_PT_MOSI_PIN   GPIO_PIN_15
#define DAQ_CS_PT_PORT        GPIOB /* software chip select, active low */
#define DAQ_CS_PT_PIN         GPIO_PIN_12

/* ---- ADC reference and supply ----------------------------------------------
   Decided 2026-11-05: both ADS7066s use the internal 2.5 V reference, and the board
   supplies 3.3 V from an external regulator. The input range is 0 V to VREF (2.5 V)
   (RANGE = 0). The 2x VREF range (5 V) is not usable with a 3.3 V supply, because an
   input must stay below AVDD + 0.3 V. Anything above 2.5 V reads as full scale.
   The REF pin needs only a 1-10 uF capacitor to GND (datasheet section 7.3.2.2),
   with no external reference connected. */
#define DAQ_ADC_TC_INTERNAL_REF  1
#define DAQ_ADC_TC_VREF_V        2.5f
#define DAQ_ADC_PT_INTERNAL_REF  1
#define DAQ_ADC_PT_VREF_V        2.5f

/* Must enable the clock of every port used above. TODO(pin map) */
#define DAQ_BOARD_GPIO_CLK_ENABLE() \
  do {                              \
    __HAL_RCC_GPIOA_CLK_ENABLE();   \
    __HAL_RCC_GPIOB_CLK_ENABLE();   \
  } while (0)

#endif /* DAQ_BOARD_H */
