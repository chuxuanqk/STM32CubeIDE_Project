/*
 * DALImaster.c
 *
 *  Created on: 2020年9月27日
 *      Author: Saber
 */

// #include "gpio.h"
#include <dali_config.h>
#include <DALImaster.h>
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "core_cmInstr.h"

#include "bsp_timer.h"
#include "bsp_exit.h"
#include "bsp_uart.h"

#define DEBUG_ERR uart_send(USART1, (uint8_t *)"ERR", 3)

#define DALI_TIMER TIM3

#define DALI_SetOutputHigh()                             \
  {                                                      \
    GPIO_WriteBit(OUT_DALI_PORT, OUT_DALI_PIN, Bit_SET); \
  }
#define DALI_SetOutputLow()                                \
  {                                                        \
    GPIO_WriteBit(OUT_DALI_PORT, OUT_DALI_PIN, Bit_RESET); \
  }

#define MAX_BF_EDGES 18 // max 18 edges per backward frame

typedef enum daliMsgTypeTag
{
  DALI_MSG_UNDETERMINED = 0,
  DALI_MSG_SHORT_ADDRESS = 1,
  DALI_MSG_GROUP_ADDRESS = 2,
  DALI_MSG_BROADCAST = 4,
  DALI_MSG_SPECIAL_COMMAND = 8
} daliMsgType_t;

typedef enum answerTypeTag
{
  ANSWER_NOT_AVAILABLE = 0,
  ANSWER_NOTHING_RECEIVED,
  ANSWER_GOT_DATA,
  ANSWER_INVALID_DATA,
  ANSWER_TOO_EARLY
} answer_t;

/* state machine related definitions */
typedef enum stateTag
{
  MS_IDLE = 0,                       // bus idle
  MS_TX_SECOND_HALF_START_BIT,       //
  MS_TX_DALI_FORWARD_FRAME,          // sending the dali forward frame
  MS_TX_STOP_BITS,                   //
  MS_SETTLING_BEFORE_BACKWARD,       // settling between forward and backward - stop bits
  MS_SETTLING_BEFORE_IDLE,           // settling before going to idle, after forward frame
  MS_WAITING_FOR_SLAVE_START_WINDOW, // waiting for 7Te, start of slave Tx window
  MS_WAITING_FOR_SLAVE_START,        // start of slave Tx window
  MS_RECEIVING_ANSWER                // receiving slave message
} MASTER_STATE;

/* definition of the captured edge data */
typedef struct capturedDataType_tag
{
  uint16_t capturedTime; // time stamp of signal edge
  uint8_t bitLevel;      // bit level *after* the edge
  uint8_t levelType;     // indication of long or short duration *after* the edge
} capturedDataType;

typedef struct capturedFrameType_tag
{
  capturedDataType capturedData[MAX_BF_EDGES];
  uint8_t capturedItems; // counter of the captured edges
} capturedFrameType;

/***********************************************************/
/* Global variables                                        */
/***********************************************************/

static volatile bool ForwardFrameReceived;
static volatile uint16_t ForwardFrame;
static volatile uint8_t BackwardFrame; // DALI slave answer
static volatile answer_t BackwardFrameAnswer;
static volatile MASTER_STATE masterState;
static volatile bool BackwardFrameFlag;
static volatile bool waitForAnswer;
static volatile bool earlyAnswer;
static volatile uint32_t daliForwardFrame;       // converted DALI master command
static volatile capturedFrameType capturedFrame; // data structure for the capture

__IO static uint8_t bitcount = 0;
__IO static uint8_t waitbitcnt = 0;

// Data variables
uint8_t master_send;              // data to send to slave device， addr + cmd (16 bit)
uint8_t master_recv;              // recv data from slave device, status (8 bit)
static volatile uint8_t dataByte; // data byte from slave device

// Processing variables
static volatile uint8_t flag;        // status flag
static volatile uint8_t bit_count;   // nr of rec/send bits
static volatile uint16_t tick_count; // nr of ticks of the timer

static volatile uint8_t bit_value;  // value of actual bit
static volatile uint8_t actual_val; // bit value in this tick of timer
static volatile uint8_t former_val; // bit value in previous tick of timer

uint8_t uart_buf[10];
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
//callback function
void Data_Master_Received(uint8_t dataByte);
//callback function type
typedef void TMDataReceivedCallback(uint8_t);
TMDataReceivedCallback *DataReceivedCallback = Data_Master_Received;

