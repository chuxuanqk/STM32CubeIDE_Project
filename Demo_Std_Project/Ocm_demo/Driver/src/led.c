/*
 * @Author: your name
 * @Date: 2020-09-23 16:05:02
 * @LastEditTime: 2020-11-24 15:06:20
 * @LastEditors: Saber
 * @Description: In User Settings Edit
 * @FilePath: \Ocm_demo\Driver\src\led.c
 */
/*
 * led.c
 *
 *  Created on: 2020年9月23日
 *      Author: Administrator
 */
#include "led.h"
#include "timer.h"

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#define GPIO_PIN_NET GPIO_Pin_13
#define GPIO_PIN_STATE GPIO_Pin_13
#define GPIO_NET GPIOC
#define GPIO_STATE GPIOB

#define GPIO_PIN_BL0940_LED_1 GPIO_Pin_3
#define GPIO_PIN_BL0940_LED_2 GPIO_Pin_2
#define GPIO_PIN_BL0940_LED_3 GPIO_Pin_1
#define GPIO_PIN_BL0940_LED_4 GPIO_Pin_0
#define GPIO_BL0940_LED GPIOA

#define SET_TIME_DEFAULT 1000
#define SHARP_TIMER_DEFAULT 500

#define STATE_LED_ON() (GPIO_ResetBits(GPIO_STATE, GPIO_PIN_STATE))
#define STATE_LED_OFF() (GPIO_SetBits(GPIO_STATE, GPIO_PIN_STATE))

void led_open(struct LED_Sharp_Struct *led_dev);

void led_close(struct LED_Sharp_Struct *led_dev);

/************************************************************************/
static void RCC_Configuration(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
}

static void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    /* Configure USART Rx/tx PIN */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_NET;
    GPIO_Init(GPIO_NET, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_STATE;
    GPIO_Init(GPIO_STATE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_BL0940_LED_1 | GPIO_PIN_BL0940_LED_2 | GPIO_PIN_BL0940_LED_3 | GPIO_PIN_BL0940_LED_4;
    GPIO_Init(GPIO_BL0940_LED, &GPIO_InitStructure);

    STATE_LED_OFF();
    // GPIO_SetBits(GPIO_STATE, GPIO_PIN_STATE);
    GPIO_SetBits(GPIO_NET, GPIO_PIN_NET);

    GPIO_ResetBits(GPIO_BL0940_LED, GPIO_PIN_BL0940_LED_1 | GPIO_PIN_BL0940_LED_2 | GPIO_PIN_BL0940_LED_3 | GPIO_PIN_BL0940_LED_4);
}

/*< 初始化LED设备状态*/
struct LED_Sharp_Struct led_net_dev = {
    .led_type = NET_LED,
    .model = LED_OFF_MODE,
    .gpiox = GPIO_NET,
    .gpio_pin = GPIO_PIN_NET,

    .sharp_timer_ms = 0,
    .set_time_ms = SET_TIME_DEFAULT,
    .count_timer = SHARP_TIMER_DEFAULT,
    .LED_ON = led_open,
    .LED_OFF = led_close,
};

struct LED_Sharp_Struct led_state_dev = {
    .led_type = STATE_LED,
    .model = LED_OFF_MODE,
    .gpiox = GPIO_STATE,
    .gpio_pin = GPIO_PIN_STATE,

    .sharp_timer_ms = 0,
    .set_time_ms = SET_TIME_DEFAULT,
    .count_timer = SHARP_TIMER_DEFAULT,
    .LED_ON = led_open,
    .LED_OFF = led_close,
};

struct LED_Sharp_Struct *_LED_GetDevice(enum LED_TYPE type)
{
    struct LED_Sharp_Struct *led_dev;

    switch (type)
    {
    case NET_LED:
        led_dev = &led_net_dev;
        break;

    case STATE_LED:
        led_dev = &led_state_dev;
        break;

    default:
        break;
    }

    return led_dev;
}

/**
 * @description: led设备状态刷新, 需要注册进ms级定时器
 * @param {type} 
 * @return {type} 
 */
void led_reflash_status(struct LED_Sharp_Struct *led)
{
    if (led->count_timer != 0)
    {
        led->count_timer--;

        if (led->count_timer == 0)
        {
            switch (led->model)
            {
            case LED_ON_MODE:
                led->LED_ON(led);
                break;

            case LED_OFF_MODE:
                led->LED_OFF(led);
                break;

            case LED_Sharp_Once_MODE:
                // led->set_time_ms = 0;
                // led->sharp_timer_ms = 0;
                break;

            case LED_Sharp_Repeat_MODE:
                led->count_timer = led->set_time_ms;
                break;

            default:
                break;
            }
        }
        else if (led->count_timer == (led->set_time_ms - 1))
        {
            led->LED_ON(led);
        }
        else if (led->count_timer == led->sharp_timer_ms)
        {
            led->LED_OFF(led);
        }
    }
}

inline void led_set_mode(enum LED_TYPE type, enum LED_Sharp_Model mode)
{
    struct LED_Sharp_Struct *led_dev = _LED_GetDevice(type);

    led_dev->model = mode;
}

inline void led_set_time(enum LED_TYPE type, uint16_t set_time, uint16_t sharp_time)
{
    struct LED_Sharp_Struct *led_dev = _LED_GetDevice(type);

    led_dev->set_time_ms = set_time;
    led_dev->sharp_timer_ms = sharp_time;
}

void led_open(struct LED_Sharp_Struct *led_dev)
{
    GPIO_SetBits(led_dev->gpiox, led_dev->gpio_pin);
}

void led_close(struct LED_Sharp_Struct *led_dev)
{
    GPIO_ResetBits(led_dev->gpiox, led_dev->gpio_pin);
}

uint32_t pwm_cnt = 0;
uint32_t max_cnt = 20;
uint32_t min_cnt = 2;
uint32_t mid_cnt = 4;
uint32_t cnt_cnt = 0;

/**
 * @description: PWM计数
 * @param {*}
 * @return {*}
 */
void pwm_led_count(void)
{
    pwm_cnt++;

    return;
}

void hw_led_init(void)
{
    RCC_Configuration();
    GPIO_Configuration();

    /* 注册LED设备到定时器 */
    // timer_creat(led_reflash_status, 10, 0, true, &led_state_dev);
    // timer_creat(led_reflash_status, 10, 0, true, &led_net_dev);

    timer_creat((void *)pwm_led_count, 1, 0, true, NULL);
}

/**
 * @description: 
 * @param {*}
 * @return {*}
 */
void pwm_led_test(void)
{
    if (pwm_cnt == min_cnt)
    {
        STATE_LED_OFF();
    }
    else if (pwm_cnt == mid_cnt)
    {
        STATE_LED_ON();
    }
    else if (pwm_cnt >= max_cnt)
    {
        cnt_cnt++;
        pwm_cnt = 0;

        if (cnt_cnt > 10)
        {
            cnt_cnt = 0;
            mid_cnt += 2;
            if (mid_cnt > max_cnt)
                mid_cnt = 4;
        }
    }

    return;
}

/**
 * @description: for test 
 * @param {type} 
 * @return {type} 
 */
void led_test(void)
{
    // led_set_mode(STATE_LED, LED_Sharp_Repeat_MODE);
    // led_set_time(STATE_LED, 2000, 1000);

    led_set_mode(NET_LED, LED_Sharp_Repeat_MODE);
    led_set_time(NET_LED, 1000, 500);
}
