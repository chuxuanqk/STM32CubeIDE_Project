#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define OS_TIMER_MAX 32

struct timer_t
{
    void (*pfun)(void *para);
    unsigned int delay;      // 延时时间
    unsigned int delay_temp; // 缓存延时时间
    unsigned int period;     // 定时器周期
    bool run;
    void *para;
};
struct timer_t timer_list[OS_TIMER_MAX];

typedef uint8_t timer_id;

void timer_init(void)
{
    uint8_t index;

    for (index = 0; index < OS_TIMER_MAX; index++)
    {
        timer_list[index].pfun = NULL;
    }
}

timer_id timer_creat(void (*pFunction)(void *para),
                     const unsigned int delay,
                     const unsigned int period,
                     bool run,
                     void *para)
{
    uint8_t index = 0;

    while ((timer_list[index].pfun != NULL) && (index < OS_TIMER_MAX))
    {
        index++;
    }

    // printf("time index %d\r\n",index);
    timer_list[index].pfun = pFunction;
    timer_list[index].delay = delay;
    timer_list[index].delay_temp = delay;
    timer_list[index].period = period;
    timer_list[index].run = run;
    timer_list[index].para = para;
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
void timer_sched(void)
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
            // timer_list[index].delay -= 1;
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