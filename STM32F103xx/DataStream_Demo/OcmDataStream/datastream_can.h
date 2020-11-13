/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.onecm.com
**
**------------------------------------File Info----------------------------------------------------------*/
/*
 * @Description: can 数据流
 * @version: 
 * @Author: Saber
 * @Date: 2020-08-24 16:17:56
 * @FilePath: \STM32F103C8_Demo\Core\OcmDataFlow\datastream_can.h
 * @LastEditors: Saber
 * @LastEditTime: 2020-08-26 17:11:12
 * @**
 * **********************************************************************************************************
 */
#ifndef __DATASTREAM_CAN_H__
#define __DATASTREAM_CAN_H__

#include "datastream_core.h"
#include "stm32f1xx_hal.h"

struct ocm_can_data
{
    uint16_t src_net_id;
    uint16_t des_net_id;
    uint8_t data_len;
    enum ocm_func_calss_type function_code;
    uint8_t port_id;     // 通道号/端口号
    uint8_t *pdata_buf;  // 实际需要处理的数据
    uint16_t check_code; // eg:modbus crc校验码/Can 已自动添加了

    struct ocm_datastream_device datastream;
};

#endif