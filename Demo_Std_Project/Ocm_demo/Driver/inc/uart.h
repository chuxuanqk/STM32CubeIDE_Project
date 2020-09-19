/*
 * uart.h
 *
 *  Created on: 2020年9月14日
 *      Author: Administrator
 */

#ifndef DRIVER_INC_UART_H_
#define DRIVER_INC_UART_H_

#include "stm32f10x.h"
#include "stdbool.h"
#include "config.h"

#define DATA_BITS_8 8
#define DATA_BITS_9 9

#define STOP_BITS_1 0
#define STOP_BITS_1_5 1
#define STOP_BITS_2 2

#define PARITY_NONE 0
#define PARITY_ODD 1
#define PARITY_EVEN 2

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

#define UART_DMA_RB_BUFSZ 108

/* Default config for serial_configure structure */
#define UART_CONFIG_DEFAULT                     \
  {                                             \
    BAUD_RATE_9600,        /* 9600 bits/s */    \
        DATA_BITS_8,       /* 8 databits */     \
        STOP_BITS_1,       /* 1 stopbit */      \
        PARITY_NONE,       /* No parity  */     \
        BIT_ORDER_LSB,     /* LSB first sent */ \
        NRZ_NORMAL,        /* Normal mode */    \
        UART_DMA_RB_BUFSZ, /* Buffer size */    \
        0                                       \
  }

struct uart_configure
{
  uint32_t baud_rate;

  uint32_t data_bits : 4;
  uint32_t stop_bits : 2;
  uint32_t parity : 2;
  uint32_t bit_order : 1;
  uint32_t invert : 1;
  uint32_t bufsz : 16;
  uint32_t reserved : 6;
};

struct uart_device
{
  USART_TypeDef *uartx;
  IRQn_Type irq;

#ifdef USE_USART_DMA_RX
  struct uart_dma
  {
    /* dma channel */
    DMA_Channel_TypeDef *rx_ch;
    /* dma global flag */
    uint32_t rx_gl_flag;
    /* dma irq channel */
    uint8_t rx_irq_ch;
    /* setting receive len */
    uint8_t setting_recv_len;
    /* last receive index */
    uint8_t last_recv_index;
  } dma_rx;
#endif

#ifdef USE_USART_DMA_TX
  struct uart_dma_tx
  {
    /* dma channel */
    DMA_Channel_TypeDef *tx_ch;
    /* dma global flag */
    uint32_t tx_gl_flag;
    /* dma irq channel */
    uint8_t tx_irq_ch;
    /* dma tx complete IT */
    uint32_t tx_tc_IT;
  } dma_tx;

#endif
};

enum UART_FLAG
{
  UART_FLAG_DEFAULT,
  UART_FLAG_DMA_TC,
  UART_FLAG_DMA_RC,
  UART_FLAG_TC,
  UART_FLAG_RC,
  UART_FLAG_IDLE,
};

struct uart_data
{
  struct uart_device *uart_device;
  bool rx_flag;                         // 接收flag
  bool tx_flag;                         // 传输flag
  uint8_t rx_index;                     // 当前接收的数据下标
  uint8_t recv_len;                     // 需要接收数据的长度
  uint8_t stream_rx[UART_DMA_RB_BUFSZ]; // 单缓冲区
  uint8_t stream_tx[UART_DMA_RB_BUFSZ];
};

void hw_uart_init(void);
int16_t uart_getc(USART_TypeDef *uartx);
int16_t uart_read(USART_TypeDef *uartx, uint8_t *pbuf, uint8_t size);
int16_t uart_write(USART_TypeDef *uartx, uint8_t *pbuf, uint8_t size);

bool uart_getFlagStatus(USART_TypeDef *uartx, enum UART_FLAG uflag);
int16_t uart_SetRead_Size(USART_TypeDef *uartx, uint8_t size);

void __attribute__((weak)) uart_dmaisr_hook(void);
void __attribute__((weak)) uart_tc_isr_hook(void);

#define uart_recv(uartx, pbuf, size) uart_read(uartx, pbuf, size)
#define uart_send(uartx, pbuf, size) uart_write(uartx, pbuf, size)

void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);

void DMA1_Channel4_IRQHandler(void);
void DMA1_Channel7_IRQHandler(void);
void DMA1_Channel2_IRQHandler(void);

#endif /* DRIVER_INC_UART_H_ */
