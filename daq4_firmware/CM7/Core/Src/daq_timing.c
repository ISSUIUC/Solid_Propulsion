/**
  * @file    daq_timing.c
  * @brief   Cycle-accurate timing from the Cortex-M7 DWT cycle counter.
  */
#include "daq_timing.h"

void daq_timing_init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->LAR = 0xC5ACCE55U; /* unlocks DWT writes on the Cortex-M7 */
  DWT->CYCCNT = 0U;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void daq_timing_delay_us(uint32_t us)
{
  const uint32_t start = daq_timing_cycles();
  const uint32_t wait = daq_timing_us_to_cycles(us);

  while ((daq_timing_cycles() - start) < wait)
  {
  }
}
