/*
 * chan_data_uart3.c
 *
 *  Created on: 2020年1月5日
 *      Author: Fox
 */

/*******************************************************
 * 接收不定长数据
 * 1.发送后开启接收，接收等待时间为APP设置的时间
 * 2.每次接收一个字节，重置计数器，此时计数为2个字节时间
 * 3.超过2个字节时间，完成接收，如果超时一个字节都没有受到，标记为通讯失败
 * *****************************************************/

// #include "channel/bsp_uart.h"
#include "third_protocol_dev/chan_data_uart3.h"
#include "virtual_device/state_led.h"

#include <stdio.h>
#include "misc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_dma.h"

#define USING_USART3_485
#define _485_GPIOx (GPIOB)
#define _485_PINx (GPIO_Pin_12)
#define Set_RS485_Recv() GPIO_ResetBits(_485_GPIOx, _485_PINx)
#define Set_RS485_Send() GPIO_SetBits(_485_GPIOx, _485_PINx)

Send_Rec_Data_Structure USART3_Receive;
Send_Rec_Data_Structure USART3_Send;
uint8_t RECV_LENGTH = 5;     // 默认接收数据长度
uint16_t Recv_OverTime = 10; // 接收超时时间10ms
uint16_t Recv_AckTime = 100; // 指令应答时间

static void USART3_Configuration(void)
{
    USART_InitTypeDef USART_InitStructure;
    USART_ClockInitTypeDef USART_ClockInitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); //RCC中打开相应的串口
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //RCC中打开相应的串口
    //USART3 GPIO初始化
    //GPIO_PinRemapConfig(GPIO_Remap_USART3, ENABLE);      //改变指定管脚脚的映射 Changes the mapping of the specified pin
    /* Configure USART3 RTS (PA12) and USART3 Tx (PA9) as alternate function push-pull 根据资料可查得各管脚对应*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* Configure USART3 CTS (PA11) and USART3 Rx (PA10) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

#ifdef USING_USART3_485
    /* Configure USART3 RS485_Send_Enable*/
    GPIO_InitStructure.GPIO_Pin = _485_PINx;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(_485_GPIOx, &GPIO_InitStructure);
    Set_RS485_Recv(); //复位USART3485管脚, 使能接收
#endif

    /*串口传输速率的大小必须与RCC所设定的时钟相对应起来*/
    USART_InitStructure.USART_BaudRate = USART3_Base_BaudRate; //设置USART3的传输速率
    /*设定数据的接收发送模式*/
    USART_InitStructure.USART_StopBits = USART_StopBits_1; //定义在帧的结尾传输一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;    //无校验
    if (USART_InitStructure.USART_Parity != USART_Parity_No)
        USART_InitStructure.USART_WordLength = USART_WordLength_9b; //奇偶校验共9bit
    else
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;                 //无奇偶校验8bit
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //指定硬件流控制模式RTS和CTS使能
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 //指定使能或失能发送和接受模式 Tx发送使能和Rx接收使能

    USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;     //提升USART时钟时使能还是失能，钟低电平活动
    USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;           //指定SLCK引脚上时钟的极性
    USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;         //时钟第二个边缘进行数据捕获
    USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable; //在SCLK引脚上输出最后发送的那个数据字的脉冲不从SCLK输出

    USART_ClockInit(USART3, &USART_ClockInitStructure);
    USART_Init(USART3, &USART_InitStructure);
    /*输入输出的中断使能*/
    // USART_ITConfig(USART3, USART_IT_IDLE, ENABLE); // 开启串口空闲IDEL中断
    USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE); //串口采用DMA发送
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); //接收中断使能

    USART_Cmd(USART3, ENABLE); //启动串口 使能USART3外设
}

static void USART3_Send_DMA1_Channel2(uint8_t *send_buffer)
{
    DMA_InitTypeDef DMA_InitStructure;

    //启动DMA时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    //DMA1通道4配置
    DMA_DeInit(DMA1_Channel2);
    //外设地址
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART3->DR);
    //内存地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)send_buffer;
    //dma传输方向， 从内存到外设
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    //设置DMA在传输时缓冲区的长度
    DMA_InitStructure.DMA_BufferSize = Send_Rec_Buffer_Len;
    //设置DMA的外设递增模式，一个外设
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    //设置DMA的内存递增模式
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    //外设数据字长
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    //内存数据字长
    DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
    //设置DMA的传输模式
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    //设置DMA的优先级别
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    //设置DMA的2个memory中的变量互相访问
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA1_Channel2, &DMA_InitStructure);
    // 清除DMA所有标志
    DMA_ClearFlag(DMA1_FLAG_GL2); // 清除DMA所有标志
    DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);
}

