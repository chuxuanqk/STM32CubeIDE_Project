/*
 * modbus_func_adjust.c
 *
 *  Created on: 2020年1月3日
 *      Author: Fox
 */

#include "config_device/config_par.h"
#include "common/jsmn.h"
#include "protocol/modbus_func_adjust.h"
#include "protocol/modbus_port_adjust.h"
#include "virtual_device/virtual_rs485.h"
#include "sys_device/sys_init.h"
#include "stm32f10x_crc.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

uint32_t iap_fireware_length = 0;      // 更新包长度
static uint32_t recv_fireware_len = 0; // 已接收的更新包长度
uint8_t tmp_buf[512] = {0};
uint8_t tmp_len = 0;

/* 升级的步骤 */
enum UPDATE_STATE
{
    TO_START = 0x01,
    TO_RECEIVE_DATA,
    TO_RECEIVE_END
};
static enum UPDATE_STATE update_state = TO_START;

/* 标记升级完成 */
void Set_Update_Down(void)
{
    unsigned int update_flag = 0xAAAAAAAA; ///< 对应bootloader的启动步骤
    WriteFlash((Application_2_Addr + Application_Size - 4), &update_flag, 1);
}

//清除配置的目标通道的互斥通道
uint8_t Modbus_Clean_Mix_Port(uint8_t temp_port, uint8_t temp_port_stop)
{
    return 1;
}

/**
 * @description:  IAP 固件升级
 * @param {uint8_t} *recbuf
 * @param {uint8_t} temp_port
 * @return {*}
 */
/*
uint8_t Modbus_Rec_Msg_IAP(uint8_t *recbuf, uint8_t temp_port)
{
    uint8_t ret = 0;
    jsmn_parse p;
    jsmntok_t t[20];

    tmp_len = recbuf[4];
    memcpy((uint8_t *)tmp_buf, (uint8_t *)(&recbuf[6]), tmp_len);
    jsmn_init(&p);
    int r = jsmn_parse(&p, (char *)tmp_buf, tmp_len, t, sizeof(t) / sizeof(t[0]));

    switch (update_state)
    {
    case TO_START:
    {
        if (tmp_buf[0] == 0x01)
        {
            iap_fireware_length = (tmp_buf[1] << 24) + (tmp_buf[2] << 16) +
                                (tmp_buf[3] << 8) + tmp_buf[4];
            recv_fireware_len = 0;
            update_state = TO_RECEIVE_DATA;

            Erase_page(Application_2_Addr, Application_Page);
        }
        break;
    }

    case TO_RECEIVE_DATA:
    {
        uint8_t halfword_size = tmp_len / 2;
        uint32_t app2addr = Application_2_Addr + recv_fireware_len;
        Write_HalfWord_Flash(app2addr, (uint16_t *)tmp_buf, halfword_size);

        recv_fireware_len += tmp_len;
        printf("> Receive data bag:%d byte\tTotal: %d byte\r\n", recv_fireware_len, iap_fireware_length);
        if (recv_fireware_len == iap_fireware_length)
            update_state = TO_RECEIVE_END;

        break;
    }

    case TO_RECEIVE_END:
    {
        Set_Update_Down();
        SystemReset(); // 软复位
        break;
    }

    default:
        break;
    }

    ret = 1;

    return ret;
}
*/

/**
 * @description: 接收到更新包数据处理
 * @param {uint8_t} *recbuf
 * @param {uint8_t} temp_port
 * @return {*}
 */
