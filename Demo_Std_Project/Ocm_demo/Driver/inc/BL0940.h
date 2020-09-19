/*
 * @Author: your name
 * @Date: 2020-09-18 14:03:54
 * @LastEditTime: 2020-09-19 11:05:10
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \Demo_Std_Project\Ocm_demo\Driver\inc\BL0940.h
 */
/*
 * BL0940.h
 *
 *  Created on: 2020年9月18日
 *      Author: Administrator
 */

#ifndef BL0940_H_
#define BL0940_H_

#include "stm32f10x.h"

struct I_FAST_RMS // 电流快速有效值
{
  uint8_t I_FAST_RMS_L;
  uint8_t I_FAST_RMS_M;
  uint8_t I_FAST_RMS_H;
};

struct I_RMS // 电流有效值
{
  uint8_t I_RMS_L;
  uint8_t I_RMS_M;
  uint8_t I_RMS_H;
};

struct V_RMS // 电压有效值
{
  uint8_t V_RMS_L;
  uint8_t V_RMS_M;
  uint8_t V_RMS_H;
};

struct WATT // 有功功率值
{
  uint8_t WATT_L;
  uint8_t WATT_M;
  uint8_t WATT_H;
};

struct CF_CNT // 电能脉冲计数值
{
  uint8_t CFA_CNT_L;
  uint8_t CFA_CNT_M;
  uint8_t CFA_CNT_H;
};

struct TPS1 // 内部温度计量值
{
  uint8_t TPS1_L;
  uint8_t TPS1_M;
  uint8_t TPS1_H;
};

struct TPS2 // 外部温度计量值
{
  uint8_t TPS2_L;
  uint8_t TPS2_M;
  uint8_t TPS2_H;
};

struct CORNER // 电流电压波形相角寄存器
{
  uint8_t CORNER_L;
  uint8_t CORNER_H;
};

struct I_WAVE // 负载电流波形
{
  uint8_t I_WAVE_L;
  uint8_t I_WAVE_M;
  uint8_t I_WAVE_H;
};

struct V_WAVE // 负载电压波形
{
  uint8_t I_WAVE_L;
  uint8_t I_WAVE_M;
  uint8_t I_WAVE_H;
};

struct DEFAULT
{
  uint8_t DEFAULT_L;
  uint8_t DEFAULT_M;
  uint8_t DEFAULT_H;
};

struct BL0940_DataTypeDef // 全电参数数据包
{
  struct I_FAST_RMS i_fast_rms;
  struct I_RMS i_rms;
  struct DEFAULT one_default;
  struct V_RMS v_rms;
  struct DEFAULT two_default;
  struct WATT watt;
  struct DEFAULT three_default;
  struct CF_CNT cfa_cnt;
  struct DEFAULT four_default;
  struct TPS1 tps1;
  struct TPS2 tps2;
  uint8_t checksum;
};

/*< 读取BL0940寄存器值指令 */
#define I_WAVE_ADDR (0x01)
#define V_WAVE_ADDR (0x02)
#define CF_CNT_ADDR (0x0A)
#define ALL_ARG_ADDR (0xAA)

/*< 写BL0940寄存器值指令 */

int8_t Read_BL0940_Reg(void *buf, uint8_t addr);
int8_t Write_BL0940_Reg(void *buf, uint8_t addr);
void hw_BL0940_init(void);

#endif /* BL0940_H_ */
