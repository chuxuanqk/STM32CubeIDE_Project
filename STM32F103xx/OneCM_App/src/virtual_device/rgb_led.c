/*
*********************************************************************************************************
*	                                  
*	模块名称 : rgbled驱动模块    
*	文件名称 : led.c
*	版    本 : V1.1
*	说    明 : 
*
*	Copyright (C), 
*
*********************************************************************************************************
*/
#include "virtual_device/rgb_led.h"

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#define LED_R_ON (GPIO_ResetBits(GPIOA, GPIO_Pin_1))
#define LED_R_OFF (GPIO_SetBits(GPIOA, GPIO_Pin_1))
#define LED_G_ON (GPIO_ResetBits(GPIOA, GPIO_Pin_2))
#define LED_G_OFF (GPIO_SetBits(GPIOA, GPIO_Pin_2))
#define LED_B_ON (GPIO_ResetBits(GPIOA, GPIO_Pin_3))
#define LED_B_OFF (GPIO_SetBits(GPIOA, GPIO_Pin_3))

struct RGBLED_Sharp_Struct LED_R_Str, LED_G_Str, LED_B_Str;
enum RGB_COLOR state_after_sharp = off; //闪烁之后的状态1：打开，0关闭

/*
*********************************************************************************************************
*	函 数 名: LED_IO_Iint
*	功能说明: 端口初始化
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void RGBLED_IO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // R:PC13 G:PC14 B:PC15
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_1);
    GPIO_SetBits(GPIOA, GPIO_Pin_2);
    GPIO_SetBits(GPIOA, GPIO_Pin_3);
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
void RGBLED_Update_Set_Value(struct RGBLED_Sharp_Struct *LED_Sharp_Str, u8 led_model, u16 set_timer, u16 sharp_timer)
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

    default:
        break;
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
*	函 数 名: RGBLED_Update_Set_Color
*	功能说明: LED状态更新函数
*	形    参：color         :颜色，枚举类型，目前支持红、绿、蓝、白四种颜色
             led_model     :led闪烁模式。LED_OFF(常关)，LED_ON(常开)，LED_Sharp_Once(闪烁一次)，LED_Sharp_Repeat(持续闪烁)
             set_timer     :闪烁模式运行时间，led常开常关模式下无效
             sharp_timer   :单次闪烁时间
             color_after_sharp:闪烁结束之后显示的颜色,只在LED_Sharp_Once模式下有效
*	返 回 值: 无
*********************************************************************************************************
*/
void RGBLED_Update_Set_Color(enum RGB_COLOR color, u8 led_model, u16 set_timer, u16 sharp_timer, enum RGB_COLOR color_after_sharp)
{
    state_after_sharp = color_after_sharp;

    switch (color)
    {
    case red:
        RGBLED_Update_Set_Value(&LED_R_Str, led_model, set_timer, sharp_timer);
        RGBLED_Update_Set_Value(&LED_G_Str, LED_OFF, set_timer, sharp_timer);
        RGBLED_Update_Set_Value(&LED_B_Str, LED_OFF, set_timer, sharp_timer);
        break;
    case green:
        RGBLED_Update_Set_Value(&LED_R_Str, LED_OFF, set_timer, sharp_timer);
        RGBLED_Update_Set_Value(&LED_G_Str, led_model, set_timer, sharp_timer);
        RGBLED_Update_Set_Value(&LED_B_Str, LED_OFF, set_timer, sharp_timer);
        break;
    case blue:
        RGBLED_Update_Set_Value(&LED_R_Str, LED_OFF, set_timer, sharp_timer);
        RGBLED_Update_Set_Value(&LED_G_Str, LED_OFF, set_timer, sharp_timer);
        RGBLED_Update_Set_Value(&LED_B_Str, led_model, set_timer, sharp_timer);
        break;
    case white:
        RGBLED_Update_Set_Value(&LED_R_Str, led_model, set_timer, sharp_timer);
        RGBLED_Update_Set_Value(&LED_G_Str, led_model, set_timer, sharp_timer);
        RGBLED_Update_Set_Value(&LED_B_Str, led_model, set_timer, sharp_timer);
        break;
    case off:
        RGBLED_Update_Set_Value(&LED_R_Str, LED_OFF, set_timer, sharp_timer);
        RGBLED_Update_Set_Value(&LED_G_Str, LED_OFF, set_timer, sharp_timer);
        RGBLED_Update_Set_Value(&LED_B_Str, LED_OFF, set_timer, sharp_timer);
        break;

    default:
        break;
    }
}

