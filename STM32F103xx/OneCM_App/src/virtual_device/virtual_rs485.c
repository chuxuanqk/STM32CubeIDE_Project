/*
 * virtual_rs485.c
 *
 *  Created on: 2020年12月21日
 *      Author: Saber
 */

#include "channel/chan_data_can1.h"
#include "protocol/modbus_port_adjust.h"
#include "virtual_device/virtual_rs485.h"
#include "channel/bsp_uart.h"

#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
#include <string.h>

RS485_Cmd rs485cmd_tmp;
RS485_Reg rs485_tmp;

/**
 * @description: 
 * @param {uint8_t} *recbuf
 * @param {uint8_t} temp_port
 * @return {*}
 */
uint8_t ModbusRs485_Rec_Change_Port(uint8_t *recbuf, uint8_t temp_port)
{
    uint8_t ret = 1;
    uint8_t *src = &recbuf[8];
    uint8_t *des = rs485_tmp.cmd->cmd_buf;
    rs485_tmp.cmd->cmd_id = recbuf[7];
    rs485_tmp.cmd->cmd_len = recbuf[4] - 3; /* 指令长度 */
    rs485_tmp.cmd->cmd_err = 0;

    memset(des, 0, CMD_MAX_SIZE);
    memcpy(des, src, rs485_tmp.cmd->cmd_len);

    rs485_tmp.cmd->cmd_flag = 1; // 设置发送flag

    return ret;
}

/**
 * @description: 
 * @param {uint8_t} *recbuf
 * @param {uint8_t} temp_port
 * @return {*}
 */
uint8_t ModbusRs485_Rec_Set_Config(uint8_t *recbuf, uint8_t temp_port)
{
    uint8_t ret = 1;
    uint8_t *src = &recbuf[7];
    uint8_t *des = rs485_tmp.dev_cfg;
    uint8_t len = recbuf[4] - 2;

    memcpy(des, src, len);

    uint16_t AckTime = (rs485_tmp.dev_cfg->delay_time_H << 8) +
                       rs485_tmp.dev_cfg->delay_time_L;
    Uart3_Set_CmdAckTime(AckTime);
    Configuration_Rs485_Change(rs485_tmp.dev_cfg);

    return ret;
}

/**
 * @description: 
 * @param {uint8_t} *recbuf
 * @param {uint8_t} temp_port
 * @return {*}
 */
uint8_t ModbusRs485_Send_Read_Port(uint8_t *sendbuf, uint8_t temp_port)
{
    uint8_t ret = 1;
    uint8_t *src = rs485_tmp.cmd->cmd_buf;
    uint8_t *des = &sendbuf[9];
    sendbuf[4] = rs485_tmp.cmd->cmd_len + 4;
    sendbuf[7] = rs485_tmp.cmd->cmd_id;
    sendbuf[8] = rs485_tmp.cmd->cmd_err;

    memcpy(des, src, rs485_tmp.cmd->cmd_len);

    return ret;
}

/**
 * @description: 
 * @param {uint8_t} *sendbuf
 * @param {uint8_t} temp_port
 * @return {*}
 */
uint8_t ModbusRs485_Send_Read_Config(uint8_t *sendbuf, uint8_t temp_port)
{
    uint8_t ret = 1;
    uint8_t *src = rs485_tmp.dev_cfg;
    uint8_t *des = &sendbuf[7];
    uint8_t len = sizeof(device_config);
    sendbuf[4] = len + 2;

    memcpy(des, src, len);

    return ret;
}

/**
 * @description: 
 * @param {uint8_t} *sendbuf
 * @param {uint8_t} temp_port
 * @return {*}
 */
uint8_t ModbusRs485_Send_Read_ReplyMsg(uint8_t *sendbuf, uint8_t temp_port)
{
    uint8_t ret = 1;
    uint8_t *src = rs485_tmp.cmd->ReplyMsg;
    uint8_t *des = &sendbuf[9];
    sendbuf[4] = rs485_tmp.cmd->ReplyMsg_len + 4;
    sendbuf[7] = rs485_tmp.cmd->cmd_id;
    sendbuf[8] = rs485_tmp.cmd->cmd_err;

    memcpy(des, src, rs485_tmp.cmd->ReplyMsg_len);

    return ret;
}

/**
 * @description: 
 * @param {uint8_t} *sendbuf
 * @param {uint8_t} temp_port
 * @return {*}
 */
uint8_t ModbusRs485_Send_ReplyMsg(uint8_t *sendbuf, uint8_t temp_port)
{
    uint8_t ret = 1;
    uint8_t *src = rs485_tmp.cmd->ReplyMsg;
    uint8_t *des = &sendbuf[9];
    sendbuf[4] = rs485_tmp.cmd->ReplyMsg_len + 4;
    sendbuf[7] = rs485_tmp.cmd->cmd_id;
    sendbuf[8] = rs485_tmp.cmd->cmd_err;

    memcpy(des, src, rs485_tmp.cmd->ReplyMsg_len);

    return ret;
}

/***********************************************************************************************/
/**
 * @description: 修改串口相关配置
 * @param {*}
 * @return {*}
 */
