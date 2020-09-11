/*
 * @Author: your name
 * @Date: 2020-09-11 17:08:43
 * @LastEditTime: 2020-09-11 18:18:06
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \Deom_Project\Ocm_demo\Driver\inc\board.h
 */
/*
 * board.h
 *
 *  Created on: Sep 11, 2020
 *      Author: Administrator
 */

#ifndef DRIVER_INC_BOARD_H_
#define DRIVER_INC_BOARD_H_

#include "stm32f10x.h"
#include "config.h"

//STM32芯片物理地址12*8字节共同体
union STM32_ID_12Byte {
    u32 id_u32[3];
    u8 id_u8[12];
};

//需要保存到Flash的配置信息结构体，存于RAM中
typedef struct
{
    // device_config dev_con[MODULE_ALL_NUM]; // 设备保存信息
    uint16_t Net_IDs[MODULE_ALL_NUM];
    uint16_t Group_Net_IDs[MODULE_ALL_NUM];
    uint8_t save_data_flag; //保存标志位，初始值为0xff说明为程序下载后第一次使用，强制复位
} Config_Data_Str;

#define SYSTICK_TICK 9000                   //24位计数器，1ms,计数器=9000
#define CONFIG_DATA_FLASH_ADDR (0x0800FC00) //保存到Flash数据地址分配
#define FLASH_Page_SIZE 1024                //STM32芯片FLASH 分页大小，64K产品分页1K

void hw_board_init(void);

#endif /* DRIVER_INC_BOARD_H_ */
