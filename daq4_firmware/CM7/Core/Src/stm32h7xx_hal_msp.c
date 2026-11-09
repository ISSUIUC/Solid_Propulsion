/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32h7xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* USER CODE BEGIN Includes */
#include "daq_board.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */
static void daq_spi_pin_init(GPIO_TypeDef *port, uint16_t pin, uint32_t af)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; /* 16 MHz SCLK */
  GPIO_InitStruct.Alternate = af;
  HAL_GPIO_Init(port, &GPIO_InitStruct);
}

/* Chip selects are left out: spi_bus.c drives them as GPIOs */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  if ((hspi->Instance != DAQ_SPI_TC_INSTANCE) && (hspi->Instance != DAQ_SPI_PT_INSTANCE))
  {
    return;
  }

  /* SPI1-3 share one kernel clock. The reset default source is PLL1Q, but the PLLs
     are off. CKPER defaults to HSI: 64 MHz. TODO(pin map): if the second bus moves
     to SPI4/5/6, it needs its own clock source here. */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI123;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_CLKP;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  DAQ_BOARD_GPIO_CLK_ENABLE();

  if (hspi->Instance == DAQ_SPI_TC_INSTANCE)
  {
    DAQ_SPI_TC_CLK_ENABLE();
    daq_spi_pin_init(DAQ_SPI_TC_SCK_PORT, DAQ_SPI_TC_SCK_PIN, DAQ_SPI_TC_AF);
    daq_spi_pin_init(DAQ_SPI_TC_MISO_PORT, DAQ_SPI_TC_MISO_PIN, DAQ_SPI_TC_AF);
    daq_spi_pin_init(DAQ_SPI_TC_MOSI_PORT, DAQ_SPI_TC_MOSI_PIN, DAQ_SPI_TC_AF);
  }
  else
  {
    DAQ_SPI_PT_CLK_ENABLE();
    daq_spi_pin_init(DAQ_SPI_PT_SCK_PORT, DAQ_SPI_PT_SCK_PIN, DAQ_SPI_PT_AF);
    daq_spi_pin_init(DAQ_SPI_PT_MISO_PORT, DAQ_SPI_PT_MISO_PIN, DAQ_SPI_PT_AF);
    daq_spi_pin_init(DAQ_SPI_PT_MOSI_PORT, DAQ_SPI_PT_MOSI_PIN, DAQ_SPI_PT_AF);
  }
}

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{

  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  /* System interrupt init*/

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
