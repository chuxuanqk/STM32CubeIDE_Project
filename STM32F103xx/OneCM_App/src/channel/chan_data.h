/*
 * chan_data.h
 *
 *  Created on: 2020年1月3日
 *      Author: Fox
 */

//根据onecm协议，规定数据流层通讯协议
#ifndef SRC_CHANNEL_CHAN_DATA_H_
#define SRC_CHANNEL_CHAN_DATA_H_

#include "stm32f10x.h"
#include "config_device/config_par.h"

#define Send_Rec_Buffer_Len 109
#define Send_Rec_Buffer_Len_More 8 // CAN接收一次最多8个字节，避免内存溢出增加8个冗余位
//通讯数据流处理结构体
typedef struct
{
    uint8_t finish_flag;
    uint8_t data_bufer[Send_Rec_Buffer_Len + Send_Rec_Buffer_Len_More];
    uint8_t count_use_time_en;
    uint8_t count_use_time;
    uint16_t data_len;
    uint16_t temp_ptr;
    uint16_t counter_01ms;
    uint16_t mac_id;
    uint16_t group_net_id;
    uint8_t module_port;
} Send_Rec_Data_Structure;

#define Bus_Source_Net_ID 0 //源NETID偏移地址
#define Bus_Target_Net_ID 2 //目标NETID偏移地址
#define Bus_Data_Len 4      //接收、发送数据长度

#define Host_Net_ID 0x0001 //主机网络地址

// chan处理
// 1.初始化数据流通道硬件，尽量采用中断接收+DMA发送方式,收发非阻塞
// 2.实现接收终端函数(接受过滤器)、接收超时、发送超时处理
// 3.若需要快速处理大量通道数据，使用通道缓冲方式
// 4.通过信号量，处理接收数据（非实时处理）
// 5.通过功能码分类，发送处理后的结果
// 6.硬件实现发送流处理，DMA/中断/阻塞方式

char Chan_Init(void); //初始化通道

char Chan_Rec_Int_Callback(Send_Rec_Data_Structure *rec_chan, void *); //接收终端回调，需要根据具体硬件实现
char Chan_Rec_Callback(Send_Rec_Data_Structure *send_chan,
                       Send_Rec_Data_Structure *rec_chan); //通道接收数据流处理回调
char Chan_Rec_Callback_With_Buffers(Send_Rec_Data_Structure *send_chan,
                                    Send_Rec_Data_Structure rec_chan[]); //带接收缓冲区的接收处理
char Chan_Timer_Callback(void);                                          //实时更新通讯数据流状态，用于实现超时复位控制，需要根据具体硬件实现

char Chan_Send_Msg(uint16_t function_num,
                   Send_Rec_Data_Structure *send_chan,
                   Send_Rec_Data_Structure *rec_chan); //通道发送数据流处理

char Chan_Send_By_Device(Send_Rec_Data_Structure *send_chan); //发送数据，需要根据具体硬件实现，可通过DMA/中断/阻塞方式发送

#endif /* SRC_CHANNEL_CHAN_DATA_H_ */