/*
*********************************************************************************************************
*	函 数 名: RGBLED_Update_Set_Singe_led
*	功能说明: RGB设置单色开关
*	形    参：color         :颜色，枚举类型，目前支持红、绿、蓝、白四种颜色
             led_model     :led闪烁模式。LED_OFF(常关)，LED_ON(常开)，LED_Sharp_Once(闪烁一次)，LED_Sharp_Repeat(持续闪烁)
             set_timer     :闪烁模式运行时间，led常开常关模式下无效
             sharp_timer   :单次闪烁时间
             color_after_sharp:闪烁结束之后显示的颜色,只在LED_Sharp_Once模式下有效
*	返 回 值: 无
*********************************************************************************************************
*/
void RGBLED_Update_Set_Singe_led(enum RGB_COLOR color, u8 led_model, u16 set_timer, u16 sharp_timer, enum RGB_COLOR color_after_sharp)
{
    state_after_sharp = color_after_sharp;
    switch (color)
    {
    case red:
        RGBLED_Update_Set_Value(&LED_R_Str, led_model, set_timer, sharp_timer);
        break;
    case green:
        RGBLED_Update_Set_Value(&LED_G_Str, led_model, set_timer, sharp_timer);
        break;
    case blue:
        RGBLED_Update_Set_Value(&LED_B_Str, led_model, set_timer, sharp_timer);
        break;
    case white:
        RGBLED_Update_Set_Value(&LED_R_Str, led_model, set_timer, sharp_timer);
        RGBLED_Update_Set_Value(&LED_G_Str, led_model, set_timer, sharp_timer);
        RGBLED_Update_Set_Value(&LED_B_Str, led_model, set_timer, sharp_timer);
        break;

    default:
        break;
    }

    return;
}

/*
*********************************************************************************************************
*	函 数 名: LED_Reflash_Output
*	功能说明: LED定时刷新函数（1ms）
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void RGBLED_Reflash_Output(void)
{
    if (LED_R_Str.count_timer)
    {
        LED_R_Str.count_timer--;
        if (LED_R_Str.count_timer == 0)
        {
            switch (LED_R_Str.sharp_model)
            {
            case LED_OFF:
            {
                LED_R_OFF;
                break;
            }
            case LED_Sharp_Once:
            {
                RGBLED_Update_Set_Color(state_after_sharp, LED_ON, 1000, 500, off);
                // RGBLED_Update_Set_Value(&LED_R_Str, LED_R_Str.model_after_once, 1000, 500);
                break;
            }
            case LED_Sharp_Repeat:
            {
                LED_R_Str.count_timer = LED_R_Str.set_timer;
                break;
            }
            case LED_ON:
            {
                LED_R_ON;
                break;
            }
            }
        }
        else if (LED_R_Str.count_timer == LED_R_Str.sharp_timer_ms)
        {
            // if (state_after_sharp & 0x01)
            //     LED_R_ON;
            // else
            LED_R_OFF;
        }
        else if (LED_R_Str.count_timer == (LED_R_Str.set_timer - 1))
        {
            LED_R_ON;
        }
    }

    if (LED_G_Str.count_timer)
    {
        LED_G_Str.count_timer--;
        if (LED_G_Str.count_timer == 0)
        {
            switch (LED_G_Str.sharp_model)
            {
            case LED_OFF:
            {
                LED_G_OFF;
                break;
            }
            case LED_Sharp_Once:
            {
                RGBLED_Update_Set_Color(state_after_sharp, LED_ON, 1000, 500, off);
                // RGBLED_Update_Set_Value(&LED_G_Str, LED_G_Str.model_after_once, 1000, 500);
                break;
            }
            case LED_Sharp_Repeat:
            {
                LED_G_Str.count_timer = LED_G_Str.set_timer;
                break;
            }
            case LED_ON:
            {
                LED_G_ON;
                break;
            }
            }
        }
        else if (LED_G_Str.count_timer == LED_G_Str.sharp_timer_ms)
        {
            // if (state_after_sharp & 0x02)
            //     LED_G_ON;
            // else
            LED_G_OFF;
        }
        else if (LED_G_Str.count_timer == (LED_G_Str.set_timer - 1))
        {
            LED_G_ON;
        }
    }

    if (LED_B_Str.count_timer)
    {
        LED_B_Str.count_timer--;
        if (LED_B_Str.count_timer == 0)
        {
            switch (LED_B_Str.sharp_model)
            {
            case LED_OFF:
            {
                LED_B_OFF;
                break;
            }
            case LED_Sharp_Once:
            {
                RGBLED_Update_Set_Color(state_after_sharp, LED_ON, 1000, 500, off);
                // RGBLED_Update_Set_Value(&LED_B_Str, LED_B_Str.model_after_once, 1000, 500);
                break;
            }
            case LED_Sharp_Repeat:
            {
                LED_B_Str.count_timer = LED_B_Str.set_timer;
                break;
            }
            case LED_ON:
            {
                LED_B_ON;
                break;
            }
            }
        }
        else if (LED_B_Str.count_timer == LED_B_Str.sharp_timer_ms)
        {
            LED_B_OFF;
        }
        else if (LED_B_Str.count_timer == (LED_B_Str.set_timer - 1))
        {

            LED_B_ON;
        }
    }
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
