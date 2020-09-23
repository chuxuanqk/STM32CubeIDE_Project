/*--------------------------------------------------------------------------------------
*   矩阵键盘驱动
*   文件：    keyboard.c
*   编写人:   Luke Zhang
*   描述:     扫描5x5矩阵键盘输入，并返回键值
*   适用范围: 驱动采用ST3.5库编写，适用于STM32F10X系列单片机
*   所用引脚: 
	5X5键盘					bit4  bit3   bit2   bit1   bit0
			，   line		PE4   PE3    PE2    PE1    PE0
			
			bit4  PC4   	SW1   SW2    SW3    SW4    SW5
			bit3  PC3   	SW6   SW7    SW8    SW9    SW10
			bit2  PC2   	SW11  SW12   SW13   SW14   SW15
			bit1  PC1   	SW16  SW17   SW18   SW19   SW20
			bit0  PC0   	SW21  SW22   SW23   SW24   SW25
			   .column
*   编写时间: 2018年11月20日
*   版本：    1.0
--------------------------------------------------------------------------------------*/

#include "keyboard.h"

#define GPIO_PIN_LINES (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3)
#define GPIO_ROW (GPIOE)
#define GPIO_PIN_COL (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3)
#define GPIO_COL (GPIOC)

//初始化
inline void KeyBoard_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOE, ENABLE);
}
/*------------------------------------------------------------------------------------
*  功能:  设置列输出和行输入
*  参数传递：
*  输入：  无
*  返回值: 无
--------------------------------------------------------------------------------------*/

inline void KeyBoard_KeyRow_ReInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// 设置行输出
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LINES;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIO_ROW, &GPIO_InitStructure);

	// 设置列输入
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_COL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIO_COL, &GPIO_InitStructure);

	// 列输出高电平
	GPIO_SetBits(GPIO_COL, GPIO_PIN_COL);
}

/*------------------------------------------------------------------------------------
*  功能:  读行的值
*  参数传递：
*  输入：  无
*  返回值: 无
--------------------------------------------------------------------------------------*/
inline uint16_t KeyBoard_Keycolumns_Read(void)
{
	uint16_t keycolumns;
	keycolumns = GPIO_ReadInputData(GPIO_COL) & 0x0f;
	return keycolumns;
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
	GPIO_Init(GPIO_ROW, &GPIO_InitStructure);

	// 设置列输出
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_COL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIO_COL, &GPIO_InitStructure);

	// 列输出高电平
	GPIO_SetBits(GPIO_COL, GPIO_PIN_COL);
}

/*------------------------------------------------------------------------------------
*  功能:  读列的值
*  参数传递：
*  输入：  无
*  返回值: 无
--------------------------------------------------------------------------------------*/
inline uint16_t KeyBoard_KeyLines_Read(void)
{
	uint16_t keyline;
	keyline = GPIO_ReadInputData(GPIO_ROW) & 0x0f; // 读取行键的状态，16bit
	return keyline;
}

/*------------------------------矩阵键盘扫描--------------------------------------------

* 功能:     扫描矩阵键盘,并返回键值 
* 参数:
*      输入:  无
*      返回:  有键按下返回该键值
*             无键按下时则返回0

--------------------------------------------------------------------------------------*/

/*--------------------------------THE END--------------------------------------------*/
