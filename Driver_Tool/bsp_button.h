/*
 * bsp_button.h
 *
 *  Created on: 2020年10月28日
 *      Author: Saber
 */

#include "stm32f10x.h"
#ifndef __BSP_BUTTON_H
#define __BSP_BUTTON_H

/* 按键滤波时间50ms, 单位10ms
 只有连续检测到50ms状态不变才认为有效，包括弹起和按下两种事件
*/
#define BUTTON_FILTER_TIME 50
// #define BUTTON_LONG_TIME 1000 / BUTTON_FILTER_TIME /* 持续1秒，认为长按事件 */
#define BUTTON_LONG_TIME 500 / BUTTON_FILTER_TIME /* 持续1秒，认为长按事件 */
#define BUTTON_LONG_TIME_MAX 0xFFFF				  /* 长按计数器最长时间 */
/*
	每个按键对应1个全局的结构体变量。
	其成员变量是实现滤波和多种按键状态所必须的
*/
typedef struct
{
	/* 下面是一个函数指针，指向判断按键手否按下的函数*/
	uint8_t (*IsKeyDownFunc)(void); /* 按键按下的判断函数,1表示按下*/
	uint8_t FilterTime;				/* 滤波时间(最大255,表示2550ms)*/
	uint16_t LongTime;				/* 按键按下持续时间, 0表示不检测长按*/
	uint8_t Count;					/*  滤波器计数 */
	uint8_t State;					/* 按键当前状态（按下还是弹起） */
	uint8_t KeyCodeShortDown;		/* 按键短按*/
	uint8_t KeyCodeShortUp;			/* 按键短按弹起*/
	uint8_t KeyCodeLongDown;		/* 按键长按 */
	uint8_t KeyCodeLongUp;			/* 按键长按弹起*/
	uint16_t LongCount;				/* 长按计数器*/
	uint8_t RepeatSpeed;			/* 连续按键周期 */
	uint8_t RepeatCount;			/* 连续按键计数器 */
	uint16_t fliter_timer;			//滤波计时器
} BUTTON_T;

/* 定义键值代码
	推荐使用enum, 不用#define，原因：
	(1) 便于新增键值,方便调整顺序，使代码看起来舒服点
	(2)	编译器可帮我们避免键值重复。
*/

/* 按键状态 */
typedef enum
{
	KEY_NONE = 0,	  /* 0表示按键事件  */
	KEY_Short_DOWN,	  /* 键短按下  */
	KEY_Short_UP,	  /* 键短按弹起  */
	KEY_LONG_DOWN,	  /* 键长按下  */
	KEY_LONG_UP,	  /* 键长按弹起 */
	KEY_LONG_3S_DOWN, /* 键长按3S */
	KEY_LONG_3S_UP,	  /* 键长按3S弹起 */
	KEY_LONG_6S_DOWN, /* 键长按6S */
	KEY_LONG_6S_UP,	  /* 键长按6S弹起 */
	KEY_LONG_8S_DOWN, /* 键长按8S */
	KEY_LONG_8S_UP,	  /* 键长按8s弹起 */
} KEY_STATE;

/* 按键ID */
typedef enum
{
	KEY_ZERO = 0,
	KEY_SW1,
	KEY_SW2,
	KEY_SW3,
	KEY_SW4,
	KEY_SW5,
	KEY_SW6,
} KEY_ID;

/* 供外部调用的函数声明 */
uint16_t bsp_KeyPro(void);
void bsp_Button_Tick(void);
void bsp_InitButtonVar(void);

#endif
