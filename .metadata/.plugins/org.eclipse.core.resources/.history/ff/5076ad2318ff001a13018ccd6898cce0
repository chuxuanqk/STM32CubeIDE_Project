/*
 * @Author: your name
 * @Date: 2020-09-24 11:59:18
 * @LastEditTime: 2020-09-24 14:26:47
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \Demo_Std_Project\Ocm_demo\common\modbus.h
 */
/*
 * modbus.h
 *
 *  Created on: 2020年9月24日
 *      Author: Administrator
 */

#ifndef MODBUS_H_
#define MODBUS_H_

#define Host_Net_ID 0x0001 //主机网络地址

#define MODULE_ALL_NUM 10 //全部设备通道总数, 用于静态分配通道内存

/*< modbus 功能码*/
enum Modbus_Function
{
  CASE_UPDATE = 0,        // 固件程序更新
  CASE_MODULE_RESPON,     // 总线占用申请
  CASE_MODULE_MSG,        // 设备信息查询
  CASE_MODULE_SET_NET_ID, // 设置设备网络ID,组ID
  CASE_CHANGE_PORT,       // 设备功能，网关主动发送更新数据
  CASE_Read_PORT,         //设备功能，读取输出状态
};

/*< 主机总线串口、CAN总线口、网关协议串口和自由串口统一逻辑总线, 同一串行协议*/
struct Modbus_Protocol
{
  uint16_t src_net_id;
  uint16_t des_net_id;
  uint8_t data_len; // data_len = 2 + 数据域大小
  uint8_t function;
  uint8_t port_id;
  uint8_t *data; // 数据域
  uint8_t crc_L;
  uint8_t crc_H;
};

#endif /* MODBUS_H_ */
