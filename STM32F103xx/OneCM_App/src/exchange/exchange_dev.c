/*
 * exchange_dev.c
 *
 *  Created on: 2020年10月28日
 *      Author: Saber
 */

/********初始化扩展硬件*****************************************************************************************
    step 1 初始化片内外设
    step 2 注册实时任务
    setp 3 注册非实时任务
    setp 4 初始化虚拟硬件状态
    notice 1 显式注册实时定时任务，默认10个定时任务，增加再考虑所有定时任务总体指令数后，在common/timer.h中修改
    notice 2 建议定时指令总数不要超过1ms可执行指令数的30%
    notice 3 非实时定时器可注册任意数量，默认20个，在common/timer_callback.h中修改
    notice 4 系统最小初始化完成了主时钟、定时等功能，虚拟设备加载需要片内外设、中断、实时定时、非实时扫描等组件
**************************************************************************************************************/

#include "channel/bsp_uart.h"
#include "exchange/exchange_dev.h"

void exchange_dev_init(void)
{
    // 初始化usart
    Hw_Uart_Init();

    return;
}
