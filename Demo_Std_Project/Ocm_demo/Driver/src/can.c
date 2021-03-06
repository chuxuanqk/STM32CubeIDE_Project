/*
 * can.c
 *
 *  Created on: 2020年9月17日
 *      Author: Administrator
 */
#include "can.h"
#include "stdbool.h "

#define CAN_DATA_BUFF_MAX 109

// typedef struct
// {
// 	uint16_t src_net_id;
// 	uint16_t des_net_id;
// 	uint8_t data_size;
// 	uint8_t function;
// 	uint8_t port;
// 	uint8_t data_buf[CAN_DATA_BUFF_MAX];
// } CAN_DataTypeDef;

struct can_rx_data
{
	bool flag;
	uint8_t get_index; // 上层应用获取数据下标
	uint8_t put_index; // 串口接收数据下标
	uint8_t data_buf[CAN_DATA_BUFF_MAX];
};

struct can_rx_data can_receive; // can接收数据缓存

static void RCC_Configuration(void)
{
	/* CAN Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
}

static void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable CAN1 RX0 interrupt IRQ channel */
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

static void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure CAN pin: RX */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure CAN pin: TX */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void CAN_Configuration(void)
{
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef CAN_FilterInitStructure;

	CAN_DeInit(CAN1);
	CAN_StructInit(&CAN_InitStructure);

	/* CAN cell init */
	CAN_InitStructure.CAN_TTCM = DISABLE;		  /* 时间触发禁止, 时间触发：CAN硬件的内部定时器被激活，并且被用于产生时间戳 */
	CAN_InitStructure.CAN_ABOM = ENABLE;		  /* 自动离线禁止，自动离线：一旦硬件监控到128次11个隐性位，就自动退出离线状态。在这里要软件设定后才能退出 */
	CAN_InitStructure.CAN_AWUM = DISABLE;		  /* 自动唤醒禁止，有报文来的时候自动退出休眠	*/
	CAN_InitStructure.CAN_NART = DISABLE;		  /* 报文重传, 如果错误一直传到成功止，否则只传一次 */
	CAN_InitStructure.CAN_RFLM = DISABLE;		  /* 接收FIFO锁定, 1--锁定后接收到新的报文摘不要，0--接收到新的报文则覆盖前一报文	*/
	CAN_InitStructure.CAN_TXFP = ENABLE;		  /* 发送优先级  0---由标识符决定  1---由发送请求顺序决定	*/
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal; /* 模式	*/
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;	  /* 重新同步跳宽，只有can硬件处于初始化模式时才能访问这个寄存器 */
	CAN_InitStructure.CAN_BS1 = CAN_BS1_4tq;	  /* 时间段1 */
	CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;	  /* 时间段2 */
	CAN_InitStructure.CAN_Prescaler = 45;		  /* 波特率预分频数 */

	/* 波特率计算方法 */
	/* CANbps= Fpclk/((BRP+1)*((Tseg1+1)+(Tseg2+1)+1)  此处计算为  CANbps=36000000/(45*(4+3+1))=100kHz */ //此处Tseg1+1 = CAN_BS1_8tp
	/* 配置大方向: Tseg1>=Tseg2  Tseg2>=tq; Tseg2>=2TSJW */

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
	CAN_FilterInitStructure.CAN_FilterNumber = 0;					 /* 过滤器0 */
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;	 /* 屏敝模式 */
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; /* 32位 */
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;				 /* 以下四个都为0, 表明不过滤任何id */
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0; /* 能够通过该过滤器的报文存到fifo0中 */
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE); /* 挂号中断, 进入中断后读fifo的报文函数释放报文清中断标志 */
	//CAN_ITConfig(CAN1,CAN_IT_TME, ENABLE);      //发送中断
	CAN_FilterInit(&CAN_FilterInitStructure);
}

//CAN1 Send IRQ
void USB_HP_CAN1_TX_IRQHandler(void) //CAN1发送中断
{
}

//CAN1 RX0 IRQ
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	CanRxMsg RxMessage;

	RxMessage.IDE = CAN_ID_EXT;
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

	can_rx_isr_hook(&RxMessage); // 对接收的数据进行处理

	CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
}

/**
 * @description: can 数据发送
 * @param {type} TxMessage  需要由外部设置ExtId
 * @return {type} 
 */
void can_send(CanTxMsg *TxMessage, uint8_t *buf, uint8_t size)
{
	uint16_t i = 0;
	uint16_t index = 0;
	uint8_t flag = CAN_NO_MB;

	TxMessage->RTR = CAN_RTR_DATA;
	TxMessage->IDE = CAN_ID_EXT;
	TxMessage->DLC = 8;

	while (size / 8)
	{
		for (i = 0; i < 8; i++)
			TxMessage->Data[i] = buf[index++];

		while (flag == CAN_NO_MB)
			flag = CAN_Transmit(CAN1, TxMessage);

		size -= 8;
	}

	if (size > 0)
	{
		TxMessage->DLC = size;

		for (i = 0; i < size; i++)
			TxMessage->Data[i] = buf[index++];

		while (flag == CAN_NO_MB)
			flag = CAN_Transmit(CAN1, TxMessage);
	}
}

/**
 * @description: Cancels a transmit request.
 * @param {type} 
 * @return {type} 
 */
void can_Tx_clear(void)
{
	CAN_CancelTransmit(CAN1, 0);
	CAN_CancelTransmit(CAN1, 1);
	CAN_CancelTransmit(CAN1, 2);
}

void hw_can_init(void)
{
	RCC_Configuration();
	NVIC_Configuration();
	GPIO_Configuration();
	CAN_Configuration();
}
