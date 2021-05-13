/*
 * sys_init.c
 *
 *  Created on: Dec 31, 2019
 *      Author: Fox
 */

#include "sys_device/sys_init.h"
// #include "common/timer_callback.h"

#include "stm32f10x.h"
#include "core_cmFunc.h"
#include "core_cm3.h"
#include "misc.h"

#define APP_BASE_ADDR ((uint32_t)0x00002800)

//系统时钟配置函数72M
void SystemClock_Config(void)
{
	ErrorStatus HSEStartUpStatus;
	RCC_DeInit();
	RCC_HSEConfig(RCC_HSE_ON);
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if (HSEStartUpStatus == SUCCESS)
	{
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		RCC_PCLK2Config(RCC_HCLK_Div1); //串口波特率的确定
		RCC_PCLK1Config(RCC_HCLK_Div2);
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
		RCC_PLLCmd(ENABLE);
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY == RESET))
		{
		}
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		while (RCC_GetSYSCLKSource() != 0x08)
		{
		}
	}
}

//初始化系统必要中断
void Sys_NVIC_Init(void)
{
#ifdef VETB_TAB_RAM
	NVYC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, APP_BASE_ADDR);
#endif
	//	Configure one bit for preemption priority
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
}

//初始化系统基本硬件环境
void sys_init_base(void)
{
	Sys_NVIC_Init();	  //初始化必要中断源
	SystemClock_Config(); //初始化必要时钟源
	//启动系统时钟信号，设置时间为1ms
	SysTick_Config(9000);								  //24位计数器，1ms,计数器=9000
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); //AHB=72M/8=9M
}

/**
 * @description: Reset the mcu by software
 * @param {*}
 * @return {*}
 */
void SystemReset(void)
{
	__set_FAULTMASK(1); // 关闭所有中断
	NVIC_SystemReset(); // 复位
}

//内核定时器中断函数
void SysTick_Handler(void)
{
	//实时定时回调任务
	//更新任务实时定时器，用于非实时处理
	// Timer_Sched();
}
