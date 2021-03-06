/*
 * uart_485.h
 *
 *  Created on: 2020年9月16日
 *      Author: Administrator
 */

#ifndef DRIVER_INC_UART_485_H_
#define DRIVER_INC_UART_485_H_

#include "uart.h"
#include "stm32f10x.h"

#define UART3_485_SEND() (GPIO_SetBits(GPIOB, GPIO_Pin_12))
#define UART3_485_RECV() (GPIO_ResetBits(GPIOB, GPIO_Pin_12))

void hw_uart_485_init(void);
uint16_t uart_485_read(USART_TypeDef* uartx, uint8_t *pbuf, uint8_t size);
uint16_t uart_485_write(USART_TypeDef* uartx, uint8_t *pbuf, uint8_t size);


#endif /* DRIVER_INC_UART_485_H_ */
