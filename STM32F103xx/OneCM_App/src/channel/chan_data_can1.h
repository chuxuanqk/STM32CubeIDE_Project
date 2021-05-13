/*
 * chan_data_can1.h
 *
 *  Created on: 2020年1月3日
 *      Author: Fox 
 */

#ifndef SRC_CHANNEL_CHAN_DATA_CAN1_H_
#define SRC_CHANNEL_CHAN_DATA_CAN1_H_

#include "channel/chan_data.h"

#define CAN_Rec_Base_Len 5  //CAN缓冲区接收固定长度，自动提取源地址
#define CAN_Send_Base_Len 3 //CAN发送缓冲区固定长度，省略冗余源地址

#define USB_LP_CAN1_RX0_Preemption_Priority 0
#define USB_LP_CAN1_RX0_Sub_Priority 3
#define USB_LP_CAN1_TX0_Preemption_Priority 1
#define USB_LP_CAN1_TX0_Sub_Priority 1

extern Send_Rec_Data_Structure CAN1_Receive;
extern Send_Rec_Data_Structure CAN1_Send;

//多通道CAN接收缓冲区
extern Send_Rec_Data_Structure CAN1_Receive_Buf[];

void CAN1_ONECM_Bus_Init(void);
void CAN1_Err_Timer(void); //CAN1 错误处理
Send_Rec_Data_Structure *Get_Can1_Receive(void);
Send_Rec_Data_Structure *Get_Can1_Send(void);
void CAN1_Send_Str(Send_Rec_Data_Structure *CAN1_Send);

#endif /* SRC_CHANNEL_CHAN_DATA_CAN1_H_ */
