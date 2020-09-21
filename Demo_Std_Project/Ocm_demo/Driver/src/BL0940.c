/*
 * @Author: your name
 * @Date: 2020-09-18 14:03:37
 * @LastEditTime: 2020-09-21 17:33:03
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

/* BL0940 串口通信选择 */
#define BL0940_SEL_Pin1 GPIO_Pin_1
#define BL0940_SEL_Pin2 GPIO_Pin_2
#define BL0940_SEL_Pin3 GPIO_Pin_3
#define BL0940_SEL_Pin4 GPIO_Pin_4

#define BL0940_SEL_GPIO GPIOA

uint8_t BL0940_CMD_ARR[] = {IDLE_ADDR,
                            I_WAVE_ADDR,
                            V_WAVE_ADDR,
                            CF_CNT_ADDR,
                            ALL_REG_ADDR};

uint8_t BL0940_READ_CMD[] = {READ_HEAD_CMD, ALL_REG_ADDR};

static void
RCC_Configuration(void)
{
}

static void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_InitStructure.GPIO_Pin = BL0940_SEL_Pin1;
    GPIO_Init(BL0940_SEL_GPIO, &GPIO_InitStructure);
}

int8_t Send_BL0940(struct BL0940_CmdTypeDef *cmd)
{
    int8_t ret = -1;

    ret = uart_send(BL0940_UART, cmd->cmd_buf, cmd->cmd_size);

    return ret;
}

int8_t Recv_BL0940(struct BL0940_CmdTypeDef *cmd)
{
    int8_t ret = -1;

    ret = uart_recv(BL0940_UART, cmd->recv_buf, cmd->recv_size);

    return ret;
}

/* BL0940 CS选择控制 */
void __BL0940_UART_CS(struct BL0940_CmdTypeDef *cmd)
{
}

void __BL0940_Start_Timer(struct BL0940_CmdTypeDef *cmd)
{
}

void BL0940_Thread(struct BL0940_CmdTypeDef *cmd)
{
    static uint8_t number = 0;
    uint8_t size; //  size = sizeof(struct BL0940_DataTypeDef);

    if (cmd->send_flag == false)
    {
        cmd->number = number;
        cmd->send_flag = true;
        __BL0940_UART_CS(cmd);
        Send_BL0940(cmd);
    }
}

/**
 * @description: 
 * @param {type} 
 * @return {type} 
 */
void hw_BL0940_init(struct BL0940_CmdTypeDef *cmd)
{
    RCC_Configuration();
    GPIO_Configuration();

    cmd->cmd_buf = BL0940_READ_CMD; // 读取所有参数
    cmd->cmd_size = sizeof(BL0940_READ_CMD) / sizeof(BL0940_READ_CMD[0]);
}
