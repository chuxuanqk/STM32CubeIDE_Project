/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.onecm.com
**
**------------------------------------File Info----------------------------------------------------------*/
/*
 * @Description: 
 * @version: 
 * @Author: Saber
 * @Date: 2020-08-31 10:23:01
 * @FilePath: \OcmDataStream\driver\usart.c
 * @LastEditors: Saber
 * @LastEditTime: 2020-09-03 17:13:07
 * @**
 * **********************************************************************************************************
 */
#include "usart.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "misc.h"

/* USART1 */
#define UART1_GPIO_TX GPIO_Pin_9
#define UART1_GPIO_RX GPIO_Pin_10
#define UART1_GPIO GPIOA

/* USART2 */
#define UART2_GPIO_TX GPIO_Pin_2
#define UART2_GPIO_RX GPIO_Pin_3
#define UART2_GPIO GPIOA

/* USART3_REMAP[1:0] = 00 */
#define UART3_GPIO_TX GPIO_Pin_10
#define UART3_GPIO_RX GPIO_Pin_11
#define UART3_GPIO GPIOB

/* STM32 uart driver */
struct ocm_uart_device
{
    USART_TypeDef *uart_device;
    IRQn_Type irq;
#ifdef OCM_SERIAL_USING_DMA
    struct uart_dma
    {
        /* dma channel */
        DMA_Channel_TypeDef *rx_ch;
        /* dma global flag */
        uint32_t rx_gl_flag;
        /* dma irq channel */
        uint8_t rx_irq_ch;
        /* setting receive len */
        ocm_size_t setting_recv_len;
        /* last receive index */
        ocm_size_t last_recv_index;
    } dma;
#endif
};

#ifdef OCM_SERIAL_USING_DMA
static void DMA_Configuration(struct ocm_datastream_device *serial);
#endif

static ocm_err_t uart_configure(struct ocm_datastream_device *serial, struct usart_configure *cfg)
{
    struct ocm_uart_device *uart;
    USART_InitTypeDef USART_InitStructure;

    uart = (struct ocm_uart_device *)serial->parent.user_data;

    USART_InitStructure.USART_BaudRate = cfg->baud_rate;

    if (cfg->data_bits == DATA_BITS_8)
    {
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    }
    else if (cfg->data_bits == DATA_BITS_9)
    {
        USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    }

    if (cfg->stop_bits == STOP_BITS_1)
    {
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
    }
    else if (cfg->stop_bits == STOP_BITS_2)
    {
        USART_InitStructure.USART_StopBits = USART_StopBits_2;
    }

    if (cfg->parity == PARITY_NONE)
    {
        USART_InitStructure.USART_Parity = USART_Parity_No;
    }
    else if (cfg->parity == PARITY_ODD)
    {
        USART_InitStructure.USART_Parity = USART_Parity_Odd;
    }
    else if (cfg->parity == PARITY_EVEN)
    {
        USART_InitStructure.USART_Parity = USART_Parity_Even;
    }

    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(uart->uart_device, &USART_InitStructure);

    /* Enable USART */
    USART_Cmd(uart->uart_device, ENABLE);

    return OCM_EOK;
}

static ocm_err_t uart_control(struct ocm_datastream_device *serial, ocm_cmd_t cmd, void *arg)
{
    struct ocm_uart_device *uart;

    uart = (struct ocm_uart_device *)serial->parent.user_data;

    switch (cmd)
    {
        /* disable interrupt */
    case OCM_DEVICE_CTRL_CLR_INT:
        /* disable interrupt */
        UART_DISABLE_IRQ(uart->irq);
        /* disable interrupt */
        USART_ITConfig(uart->uart_device, USART_IT_RXNE, DISABLE);
        break;

        /* enable interrupt */
    case OCM_DEVICE_CTRL_SET_INT:
        /* enable rx irq */
        UART_ENABLE_IRQ(uart->irq);
        /* enable interrupt */
        USART_ITConfig(uart->uart_device, USART_IT_RXNE, ENABLE);
        break;

#ifdef OCM_SERIAL_USING_DMA
    case OCM_DEVICE_CTRL_CONFIG:
        if ((ocm_base_t)(arg) == OCM_DEVICE_FLAG_DMA_RX)
        {
            DMA_Configuration(serial);
        }

        break;
#endif
    default:
        break;
    }

    return OCM_EOK;
}