/*************************************************************************************************/

static void RCC_Configuration(void)
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
}

static void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = OUT_DALI_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(OUT_DALI_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = IN_DALI_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(IN_DALI_PORT, &GPIO_InitStructure);
}

/**
 * @description: receiving flag status
 * @param {type} None
 * @return {type} flag
 */
uint8_t inline get_masterflag(void)
{
  return flag;
}

/**
 * @description: 
 * @param {type} 
 * @return {type} 
 */
static void inline set_masterflag(enum MASTER_FLAG mflag)
{
  flag = mflag;
}

/**
 * @description: sending DALI BUS state
 * @param {type} 
 * @return {type} 
 */
uint8_t get_masterstate(void)
{
  return masterState;
}

/*******************************DALI SEND DATA*********************************************/

static inline uint32_t DALI_ConvertForwardFrame(uint16_t forwardFrame)
{
  uint32_t convertedForwardFrame = 0;
  int8_t i;

  for (i = 15; i >= 0; i--)
  {
    if (forwardFrame & (1 << i))
    { // shift in bits values '0' and '1'
      convertedForwardFrame <<= 1;
      convertedForwardFrame <<= 1;
      convertedForwardFrame |= 1;
    }
    else
    { // shift in bits values '1' and '0'
      convertedForwardFrame <<= 1;
      convertedForwardFrame |= 1;
      convertedForwardFrame <<= 1;
    }
  }
  return convertedForwardFrame;
}

static inline daliMsgType_t DALI_CheckMsgType(uint16_t forwardFrame)
{
  daliMsgType_t type = DALI_MSG_UNDETERMINED;

  if ((forwardFrame & 0x8000) == 0)
  {
    type = DALI_MSG_SHORT_ADDRESS;
  }
  else if ((forwardFrame & 0xE000) == 0x8000)
  {
    type = DALI_MSG_GROUP_ADDRESS;
  }
  else if ((forwardFrame & 0xFE00) == 0xFE00)
  {
    type = DALI_MSG_BROADCAST;
  }
  else if (((forwardFrame & 0xFF00) >= 0xA000) &&
           ((forwardFrame & 0xFF00) <= 0xFD00))
  {
    type = DALI_MSG_SPECIAL_COMMAND;
  }
  return type;
}

/**
 * @description: DALI协议，检测命令是否需要等待从设备返回信息
 * @param {type} 
 * @return {type} 
 */
static inline bool DALI_CheckWaitForAnswer(uint16_t forwardFrame, daliMsgType_t type)
{
  bool waitFlag = false;

  if (type == DALI_MSG_SPECIAL_COMMAND)
  {
    // Special commands
    if ((forwardFrame == COMPARE) ||
        ((forwardFrame & 0xFF81) == VERIFY_SHORT_ADDRESS) ||
        (forwardFrame == QUERY_SHORT_ADDRESS))
    {
      waitFlag = true;
    }
  }
  else
  {
    // Query commands
    if ((((forwardFrame & 0x01FF) >= CMD144) && ((forwardFrame & 0x01FF) <= CMD157)) ||
        (((forwardFrame & 0x01FF) >= CMD160) && ((forwardFrame & 0x01FF) <= CMD165)) ||
        (((forwardFrame & 0x01FF) >= CMD176) && ((forwardFrame & 0x01FF) <= CMD197)) ||
        ((forwardFrame & 0x01FF) == CMD255))
    {
      waitFlag = true;
    }
  }
  return waitFlag;
}

/**
 * @description: 检测命令是否需要重复发送
 * @param {*}
 * @return {*}
 */
static inline bool DALI_CheckRepeatCmd(uint16_t forwardFrame, daliMsgType_t type)
{
  bool repeatCmd = false;

  if (type == DALI_MSG_SPECIAL_COMMAND)
  {
    // Special commands 'initialize' and 'randomize' shall be repeated within 100 ms
    if (((forwardFrame & 0xFF00) == INITIALISE) ||
        (forwardFrame == RANDOMISE))
    {
      repeatCmd = true;
    }
  }
  else
  {
    // Configuration commands (32 - 129) shall all be repeated within 100 ms
    if (((forwardFrame & 0x01FF) >= CMD32) &&
        ((forwardFrame & 0x01FF) <= CMD129))
    {
      repeatCmd = true;
    }
  }
  return repeatCmd;
}

