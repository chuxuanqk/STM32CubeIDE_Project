/*
 * @Author: your name
 * @Date: 2020-09-22 14:56:47
 * @LastEditTime: 2020-09-22 15:06:47
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \Demo_Std_Project\Ocm_demo\common\timer.h
 */
/*
 * timer.h
 *
 *  Created on: 2020年9月22日
 *      Author: Administrator
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
    unsigned int delay;      // 延时时间
    unsigned int delay_temp; // 缓存延时时间
    unsigned int period;     // 定时器周期
    bool run;
    void *para;
};

typedef int8_t timer_id;

void timer_init(void);
timer_id timer_creat(void (*pFunction)(void *para),
                     const unsigned int delay,
                     const unsigned int period,
                     bool run,
                     void *para);
bool timer_delete(const timer_id index);
void timer_start(const timer_id index);
void timer_stop(const timer_id index);
void timer_sched(void);

void TickHandler(void);
void get_sys_uptime(uint32_t *psec_cnt, uint16_t *pmsec_cnt);

#endif /* TIMER_H_ */