static void USART3_Recv_DMA1_Channel3(uint8_t *send_buffer)
{
    DMA_InitTypeDef DMA_InitStructure;

    // 开启DMA时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    // 设置DMA源地址：串口数据寄存器地址*/
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(USART3->DR);
    // 内存地址(要传输的变量的指针)
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)send_buffer;
    // 方向：从外设到内存
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    // 传输大小
    DMA_InitStructure.DMA_BufferSize = Send_Rec_Buffer_Len;
    // 外设地址不增
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    // 内存地址自增
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    // 外设数据单位
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    // 内存数据单位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    // DMA模式，一次或者循环模式
    //DMA_InitStructure.DMA_Mode = DMA_Mode_Normal ;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    // 优先级：中
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    // 禁止内存到内存的传输
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    // 配置DMA通道
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);
    // 清除DMA所有标志
    DMA_ClearFlag(DMA1_FLAG_TC5);
    DMA_ITConfig(DMA1_Channel3, DMA_IT_TE, ENABLE);
    // 使能DMA
    DMA_Cmd(DMA1_Channel3, ENABLE);
}

static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

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

    return;
}

/**
 * @description: 设置接收超时时间
 * @param {overtime} 
 * @return {*}
 */
inline void Uart3_Set_Recv_OverTime(uint16_t baurdrate)
{
    Recv_OverTime = (10000 / baurdrate) + 2;
}

inline void Uart3_Set_CmdAckTime(uint16_t AckTime)
{
    if (AckTime > 40)
        Recv_AckTime = AckTime;
}

/**
 * @description: 
 * @param {Send_Rec_Data_Structure} *USART_Send
 * @return {*}
 */
void Uart3_Send_Str(Send_Rec_Data_Structure *USART_Send)
{
    USART_Send->finish_flag = 1;

#ifdef USING_USART3_485
    Set_RS485_Send();
#endif
    DMA_ClearFlag(DMA1_FLAG_GL2);
    DMA_Cmd(DMA1_Channel2, DISABLE);
    DMA_SetCurrDataCounter(DMA1_Channel2, USART_Send->data_len); //设置数据长度

    DMA_Cmd(DMA1_Channel2, ENABLE); //启动DMA发送

    return;
}

void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) //判断发生接收中断
    {
        USART3_Receive.data_bufer[USART3_Receive.temp_ptr] = USART_ReceiveData(USART3); //接收数据，存入串口缓冲区

        if (USART3_Receive.temp_ptr == 0) //判断是否开始接收，启动超时接收
        {
            USART3_Receive.counter_01ms = 0;      //复位超时计时器
            USART3_Receive.count_use_time_en = 2; //开启接收超时计时
        }
        else
        {
            USART3_Receive.counter_01ms = 0; //复位超时计时器
        }

        USART3_Receive.temp_ptr++;
        if (USART3_Receive.temp_ptr > (Send_Rec_Buffer_Len - 1)) // 接收到指定长度过长，丢弃
        {
            USART3_Receive.temp_ptr = 0;          //复位缓冲区指针
            USART3_Receive.count_use_time_en = 0; //停止超时计时
            USART3_Receive.counter_01ms = 0;      //复位超时计时器
            USART3_Receive.finish_flag = 0;       //复位接收标志位
        }
        // else if (USART3_Receive.temp_ptr > (RECV_LENGTH - 1)) // 接收到默认长度
        // {
        //     USART3_Receive.data_len = USART3_Receive.temp_ptr; // 更新数据包长度
        //     USART3_Receive.finish_flag = 1;                    // 通知上层函数处理
        // }

        USART_ClearITPendingBit(USART3, USART_IT_RXNE); //清除中断标志
    }

    if (USART_GetITStatus(USART3, USART_IT_TC) != RESET)
    {
#ifdef USING_USART3_485
        Set_RS485_Recv();
#endif
        USART3_Send.finish_flag = 0;          //允许再次发送
        USART3_Send.temp_ptr = 0;             //复位串口缓冲区指
        USART3_Receive.counter_01ms = 0;      //复位超时计时器
        USART3_Receive.count_use_time_en = 1; //开启ACK超时计时

        USART_ClearITPendingBit(USART3, USART_IT_TC);
    }

    return;
}

//串口3 DMA中断，在发送最后2个字节触发该中断
void DMA1_Channel2_IRQHandler(void)
{
    // USART3_Send.count_use_time_en = 1; //发送完成产生一个时间间隔

    DMA_ClearFlag(DMA1_FLAG_TC2);
    DMA_Cmd(DMA1_Channel2, DISABLE);
    USART_ClearITPendingBit(USART3, USART_IT_TC);
    USART_ITConfig(USART3, USART_IT_TC, ENABLE); // 开启串口发送一帧数据完成中断
}

inline Send_Rec_Data_Structure *Get_Uart3_Send_Str(void)
{
    return &USART3_Send;
}

