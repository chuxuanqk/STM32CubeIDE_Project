/*
 * keyboard_scan.c
 *
 *  Created on: 2020年1月3日
 *      Author: Fox
 */

/*--------------------------------矩阵键盘初始化----------------------------------------
*  功能:  初始化stm32单片机GPIO  //PA0-PA7
*  参数传递：
*  输入：  无
*  返回值: 无
--------------------------------------------------------------------------------------*/

#include "virtual_device/keyboard_scan.h"

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

/* 矩阵按键引脚 3*2 大网关 */
#define GPIO_PIN_LINES (GPIO_Pin_3 | GPIO_Pin_4)
#define GPIO_LINE (GPIOB)
#define GPIO_PIN_LINES2 (GPIO_Pin_4)
#define GPIO_LINE2 (GPIOA)

#define GPIO_PIN_COLS (GPIO_Pin_13 | GPIO_Pin_14)
#define GPIO_COL (GPIOA)

#define LINE_INPUT_MASK ((uint16_t)0x0018)  // 行线掩码 PA3 PA4
#define LINE2_INPUT_MASK ((uint16_t)0x0004) // 行线掩码 PB4>>2
#define COL_INPUT_MASK ((uint16_t)0x6000)   // 列线掩码 PA13 PA14

/*------------------------------------------------------------------------------------
*  功能:  设置列输出和行输入
*  参数传递：
*  输入：  无
*  返回值: 无
--------------------------------------------------------------------------------------*/
inline void KeyBoard_KeyLines_ReInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 设置行输出
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LINES;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIO_LINE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LINES2;
    GPIO_Init(GPIO_LINE2, &GPIO_InitStructure);

    // 设置列输入
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_COLS;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIO_COL, &GPIO_InitStructure);

    GPIO_SetBits(GPIO_LINE, GPIO_PIN_LINES);   // 行输出高电平
    GPIO_SetBits(GPIO_LINE2, GPIO_PIN_LINES2); // 行输出高电平
}

/*------------------------------------------------------------------------------------
*  功能:  设置行输出和列输入
*  参数传递：
*  输入：  无
*  返回值: 无
--------------------------------------------------------------------------------------*/
inline void KeyBoard_Keycolumns_ReInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 设置行输入
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LINES;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIO_LINE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LINES2;
    GPIO_Init(GPIO_LINE2, &GPIO_InitStructure);

    // 设置列输出
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_COLS;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIO_COL, &GPIO_InitStructure);

    // 列输出高电平
    GPIO_SetBits(GPIO_COL, GPIO_PIN_COLS);
}

/**
 * @description: 读取行线的输入状态
 * @param {*}
 * @return {*}
 */
inline uint16_t KeyBoard_KeyLines_Read(void)
{
    return ((GPIO_ReadInputData(GPIO_LINE) & LINE_INPUT_MASK) |
            ((GPIO_ReadInputData(GPIO_LINE2) >> 2) & LINE2_INPUT_MASK));
}

/**
 * @description: 读取列线的输入状态

 * @param {*}
 * @return {*}
 */
inline uint16_t KeyBoard_Keycolumns_Read(void)
{
    return ((GPIO_ReadInputData(GPIO_COL) & COL_INPUT_MASK));
}

void KeyBoard_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    KeyBoard_KeyLines_ReInit();

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE); // 关闭SW调试映射

    return;
}

/******************************************************************************************/

/**
 * @description: 
 * @param {*}
 * @return {*}
 */
void KeyBoardSingle_Init(void)
{
    RCC_APB2PeriphClockCmd(Button_RCC, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = SET_PINx | NOT_DISTURB_PINx | CLEAN_PINx | LATER_PINx;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 外部上拉，浮空检测
    GPIO_Init(SET_GPIOx, &GPIO_InitStructure);
}

/**
 * @description: 普通按键检测
 * @param {*}
 * @return {*}
 */
inline uint16_t bsp_ButtonSingle_Read(void)
{
    uint16_t val = (uint16_t)(Button_Read(SET_GPIOx, SET_PINx) |
                              (Button_Read(NOT_DISTURB_GPIOx, NOT_DISTURB_PINx) << 1) |
                              (Button_Read(CLEAN_GPIOx, CLEAN_PINx) << 2) |
                              (Button_Read(LATER_GPIOx, LATER_PINx) << 3));
    val = (~val) & 0x000F;

    return val;
}
