/*
 * gpio.h
 *
 *  Created on: 2020年9月15日
 *      Author: Administrator
 */

#ifndef DRIVER_INC_GPIO_H_
#define DRIVER_INC_GPIO_H_


#include "stm32f10x.h"

#define NET_LED_TRIGGLE	GPIO_WriteBit(GPIOB, GPIO_Pin_13, 1-GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_13))

void hw_gpio_init(void);

#endif /* DRIVER_INC_GPIO_H_ */
