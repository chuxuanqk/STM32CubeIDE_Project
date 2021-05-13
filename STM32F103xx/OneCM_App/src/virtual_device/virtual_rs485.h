/*
 * virtual_rs485.h
 *
 *  Created on: 2020年12月21日
 *      Author: Saber
 */
#ifndef VIRTUAL_DEVICE_VIRTUAL_RS485_H_
#define VIRTUAL_DEVICE_VIRTUAL_RS485_H_

#include "config_device/config_par.h"
#include "third_protocol_dev/chan_data_uart3.h"

#define CMD_MAX_SIZE 109

/* 485指令 */
typedef struct
{
    uint8_t cmd_id;
    uint8_t cmd_len;
    uint8_t ReplyMsg_len;
    uint8_t cmd_err;                // 指令错误码
    uint8_t cmd_flag;               // 指令标志位
    uint8_t Reply_flag;             // 回复标志位
    uint8_t ReplyMsg[CMD_MAX_SIZE]; // 返回信息
    uint8_t cmd_buf[CMD_MAX_SIZE];  // 指令
} RS485_Cmd;

/* 虚拟485设备 */
typedef struct
{
    Send_Rec_Data_Structure *send;
    Send_Rec_Data_Structure *recv;
    device_config *dev_cfg;
    RS485_Cmd *cmd;
} RS485_Reg;

void Virtual_Rs485_Init(void);
void Configuration_Rs485_Change(device_config *dev_cfg);
void Virtual_Rs485_Update(void);

uint8_t ModbusRs485_Rec_Change_Port(uint8_t *recbuf, uint8_t temp_port);
uint8_t ModbusRs485_Rec_Set_Config(uint8_t *recbuf, uint8_t temp_port);
uint8_t ModbusRs485_Send_Read_Port(uint8_t *sendbuf, uint8_t temp_port);
uint8_t ModbusRs485_Send_Read_Config(uint8_t *sendbuf, uint8_t temp_port);
uint8_t ModbusRs485_Send_Read_ReplyMsg(uint8_t *sendbuf, uint8_t temp_port);
uint8_t ModbusRs485_Send_ReplyMsg(uint8_t *sendbuf, uint8_t temp_port);

#endif /* VIRTUAL_DEVICE_VIRTUAL_RS485_H_ */
