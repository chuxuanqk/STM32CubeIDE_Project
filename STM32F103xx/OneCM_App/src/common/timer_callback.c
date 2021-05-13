/*
 * timer.c
 *
 *  Created on: 2020年9月22日
 *      Author: Saber
 */

#include "common/timer_callback.h"

#define OS_TIMER_MAX 16

struct timer_t timer_list[OS_TIMER_MAX];

// #define SYS_TICK_RATE_HZ 1000
// static volatile uint16_t millisec_increment = 1000 / SYS_TICK_RATE_HZ;
static volatile uint64_t sys_ms_cnt = 0;

/**
 * @description: 需要1ms定时器支持
 * @param {type} 
 * @return {type} 
 */
void TickHandler(void)
{
    sys_ms_cnt++;
}

/**
 * @description: 获取ms计数值
 * @param {*}
 * @return {*}
 */
uint32_t Get_Sys_Mscnt(void)
{
    return sys_ms_cnt;
}

/**
 * @description: 阻塞延时
 * @param {*}
 * @return {*}
 */
void Delay_ms(uint32_t ms)
{
    uint64_t sys_ms_pre = sys_ms_cnt;
    sys_ms_pre = sys_ms_pre + ms;

    while (sys_ms_pre > sys_ms_cnt)
    {
    }

    return;
}

void Timer_Init(void)
{
    uint8_t index;

    for (index = 0; index < OS_TIMER_MAX; index++)
    {
        timer_list[index].pfun = NULL;
    }
}

/**
 * @description: period : 0 一直重复执行， n(>0)  重复执行n次
 * @param {type} 
 * @return {type} 
 */
timer_id Timer_Creat(void (*pFunction)(void *para),
                     const unsigned int delay,
                     const unsigned int period,
                     bool run,
                     void *para)
{
    timer_id index = 0;

    while ((timer_list[index].pfun != NULL) && (index < OS_TIMER_MAX))
    {
        index++;
    }

    if (index < OS_TIMER_MAX)
    {
        // printf("time index %d\r\n",index);
        timer_list[index].pfun = pFunction;
        timer_list[index].delay = delay;
        timer_list[index].delay_temp = delay;
        timer_list[index].period = period;
        timer_list[index].run = run;
        timer_list[index].para = para;
    }
    else
    {
        index = -1;
    }

    return index;
}

bool Timer_Delete(const timer_id index)
{
    if (index >= OS_TIMER_MAX)
        return false;

    if (timer_list[index].pfun == NULL)
    {
        return false;
    }
    timer_list[index].pfun = NULL;
    timer_list[index].delay = 0;
    timer_list[index].period = 0;
    timer_list[index].run = false;
    timer_list[index].para = NULL;
    return true;
}

void Timer_Start(const timer_id index)
{
    if (index >= OS_TIMER_MAX)
        return;

    timer_list[index].run = true;
}

void Timer_Stop(const timer_id index)
{
    if (index >= OS_TIMER_MAX)
        return;

    timer_list[index].run = false;
}

/**
 * @description: period : 0 一直重复执行， n(>0)  重复执行n次
 * @param {type} 
 * @return {type} 
 */
inline void Timer_Sched(void)
{
    uint8_t index;

    for (index = 0; index < OS_TIMER_MAX; index++)
    {
        if (timer_list[index].delay == 0)
        {
            if (timer_list[index].run)
            {
                (*timer_list[index].pfun)(timer_list[index].para);

                if (timer_list[index].period == 0)
                {
                    timer_list[index].delay = timer_list[index].delay_temp;
                }
                else if (timer_list[index].period == 1)
                {
                    Timer_Delete(index);
                }
                else
                {
                    timer_list[index].delay = timer_list[index].delay_temp;
                    --timer_list[index].period;
                }
            }
        }
        else
        {
            --timer_list[index].delay;
        }
    }
}
