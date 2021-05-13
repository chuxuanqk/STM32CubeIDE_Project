/*
 * led_7seg.c
 *
 *  Created on: 2020年10月14日
 *      Author: Administrator
 */

#include "virtual_device/bsp_led_7seg.h"

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

/********7段数码管部分定义*********************************************/

#define Timer_Base_7SEG_Delay (1000 / (90 * 4)) //根据人眼余晖效应计算延时时间,刷新率不低于60Hz, 70Hz较稳定

LED_7SEG_DEC_SHOW_Str LED_7SEG_DEC_BUF_Str;

/**************************************
 * 数码管段IO; DALI网关数码管IO口
 * seg_a : PA5
 * seg_b : PC15
 * seg_c : PA1
 * seg_d : PA3
 * seg_e : PA2
 * seg_f : PA4
 * seg_g : PA7
 * seg_dp: PA6
***************************************/
// #define seg_a(X) ((X == 1) ? (GPIOA->BSRR = GPIO_Pin_5) : (GPIOA->BRR = GPIO_Pin_5))
// #define seg_b(X) ((X == 1) ? (GPIOC->BSRR = GPIO_Pin_15) : (GPIOC->BRR = GPIO_Pin_15))
// #define seg_c(X) ((X == 1) ? (GPIOA->BSRR = GPIO_Pin_1) : (GPIOA->BRR = GPIO_Pin_1))
// #define seg_d(X) ((X == 1) ? (GPIOA->BSRR = GPIO_Pin_3) : (GPIOA->BRR = GPIO_Pin_3))
// #define seg_e(X) ((X == 1) ? (GPIOA->BSRR = GPIO_Pin_2) : (GPIOA->BRR = GPIO_Pin_2))
// #define seg_f(X) ((X == 1) ? (GPIOA->BSRR = GPIO_Pin_4) : (GPIOA->BRR = GPIO_Pin_4))
// #define seg_g(X) ((X == 1) ? (GPIOA->BSRR = GPIO_Pin_7) : (GPIOA->BRR = GPIO_Pin_7))
// #define seg_dp(X) ((X == 1) ? (GPIOA->BSRR = GPIO_Pin_6) : (GPIOA->BRR = GPIO_Pin_6))

// //位选IO
// // #define dis_com4(X) ((X == 1) ? (GPIOB->BSRR = GPIO_Pin_9) : (GPIOB->BRR = GPIO_Pin_9))
// // #define dis_com3(X) ((X == 1) ? (GPIOB->BSRR = GPIO_Pin_8) : (GPIOB->BRR = GPIO_Pin_8))
// // #define dis_com2(X) ((X == 1) ? (GPIOB->BSRR = GPIO_Pin_7) : (GPIOB->BRR = GPIO_Pin_7))
// // #define dis_com1(X) ((X == 1) ? (GPIOB->BSRR = GPIO_Pin_6) : (GPIOB->BRR = GPIO_Pin_6))
// #define dis_com1(X) ((X == 1) ? (GPIOB->BSRR = GPIO_Pin_9) : (GPIOB->BRR = GPIO_Pin_9))
// #define dis_com2(X) ((X == 1) ? (GPIOB->BSRR = GPIO_Pin_8) : (GPIOB->BRR = GPIO_Pin_8))
// #define dis_com3(X) ((X == 1) ? (GPIOB->BSRR = GPIO_Pin_7) : (GPIOB->BRR = GPIO_Pin_7))
// #define dis_com4(X) ((X == 1) ? (GPIOB->BSRR = GPIO_Pin_6) : (GPIOB->BRR = GPIO_Pin_6))

