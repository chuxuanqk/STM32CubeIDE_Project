/*
 * keyboard_scan.h
 *
 *  Created on: 2020年1月3日
 *      Author: Fox
 */

#ifndef SRC_VIRTUAL_DEVICE_KEYBOARD_SCAN_H_
#define SRC_VIRTUAL_DEVICE_KEYBOARD_SCAN_H_

#include "stm32f10x.h"

/* 2*3 矩阵按键 */
/*
* 按键线码定义 KEYCODE_SWX = KeyLines * 65536 + Keycolumns
* KEYCODE_SWX = (KeyLines << 16) + Keycolumns
*/
#define KEYCODE_SW1 (0x00082000)
#define KEYCODE_SW2 (0x00084000)
#define KEYCODE_SW3 (0x00102000)
#define KEYCODE_SW4 (0x00104000)
#define KEYCODE_SW5 (0x00042000)
#define KEYCODE_SW6 (0x00044000)

void KeyBoard_KeyLines_ReInit(void);
void KeyBoard_Keycolumns_ReInit(void);
void KeyBoard_Init(void);
uint16_t KeyBoard_KeyLines_Read(void);
uint16_t KeyBoard_Keycolumns_Read(void);

/* 普通按键 */
#define SET_GPIOx (GPIOA)
#define SET_PINx (GPIO_Pin_5)
#define NOT_DISTURB_GPIOx (GPIOA)
#define NOT_DISTURB_PINx (GPIO_Pin_6)
#define CLEAN_GPIOx (GPIOA)
#define CLEAN_PINx (GPIO_Pin_7)
#define LATER_GPIOx (GPIOA)
#define LATER_PINx (GPIO_Pin_8)

#define Button_RCC (RCC_APB2Periph_GPIOA)
#define Button_Read(GPIOx, PINx) GPIO_ReadInputDataBit(GPIOx, PINx)

void KeyBoardSingle_Init(void);
uint16_t bsp_ButtonSingle_Read(void);

#endif /* SRC_VIRTUAL_DEVICE_KEYBOARD_SCAN_H_ */
