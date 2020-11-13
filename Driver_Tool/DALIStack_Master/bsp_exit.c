/*
 * @Author: your name
 * @Date: 2020-09-28 14:43:35
 * @LastEditTime: 2020-10-26 10:56:44
 * @LastEditors: Saber
 * @Description: In User Settings Edit
 * @FilePath: \DALI_DEMO\code\Ocm_demo\Driver\src\bsp_exit.c
 */
/*
 * bsp_exit.c
 *
 *  Created on: 2020年9月28日
 *      Author: Administrator
 */

#include "bsp_exit.h"

#include "misc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"


static void RCC_Configuration(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); //开启 AFIO 时钟
}

static void EXIT_Configuration(void)
{
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);

    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; // 上升沿检测
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;

    EXTI_Init(&EXTI_InitStructure);
}

static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the EXTI0 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;

    NVIC_Init(&NVIC_InitStructure);
    EXTI_ClearITPendingBit(EXTI_Line0);
}

void EXITX_SetStatus(uint32_t EXTI_Line, bool flag)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;

    if (flag)
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    else
        NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;

    NVIC_Init(&NVIC_InitStructure);
    EXTI_ClearITPendingBit(EXTI_Line0);
}

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        /* Clear the EXTI Line 0 */
        EXTI_ClearITPendingBit(EXTI_Line0);
        EXITX_SetStatus(EXTI_Line0, DISABLE);

        EXIT0_Hook();
    }
}

void hw_exit_init(void)
{
    RCC_Configuration();
    EXIT_Configuration();
    NVIC_Configuration();
}
