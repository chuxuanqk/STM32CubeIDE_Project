/**
  ******************************************************************************
  * File Name          : USART.c
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

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "gpio.h"
#include "stdio.h"
#include "stdarg.h"
#include "string.h"

#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)

extern void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
extern void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

char rDataBuf1[1] = {0}; // 采用串口中断，每次接�?1个字节触发一次中�?
char rDataBuf3[1] = {0};

ocm_usart_device usart1_tx_dev;
ocm_usart_device usart1_rc_dev;
ocm_usart_device usart3_tx_dev;
ocm_usart_device usart3_rc_dev;

/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart3_tx;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }

}
/* USART3 init function */

void MX_USART3_UART_Init(void)
{

  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */
    usart1_tx_dev.huart = uartHandle;
    usart1_rc_dev.huart = uartHandle;
  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA1_Channel5;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart1_rx);

    /* USART1_TX Init */
    hdma_usart1_tx.Instance = DMA1_Channel4;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_MEDIUM;
    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart1_tx);

  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspInit 0 */
    usart3_tx_dev.huart = uartHandle;
    usart3_rc_dev.huart = uartHandle;
  /* USER CODE END USART3_MspInit 0 */
    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USART3 DMA Init */
    /* USART3_RX Init */
    hdma_usart3_rx.Instance = DMA1_Channel3;
    hdma_usart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart3_rx.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&hdma_usart3_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart3_rx);

    /* USART3_TX Init */
    hdma_usart3_tx.Instance = DMA1_Channel2;
    hdma_usart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_tx.Init.Mode = DMA_NORMAL;
    hdma_usart3_tx.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&hdma_usart3_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart3_tx);

  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10|GPIO_PIN_11);

    /* USART3 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
 * @description: 串口1/3接收中断初始�??
 * @param {type} 
 * @return {type} 
 */
void MX_UART_IT_Init(void)
{

#ifdef SERIAL_USING_DMA
  HAL_UART_Receive_DMA(&huart1, (uint8_t *)usart1_rc_dev.data_buf, UART_BUF_SIZE); // 设置DMA传输，将串口1的数据搬运到usart1_rc_buf.data_buf中，
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);                                     // 使能串口1空闲(idle)中断
  __HAL_UART_CLEAR_IDLEFLAG(&huart1);                                              // 清除空闲中断标志�??
  HAL_UART_Receive_DMA(&huart3, (uint8_t *)usart3_rc_dev.data_buf, UART_BUF_SIZE); // 设置DMA传输，将串口1的数据搬运到usart1_rc_buf.data_buf中，
  __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);                                     // 使能串口1空闲(idle)中断
  __HAL_UART_CLEAR_IDLEFLAG(&huart3);                                              // 清除空闲中断标志�??
#else
  HAL_UART_Receive_IT(&huart1, (uint8_t *)rDataBuf1, 1);
  HAL_UART_Receive_IT(&huart3, (uint8_t *)rDataBuf3, 1);
  __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_RXNE);
  __HAL_UART_CLEAR_FLAG(&huart3, UART_FLAG_RXNE);

#endif

  ENABLE_RX_485; // 485接收使能
}

/**
 * @name: HAL_UART_TxCpltCallback
 * @description: DMA串口传输完成回调函数
 * @param {type} 
 * @return {type} 
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART3)
  {
    ENABLE_RX_485; // 传输完成，使�??485接收
  }
  else if (huart->Instance == USART1)
  {
    /* pass */
    __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_TXE);
    // UsartDmaPrintf(huart, "TxCplt1 Complete\r\n");
  }
}

/**
 * @name: HAL_UART_RxCpltCallback·
 * @description: 串口接收完成回调函数, 采用DMA模式必须在数据超过接收长度才会触�??
 * @param {type} 
 * @return {type} 
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_RXNE);

#ifndef SERIAL_USING_DMA
  if (huart->Instance == USART1)
  {

    usart1_rc_dev.data_buf[usart1_rc_dev.index] = rDataBuf1[0];
    usart1_rc_dev.index++;

    if (rDataBuf1[0] == 0x0a)
    {
      usart1_rc_dev.recv_end_flag = true;
      usart1_rc_dev.index = 0;
    }
    UsartDmaPrintf(huart, "RxCplt1 Complete\r\n");

    HAL_UART_Receive_IT(&huart1, (uint8_t *)rDataBuf1, 1);
  }
  else if (huart->Instance == USART3)
  {
    /*pass data reduce*/
    UsartDmaPrintf(huart, "RxCplt3 Complete\r\n");

    HAL_UART_Receive_IT(huart, (uint8_t *)rDataBuf3, 1);
  }

#endif
}

/**
 * @description: 
 * @param {type} 
 * @return {type} 
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart == &huart1)
  {
    UsartDmaPrintf(&huart1, "Error\r\n");
  }
}

/**
 * @name: UART_IDLE_IRQ
 * @description: 在第�??次上电串口处于空闲状态，会有很多\0数据。每次接收完数据之后1个字节的时间触发串口空闲中断
 *                (ps：HAL_UART_Receive_DMA不能放在HAL_UART_MspInit)
 *                不采用DMA完全接收中断，由于DMA完全接收中断只�?�合于接收定长数据处�??
 *                当发送方数据量比较大且在10ms的频次发送数据，会出现无法触发空闲中断的现象�??
 *                数据量在12字节以内可以实现1ms数据的连续收发�?�具体问题还未发现是�??么，只知道触发的是DMA中断�??
 * @param {type} 
 * @return {type} 
 */
