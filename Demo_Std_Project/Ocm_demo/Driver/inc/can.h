/*
 * @Author: your name
 * @Date: 2020-09-17 15:50:30
 * @LastEditTime: 2020-09-23 11:23:59
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \Demo_Std_Project\Ocm_demo\Driver\inc\can.h
 */
/*
 * can.h
 *
 *  Created on: 2020年9月17日
 *      Author: Administrator
 */

#ifndef DRIVER_INC_CAN_H_
#define DRIVER_INC_CAN_H_

#include "stm32f10x.h"
#include "stm32f10x_can.h"

void hw_can_init(void);
void can_send(CanTxMsg *TxMessage, uint8_t *buf, uint8_t size);
void can_Tx_clear(void);
void USB_HP_CAN1_TX_IRQHandler(void);
void USB_LP_CAN1_RX0_IRQHandler(void);

void __attribute__((weak)) can_rx_isr_hook(CanRxMsg *rxmessage);

#endif /* DRIVER_INC_CAN_H_ */
