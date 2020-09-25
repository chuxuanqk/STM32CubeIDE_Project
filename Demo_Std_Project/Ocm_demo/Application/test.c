/*
 * @Author: your name
 * @Date: 2020-09-14 16:35:10
 * @LastEditTime: 2020-09-25 18:12:17
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \Demo_Std_Project\Ocm_demo\Application\test.c
 */
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
  uint16_t size = -1;
  uint8_t buf_rc[40];

  if (uart_getFlagStatus(USART1, UART_FLAG_RC))
  {
    size = uart_read(USART1, buf_rc, 2);
    uart_write(USART1, buf_rc, size);
    memset(buf_rc, 0, sizeof(buf_rc));
  }
}
