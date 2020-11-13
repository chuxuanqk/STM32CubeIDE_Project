/*
 * @Author: your name
 * @Date: 2020-10-14 11:01:34
 * @LastEditTime: 2020-10-14 16:26:51
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \code\Ocm_demo\Driver\inc\led_7seg.h
 */
/*
 * led_7seg.h
 *
 *  Created on: 2020年10月14日
 *      Author: Administrator
 */

#ifndef LED_7SEG_H_
#define LED_7SEG_H_

#include "stdint.h"

//7段数码管控制
#define LED_7SEG_DEC_LEN 4      //7段数码管位数
#define LED_7SEG_DEC_BUF_LEN 16 //7段数码管最多可设置的帧数
typedef struct
{
    char show_ascii_array[LED_7SEG_DEC_BUF_LEN][LED_7SEG_DEC_LEN];    //数码管显示的ASCII码
    char show_ascii_array_ex[LED_7SEG_DEC_BUF_LEN][LED_7SEG_DEC_LEN]; //上一次数码管显示的ASCII码
    uint8_t show_frame_len;                                           //动画显示的长度
    uint8_t show_frame_len_ex;                                        //上一次显示动画帧数
    uint8_t show_frame_num_cur;                                       //当前显示的帧号
    uint16_t show_frame_delay_ms;                                     //动画显示的每一帧时间间隔
    uint16_t show_frame_delay_ms_ex;                                  //上一次显示的动画帧时间间隔
    uint16_t show_frame_delay_ms_cur;                                 //动画显示每一帧时间间隔动态变化寄存器
} LED_7SEG_DEC_SHOW_Str;

void LED_7SEG_Init(void);
void LED_7SEG_DEC_Set(char *show_array, uint16_t array_len, uint16_t frame_delay_ms,
                      char *show_array_ex, uint16_t array_len_ex, uint16_t frame_delay_ms_ex);
void LED_7SEG_DEC_SHOW(void);

#endif /* LED_7SEG_H_ */
