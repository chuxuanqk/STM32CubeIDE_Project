/*
 * config_par.h
 *
 *  Created on: 2020年1月3日
 *      Author: Fox
 */

#ifndef SRC_CONFIG_DEVICE_CONFIG_PAR_H_
#define SRC_CONFIG_DEVICE_CONFIG_PAR_H_

#include "stdbool.h"
#include "stm32f10x.h"

//全部设备通道总数
#define MODULE_ALL_NUM (1)

//模块内存储的网络ID号数量
#define Net_IDs_Single MODULE_ALL_NUM
#define Net_IDs_All (Net_IDs_Single + 1)

/*=====用户配置(根据自己的分区进行配置)=====*/
/* 大小为0x200的倍数 */
#define BootLoader_Size 0x5000U  ///< BootLoader的大小 20K
#define Application_Size 0xD800U ///< 应用程序的大小 54K
#define Application_Page 54

#define Application_1_Addr 0x08005000U ///< 应用程序1的首地址
#define Application_2_Addr 0x08012800U ///< 应用程序2的首地址
/*==========================================*/

//设备信息及保存信息配置
#define Product_Type "123412341234123412341234123412341234" // 36字符长度的产品型号
#define Product_Mac "432143214321432143214321"              // 26字符长度的产品MAC
#define Product_Firmware_Version "1.1.0"                    // 产品固件版本
#define Product_UpdataPakge_Version "1.1.0"                 // 产品更新包版本
#define Product_Protoc0l_Version "1.1.0"                    // 协议版本
#define Product_Hardware_Version "1.1.0"                    // 硬件版本
#define Product_Dev_Version 0x1234                          // 设备类型
#define Product_Subdev_Version 0x4321                       // 设备子类型

#define Mac_Style_Single ((uint16_t)0x0052) //设备类型，通用场景控制器
#define Version 0x0100                      //固件版本,BCD码，v1.0

#define CONFIG_DATA_FLASH_ADDR_SIZE Net_IDs_All * 2 //配置信息大小，总数*2（字节）
#define PageSize 1024                               //STM32芯片FLASH 分页大小，64K产品分页1K
//保存到Flash数据地址分配
// #define CONFIG_DATA_FLASH_ADDR (0x0800F000) // FLASH最后1k起始地址; Falsh End address: 0x08010000
#define CONFIG_DATA_FLASH_ADDR (0x08011000U) // FLASH最后1k起始地址; Falsh End address: 0x08010000

//设备配置信息
typedef struct
{
    // uint32_t baud_rate;      // 波特率
    uint8_t baud_rate_HH;
    uint8_t baud_rate_HL;
    uint8_t baud_rate_LH;
    uint8_t baud_rate_LL;
    uint8_t data_bit;        // 数据位
    uint8_t stop_bit;        // 停止位
    uint8_t parity_bit;      // 校验位
    uint8_t response_Signal; // 应答信号( 0:无接收; 1:有接收；2有接收且主动发送)
    // uint16_t delay_time;     // 指令间隔时间，ms
    uint8_t delay_time_H; // 指令间隔时间，ms
    uint8_t delay_time_L;
} device_config;

//需要保存到Flash的配置信息结构体，存于RAM中
typedef struct
{
    device_config dev_con;
    uint16_t Net_IDs[Net_IDs_All];
    uint16_t Group_Net_IDs[Net_IDs_All];
    uint8_t save_data_flag; //保存标志位，初始值为0xff说明为程序下载后第一次使用，强制复位
} Config_Data_Str;

//STM32芯片物理地址12*8字节共同体
union STM32_ID_12Byte
{
    u32 id_u32[3];
    uint8_t id_u8[12];
};

extern Config_Data_Str Config_Data; // 配置信息
extern union STM32_ID_12Byte STM32_ID;

/* Flash function */
uint32_t Erase_page(uint32_t pageaddr, uint32_t num);
void ReadFlash(uint32_t addr, uint32_t *buff, uint16_t word_size);
void WriteFlash(uint32_t addr, uint32_t *buff, int word_size);
void Write_HalfWord_Flash(uint32_t addr, uint16_t *buff, int halfword_size);

//配置信息载入和写入操作
void Write_Config_To_Flash(Config_Data_Str *config);  //把配置保存到flash
void Read_Config_Form_Flash(Config_Data_Str *config); //从flash读取配置数据到结构体
void Config_Data_ReInit(Config_Data_Str *config);     //重置所有保存数据,需要调用写入flash函数
void Config_Data_Clear_Even(void);

void Cpu_GetId(void);
bool Is_Config_portid(uint8_t temp_port);
Config_Data_Str *Get_Config_data(void);
union STM32_ID_12Byte *Get_CpuId(void);

void Test_flash(void);

#endif /* SRC_CONFIG_DEVICE_CONFIG_PAR_H_ */
