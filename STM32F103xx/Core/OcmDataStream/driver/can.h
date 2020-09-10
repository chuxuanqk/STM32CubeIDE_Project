/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.onecm.com
**
**------------------------------------File Info----------------------------------------------------------*/
/*
 * @Description: 
 * @version: 
 * @Author: Saber
 * @Date: 2020-09-02 11:18:09
 * @FilePath: \OcmDataStream\driver\can.h
 * @LastEditors: Saber
 * @LastEditTime: 2020-09-03 14:32:57
 * @**
 * **********************************************************************************************************
 */
#ifndef __CAN_H_
#define __CAN_H_

#include "stm32f10x.h"
#include "datastream_core.h"

/* Defalut config for can_configure structure */
#define OCM_CAN_CONFIG_DEFAULT \
    {
}

struct can_configure
{
};

void USB_LP_CAN1_RX0_IRQHandler(void);
void USB_HP_CAN1_TX_IRQHandler(void);

#endif
