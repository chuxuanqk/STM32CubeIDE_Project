/*
 * timer.c
 *
 *  Created on: 2020年9月22日
 *      Author: Saber
 */

#include "timer_callback.h"

#define OS_TIMER_MAX 16

struct timer_t timer_list[OS_TIMER_MAX];

#define SYS_TICK_RATE_HZ 1000

static volatile uint32_t sys_delayms_cmt;
static volatile uint32_t sys_seconds_cnt;
static volatile uint16_t sys_millisec_cnt;
static volatile uint16_t millisec_increment = 1000 / SYS_TICK_RATE_HZ;

/**
 * @description: 需要1ms定时器支持
 * @param {type} 
 * @return {type} 
 */
void TickHandler(void)
{
    sys_millisec_cnt += millisec_increment;
    if (sys_millisec_cnt >= 1000)
    {
        sys_millisec_cnt -= 1000;
        sys_seconds_cnt++;
        // NET_LED_TRIGGLE;
        // uart_send(USART1, "HE", 2);
    }
}

/**
 * @description: 
 * @param {type} 
 * @return {type} 
 */
void TickDelayms(void)
{
    sys_delayms_cmt++;
}

/**
 * @description: ms 延时, 阻塞方式; 不要用于长时间延时, 
 *               TODO: 需要修改, 由于 while()及之后语句占用了时间，导致延时不准确
 * @param {type} 
 * @return {type} 
 */
void delay_ms(uint32_t tms)
{
    uint32_t dtms = tms;
    timer_id ms_id = -1;
    sys_delayms_cmt = 0;

    ms_id = timer_creat((void *)TickDelayms, 1, 0, true, NULL);

    while (sys_delayms_cmt != dtms)
    {
    }
    timer_delete(ms_id);
}

void get_sys_uptime(uint32_t *psec_cnt, uint16_t *pmsec_cnt)
{
    // we do not want to disable the systick interrupt, so we have to deal
    // with a systick during the execution of this function
    uint32_t sec_cnt_1 = sys_seconds_cnt;
    uint16_t msec_cnt_1 = sys_millisec_cnt;
    uint32_t sec_cnt_2 = sys_seconds_cnt;
    uint16_t msec_cnt_2 = sys_millisec_cnt;
    if ((sec_cnt_1 == sec_cnt_2) && (msec_cnt_2 < msec_cnt_1))
    {
        sec_cnt_2 += 1;
    }
    *psec_cnt = sec_cnt_2;
    *pmsec_cnt = msec_cnt_2;
}

void timer_init(void)
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
timer_id timer_creat(void (*pFunction)(void *para),
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

bool timer_delete(const timer_id index)
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

void timer_start(const timer_id index)
{
    if (index >= OS_TIMER_MAX)
        return;

    timer_list[index].run = true;
}

void timer_stop(const timer_id index)
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
inline void timer_sched(void)
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
                    timer_delete(index);
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

/*< for test*/
void f1(void *para)
{
    printf("f1\r\n");
}

void f2(void *para)
{
    printf("f2\r\n");
}

void f3(void *para)
{
    printf("f3\r\n");
}

/*
int main()
{
    printf("Hello world!\n");

    timer_init();

    timer_id t1 = timer_creat(f1, 10, 10, false, NULL);
    timer_id t2 = timer_creat(f2, 10, 10, false, NULL);
    timer_id t3 = timer_creat(f3, 10, 10, false, NULL);

    timer_start(t2);
    timer_start(t3);

    while (1)
    {
        timer_sched();
    }

    return 0;
}
*/
