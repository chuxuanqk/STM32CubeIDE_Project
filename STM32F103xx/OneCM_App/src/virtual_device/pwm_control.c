

#include "config_device/config_par.h"
#include "virtual_device/pwm_control.h"
#include "virtual_device/state_led.h"

#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include <string.h>

// 最大PWM通道数
#define PWM_PORT_NUM 8
#define PWM_Port_Number (PWM_PORT_NUM + 1)

struct PWM_Output_Buffer_Struct PWM_Output_Buffer_Str[PWM_Port_Number];

//pwm通道对应表
static uint16_t *const pwm_mapping[PWM_Port_Number] = {
    0,                       //not use
    (uint16_t *)&TIM3->CCR1, // PA6
    (uint16_t *)&TIM3->CCR2, // PA7
    (uint16_t *)&TIM3->CCR3, // PB0
    (uint16_t *)&TIM3->CCR4, // PB1
    (uint16_t *)&TIM4->CCR1, // PB6
    (uint16_t *)&TIM4->CCR2, // PB7
    (uint16_t *)&TIM4->CCR3, // PB8
    (uint16_t *)&TIM4->CCR4, // PB9
};

/*
*********************************************************************************************************
*	函 数 名: Init_TIMx_24_PWM
*	功能说明: PWM初始化
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Init_TIMx_24_PWM(u16 pwm_pluse)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //设置为复用推免输出

    TIM_TimeBaseStructure.TIM_Period = PWM_Period;                // 自动重装载寄存器的值
    TIM_TimeBaseStructure.TIM_Prescaler = 1 - 1;                  // 时钟预分频数,PWM载波周期>200Hz
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;       //采样分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down; //向下计数
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;              //重复寄存器，用于自动更新pwm占空比

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;             //设置为pwm1输出模式
    TIM_OCInitStructure.TIM_Pulse = pwm_pluse;                    //设置占空比时间
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCNPolarity_High;    //设置输出极性
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //使能TIMx_CHx输出

    /*****普通定时器4***********************************************************************************************************************/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //设置引脚时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);  //设置TIM4时钟

    /********完全重映射TIM4引脚*********/
    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); //重映射必须要开AFIO时钟
    // GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE);

    /********TIM4_CHx 引脚配置*********/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //第三步，定时器基本配置
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    //第四步pwm输出配置
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);          //按照指定参数初始化
    TIM_OC2Init(TIM4, &TIM_OCInitStructure);          //按照指定参数初始
    TIM_OC3Init(TIM4, &TIM_OCInitStructure);          //按照指定参数初始化
    TIM_OC4Init(TIM4, &TIM_OCInitStructure);          //按照指定参数初始化
                                                      //第六步，使能端的打开
    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable); //使能TIMx在CCR1上的预装载寄存器
    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable); //使能TIMx在CCR2上的预装载寄存器
    TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable); //使能TIMx在CCR3上的预装载寄存器
    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable); //使能TIMx在CCR4上的预装载寄存器

    TIM_ARRPreloadConfig(TIM4, ENABLE); //使能TIMx在ARR上的预装载寄存器
    TIM_Cmd(TIM4, ENABLE);              //打开TIM3
    //下面这句是高级定时器才有的，输出pwm必须打开
    //TIM_CtrlPWMOutputs(TIM4, ENABLE); //pwm输出使能，一定要记得打开

    /******普通定时器3**********************************************************************************************************************/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //设置引脚时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  //设置TIM3时钟

    /********TIM3_CHx 引脚配置*********/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //第三步，定时器基本配置
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    //第四步pwm输出配置
    TIM_OC1Init(TIM3, &TIM_OCInitStructure); //按照指定参数初始化
    TIM_OC2Init(TIM3, &TIM_OCInitStructure); //按照指定参数初始
    TIM_OC3Init(TIM3, &TIM_OCInitStructure); //按照指定参数初始化
    TIM_OC4Init(TIM3, &TIM_OCInitStructure); //按照指定参数初始化

    //第六步，使能端的打开
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable); //使能TIMx在CCR1上的预装载寄存器
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable); //使能TIMx在CCR2上的预装载寄存器
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable); //使能TIMx在CCR3上的预装载寄存器
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable); //使能TIMx在CCR4上的预装载寄存器

    TIM_ARRPreloadConfig(TIM3, ENABLE); //使能TIMx在ARR上的预装载寄存器
    TIM_Cmd(TIM3, ENABLE);              //打开TIM3
    //下面这句是高级定时器才有的，输出pwm必须打开
    //TIM_CtrlPWMOutputs(TIM3, ENABLE); //pwm输出使能，一定要记得打开
}

