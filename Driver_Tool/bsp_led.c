/*
 * led.c
 *
 *  Created on: 2020年9月23日
 *      Author: Saber
 */

#include "bsp_led.h"

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#define GPIO_PIN_NET GPIO_Pin_4
#define GPIO_NET GPIOB
#define GPIO_PIN_STATE GPIO_Pin_5
#define GPIO_STATE GPIOB

#define SET_TIME_DEFAULT 1000
#define SHARP_TIMER_DEFAULT 500

static void led_open(struct LED_Sharp_Struct *led_dev);
static void led_close(struct LED_Sharp_Struct *led_dev);

static void led_set_mode(enum LED_TYPE type, enum LED_Sharp_Model mode);
static void led_set_time(enum LED_TYPE type, uint16_t set_time, uint16_t sharp_time);

/************************************************************************/
static void RCC_Configuration(void)
{
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
 * @description: 
 * @param {*}
 * @return {*}
 */
static inline void led_set_mode(enum LED_TYPE type, enum LED_Sharp_Model mode)
{
    struct LED_Sharp_Struct *led_dev = _LED_GetDevice(type);

    led_dev->model = mode;
}

/**
 * @description: 设置LED状态配置
 * @param {*}   type： LED 类型
 *              set_time: 设置LED总的计数值 ms,  递减
 *              sharp_time: 到达闪烁的计数值 ms
 *              eg: set_time=200, sharp_time=150; 即 set_time-- ....--> (set_time == sharptime) LED状态改变
 * @return {*}
 */
static inline void led_set_time(enum LED_TYPE type, uint16_t set_time, uint16_t sharp_time)
{
    struct LED_Sharp_Struct *led_dev = _LED_GetDevice(type);

    led_dev->set_time_ms = set_time;
    led_dev->count_timer = led_dev->set_time_ms;
    led_dev->sharp_timer_ms = sharp_time;
}

static void led_open(struct LED_Sharp_Struct *led_dev)
{
    GPIO_ResetBits(led_dev->gpiox, led_dev->gpio_pin);
}

static void led_close(struct LED_Sharp_Struct *led_dev)
{
    GPIO_SetBits(led_dev->gpiox, led_dev->gpio_pin);
}

/********************************外部调用函数****************************************************/

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

/**
 * @description: led设备初始化
 * @param {*}
 * @return {*}
 */
void hw_led_init(void)
{
    RCC_Configuration();
    GPIO_Configuration();
    led_set_mode(NET_LED, LED_OFF_MODE);
    led_set_mode(STATE_LED, LED_OFF_MODE);
}

/**
 * @description: net指示灯闪烁
 * @param {*}
 * @return {*}
 */
void net_led_sharp(void)
{
    led_set_mode(NET_LED, LED_Sharp_Once_MODE);
    led_set_time(NET_LED, 200, 150);
}

/**
 * @description: state指示灯闪烁
 * @param {*}
 * @return {*}
 */
void state_led_sharp(void)
{
    led_set_mode(STATE_LED, LED_Sharp_Once_MODE);
    led_set_time(STATE_LED, 200, 150);
}

/**
 * @description: for test 
 * @param {type} 
 * @return {type} 
 */
void led_test(void)
{
    // led_set_mode(NET_LED, LED_Sharp_Repeat_MODE);
    // led_set_time(NET_LED, 1000, 500);

    led_set_mode(STATE_LED, LED_Sharp_Repeat_MODE);
    led_set_time(STATE_LED, 1000, 900);
}