void UART_IDLE_IRQ(UART_HandleTypeDef *huart)
{
  uint32_t temp;
  ocm_usart_device *tbufuart = NULL;
  DMA_HandleTypeDef *hdma_temp = NULL;

  if ((__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET) &&  // idle标志被置�??
      (__HAL_UART_GET_IT_SOURCE(huart, UART_IT_IDLE) != RESET)) // 解决初始上电，有许多\0数据问题
  {

    HAL_UART_DMAStop(huart);

    if (huart->Instance == USART1)
    {
      hdma_temp = &hdma_usart1_rx;
      tbufuart = &usart1_rc_dev;
    }
    else if (huart->Instance == USART3)
    {
      hdma_temp = &hdma_usart3_rx;
      tbufuart = &usart3_rc_dev;
    }

    temp = __HAL_DMA_GET_COUNTER(hdma_temp);   // 获取DMA中未传输的数据个�??
    tbufuart->data_len = UART_BUF_SIZE - temp; // 总计数减去未传输的数据个�??,得到已经接收的数据个�??

    tbufuart->recv_end_flag = true; // 接收完成标志位置1

    __HAL_UART_CLEAR_IDLEFLAG(huart); // 清除标志

    HAL_UART_Receive_DMA(huart, (uint8_t *)tbufuart->data_buf, UART_BUF_SIZE); // 设置DMA传输，将串口1的数据搬运到usart1_rc_buf.data_buf�??
  }
}

/**
 * @description: 串口空闲中断数据处理demo
 * @param {type} 
 * @return {type} 
 */
void Ocm_Uart_Recv_Deal(ocm_usart_device *dev)
{
  if (dev->recv_end_flag == true)
  {
    dev->recv_end_flag = false;

    DMA_Usart_Send(dev->huart, (uint8_t *)dev->data_buf, dev->data_len);
    memset(dev->data_buf, 0, dev->data_len);
    dev->data_len = 0;
  }
}

/**
 * @name: 
 * @description: 重写fputc，串�?????1作为打印调试
 * @param {type} 
 * @return {type} 
 */
PUTCHAR_PROTOTYPE
{
  while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET)
  {
  }
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}

/**
 * @name: 
 * @description: 通过DMA数据传输
 * @param {type} 
 * @return {type} 
 */
void UsartDmaPrintf(UART_HandleTypeDef *huart, const char *format, ...)
{
  uint16_t len;
  va_list args;
  char *pUartTxBuf = NULL;

  if (huart == &huart1)
  {
    pUartTxBuf = (char *)&(usart1_tx_dev.data_buf[0]);
  }
  else if (huart == &huart3)
  {
    // 串口3用作485通信
    ENABLE_TX_485;
    pUartTxBuf = (char *)&(usart3_tx_dev.data_buf[0]);
  }

  memset(pUartTxBuf, 0, UART_BUF_SIZE);
  va_start(args, format);
  len = vsnprintf((char *)pUartTxBuf, UART_BUF_SIZE + 1, (char *)format, args);
  va_end(args);

  while (HAL_UART_Transmit_DMA(huart, (uint8_t *)pUartTxBuf, len) != HAL_OK) // DMA send Data从内存到外设uart
    Error_Handler();
}

/**
 * @name: DMA_Usart_Send
 * @description: 根据数据地址和长度，由DMA发�?�数�??
 * @param {type} 
 * @return {type} 
 */
void DMA_Usart_Send(UART_HandleTypeDef *huart, uint8_t *buf, uint8_t len)
{
  char *pUartTxBuf = NULL;

  if (huart == &huart1)
  {
    pUartTxBuf = (char *)usart1_tx_dev.data_buf;
  }
  else if (huart == &huart3)
  {
    ENABLE_TX_485;
    pUartTxBuf = (char *)usart3_tx_dev.data_buf;
  }

  memcpy(pUartTxBuf, buf, len);

  while (HAL_UART_Transmit_DMA(huart, (uint8_t *)pUartTxBuf, len) != HAL_OK) // DMA send Data 从内存到外设uart
    Error_Handler();
}

/**
 * @description: 串口中断处理
 * @param {type} 
 * @return {type} 
 */
void Usart_irq(ocm_usart_device *dev)
{
#ifdef SERIAL_USING_DMA
  uint32_t temp;

  if ((__HAL_UART_GET_FLAG(dev->huart, UART_FLAG_IDLE) != RESET) &&  // idle标志被置�??
      (__HAL_UART_GET_IT_SOURCE(dev->huart, UART_IT_IDLE) != RESET)) // 解决初始上电，有许多\0数据问题
  {
    HAL_UART_DMAStop(dev->huart);

    temp = __HAL_DMA_GET_COUNTER(dev->huart->hdmarx);
    dev->data_len = UART_BUF_SIZE - temp;

    dev->recv_end_flag = true;

    __HAL_UART_CLEAR_IDLEFLAG(dev->huart);

    while (HAL_UART_Receive_DMA(dev->huart, (uint8_t *)dev->data_buf, UART_BUF_SIZE) != HAL_OK)
      ;
  }

#endif
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
