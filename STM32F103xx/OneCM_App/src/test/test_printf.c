/*
 * test_printf.c
 *
 *  Created on: 2020年11月25日
 *      Author: Administrator
 */

// #include "channel/bsp_uart.h"
// #include "third_protocol_dev/chan_data_uart3.h"
// #include "test/test_printf.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define Product_Type "123412341234123412341234123412341234" // 36字符长度的产品型号
#define Product_Mac "432143214321432143214321"              // 26字符长度的产品MAC
#define Product_Firmware_Version "1.1.0"                    // 产品固件版本
#define Product_UpdataPakge_Version "1.1.0"                 // 产品更新包版本
#define Product_Protoc0l_Version "1.1.0"                    // 协议版本
#define Product_Hardware_Version "1.1.0"                    // 硬件版本
#define Product_Dev_Version 0x1234                          // 设备类型
#define Product_Subdev_Version 0x4321                       // 设备子类型

/*
uint8_t recv_buf[30];

void Test_RS485(void)
{
    int8_t size = -1;
    uint8_t recv_len = 5;
    uart_SetRead_Size(USART1, recv_len);

    if (uart_getFlagStatus(USART1, UART_FLAG_RC))
    {
        Send_Rec_Data_Structure *Send_Rs485 = Get_Uart3_Send_Str();
        size = uart_read(USART1, recv_buf, recv_len);
        uart_send(USART1, recv_buf, size);

        Send_Rs485->data_len = size;
        memset(Send_Rs485->data_bufer, 0, sizeof(Send_Rs485->data_bufer));
        memcpy(Send_Rs485->data_bufer, recv_buf, size);
        Uart3_Send_Str(Send_Rs485);
        Set_RecvLength(recv_len);
    }
}


void Uart3_Rec_Data_Proc(uint8_t rec_err, Send_Rec_Data_Structure *uart_recv)
{
    uint8_t size = uart_recv->data_len;
    static uint8_t err_count = 0;
    Send_Rec_Data_Structure *Send_Rs485 = Get_Uart3_Send_Str();

    uart_recv->finish_flag = 0;
    uart_recv->temp_ptr = 0;

    // memset(recv_buf, 0, sizeof(recv_buf));
    // memcpy(recv_buf, uart_recv->data_bufer, size);
    // uart_write(USART1, recv_buf, size);

    if (rec_err == 1)
    {
        err_count++;
    }
    else
    {
        Send_Rs485->data_len = size;
        memcpy(Send_Rs485->data_bufer, uart_recv->data_bufer, size);
        Uart3_Send_Str(Send_Rs485);
    }
}
*/


/*
int main(int argc, char const *argv[])
{

    char msg[200] = {'\0'};
    // size_t slen = sprintf(msg, "{\"f\":%d,\"md\":\"%s\",\"mac\":\"%s\",\"sv\":\"%s\",\"uv\":\"%s\",\"pv\":\"%s\",\"hv\":\"%s\",\"ft\":%d,\"st\":%d}",
    //                       2, "123456", "4321", "1.1.0", "1.1.0", "1.1.0", "1.1.0", 0x1234, 0x4321);
    size_t slen = sprintf(msg, "{\"f\":%d,\"md\":\"%s\",\"mac\":\"%s\",\"sv\":\"%s\",\"uv\":\"%s\",\"pv\":\"%s\",\"hv\":\"%s\",\"ft\":%d,\"st\":%d}",
                          2,
                          Product_Type,
                          Product_Mac,
                          Product_Firmware_Version,
                          Product_UpdataPakge_Version,
                          Product_Protoc0l_Version,
                          Product_Hardware_Version,
                          Product_Dev_Version,
                          Product_Subdev_Version);

    printf("%d\r\n", '\0');
    printf("slen: %d\r\n", slen);
    printf("msg: %s\r\n", msg);

    return 0;
}*/
