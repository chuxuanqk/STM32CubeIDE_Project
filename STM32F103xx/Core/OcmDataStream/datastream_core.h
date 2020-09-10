/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.onecm.com
**
**------------------------------------File Info----------------------------------------------------------*/
/*
 * @Description: 
 * @version: 
 * @Author: Saber
 * @Date: 2020-08-25 10:01:08
 * @FilePath: \OcmDataStream\datastream_core.h
 * @LastEditors: Saber
 * @LastEditTime: 2020-09-03 10:50:44
 * @**
 * **********************************************************************************************************
 */

#ifndef __DATAFLOW_CORE_H__
#define __DATAFLOW_CORE_H__

#include "ocm_def.h"
#include "ocm_core.h"

#define BIT_ORDER_LSB 0
#define BIT_ORDER_MSB 1

#define OCM_DATASTREAM_BUFSZ_MAX 105

#define OCM_USART_EVENT_RX_IND 0x01     /* Rx indication */
#define OCM_USART_EVENT_TX_DONE 0x02    /* Tx complete   */
#define OCM_USART_EVENT_RX_DMADONE 0x03 /* Rx DMA transfer done */
#define OCM_USART_EVENT_TX_DMADONE 0x04 /* Tx DMA transfer done */
#define OCM_USART_EVENT_RX_TIMEOUT 0x05 /* Rx timeout    */

#define OCM_DATASTREAM_DMA_RX 0x01
#define OCM_DATASTREAM_DMA_TX 0x02

#define OCM_CAN_EVENT_RX_IND 0x06     /* Rx indication */
#define OCM_CAN_EVENT_TX_DONE 0x07    /* Tx complete   */
#define OCM_CAN_EVENT_RX_DMADONE 0x08 /* Rx DMA transfer done */
#define OCM_CAN_EVENT_TX_DMADONE 0x09 /* Tx DMA transfer done */
#define OCM_CAN_EVENT_RX_TIMEOUT 0x0A /* Rx timeout    */

/**
 * DataStream FIFO mode
*/
struct ocm_datastream_rx_fifo
{
    /* software fifo*/
    ocm_uint8_t *buffer;

    ocm_uint16_t put_index, get_index;

    ocm_bool_t is_full;
}

struct ocm_datastream_tx_fifo
{
    /* software fifo*/
    ocm_flag_t tx_flag;
}

/* 
 * DataStream DMA mode
 */
struct ocm_datastream_rx_dma
{
    ocm_bool_t activated;
};

struct ocm_datastream_tx_dma
{
    ocm_bool_t activated;
};

typedef struct ocm_datastream_device *ocm_datastream_t;
/*
* operations set for datastream object,This is supply for datastream device drive ops
*/
struct ocm_datastream_ops
{
    ocm_err_t (*configure)(ocm_datastream_t datastream, void *cfg);
    ocm_err_t (*control)(ocm_datastream_t datastream, ocm_cmd_t cmd, void *arg);

    /* single byte usart data stream */
    ocm_base_t (*putc)(ocm_datastream_t datastream, char c);
    ocm_base_t (*getc)(ocm_datastream_t datastream);

    ocm_size_t (*dma_transmit)(ocm_datastream_t datastream, ocm_uint8_t *buf, ocm_size_t size, ocm_base_t direction);

    /* multi byte can/usart data stream*/
    ocm_size_t (*sendmsg)(ocm_datastream_t datastream, char *buf);
    ocm_size_t (*recvmsg)(ocm_datastream_t datastream, char *buf);
};

/*
 * datastream structure
 */
struct ocm_datastream_device
{
    struct ocm_device parent; /**< inherit from ocm_device */

    ocm_time_t time_s; /**< time stamp*/

    const struct ocm_datastream_ops *ops;
    void *config; // 相应设备相应config

    void *datastream_rx;
    void *datastream_tx;
};

void ocm_hw_datastream_isr(ocm_datastream_t serial, ocm_event_t event);

ocm_err_t ocm_hw_datastream_register(ocm_datastream_t pdatastream,
                                     const char *name,
                                     ocm_flag_t flag,
                                     void *data);

#endif