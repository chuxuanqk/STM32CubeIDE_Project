/*
 * chan_data_can1.c
 *
 *  Created on: 2020年1月3日
 *      Author: Fox
 */

#include "channel/chan_data_can1.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_can.h"
#include "stm32f10x_rcc.h"

Send_Rec_Data_Structure CAN1_Receive;
Send_Rec_Data_Structure CAN1_Send;

//多通道CAN接收缓冲区
Send_Rec_Data_Structure CAN1_Receive_Buf[Net_IDs_All + 1];

inline Send_Rec_Data_Structure *Get_Can1_Receive(void)
{
    return &CAN1_Receive;
}

inline Send_Rec_Data_Structure *Get_Can1_Send(void)
{
    return &CAN1_Send;
}

//CAN1中断初始化
void CAN1_NVIC_Init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    // Enable CAN1 RX0 interrupt IRQ channel
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USB_LP_CAN1_RX0_Preemption_Priority;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = USB_LP_CAN1_RX0_Sub_Priority;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable CAN1 TX0 interrupt IRQ channel
    // NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
    // NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USB_LP_CAN1_TX0_Preemption_Priority;
    // NVIC_InitStructure.NVIC_IRQChannelSubPriority = USB_LP_CAN1_TX0_Sub_Priority;
    // NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    // NVIC_Init(&NVIC_InitStructure);
}

//CAN1初始化
void CAN1_Configuration(void)
{
    CAN_InitTypeDef CAN_InitStructure;
    CAN_FilterInitTypeDef CAN_FilterInitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* CAN Periph clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    //GPIO_PinRemapConfig(GPIO_Remap1_CAN1,ENABLE);

    /* Configure CAN pin: RX */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure CAN pin: TX */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* CAN register init */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE); //CAN1频率使能
    CAN_DeInit(CAN1);
    CAN_StructInit(&CAN_InitStructure);

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
    CAN_FilterInitStructure.CAN_FilterNumber = 0;                    /* 过滤器0 */
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;  /* 屏敝模式 */
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; /* 32位 */
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;               /* 以下四个都为0, 表明不过滤任何id */
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0; /* 能够通过该过滤器的报文存到fifo0中 */
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE); /* 挂号中断, 进入中断后读fifo的报文函数释放报文清中断标志 */
    //CAN_ITConfig(CAN1,CAN_IT_TME, ENABLE);      //发送中断
    CAN_FilterInit(&CAN_FilterInitStructure);
}

//CAN发送函数,采用查询方式，中断方式稳定性差
void CAN1_Send_Str(Send_Rec_Data_Structure *CAN1_Send)
{
    CanTxMsg TxMessage;
    uint16_t i;
    CAN1_Send->finish_flag = 1;
    CAN1_Send->count_use_time_en = 1;

    //TxMessage.StdId = CAN1_Send->mac_id;
    TxMessage.ExtId = CAN1_Send->mac_id;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.IDE = CAN_ID_EXT;

    CAN1_Send->temp_ptr = 0;

    while (CAN1_Send->data_len / 8)
    {
        TxMessage.DLC = 8;

        for (i = 0; i < 8; i++)
        {
            TxMessage.Data[i] = CAN1_Send->data_bufer[CAN1_Send->temp_ptr++];
        }
        i = CAN_NO_MB;
        while ((i == CAN_NO_MB) & (CAN1_Send->finish_flag == 1))
        {
            i = CAN_Transmit(CAN1, &TxMessage);
        }
        CAN1_Send->data_len -= 8;
    }

    if (CAN1_Send->data_len > 0)
    {
        //TxMessage.DLC = 8;
        TxMessage.DLC = CAN1_Send->data_len;

        for (i = 0; i < CAN1_Send->data_len; i++)
        {
            TxMessage.Data[i] = CAN1_Send->data_bufer[CAN1_Send->temp_ptr++];
        }

        i = CAN_NO_MB;
        while ((i == CAN_NO_MB) & (CAN1_Send->finish_flag == 1))
        {
            i = CAN_Transmit(CAN1, &TxMessage);
        }

        if (CAN1_Send->finish_flag == 0)
        {
            CAN_CancelTransmit(CAN1, 0);
            CAN_CancelTransmit(CAN1, 1);
            CAN_CancelTransmit(CAN1, 2);
        }

        CAN1_Send->data_len = 0;
    }

    CAN1_Send->finish_flag = 0;
    CAN1_Send->count_use_time_en = 0;
    CAN1_Send->counter_01ms = 0;
}