//数码管段IO
#define seg_a(X) ((X == 1) ? (GPIOA->BSRR = GPIO_Pin_7) : (GPIOA->BRR = GPIO_Pin_7))
#define seg_b(X) ((X == 1) ? (GPIOA->BSRR = GPIO_Pin_0) : (GPIOA->BRR = GPIO_Pin_0))
#define seg_c(X) ((X == 1) ? (GPIOA->BSRR = GPIO_Pin_1) : (GPIOA->BRR = GPIO_Pin_1))
#define seg_d(X) ((X == 1) ? (GPIOA->BSRR = GPIO_Pin_3) : (GPIOA->BRR = GPIO_Pin_3))
#define seg_e(X) ((X == 1) ? (GPIOA->BSRR = GPIO_Pin_2) : (GPIOA->BRR = GPIO_Pin_2))
#define seg_f(X) ((X == 1) ? (GPIOA->BSRR = GPIO_Pin_6) : (GPIOA->BRR = GPIO_Pin_6))
#define seg_g(X) ((X == 1) ? (GPIOB->BSRR = GPIO_Pin_1) : (GPIOB->BRR = GPIO_Pin_1))
#define seg_dp(X) ((X == 1) ? (GPIOB->BSRR = GPIO_Pin_0) : (GPIOB->BRR = GPIO_Pin_0))

//位选IO
#define dis_com4(X) ((X == 1) ? (GPIOB->BSRR = GPIO_Pin_6) : (GPIOB->BRR = GPIO_Pin_6))
#define dis_com3(X) ((X == 1) ? (GPIOB->BSRR = GPIO_Pin_7) : (GPIOB->BRR = GPIO_Pin_7))
#define dis_com2(X) ((X == 1) ? (GPIOB->BSRR = GPIO_Pin_8) : (GPIOB->BRR = GPIO_Pin_8))
#define dis_com1(X) ((X == 1) ? (GPIOB->BSRR = GPIO_Pin_9) : (GPIOB->BRR = GPIO_Pin_9))

//数码管对应的按键
// #define LED7seg_key_set GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6)
// #define LED7seg_key_1 (1) //GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6)
// #define LED7seg_key_2 (1) //GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7)
// #define LED7seg_key_3 (1) //GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)
// #define LED7seg_key_4 (1) //GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9)

//数码管编码0-9，A-Z
const uint8_t Dis_Code[40] = {
    0X3F, //0
    0x06, //1
    0x5B, //2
    0x4F, //3
    0X66, //4
    0X6D, //5
    0X7D, //6
    0X07, //7
    0X7F, //8
    0X6F, //9
    0x77, //A
    0X7C, //b
    0X39, //C
    0X5E, //d
    0X79, //E
    0X71, //F
    0X3D, //G
    0X76, //H
    0X10, //I
    0X0E, //J
    0X7A, //K
    0X38, //L
    0X55, //M
    0X54, //N
    0X5C, //O
    0X73, //P
    0X67, //Q
    0X50, //R
    0X6D, //S
    0X78, //T
    0X3E, //U
    0X62, //V
    0X6A, //W
    0X36, //X
    0X6E, //Y
    0X49  //Z
};

const char _Led_7Seg_Show_Band[7][LED_7SEG_DEC_LEN] = {
    {'*', '*', '*', '*'},
    {'*', '*', '*', 'S'},
    {'*', '*', 'C', 'S'},
    {'*', 'G', 'C', 'S'},
    {'o', 'G', 'C', 'S'},
    {'*', 'G', 'C', 'S'},
    {'o', 'G', 'C', 'S'},
};

// const char Led_7Seg_Show_Reset[6][LED_7SEG_DEC_LEN] = {
//     {'L', 'I', 'I', 'K'},
//     {'*', '*', '*', '*'},
//     {'L', 'I', 'I', 'K'},
//     {'*', '*', '*', '*'},
//     {'L', 'I', 'I', 'K'},
//     {'*', '*', '*', '*'}};

