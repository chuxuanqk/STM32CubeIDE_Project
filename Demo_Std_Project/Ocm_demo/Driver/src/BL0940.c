/*
 * @Author: your name
 * @Date: 2020-09-18 14:03:37
 * @LastEditTime: 2020-09-22 18:03:37
 * @LastEditors: Please set LastEditors
 * @Description: 提供BL0940芯片的数据读取接口
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
#define TIMEOUT 10

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

static void RCC_Configuration(void)
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

/* BL0940 CS选择控制 */
void __BL0940_UART_CS(struct BL0940_CmdTypeDef *cmd)
{
}

/**
 * @description: 传输指令到BL0940
 * @param {type} 
 * @return {type} 
 */
int16_t Send_BL0940(struct BL0940_CmdTypeDef *cmd)
{
    int16_t ret = -1;
    if (cmd->send_flag == false)
    {
        cmd->send_flag = true;
        __BL0940_UART_CS(cmd);

        uart_SetRead_Size(BL0940_UART, cmd->recv_size); // 设置串口需要读取的数据大小
        ret = uart_write(BL0940_UART, cmd->cmd_buf, cmd->cmd_size);
    }
    return ret;
}

/**
 * @description: 接收BL0940传输的数据
 * @param {type} 
 * @return {type} 
 */
int16_t Recv_BL0940(struct BL0940_CmdTypeDef *cmd)
{
    int16_t ret = -1;
    ret = uart_read(BL0940_UART, cmd->recv_buf, cmd->recv_size);

    return ret;
}

/*
void BL0940_Thread(struct BL0940_CmdTypeDef *cmd)
{
    static uint32_t cur_sec_cnt;
    static uint16_t cur_msec_cnt;
    static uint32_t start_sec_cnt;
    static uint16_t start_msec_cnt;

    if (cmd->send_flag == false)
    {
        cmd->send_flag = true;
        __BL0940_UART_CS(cmd);

        uart_SetRead_Size(BL0940_UART, cmd->recv_size); // 设置串口需要读取的数据大小
        uart_write(BL0940_UART, cmd->cmd_buf, cmd->cmd_size);

        cmd->counter_01ms = 0;
        get_sys_uptime(&cur_sec_cnt, &cur_msec_cnt);
        start_sec_cnt = cur_sec_cnt;
        start_msec_cnt = cur_msec_cnt;
    }
    else
    {
        get_sys_uptime(&cur_sec_cnt, &cur_msec_cnt);
        if (cur_sec_cnt == start_sec_cnt)
        {
            cmd->counter_01ms = cur_msec_cnt - start_msec_cnt;
        }
        else
        {
            cmd->counter_01ms = 1000 - start_msec_cnt;
            if (cur_sec_cnt != (start_sec_cnt + 1))
            {
                cmd->counter_01ms += ((cur_sec_cnt - (start_sec_cnt + 1)) * 1000);
            }
            cmd->counter_01ms += cur_msec_cnt;
        }

        if (cmd->counter_01ms >= TIMEOUT)
        {
            cmd->send_flag = false;
            cmd->connent_error++;
        }
        else
        {
            if (uart_read(BL0940_UART, cmd->recv_buf, cmd->recv_size) != -1)
            {
            }
        }
    }
}
*/

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