static bool DALI_CheckTimingError()
{
  return false;
}

static bool DALI_CheckLogicalError()
{
  return false;
}

static bool DALI_Decode(void)
{
  if (DALI_CheckTimingError())
    return false;

  if (DALI_CheckLogicalError())
    return false;

  return true;
}

/**
 * @description: DALI Master 数据发送，打开定时器，(ps:要不要采用定时器输入捕获？关闭输入捕获)
 * @param {type} 
 * @return {type} 
 */
static inline void DALI_DoTransmission(uint32_t convertedForwardFrame, bool waitFlag)
{
  masterState = MS_TX_SECOND_HALF_START_BIT;
  waitForAnswer = waitFlag;
  daliForwardFrame = convertedForwardFrame;

  bit_count = 0;
  tick_count = 0x01; //tick_cout 需要从0x01开始计数

  // Activate the timer module to output the forward frame
  // TIM_SetCounter(DALI_TIMER, 0);                   // clear timer counter
  // TIM_SetFrequency(DALI_TIMER, 2400);              // ~ 2400 Hz (half bit time)
  TIM_ITConfig(DALI_TIMER, TIM_IT_Update, ENABLE); // enable TIM_IT_Update ;
  TIM_Cmd(DALI_TIMER, ENABLE);                     // ENABLE TIM
  EXITX_SetStatus(DALI_EXT_LINE, DISABLE);         // 关闭接收触发

  DALI_SetOutputLow(); // init gpio output low
  set_masterflag(MASTER_SENDING_DATA);

  while (masterState != MS_IDLE) // 阻塞等待传输完成？ TODO 修改成非阻塞方式
  {
    // wait till transmission is completed
    // __WFI();
  }

  // if (waitForAnswer)
  // {
  // }
}

/**
 * @description: 
 * @param {*}
 * @return {*}
 */
bool Is_Wait_Answer(void)
{
  return waitForAnswer;
}

/**
 * @description: DALI 控制器数据发送模块
 * @param {type} 
 * @return {type} 
 */
static inline void DALI_Master_Send(uint16_t forwardFrame)
{
  uint8_t i = 0;
  uint8_t n = 1;
  uint32_t convertedForwardFrame = DALI_ConvertForwardFrame(forwardFrame);
  daliMsgType_t daliMsgType = DALI_CheckMsgType(forwardFrame);
  bool waitFlag = DALI_CheckWaitForAnswer(forwardFrame, daliMsgType);

  if (DALI_CheckRepeatCmd(forwardFrame, daliMsgType))
    n = 2;

  while (i < n)
  {
    DALI_DoTransmission(convertedForwardFrame, waitFlag);
    i++;
  }
}

/**
 * @description: Send cmd to the slave device
 * @param {type} 
 * @return {type} 
 */
void inline master_send_data(uint16_t byteToSend)
{
  DALI_Master_Send(byteToSend);
}

/**
 * @description:  DALI protocol physical layer for master device
 * @param {type} 
 * @return {type} 
 */
