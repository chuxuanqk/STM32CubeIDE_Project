/*
 * @Author: your name
 * @Date: 2020-09-12 10:12:21
 * @LastEditTime: 2020-10-28 17:19:32
 * @LastEditors: Saber
 * @Description: In User Settings Edit
 * @FilePath: \code\src\virtual_device\driver\bsp_flash.c
 */
#include "bsp_flash.h"

#define FLASH_READHALFWORD(faddr) (*(__IO uint16_t *)faddr) // 读取flash半字数据

/**
 * @description: 不检查写入
 * @param {type} 
 * @return {type} 
 */
void __Flash_Write_NoCheck(uint32_t WriteAddr, uint16_t *pBuffer, uint16_t buf_size)
{
    uint16_t i;

    for (i = 0; i < buf_size; i++)
    {
        FLASH_ProgramHalfWord(WriteAddr, pBuffer[i]);
        WriteAddr += 2; // 地址增加2, 一个32位地址指向1个字节
    }
}

/**
 * @description: 
 * @param {type} 
 * @return {type} 
 */
void FLASH_Read(uint32_t ReadAddr, uint16_t *pBuffer, uint16_t buf_size)
{
    uint16_t i;
    for (i = 0; i < buf_size; i++)
    {
        pBuffer[i] = FLASH_READHALFWORD(ReadAddr); // 读取2个字节
        ReadAddr += 2;                             // 偏移2个字节
    }
}

/**
 * @description: 从指定地址开始写入指定长度的数据
 * @param   WriteAddr: 1.地址必须是用户代码区以外的地址  2.地址必须是2的倍数
 *          pBuffer:   数据指针     
 *          buf_size ：半字(16位)数 (需要写入的16位数据的个数)
 * @return {type} 
 */
void Flash_Write(uint32_t WriteAddr, uint16_t *pBuffer, uint16_t buf_size)
{
    uint32_t secpos;    // 扇区地址(第几扇区)
    uint16_t secoff;    // 扇区内偏移地址
    uint16_t secremain; // 扇区内剩余地址
    uint16_t i;
    uint32_t offaddr; // 去掉CONFIG_FLASH_ADDR后的地址

    if (WriteAddr < CONFIG_FLASH_ADDR || (WriteAddr >= (FLASH_BASE + 1024 * FLASH_SIZE))) // 地址非法
        return;

    FLASH_Unlock();                   // 解锁写保护
    offaddr = WriteAddr - FLASH_BASE; // 实际偏移地址
    secpos = offaddr / FLASH_SECTOR_SIZE;
    secoff = (offaddr % FLASH_SECTOR_SIZE) / 2; // 在扇区内的偏移(2个字节为基本单位)
    secremain = FLASH_SECTOR_SIZE / 2 - secoff; // 扇区剩余空间大小,可存最大空间

    if (buf_size <= secremain)
        secremain = buf_size; // 不大于该扇区范围，修改为需要存储的空间大小

    while (1)
    {
        FLASH_Read(secpos * FLASH_SECTOR_SIZE + FLASH_BASE, FLASH_BUF, FLASH_SECTOR_SIZE / 2); // 读出整个扇区的内容

        for (i = 0; i < secremain; i++) // 校验数据
        {
            if (FLASH_BUF[secoff + i] != 0xFFFF)
                break; // 需要擦除
        }

        if (i < secremain) // 需要擦除
        {
            FLASH_ErasePage(secpos * FLASH_SECTOR_SIZE + FLASH_BASE);

            for (i = 0; i < secremain; i++) // 复制需要保存的数据
            {
                FLASH_BUF[i + secoff] = pBuffer[i];
            }

            __Flash_Write_NoCheck(secpos * FLASH_SECTOR_SIZE + FLASH_BASE, FLASH_BUF, FLASH_SECTOR_SIZE / 2); // 写入整个扇区
        }
        else
        {
            __Flash_Write_NoCheck(WriteAddr, pBuffer, secremain); // 写已经擦除了的，直接写入扇区剩余区间
        }

        if (buf_size == secremain)
            break; // 写入结束
        else       // 写入未结束
        {
            secpos++;
            secoff = 0;
            pBuffer += secremain;
            WriteAddr += secremain;
            buf_size -= secremain;
            if (buf_size > (FLASH_SECTOR_SIZE / 2)) // 下一扇区还是写不完
                secremain = FLASH_SECTOR_SIZE / 2;
            else // 下一扇区可以写完
                secremain = buf_size;
        }
    }

    FLASH_Lock(); //上锁
}
