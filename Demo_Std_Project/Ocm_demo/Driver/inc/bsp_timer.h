/*
 * @Author: your name
 * @Date: 2020-09-26 15:13:06
 * @LastEditTime: 2020-09-26 18:33:15
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \Ocm_demo\Driver\inc\bsp_timer.h
 */
/*
 * bsp_timer.h
 *
 *  Created on: 2020年9月26日
 *      Author: Administrator
 */

#ifndef BSP_TIMER_H_
#define BSP_TIMER_H_

#include "stm32f10x.h"
#include "stm32f10x_tim.h"

#include "stdbool.h"

#define DALI_TIMER TIM3

struct TIM_Device
{
    TIM_TypeDef *timer_type;
    __IO uint16_t SR;
};

struct TIM_Device_Cfg
{
    TIM_TypeDef *timer_type;

    uint16_t prescaler;      // 预分频值
    uint16_t arr_period;     // 自动重装载值
    uint16_t clock_division; // 时钟分频
    uint16_t counter_mode;   // 计数方式
};

#define TIM_SR_UPDATE ((uint16_t)0x0001)

void hw_bsptimer_init(void);

void TIM_SetStatus(TIM_TypeDef *TIMx, uint16_t TIM_FLAG);
bool TIM_GetStatus(TIM_TypeDef *TIMx, uint16_t TIM_FLAG);
void TIM_ClearStatus(TIM_TypeDef *TIMx, uint16_t TIM_FLAG);

#endif /* BSP_TIMER_H_ */
