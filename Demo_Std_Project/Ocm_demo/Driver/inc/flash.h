/*
 * @Author: your name
 * @Date: 2020-09-12 10:13:40
 * @LastEditTime: 2020-09-12 15:06:59
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \Demo_Std_Project\Ocm_demo\Driver\inc\flash.h
 */
#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32f10x.h"
#include "core_cm3.h"
#include "stm32f10x_flash.h"

#define FLASH_SIZE 64
#define FLASH_SECTOR_SIZE 1024
#define CONFIG_FLASH_ADDR (0x0800FC00) // Flash数据地址分配

uint16_t FLASH_BUF[FLASH_SECTOR_SIZE / 2]; // 1024个字节(半字16位:2个字节)

void FLASH_Read(uint32_t ReadAddr, uint16_t *pBuffer, uint16_t buf_size);
void Flash_Write(uint32_t WriteAddr, uint16_t *pBuffer, uint16_t buf_size);

#endif