inline Send_Rec_Data_Structure *Get_Uart3_Recv_Str(void)
{
    return &USART3_Receive;
}

char Uart3_Err_Timer(void)
{
    if (USART3_Receive.count_use_time_en == 1) //串口3 ACK超时允许
    {
        USART3_Receive.counter_01ms++;

        // if (USART3_Receive.counter_01ms > (1 * 10 * 1000 * 130 / USART3_Base_BaudRate + 1)) //发送后延时130ms
        if (USART3_Receive.counter_01ms > Recv_AckTime) // 默认接收超时100ms
        {
            USART3_Receive.data_len = USART3_Receive.temp_ptr;
            USART3_Receive.temp_ptr = 0;          //复位串口缓冲区指
            USART3_Receive.counter_01ms = 0;      //复位0.1ms计数器
            USART3_Receive.count_use_time_en = 0; //复位接收超时标志位
            USART3_Receive.finish_flag = 1;       //置位接收完成标志位
        }
    }
    else if (USART3_Receive.count_use_time_en == 2) // 接收超时
    {
        USART3_Receive.counter_01ms++;

        if (USART3_Receive.counter_01ms > Recv_OverTime) // 超过10ms未接收到字节则判断为接收完成
        {
            USART3_Receive.data_len = USART3_Receive.temp_ptr;
            USART3_Receive.temp_ptr = 0;          //复位串口缓冲区指
            USART3_Receive.counter_01ms = 0;      //复位0.1ms计数器
            USART3_Receive.count_use_time_en = 0; //复位接收超时标志位
            USART3_Receive.finish_flag = 1;       //置位接收完成标志位
        }
    }

    // if (USART3_Send.count_use_time_en == 1) //串口3发送超时允许
    // {
    //     USART3_Send.counter_01ms++;
    //     if (USART3_Send.counter_01ms > (1 * 10 * 1000 * 2 / USART3_Base_BaudRate + 1)) //串口DMA发送最后2字节时间
    //     {
    //         USART3_Send.counter_01ms = 0;      //复位0.1ms计数器
    //         USART3_Send.temp_ptr = 0;          //复位串口缓冲区指
    //         USART3_Send.count_use_time_en = 0; //复位接收超时标志位

    //         USART3_Receive.count_use_time_en = 1; //置位接收超时计数器，等待设备应答
    //     }
    // }
}

/**
 * @description: 设置接收数据的长度 
 * @param {*}
 * @return {*}
 */
void Set_RecvLength(uint8_t rcl)
{
    RECV_LENGTH = rcl;
    return;
}

/**
 * @description: 
 * @param {*}
 * @return {*}
 */
void Uart3_Rec_Contorl(void)
{
    uint16_t i = 0;
    uint8_t rec_err = 0;

    if (USART3_Receive.finish_flag == 1)
    {
        USART3_Receive.count_use_time_en = 0; //复位接收超时标志位
        USART3_Receive.counter_01ms = 0;      //复位超时接收计数器
        USART3_Receive.module_port = 0;       //复位串口端口
        USART3_Receive.finish_flag = 0;       //复位标志位
        USART3_Receive.temp_ptr = 0;          //复位缓冲区指针

        if (USART3_Receive.data_len == 0) //通讯超时，标记错误
        {
            rec_err = 1;
            Uart3_Rec_Data_Proc(rec_err, &USART3_Receive);
        }
        else
        {
            rec_err = 0;
            Uart3_Rec_Data_Proc(rec_err, &USART3_Receive);
        }

        //复位串口接收相关参数

        USART3_Receive.data_len = 0;
    }

    return;
}

/**
 * @description: 
 * @param {*}
 * @return {*}
 */
void Uart3_Send_Meg(uint8_t func_num, Send_Rec_Data_Structure *uart_send)
{
    uint8_t i = 0;
    uint8_t send_buf[30] = {0};

    i = Uart3_Send_Data_Proc(func_num, send_buf, uart_send->module_port);

    if (i > 0)
    {
        uart_send->data_len = i;

        for (i = 0; i < uart_send->data_len; i++)
        {
            uart_send->data_bufer[i] = send_buf[i];
        }

        Uart3_Send_Str(uart_send);
    }

    return;
}

void Uart3_Bus_Init(void)
{
    USART3_Configuration();
    NVIC_Configuration();
    USART3_Send_DMA1_Channel2(USART3_Send.data_bufer);

    USART3_Receive.data_len = RECV_LENGTH;
};

/********************外部实现*******************************/
void __attribute__((weak)) Uart3_Rec_Data_Proc(uint8_t rec_err, Send_Rec_Data_Structure *uart_recv) { return; }
uint8_t __attribute__((weak)) Uart3_Send_Data_Proc(uint8_t func_num, uint8_t *psend, uint8_t chan_num) { return 0; }
