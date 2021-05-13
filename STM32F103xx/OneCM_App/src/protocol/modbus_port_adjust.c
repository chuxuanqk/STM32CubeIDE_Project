/*
 * modbus_port_adjust.c
 *
 *  Created on: 2020年1月3日
 *      Author: Fox  
  */
#include <string.h>
#include <stdio.h>

#include "channel/chan_data_can1.h"
#include "protocol/modbus_port_adjust.h"
#include "virtual_device/state_led.h"
#include "config_device/config_par.h"

#include "stm32f10x.h"

char msg[256] = {'\0'};

//CAN接收处理函数
void CAN1_Rec_Contorl(Send_Rec_Data_Structure *CAN1_Receive, Send_Rec_Data_Structure *CAN1_Send,
                      Send_Rec_Data_Structure *USART1_Send, Send_Rec_Data_Structure *USART3_Send)
{
    uint8_t temp_port = 0;

    if (0 == (CAN1_Receive->data_bufer[Bus_Target_Net_ID] * 256 + CAN1_Receive->data_bufer[Bus_Target_Net_ID + 1]))
    {
        net_led_sharp(); //闪烁网络指示灯
        uint8_t function = CAN1_Receive->data_bufer[CASE_FUNCTION];

        switch (function) //选择功能码
        {
        case CASE_MODULE_IAP:
        {
            break;
        }

        case CASE_MODULE_MSG: //发送设备信息，包括Net_ID,设备类型，设备固件版本，设备物理地址
        {
            if ((CAN1_Send->finish_flag == 0) && (CAN1_Send->count_use_time_en == 0)) //等待CAN1总线释放
            {
                //仅发送整个模块的Net_ID
                CAN1_Send->mac_id = Config_Data.Net_IDs[0];
                CAN1_Send->group_net_id = Config_Data.Group_Net_IDs[0];
                CAN1_Send->module_port = 0;
                CAN1_Send_Meg(CASE_MODULE_MSG, CAN1_Send, CAN1_Receive);

                //复位全局标志位
                CAN1_Receive->mac_id = 0; //解除CAN1的ID过滤
                CAN1_Receive->data_len = CAN_Rec_Base_Len;
                CAN1_Receive->finish_flag = 0;
            }
            break;
        }

        case CASE_MODULE_SET_ID: //接收到修改网络地址命令
        {
            temp_port = CAN1_Receive->data_bufer[27];

            if ((CAN1_Send->finish_flag == 0) && (CAN1_Send->count_use_time_en == 0)) //等待CAN1总线释放
            {

                //接收消息正确，数据处理完成，发送应答
                if (Modbus_Rec_Set_ID(CAN1_Receive->data_bufer, temp_port, Config_Data.Net_IDs, Config_Data.Group_Net_IDs))
                {
                    CAN1_Send->mac_id = Config_Data.Net_IDs[temp_port];
                    CAN1_Send->group_net_id = Config_Data.Group_Net_IDs[temp_port];
                    CAN1_Send->module_port = temp_port;

                    CAN1_Send_Meg(CASE_MODULE_SET_ID, CAN1_Send, CAN1_Receive); //向主机返回状态信息
                }

                //复位全局标志位
                CAN1_Receive->mac_id = 0; //解除CAN1的ID过滤
                CAN1_Receive->data_len = CAN_Rec_Base_Len;
                CAN1_Receive->finish_flag = 0;
            }
            break;
        }

        default:
        {
            //复位全局标志位
            CAN1_Receive->mac_id = 0; //解除CAN1的ID过滤
            CAN1_Receive->data_len = CAN_Rec_Base_Len;
            CAN1_Receive->finish_flag = 0;
            break;
        }
        }
    }
    else
    {
        net_led_sharp(); //闪烁网络指示灯

        temp_port = CAN1_Receive->module_port; // 端口映射, 直接获取接收的端口号

        CAN1_Send->mac_id = Config_Data.Net_IDs[temp_port];
        CAN1_Send->group_net_id = Config_Data.Group_Net_IDs[temp_port];
        CAN1_Send->module_port = temp_port;
        uint8_t function = CAN1_Receive->data_bufer[CASE_FUNCTION];

        switch (function) //选择功能码
        {
        case CASE_MODULE_IAP:
        {
            if ((CAN1_Send->finish_flag == 0) && (CAN1_Send->count_use_time_en == 0)) //等待CAN1总线释放
            {
                Modbus_Rec_PkgData_IAP(CAN1_Receive->data_bufer, temp_port, CAN1_Send);

                //复位全局标志位
                CAN1_Receive->mac_id = 0; //解除CAN1的ID过滤
                CAN1_Receive->data_len = CAN_Rec_Base_Len;
                CAN1_Receive->finish_flag = 0;
            }
            break;
        }

        case CASE_MODULE_RESPON:
        {
            //复位全局标志位
            CAN1_Receive->mac_id = 0; //解除CAN1的ID过滤
            CAN1_Receive->data_len = CAN_Rec_Base_Len;
            CAN1_Receive->finish_flag = 0;
            break;
        }

        case CASE_MODULE_MSG: //发送设备信息，包括Net_ID,设备类型，设备固件版本，设备物理地址
        {

            if ((CAN1_Send->finish_flag == 0) && (CAN1_Send->count_use_time_en == 0)) //等待CAN1总线释放
            {
                CAN1_Send_Meg(CASE_MODULE_MSG, CAN1_Send, CAN1_Receive);

                //复位全局标志位
                CAN1_Receive->mac_id = 0; //解除CAN1的ID过滤
                CAN1_Receive->data_len = CAN_Rec_Base_Len;
                CAN1_Receive->finish_flag = 0;
            }
            break;
        }

        case CASE_MODULE_SET_ID: //接收到修改网络地址命令
        {
            if ((CAN1_Send->finish_flag == 0) && (CAN1_Send->count_use_time_en == 0)) //等待CAN1总线释放
            {
                //接收消息正确，数据处理完成，发送应答
                if (Modbus_Rec_Set_ID(CAN1_Receive->data_bufer, temp_port, Config_Data.Net_IDs, Config_Data.Group_Net_IDs))
                {
                    CAN1_Send->mac_id = Config_Data.Net_IDs[temp_port];
                    CAN1_Send->group_net_id = Config_Data.Group_Net_IDs[temp_port];
                    CAN1_Send->module_port = temp_port;

                    CAN1_Send_Meg(CASE_MODULE_SET_ID, CAN1_Send, CAN1_Receive); //向主机返回状态信息
                }

                //复位全局标志位
                CAN1_Receive->mac_id = 0; //解除CAN1的ID过滤
                CAN1_Receive->data_len = CAN_Rec_Base_Len;
                CAN1_Receive->finish_flag = 0;
            }
            break;
        }

        case CASE_CHANGE_PORT: //本设备功能，设置开关
        {
            if (Modbus_Rec_Change_Port(CAN1_Receive->data_bufer, temp_port) == 0)
            {
                CAN1_Receive->mac_id = 0; //解除CAN1的ID过滤
                CAN1_Receive->data_len = CAN_Rec_Base_Len;
                CAN1_Receive->finish_flag = 0;
                break;
            }

            if ((CAN1_Send->finish_flag == 0) && (CAN1_Send->count_use_time_en == 0)) //等待CAN1总线释放
            {
                CAN1_Send_Meg(CASE_CHANGE_PORT, CAN1_Send, CAN1_Receive); //返回动作状态

                CAN1_Receive->mac_id = 0; //解除CAN1的ID过滤
                CAN1_Receive->data_len = CAN_Rec_Base_Len;
                CAN1_Receive->finish_flag = 0;
            }
            break;
        }

        case CASE_READ_PORT:
        {
            if ((CAN1_Send->finish_flag == 0) && (CAN1_Send->count_use_time_en == 0)) //等待CAN1总线释放
            {
                CAN1_Send_Meg(CASE_READ_PORT, CAN1_Send, CAN1_Receive); //返回动作状态

                CAN1_Receive->mac_id = 0; //解除CAN1的ID过滤
                CAN1_Receive->data_len = CAN_Rec_Base_Len;
                CAN1_Receive->finish_flag = 0;
            }
            break;
        }

        case CASE_MODULE_READ_MSG:
        {
            if ((CAN1_Send->finish_flag == 0) && (CAN1_Send->count_use_time_en == 0)) //等待CAN1总线释放
            {
                CAN1_Send_Meg(CASE_MODULE_READ_MSG, CAN1_Send, CAN1_Receive); //返回动作状态

                CAN1_Receive->mac_id = 0; //解除CAN1的ID过滤
                CAN1_Receive->data_len = CAN_Rec_Base_Len;
                CAN1_Receive->finish_flag = 0;
            }

            break;
        }

        case CASE_SET_CONFIG:
        {
            if (Modbus_Rec_Set_Config(CAN1_Receive->data_bufer, temp_port) == 0)
            {
                CAN1_Receive->mac_id = 0; //解除CAN1的ID过滤
                CAN1_Receive->data_len = CAN_Rec_Base_Len;
                CAN1_Receive->finish_flag = 0;
                break;
            }

            if ((CAN1_Send->finish_flag == 0) && (CAN1_Send->count_use_time_en == 0)) //等待CAN1总线释放
            {
                CAN1_Send_Meg(CASE_SET_CONFIG, CAN1_Send, CAN1_Receive); //返回动作状态

                CAN1_Receive->mac_id = 0; //解除CAN1的ID过滤
                CAN1_Receive->data_len = CAN_Rec_Base_Len;
                CAN1_Receive->finish_flag = 0;
            }
            break;
        }

        case CASE_READ_CONFIG:
        {
            if ((CAN1_Send->finish_flag == 0) && (CAN1_Send->count_use_time_en == 0)) //等待CAN1总线释放
            {
                CAN1_Send_Meg(CASE_READ_CONFIG, CAN1_Send, CAN1_Receive); //返回动作状态
                CAN1_Receive->mac_id = 0;                                 //解除CAN1的ID过滤
                CAN1_Receive->data_len = CAN_Rec_Base_Len;
                CAN1_Receive->finish_flag = 0;
            }
            break;
        }

        case CASE_MODULE_JSON_DATA: /* json数据包 */
        {
            Modbus_Rec_Msg_IAP(CAN1_Receive->data_bufer, temp_port, CAN1_Send);

            //复位全局标志位
            CAN1_Receive->mac_id = 0; //解除CAN1的ID过滤
            CAN1_Receive->data_len = CAN_Rec_Base_Len;
            CAN1_Receive->finish_flag = 0;

            break;
        }

        default:
        {
            //复位全局标志位
            CAN1_Receive->mac_id = 0; //解除CAN1的ID过滤
            CAN1_Receive->data_len = CAN_Rec_Base_Len;
            CAN1_Receive->finish_flag = 0;
            break;
        }
        }

        if (temp_port >= Net_IDs_All) //组ID
        {
            for (temp_port = 0; temp_port < Net_IDs_All; temp_port++)
            {
                if (Config_Data.Group_Net_IDs[temp_port] == (CAN1_Receive->data_bufer[Bus_Target_Net_ID] * 256 + CAN1_Receive->data_bufer[Bus_Target_Net_ID + 1]))
                {
                    net_led_sharp(); //闪烁网络指示灯

                    switch (CAN1_Receive->data_bufer[CASE_FUNCTION]) //选择功能码
                    {
                    case CASE_CHANGE_PORT: //本设备功能，设置开关
                    {
                        Modbus_Rec_Change_Port(CAN1_Receive->data_bufer, temp_port);
                        CAN1_Receive->mac_id = 0; //解除CAN1的ID过滤
                        CAN1_Receive->data_len = CAN_Rec_Base_Len;
                        CAN1_Receive->finish_flag = 0;
                        break;
                    }
                    }
                }
            }

            //扫描完成，结束接收处理
            CAN1_Receive->mac_id = 0; //解除CAN1的ID过滤
            CAN1_Receive->data_len = CAN_Rec_Base_Len;
            CAN1_Receive->finish_flag = 0;
        }
    }
}