//CAN1 Send IRQ
void USB_HP_CAN1_TX_IRQHandler(void) //CAN1发送中断
{
    CanTxMsg TxMessage;
    uint16_t i;

    if (CAN1_Send.data_len == 0)
    {
        CAN1_Send.finish_flag = 0;
        CAN1_Send.count_use_time_en = 0;
        CAN1_Send.counter_01ms = 0;
    }
    else
    {
        if (CAN1_Send.data_len / 8)
        {
            TxMessage.DLC = 8; /* 数据长度, can报文规定最大的数据长度为8字节 */

            for (i = 0; i < 8; i++)
            {
                TxMessage.Data[i] = CAN1_Send.data_bufer[CAN1_Send.temp_ptr++];
            }
            CAN_Transmit(CAN1, &TxMessage); /* 返回这个信息请求发送的邮箱号0,1,2或没有邮箱申请发送no_box */

            CAN1_Send.data_len -= 8;
        }
        else if (CAN1_Send.data_len > 0)
        {
            //TxMessage.DLC = 8;     /* 数据长度, can报文规定最大的数据长度为8字节 */
            TxMessage.DLC = CAN1_Send.data_len; /* 数据长度, can报文规定最大的数据长度为8字节 */
            for (i = 0; i < CAN1_Send.data_len; i++)
            {
                TxMessage.Data[i] = CAN1_Send.data_bufer[CAN1_Send.temp_ptr++];
            }

            CAN_Transmit(CAN1, &TxMessage); /* 返回这个信息请求发送的邮箱号0,1,2或没有邮箱申请发送no_box */
            CAN1_Send.data_len = 0;
        }
    }
    CAN_ClearITPendingBit(CAN1, CAN_IT_TME);

    //CAN_ITConfig(CAN1,CAN_IT_TME, DISABLE);
}

