/*
 * chan_data_uart3.h
 *
 *  Created on: 2020年1月5日
 *      Author: Fox
 */

#ifndef CHANNEL_CHAN_DATA_UART3_H_
#define CHANNEL_CHAN_DATA_UART3_H_

#include "channel/chan_data.h"
//引用第三方协议并实现功能

#define USART3_Base_BaudRate 115200

void Uart3_Bus_Init(void);
void Uart3_Rec_Contorl(void);
void Uart3_Rec_Data_Proc(uint8_t rec_err, Send_Rec_Data_Structure *uart_recv);
void Uart3_Send_Meg(uint8_t func_num, Send_Rec_Data_Structure *uart_send);
void Uart3_Send_Str(Send_Rec_Data_Structure *USART_Send);
uint8_t Uart3_Send_Data_Proc(uint8_t func_num, uint8_t *psend, uint8_t chan_num);
Send_Rec_Data_Structure *Get_Uart3_Send_Str(void);
Send_Rec_Data_Structure *Get_Uart3_Recv_Str(void);
char Uart3_Err_Timer(void);
void Set_RecvLength(uint8_t rcl);
void Uart3_Set_Recv_OverTime(uint16_t baurdrate);
void Uart3_Set_CmdAckTime(uint16_t AckTime);

// void Configuration_Uart3();

#endif /* CHANNEL_CHAN_DATA_UART3_H_ */
