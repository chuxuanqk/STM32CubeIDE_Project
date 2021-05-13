/*
 * Ocm_OTA.h
 *
 *  Created on: 2021年1月12日
 *      Author: Saber
 */
#ifndef COMMON_OCM_OTA_H_
#define COMMON_OCM_OTA_H_

#include <stdint.h>

#define FLASH_PAGE_SIZE 0x400U
#define PageSize FLASH_PAGE_SIZE

/*=====用户配置(根据自己的分区进行配置)=====*/
/* 大小为0x200的倍数 */
#define BootLoader_Size 0x2800U  ///< BootLoader的大小 10K
#define Application_Size 0x7800U ///< 应用程序的大小 30K
#define Application_Size 0x5000U ///< 应用程序的大小 20K

#define Base_Addr 0x08000000U
#define Application_1_Addr (Base_Addr + BootLoader_Size)           ///< 应用程序1的首地址
#define Application_2_Addr (Application_1_Addr + Application_Size) ///< 应用程序2的首地址
/*==========================================*/

/* 启动的步骤 */
#define Startup_Normol 0xFFFFFFFF ///< 正常启动
#define Startup_Update 0xAAAAAAAA ///< 升级再启动
#define Startup_Reset 0x5555AAAA  ///< ***恢复出厂 暂未使用***

void Start_BootLoader(void);

void Test_Flash(void);

#endif /* COMMON_OCM_OTA_H_ */