//ModBus协议
void CAN1_Send_Meg(uint16_t function_num, Send_Rec_Data_Structure *CAN1_Send, Send_Rec_Data_Structure *CAN1_Receive)
{
    uint16_t i;
    uint8_t send_buf[40]; //串口发送缓冲区

    send_buf[2] = Host_Net_ID / 256; //目标地址
    send_buf[3] = Host_Net_ID % 256;

    send_buf[4] = 2;                      //数据长度
    send_buf[5] = function_num;           //功能码
    send_buf[6] = CAN1_Send->module_port; //端口号

    switch (function_num)
    {
    case CASE_MODULE_MSG:
    {
        //send_buf[0]=CAN1_Send->mac_id/256;//源地址，缺省
        //send_buf[1]=CAN1_Send->mac_id/256;
        send_buf[2] = Host_Net_ID / 256; //目标地址
        send_buf[3] = Host_Net_ID % 256;

        send_buf[4] = 23;              //数据长度
        send_buf[5] = CASE_MODULE_MSG; //功能码
        send_buf[6] = 0x00;            //端口号

        send_buf[7] = CAN1_Send->mac_id / 256;
        send_buf[8] = CAN1_Send->mac_id % 256;
        send_buf[9] = CAN1_Send->group_net_id / 256;
        send_buf[10] = CAN1_Send->group_net_id % 256;

        //写入设备类型与版本
        Modbus_Send_Set_ID(send_buf, CAN1_Send->module_port);

        send_buf[15] = STM32_ID.id_u8[0];
        send_buf[16] = STM32_ID.id_u8[1];
        send_buf[17] = STM32_ID.id_u8[2];
        send_buf[18] = STM32_ID.id_u8[3];
        send_buf[19] = STM32_ID.id_u8[4];
        send_buf[20] = STM32_ID.id_u8[5];
        send_buf[21] = STM32_ID.id_u8[6];
        send_buf[22] = STM32_ID.id_u8[7];
        send_buf[23] = STM32_ID.id_u8[8];
        send_buf[24] = STM32_ID.id_u8[9];
        send_buf[25] = STM32_ID.id_u8[10];
        send_buf[26] = STM32_ID.id_u8[11];
        send_buf[27] = CAN1_Send->module_port;

        CAN1_Send->mac_id = 0x0000; //广播指令到模块设置上

        break;
    }

    case CASE_MODULE_SET_ID:
    {
        //send_buf[0]=CAN1_Send->mac_id/256;//源地址，缺省
        //send_buf[1]=CAN1_Send->mac_id/256;
        send_buf[2] = Host_Net_ID / 256; //目标地址
        send_buf[3] = Host_Net_ID % 256;

        send_buf[4] = 23;                 //数据长度
        send_buf[5] = CASE_MODULE_SET_ID; //功能码
        send_buf[6] = 0x00;               //端口号

        send_buf[7] = CAN1_Send->mac_id / 256;
        send_buf[8] = CAN1_Send->mac_id % 256;
        send_buf[9] = CAN1_Send->group_net_id / 256;
        send_buf[10] = CAN1_Send->group_net_id % 256;

        //写入设备类型、版本、编号
        Modbus_Send_Set_ID(send_buf, CAN1_Send->module_port);

        send_buf[15] = STM32_ID.id_u8[0];
        send_buf[16] = STM32_ID.id_u8[1];
        send_buf[17] = STM32_ID.id_u8[2];
        send_buf[18] = STM32_ID.id_u8[3];
        send_buf[19] = STM32_ID.id_u8[4];
        send_buf[20] = STM32_ID.id_u8[5];
        send_buf[21] = STM32_ID.id_u8[6];
        send_buf[22] = STM32_ID.id_u8[7];
        send_buf[23] = STM32_ID.id_u8[8];
        send_buf[24] = STM32_ID.id_u8[9];
        send_buf[25] = STM32_ID.id_u8[10];
        send_buf[26] = STM32_ID.id_u8[11];
        send_buf[27] = CAN1_Send->module_port;

        break;
    }

    case CASE_CHANGE_PORT:
    {
        Modbus_Send_Change_Port(send_buf, CAN1_Send->module_port);

        break;
    }

    case CASE_READ_PORT:
    {
        Modbus_Send_Read_Port(send_buf, CAN1_Send->module_port);

        break;
    }

    case CASE_MODULE_READ_MSG:
    {
        Modebus_Send_Read_ReplyMsg(send_buf, CAN1_Send->module_port);

        break;
    }

    case CASE_MODULE_SEND_MSG:
    {
        Modebus_Send_ReplyMsg(send_buf, CAN1_Send->module_port);

        break;
    }

    case CASE_SET_CONFIG:
    {
        Modbus_Send_Set_Config(send_buf, CAN1_Send->module_port);

        break;
    }

    case CASE_READ_CONFIG:
    {
        Modbus_Send_Read_Config(send_buf, CAN1_Send->module_port);

        break;
    }

    default:
        break;
    }

    i = CAN_Rec_Base_Len + send_buf[4] - 2; //去掉源地址
    CAN1_Send->data_len = i;
    //可直接内存拷贝，允许再次接收
    for (i = 0; i < CAN1_Send->data_len; i++)
    {
        CAN1_Send->data_bufer[i] = send_buf[i + 2]; //数据包中去掉源地址，从CAN帧中提取
    }

    net_led_sharp();          // 发送数据闪烁指示
    CAN1_Send_Str(CAN1_Send); //通过CAN口发送数据
}

