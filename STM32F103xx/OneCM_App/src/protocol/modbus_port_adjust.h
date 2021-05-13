/*
 * modbus_port_adjust.h
 *
 *  Created on: 2020年1月3日
 *      Author: Fox
 */

#ifndef PROTOCOL_MODBUS_PORT_ADJUST_H_
#define PROTOCOL_MODBUS_PORT_ADJUST_H_

#include "protocol/modbus_func_adjust.h"

#define NULL ((void *)0)
#define CASE_FUNCTION 0x05 //功能码在数据的位置

#define CASE_MODULE_IAP 0x00       // 设备程序升级，可广播
#define CASE_MODULE_RESPON 0x01    // 应答总线占用命令，可广播
#define CASE_MODULE_MSG 0x02       // 反馈从机模块命令， 可广播
#define CASE_MODULE_SET_ID 0x03    // 设置网络ID,组播ID
#define CASE_CHANGE_PORT 0x04      // 设备功能，改变输出状
#define CASE_READ_PORT 0x05        // 设备功能，读取输出状态
#define CASE_MODULE_READ_MSG 0x06  // 设备功能，成查询信息
#define CASE_MODULE_SEND_MSG 0x07  // 设备功能，主动发送信息
#define CASE_SET_CONFIG 0x08       // 设置配置信息
#define CASE_READ_CONFIG 0x09      // 读取配置信息
#define CASE_MODULE_JSON_DATA 0x7B // 设备程序升级, 123-->'{'

void CAN1_Rec_Contorl(Send_Rec_Data_Structure *CAN1_Receive, Send_Rec_Data_Structure *CAN1_Send,
                      Send_Rec_Data_Structure *USART1_Send, Send_Rec_Data_Structure *USART3_Send);
void CAN1_Ports_Adjust(void);
void CAN1_Send_Meg(uint16_t function_num, Send_Rec_Data_Structure *CAN1_Send, Send_Rec_Data_Structure *CAN1_Receive);
void CAN1_Send_JsonMsg(uint16_t function_num, Send_Rec_Data_Structure *CAN1_Send);
void CAN1_Send_JsonUpdateMsg(uint16_t function_num, uint32_t pkg_len, Send_Rec_Data_Structure *CAN1_Send);

#endif /* PROTOCOL_MODBUS_PORT_ADJUST_H_ */
