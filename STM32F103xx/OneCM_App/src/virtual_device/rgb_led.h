
#ifndef RGB_LED_H
#define RGB_LED_H
#include "stm32f10x_tim.h"

//LED灯控制
struct RGBLED_Sharp_Struct
{
    u8 sharp_model;
    u8 model_after_once;
    u16 sharp_timer_ms;
    u16 count_timer;
    u16 set_timer;
};

enum RGBLED_Sharp_Model
{
    LED_OFF = 0,
    LED_Sharp_Once = 1,
    LED_Sharp_Repeat = 2,
    LED_ON = 3,
    LED_LOCK_OFF = 255
};

enum RGB_COLOR
{
    off = 0,
    red,
    green,
    blue,
    white
};

extern struct RGBLED_Sharp_Struct LED_R_Str, LED_G_Str, LED_B_Str;

void RGBLED_IO_Iint(void);
void RGBLED_Update_Set_Value(struct RGBLED_Sharp_Struct *LED_Sharp_Str, u8 led_model, u16 set_timer, u16 sharp_timer);
void RGBLED_Update_Set_Color(enum RGB_COLOR color, u8 led_model, u16 set_timer, u16 sharp_timer, enum RGB_COLOR color_after_sharp);
void RGBLED_Update_Set_Singe_led(enum RGB_COLOR color, u8 led_model, u16 set_timer, u16 sharp_timer, enum RGB_COLOR color_after_sharp);
void RGBLED_Reflash_Output(void);

#define RGBLED_Normal_Mode() RGBLED_Update_Set_Color(green, LED_ON, 1000, 500, green)        // 正常模式，常亮绿灯
#define RGBLED_CAN_Net() RGBLED_Update_Set_Color(blue, LED_Sharp_Once, 200, 75, green)       // 正常模式，设备与主机CAN通信闪烁蓝灯
#define RGBLED_Cfg_Mode() RGBLED_Update_Set_Color(white, LED_ON, 1000, 500, white)           // 配置模式，常亮白灯
#define RGBLED_Cfg_Add_Dev() RGBLED_Update_Set_Color(blue, LED_Sharp_Once, 200, 75, white)   // 配置模式添加设备，快闪蓝灯后常亮白灯
#define RGBLDE_Cfg_Clean() RGBLED_Update_Set_Color(blue, LED_Sharp_Once, 4000, 3000, white); // 配置模式清除数据，蓝灯长亮3s后常亮白灯

#endif