uint8_t Modbus_Rec_PkgData_IAP(uint8_t *recbuf, uint8_t temp_port, Send_Rec_Data_Structure *CAN1_Send)
{
    uint8_t tmp_len = recbuf[4] - 1;
    uint8_t function_num = recbuf[5];

    if (tmp_len % 2) /* 接收字节数不为偶数 */
        return 0;

    uint8_t data_len = tmp_len - 4;
    uint8_t *pdata = &recbuf[6];
    uint8_t i = data_len;
    uint32_t crc32 = (pdata[i++] << 24) +
                     (pdata[i++] << 16) +
                     (pdata[i++] << 8) +
                     pdata[i];
    /* 硬件CRC32校验 */
    uint32_t tmp_crc32 = CRC_CalcBlockCRC(pdata, data_len);
    if (tmp_crc32 != crc32)
        return 0;

    uint8_t halfword_size = data_len / 2;
    uint32_t app2addr = Application_2_Addr + recv_fireware_len;
    Write_HalfWord_Flash(app2addr, (uint16_t *)tmp_buf, halfword_size);

    recv_fireware_len += data_len; // 接收的数据长度
    printf("> Receive data bag:%d byte", recv_fireware_len);

    if ((CAN1_Send->finish_flag == 0) && (CAN1_Send->count_use_time_en == 0)) //等待CAN1总线释放
    {
        CAN1_Send_JsonUpdateMsg(function_num, recv_fireware_len, CAN1_Send);
    }

    return 1;
}

/**
 * @description: 
 * @param {uint8_t} *recbuf
 * @param {uint8_t} temp_port
 * @param {Send_Rec_Data_Structure} *CAN1_Send
 * @return {*}
 */
uint8_t Modbus_Rec_Msg_IAP(uint8_t *recbuf, uint8_t temp_port, Send_Rec_Data_Structure *CAN1_Send)
{
    jsmn_parser p;
    jsmntok_t t[20];

    tmp_len = recbuf[4];
    memcpy((uint8_t *)tmp_buf, (uint8_t *)(&recbuf[5]), tmp_len);
    jsmn_init(&p);
    int r = jsmn_parse(&p, (char *)tmp_buf, tmp_len, t, sizeof(t) / sizeof(t[0]));
    int function = -1;
    long port = -1;

    if (tmp_buf[1] == 'f')
    {
        char *temp = tmp_buf + t[2].start;
        function = atoi(temp);
    }

    if (tmp_buf[3] == 'pd')
    {
        char *temp = tmp_buf + t[4].start;
        char *ptr;
        // port = atoi(temp);
        port = strtol(temp, ptr, 10);
    }

    if ((function == -1) || (port == -1))
        return 0;

    /* 信息处理 */
    switch (function)
    {
    case CASE_MODULE_IAP: /* 握手信号 */
    {
        /* 初始化记录接收包长度 */
        iap_fireware_length = 0;
        recv_fireware_len = 0;
        Erase_page(Application_2_Addr, Application_Page);
        /**/

        if ((CAN1_Send->finish_flag == 0) && (CAN1_Send->count_use_time_en == 0)) //等待CAN1总线释放
        {
            CAN1_Send_JsonMsg(CASE_MODULE_IAP, CAN1_Send);
        }
        break;
    }

    case CASE_MODULE_MSG:
    {
        if ((CAN1_Send->finish_flag == 0) && (CAN1_Send->count_use_time_en == 0)) //等待CAN1总线释放
        {
            CAN1_Send_JsonMsg(CASE_MODULE_MSG, CAN1_Send);
        }
        break;
    }

    default:
        break;
    }

    return 1;
}

uint8_t Modbus_Rec_Set_ID(uint8_t *recbuf, uint8_t temp_port, uint16_t *net_id, uint16_t *group_net_id)
{
    if (temp_port < Net_IDs_Single)
    {
        uint8_t i = 11; //根据协议，i从9开始匹配设备类型、固件版本和物理地址

        if ((recbuf[11] * 256 + recbuf[12]) == Mac_Style_Single)
            i = i + 2;

        if ((recbuf[13] * 256 + recbuf[14]) == Version)
            i = i + 2;

        if (i == 15)
        {
            for (; i < 27; i++)
            {
                if (recbuf[i] != STM32_ID.id_u8[i - 15]) //判断物理地址是否正确
                    break;
            }
        }
        if (i >= 27) //物理地址正确
        {
            //修改端口在允许范围内
            if (temp_port < Net_IDs_All)
            {

                *net_id = recbuf[7] * 256 + recbuf[8];        //修改网络地址
                *group_net_id = recbuf[9] * 256 + recbuf[10]; //修改组播地址

                Write_Config_To_Flash(&Config_Data); //修改的配置数据写入文件

                return 1;
            }
        }
    }

    return 0;
}

