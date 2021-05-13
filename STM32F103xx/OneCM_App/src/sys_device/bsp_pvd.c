/*
 * bsp_pvd.c
 *
 *  Created on: 2020年11月18日
 *      Author: Saber
 */

/* PVD = Programmable Votage Detector 可编程电压监测器 *
当电压过低时，对系统的一些数据使用掉电不丢失的 EEPROM 或 Flash 保存起来，
同时对外设进行相应的保护操作。 
PVD的作用是监视供电电压，在供电电压下降到给定的阀值以下时，产生一个中断，通知软件做紧急处理。
当供电电压又恢复到给定的阀值以上时，也会产生一个中断，通知软件供电恢复。
供电下降的阀值与供电上升的PVD阀值有一个固定的差值，
引入这个差值的目的是为了防止电压在阀值上下小幅抖动，而频繁地产生中断。
利用其内部的PVD对VDD的电压进行监控
*/

#include "sys_device/bsp_pvd.h"

#include "misc.h"
#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_rcc.h"

#define NET2_LED_GPIO GPIOC
#define NET2_LED_PIN GPIO_Pin_13

#define NET2_TR() GPIO_WriteBit(NET2_LED_GPIO, NET2_LED_PIN, 1 - GPIO_ReadInputDataBit(NET2_LED_GPIO, NET2_LED_PIN))

// static void PVD_Configuration(void)
// {
//     PWR_PVDLevelConfig(PWR_PVDLevel_2V8); // 设定监控阈值
//     PWR_PVDCmd(ENABLE);
// }

static void RCC_Configuration(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
}

static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

static void EXTI_Configuration()
{
    EXTI_InitTypeDef EXTI_InitStructure;

    EXTI_ClearITPendingBit(EXTI_Line16);
    EXTI_InitStructure.EXTI_Line = EXTI_Line16; // PVD连接到中断线16上
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //表示电压从高电压下降到低于设定的电压阀值产生中断
    // EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; //表示电压从高电压下降到低于设定的电压阀值产生中断
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;

    EXTI_Init(&EXTI_InitStructure);
}

static void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = NET2_LED_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

    GPIO_Init(NET2_LED_GPIO, &GPIO_InitStructure);
    GPIO_SetBits(NET2_LED_GPIO, NET2_LED_PIN);

    return;
}

void PVD_Init(void)
{
    RCC_Configuration();
    NVIC_Configuration();
    EXTI_Configuration();
    GPIO_Configuration();
    // PVD_Configuration();

    PWR_ClearFlag(PWR_FLAG_PVDO);
    PWR_PVDLevelConfig(PWR_PVDLevel_2V9); // 设定监控阈值
    PWR_PVDCmd(ENABLE);
}

int8_t flag = 0;

// void PVD_IRQHandler(void)
// {

//     EXTI_ClearITPendingBit(EXTI_Line16); // 清除中断

//     if (PWR_GetFlagStatus(PWR_FLAG_PVDO) != RESET)
//     {
//         PWR_ClearFlag(PWR_FLAG_PVDO);

//         if (flag == 0)
//         {
//             flag = 1;
//         }
//         else
//         {
//             GPIO_ResetBits(TEST_LED_GPIO, TEST_LED_PIN);
//         }
//     }
// }

/**
 * @description: 掉电保存电能脉冲信息
 * @param {*}
 * @return {*}
 */
void PVD_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line16) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line16); // 清除中断

        if (PWR_GetFlagStatus(PWR_FLAG_PVDO) != RESET)
        {
            PWR_ClearFlag(PWR_FLAG_PVDO);

            /* code */
            // BL0940_Power_Down_Save();
        }
    }
}
