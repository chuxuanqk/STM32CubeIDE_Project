/*
 * timer.h
 *
 *  Created on: 2020年9月22日
 *      Author: Saber
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

struct timer_t
{
    void (*pfun)(void *para);
    uint32_t delay;      // 延时时间
    uint32_t delay_temp; // 缓存延时时间
    uint32_t period;     // 定时器周期
    bool run;
    void *para;
};

typedef int8_t timer_id;

void Timer_Init(void);
timer_id Timer_Creat(void (*pFunction)(void *para),
                     const unsigned int delay,
                     const unsigned int period,
                     bool run,
                     void *para);
bool Timer_Delete(const timer_id index);
void Timer_Start(const timer_id index);
void Timer_Stop(const timer_id index);
void Timer_Sched(void);

void TickHandler(void);
uint32_t Get_Sys_Mscnt(void);
void Delay_ms(uint32_t ms);

#define Register_Realtime_Repeat_Timer_1ms(fun_cb) Timer_Creat((void *)fun_cb, 1, 0, true, NULL) // 兼容以前项目1ms函数接口，默认回调函数为无参

#endif /* TIMER_H_ */