/**
 * @description:  CASE_CHANGE_PORT 0x04
 * @param {uint8_t} *recbuf
 * @param {uint8_t} temp_port
 * @return {*}
 */
uint8_t Modbus_Rec_Change_Port(uint8_t *recbuf, uint8_t temp_port)
{
    uint8_t ret = ModbusRs485_Rec_Change_Port(recbuf, temp_port);

    return ret;
}

/**
 * @description: 
 * @param {uint8_t} *recbuf
 * @param {uint8_t} temp_port
 * @return {*}
 */
uint8_t Modbus_Rec_Set_Config(uint8_t *recbuf, uint8_t temp_port)
{
    uint8_t ret = ModbusRs485_Rec_Set_Config(recbuf, temp_port);

    return ret;
}

/**
 * @description: 
 * @param {uint8_t} *recbuf
 * @param {uint8_t} temp_port
 * @return {*}
 */
uint8_t Modbus_Rec_Read_Config(uint8_t *recbuf, uint8_t temp_port) { return 0; }

/*********************************************************************************************/
/**
 * @description: IAP ACK
 * @param {uint8_t} *sendbuf
 * @param {uint8_t} temp_port
 * @return {*}
 */
uint8_t Modbus_Send_ACK_IAP(uint8_t *sendbuf, uint8_t temp_port)
{
    uint8_t ret = 0;

    ret = 1;

    return ret;
}

/**
 * @description: 
 * @param {uint8_t} *sendbuf
 * @param {uint8_t} temp_port
 * @return {*}
 */
uint8_t Modbus_Send_Set_ID(uint8_t *sendbuf, uint8_t temp_port)
{
    uint8_t ret = 0;
    uint16_t module_style, module_version;

    module_style = Mac_Style_Single;
    module_version = Version;
    ret = 1;

    if (ret)
    {
        sendbuf[11] = module_style / 256;
        sendbuf[12] = module_style % 256;
        sendbuf[13] = module_version / 256;
        sendbuf[14] = module_version % 256;
    }

    return ret;
}

/**
 * @description: 
 * @param {uint8_t} *sendbuf
 * @param {uint8_t} temp_port
 * @return {*}
 */
uint8_t Modbus_Send_Read_Port(uint8_t *sendbuf, uint8_t temp_port)
{
    uint8_t ret = ModbusRs485_Send_Read_Port(sendbuf, temp_port);

    return ret;
}

/**
 * @description: 查询指令返回信息
 * @param {*}
 * @return {*}
 */
uint8_t Modebus_Send_Read_ReplyMsg(uint8_t *sendbuf, uint8_t temp_port)
{
    uint8_t ret = ModbusRs485_Send_Read_ReplyMsg(sendbuf, temp_port);

    return ret;
}

/**
 * @description: 主动发送指令返回信息
 * @param {*}
 * @return {*}
 */
uint8_t Modebus_Send_ReplyMsg(uint8_t *sendbuf, uint8_t temp_port)
{
    uint8_t ret = ModbusRs485_Send_ReplyMsg(sendbuf, temp_port);

    return ret;
}

/**
 * @description: 
 * @param {uint8_t} *sendbuf
 * @param {uint8_t} temp_port
 * @return {*}
 */
uint8_t Modbus_Send_Read_Config(uint8_t *sendbuf, uint8_t temp_port)
{
    uint8_t ret = ModbusRs485_Send_Read_Config(sendbuf, temp_port);

    return ret;
}

/**
 * @description: 
 * @param {uint8_t} *sendbuf
 * @param {uint8_t} temp_port
 * @return {*}
 */
uint8_t Modbus_Send_Change_Port(uint8_t *sendbuf, uint8_t temp_port) { return 0; }

/**
 * @description: 
 * @param {uint8_t} *sendbuf
 * @param {uint8_t} temp_port
 * @return {*}
 */
uint8_t Modbus_Send_Set_Config(uint8_t *sendbuf, uint8_t temp_port) { return 0; }
