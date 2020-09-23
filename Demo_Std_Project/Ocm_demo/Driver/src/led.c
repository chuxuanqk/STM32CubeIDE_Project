/*
 * @Author: your name
 * @Date: 2020-09-23 16:05:02
 * @LastEditTime: 2020-09-23 18:06:08
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \Demo_Std_Project\Ocm_demo\Driver\src\led.c
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
#define GPIO_PIN_STATE GPIO_Pin_14
#define GPIO_NET GPIOC
#define GPIO_STATE GPIOC

#define GPIO_PIN_BL0940_LED_1 GPIO_Pin_3
#define GPIO_PIN_BL0940_LED_2 GPIO_Pin_2
#define GPIO_PIN_BL0940_LED_3 GPIO_Pin_1
#define GPIO_PIN_BL0940_LED_4 GPIO_Pin_0
#define GPIO_BL0940_LED GPIOA

#define SET_TIME_DEFAULT 1000
#define SHARP_TIMER_DEFAULT 500

/*< 初始化LED设备状态*/
struct LED_Sharp_Struct led_net_dev = {
    .led_type = NET_LED,
    .model = LED_OFF_MODE,
    .sharp_timer_ms = 0,
    .set_time_ms = SET_TIME_DEFAULT,
    .count_timer = SHARP_TIMER_DEFAULT,
    .LED_ON = GPIO_SetBits(GPIO_NET, GPIO_PIN_NET),
    .LED_OFF = GPIO_ResetBits(GPIO_NET, GPIO_PIN_NET),
};

struct LED_Sharp_Struct led_state_dev = {
    .led_type = STATE_LED,
    .model = LED_OFF_MODE,
    .sharp_timer_ms = 0,
    .set_time_ms = SET_TIME_DEFAULT,
    .count_timer = SHARP_TIMER_DEFAULT,
    .LED_ON = GPIO_SetBits(GPIO_STATE, GPIO_PIN_STATE),
    .LED_OFF = GPIO_ResetBits(GPIO_STATE, GPIO_PIN_STATE),
};

/************************************************************************/
static void RCC_Configuration(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
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

    GPIO_ResetBits(GPIO_NET, GPIO_PIN_NET);
    GPIO_ResetBits(GPIO_STATE, GPIO_PIN_STATE);
    GPIO_ResetBits(GPIO_BL0940_LED, GPIO_PIN_BL0940_LED_1 | GPIO_PIN_BL0940_LED_2 | GPIO_PIN_BL0940_LED_3 | GPIO_PIN_BL0940_LED_4);
}

inline struct LED_Sharp_Struct *__LED_GetDevice(enum LED_TYPE type)
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
                led->LED_ON;
                break;

            case LED_OFF_MODE:
                led->LED_OFF;
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
            led->LED_ON;
        }
        else if (led->count_timer == led->sharp_timer_ms)
        {
            led->LED_OFF;
        }
    }
}

inline void led_set_mode(enum LED_TYPE type, enum LED_Sharp_Model mode)
{
    struct LED_Sharp_Struct *led_dev = __LED_GetDevice(type);

    led_dev->model = mode;
}

inline void led_set_time(enum LED_TYPE type, uint16_t set_time, uint16_t sharp_time)
{
    struct LED_Sharp_Struct *led_dev = __LED_GetDevice(type);

    led_dev->set_time_ms = set_time;
    led_dev->sharp_timer_ms = sharp_time;
}

void hw_led_init(void)
{
    RCC_Configuration();
    GPIO_Configuration();

    /* 注册LED设备到定时器 */
    timer_creat(&led_reflash_status, 10, 0, true, &led_net_dev);
    timer_creat(&led_reflash_status, 10, 0, true, &led_state_dev);
}