/*
 * uart_485.c
 *
 *  Created on: 2020年9月16日
 *      Author: Administrator
 */

#include "uart_485.h"

static void RCC_Configuration(void)
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
}

static void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  UART3_485_RECV();
}

void uart_tc_isr_hook(void)
{
  UART3_485_RECV();
}

uint16_t uart_485_read(USART_TypeDef* uartx, uint8_t *pbuf, uint8_t size)
{
  return uart_read(uartx, pbuf, size);
}

uint16_t uart_485_write(USART_TypeDef* uartx, uint8_t *pbuf, uint8_t size)
{
  UART3_485_SEND();
  return uart_write(uartx, pbuf, size);
}

void hw_uart_485_init(void)
{
  RCC_Configuration();
  GPIO_Configuration();
}