//显示一个字符,0-9,A-Z
static void Led_7seg_Dis_Char(char cha)
{
    uint8_t code;
    //数字
    if (cha >= '0' && cha <= '9')
        code = Dis_Code[cha - '0'];
    //大写字母
    else if (cha >= 'A' && cha <= 'Z')
        code = Dis_Code[cha - 'A' + 10];
    //小写字母
    else if (cha >= 'a' && cha <= 'z')
        code = Dis_Code[cha - 'a' + 10];
    else
        code = 0;

    code = ~code;
    //管脚输出
    seg_a(((code & 0x01) > 0));
    seg_b(((code & 0x02) > 0));
    seg_c(((code & 0x04) > 0));
    seg_d(((code & 0x08) > 0));
    seg_e(((code & 0x10) > 0));
    seg_f(((code & 0x20) > 0));
    seg_g(((code & 0x40) > 0));
}

//7段数码管引脚定义
void LED_7SEG_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure2;
    //第一步：配置时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
    //第二步，配置goio口
    /******** GPIOB 引脚配置*********/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); //重映射必须要开AFIO时钟
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE); //禁止JTAG功能，把PB3，PB4作为普通IO口使用

    /******** GPIOA 引脚配置*********/
    // GPIO_InitStructure2.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure2.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure2.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure2.GPIO_Mode = GPIO_Mode_Out_PP; //设置为推免输出
    GPIO_Init(GPIOA, &GPIO_InitStructure2);

    /******** GPIOB 引脚配置*********/
    // GPIO_InitStructure2.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure2.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_InitStructure2);

    /******** GPIOC 引脚配置*********/
    GPIO_InitStructure2.GPIO_Pin = GPIO_Pin_15;
    GPIO_Init(GPIOC, &GPIO_InitStructure2);

    dis_com1(1);
    dis_com2(1);
    dis_com3(1);
    dis_com4(1);
    seg_a(1);
    seg_b(1);
    seg_c(1);
    seg_d(1);
    seg_e(1);
    seg_f(1);
    seg_g(1);
    seg_dp(1);

    LED_7SEG_DEC_Set((char *)_Led_7Seg_Show_Band, sizeof(_Led_7Seg_Show_Band), 250, //显示数码管动画
                     (char *)_Led_7Seg_Show_Band, 0, 250);
}

//定时回调函数，利用余晖效应动态刷新数码管
//4位数码管显示，可实现静态或动态数码管显示4位ascii码,可显示0-9,A-Z,空格及小写n
//总共可设置16帧动画
//每帧显示时间间隔相同，可设置，单位ms
/**
 * @description: show_array 当前帧指针  
 *               array_len 数组长度   0 则显示默认帧 "RONE"
 *               frame_delay_ms 显示的每一帧时间间隔
 *               show_array_ex 前一帧指针 
 *               array_len_ex 数组长度  0 则不显示
 *               frame_delay_ms_ex 显示的每一帧时间间隔
 * @param {*}
 * @return {*}
 */
void LED_7SEG_DEC_Set(char *show_array, uint16_t array_len, uint16_t frame_delay_ms,
                      char *show_array_ex, uint16_t array_len_ex, uint16_t frame_delay_ms_ex)
{
    uint16_t i, j, k;

    LED_7SEG_DEC_BUF_Str.show_frame_len = array_len / LED_7SEG_DEC_LEN;
    LED_7SEG_DEC_BUF_Str.show_frame_num_cur = 0;
    LED_7SEG_DEC_BUF_Str.show_frame_delay_ms = frame_delay_ms;
    LED_7SEG_DEC_BUF_Str.show_frame_delay_ms_cur = frame_delay_ms;

    k = 0;
    for (j = 0; j < LED_7SEG_DEC_BUF_Str.show_frame_len; j++)
        for (i = 0; i < LED_7SEG_DEC_LEN; i++)
        {
            LED_7SEG_DEC_BUF_Str.show_ascii_array[j][i] = show_array[k];
            k++;
        }

    k = 0;
    LED_7SEG_DEC_BUF_Str.show_frame_len_ex = array_len_ex / LED_7SEG_DEC_LEN;
    LED_7SEG_DEC_BUF_Str.show_frame_delay_ms_ex = frame_delay_ms_ex;
    for (j = 0; j < LED_7SEG_DEC_BUF_Str.show_frame_len_ex; j++)
        for (i = 0; i < LED_7SEG_DEC_LEN; i++)
        {
            LED_7SEG_DEC_BUF_Str.show_ascii_array_ex[j][i] = show_array_ex[k];
            k++;
        }
}

