/*
 * board.c
 *
 *  Created on: Sep 11, 2020
 *      Author: Administrator
 */
#include "board.h"

#include "misc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_flash.h"

union STM32_ID_12Byte STM32_ID;
Config_Data_Str Config_Data;

void Write_Config_To_Flash(Config_Data_Str *config)
{
    u16 i;
    u8 *data_ptr = (u8 *)config;
    u32 flash_addr = CONFIG_DATA_FLASH_ADDR;
    FLASH_Unlock();                                                                            /* 每次擦除Flash中数据时得先解锁 */
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); //清除标记
    for (i = 0; i < (sizeof(Config_Data_Str) / FLASH_Page_SIZE + 1); i++)
        while (FLASH_COMPLETE != FLASH_ErasePage(CONFIG_DATA_FLASH_ADDR + i * FLASH_Page_SIZE))
            ;

    for (i = 0; i < sizeof(Config_Data_Str); i++)
    {
        while (FLASH_COMPLETE != FLASH_ProgramHalfWord(flash_addr, (data_ptr[i] + data_ptr[i + 1] * 256)))
            ;
        i++;
        flash_addr += 2;
    }
}

void Config_Data_ReInit(Config_Data_Str *config)
{
    u16 i;
    u8 *temp_ptr = (u8 *)config;
    //清零所有参数
    for (i = 0; i < sizeof(Config_Data_Str); i++)
    {
        temp_ptr[i] = 0;
    }
    //设置默认值
    config->save_data_flag = 1;

    // Dev_Config_Reinit(&(config->dev_con));
    // Dev_Config_Reinit((config->dev_con));
};

void Read_Config_Form_Flash(Config_Data_Str *config)
{
    u16 i;
    u8 *data_ptr = (u8 *)config;
    u16 tmp_read;
    u32 flash_addr = CONFIG_DATA_FLASH_ADDR;
    for (i = 0; i < sizeof(Config_Data_Str); i++)
    {
        tmp_read = *(vu16 *)(flash_addr);
        data_ptr[i] = tmp_read % 256;
        i++;
        if (i < sizeof(Config_Data_Str))
            data_ptr[i] = tmp_read / 256;

        flash_addr += 2;
    }
    //判断是否为第一次启动
    if (config->save_data_flag != 1)
    {
        Config_Data_ReInit(config);    //重置所有参数
        Write_Config_To_Flash(config); //回写参数到flash
    }
}

//读取ChipID
void Cpu_GetId(void)
{
    STM32_ID.id_u32[0] = *(__IO u32 *)(0x1FFFF7E8); //产品唯一身份标识寄存器(96位)
    STM32_ID.id_u32[1] = *(__IO u32 *)(0x1FFFF7EC);
    STM32_ID.id_u32[2] = *(__IO u32 *)(0x1FFFF7F0);
}

void RCC_Configuration(void)
{
    ErrorStatus HSEStartUpStatus;

    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
    HSEStartUpStatus = RCC_WaitForHSEStartUp();
    if (HSEStartUpStatus == SUCCESS)
    {
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
        RCC_PLLCmd(ENABLE);
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY == RESET))
        {
        }
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        RCC_PCLK1Config(RCC_HCLK_Div2);
        RCC_PCLK2Config(RCC_HCLK_Div1); //串口波特率的确定

        while (RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }
}

void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
#ifdef VETB_TAB_RAM
    NVYC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
    /* Configure one bit for preemption priority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    /* Enable the USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;         //通道设置为串口1中断（故后面应选择在“void USART1_IRQHandler(void)”开中断）
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //中断占先等级0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        //中断响应优先级0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           //打开中断
    NVIC_Init(&NVIC_InitStructure);

    //DMA发送中断设置
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Enable CAN1 RX0 interrupt IRQ channel */
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Enable CAN1 TX0 interrupt IRQ channel */
    /*
  NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	*/

    /* Enable the USART2 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;         //通道设置为串口1中断（故后面应选择在“void USART1_IRQHandler(void)”开中断）
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //中断占先等级0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;        //中断响应优先级0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           //打开中断
    NVIC_Init(&NVIC_InitStructure);

    //DMA发送中断设置
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Enable the USART3 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;         //通道设置为串口1中断（故后面应选择在“void USART1_IRQHandler(void)”开中断）
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //中断占先等级0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;        //中断响应优先级0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           //打开中断
    NVIC_Init(&NVIC_InitStructure);

    //DMA发送中断设置
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //窗口看门狗中断
    //    NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn;
    //    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    //    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    //    NVIC_Init(&NVIC_InitStructure);
}

void hw_board_init(void)
{
    RCC_Configuration();  // 初始化时钟
    NVIC_Configuration(); // 初始化向量中断

    //启动系统时钟信号，设置时间为1ms
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); //AHB=72M/8=9M
    SysTick_Config(SYSTICK_TICK);

    Cpu_GetId();                          //网关物理地址
    Read_Config_Form_Flash(&Config_Data); //载入保存的25个Net_ID
}
