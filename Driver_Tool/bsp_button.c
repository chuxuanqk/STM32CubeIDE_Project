/*
 * bsp_button.c
 *
 *  Created on: 2020年10月28日
 *      Author: Saber
 */

#include <stdio.h>

#include "bsp_button.h"

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#include "bsp_led.h"
#include "bsp_uart.h"

/*
* 按键线码定义 KEYCODE_SWX = KeyLines * 65536 + Keycolumns
* KEYCODE_SWX = (KeyLines << 16) + Keycolumns
*/
#define KEYCODE_SW1 (0x80001000)
#define KEYCODE_SW2 (0x80000800)
#define KEYCODE_SW3 (0x40001000)
#define KEYCODE_SW4 (0x40000800)
#define KEYCODE_SW5 (0x20001000)
#define KEYCODE_SW6 (0x20000800)

BUTTON_T s_Btn;

void bsp_InitButtonVar(void);
__IO uint16_t bsp_DetectButton(BUTTON_T *_pBtn);

// 485 大网关按键引脚 2*2
// #define GPIO_PIN_LINES (GPIO_Pin_3 | GPIO_Pin_4)
#define GPIO_PIN_LINES (GPIO_Pin_15 | GPIO_Pin_14 | GPIO_Pin_13)
#define GPIO_LINE (GPIOB)

// #define GPIO_PIN_COLS (GPIO_Pin_13 | GPIO_Pin_14)
#define GPIO_PIN_COLS (GPIO_Pin_12 | GPIO_Pin_11)
#define GPIO_COL (GPIOB)

// #define LINE_INPUT_MASK ((uint16_t)0x0018) // 行线掩码
// #define COL_INPUT_MASK ((uint16_t)0x6000)  // 列线掩码
#define LINE_INPUT_MASK ((uint16_t)0xE000) // 行线掩码
#define COL_INPUT_MASK ((uint16_t)0x1800)  // 列线掩码

#define KeyBoard_KeyLines_Read() (GPIO_ReadInputData(GPIO_LINE) & LINE_INPUT_MASK) // 读取行线的输入状态
#define KeyBoard_Keycolumns_Read() (GPIO_ReadInputData(GPIO_COL) & COL_INPUT_MASK) // 读取列线的输入状态

static void KeyBoard_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
}

/*------------------------------------------------------------------------------------
*  功能:  设置列输出和行输入
*  参数传递：
*  输入：  无
*  返回值: 无
--------------------------------------------------------------------------------------*/
static inline void KeyBoard_KeyLines_ReInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// 设置行输出
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LINES;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIO_LINE, &GPIO_InitStructure);

	// 设置列输入
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_COLS;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIO_COL, &GPIO_InitStructure);

	GPIO_SetBits(GPIO_LINE, GPIO_PIN_LINES); // 行输出高电平
}

/*------------------------------------------------------------------------------------
*  功能:  设置行输出和列输入
*  参数传递：
*  输入：  无
*  返回值: 无
--------------------------------------------------------------------------------------*/
static inline void KeyBoard_Keycolumns_ReInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// 设置行输入
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LINES;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIO_LINE, &GPIO_InitStructure);

	// 设置列输出
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_COLS;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIO_COL, &GPIO_InitStructure);

	// 列输出高电平
	GPIO_SetBits(GPIO_COL, GPIO_PIN_COLS);
}

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
	KeyBoard_KeyLines_ReInit();

	s_Btn.FilterTime = BUTTON_FILTER_TIME;	 /* 按键滤波时间 */
	s_Btn.LongTime = BUTTON_LONG_TIME;		 /* 长按时间 */
	s_Btn.State = 0;						 /* 按键缺省状态，0为未按下*/
	s_Btn.KeyCodeShortDown = KEY_Short_DOWN; /* 按键按下的键值代码 */
	s_Btn.KeyCodeShortUp = KEY_Short_UP;	 /* 按键弹起的键值代码 */
	s_Btn.KeyCodeLongDown = KEY_LONG_DOWN;	 /* 按键被持续按下的键值代码*/
	s_Btn.KeyCodeLongUp = KEY_LONG_UP;

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE); // 关闭SW调试映射
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

/*
*********************************************************************************************************
*	函 数 名: bsp_KeyPro
*	功能说明: 检测所有按键。非阻塞状态，必须被周期性的调用。
*	形    参：无
*	返 回 值: 无
**********************************************************************************************************/
uint16_t bsp_KeyPro(void)
{
	return bsp_DetectButton(&s_Btn); /*  键 */
}
