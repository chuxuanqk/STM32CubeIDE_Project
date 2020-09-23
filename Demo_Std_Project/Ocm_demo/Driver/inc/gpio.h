/*
 * @Author: your name
 * @Date: 2020-09-15 13:33:14
 * @LastEditTime: 2020-09-23 16:39:46
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: \Demo_Std_Project\Ocm_demo\Driver\inc\gpio.h
 */
/*
 * gpio.h
 *
 *  Created on: 2020年9月15日
 *      Author: Administrator
 */

#ifndef DRIVER_INC_GPIO_H_
#define DRIVER_INC_GPIO_H_

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

#define NET_LED_TRIGGLE GPIO_WriteBit(GPIOB, GPIO_Pin_13, 1 - GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_13))

void hw_gpio_init(void);

#endif /* DRIVER_INC_GPIO_H_ */