static ocm_base_t uart_putc(struct ocm_datastream_device *serial, char c)
{
    struct ocm_uart_device *uart;

    uart = (struct ocm_uart_device *)serial->parent.user_data;

    /* 打开方式有发送标志 */
    if (serial->parent.open_flag & OCM_DEVICE_FLAG_INT_TX)
    {
        if (!(uart->uart_device->SR & USART_FLAG_TXE))
        {
            USART_ITConfig(uart->uart_device, USART_IT_TC, ENABLE);

            return -1;
        }
        uart->uart_device->DR = c;

        USART_ITConfig(uart->uart_device, USART_IT_TC, ENABLE);
    }
    else
    {
        USART_ClearFlag(uart->uart_device, USART_FLAG_TC);
        uart->uart_device->DR = c;
        while (!(uart->uart_device->SR & USART_FLAG_TC))
            ;
    }

    return 1;
}

static ocm_base_t uart_getc(struct ocm_datastream_device *serial)
{
    ocm_base_t ch;
    struct ocm_uart_device *uart;

    uart = (struct ocm_uart_device *)serial->parent.user_data;

    ch = -1;
    if (uart->uart_device->SR & USART_FLAG_RXNE)
        ch = uart->uart_device->DR & 0xff;

    return ch;
}

#ifdef OCM_SERIAL_USING_DMA
/**
 * Serial port receive idle process. This need add to uart idle ISR.
 *
 * @param serial serial device
 */
static void dma_uart_rx_idle_isr(struct ocm_datastream_device *serial)
{
    struct ocm_uart_device *uart = (struct ocm_uart_device *)serial->parent.user_data;
    ocm_size_t recv_total_index, recv_len;
    ocm_base_t level;

    /* disable interrupt */
    level = ocm_hw_interrupt_disable();

    recv_total_index = uart->dma.setting_recv_len - DMA_GetCurrDataCounter(uart->dma.rx_ch);
    recv_len = recv_total_index - uart->dma.last_recv_index;
    uart->dma.last_recv_index = recv_total_index;
    /* enable interrupt */
    ocm_hw_interrupt_enable(level);

    if (recv_len)
        ocm_hw_datastream_isr(serial, OCM_USART_EVENT_RX_DMADONE | (recv_len << 8));

    /* read a data for clear receive idle interrupt flag */
    USART_ReceiveData(uart->uart_device);
    DMA_ClearFlag(uart->dma.rx_gl_flag);
}

/**
 * DMA receive done process. This need add to DMA receive done ISR.
 *
 * @param serial serial device
 */
static void dma_rx_done_isr(struct ocm_datastream_device *serial)
{
    struct ocm_uart_device *uart = (struct ocm_uart_device *)serial->parent.user_data;
    rt_size_t recv_len;
    rt_base_t level;

    /* disable interrupt */
    level = ocm_hw_interrupt_disable();

    recv_len = uart->dma.setting_recv_len - uart->dma.last_recv_index;
    /* reset last recv index */
    uart->dma.last_recv_index = 0;
    /* enable interrupt */
    ocm_hw_interrupt_enable(level);

    if (recv_len)
        ocm_hw_datastream_isr(serial, OCM_USART_EVENT_RX_DMADONE | (recv_len << 8));

    DMA_ClearFlag(uart->dma.rx_gl_flag);
}
#endif

/**
 * @brief: Uart common interrupt process. This need add to uart ISR.
 * @param {type} 
 * @return {type} 
 */
