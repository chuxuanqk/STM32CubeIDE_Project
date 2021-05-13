/*
 * config_par.c
 *
 *  Created on: 2020年1月3日
 *      Author: Fox
 */

#include "config_device/config_par.h"

#include "stm32f10x_flash.h"

Config_Data_Str Config_Data;    // 配置信息
union STM32_ID_12Byte STM32_ID; // STM32芯片物理地址12*8字节共同体

/**
 * @description: 擦除页
 * @param {pageaddr} 起始地址
 * @param {num} 擦除的页数
 * @return {1}
 */
uint32_t Erase_page(uint32_t pageaddr, uint32_t num)
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
void ReadFlash(uint32_t addr, uint32_t *buff, uint16_t word_size)
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
void WriteFlash(uint32_t addr, uint32_t *buff, int word_size)
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
 * @description: 写数据，半字
 * @param {uint32_t} addr
 * @param {uint16_t} *buff
 * @param {int} halfword_size
 * @return {*}
 */
void Write_HalfWord_Flash(uint32_t addr, uint16_t *buff, int halfword_size)
{
    FLASH_Unlock();

    for (uint32_t i = 0; i < halfword_size; i++)
    {
        while (FLASH_COMPLETE != FLASH_ProgramHalfWord(addr + 2 * i, buff[i]))
            ;
    }

    FLASH_Lock();
}

/**
 * @description: 写设备数据到flash
 * @param {Config_Data_Str} *config
 * @return {*}
 */
void Write_Config_To_Flash(Config_Data_Str *config)
{
    uint32_t flash_addr = CONFIG_DATA_FLASH_ADDR;
    uint32_t *pdata = (uint32_t *)config;
    uint8_t word_size = (sizeof(Config_Data_Str) / 4);

    Erase_page(flash_addr, 1);
    WriteFlash(flash_addr, pdata, word_size);
}

void Dev_Config_Reinit(device_config *dev_con)
{
    uint8_t i = 0; //设置默认值

    for (i = 0; i < Net_IDs_All; i++)
    {
        // dev_con->baud_rate = 9600;
        dev_con->baud_rate_HH = 0x00;
        dev_con->baud_rate_HL = 0x00;
        dev_con->baud_rate_LH = 0x25;
        dev_con->baud_rate_LL = 0x80;
        dev_con->data_bit = 8;
        dev_con->stop_bit = 1;
        dev_con->parity_bit = 0;
        dev_con->response_Signal = 0;
        dev_con->delay_time_H = 0;
        dev_con->delay_time_L = 0x64;
    }

    return;
}

void Config_Data_ReInit(Config_Data_Str *config)
{
    uint16_t i;
    uint8_t *temp_ptr = (uint8_t *)config;
    //清零所有参数
    for (i = 0; i < sizeof(Config_Data_Str); i++)
    {
        temp_ptr[i] = 0;
    }

    Dev_Config_Reinit(&config->dev_con);

    config->save_data_flag = 1;
};

/**
 * @description: 读取Flash中的设备数据
 * @param {*}
 * @return {*}
 */
void Read_Config_Form_Flash(Config_Data_Str *config)
{
    uint32_t flash_addr = CONFIG_DATA_FLASH_ADDR;
    uint32_t *pdata = (uint32_t *)config;
    uint8_t word_size = (sizeof(Config_Data_Str) / 4);

    ReadFlash(flash_addr, pdata, word_size);

    printf("data: %d, stop: %d, flag: %d\r\n",
           Config_Data.dev_con.data_bit, Config_Data.dev_con.stop_bit,
           Config_Data.save_data_flag);

    if (Config_Data.save_data_flag != 1)
    {
        Config_Data_ReInit(config);
        Write_Config_To_Flash(config);
    }
}

/**
 * @description: 清除配置数据
 * @param {*}
 * @return {*}
 */
void Config_Data_Clear_Even(void)
{
    uint8_t i = 0;

    Config_Data_ReInit(&Config_Data); //清除配置信息
    Write_Config_To_Flash(&Config_Data);
}

