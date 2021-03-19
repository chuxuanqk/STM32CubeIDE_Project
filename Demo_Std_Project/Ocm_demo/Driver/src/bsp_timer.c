/*
 * @Author: your name
 * @Date: 2020-09-26 15:12:45
 * @LastEditTime: 2020-11-24 15:19:51
 * @LastEditors: Saber
 * @Description: In User Settings Edit
 * @FilePath: \Ocm_demo\Driver\src\bsp_timer.c
 */
/*
 * bsp_timer.c
 *
 *  Created on: 2020年9月26日
 *      Author: Administrator
 */

#include "bsp_timer.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

#include "led.h"
#include "uart.h"

static void RCC_Configuration(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
}

static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    //中断优先级 NVIC 设置
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;           //TIM3 中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //先占优先级 0 级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        //从优先级 1 级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           //IRQ 通道被使能

    NVIC_Init(&NVIC_InitStructure); //初始化 NVIC 寄存器
}

static void TIM_Configuration(struct TIM_Device_Cfg *cfg)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_Cmd(cfg->timer_type, DISABLE);

    TIM_TimeBaseStructure.TIM_Period = cfg->arr_period;            //设置自动重装载寄存器周期的值
    TIM_TimeBaseStructure.TIM_Prescaler = cfg->prescaler;          //设置时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = cfg->clock_division; //设置时钟分割
    TIM_TimeBaseStructure.TIM_CounterMode = cfg->counter_mode;     //TIM 向上计数

    TIM_TimeBaseInit(cfg->timer_type, &TIM_TimeBaseStructure); // 初始化定时器
    TIM_ITConfig(cfg->timer_type, TIM_IT_Update, ENABLE);      // 允许更新中断
    TIM_Cmd(cfg->timer_type, ENABLE);                          // 使能 TIM3
}

struct TIM_Device_Cfg tim3_cfg = {
    .timer_type = TIM3,
    .prescaler = 1, // 默认设置溢出频率为 Tout=10us,(arr+1)*(psc+1) = fclk(Hz) *Tout(s) = 72*10^6* 1*10^-5 = 720
    .arr_period = 359,
    .clock_division = TIM_CKD_DIV1,
    .counter_mode = TIM_CounterMode_Up,
};

static void tim_isr(struct TIM_Device_Cfg *cfg)
{
    if (TIM_GetITStatus(cfg->timer_type, TIM_IT_Update) != RESET) //检查 TIM 更新中断发生与否
    {
        TIM_ClearITPendingBit(cfg->timer_type, TIM_IT_Update); //清除 TIM 更新中断标志
        led_set_time(NET_LED, 1000, 900);
        led_set_mode(NET_LED, LED_Sharp_Repeat_MODE);
    }
}

void TIM3_IRQHandler(void) //TIM3 中断
{
    tim_isr(&tim3_cfg);
}

void hw_bsptimer_init(void)
{
    struct TIM_Device_Cfg *cfg;

    RCC_Configuration();
    NVIC_Configuration();

    // TIM3, (tim 溢出频率)2400 HZ = 72MHZ/((arr+1)(psc+1)) ==> arr=999, psc=29; Tout = 1/2400= 417us
    // tim3_cfg.arr_period = 999;
    // tim3_cfg.prescaler = 29;
    tim3_cfg.prescaler = 1999; // 1s 计数
    tim3_cfg.arr_period = 35999;
    cfg = &tim3_cfg;
    TIM_Configuration(cfg);
}
