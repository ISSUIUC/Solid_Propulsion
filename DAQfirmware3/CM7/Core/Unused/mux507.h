/**
  * @file    mux507.h
  * @brief   TI MUX507 analog mux control. U3 and U6 share their address lines.
  */
#ifndef MUX507_H
#define MUX507_H

#include "stm32h7xx_hal.h"

typedef enum
{
  MUX507_U3 = 0,
  MUX507_U6
} mux507_t;

/* Configures the control lines with both muxes disabled. */
void mux507_init(void);

/* Disables both muxes, sets the address, then enables mux on channel 1-8.
   The caller must allow the signal to settle before sampling it. */
HAL_StatusTypeDef mux507_select(mux507_t mux, uint8_t channel);

#endif /* MUX507_H */