static inline void send_tick(void)
{
  //access to the routine just every 4 ticks = every half bit 416us
  if ((tick_count & 0x03) == 0)
  {
    switch (masterState)
    {
    case MS_TX_SECOND_HALF_START_BIT:
    {
      DALI_SetOutputHigh();
      bit_count = 0;
      masterState = MS_TX_DALI_FORWARD_FRAME;

      break;
    }

    case MS_TX_DALI_FORWARD_FRAME:
    {
      if (daliForwardFrame & 0x80000000)
      {
        DALI_SetOutputHigh();
      }
      else
      {
        DALI_SetOutputLow();
      }
      daliForwardFrame <<= 1;
      bit_count++;

      if (bit_count == 32)
      {
        bit_count = 0;
        masterState = MS_TX_STOP_BITS;
      }
      break;
    }

    case MS_TX_STOP_BITS: // 发送两个停止位，TE=416us, 4*TE
    {
      DALI_SetOutputHigh();

      // the first half of the first stop bit has just been output.
      // do we have to wait for an answer?

      bit_count++;
      if (bit_count == 4) // 4*416， 停止位发送完成
      {
        if (waitForAnswer) // 需要从设备应答
        {
          bit_count = 0;
          earlyAnswer = false;

          masterState = MS_SETTLING_BEFORE_BACKWARD;
        }
        else // 不需要从设备应答
        {
          bit_count = 0;
          masterState = MS_SETTLING_BEFORE_IDLE;
        }
      }

      break;
    }

    case MS_SETTLING_BEFORE_BACKWARD: // 设置接收帧
    {
      // setup the first window limit for the slave answer
      // slave should not respond before 7TE
      bit_count++;
      if (bit_count >= 8) // 10*416=4160us, 7*416= 2912us, 11*416=4576
      {
        bit_count = 0;
        masterState = MS_WAITING_FOR_SLAVE_START_WINDOW;
      }

      break;
    }

    case MS_WAITING_FOR_SLAVE_START_WINDOW:
    {
      // 采用外部中断起始位触发接收数据过程
      // bit_count = 0;
      bit_count++;
      EXITX_SetStatus(DALI_EXT_LINE, ENABLE); // 使能外部中断，边沿触发
      masterState = MS_WAITING_FOR_SLAVE_START;
      break;
    }

    case MS_WAITING_FOR_SLAVE_START: // 等待接收超时处理
    {
      bit_count++;
      if (bit_count >= 30) //104*4*30/1000= 12.48ms
      {
        // DEBUG_ERR;
        masterState = MS_IDLE; // 释放总线
      }
      break;
    }

    case MS_SETTLING_BEFORE_IDLE:
    {
      // no answer from slave expected, need to wait for the remaining
      // bus idle time before next forward frame
      // add additional 3 TE to minimum specification to be not at the edge of the timing specification

      bit_count++;
      if (bit_count == 29)
        masterState = MS_IDLE; // 释放总线

      break;
    }

    default:
      break;
    }
  }

  tick_count++;

  return;
}

/*******************************DALI RECE DATA*********************************************/
/**
 * @description: 接收从机的数据，数据处理
 * @param {type} 
 * @return {type} 
 */
void Data_Master_Received(uint8_t dataByte)
{
  BackwardFrame = dataByte;
  // Data has been received from master device and dataByte (1st byte)
  // uart_send(USART1, &BackwardFrame, 1);
}

/**
 * @description: 若硬件接收数据电平与DALI协议相反，则将实际电平反向，转化为与DALI协议一致
 * @param {type} 
 * @return {type} 
 */
static inline uint8_t get_DALIIN(void)
{
  if (INVERT_IN_DALI)
  {
    if (GPIO_ReadInputDataBit(IN_DALI_PORT, IN_DALI_PIN))
      return 0;
    else
      return 1;
  }
  else
  {
    if (GPIO_ReadInputDataBit(IN_DALI_PORT, IN_DALI_PIN))
      return 1;
    else
      return 0;
  }
}

/**
 * @description: 未使用该函数
 * @param {type} 
 * @return {type} 
 */
static inline void receive_data(void)
{
  // null variables
  dataByte = 0;
  bit_count = 0;
  tick_count = 0;
  former_val = 1; // 常态电平

  // BackwardFrame = 0;
  // BackwardFrameFlag = true; // 接收到起始位

  set_masterflag(MASTER_RECEIVING_DATA); // 检测到边沿触发
  // disable external interrupt on DALI in port
  // EXITX_SetStatus(DALI_EXT_LINE, DISABLE);
}

/**
 * @description: 处理接收数据帧, need timer 9600Hz
 * @param {type} 
 * @return {type} 
 */
static inline void receive_tick(void)
{
  // static volatile uint8_t cut_point = 0x02;
  // Because of the structure of current amplifier, input has to be negated
  actual_val = get_DALIIN();
  tick_count++;

  // edge detected
  if (actual_val != former_val)
  {
    switch (bit_count)
    {
    case 0: // 1st start bit
      if (tick_count > 2)
      {
        bit_count = 1;
        tick_count = 0;
      }
      break;

    case 9: // 1st stop bit
      if (tick_count > 0x06)
      {
        set_masterflag(MASTER_ERR);
      }
      break;

    default: // other bits ;data bits
      if (tick_count > 0x05)
      {
        dataByte |= (actual_val << (8 - bit_count));

        bit_count++;
        tick_count = 0;
      }
      break;
    }
  }
  else // voltage level stable
  {
    switch (bit_count)
    {
    case 0:                  // start timeout
      if (tick_count > 0x0A) // too long start bit 104*8=832us, 104*16=1664us
      {
        set_masterflag(MASTER_ERR);
      }
      break;

    case 9: // First stop bit
      if (tick_count > 0x08)
      {
        if (actual_val == 0) // wrong level of stop bit
        {
          set_masterflag(MASTER_ERR);
        }
        else
        {
          bit_count++;
          tick_count = 0;
        }
      }
      break;

    case 10:                  // Second stop bit
      if (tick_count >= 0x08) // 832 us, 1200HZ 一个完整周期
      {
        set_masterflag(MASTER_NO_ACTION);
        masterState = MS_IDLE; // 释放总线

        BackwardFrame = dataByte;
        DataReceivedCallback(dataByte); // 数据接收处理回调函数
      }
      break;

    default: // normal bits
      if (tick_count > 0x08)
      {
        set_masterflag(MASTER_ERR); // too long delay before edge
      }
      break;
    }
  }

  former_val = actual_val;

  if (get_masterflag() == MASTER_ERR) // 接收错误处理
  {
    set_masterflag(MASTER_NO_ACTION); // DALI master no action
    masterState = MS_IDLE;            // 释放总线
    // master needn't enable EXTI.
  }
}

