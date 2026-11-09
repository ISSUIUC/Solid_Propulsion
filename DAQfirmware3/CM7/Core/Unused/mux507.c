/**
  * @file    mux507.c
  * @brief   TI MUX507 analog mux control. U3 and U6 share their address lines.
  */
#include "mux507.h"
#include "daq_board.h"

static void mux507_init_pin(GPIO_TypeDef *port, uint16_t pin)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void mux507_init(void)
{
  DAQ_BOARD_GPIO_CLK_ENABLE();

  mux507_init_pin(DAQ_MUX_U3_EN_PORT, DAQ_MUX_U3_EN_PIN);
  mux507_init_pin(DAQ_MUX_U6_EN_PORT, DAQ_MUX_U6_EN_PIN);
  mux507_init_pin(DAQ_MUX_A0_PORT, DAQ_MUX_A0_PIN);
  mux507_init_pin(DAQ_MUX_A1_PORT, DAQ_MUX_A1_PIN);
  mux507_init_pin(DAQ_MUX_A2_PORT, DAQ_MUX_A2_PIN);
}

HAL_StatusTypeDef mux507_select(mux507_t mux, uint8_t channel)
{
  uint8_t address;

  if ((channel < 1U) || (channel > 8U))
  {
    return HAL_ERROR;
  }
  address = channel - 1U;

  HAL_GPIO_WritePin(DAQ_MUX_U3_EN_PORT, DAQ_MUX_U3_EN_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(DAQ_MUX_U6_EN_PORT, DAQ_MUX_U6_EN_PIN, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(DAQ_MUX_A0_PORT, DAQ_MUX_A0_PIN, (address & 0x1U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(DAQ_MUX_A1_PORT, DAQ_MUX_A1_PIN, (address & 0x2U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(DAQ_MUX_A2_PORT, DAQ_MUX_A2_PIN, (address & 0x4U) ? GPIO_PIN_SET : GPIO_PIN_RESET);

  if (mux == MUX507_U3)
  {
    HAL_GPIO_WritePin(DAQ_MUX_U3_EN_PORT, DAQ_MUX_U3_EN_PIN, GPIO_PIN_SET);
  }
  else
  {
    HAL_GPIO_WritePin(DAQ_MUX_U6_EN_PORT, DAQ_MUX_U6_EN_PIN, GPIO_PIN_SET);
  }
  return HAL_OK;
}