void Configuration_Rs485_Change(device_config *dev_cfg)
{
    USART_InitTypeDef USART_InitStructure;
    uint32_t baudRate = (dev_cfg->baud_rate_HH << 24) + (dev_cfg->baud_rate_HL << 16) +
                        (dev_cfg->baud_rate_LH << 8) + dev_cfg->baud_rate_LL;

    // USART_DeInit(USART3);
    // RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART3, ENABLE);
    /*串口传输速率的大小必须与RCC所设定的时钟相对应起来*/
    USART_InitStructure.USART_BaudRate = baudRate; //设置USART3的传输速率
    /*设定数据的接收发送模式*/
    USART_InitStructure.USART_StopBits = dev_cfg->stop_bit; //定义在帧的结尾传输一个停止位
    USART_InitStructure.USART_Parity = dev_cfg->parity_bit; //无校验
    if (USART_InitStructure.USART_Parity != USART_Parity_No)
        USART_InitStructure.USART_WordLength = USART_WordLength_9b; //奇偶校验共9bit
    else
        USART_InitStructure.USART_WordLength = USART_WordLength_8b; //无奇偶校验8bit

    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 //指定使能或失能发送和接受模式 Tx发送使能和Rx接收使能
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //指定硬件流控制模式RTS和CTS使能

    USART_Init(USART3, &USART_InitStructure);
}

/**
 * @description: 初始化485串口配置
 * @param {*}
 * @return {*}
 */
void Virtual_Rs485_Init(void)
{
    Config_Data_Str *cfg = Get_Config_data();

    rs485_tmp.dev_cfg = &cfg->dev_con;
    rs485_tmp.send = Get_Uart3_Send_Str(); /* 初始化串口3为虚拟485设备 */
    rs485_tmp.recv = Get_Uart3_Recv_Str();
    rs485_tmp.cmd = &rs485cmd_tmp;
    rs485_tmp.cmd->cmd_err = 0;
    memset(rs485_tmp.cmd, 0, sizeof(RS485_Cmd));

    Uart3_Bus_Init();
    Configuration_Rs485_Change(rs485_tmp.dev_cfg);
    uint32_t baudRate = (rs485_tmp.dev_cfg->baud_rate_HH << 24) + (rs485_tmp.dev_cfg->baud_rate_HL << 16) +
                        (rs485_tmp.dev_cfg->baud_rate_LH << 8) + rs485_tmp.dev_cfg->baud_rate_LL;
    Uart3_Set_Recv_OverTime(baudRate);
}

/**
 * @description: 串口3接收回调函数
 * @param {uint8_t} rec_err 错误码
 * @param {Send_Rec_Data_Structure} *uart_recv
 * @return {*}
 */
void Uart3_Rec_Data_Proc(uint8_t rec_err, Send_Rec_Data_Structure *uart_recv)
{
    static uint8_t err_count = 0;

    if ((rec_err == 1) && (rs485_tmp.dev_cfg->response_Signal != 0)) /* 设置应答且没有数据返回 */
    {
        err_count++;
        if (err_count >= 3)
            rs485_tmp.cmd->cmd_err = 255;
        else
            Uart3_Send_Str(rs485_tmp.send); /* 自动重发3次 */
    }
    else
    {
        err_count = 0;
        rs485_tmp.cmd->cmd_err = 0;
        rs485_tmp.cmd->ReplyMsg_len = uart_recv->data_len;
        rs485_tmp.cmd->Reply_flag = 1;
        memset(rs485_tmp.cmd->ReplyMsg, 0, CMD_MAX_SIZE);
        memcpy(rs485_tmp.cmd->ReplyMsg, uart_recv->data_bufer, uart_recv->data_len);
    }
}

/**
 * @description: 数据更新处理
 * @param {*}
 * @return {*}
 */
void Virtual_Rs485_Update(void)
{
    if (rs485_tmp.cmd->cmd_flag == 1) /* 发送指令 */
    {
        rs485_tmp.cmd->Reply_flag = 0;
        rs485_tmp.cmd->cmd_flag = 0;
        rs485_tmp.send->data_len = rs485_tmp.cmd->cmd_len;
        rs485_tmp.send->module_port = rs485_tmp.cmd->cmd_id;
        rs485_tmp.recv->module_port = rs485_tmp.cmd->cmd_id;
        memset(rs485_tmp.send->data_bufer, 0, Send_Rec_Buffer_Len);
        memset(rs485_tmp.recv->data_bufer, 0, Send_Rec_Buffer_Len);
        memcpy(rs485_tmp.send->data_bufer, rs485_tmp.cmd->cmd_buf, rs485_tmp.send->data_len);

        Uart3_Send_Str(rs485_tmp.send);
    }

    if ((rs485_tmp.cmd->Reply_flag != 0) &&
        (rs485_tmp.dev_cfg->response_Signal == 2)) // 配置为有接收且主动发送
    {
        Send_Rec_Data_Structure *can_send = Get_Can1_Send();
        Send_Rec_Data_Structure *can_recv = Get_Can1_Receive();
        CAN1_Send_Meg(CASE_MODULE_SEND_MSG, can_send, can_recv);

        rs485_tmp.cmd->Reply_flag = 0;
    }
}
