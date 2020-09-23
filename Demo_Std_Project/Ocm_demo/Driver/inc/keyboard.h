/*
 * @Author: your name
 * @Date: 2020-09-23 13:55:51
 * @LastEditTime: 2020-09-23 14:55:49
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \Demo_Std_Project\Ocm_demo\Driver\inc\keyboard.h
 */
/*--------------------------------------------------------------------------------------
*		矩阵键盘驱动
*  文件：      keyboard.h
*  编写人:     LiuHui 
*  描述:       扫描4x4矩阵键盘输入,并返回键值 
*  适用范围:   驱动为ST3.5库编写,适用于STM32F10x系列单片机
*  所用引脚:   PA0-PA7 
*  编写时间:   2013年11月22日

*  版本: 1.0 

--------------------------------------------------------------------------------------*/

#ifndef __KEYBOARD_H
#define __KEYBOARD_H
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

void KeyBoard_KeyLines_ReInit(void);
void KeyBoard_Keycolumns_ReInit(void);

uint16_t KeyBoard_KeyLines_Read(void);
uint16_t KeyBoard_Keycolumns_Read(void);
void KeyBoard_Init(void);
#endif

/*----------------------------------THE END------------------------------------------*/
