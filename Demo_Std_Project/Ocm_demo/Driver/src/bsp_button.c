/*
*********************************************************************************************************
*	                                  
*	模块名称 : 按键驱动模块
*	文件名称 : bsp_button.c
*	版    本 : V2.0
*	说    明 : 实现按键的检测，具有软件滤波机制，可以检测如下事件：
*				(1) 按键按下
*				(2) 按键弹起
*				(3) 长按键
*				(4) 长按时自动连发
*				(5) 组合键
*
*	修改记录 :
*		版本号  日期       作者    说明
*		v0.1    2009-12-27 armfly  创建该文件，ST固件库版本为V3.1.2
*		v1.0    2011-01-11 armfly  ST固件库升级到V3.4.0版本。
*       v2.0    2011-10-16 armfly  ST固件库升级到V3.5.0版本。
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "stm32f10x.h"
#include <stdio.h>

#include "bsp_button.h"
#include "keyboard.h"

/*
			   .column
*/
// 按键线码定义 KEYCODE_SWX = KeyLines * 65536 + Keycolumns
// #define KEYCODE_SW21 (0x00100001)
// #define KEYCODE_SW22 (0x00080001)
// #define KEYCODE_SW23 (0x00040001)
// #define KEYCODE_SW24 (0x00020001)
// #define KEYCODE_SW25 (0x00010001)

// #define KEYCODE_SW16 (0x00100002)
// #define KEYCODE_SW17 (0x00080002)
// #define KEYCODE_SW18 (0x00040002)
// #define KEYCODE_SW19 (0x00020002)
// #define KEYCODE_SW20 (0x00010002)

#define KEYCODE_SW11 (0x00020002)
#define KEYCODE_SW12 (0x00010002)
#define KEYCODE_SW13 (0x00080001)
#define KEYCODE_SW14 (0x00040001)
// #define KEYCODE_SW15 (0x00010004)

#define KEYCODE_SW6 (0x00040004)
#define KEYCODE_SW7 (0x00020004)
#define KEYCODE_SW8 (0x00010004)
#define KEYCODE_SW9 (0x00080002)
#define KEYCODE_SW10 (0x00040002)

#define KEYCODE_SW1 (0x00080008)
#define KEYCODE_SW2 (0x00040008)
#define KEYCODE_SW3 (0x00020008)
#define KEYCODE_SW4 (0x00010008)
#define KEYCODE_SW5 (0x00080004)

BUTTON_T s_Btn;
//static KEY_FIFO_T s_Key;		/* 按键FIFO变量,结构体 */
void bsp_InitButtonVar(void);
__IO uint16_t bsp_DetectButton(BUTTON_T *_pBtn);

/*
*********************************************************************************************************
*	函 数 名: bsp_Button_Tick
*	功能说明: 按键滤波器
*	形    参：
*			 
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_Button_Tick(void)
{
	if (s_Btn.fliter_timer > 0)
	{
		s_Btn.fliter_timer--;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_InitButtonVar
*	功能说明: 初始化按键变量
*	形    参：strName : 例程名称字符串
*			  strDate : 例程发布日期
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitButtonVar(void)
{
	/* 初始化按键变量，支持按下、弹起、长按 */
	KeyBoard_Init();
	KeyBoard_KeyLines_ReInit();

	s_Btn.FilterTime = BUTTON_FILTER_TIME;	 /* 按键滤波时间 */
	s_Btn.LongTime = BUTTON_LONG_TIME;		 /* 长按时间 */
	s_Btn.State = 0;						 /* 按键缺省状态，0为未按下*/
	s_Btn.KeyCodeShortDown = KEY_Short_DOWN; /* 按键按下的键值代码 */
	s_Btn.KeyCodeShortUp = KEY_Short_UP;	 /* 按键弹起的键值代码 */
	s_Btn.KeyCodeLongDown = KEY_LONG_DOWN;	 /* 按键被持续按下的键值代码*/
	s_Btn.KeyCodeLongUp = KEY_LONG_UP;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_DetectButton
*	功能说明: 检测一个按键。非阻塞状态，必须被周期性的调用。
*	形    参：按键结构变量指针
*	返 回 值: Read_KeyValue_Code = Read_KeyValue_transmit + KEY_ENUM * 256
			Read_KeyValue_transmit：映射的键号
			KEY_ENUM 按键状态
