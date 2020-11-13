/*
 * @Author: your name
 * @Date: 2020-09-28 14:43:21
 * @LastEditTime: 2020-10-30 15:20:18
 * @LastEditors: Saber
 * @Description: In User Settings Edit
 * @FilePath: \code\src\virtual_device\driver\bsp_exit.h
 */
/*
 * bsp_exit.h
 *
 *  Created on: 2020年9月28日
 *      Author: Administrator
 */

#ifndef BSP_EXIT_H_
#define BSP_EXIT_H_

#include "stm32f10x_exti.h"
#include "stdbool.h"

void hw_exit_init(void);
void EXITX_SetStatus(uint32_t EXTI_Line, bool flag);

void __attribute__((weak)) EXIT0_Hook(void);

#endif /* BSP_EXIT_H_ */