/**
 * @description: 应答帧
 * @param {*}
 * @return {*}
 */
uint8_t dali_read_backwardframe(void)
{
  return BackwardFrame;
}

/*******************************HOOK FUNCTION*********************************************/

/**
 * @description:  定时器3，频率:9600HZ , T=104us
 *                用于保证数据接收和发送的时序处理符合DALI协议要求
 * @param {type} 
 * @return {type} 
 */
inline void TIM3_ISR_Hook(void)
{
  if (TIM_GetITStatus(DALI_TIMER, TIM_IT_Update) != RESET)
  {
    TIM_ClearITPendingBit(DALI_TIMER, TIM_IT_Update);

    if (get_masterflag() == MASTER_RECEIVING_DATA)
    {
      receive_tick();
    }
    else if (get_masterflag() == MASTER_SENDING_DATA)
    {
      send_tick();
    }

    if (get_masterflag() == MASTER_NO_ACTION) //check idle voltage on DALI bus
    {
    }
  }
}

/**
 * @description: 外部中断0钩子函数，根据DALI协议接收引脚常态为高，起始位为下降沿触发。
 *              又由于本DALI网关硬件设计将其电平反转，所以接收引脚常态为低，起始位为上升沿触发
 * @param {type} 
 * @return {type} 
 */
inline void EXIT0_Hook(void)
{
  receive_data();
}

static inline void DALI_Master_Init(void)
{
  // First init ALL the global variables
  ForwardFrameReceived = false;
  ForwardFrame = 0;
  BackwardFrame = 0;
  BackwardFrameAnswer = ANSWER_NOT_AVAILABLE;
  masterState = MS_IDLE;
  BackwardFrameFlag = false;
  waitForAnswer = false;
  earlyAnswer = false;
  daliForwardFrame = 0;
  capturedFrame.capturedItems = 0;

  set_masterflag(MASTER_NO_ACTION);
  DALI_SetOutputHigh();
}

void hw_dalimaster_init(void)
{
  RCC_Configuration();
  GPIO_Configuration();

  DALI_Master_Init();
  hw_bsptimer_init();
  hw_exit_init();
}

void dalimaster_test()
{
  static uint16_t num = 0;

  static bool cnt_flag = false;
  static bool flag = false;
  static uint16_t cmd = 0x0000;

  static uint32_t psec_cnt_re = 0;
  static uint16_t pmsec_cnt_re = 0;
  static uint32_t psec_cnt_now = 0;
  static uint16_t pmsec_cnt_now = 0;

  if (cnt_flag == false)
  {

    get_sys_uptime(&psec_cnt_re, &pmsec_cnt_re);
    cnt_flag = true;
  }
  else
  {
    get_sys_uptime(&psec_cnt_now, &pmsec_cnt_now);

    if (psec_cnt_now > psec_cnt_re)
    {
      num = psec_cnt_now - psec_cnt_re;
    }

    if (num >= 1)
    {
      cnt_flag = false;
      num = 0;

      if (flag == false)
      {
        cmd = 0x0000;
        flag = true;
      }
      else
      {
        cmd = 0x00EF;
        flag = false;
      }

      if (masterState == MS_IDLE)
      {
        DALI_Master_Send(cmd);
      }
    }
  }
}

inline void dalimaster1_test(uint16_t cmd)
{
  DALI_Master_Send(cmd);
}
