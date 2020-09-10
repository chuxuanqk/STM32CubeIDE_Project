/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.onecm.com
**
**------------------------------------File Info----------------------------------------------------------*/
/*
 * @Description: 
 * @version: 
 * @Author: Saber
 * @Date: 2020-09-02 11:18:17
 * @FilePath: \OcmDataStream\driver\can.c
 * @LastEditors: Saber
 * @LastEditTime: 2020-09-04 15:21:01
 * @**
 * **********************************************************************************************************
 */
#include "can.h"

#include "misc.h"
#include "stm32f10x_i"
#include "stm32f10x_rcc.h"
#include "stm32f10x_can.h"
#include "stm32f10x_gpio.h"

/* STM32 can driver */
struct ocm_can_device
{
    CAN_TypeDef *can_device;
    IRQn_Type irq;
};

/* CAN1 */
#define CAN1_GPIO_RX GPIO_Pin_11
#define CAN1_GPIO_TX GPIO_Pin_12
#define CAN1_GPIO GPIO_A

/* can protect function  */
static ocm_err_t can_configure(struct ocm_datastream_device *can, struct can_configure *cfg)
{
}

static ocm_err_t can_control(struct ocm_datastream_device *can, ocm_cmd_t cmd, void *arg)
{
}

static ocm_base_t can_send(struct ocm_datastream_device *can, char *buf)
{
}

static ocm_base_t can_recv(struct ocm_datastream_device *can, char *buf)
{
}

/**
 * @brief: Can common interrupt process.This need add to USB_LP_CAN1_RX0_IRQHandler.
 * @param {type} 
 * @return {type} 
 */
static void can_isr(struct ocm_datastream_device *can)
{
    struct ocm_can_device *bxcan = (struct ocm_can_device *)can->parent.user_data;

    if (CAN_GetFlagStatus(bxcan->can_device, CAN_FLAG_FMP0))
    {
    }
}

/* init ops */
static const struct ocm_datastream_ops can_ops =
    {
        .configure = can_configure,
        .control = can_control,
        .sendmsg = can_send,
        .recvmsg = can_recv,
};

struct ocm_can_device can1 =
    {
        CAN1,
        USB_HP_CAN1_TX_IRQn,
};

struct ocm_datastream_device can_stream;
struct can_configure can1_config = OCM_CAN_CONFIG_DEFAULT;

/* CAN1 Send IRQ */
void USB_HP_CAN1_TX_IRQHandler(void) //CAN1发送中断
{
    CAN_ClearITPendingBit(CAN1, CAN_IT_TME);
}

/* CAN1 RX0 IRQ */
void USB_LP_CAN1_RX0_IRQHandler(void)
{
    can_isr(&can_stream);
}

static void RCC_Configuration(void)
{
    /* Enable CAN1 GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);
    /* Enable CAN1 clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
}

static void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    /* COnfigure CAN1 Rx/tx PIN */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Pin = CAN1_GPIO_RX;
    GPIO_Init(CAN1_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = CAN1_GPIO_TX;
    GPIO_Init(CAN1_GPIO, &GPIO_InitStructure);
}

static void NVIC_Configuration(struct ocm_can_device *can)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the CAN1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = can->irq;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 中断抢占优先级0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        // 中断响应优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void CAN_Configuration(void)
{
    CAN_InitTypeDef CAN_InitStructure;
    CAN_FilterInitTypeDef CAN_FilterInitStructure;

    /* CAN cell init */
    CAN_InitStructure.CAN_TTCM = DISABLE;         /* 时间触发禁止, 时间触发：CAN硬件的内部定时器被激活，并且被用于产生时间戳 */
    CAN_InitStructure.CAN_ABOM = ENABLE;          /* 自动离线禁止，自动离线：一旦硬件监控到128次11个隐性位，就自动退出离线状态。在这里要软件设定后才能退出 */
    CAN_InitStructure.CAN_AWUM = DISABLE;         /* 自动唤醒禁止，有报文来的时候自动退出休眠	*/
    CAN_InitStructure.CAN_NART = DISABLE;         /* 报文重传, 如果错误一直传到成功止，否则只传一次 */
    CAN_InitStructure.CAN_RFLM = DISABLE;         /* 接收FIFO锁定, 1--锁定后接收到新的报文摘不要，0--接收到新的报文则覆盖前一报文	*/
    CAN_InitStructure.CAN_TXFP = ENABLE;          /* 发送优先级  0---由标识符决定  1---由发送请求顺序决定	*/
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal; /* 模式	*/
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;      /* 重新同步跳宽，只有can硬件处于初始化模式时才能访问这个寄存器 */
    CAN_InitStructure.CAN_BS1 = CAN_BS1_4tq;      /* 时间段1 */
    CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;      /* 时间段2 */
    CAN_InitStructure.CAN_Prescaler = 45;         /* 波特率预分频数 */

    if (CAN_Init(CAN1, &CAN_InitStructure) == CANINITFAILED)
    {
        /* 初始化时先设置CAN_MCR的初始化位 */
        /* 然后查看硬件是否真的设置了CAN_MSR的初始化位来确认是否进入了初始化模式  */
    }

    /* 配置CAN过滤器 */
    /* 32位对应的id */
    /* stdid[10:0]，extid[17:0],ide,rtr	*/
    /* 16位对应的id */
    /* stdid[10:0],ide,rtr,extid[17:15] */
    /* 一般使用屏蔽模式	*/
    /* 要注意的是fifo接收存满了中断，还有就是fifo的概念，即取的一直是最早那一个数据， 要释放才能取下一个数据 */
    /* 常使用的中断有 */
    /* 1,有信息中断，即fifo挂号中断 */
    /* 2,fifo满中断	*/
    /* 3,fifo满之后又有信息来则中断，即fifo溢出中断	*/
    CAN_FilterInitStructure.CAN_FilterNumber = 0;                    /* 过滤器0 */
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;  /* 屏敝模式 */
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; /* 32位 */
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;               /* 以下四个都为0, 表明不过滤任何id */
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0; /* 关联FIFO0 */
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE); /* 挂号中断, 进入中断后读fifo的报文函数释放报文清中断标志 */
    //CAN_ITConfig(CAN1,CAN_IT_TME, ENABLE);      // 使能发送中断，咱不需要
    CAN_FilterInit(&CAN_FilterInitStructure);
}

static void DMA_Configuration(struct ocm_datastream_device *can)
{
}

/**
 * @brief: 
 * @param {type} 
 * @return {type} 
 */
void ocm_hw_can_init(void)
{
    struct ocm_can_device *can;

    RCC_Configuration();
    GPIO_Configuration();

    can = &can1;

    can_stream.ops = can_ops;
    /* init can configurate */
    can_stream.config = (void *)(&can1_config);

    CAN_DeInit(can->can_device);
    NVIC_Configuration(can);

    /* register can1 device */
    ocm_hw_datastream_register(&can_stream, "can1", OCM_DEVICE_FLAG_RDWR | OCM_DEVICE_FLAG_INT_RX | OCM_DEVICE_FLAG_INT_TX,
                               can);
}
