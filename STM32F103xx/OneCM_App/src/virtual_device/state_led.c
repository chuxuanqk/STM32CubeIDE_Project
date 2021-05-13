/*
 * state_led.c
 *
 *  Created on: Jan 1, 2020
 *      Author: Fox
 */

#include "virtual_device/state_led.h"

#include "stm32f10x_rcc.h"

#define STATE_GPIO GPIOC
#define STATE_PIN (GPIO_Pin_13)
#define NET_GPIO GPIOB
#define NET_PIN GPIO_Pin_13

#define RCC_LED (RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOB)

#define STATE_LED_ON() (GPIO_ResetBits(STATE_GPIO, STATE_PIN))
#define STATE_LED_OFF() (GPIO_SetBits(STATE_GPIO, STATE_PIN))
#define NET_LED_ON() (GPIO_ResetBits(NET_GPIO, NET_PIN))
#define NET_LED_OFF() (GPIO_SetBits(NET_GPIO, NET_PIN))

struct LED_Sharp_Struct State_LED_Str, Net_LED_Str;

/*
*********************************************************************************************************
*	函 数 名: LED_IO_Iint
*	功能说明: 端口初始化
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void LED_IO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_LED, ENABLE);

    GPIO_InitStructure.GPIO_Pin = STATE_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //GPIO_Mode_Out_PP;//推挽输出
    GPIO_Init(STATE_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = NET_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //GPIO_Mode_Out_PP;//推挽输出
    GPIO_Init(NET_GPIO, &GPIO_InitStructure);

    STATE_LED_OFF();
    NET_LED_OFF();
}

/*
*********************************************************************************************************
*	函 数 名: LED_Update_Set_Value
*	功能说明: LED状态更新函数
*	形    参：LED_Sharp_Str:led结构体，一个结构体代表一个led
             led_model     :led闪烁模式。LED_OFF(常关)，LED_ON(常开)，LED_Sharp_Once(闪烁一次)，LED_Sharp_Repeat(持续闪烁)
             set_timer     :闪烁模式运行时间，led常开常关模式下无效
             sharp_timer   :单次闪烁时间
*	返 回 值: 无
*********************************************************************************************************
*/
void LED_Update_Set_Value(struct LED_Sharp_Struct *LED_Sharp_Str, u8 led_model, u16 set_timer, u16 sharp_timer)
{
    u8 update_led_enable = 0;
    set_timer++;
    switch (led_model)
    {
    case LED_OFF:
    {
        sharp_timer = set_timer = 1;
        LED_Sharp_Str->model_after_once = LED_OFF;
        update_led_enable = 1;
        break;
    }
    case LED_ON:
    {
        sharp_timer = set_timer = 2;
        LED_Sharp_Str->model_after_once = LED_ON;
        update_led_enable = 1;
        break;
    }
    case LED_Sharp_Once:
    {
        if (sharp_timer < 2)
            sharp_timer = 2;
        else if (sharp_timer > set_timer)
            sharp_timer = set_timer;

        if (!(LED_Sharp_Str->sharp_model == LED_Sharp_Once)) //????????,???????
        {
            LED_Sharp_Str->model_after_once = LED_Sharp_Str->sharp_model;
            update_led_enable = 1;
        }
        break;
    }
    case LED_Sharp_Repeat:
    {
        if (sharp_timer < 2)
            sharp_timer = 2;
        else if (sharp_timer > set_timer)
            sharp_timer = set_timer;
        update_led_enable = 1;
        break;
    }
    }

    if (update_led_enable)
    {
        LED_Sharp_Str->sharp_model = led_model;
        LED_Sharp_Str->count_timer = LED_Sharp_Str->set_timer = set_timer;
        LED_Sharp_Str->sharp_timer_ms = set_timer - sharp_timer;
    }
}

/*
*********************************************************************************************************
*	函 数 名: LED_Reflash_Output
*	功能说明: LED定时刷新函数（1ms）
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void LED_Reflash_Output(void)
{
    if (State_LED_Str.count_timer)
    {
        State_LED_Str.count_timer--;
        if (State_LED_Str.count_timer == 0)
        {
            switch (State_LED_Str.sharp_model)
            {
            case LED_OFF:
            {
                STATE_LED_OFF();
                break;
            }
            case LED_Sharp_Once:
            {
                LED_Update_Set_Value(&State_LED_Str, State_LED_Str.model_after_once, 1000, 500);
                break;
            }
            case LED_Sharp_Repeat:
            {
                State_LED_Str.count_timer = State_LED_Str.set_timer;
                break;
            }
            case LED_ON:
            {
                STATE_LED_ON();
                break;
            }
            }
        }
        else if (State_LED_Str.count_timer == State_LED_Str.sharp_timer_ms)
        {
            STATE_LED_OFF();
        }
        else if (State_LED_Str.count_timer == (State_LED_Str.set_timer - 1))
        {
            STATE_LED_ON();
        }
    }

    if (Net_LED_Str.count_timer)
    {
        Net_LED_Str.count_timer--;
        if (Net_LED_Str.count_timer == 0)
        {
            switch (Net_LED_Str.sharp_model)
            {
            case LED_OFF:
            {
                NET_LED_OFF();
                break;
            }
            case LED_Sharp_Once:
            {
                LED_Update_Set_Value(&Net_LED_Str, Net_LED_Str.model_after_once, 1000, 500);
                break;
            }
            case LED_Sharp_Repeat:
            {
                Net_LED_Str.count_timer = Net_LED_Str.set_timer;
                break;
            }
            case LED_ON:
            {
                NET_LED_ON();
                break;
            }
            }
        }
        else if (Net_LED_Str.count_timer == Net_LED_Str.sharp_timer_ms)
        {
            NET_LED_OFF();
        }
        else if (Net_LED_Str.count_timer == (Net_LED_Str.set_timer - 1))
        {
            NET_LED_ON();
        }
    }
}
