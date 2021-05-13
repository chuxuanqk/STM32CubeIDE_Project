#ifndef PWM_CONTROL_H
#define PWM_CONTROL_H

#include "stm32f10x.h"

#define PWM_Period (65535 / 25) //默认，PWM周期设置，其他参数已设置，基本频率1kHz,25kHz避免电磁噪音
#define PWM_DIV_Timeval 50		//默认 ，最小值变化分度等于 100/PWM_Period

#define PWM_Speed_Present_Open 0   //继电器打开百分比
#define PWM_Speed_Present_Close 0  //继电器关闭动作速度百分比，0为立即关闭
#define PWM_Speed_Present_Base 500 //用于实现互斥的PWM变化速率，同时实现互斥延时

//PWM脉冲输出模式
enum PWM_Sharp_Model
{
	PWM_OFF = 0,
	PWM_Sharp_Once = 1,
	PWM_Sharp_Repeat = 2,
	PWM_ON = 3,
	PWM_LOCK_OFF = 255
};

enum PWM_Output_Button_State
{
	Button_Var = 0,
	Button_Add = 1,
	Button_Add_End = 2,
	Button_Dec = 3,
	Button_Dec_End = 4
};

struct PWM_Output_Buffer_Struct
{
	u16 set_value;		  //PWM目标值
	u16 now_value;		  //PWM当前值
	u16 ex_value;		  //过去状态值
	u32 set_hold_time_ms; //PWM值保持时间，设置0无效，保持到该时间后恢复原来状态
	u32 timer_ms;		  //定时器值
	u32 div_counter;	  //PWM累加次数，根据目标值，当前值及增量值计算
	u16 delta_value;	  //PWM单次增量
	u8 reinit_pwm_pin;
	u8 sharp_model;	 //脉冲模式
	u8 set_present;	 //保存的变化速率百分比
	u8 lock_change;	 //参数修改锁，用于避免定时器与修改之间的冲突
	u8 updown_flag;	 //按键增大减少状态量
	u32 pwm_period;	 //PWM载波周期
	u16 div_timeval; //最小值等于 100/PWM_Period
};

void Init_TIMx_24_PWM(u16 pwm_pluse);	  //PWM初始化函数
void PWM_Reflash_Output_Check_Lock(void); //PWM动态变化处理函数,增加组合通道互斥功能
//根据目标值，当前值和每秒增量百分比计算增量次数及增量值
//变化速率扩展到1s的百分之65535,仅针对继电器等需要快速变化输出场合
void PWM_Update_Set_Value_With_Pluse(struct PWM_Output_Buffer_Struct *PWM_Output_Buffer_Str, u16 set_value, u16 ex_value, u16 delta_precent, u32 set_hold_time_ms, enum PWM_Sharp_Model sharp_model);
void PWM_Output_Buf_Clear(struct PWM_Output_Buffer_Struct *PWM_Output_Buffer_Str);
struct PWM_Output_Buffer_Struct *Get_PWM_Output_Str(uint8_t temp_port);

#endif
