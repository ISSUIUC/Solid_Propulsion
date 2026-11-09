/**
  * @file    daq_timing.h
  * @brief   Cycle-accurate timing from the Cortex-M7 DWT cycle counter.
  */
#ifndef DAQ_TIMING_H
#define DAQ_TIMING_H

#include "stm32h7xx_hal.h"

/* Starts the cycle counter. Call once after SystemClock_Config(). */
void daq_timing_init(void);

/* Free-running CPU cycle count. Wraps about every 67 s at 64 MHz; compare with subtraction. */
static inline uint32_t daq_timing_cycles(void)
{
  return DWT->CYCCNT;
}

static inline uint32_t daq_timing_us_to_cycles(uint32_t us)
{
  return us * (SystemCoreClock / 1000000U);
}

/* Busy-waits for at least us microseconds. */
void daq_timing_delay_us(uint32_t us);

#endif /* DAQ_TIMING_H */