*********************************************************************************************************
*/
__IO uint16_t bsp_DetectButton(BUTTON_T *_pBtn)
{
	uint16_t Read_KeyValue_Code = 0;
	static uint8_t Read_KeyValue_Step = 0, Read_KeyValue_transmit;
	static uint16_t KeyLines, Keycolumns;

	switch (Read_KeyValue_Step)
	{
	//判断是否有键盘输入
	case 0:
	{
		// KeyBoard_KeyLines_ReInit();
		Keycolumns = KeyBoard_Keycolumns_Read();
		if (Keycolumns != 0)
		{
			Read_KeyValue_Step = 1;
			_pBtn->LongCount = 0;
			_pBtn->RepeatCount = 0;

			_pBtn->fliter_timer = BUTTON_FILTER_TIME;
		}
		break;
	}
	//键盘软件滤波完成，开始检查键值
	case 1:
	{
		if (_pBtn->fliter_timer == 0)
		{
			Keycolumns = KeyBoard_Keycolumns_Read();
			if (Keycolumns != 0)
			{
				Read_KeyValue_Step = 2;
				Read_KeyValue_transmit = 0xff;
			}
			else
			{
				Read_KeyValue_Step = 0;
			}
		}
		break;
	}
	//根据键盘编码产生键值
	case 2:
	{
		if (_pBtn->fliter_timer == 0)
		{
			KeyBoard_Keycolumns_ReInit();
			KeyLines = KeyBoard_KeyLines_Read();

			switch (KeyLines * 65536 + Keycolumns)
			{
				/*********单独按键****************************/

			case KEYCODE_SW1:
			{
				Read_KeyValue_transmit = 0;
			}
			break;
			case KEYCODE_SW2:
			{
				Read_KeyValue_transmit = 1;
			}
			break;
			case KEYCODE_SW3:
			{
				Read_KeyValue_transmit = 2;
			}
			break;
			case KEYCODE_SW4:
			{
				Read_KeyValue_transmit = 3;
			}
			break;

			case KEYCODE_SW5:
			{
				Read_KeyValue_transmit = 4;
			}
			break;
			case KEYCODE_SW6:
			{
				Read_KeyValue_transmit = 5;
			}
			break;
			case KEYCODE_SW7:
			{
				Read_KeyValue_transmit = 6;
			}
			break;
			case KEYCODE_SW8:
			{
				Read_KeyValue_transmit = 7;
			}
			break;
			case KEYCODE_SW9:
			{
				Read_KeyValue_transmit = 8;
			}
			break;
			case KEYCODE_SW10:
			{
				Read_KeyValue_transmit = 9;
			}
			break;
			case KEYCODE_SW11:
			{
				Read_KeyValue_transmit = 10;
			}
			break;
			case KEYCODE_SW12:
			{
				Read_KeyValue_transmit = 11;
			}
			break;
			case KEYCODE_SW13:
			{
				Read_KeyValue_transmit = 12;
			}
			break;
			case KEYCODE_SW14:
			{
				Read_KeyValue_transmit = 13;
			}
			break;

			/*********组合按键****************************/
			case KEYCODE_SW3 | KEYCODE_SW4:
			{
				Read_KeyValue_transmit = 10;
			}
			break;
			case KEYCODE_SW5 | KEYCODE_SW6:
			{
				Read_KeyValue_transmit = 11;
			}
			break;
			case KEYCODE_SW7 | KEYCODE_SW8:
			{
				Read_KeyValue_transmit = 12;
			}
			break;
			case KEYCODE_SW9 | KEYCODE_SW10:
			{
				Read_KeyValue_transmit = 13;
			}
			break;
			case KEYCODE_SW3 | KEYCODE_SW5:
			{
				Read_KeyValue_transmit = 14;
			}
			break;
			case KEYCODE_SW6 | KEYCODE_SW8:
			{
				Read_KeyValue_transmit = 15;
			}
			break;

			default:
			{
				//Read_KeyValue_transmit = 1; // fordubug
			}
			}

			if (Read_KeyValue_transmit != 0xff)
			{
				Read_KeyValue_Step = 10;
			}
			else
			{
				Read_KeyValue_Step = 0;
				KeyBoard_KeyLines_ReInit();
			}
		}
		break;
	}
	//key pressed
	case 10:
	{
		if (_pBtn->fliter_timer == 0)
		{
			if (_pBtn->State == 0)
			{
				_pBtn->State = 1;
				/* 发送按钮按下的消息 */
				if (_pBtn->KeyCodeShortDown > 0)
				{
					/* 键值放入按键FIFO */
					Read_KeyValue_Code = Read_KeyValue_transmit + _pBtn->KeyCodeShortDown * 256;
				}
			}

			if (KeyLines == KeyBoard_KeyLines_Read()) //按键持续按下
			{
				if (_pBtn->LongTime > 0) //allow longpress
				{
					/* 发送按钮持续按下的消息*/
					if (_pBtn->LongCount < BUTTON_LONG_TIME_MAX)
					{
						_pBtn->LongCount++;
						if (_pBtn->LongCount == _pBtn->LongTime)
						{
							/* 键值放入按键FIFO*/
							Read_KeyValue_Code = Read_KeyValue_transmit + _pBtn->KeyCodeLongDown * 256;
						}
					}
				}
				_pBtn->fliter_timer = BUTTON_FILTER_TIME;
			}
			else //按键松手
			{
				Read_KeyValue_Step = 11;
				_pBtn->fliter_timer = BUTTON_FILTER_TIME * 2;
			}
		}
		break;
	}
	//key release
	case 11:
	{
		if (_pBtn->fliter_timer == 0)
		{
			/* 发送按钮弹起的消息*/
			if (_pBtn->LongCount >= _pBtn->LongTime)
			{
				if (_pBtn->KeyCodeLongUp > 0)
				{
					/* 键值放入按键FIFO */
					Read_KeyValue_Code = Read_KeyValue_transmit + _pBtn->KeyCodeLongUp * 256;
				}
			}
			else
			{
				if (_pBtn->KeyCodeShortUp > 0)
				{
					/* 键值放入按键FIFO */
					Read_KeyValue_Code = Read_KeyValue_transmit + _pBtn->KeyCodeShortUp * 256;
				}
			}
			_pBtn->State = 0;

			Read_KeyValue_Step = 0;
			KeyBoard_KeyLines_ReInit();
		}
		break;
	}

	default:
		break;
	}
	return Read_KeyValue_Code;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_KeyPro
*	功能说明: 检测所有按键。非阻塞状态，必须被周期性的调用。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
uint16_t bsp_KeyPro(void)
{
	return bsp_DetectButton(&s_Btn); /*  键 */
	;
}
