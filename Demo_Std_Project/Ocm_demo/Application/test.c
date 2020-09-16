/*
 * test.c
 *
 *  Created on: 2020年9月14日
 *      Author: Administrator
 */

#include "uart.h"
#include "test.h"

void uart_test(void)
{
  uint16_t size;
  uint8_t buf_rc[20];

   size = uart_read(USART1, buf_rc, 5);
  if(size != -1)
  {
      uart_write(USART1, buf_rc, size);
  }
}