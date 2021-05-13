/*
 * modbus_func_adjust.h
 *
 *  Created on: 2020年1月3日
 *      Author: Fox
 */

#ifndef PROTOCOL_MODBUS_FUNC_ADJUST_H_
#define PROTOCOL_MODBUS_FUNC_ADJUST_H_

#include "channel/chan_data_can1.h"

/* CAN接收数据处理 */
uint8_t Modbus_Rec_PkgData_IAP(uint8_t *recbuf, uint8_t temp_port, Send_Rec_Data_Structure *CAN1_Send);
uint8_t Modbus_Rec_Set_ID(uint8_t *recbuf, uint8_t temp_port, uint16_t *net_id, uint16_t *group_net_id);
uint8_t Modbus_Rec_Change_Port(uint8_t *recbuf, uint8_t temp_port);
uint8_t Modbus_Rec_Read_Port(uint8_t *recbuf, uint8_t temp_port);
uint8_t Modbus_Rec_Set_Config(uint8_t *recbuf, uint8_t temp_port);
uint8_t Modbus_Rec_Read_Config(uint8_t *recbuf, uint8_t temp_port);
uint8_t Modbus_Rec_Msg_IAP(uint8_t *recbuf, uint8_t temp_port, Send_Rec_Data_Structure *CAN1_Send);

/*CAN发送数据处理*/
uint8_t Modbus_Send_ACK_IAP(uint8_t *sendbuf, uint8_t temp_port);
uint8_t Modbus_Send_Set_ID(uint8_t *sendbuf, uint8_t temp_port);
uint8_t Modbus_Send_Change_Port(uint8_t *sendbuf, uint8_t temp_port);
uint8_t Modbus_Send_Read_Port(uint8_t *sendbuf, uint8_t temp_port);
uint8_t Modbus_Send_Set_Config(uint8_t *sendbuf, uint8_t temp_port);
uint8_t Modbus_Send_Read_Config(uint8_t *sendbuf, uint8_t temp_port);
uint8_t Modebus_Send_Read_ReplyMsg(uint8_t *sendbuf, uint8_t temp_port);
uint8_t Modebus_Send_ReplyMsg(uint8_t *sendbuf, uint8_t temp_port);

#endif /* PROTOCOL_MODBUS_FUNC_ADJUST_H_ */
