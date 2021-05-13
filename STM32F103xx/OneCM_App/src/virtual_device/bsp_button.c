/*
 * bsp_button.c
 *
 *  Created on: 2020年10月28日
 *      Author: Saber
 */

#include "virtual_device/keyboard_scan.h"
#include "virtual_device/bsp_button.h"

#include <stdio.h>
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

BUTTON_T s_Btn;

__IO uint16_t bsp_DetectButton(BUTTON_T *_pBtn);	   // 矩阵按键检测
__IO uint16_t bsp_DetectButtonSingle(BUTTON_T *_pBtn); // 单个按键检测

/********************************************************************************************************/
/* 矩阵按键 */
/*
*********************************************************************************************************
*	函 数 名: bsp_Button_Tick
*	功能说明: 按键滤波器
*	形    参：
*			 
*	返 回 值: 无
**********************************************************************************************************/
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
*	形    参：
*	返 回 值: 无
**********************************************************************************************************/
void bsp_InitButtonVar(void)
{
	/* 初始化按键变量，支持按下、弹起、长按 */
	KeyBoard_Init();

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
*	形    参: 按键结构变量指针
*	返 回 值: Read_KeyValue_Code = key_number + KEY_STATE * 256
			  Read_KeyValue_transmit：映射的键号
			  KEY_STATE 按键状态
**********************************************************************************************************/
__IO uint16_t bsp_DetectButton(BUTTON_T *_pBtn)
{
	uint16_t Read_KeyValue_Code = 0;
	static uint8_t Read_KeyValue_Step = 0, Read_KeyValue_transmit = KEY_ZERO;
	static uint16_t KeyLines = 0, Keycolumns = 0;

	switch (Read_KeyValue_Step)
	{
	//判断是否有键盘输入
	case 0:
	{
		KeyBoard_KeyLines_ReInit();
		Keycolumns = KeyBoard_Keycolumns_Read();

		if (Keycolumns == 0)
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
				Read_KeyValue_transmit = KEY_SW1;
				break;
			}

			case KEYCODE_SW2:
			{
				Read_KeyValue_transmit = KEY_SW2;
				break;
			}

			case KEYCODE_SW3:
			{
				Read_KeyValue_transmit = KEY_SW3;
				break;
			}

			case KEYCODE_SW4:
			{
				Read_KeyValue_transmit = KEY_SW4;
				break;
			}

			case KEYCODE_SW5:
			{
				Read_KeyValue_transmit = KEY_SW5;
				break;
			}

			case KEYCODE_SW6:
			{
				Read_KeyValue_transmit = KEY_SW6;
				break;
			}

			default:
			{
				Read_KeyValue_transmit = KEY_ZERO;
				break;
			}
			}

			if (Read_KeyValue_transmit != KEY_ZERO)
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
					if (_pBtn->LongCount < BUTTON_LONG_TIME_MAX) /* 发送按钮持续按下的消息*/
					{
						_pBtn->LongCount++;

						if (_pBtn->LongCount == _pBtn->LongTime * 8)
							Read_KeyValue_Code = Read_KeyValue_transmit + (KEY_LONG_8S_DOWN * 256);
						else if (_pBtn->LongCount == _pBtn->LongTime * 6)
							Read_KeyValue_Code = Read_KeyValue_transmit + (KEY_LONG_6S_DOWN * 256);
						else if (_pBtn->LongCount == _pBtn->LongTime * 3)
							Read_KeyValue_Code = Read_KeyValue_transmit + (KEY_LONG_3S_DOWN * 256);
						else if (_pBtn->LongCount == _pBtn->LongTime)
							Read_KeyValue_Code = Read_KeyValue_transmit + _pBtn->KeyCodeLongDown * 256; /* 键值放入按键FIFO*/
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

					if (_pBtn->LongCount >= _pBtn->LongTime * 8)
						Read_KeyValue_Code = Read_KeyValue_transmit + (KEY_LONG_8S_UP * 256);
					else if (_pBtn->LongCount >= _pBtn->LongTime * 6)
						Read_KeyValue_Code = Read_KeyValue_transmit + (KEY_LONG_6S_UP * 256);
					else if (_pBtn->LongCount >= _pBtn->LongTime * 3)
						Read_KeyValue_Code = Read_KeyValue_transmit + (KEY_LONG_3S_UP * 256);
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

/************************************************************************************************/
/* 普通按键 */
typedef enum
{
	START_READ,		   // 判断键盘输入
	DETECT_READ,	   // 滤波检查键值
	MAP_BUTTONVALUE,   // 键值映射
	KEEP_PRESSED = 10, // 长按
	BUTTON_RELEASE,	   // 按键松开
} ReadStep;

/* 按键键值映射 */
#define BUTTONCODE_SET (0x0001)
#define BUTTONCODE_NOT_DISTURB (0x0002)
#define BUTTONCODE_CLEAN (0x0004)
#define BUTTONCODE_LATER (0x0008)

void bsp_InitButtonSingle(void)
{
	KeyBoardSingle_Init();

	s_Btn.FilterTime = BUTTON_FILTER_TIME;	 /* 按键滤波时间 */
	s_Btn.LongTime = BUTTON_LONG_TIME;		 /* 长按时间 */
	s_Btn.State = 0;						 /* 按键缺省状态，0为未按下*/
	s_Btn.KeyCodeShortDown = KEY_Short_DOWN; /* 按键按下的键值代码 */
	s_Btn.KeyCodeShortUp = KEY_Short_UP;	 /* 按键弹起的键值代码 */
	s_Btn.KeyCodeLongDown = KEY_LONG_DOWN;	 /* 按键被持续按下的键值代码*/
	s_Btn.KeyCodeLongUp = KEY_LONG_UP;
}

/**
 * @description: Read_KeyValue_Code = key_number + KEY_STATE * 256
 * @param {*}
 * @return {*}
 */
__IO uint16_t bsp_DetectButtonSingle(BUTTON_T *_pBtn)
{
	uint16_t Read_ButtonValue_Code = 0;
	static ReadStep Read_ButtonValue_Steps = 0;
	static uint8_t Read_ButtonValue_transmits;
	static uint16_t ButtonValue;

	switch (Read_ButtonValue_Steps)
	{
	case START_READ:
	{
		ButtonValue = bsp_ButtonSingle_Read();

		if (ButtonValue == 0)
		{
			Read_ButtonValue_Steps = DETECT_READ;
			_pBtn->LongCount = 0;
			_pBtn->RepeatCount = 0;

			_pBtn->fliter_timer = BUTTON_FILTER_TIME;
		}
		break;
	}

	case DETECT_READ:
	{
		if (_pBtn->fliter_timer == 0)
		{
			ButtonValue = bsp_ButtonSingle_Read();

			if (ButtonValue == 0)
			{
				Read_ButtonValue_Steps = MAP_BUTTONVALUE;
				Read_ButtonValue_transmits = 0xff;
			}
			else
			{
				Read_ButtonValue_Steps = START_READ;
			}
		}
		break;
	}

	case MAP_BUTTONVALUE:
	{
		if (_pBtn->fliter_timer == 0)
		{
			switch (ButtonValue)
			{
			case BUTTONCODE_SET:
			{
				Read_ButtonValue_transmits = Button_SET;
				break;
			}

			case BUTTONCODE_NOT_DISTURB:
			{
				Read_ButtonValue_transmits = Button_NOT_DISTURB;
				break;
			}

			case BUTTONCODE_CLEAN:
			{
				Read_ButtonValue_transmits = Button_CLEAN;
				break;
			}

			case BUTTONCODE_LATER:
			{
				Read_ButtonValue_transmits = Button_LATER;
				break;
			}

			default:
				Read_ButtonValue_transmits = Button_IDLE;
				break;
			}

			if (Read_ButtonValue_transmits != Button_IDLE)
				Read_ButtonValue_Steps = KEEP_PRESSED;
			else
				Read_ButtonValue_Steps = START_READ;
		}
		break;
	}

	case KEEP_PRESSED:
	{
		if (_pBtn->fliter_timer == 0)
		{
			if (_pBtn->State == 0)
			{
				_pBtn->State = 1;
				if (_pBtn->KeyCodeShortDown > 0)
					Read_ButtonValue_Code = Read_ButtonValue_transmits + _pBtn->KeyCodeShortDown * 256;
			}
		}

		if (bsp_ButtonSingle_Read() == 0) // 按键持续按下
		{
			if (_pBtn->LongTime > 0)
			{
				if (_pBtn->LongCount < BUTTON_LONG_TIME_MAX)
				{
					_pBtn->LongCount++;

					if (_pBtn->LongCount == _pBtn->LongTime * 8)
						Read_ButtonValue_Code = Read_ButtonValue_transmits + (KEY_LONG_8S_DOWN * 256);
					else if (_pBtn->LongCount == _pBtn->LongTime * 3)
						Read_ButtonValue_Code = Read_ButtonValue_transmits + (KEY_LONG_3S_DOWN * 256);
					else if (_pBtn->LongCount == _pBtn->LongTime)
						Read_ButtonValue_Code = Read_ButtonValue_transmits + (_pBtn->KeyCodeLongDown * 256);
				}
			}

			_pBtn->fliter_timer = BUTTON_FILTER_TIME;
		}
		else
		{
			Read_ButtonValue_Steps = BUTTON_RELEASE;
			_pBtn->fliter_timer = BUTTON_FILTER_TIME * 2;
		}
		break;
	}

	case BUTTON_RELEASE:
	{
		if (_pBtn->fliter_timer == 0)
		{
			if (_pBtn->LongCount >= _pBtn->LongTime)
			{
				if (_pBtn->KeyCodeLongUp > 0)
				{
					Read_ButtonValue_Code = Read_ButtonValue_transmits + _pBtn->KeyCodeLongUp * 256;

					if (_pBtn->LongCount >= _pBtn->LongTime * 8)
						Read_ButtonValue_Code = Read_ButtonValue_transmits + (KEY_LONG_8S_UP * 256);
					else if (_pBtn->LongCount >= _pBtn->LongTime * 3)
						Read_ButtonValue_Code = Read_ButtonValue_transmits + (KEY_LONG_3S_UP * 256);
				}
			}
			else
			{
				if (_pBtn->KeyCodeShortUp > 0)
					Read_ButtonValue_Code = Read_ButtonValue_transmits + _pBtn->KeyCodeShortUp * 256;
			}

			_pBtn->State = 0;
			Read_ButtonValue_Steps = START_READ;
		}

		break;
	}

	default:
		break;
	}

	return Read_ButtonValue_Code;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_KeyPro
*	功能说明: 检测所有按键。非阻塞状态，必须被周期性的调用。
*	形    参：无
*	返 回 值: 无
**********************************************************************************************************/
uint16_t bsp_KeyPro(void)
{
	return bsp_DetectButton(&s_Btn); /* 矩阵按键 */

	// return bsp_DetectButtonSingle(&s_Btn); /* 普通按键 */
}
