/*
 * @Author: your name
 * @Date: 2020-09-18 14:03:37
 * @LastEditTime: 2020-09-19 11:08:15
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \Demo_Std_Project\Ocm_demo\Driver\src\BL0940.c
 */
/*
 * BL0940.c
 *
 *  Created on: 2020年9月18日
 *      Author: Administrator
 */

#include "BL0940.h"
#include "uart.h"

#define BL0940_UART USART3

#define READ_HEAD_CMD (0x58)
#define WRITER_HEAD_CMD (0xA8)

struct cmd_type
{
    uint8_t head;
    uint8_t addr;
};

/**
 * @description: 读取BL0940中寄存器的值, 依赖于串口收发数据
 * @param {type} buf: 存储读取寄存器值的缓存指针
 *               cmd: 指令
 * @return {type} 
 */
int8_t Read_BL0940_Reg(void *buf, uint8_t addr)
{
    int8_t ret = -1;
    int8_t size = 0;
    struct cmd_type cmd = {READ_HEAD_CMD, addr};
    struct cmd_type *pcmd = &cmd;

    switch (addr)
    {
    case I_WAVE_ADDR:
        uart_send(BL0940_UART, (uint8_t *)pcmd, sizeof(struct cmd_type));
        // TODO: 接收超时，1个字节2416us , 3个字节 8ms, 35个字节85ms
        do
        {
            size = uart_recv(BL0940_UART, (uint8_t *)buf, sizeof(struct I_WAVE));
            if (size != -1)
                ret = size;
        } while (true);
        break;

    case V_WAVE_ADDR:
        uart_send(BL0940_UART, (uint8_t *)pcmd, sizeof(struct cmd_type));
        // TODO: 接收超时
        do
        {
            size = uart_recv(BL0940_UART, (uint8_t *)buf, sizeof(struct V_WAVE));
            if (size != -1)
                ret = size;
        } while (true);

        break;

    case CF_CNT_ADDR:
        uart_send(BL0940_UART, (uint8_t *)pcmd, sizeof(struct cmd_type));
        // TODO: 接收超时
        do
        {
            size = uart_recv(BL0940_UART, (uint8_t *)buf, sizeof(struct CF_CNT));
            if (size != -1)
                ret = size;
        } while (true);
        break;

    case ALL_ARG_ADDR:
        uart_send(BL0940_UART, (uint8_t *)pcmd, sizeof(struct cmd_type));
        // TODO: 接收超时
        do
        {
            size = uart_recv(BL0940_UART, (uint8_t *)buf, sizeof(struct BL0940_DataTypeDef));
            if (size != -1)
                ret = size;
        } while (true);

        break;

    default:
        break;
    }

    return ret;
}

/**
 * @description: 
 * @param {type} 
 * @return {type} 
 */
int8_t Write_BL0940_Reg(void *buf, uint8_t addr)
{
    int8_t ret = -1;

    switch (addr)
    {

    default:
        break;
    }

    return ret;
}

/**
 * @description: 
 * @param {type} 
 * @return {type} 
 */
void hw_BL0940_init(void)
{
}