void LED_7SEG_DEC_SHOW(void)
{
    static uint16_t timer_7seg_loop_delay_ms = Timer_Base_7SEG_Delay; //7段显示数码管扫描延时
    static uint16_t seg_num_current = 0;                              //7段显示数码管位置
    static char led_7seg_show[LED_7SEG_DEC_LEN];                      //数码管动态显示的字符

    uint8_t i, j;

    if (LED_7SEG_DEC_BUF_Str.show_frame_delay_ms_cur > 0) //处理帧延时
    {
        LED_7SEG_DEC_BUF_Str.show_frame_delay_ms_cur--;
    }

    //7段数码管扫描处理
    if (timer_7seg_loop_delay_ms > 0) //处理扫描延时
    {
        timer_7seg_loop_delay_ms--;
        if (timer_7seg_loop_delay_ms == 0)
        {
            //禁止所有位选IO输出,避免余晖效应
            timer_7seg_loop_delay_ms = Timer_Base_7SEG_Delay;
            dis_com1(1);
            dis_com2(1);
            dis_com3(1);
            dis_com4(1);
            seg_a(1);
            seg_b(1);
            seg_c(1);
            seg_d(1);
            seg_e(1);
            seg_f(1);
            seg_g(1);

            //根据显示模式选择需要刷新显示的内容
            if (LED_7SEG_DEC_BUF_Str.show_frame_len == 0) //没有设置，显示默认信息
            {
                //显示 "1ONE"
                led_7seg_show[0] = 'E';
                led_7seg_show[1] = 'N';
                led_7seg_show[2] = '0';
                led_7seg_show[3] = 'R';
            }
            else if (LED_7SEG_DEC_BUF_Str.show_frame_len == 1) //只有一帧，静态显示
            {
                if (LED_7SEG_DEC_BUF_Str.show_frame_delay_ms_cur == 0)
                {
                    if (LED_7SEG_DEC_BUF_Str.show_frame_len_ex) //设置了显示上一个动画
                    {
                        for (j = 0; j < LED_7SEG_DEC_BUF_Str.show_frame_len_ex; j++) //填充上一个动画帧数据
                            for (i = 0; i < LED_7SEG_DEC_LEN; i++)
                                LED_7SEG_DEC_BUF_Str.show_ascii_array[j][i] = LED_7SEG_DEC_BUF_Str.show_ascii_array_ex[j][i];
                        LED_7SEG_DEC_BUF_Str.show_frame_len = LED_7SEG_DEC_BUF_Str.show_frame_len_ex;
                        LED_7SEG_DEC_BUF_Str.show_frame_len_ex = 0;
                        LED_7SEG_DEC_BUF_Str.show_frame_delay_ms = LED_7SEG_DEC_BUF_Str.show_frame_delay_ms_ex;
                        LED_7SEG_DEC_BUF_Str.show_frame_delay_ms_cur = LED_7SEG_DEC_BUF_Str.show_frame_delay_ms; //第一帧，重新计时
                    }
                    LED_7SEG_DEC_BUF_Str.show_frame_num_cur = 0; //所有帧显示完成，指向第一帧重复显示
                }

                for (i = 0; i < LED_7SEG_DEC_LEN; i++)
                {
                    led_7seg_show[i] = LED_7SEG_DEC_BUF_Str.show_ascii_array[0][i]; //填充数码管显示ascii缓冲区
                }
            }
            else //显示动画
            {
                if (LED_7SEG_DEC_BUF_Str.show_frame_num_cur == 0) //第一帧，直接显示
                {
                    for (i = 0; i < LED_7SEG_DEC_LEN; i++)
                    {
                        led_7seg_show[i] = LED_7SEG_DEC_BUF_Str.show_ascii_array[LED_7SEG_DEC_BUF_Str.show_frame_num_cur][i];
                    }
                    LED_7SEG_DEC_BUF_Str.show_frame_delay_ms_cur = LED_7SEG_DEC_BUF_Str.show_frame_delay_ms; //第一帧，重新计时
                    LED_7SEG_DEC_BUF_Str.show_frame_num_cur++;                                               //指向下一帧显示
                }
                else if (LED_7SEG_DEC_BUF_Str.show_frame_delay_ms_cur == 0) //其他帧，延时等待显示
                {
                    if ((LED_7SEG_DEC_BUF_Str.show_frame_num_cur > (LED_7SEG_DEC_BUF_Str.show_frame_len - 1)) ||
                        (LED_7SEG_DEC_BUF_Str.show_frame_num_cur > (LED_7SEG_DEC_BUF_LEN - 1))) //禁止超出显示缓冲区大小
                    {
                        if (LED_7SEG_DEC_BUF_Str.show_frame_len_ex) //设置了显示上一个动画
                        {
                            for (j = 0; j < LED_7SEG_DEC_BUF_Str.show_frame_len_ex; j++) //填充上一个动画帧数据
                                for (i = 0; i < LED_7SEG_DEC_LEN; i++)
                                    LED_7SEG_DEC_BUF_Str.show_ascii_array[j][i] = LED_7SEG_DEC_BUF_Str.show_ascii_array_ex[j][i];
                            LED_7SEG_DEC_BUF_Str.show_frame_len = LED_7SEG_DEC_BUF_Str.show_frame_len_ex;
                            LED_7SEG_DEC_BUF_Str.show_frame_len_ex = 0;
                            LED_7SEG_DEC_BUF_Str.show_frame_delay_ms = LED_7SEG_DEC_BUF_Str.show_frame_delay_ms_ex;
                        }
                        LED_7SEG_DEC_BUF_Str.show_frame_num_cur = 0; //所有帧显示完成，指向第一帧重复显示
                    }
                    else
                    {
                        for (i = 0; i < LED_7SEG_DEC_LEN; i++) //填充新的帧显示数据
                        {
                            led_7seg_show[i] = LED_7SEG_DEC_BUF_Str.show_ascii_array[LED_7SEG_DEC_BUF_Str.show_frame_num_cur][i];
                        }
                        LED_7SEG_DEC_BUF_Str.show_frame_delay_ms_cur = LED_7SEG_DEC_BUF_Str.show_frame_delay_ms;
                        LED_7SEG_DEC_BUF_Str.show_frame_num_cur++;
                    }
                }
            }

            //使用7段数码管扫描显示法，动态显示
            //位数 1|2|3|4 对应 GPIOB 9|8|7|6
            switch (seg_num_current) //选择数码管位数
            {
            case 0:
            {
                dis_com1(0); //使能对应位选IO输出
                break;
            }
            case 1:
            {
                dis_com2(0);
                break;
            }
            case 2:
            {
                dis_com3(0);
                break;
            }
            case 3:
            {
                dis_com4(0);
                break;
            }
            }
            //动态显示一个字符
            Led_7seg_Dis_Char(led_7seg_show[seg_num_current]);
            //判断是否显示小数点
            // if (position == pointer)
            // {
            // 	GPIO_ResetBits(GPIOB, GPIO_Pin_0); //显示小数点
            // }
            //循环变量控制
            seg_num_current++;
            if (seg_num_current > 3)
            {
                seg_num_current = 0;
            }
        }
    }
}
