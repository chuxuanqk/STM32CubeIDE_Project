/*
 * state_led.h
 *
 *  Created on: Jan 1, 2020
 *      Author: Fox
 */

#ifndef SRC_VIRTUAL_DEVICE_STATE_LED_H_
#define SRC_VIRTUAL_DEVICE_STATE_LED_H_

#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"

//LED灯控制
struct LED_Sharp_Struct
{
    u8 sharp_model;
    u8 model_after_once;
    u16 sharp_timer_ms;
    u16 count_timer;
    u16 set_timer;
};

enum LED_Sharp_Model
{
    LED_OFF = 0,
    LED_Sharp_Once = 1,
    LED_Sharp_Repeat = 2,
    LED_ON = 3,
    LED_LOCK_OFF = 255
};

extern struct LED_Sharp_Struct State_LED_Str, Net_LED_Str;

void LED_IO_Init(void);
void LED_Update_Set_Value(struct LED_Sharp_Struct *LED_Sharp_Str, u8 led_model, u16 set_timer, u16 sharp_timer);
void LED_Reflash_Output(void);

#define net_led_sharp() (LED_Update_Set_Value(&Net_LED_Str, LED_Sharp_Once, 200, 30))
#define state_led_sharp() (LED_Update_Set_Value(&State_LED_Str, LED_Sharp_Once, 200, 30))
#define state_led_close() (LED_Update_Set_Value(&State_LED_Str, LED_OFF, 1000, 500))
// #define state_led_sharp_repeat() (LED_Update_Set_Value(&State_LED_Str, LED_Sharp_Repeat, 1000, 500))
#define state_led_sharp_repeat() (LED_Update_Set_Value(&State_LED_Str, LED_Sharp_Repeat, 200, 30))

#endif /* SRC_VIRTUAL_DEVICE_STATE_LED_H_ */