//CAN1 RX0 IRQ
void USB_LP_CAN1_RX0_IRQHandler(void)
{
    CanRxMsg RxMessage;
    uint16_t i, j, target_net_id;
    Config_Data_Str *config_data = Get_Config_data();

    RxMessage.IDE = CAN_ID_EXT;
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
    if (CAN1_Receive.finish_flag == 0)
    {
        //if ((CAN1_Receive.mac_id==RxMessage.StdId)|(CAN1_Receive.mac_id==0))
        if ((CAN1_Receive.mac_id == RxMessage.ExtId) || (CAN1_Receive.mac_id == 0))
        {
            if (CAN1_Receive.temp_ptr == 0)
            {
                CAN1_Receive.count_use_time_en = 1;
                //CAN1_Receive.mac_id=RxMessage.StdId;
                CAN1_Receive.mac_id = RxMessage.ExtId;
                CAN1_Receive.data_bufer[Bus_Source_Net_ID] = CAN1_Receive.mac_id / 256;
                CAN1_Receive.data_bufer[Bus_Source_Net_ID + 1] = CAN1_Receive.mac_id % 256;
                CAN1_Receive.temp_ptr += 2;
                for (i = 0; i < RxMessage.DLC; i++)
                    CAN1_Receive.data_bufer[i + CAN1_Receive.temp_ptr] = RxMessage.Data[i];

                CAN1_Receive.data_len = CAN1_Receive.data_len + CAN1_Receive.data_bufer[Bus_Data_Len];
                if (CAN1_Receive.data_len > Send_Rec_Buffer_Len)
                    CAN1_Receive.data_len = Send_Rec_Buffer_Len;
            }
            else
            {
                for (i = 0; i < RxMessage.DLC; i++)
                    CAN1_Receive.data_bufer[i + CAN1_Receive.temp_ptr] = RxMessage.Data[i];
            }
            CAN1_Receive.temp_ptr += RxMessage.DLC;

            /* 将缓冲区数据传输到对应通道中 */
            if ((CAN1_Receive.temp_ptr >= CAN1_Receive.data_len) || (CAN1_Receive.temp_ptr > (Send_Rec_Buffer_Len - 1)))
            {
                target_net_id = CAN1_Receive.data_bufer[Bus_Target_Net_ID] * 256 + CAN1_Receive.data_bufer[Bus_Target_Net_ID + 1];

                if (target_net_id == 0)
                {
                    if (CAN1_Receive_Buf[0].finish_flag == 0)
                    {
                        for (j = 0; j < CAN1_Receive.data_len; j++)
                            CAN1_Receive_Buf[0].data_bufer[j] = CAN1_Receive.data_bufer[j];

                        CAN1_Receive_Buf[0].data_len = CAN1_Receive.data_len;
                        CAN1_Receive_Buf[0].finish_flag = 1;
                    }
                }
                else
                {
                    for (i = 1; i < (Net_IDs_All + 1); i++)
                    {
                        if ((target_net_id == config_data->Net_IDs[i - 1]) || (target_net_id == config_data->Group_Net_IDs[i - 1]))
                        {

                            if (CAN1_Receive_Buf[i].finish_flag == 0)
                            {
                                for (j = 0; j < CAN1_Receive.data_len; j++)
                                    CAN1_Receive_Buf[i].data_bufer[j] = CAN1_Receive.data_bufer[j];

                                CAN1_Receive_Buf[i].data_len = CAN1_Receive.data_len;
                                CAN1_Receive_Buf[i].finish_flag = 1;
                            }
                            break;
                        }
                    }
                }

                CAN1_Receive.temp_ptr = 0;
                CAN1_Receive.mac_id = 0;
                CAN1_Receive.count_use_time_en = 0;
                CAN1_Receive.counter_01ms = 0;
                CAN1_Receive.finish_flag = 0; // 释放接收缓冲区
                CAN1_Receive.data_len = CAN_Rec_Base_Len;
            }
        }
    }
    CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
}

//初始化支持onecm协议的can接口
void CAN1_ONECM_Bus_Init(void)
{
    //初始化CAN接收中断
    CAN1_NVIC_Init();
    //初始CAN配置
    CAN1_Configuration();
    //初始化CAN缓冲区参数
    // CAN1_Send.mac_id=Net_ID;//初始化发送ID
    CAN1_Receive.data_len = CAN_Rec_Base_Len; //初始化Can_接收长度
}

void CAN1_Err_Timer(void)
{
    if (CAN1_Receive.count_use_time_en == 1) //串口接收超时允许
    {
        CAN1_Receive.counter_01ms++;
        if (CAN1_Receive.counter_01ms > (100 / 1)) //接收时间超过100ms
        {
            CAN1_Receive.counter_01ms = 0;            //复位0.1ms计数器
            CAN1_Receive.temp_ptr = 0;                //复位串口缓冲区指针
            CAN1_Receive.count_use_time_en = 0;       //复位接收超时标志位
            CAN1_Receive.data_len = CAN_Rec_Base_Len; //复位接收长度

            CAN1_Receive.mac_id = 0; //解除CAN1的ID过滤

            CAN1_Receive.finish_flag = 0;
        }
    }

    if (CAN1_Send.count_use_time_en == 1) //串口发送超时允许
    {
        CAN1_Send.counter_01ms++;
        if (CAN1_Send.counter_01ms > (100 / 1)) //接收时间超过100ms
        {
            CAN1_Send.counter_01ms = 0; //复位0.1ms计数器
            //CAN1_Send.temp_ptr=0;          //不需要复位也不能复位
            CAN1_Send.count_use_time_en = 0; //复位接收超时标志位
            //CAN1_Send.data_len=7;          //不需要复位也不能复位
            CAN1_Send.finish_flag = 0;
        }
    }
}
