/*
 * @Author: your name
 * @Date: 2020-09-15 13:33:03
 * @LastEditTime: 2020-09-23 16:59:19
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: \Demo_Std_Project\Ocm_demo\Driver\src\gpio.c
 */
/*
 * gpio.c
 *
 *  Created on: 2020年9月15日
 *      Author: Administrator
 */

#include "gpio.h"

static void RCC_Configuration(void)
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
}

static void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  /* Configure USART Rx/tx PIN */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_SetBits(GPIOB, GPIO_Pin_13);
  GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}

void hw_gpio_init(void)
{
  RCC_Configuration();
  GPIO_Configuration();
}