static void uart_isr(struct ocm_datastream_device *serial)
{
    struct ocm_uart_device *uart = (struct ocm_uart_device *)serial->parent.user_data;

    if (USART_GetITStatus(uart->uart_device, USART_IT_RXNE) != RESET)
    {
        /* 奇偶校验错误检测，无校验错误 */
        if (USART_GetFlagStatus(uart->uart_device, USART_FLAG_PE) == RESET)
        {
            ocm_hw_datastream_isr(serial, OCM_USART_EVENT_RX_IND) ；
        }
        /* clear interrupt */
        USART_ClearITPendingBit(uart->uart_device, USART_IT_RXNE);
    }

    /* 检测到总线空闲，USART_CR1的IDLEIE为1 */
    if (USART_GetFlagStatus(uart->uart_device, USART_IT_IDLE) != RESET)
    {
        dma_uart_rx_idle_isr(serial);
    }

    /* 一帧数据发送完成后，并且TXE=1时即数据已经被转移到移位寄存器中， USART_CR1中的TCIE为1产生中断*/
    if (USART_GetFlagStatus(uart->uart_device, USART_IT_TC) != RESET)
    {
        /* clear interrupt */
        if (serial->parent.open_flag & OCM_DEVICE_FLAG_INT_TX)
        {
            ocm_hw_datastream_isr(serial, OCM_USART_EVENT_TX_DONE);
        }
        USART_ITConfig(uart->uart_device, USART_IT_TC, DISABLE);
        USART_ClearITPendingBit(uart->uart_device, USART_IT_TC);
    }

    /* 检测到过载错误，当RXNE仍是1时，当前被接收在移位寄存器中的数据，需要传送至RDR寄存器是，硬件将该位 置位 */
    if (USART_GetFlagStatus(uart->uart_device, USART_FLAG_ORE) == SET)
    {
        USART_ReceiveData(uart->uart_device);
    }
}

/* init ops */
static const struct ocm_datastream_ops uart_ops =
    {
        .configure = uart_configure,
        .control = uart_control,
        .putc = uart_putc,
        .getc = uart_getc,
};

/* UART1 device driver structure */
struct ocm_uart_device uart1 =
    {
        USART1,
        USART1_IRQn,
#ifdef OCM_SERIAL_USING_DMA
        {
            DMA1_Channel5,
            DMA1_FLAG_GL5,
            DMA1_Channel5_IRQn,
            0,
        },
#endif
};

struct ocm_datastream_device serial1;
struct usart_configure config1 = OCM_SERIAL_CONFIG_DEFAULT;

void USART1_IRQHandler(void)
{
    uart_isr(&serial1);
}

#ifdef OCM_SERIAL_USING_DMA
void DMA1_Channel5_IRQHandler(void)
{
    dma_rx_done_isr(&serial1);
}
#endif

/* UART2 device driver structure */
struct ocm_uart_device uart2 =
    {
        USART2,
        USART2_IRQn,
#ifdef OCM_SERIAL_USING_DMA
        {
            DMA1_Channel5,
            DMA1_FLAG_GL5,
            DMA1_Channel5_IRQn,
            0,
        },
#endif
};

struct ocm_datastream_device serial2;
struct usart_configure config2 = OCM_SERIAL_CONFIG_DEFAULT;

void USART2_IRQHandler(void)
{
    uart_isr(&serial2);
}

#ifdef OCM_SERIAL_USING_DMA
void DMA1_Channel6_IRQHandler(void)
{
    dma_rx_done_isr(&serial2);
}
#endif

/* UART3 device driver structure */
struct ocm_uart_device uart3 =
    {
        USART3,
        USART3_IRQn,
#ifdef OCM_SERIAL_USING_DMA
        {
            DMA1_Channel5,
            DMA1_FLAG_GL5,
            DMA1_Channel5_IRQn,
            0,
        },
#endif
};

struct ocm_datastream_device serial3;
struct usart_configure config3 = OCM_SERIAL_CONFIG_DEFAULT;

void USART3_IRQHandler(void)
{
    uart_isr(&serial3);
}

#ifdef OCM_SERIAL_USING_DMA
void DMA1_Channel3_IRQHandler(void)
{
    dma_rx_done_isr(&serial2);
}
#endif