#ifdef OLD_CFG_CODE
//配置信息初始化
void Dev_Config_Reinit(device_config *dev_con)
{
    //设置默认值
    uint8_t i = 0;

    for (i = 0; i < Net_IDs_All; i++)
    {
        // dev_con->baud_rate = 9600;
        dev_con->baud_rate_HH = 0x00;
        dev_con->baud_rate_HL = 0x00;
        dev_con->baud_rate_LH = 0x25;
        dev_con->baud_rate_LL = 0x80;
        dev_con->data_bit = 8;
        dev_con->stop_bit = 1;
        dev_con->parity_bit = 0;
        dev_con->response_Signal = 0;
        dev_con->delay_time_H = 0;
        dev_con->delay_time_L = 0x64;
    }

    return;
}

void Write_Config_To_Flash(Config_Data_Str *config)
{
    uint16_t i;
    uint8_t *data_ptr = (uint8_t *)config;
    u32 flash_addr = CONFIG_DATA_FLASH_ADDR;
    FLASH_Unlock();                                                                            /* 每次擦除Flash中数据时得先解锁 */
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); //清除标记
    for (i = 0; i < (sizeof(Config_Data_Str) / FLASH_Page_SIZE + 1); i++)
        while (FLASH_COMPLETE != FLASH_ErasePage(CONFIG_DATA_FLASH_ADDR + i * FLASH_Page_SIZE))
            ;

    for (i = 0; i < sizeof(Config_Data_Str); i++)
    {
        while (FLASH_COMPLETE != FLASH_ProgramHalfWord(flash_addr, (data_ptr[i] + data_ptr[i + 1] * 256)))
            ;
        i++;
        flash_addr += 2;
    }
}

void Read_Config_Form_Flash(Config_Data_Str *config)
{
    uint16_t i;
    uint8_t *data_ptr = (uint8_t *)config;
    uint16_t tmp_read;
    u32 flash_addr = CONFIG_DATA_FLASH_ADDR;
    for (i = 0; i < sizeof(Config_Data_Str); i++)
    {
        tmp_read = *(uint16_t *)(flash_addr);
        data_ptr[i] = tmp_read % 256;
        i++;
        if (i < sizeof(Config_Data_Str))
            data_ptr[i] = tmp_read / 256;

        flash_addr += 2;
    }
    //判断是否为第一次启动
    if (config->save_data_flag != 1)
    {
        Config_Data_ReInit(config);    //重置所有参数
        Write_Config_To_Flash(config); //回写参数到flash
    }
}

void Config_Data_ReInit(Config_Data_Str *config)
{
    uint16_t i;
    uint8_t *temp_ptr = (uint8_t *)config;
    //清零所有参数
    for (i = 0; i < sizeof(Config_Data_Str); i++)
    {
        temp_ptr[i] = 0;
    }

    Dev_Config_Reinit(&config->dev_con);

    config->save_data_flag = 1;
};

/**
 * @description: 清除配置数据
 * @param {*}
 * @return {*}
 */
void Config_Data_Clear_Even(void)
{
    uint8_t i = 0;

    //清除配置信息
    Config_Data_ReInit(&Config_Data);
    Write_Config_To_Flash(&Config_Data);
}

#endif

/**
 * @description: 判断是否配置了通道,未完成
 * @param {*}
 * @return {*}
 */
bool Is_Config_portid(uint8_t temp_port)
{
    bool rflag = false;

    if (temp_port == 0) // 广播，默认开启
        rflag = true;

    // 已配置进config
    // if()
    // rflag = true;

    return rflag;
}

//读取ChipID
void Cpu_GetId(void)
{
    STM32_ID.id_u32[0] = *(__IO u32 *)(0x1FFFF7E8); //产品唯一身份标识寄存器(96位)
    STM32_ID.id_u32[1] = *(__IO u32 *)(0x1FFFF7EC);
    STM32_ID.id_u32[2] = *(__IO u32 *)(0x1FFFF7F0);
}

/**
 * @description: 提供外部模块访问Config_Data
 * @param {*}
 * @return {*}
 */
Config_Data_Str *Get_Config_data(void)
{
    return &Config_Data;
}

union STM32_ID_12Byte *Get_CpuId(void)
{
    return &STM32_ID;
}
