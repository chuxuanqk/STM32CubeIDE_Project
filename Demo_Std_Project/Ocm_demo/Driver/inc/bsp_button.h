/*
*********************************************************************************************************
*	                                  
*	模块名称 : 按键驱动模块    
*	文件名称 : bsp_button.h
*	版    本 : V2.0
*	说    明 : 头文件
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "stm32f10x.h"
#ifndef __BSP_BUTTON_H
#define __BSP_BUTTON_H

/* 按键滤波时间50ms, 单位10ms
 只有连续检测到50ms状态不变才认为有效，包括弹起和按下两种事件
*/
#define BUTTON_FILTER_TIME 50
#define BUTTON_LONG_TIME 1000 / BUTTON_FILTER_TIME /* 持续1秒，认为长按事件 */
#define BUTTON_LONG_TIME_MAX 0xFFFF				   //长按计数器最长时间
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

typedef enum
{
	KEY_NONE = 0,		/* 0 表示按键事件  */
	KEY_Short_DOWN = 1, /* 键短按下  */
	KEY_Short_UP = 2,   /* 键短按弹起  */
	KEY_LONG_DOWN = 3,  /* 键长按下  */
	KEY_LONG_UP = 4		/* 键长按弹起 */
} KEY_ENUM;

/* 按键ID */
enum
{
	KID_TAMPER = 0,
	KID_WAKEUP,
	KID_USER,
	KID_JOY_UP,
	KID_JOY_DOWN,
	KID_JOY_LEFT,
	KID_JOY_RIGHT,
	KID_JOY_OK
};

/* 按键FIFO用到变量 */
#define KEY_FIFO_SIZE 20
typedef struct
{
	uint16_t Buf[KEY_FIFO_SIZE]; /* 键值缓冲区 */
	uint8_t Read;				 /* 缓冲区读指针 */
	uint8_t Write;				 /* 缓冲区写指针 */
} KEY_FIFO_T;

/* 供外部调用的函数声明 */
void bsp_InitButtonVar(void);
void bsp_PutKey(uint16_t _KeyCode);
uint16_t bsp_GetKey(void);
uint16_t bsp_KeyPro(void);
uint8_t bsp_KeyState(uint8_t _ucKeyID);
uint8_t bsp_KeyState(uint8_t _ucKeyID);
void bsp_Button_Tick(void);
extern BUTTON_T s_Btn;
#endif
