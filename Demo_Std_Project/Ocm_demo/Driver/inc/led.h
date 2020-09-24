/*
 * @Author: your name
 * @Date: 2020-09-23 16:04:41
 * @LastEditTime: 2020-09-24 10:52:45
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \Demo_Std_Project\Ocm_demo\Driver\inc\led.h
 */
/*
 * led.h
 *
 *  Created on: 2020年9月23日
 *      Author: Administrator
 */

#ifndef LED_H_
#define LED_H_
#include "stm32f10x.h"

enum LED_Sharp_Model
{
    LED_ON_MODE = 1,
    LED_OFF_MODE,
    LED_Sharp_Once_MODE,
    LED_Sharp_Repeat_MODE,

    LED_LOCK_OFF_MODE = 255
};

/*< 定义需要控制的LED类型 */
enum LED_TYPE
{
    NET_LED = 1,
    STATE_LED,
    BL0940_LED_1,
    BL0940_LED_2,
    BL0940_LED_3,
    BL0940_LED_4,
};

//LED灯控制
struct LED_Sharp_Struct
{
    enum LED_TYPE led_type;     // LED 类型
    enum LED_Sharp_Model model; // LED 模式
    GPIO_TypeDef *gpiox;
    uint16_t gpio_pin;

    uint16_t sharp_timer_ms; // 闪烁间隔时间
    uint16_t set_time_ms;    // 设置完整计数时间
    uint16_t count_timer;    // 闪烁间隔时间计数器

    void (*LED_ON)(struct LED_Sharp_Struct *led_dev);
    void (*LED_OFF)(struct LED_Sharp_Struct *led_dev);
};

void hw_led_init(void);
void led_reflash_status(struct LED_Sharp_Struct *led);
void led_set_mode(enum LED_TYPE type, enum LED_Sharp_Model mode);
void led_set_time(enum LED_TYPE type, uint16_t set_time, uint16_t sharp_time);

void led_test(void);

#endif /* LED_H_ */