/**
 * @description: 发送Json数据包给主机
 * @param {uint16_t} function_num
 * @param {Send_Rec_Data_Structure} *CAN1_Send
 * @return {*}
 */
void CAN1_Send_JsonMsg(uint16_t function_num, Send_Rec_Data_Structure *CAN1_Send)
{
    uint16_t i;
    uint8_t send_buf[200] = {0}; //串口发送缓冲区

    send_buf[2] = Host_Net_ID / 256; //目标地址
    send_buf[3] = Host_Net_ID % 256;
    send_buf[4] = 2; //数据长度

    switch (function_num)
    {
    case CASE_MODULE_IAP: /* 返回握手信息 */
    {
        memset(msg, '\0', sizeof(msg) / sizeof(msg[0]));
        send_buf[4] = sprintf(msg, "{\"f\":%d,\"sc\":\"%s\",\"msg\":\"%s\"}",
                              function_num,
                              "0",
                              "0");

        memcpy(&send_buf[5], msg, send_buf[4]);

        break;
    }

    case CASE_MODULE_MSG:
    {
        memset(msg, '\0', sizeof(msg) / sizeof(msg[0]));
        /* json数据包 */
        send_buf[4] = sprintf(msg, "{\"f\":%d,\"md\":\"%s\",\"mac\":\"%s\",\"sv\":\"%s\",\"uv\":\"%s\",\"pv\":\"%s\",\"hv\":\"%s\",\"ft\":%d,\"st\":%d}",
                              function_num,
                              Product_Type,
                              Product_Mac,
                              Product_Firmware_Version,
                              Product_UpdataPakge_Version,
                              Product_Protoc0l_Version,
                              Product_Hardware_Version,
                              Product_Dev_Version,
                              Product_Subdev_Version);

        memcpy(&send_buf[5], msg, send_buf[4]);

        break;
    }

    default:
        break;
    }

    i = CAN_Rec_Base_Len + send_buf[4] - 2; //去掉源地址
    CAN1_Send->data_len = i;
    //可直接内存拷贝，允许再次接收
    for (i = 0; i < CAN1_Send->data_len; i++)
    {
        CAN1_Send->data_bufer[i] = send_buf[i + 2]; //数据包中去掉源地址，从CAN帧中提取
    }

    net_led_sharp();          // 发送数据闪烁指示
    CAN1_Send_Str(CAN1_Send); //通过CAN口发送数据
}

