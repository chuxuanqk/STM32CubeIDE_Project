/*
 * uart.c
 *
 *  Created on: 2020年9月14日
 *      Author: Saber
 */

#include "channel/bsp_uart.h"
#include "misc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "string.h"

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

/**
 * @description: 重载_write,提供printf  
 * @param {*}
 * @return {*}
 */
int _write(int fd, char *pBuffer, int size)
{
  for (int i = 0; i < size; i++)
  {
    while ((DEBUG_USART->SR & 0X40) == 0)
      ;                                    //等待上一次串口数据发送完成
    DEBUG_USART->DR = (uint8_t)pBuffer[i]; //写DR,串口1将发送数据
  }
  return size;
}

void __uart_setflag(USART_TypeDef *uartx, enum UART_FLAG uflag, bool status);
struct uart_data *__get_duartx(USART_TypeDef *uartx);

static void RCC_Configuration(void)
{
  /* Enable UART GPIO clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
  /* Enable UART clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

#ifdef USING_UART2
  /* Enable UART GPIO clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
  /* Enable UART clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
#endif

  /* Enable UART GPIO clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
  /* Enable UART clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
}

static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

#ifdef USING_UART1
  /* Enable the USART1 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;         //通道设置为串口1中断（故后面应选择在“void USART1_IRQHandler(void)”开中断）
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //中断占先等级0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        //中断响应优先级0
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           //打开中断
  NVIC_Init(&NVIC_InitStructure);

  //DMA发送中断设置
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
#endif

#ifdef USING_UART2
  /* Enable the USART2 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;         //通道设置为串口1中断（故后面应选择在“void USART1_IRQHandler(void)”开中断）
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //中断占先等级0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;        //中断响应优先级0
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           //打开中断
  NVIC_Init(&NVIC_InitStructure);

  //DMA发送中断设置
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

#endif

#ifdef USING_UART3
  /* Enable the USART3 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;         //通道设置为串口1中断（故后面应选择在“void USART1_IRQHandler(void)”开中断）
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //中断占先等级0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;        //中断响应优先级0
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           //打开中断
  NVIC_Init(&NVIC_InitStructure);

  //DMA发送中断设置
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

#endif
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

#ifdef USING_UART2
  /* Configure USART Rx/tx PIN */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = UART2_GPIO_RX;
  GPIO_Init(UART2_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = UART2_GPIO_TX;
  GPIO_Init(UART2_GPIO, &GPIO_InitStructure);
#endif

#ifdef USING_UART3
  /* Configure USART Rx/tx PIN */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = UART3_GPIO_RX;
  GPIO_Init(UART3_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = UART3_GPIO_TX;
  GPIO_Init(UART3_GPIO, &GPIO_InitStructure);
#endif
}

static void UART_Configuration(struct uart_device *uart, struct uart_configure *cfg)
{
  USART_InitTypeDef USART_InitStructure;
  USART_ClockInitTypeDef USART_ClockInitStructure;

  USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;     //提升USART时钟时使能还是失能，钟低电平活动
  USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;           //指定SLCK引脚上时钟的极性
  USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;         //时钟第二个边缘进行数据捕获
  USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable; //在SCLK引脚上输出最后发送的那个数据字的脉冲不从SCLK输出
  USART_ClockInit(uart->uartx, &USART_ClockInitStructure);

  USART_DeInit(uart->uartx);

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
  else if (cfg->stop_bits == STOP_BITS_1_5)
  {
    USART_InitStructure.USART_StopBits = USART_StopBits_1_5;
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
  USART_Init(uart->uartx, &USART_InitStructure);

  //串口采用DMA发送
  USART_DMACmd(uart->uartx, USART_DMAReq_Tx, ENABLE);

#ifdef USE_USART_DMA_RX
  USART_ITConfig(uart->uartx, USART_IT_IDLE, ENABLE);
  USART_ClearITPendingBit(uart->uartx, USART_IT_IDLE);
#else
  USART_ITConfig(uart->uartx, USART_IT_RXNE, ENABLE);  // 接收中断使能
  USART_ClearITPendingBit(uart->uartx, USART_IT_RXNE); // 清除接收完成中断
#endif
  /* Enable USART */
  USART_Cmd(uart->uartx, ENABLE);
}

#ifdef USE_USART_DMA_RX
static void UART_DMA_Rx_Config(struct uart_data *pdata)
{
  DMA_InitTypeDef DMA_InitStructure;
  struct uart_device *uart = pdata->uart_device;

  // 开启DMA时钟
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  // 设置DMA源地址：串口数据寄存器地址*/
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (uart->uartx->DR);
  // 内存地址(要传输的变量的指针)
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pdata->stream_rx;
  // 方向：从内存到外设
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  // 传输大小
  DMA_InitStructure.DMA_BufferSize = USART_RX_BUFF_SIZE;
  // 外设地址不增
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  // 内存地址自增
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  // 外设数据单位
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  // 内存数据单位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  // DMA模式，一次或者循环模式
  //DMA_InitStructure.DMA_Mode = DMA_Mode_Normal ;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  // 优先级：中
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  // 禁止内存到内存的传输
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  // 配置DMA通道
  DMA_Init(&uart->dma_tx.rx_irq_ch, &DMA_InitStructure);
  // 清除DMA标志
  DMA_ClearFlag(uart->dma_tx.rx_gl_flag);
}
#endif

#ifdef USE_USART_DMA_TX
static void UART_DMA_Tx_Config(struct uart_data *pdata)
{
  DMA_InitTypeDef DMA_InitStructure;
  struct uart_device *uart = pdata->uart_device;

  //启动DMA时钟
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  //通道配置
  DMA_DeInit(uart->dma_tx.tx_ch);
  //外设地址
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (uart->uartx->DR);
  //内存地址
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pdata->stream_tx;
  //dma传输方向单向
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  //设置DMA在传输时缓冲区的长度
  DMA_InitStructure.DMA_BufferSize = UART_DMA_RB_BUFSZ;
  //设置DMA的外设递增模式，一个外设
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  //设置DMA的内存递增模式
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  //外设数据字长
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  //内存数据字长
  DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
  // DMA模式，一次或者循环模式
  //DMA_InitStructure.DMA_Mode = DMA_Mode_Normal ;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  //设置DMA的优先级别
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  //设置DMA的2个memory中的变量互相访问
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

  DMA_Init(uart->dma_tx.tx_ch, &DMA_InitStructure);
  // 清除DMA标志
  //  DMA_ClearFlag(uart->dma_tx.tx_gl_flag);
  DMA_ITConfig(uart->dma_tx.tx_ch, DMA_IT_TC, ENABLE); // dma传输中断
  DMA_Cmd(uart->dma_tx.tx_ch, DISABLE);
}
#endif

static void uart_isr(struct uart_data *puart)
{
  USART_TypeDef *uart = puart->uart_device->uartx;

  if (USART_GetITStatus(uart, USART_IT_RXNE) != RESET)
  {
    if (uart_getFlagStatus(uart, UART_FLAG_RC) == false)
    {
      puart->stream_rx[puart->rx_index] = (uint8_t)USART_ReceiveData(uart);
      ++puart->rx_index;
      puart->rx_index = puart->rx_index % UART_DMA_RB_BUFSZ;

      if (puart->rx_index == puart->recv_len)
      {
        puart->rx_index = 0;
        __uart_setflag(uart, UART_FLAG_RC, true);
      }
    }

    /* clear interrupt */
    USART_ClearITPendingBit(uart, USART_IT_RXNE);
  }

#ifdef USE_USART_DMA_RX
  /* 检测到总线空闲，USART_CR1的IDLEIE为1 */
  if (USART_GetITStatus(uart, USART_IT_IDLE) != RESET)
  {
  }
#endif

  /* 一帧数据发送完成后，并且TXE=1时即数据已经被转移到移位寄存器中， USART_CR1中的TCIE为1产生中断*/
  if (USART_GetITStatus(uart, USART_IT_TC) != RESET)
  {
    // 传输完成，将其UART_FLAG_xx_TC复位
#ifdef USE_USART_DMA_TX
    __uart_setflag(uart, UART_FLAG_DMA_TC, false);
#elif
    __uart_setflag(uart, UART_FLAG_TC, false);
#endif

    /* clear interrupt */
    uart_tc_isr_hook();
    USART_ClearITPendingBit(uart, USART_IT_TC);
    USART_ITConfig(uart, USART_IT_TC, DISABLE);
  }

  if (USART_GetITStatus(uart, USART_IT_TXE) != RESET)
  {
  }

  /* 检测到过载错误，当RXNE仍是1时，当前被接收在移位寄存器中的数据，需要传送至RDR寄存器是，硬件将该位 置位 */
  if (USART_GetITStatus(uart, USART_FLAG_ORE) == SET)
  {
    USART_ReceiveData(uart);
  }
}

static void dma_isr(struct uart_data *puart)
{
  struct uart_dma_tx *dma_tx = &puart->uart_device->dma_tx;

  if (DMA_GetITStatus(dma_tx->tx_tc_IT) != RESET)
  {
    if (puart->uart_device->uartx == USART3)
    {
      USART_ClearITPendingBit(puart->uart_device->uartx, USART_IT_TC);
      USART_ITConfig(puart->uart_device->uartx, USART_IT_TC, ENABLE); // 传输中断使能
    }

    uart_dmaisr_hook();
    puart->tx_flag = false; // 使能再次发送
    DMA_Cmd(dma_tx->tx_ch, DISABLE);
    DMA_ClearFlag(dma_tx->tx_gl_flag);
  }
}

#ifdef USING_UART1
struct uart_configure config1 = UART_CONFIG_DEFAULT;
struct uart_device uart1 = {
    USART1,
    USART1_IRQn,

#ifdef USE_USART_DMA_RX
    .dma_rx = {
        DMA1_Channel5,
        DMA1_FLAG_GL5,
        DMA1_Channel5_IRQn,
        0,
    },
#endif

#ifdef USE_USART_DMA_TX
    .dma_tx = {
        DMA1_Channel4,
        DMA1_FLAG_GL4,
        DMA1_Channel4_IRQn,
        DMA1_IT_TC4,
    }
#endif

};
struct uart_data duart1 = {
    .uart_device = &uart1,
    .rx_flag = false,
    .tx_flag = false,
    .rx_index = 0,
    .recv_len = 2,
};

void USART1_IRQHandler(void)
{
  uart_isr(&duart1);
}

void DMA1_Channel4_IRQHandler(void)
{
  dma_isr(&duart1);
}

#endif

#ifdef USING_UART2
struct uart_configure config2 = UART_CONFIG_DEFAULT;
struct uart_device uart2 = {
    USART2,
    USART2_IRQn,

#ifdef USE_USART_DMA_RX
    .dma_rx = {
        DMA1_Channel6,
        DMA1_FLAG_GL6,
        DMA1_Channel6_IRQn,
        0,
    },
#endif

#ifdef USE_USART_DMA_TX
    .dma_tx = {
        DMA1_Channel7,
        DMA1_FLAG_GL7,
        DMA1_Channel7_IRQn,
        DMA1_IT_TC7,
    }
#endif

};
struct uart_data duart2 = {
    .uart_device = &uart2,
    .rx_flag = false,
    .tx_flag = false,
    .rx_index = 0,
    .recv_len = 2,
};

void USART2_IRQHandler(void)
{
  uart_isr(&duart2);
}

void DMA1_Channel7_IRQHandler(void)
{
  dma_isr(&duart2);
}
#endif

#ifdef USING_UART3
struct uart_configure config3 = UART_CONFIG_DEFAULT;
struct uart_device uart3 = {
    USART3,
    USART3_IRQn,

#ifdef USE_USART_DMA_RX
    .dma_rx = {
        DMA1_Channel3,
        DMA1_FLAG_GL3,
        DMA1_Channel3_IRQn,
        0,
    },
#endif

#ifdef USE_USART_DMA_TX
    .dma_tx = {
        DMA1_Channel2,
        DMA1_FLAG_GL2,
        DMA1_Channel2_IRQn,
        DMA1_IT_TC2,
    }
#endif
};
struct uart_data duart3 = {
    .uart_device = &uart3,
    .rx_flag = false,
    .tx_flag = false,
    .rx_index = 0,
    .recv_len = 2,
};

void USART3_IRQHandler(void)
{
  uart_isr(&duart3);
}

void DMA1_Channel2_IRQHandler(void)
{
  dma_isr(&duart3);
}
#endif

/**
 * @description: 
 * @param {type} 
 * @return {type} 
 */
struct uart_data *__get_duartx(USART_TypeDef *uartx)
{
  struct uart_data *pstream;

#ifdef USING_UART1
  if (USART1 == uartx)
    pstream = &duart1;
#endif
#ifdef USING_UART2
  else if (USART2 == uartx)
    pstream = &duart2;
#endif
#ifdef USING_UART3
  else if (USART3 == uartx)
    pstream = &duart3;
#endif

  return pstream;
}

/**
 * @description: 内部函数，提供修改相关FLAG状态
 * @param {type} 
 * @return {type} 
 */
void __uart_setflag(USART_TypeDef *uartx, enum UART_FLAG uflag, bool status)
{
  struct uart_data *pstream = __get_duartx(uartx);

  switch (uflag)
  {
  case UART_FLAG_DMA_TC:
    pstream->tx_flag = status;
    break;

  case UART_FLAG_DMA_RC:
    pstream->rx_flag = status;
    break;

  case UART_FLAG_TC:
    pstream->tx_flag = status;
    break;

  case UART_FLAG_RC:
    pstream->rx_flag = status;
    break;

  case UART_FLAG_IDLE:
    break;

  default:
    break;
  }
}

/**
 * @description: 
 * @param {*}
 * @return {*}
 */
bool uart_getFlagStatus(USART_TypeDef *uartx, enum UART_FLAG uflag)
{
  bool ret = false;
  struct uart_data *pstream = __get_duartx(uartx);

  switch (uflag)
  {
  case UART_FLAG_DMA_TC:
    ret = (pstream->tx_flag == true) ? true : false;
    break;

  case UART_FLAG_DMA_RC:
    ret = (pstream->rx_flag == true) ? true : false;
    break;

  case UART_FLAG_TC:
    ret = (pstream->tx_flag == true) ? true : false;
    break;

  case UART_FLAG_RC:
    ret = (pstream->rx_flag == true) ? true : false;
    break;

  case UART_FLAG_IDLE:
    if ((pstream->tx_flag == false) && (pstream->rx_flag == false))
      ret = true;
    break;

  default:
    break;
  }

  return ret;
}

static void clear_stream_rx(USART_TypeDef *uartx)
{
  struct uart_data *pstream = __get_duartx(uartx);
}

int16_t uart_getc(USART_TypeDef *uartx)
{
  int16_t ch = -1;

  if (uartx->SR & USART_FLAG_RXNE)
  {
    ch = uartx->DR & 0x01FF;
  }
  return ch;
}

/**
 * @description: 设置需要接收的字节长度，建议在使用uart_write给设备发送信息之前，设置设备返回的数据字节长度
 * @param {type} 
 * @return {type} 
 */
int16_t uart_SetRead_Size(USART_TypeDef *uartx, uint8_t size)
{
  struct uart_data *pstream = __get_duartx(uartx);

  pstream->recv_len = size;

  return pstream->recv_len;
}

int16_t uart_read(USART_TypeDef *uartx, uint8_t *pbuf, uint8_t size)
{
  uint8_t i = 0;
  int16_t ret = -1;
  struct uart_data *pstream = __get_duartx(uartx);

  if (size == 0)
    return ret;

  if (pstream->recv_len != size)
    pstream->recv_len = size;

  if (uart_getFlagStatus(uartx, UART_FLAG_RC) == true)
  {
    __uart_setflag(uartx, UART_FLAG_RC, false); // 使能下次接收

    for (i = 0; i < size; i++)
      pbuf[i] = pstream->stream_rx[i];

    ret = size;
    // pstream->rx_index = 0;

    memset(pstream->stream_rx, 0, sizeof(pstream->stream_rx)); // refflush
  }

  return ret;
}

int16_t uart_write(USART_TypeDef *uartx, uint8_t *pbuf, uint8_t buf_size)
{
  uint8_t size = buf_size;
  int16_t ret = -1;
  uint8_t i = 0;
  struct uart_data *pstream = __get_duartx(uartx);

  if (uart_getFlagStatus(uartx, UART_FLAG_DMA_TC) == false)
  {
    ret = size;
    for (i = 0; i < size; i++)
      pstream->stream_tx[i] = pbuf[i];

#ifdef USE_USART_DMA_TX
    __uart_setflag(uartx, UART_FLAG_DMA_TC, true);
    //DMA方式发送,设置数据长度
    DMA_SetCurrDataCounter(pstream->uart_device->dma_tx.tx_ch, size);
    //启动DMA发送
    DMA_Cmd(pstream->uart_device->dma_tx.tx_ch, ENABLE);
#elif
    __uart_setflag(uartx, UART_FLAG_TC, true);
    // 按字节发送
    for (i = 0; i < size; i++)
      USART_SendData(pstream->uart_device->uartx, pbuf[0]);
#endif
  }
  return ret;
}

void Hw_Uart_Init(void)
{
  struct uart_device *uart;

  RCC_Configuration();
  NVIC_Configuration();
  GPIO_Configuration();

#ifdef USING_UART1
  config1.baud_rate = BAUD_RATE_115200;
  uart = &uart1;

  UART_Configuration(uart, &config1);
  UART_DMA_Tx_Config(&duart1);
#endif

#ifdef USING_UART2
  uart = &uart2;
  config2.baud_rate = BAUD_RATE_9600;

  UART_Configuration(uart, &config2);
  UART_DMA_Tx_Config(&duart2);
#endif

#ifdef USING_UART3
  uart = &uart3;
  config3.baud_rate = BAUD_RATE_9600;

  UART_Configuration(uart, &config3);
  UART_DMA_Tx_Config(&duart3);
#endif
}