static void RCC_Configuration(void)
{
    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    /* Enable UART clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
}

static void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    /* Configure USART Rx/tx PIN */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = UART1_GPIO_RX;
    GPIO_Init(UART1_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = UART1_GPIO_TX;
    GPIO_Init(UART1_GPIO, &GPIO_InitStructure);

    /* Configure USART Rx/tx PIN */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = UART2_GPIO_RX;
    GPIO_Init(UART2_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = UART2_GPIO_TX;
    GPIO_Init(UART2_GPIO, &GPIO_InitStructure);

    /* Configure USART Rx/tx PIN */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = UART3_GPIO_RX;
    GPIO_Init(UART3_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = UART3_GPIO_TX;
    GPIO_Init(UART3_GPIO, &GPIO_InitStructure);
}

static void NVIC_Configuration(struct ocm_uart_device *uart)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = uart->irq;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

#ifdef OCM_SERIAL_USING_DMA
static void DMA_Configuration(struct ocm_datastream_device *serial)
{
    struct ocm_uart_device *uart = (struct ocm_uart_device *)serial->parent.user_data;
    struct ocm_datastream_rx_fifo *rx_fifo = (struct ocm_datastream_rx_fifo *)serial->serial_rx;
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    uart->dma.setting_recv_len = serial->config.bufsz;

    /* enable transmit idle interrupt */
    USART_ITConfig(uart->uart_device, USART_IT_IDLE, ENABLE);

    /* DMA clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

    /* rx dma config */
    DMA_DeInit(uart->dma.rx_ch);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (uart->uart_device->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)rx_fifo->buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = serial->config.bufsz;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(uart->dma.rx_ch, &DMA_InitStructure);
    DMA_ClearFlag(uart->dma.rx_gl_flag);
    DMA_ITConfig(uart->dma.rx_ch, DMA_IT_TC, ENABLE);
    USART_DMACmd(uart->uart_device, USART_DMAReq_Rx, ENABLE);
    DMA_Cmd(uart->dma.rx_ch, ENABLE);

    /* rx dma interrupt config */
    NVIC_InitStructure.NVIC_IRQChannel = uart->dma.rx_irq_ch;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
#endif

/**
 * @brief: 
 * @param {type} 
 * @return {type} 
 */
void ocm_hw_usart_init(void)
{
    struct ocm_uart_device *uart;

    RCC_Configuration();
    GPIO_Configuration();

    uart = &uart1;
    config1.baud_rate = BAUD_RATE_9600;

    serial1.ops = &uart_ops;
    serial1.config = (void *)(&config1);

    NVIC_Configuration(uart);

    /* register UART1 device */
    ocm_hw_datastream_register(&serial1, "uart1",
                               OCM_DEVICE_FLAG_RDWR | OCM_DEVICE_FLAG_INT_RX |
                                   OCM_DEVICE_FLAG_INT_TX | OCM_DEVICE_FLAG_DMA_RX,
                               uart);

    uart = &uart2;
    config2.baud_rate = BAUD_RATE_9600;

    serial2.ops = &uart_ops;
    serial2.config = (void *)(&config2);

    NVIC_Configuration(uart);

    /* register UART2 device */
    ocm_hw_datastream_register(&serial2, "uart2",
                               OCM_DEVICE_FLAG_RDWR | OCM_DEVICE_FLAG_INT_RX |
                                   OCM_DEVICE_FLAG_INT_TX | OCM_DEVICE_FLAG_DMA_RX,
                               uart);

    uart = &uart3;
    config3.baud_rate = BAUD_RATE_9600;

    serial3.ops = &uart_ops;
    serial3.config = (void *)(&config3);

    NVIC_Configuration(uart);

    /* register UART3 device */
    ocm_hw_datastream_register(&serial3, "uart3",
                               OCM_DEVICE_FLAG_RDWR | OCM_DEVICE_FLAG_INT_RX |
                                   OCM_DEVICE_FLAG_INT_TX | OCM_DEVICE_FLAG_DMA_RX,
                               uart);
}
