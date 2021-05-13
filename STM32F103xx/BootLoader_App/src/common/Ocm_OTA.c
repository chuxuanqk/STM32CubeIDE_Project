/*
 * Ocm_OTA.c
 *
 *  Created on: 2021年1月12日
 *      Author: Saber
 */

#include "Ocm_OTA.h"
#include <stdio.h>
#include <stdint.h>

#include "stm32f10x_flash.h"

typedef void (*Jump_Fun)(void);
/**
 * @description: 擦除页
 * @param {pageaddr} 起始地址
 * @param {num} 擦除的页数
 * @return {1}
 */
static uint32_t Erase_page(uint32_t pageaddr, uint32_t num)
{
    FLASH_Unlock();

    /* 擦除FLASH */
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); /* 清空所有标志位 */

    /* 按页擦除 */
    for (uint32_t EraseCounter = 0; EraseCounter < num; EraseCounter++)
    {
        while (FLASH_COMPLETE != FLASH_ErasePage(pageaddr + (PageSize * EraseCounter)))
            ;
    }

    FLASH_Lock();

    return 1;
}

/**
 * @description: 读数据
 * @param {addr}        读取的flash地址
 * @param {*buff}       读出数据的数组指针
 * @param {word_size}   字长长度
 * @return {*}
 */
static void ReadFlash(uint32_t addr, uint32_t *buff, uint16_t word_size)
{
    for (uint32_t i = 0; i < word_size; i++)
        buff[i] = *(__IO uint32_t *)(addr + 4 * i);

    return;
}

/**
 * @description: 写数据
 * @param {addr}    写入的flash地址
 * @param {*buff}   写入数据的数组指针
 * @param {word_size} 长度
 * @return {*}
 */
static void WriteFlash(uint32_t addr, uint32_t *buff, int word_size)
{
    FLASH_Unlock();

    for (uint32_t i = 0; i < word_size; i++)
    {
        while (FLASH_COMPLETE != FLASH_ProgramWord(addr + 4 * i, buff[i]))
            ;
    }

    FLASH_Lock();
}

/**
 * @description: 读取启动模式
 * @param {*}
 * @return {*}
 */
uint32_t Read_Start_Mode(void)
{
    uint32_t mode = 0;
    ReadFlash((Application_2_Addr + Application_Size - 4), &mode, 1);

    return mode;
}

/**
 * @description: 进行程序的覆盖
 *              1.擦除目的地址
 *              2.源地址的代码拷贝到目的地址
 *              3.擦除源地址
 * @param {uint32_t} src_addr  搬运的源地址
 * @param {uint32_t} des_addr  搬运的目的地址
 * @param {uint32_t} byte_size 搬运的程序大小
 * @return {*}
 */
void MoveCode(uint32_t src_addr, uint32_t des_addr, uint32_t byte_size)
{
    /*1.擦除目的地址*/
    printf("> Start erase des flash......\r\n");
    Erase_page(des_addr, (byte_size / PageSize));
    printf("> Erase des flash down......\r\n");

    /*2.开始拷贝, 整页读取拷贝*/
    uint32_t temp[256];

    printf("> Start copy......\r\n");
    for (int i = 0; i < byte_size / 1024; i++)
    {
        ReadFlash((src_addr + i * 1024), temp, 256);
        WriteFlash((des_addr + i * 1024), temp, 256);
    }
    printf("> Copy down......\r\n");

    /*3.擦除源地址*/
    printf("> Start erase src flash......\r\n");
    Erase_page(src_addr, (byte_size / PageSize));
    printf("> Erase src flash down......\r\n");

    return;
}

/* 采用汇编设置栈的值 */
void MSR_MSP(uint32_t ulAddr)
{
    __ASM volatile(
        "MSR MSP, R0\n"
        "BX R14\n");
}

/* 程序跳转函数 */
void IAP_ExecuteApp(uint32_t App_Addr)
{
    Jump_Fun JumpToApp;
    uint_fast32_t stack = (uint_fast32_t)(*(__IO uint32_t *)App_Addr);
    uint_fast32_t Reset = (uint_fast32_t)(*(__IO uint32_t *)(App_Addr + 4));

    if ((stack & 0x2FFE0000U) == 0x20000000U) //检查栈顶地址是否合法.
    {
        __disable_irq();
        JumpToApp = (Jump_Fun)Reset;         //用户代码区第二个字为程序开始地址(复位地址)
        MSR_MSP(*(__IO uint32_t *)App_Addr); //初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
        __enable_irq();
        JumpToApp(); //执行复位函数, 跳转到APP.
    }
    else
    {
        printf("No Application Start.....");
        while (1)
            ;
    }
}

/**
 * @description: 启动BootLoader
 * @param {*}
 * @return {*}
 */
void Start_BootLoader(void)
{
    printf("\r\n");
    printf("***********************************\r\n");
    printf("*                                 *\r\n");
    printf("*         OneCM's BootLoader      *\r\n");
    printf("*                                 *\r\n");
    printf("***********************************\r\n");

    printf("> Choose a startup method......\r\n");

    switch (Read_Start_Mode()) ///< 读取是否启动应用程序 */
    {
    case Startup_Normol: ///< 正常启动 */
    {
        printf("> Normal start......\r\n");
        break;
    }

    case Startup_Update: ///< 升级再启动 */
    {
        printf("> Start update......\r\n");
        MoveCode(Application_2_Addr, Application_1_Addr, Application_Size);
        printf("> Update down......\r\n");
        break;
    }

    case Startup_Reset: ///< 恢复出厂设置 目前没使用 */
    {
        printf("> Restore to factory program......\r\n");
        break;
    }

    default: ///< 启动失败
    {
        printf("> Error:%lx!!!......\r\n", Read_Start_Mode());
        return;
    }
    }

    /* 跳转到应用程序 */
    IAP_ExecuteApp(Application_1_Addr);
}

void Test_Flash(void)
{
    uint32_t data_addr = (uint32_t)0x08004800;
    uint32_t data_buf[4] = {100,
                            200,
                            300,
                            400};
    uint32_t tmp_buf[10] = {0};

    Erase_page(data_addr, 1);

    ReadFlash(data_addr, tmp_buf, 4);
    for (uint8_t i = 0; i < 4; i++)
    {
        printf("data[%d]: %ld\t", i, tmp_buf[i]);
    }

    printf("\r\n");
    WriteFlash(data_addr, data_buf, 4);
    ReadFlash(data_addr, tmp_buf, 4);

    for (uint8_t i = 0; i < 4; i++)
    {
        printf("data[%d]: %ld\t", i, tmp_buf[i]);
    }
    printf("\r\n");

    return;
}
