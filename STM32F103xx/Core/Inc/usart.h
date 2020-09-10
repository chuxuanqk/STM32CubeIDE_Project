/**
  ******************************************************************************
  * File Name          : USART.h
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
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
#ifndef __usart_H
#define __usart_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdbool.h>

#define SERIAL_USING_DMA
/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

/* USER CODE BEGIN Private defines */
  void UsartDmaPrintf(UART_HandleTypeDef *huart, const char *format, ...);
  void DMA_Usart_Send(UART_HandleTypeDef *huart, uint8_t *buf, uint8_t len);
/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART3_UART_Init(void);

/* USER CODE BEGIN Prototypes */

  typedef struct
  {
    UART_HandleTypeDef *huart;
    uint16_t index;
    bool recv_end_flag;
    uint16_t data_len;
    uint8_t data_buf[UART_BUF_SIZE];
  } ocm_usart_device; // 串口设备�??

  extern ocm_usart_device usart1_rc_dev;

  void MX_UART_IT_Init(void);                     // 串口接收中断方式初始�??
  void Usart_irq(ocm_usart_device *dev);          // 串口中断
  void Ocm_Uart_Recv_Deal(ocm_usart_device *dev); // 串口空闲中断数据处理demo

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ usart_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
