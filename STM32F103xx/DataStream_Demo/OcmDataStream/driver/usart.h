/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.onecm.com
**
**------------------------------------File Info----------------------------------------------------------*/
/*
 * @Description: 
 * @version: 
 * @Author: Saber
 * @Date: 2020-08-31 10:22:53
 * @FilePath: \OcmDataStream\driver\usart.h
 * @LastEditors: Saber
 * @LastEditTime: 2020-09-01 09:56:53
 * @**
 * **********************************************************************************************************
 */
#ifndef __USART_H__
#define __USART_H__

#include "stm32f10x.h"
#include "datastream_core.h"

#define DATA_BITS_8 8
#define DATA_BITS_9 9

#define STOP_BITS_1 0
#define STOP_BITS_2 1

#define PARITY_NONE 256
#define PARITY_ODD 512
#define PARITY_EVEN 1024
#define PARITY_MARK 2048
#define PARITY_SPACE 4096

#define BAUD_RATE_2400 2400
#define BAUD_RATE_4800 4800
#define BAUD_RATE_9600 9600
#define BAUD_RATE_19200 19200
#define BAUD_RATE_38400 38400
#define BAUD_RATE_57600 57600
#define BAUD_RATE_115200 115200
#define BAUD_RATE_230400 230400
#define BAUD_RATE_460800 460800
#define BAUD_RATE_921600 921600
#define BAUD_RATE_2000000 2000000
#define BAUD_RATE_3000000 3000000

#define BIT_ORDER_LSB 0
#define BIT_ORDER_MSB 1

#define NRZ_NORMAL 0   /* Non Return to Zero : normal mode */
#define NRZ_INVERTED 1 /* Non Return to Zero : inverted mode */

#define OCM_SERIAL_RB_BUFSZ 64

/* Default config for serial_configure structure */
#define OCM_SERIAL_CONFIG_DEFAULT                     \
    {                                                 \
        BAUD_RATE_9600,          /* 115200 bits/s */  \
            DATA_BITS_8,         /* 8 databits */     \
            STOP_BITS_1,         /* 1 stopbit */      \
            PARITY_NONE,         /* No parity  */     \
            BIT_ORDER_LSB,       /* LSB first sent */ \
            NRZ_NORMAL,          /* Normal mode */    \
            OCM_SERIAL_RB_BUFSZ, /* Buffer size */    \
            0                                         \
    }

struct usart_configure
{
    rt_uint32_t baud_rate;

    rt_uint32_t data_bits : 4;
    rt_uint32_t stop_bits : 2;
    rt_uint32_t parity : 2;
    rt_uint32_t bit_order : 1;
    rt_uint32_t invert : 1;
    rt_uint32_t bufsz : 16;
    rt_uint32_t reserved : 6;
};

#define UART_ENABLE_IRQ(n) NVIC_EnableIRQ((n))
#define UART_DISABLE_IRQ(n) NVIC_DisableIRQ((n))

void ocm_hw_usart_init(void);

#endif