/*
*********************************************************************************************************
*	函 数 名: PWM_Update_Set_Value_With_Pluse
*	功能说明: PWM输出效果变量初始化函数
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void PWM_Update_Set_Value_With_Pluse(struct PWM_Output_Buffer_Struct *PWM_Output_Buffer_Str, u16 set_value,
                                     u16 ex_value, u16 delta_precent, u32 set_hold_time_ms, enum PWM_Sharp_Model sharp_model)
{
    //清除上一次的PWM状态
    //PWM_Output_Buf_Clear(PWM_Output_Buffer_Str);

    if (PWM_Output_Buffer_Str->lock_change) //通道被上锁,主要用于过流锁定继电器无法控制，不建议采用这种写法,2020/11/26,Saber
        return;

    PWM_Output_Buffer_Str->lock_change = 1; //上锁

    PWM_Output_Buffer_Str->set_value = set_value;
    PWM_Output_Buffer_Str->ex_value = ex_value;
    PWM_Output_Buffer_Str->set_present = delta_precent;
    PWM_Output_Buffer_Str->set_hold_time_ms = set_hold_time_ms;
    PWM_Output_Buffer_Str->sharp_model = sharp_model;

    PWM_Output_Buffer_Str->pwm_period = (u32)PWM_Period;
    PWM_Output_Buffer_Str->div_timeval = (u16)PWM_DIV_Timeval;

    if (delta_precent > 0) //根据输入每秒变化百分比计算每一份模拟量变化量
        PWM_Output_Buffer_Str->delta_value = (u16)(PWM_Output_Buffer_Str->pwm_period * PWM_Output_Buffer_Str->div_timeval / 1000 / 100 * delta_precent);
    else
        PWM_Output_Buffer_Str->delta_value = PWM_Output_Buffer_Str->pwm_period; //直接等于变化差值

    switch (sharp_model)
    {
    case PWM_Sharp_Once:
    {
        break;
    }
    case PWM_Sharp_Repeat:
    {
        if (set_value > ex_value)
            PWM_Output_Buffer_Str->now_value = ex_value;
        break;
    }
    }

    if (PWM_Output_Buffer_Str->set_value > PWM_Output_Buffer_Str->now_value) //模拟量向上增长，计算定时器 PWM_DIV_Timeval的份数
    {
        PWM_Output_Buffer_Str->div_counter = ((PWM_Output_Buffer_Str->set_value - PWM_Output_Buffer_Str->now_value) / PWM_Output_Buffer_Str->delta_value +
                                              (((PWM_Output_Buffer_Str->set_value - PWM_Output_Buffer_Str->now_value) % PWM_Output_Buffer_Str->delta_value) ? 1 : 0));
    }
    else if (PWM_Output_Buffer_Str->set_value == PWM_Output_Buffer_Str->now_value) //模拟量向下减少
        PWM_Output_Buffer_Str->div_counter = 1;
    else
    {
        PWM_Output_Buffer_Str->div_counter = ((PWM_Output_Buffer_Str->now_value - PWM_Output_Buffer_Str->set_value) / PWM_Output_Buffer_Str->delta_value +
                                              (((PWM_Output_Buffer_Str->now_value - PWM_Output_Buffer_Str->set_value) % PWM_Output_Buffer_Str->delta_value) ? 1 : 0));
        //PWM_Output_Buffer_Str->delta_value=0-PWM_Output_Buffer_Str->delta_value;
    }

    PWM_Output_Buffer_Str->timer_ms = PWM_Output_Buffer_Str->div_timeval; //启动定时器

    PWM_Output_Buffer_Str->lock_change = 0; //解锁
}

/*
*********************************************************************************************************
*	函 数 名: PWM_Output_Buf_Clear
*	功能说明: 清除PWM输出结构体
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void PWM_Output_Buf_Clear(struct PWM_Output_Buffer_Struct *PWM_Output_Buffer_Str)
{
    memset(PWM_Output_Buffer_Str, 0, sizeof(struct PWM_Output_Buffer_Struct));
}

/*
*********************************************************************************************************
*	函 数 名: PWM_Reflash_Output_Check_Lock
*	功能说明: PWM动态输出函数， 1ms定时查询
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void PWM_Reflash_Output_Check_Lock(void)
{
    u8 i;

    //产生亮度动态变化
    for (i = 1; i < PWM_Port_Number; i++) //检查调光通道所有定时器
    {
        if (PWM_Output_Buffer_Str[i].lock_change == 0) //主函数没有上锁
        {
            if (PWM_Output_Buffer_Str[i].timer_ms > 0) //定时器启动，开始变化
            {
                PWM_Output_Buffer_Str[i].timer_ms--;
                if (PWM_Output_Buffer_Str[i].timer_ms == 0) //通道变化时间间隔满足
                {
                    //启动延时脉冲功能
                    if ((PWM_Output_Buffer_Str[i].now_value == PWM_Output_Buffer_Str[i].set_value) && (PWM_Output_Buffer_Str[i].set_hold_time_ms > 0))
                    {
                        PWM_Output_Buffer_Str[i].div_counter--;       //间隔时间份数减一
                        if (PWM_Output_Buffer_Str[i].div_counter > 0) //修改通道输出
                        {
                            PWM_Output_Buffer_Str[i].timer_ms = PWM_Output_Buffer_Str[i].div_timeval;
                        }
                        else //再次设定目标值
                        {
                            switch (PWM_Output_Buffer_Str[i].sharp_model)
                            {
                            case PWM_OFF:
                            case PWM_ON:
                            {
                                PWM_Output_Buffer_Str[i].set_hold_time_ms = 0;
                                break;
                            }
                            case PWM_Sharp_Once:
                            {
                                PWM_Update_Set_Value_With_Pluse(&PWM_Output_Buffer_Str[i], PWM_Output_Buffer_Str[i].ex_value, PWM_Output_Buffer_Str[i].set_value, PWM_Output_Buffer_Str[i].set_present, 0, PWM_ON);
                                break;
                            }
                            case PWM_Sharp_Repeat:
                            {
                                PWM_Update_Set_Value_With_Pluse(&PWM_Output_Buffer_Str[i], PWM_Output_Buffer_Str[i].ex_value,
                                                                PWM_Output_Buffer_Str[i].set_value, PWM_Output_Buffer_Str[i].set_present,
                                                                PWM_Output_Buffer_Str[i].set_hold_time_ms, PWM_Sharp_Repeat);
                                break;
                            }
                            }
                        }
                    }
                    else
                    {
                        PWM_Output_Buffer_Str[i].div_counter--;       //间隔时间份数减一
                        if (PWM_Output_Buffer_Str[i].div_counter > 0) //修改通道输出
                        {
                            if (PWM_Output_Buffer_Str[i].set_value >= PWM_Output_Buffer_Str[i].now_value)
                                PWM_Output_Buffer_Str[i].now_value += PWM_Output_Buffer_Str[i].delta_value;
                            else
                                PWM_Output_Buffer_Str[i].now_value -= PWM_Output_Buffer_Str[i].delta_value;

                            PWM_Output_Buffer_Str[i].timer_ms = PWM_Output_Buffer_Str[i].div_timeval;
                        }
                        else
                        {
                            PWM_Output_Buffer_Str[i].now_value = PWM_Output_Buffer_Str[i].set_value; //最后一次变化，直接变化到目标值，避免除整误差

                            if (PWM_Output_Buffer_Str[i].set_hold_time_ms > 0)
                            {
                                PWM_Output_Buffer_Str[i].div_counter = PWM_Output_Buffer_Str[i].set_hold_time_ms / PWM_Output_Buffer_Str->div_timeval;

                                if (PWM_Output_Buffer_Str[i].div_counter > 0)
                                    PWM_Output_Buffer_Str[i].timer_ms = PWM_Output_Buffer_Str[i].div_timeval; //复位定时器
                            }
                            else
                            {
                                switch (PWM_Output_Buffer_Str[i].sharp_model)
                                {
                                case PWM_OFF:
                                case PWM_ON:
                                {
                                    PWM_Output_Buffer_Str[i].set_hold_time_ms = 0;
                                    break;
                                }
                                case PWM_Sharp_Once:
                                {
                                    // set_hold_time_ms为0，默认一直开启
                                    PWM_Update_Set_Value_With_Pluse(&PWM_Output_Buffer_Str[i], PWM_Output_Buffer_Str[i].ex_value,
                                                                    PWM_Output_Buffer_Str[i].set_value, PWM_Output_Buffer_Str[i].set_present,
                                                                    0, PWM_ON);
                                    break;
                                }
                                case PWM_Sharp_Repeat:
                                {
                                    PWM_Update_Set_Value_With_Pluse(&PWM_Output_Buffer_Str[i], PWM_Output_Buffer_Str[i].ex_value,
                                                                    PWM_Output_Buffer_Str[i].set_value, PWM_Output_Buffer_Str[i].set_present,
                                                                    PWM_Output_Buffer_Str[i].set_hold_time_ms, PWM_Sharp_Repeat);
                                    break;
                                }
                                }
                            }
                        }

                        //刷新PWM输出，映射到实际的管脚
                        *pwm_mapping[i] = PWM_Output_Buffer_Str[i].now_value;
                    }
                }
            }
        }
    }
}

/**
 * @description: 
 * @param {*} tmp_port: 0~4
 * @return {*}
 */
struct PWM_Output_Buffer_Struct *Get_PWM_Output_Str(uint8_t tmp_port)
{
    return (&PWM_Output_Buffer_Str[tmp_port]);
}