/**
 * @description: 应答主机更新包请求
 * @param {uint16_t} function_num
 * @param {Send_Rec_Data_Structure} *CAN1_Send
 * @return {*}
 */
void CAN1_Send_JsonUpdateMsg(uint16_t function_num, uint32_t pkg_len, Send_Rec_Data_Structure *CAN1_Send)
{
    uint16_t i;
    uint8_t send_buf[100] = {0}; //串口发送缓冲区

    send_buf[2] = Host_Net_ID / 256; //目标地址
    send_buf[3] = Host_Net_ID % 256;
    send_buf[4] = 2; //数据长度

    memset(msg, '\0', sizeof(msg) / sizeof(msg[0]));
    send_buf[4] = sprintf(msg, "{\"f\":%d,\"rb\":%d}", function_num, pkg_len);
    memcpy(&send_buf[5], msg, send_buf[4]);

    i = CAN_Rec_Base_Len + send_buf[4] - 2; //去掉源地址
    CAN1_Send->data_len = i;
    //可直接内存拷贝，允许再次接收
    for (i = 0; i < CAN1_Send->data_len; i++)
    {
        CAN1_Send->data_bufer[i] = send_buf[i + 2]; //数据包中去掉源地址，从CAN帧中提取
    }

    net_led_sharp();          // 发送数据闪烁指示
    CAN1_Send_Str(CAN1_Send); //通过CAN口发送数据
}

//CAN1 非实时任务
void CAN1_Ports_Adjust(void)
{
    static uint8_t can_receive_buf_ptr = 0;

    for (; can_receive_buf_ptr < (Net_IDs_All + 1); can_receive_buf_ptr++)
    {
        if (CAN1_Receive_Buf[can_receive_buf_ptr].finish_flag == 1)
        {
            if (can_receive_buf_ptr != 0)                                                    // 1~N
                CAN1_Receive_Buf[can_receive_buf_ptr].module_port = can_receive_buf_ptr - 1; //更新module_port,可通过can中断接收更新，其中广播ID与通道0共用缓冲区

            CAN1_Rec_Contorl(&CAN1_Receive_Buf[can_receive_buf_ptr], &CAN1_Send, NULL, NULL); //Can接收处理函数，如完成后自动复位标志位
            break;
        }
    }

    if (can_receive_buf_ptr >= (Net_IDs_All + 1))
        can_receive_buf_ptr = 0;
}
