/**
  ******************************************************************************
  * File Name          : gpio.h
  * Description        : This file contains all the functions prototypes for
  *                      the gpio
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __gpio_H
#define __gpio_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */
#define ENABLE_TX_485 HAL_GPIO_WritePin(PGND_GPIO_Port, PGND_Pin, GPIO_PIN_SET)
#define ENABLE_RX_485 HAL_GPIO_WritePin(PGND_GPIO_Port, PGND_Pin, GPIO_PIN_RESET)

#define STATE_LED_ON HAL_GPIO_WritePin(STATE_LED_GPIO_Port, STATE_LED_Pin, GPIO_PIN_RESET)
#define STATE_LED_OFF HAL_GPIO_WritePin(STATE_LED_GPIO_Port, STATE_LED_Pin, GPIO_PIN_SET)
#define NET_LED_ON HAL_GPIO_WritePin(NET_LED_GPIO_Port, NET_LED_Pin, GPIO_PIN_RESET)
#define NET_LED_OFF HAL_GPIO_WritePin(NET_LED_GPIO_Port, NET_LED_Pin, GPIO_PIN_SET)

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ pinoutConfig_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
