/*
 * dali_cmd.c
 *
 *  Created on: 2020年10月17日
 *      Author: Saber
 */

#include <dali_cmd.h>
#include <DALImaster.h>
#include "bsp_uart.h"
#include "timer_callback.h"

/***********************************************************************************************************************************************
 *                      DALI CMD Type of Addresses
 *                      ADDRESS BYTE |  COMMANDBYTE
 * Short address          0AAAAAAS                      (AAAAAA = 0 to 63, S = 0/1, 0:  CMDBYTE is arc power level, 1: CMDBYTE is command byte)
 * Group address          100AAAAS                      (AAAA = 0 to 15, S = 0/1, 0:  CMDBYTE is arc power level, 1: CMDBYTE is command byte)
 * Broadcast address      1111111S                      (S = 0/1, 0:  CMDBYTE is arc power level, 1: CMDBYTE is command byte)
 * Special command        101CCCC1                      (CCCC = command number, 特殊命令)
 * 
*************************************************************************************************************************************************/

/* CMD : shortaddr + cmdbyte */
enum DALI_CMD
{
    LAMP_OFF = 0x0000,
    LAMP_UP = 0x0001,
    LAMP_DOWN = 0x0002, // up,  down 20ms using the selected FADE RATE
    LAMP_RESET = 0x0020,

    STORE_ACTUAL_LEVEL_IN_THE_DTR = 0x0021,
    STORE_THE_DTR_AS_MAX_LEVEL = 0x002A,
    STORE_TEH_DTR_AS_MIN_LEVEL = 0x002B,
    STORE_THE_DTR_AS_SYSTEM_FAILURE_LEVEL = 0x002C, // 设置系统失效亮度
    STORE_THE_DTR_AS_POWER_ON_LEVEL = 0x002D,       // 设置DTR值为上电默认亮度
    STORE_THE_DTR_AS_FADE_TIME = 0x002E,
    STORE_THE_DTR_AS_FADE_RATE = 0x002F,

    /* System para setting */
    STORE_DTR_AS_SHORT_ADDRESS = 0x0080,

    /* Queries related to status information */
    QUERY_STATUS = 0x0090,       // Status information request
    QUERY_CONTROL_GEAR = 0x0091, // The answer is YES or NO,  YES:0xFF,  NO 无应答
    QUERY_LAMP_FAILURE = 0x0092,
    QUERY_LAMP_POWER_ON = 0x0093,
    QUERY_RESET_STATE = 0x0095,
    QUERY_VERSION_NUMBER = 0x0097,
    QUERY_CONTENT_DTR = 0x0098, // DRT content
    QUERY_DEVICE_TYPE = 0x0099,
    QUERY_CONTENT_DTR1 = 0x009C,
    QUERY_CONTENT_DTR2 = 0x009D,

    /* Queries related to arc power parameter settings */
    QUERY_ACTUAL_LEVEL = 0x00A0,
    QUERY_MAX_LEVEL = 0x00A1,
    QUERY_MIN_LEVEL = 0X00A2,
    QUERY_POWERON_LEVEL = 0x00A3, // 查询起亮值
    QUERY_SYSTEM_FAILURE_LEVEL = 0x00A4,
    QUERY_FADE_TIME_AND_RATE = 0x00A5, // 查询 fade and time rate XY. X: fade time  Y: fade rate

    SET_TEMPORARY_COLOUR_TEMPERATURE = 0x00E7,
};

/* CMD : specialcmd + cmdbyte */
enum DALI_SPECIALCMD
{
    TERLMINATE = 0x00A1,      // 终止特殊命令处理流程
    DATA_TRANSFER_REG = 0xA3, // A3 XX : 将A3存储到寄存器DTR中
    INITIALISE_ADDR = 0x00A5, // A5 XX 选中镇流器，该命令将启动一个15分钟的定时，命令259~270只能在这时间内进行处理
    RANDOMISE_ADDR = 0x00A7,
    COMPARE_ADDR = 0x00A9,
    WITHDRAW = 0x00AB,
    SEARCHADDRH = 0x00B1,
    SEARCHADDRM = 0x00B3,
    SEARCHADDRL = 0x00B5,
    PROGRAM_SHORT_ADDR = 0x00B7,
    VERIFY_SHORT_ADDR = 0x00B9,
    QUERY_SHORT_ADDR = 0x00BB,
    PHYSICAL_SELECTION = 0x00BD,
    DALIC_SETDTR1 = 0x00C3,
    DALIC_SETDTR2 = 0x00C5,

    ENABLE_DEVICE_TYPE8 = 0xC108,
};

/************************************内部函数*********************************************************/
static inline void Cmd_DALITransmit(uint16_t addr, uint16_t cmd)
{
    uint16_t addr_byte = addr;
    uint16_t cmd_byte = cmd;
    uint16_t dali_cmd = (addr_byte << 8) | cmd_byte;

    master_send_data(dali_cmd);
}

/**
 * @description: 将需要设定的值放入DRT中
 *              Special cmd: DATA_TRANSFER_REG = 0xA3
 * @param {*}
 * @return {*}
 */
static void Cmd_StoreDataToDTR(uint8_t data)
{
    uint16_t addr_byte = (uint16_t)DATA_TRANSFER_REG;
    uint16_t cmd_byte = (uint16_t)data;
    uint16_t dali_cmd = (addr_byte << 8) | cmd_byte;

    master_send_data(dali_cmd);
}

/**
 * @description: 将需要设定的值放入DRT1中
 * @param {*}
 * @return {*}
 */
static void Cmd_StoreDataToDTR1(uint8_t data)
{
    uint16_t addr_byte = (uint16_t)DALIC_SETDTR1;
    uint16_t cmd_byte = (uint16_t)data;
    uint16_t dali_cmd = (addr_byte << 8) | cmd_byte;

    master_send_data(dali_cmd);
}

/**
 * @description: 
 * @param {*}
 * @return {*}
 */
void Cmd_EnableDevType8(void)
{
    uint16_t dali_cmd = ENABLE_DEVICE_TYPE8;

    master_send_data(dali_cmd);
}

/*****************************DALI COMMEND CMD FUNCTION**************************************/
/**
 * @description: 发送直接设置亮度等级命令
 * @param {type} short_addr: 0-63
 *               dim_level: 0-255(0x00-0xFE)
 * @return {type} 
 */
void Cmd_SetDimLevel(uint8_t short_addr, uint8_t dim_level)
{
    uint16_t addr_byte = (((uint16_t)short_addr) << 1) | 0x0000; // S = 0x0000 ,CMDBYTE is arc power level

    Cmd_DALITransmit(addr_byte, (uint16_t)dim_level);
}

/**
 * @description: 发送设置起亮等级命令
 *              1. 先将值存储到 DTR(Data Transfer Register)中
 *              2. 将DTR中的值设为起亮等级
 * @param {type} short_addr: 0-63
 *               power_on_level: 0-255(0x00-0xFE)
 * @return {type} 
 */
void Cmd_SetPowerOnLevel(uint8_t short_addr, uint8_t power_on_level)
{
    uint8_t i = 0;
    uint16_t addr_byte = (((uint16_t)short_addr) << 1) | 0x0001; // S = 0x0001 ,CMDBYTE is arc command byte

    /* 为确保数值存入DRT，采用重复发送多次指令方式保证数据传输成功 */
    for (i = 0; i < 2; i++)
        Cmd_StoreDataToDTR(power_on_level);

    Cmd_DALITransmit(addr_byte, STORE_THE_DTR_AS_POWER_ON_LEVEL);
}

/**
 * @description: 设置系统失效亮度值
 * @param {*} dim_level: 0-255(0x00-0xFE)
 * @return {*}
 */
void Cmd_SetSystemFailureLevel(uint8_t short_addr, uint8_t dim_level)
{
    uint8_t i = 0;
    uint16_t addr_byte = (((uint16_t)short_addr) << 1) | 0x0001; // S = 0x0001, CMDBYTE is command byte

    /* 为确保数值存入DRT，采用重复发送多次指令方式保证数据传输成功 */
    for (i = 0; i < 2; i++)
        Cmd_StoreDataToDTR(dim_level);

    Cmd_DALITransmit(addr_byte, STORE_THE_DTR_AS_POWER_ON_LEVEL);
}

/**
 * @description: 设置渐变时间，在两个亮度之间变化时，时间间隔。
 * @param {*}   time_rate: 0~15
 * @return {*}
 */
void Cmd_SetTimeRate(uint8_t short_addr, uint8_t time_rate)
{
    uint8_t i = 0;
    uint16_t addr_byte = (((uint16_t)short_addr) << 1) | 0x0001; // S = 0x0001, CMDBYTE is command byte

    /* 为确保数值存入DRT，采用重复发送多次指令方式保证数据传输成功 */
    for (i = 0; i < 2; i++)
        Cmd_StoreDataToDTR(time_rate);

    Cmd_DALITransmit(addr_byte, STORE_THE_DTR_AS_FADE_TIME);
}

/**
 * @description: 设置渐变率, 一定时间内改变亮度的次数
 * @param {*} fate_rate: 1~15
 * @return {*}
 */
void Cmd_SetFateRate(uint8_t short_addr, uint8_t fate_rate)
{
    uint8_t i = 0;
    uint16_t addr_byte = (((uint16_t)short_addr) << 1) | 0x0001; // S = 0x0001, CMDBYTE is command byte

    /* 为确保数值存入DRT，采用重复发送多次指令方式保证数据传输成功 */
    for (i = 0; i < 2; i++)
        Cmd_StoreDataToDTR(fate_rate);

    Cmd_DALITransmit(addr_byte, STORE_THE_DTR_AS_FADE_RATE);
}

/**
 * @description: 
 * @param {*}
 * @return {*}
 */
void Cmd_SetTemperatureColour(uint8_t short_addr)
{
    uint16_t addr_byter = (((uint16_t)short_addr) << 1) | 0x0001;

    Cmd_DALITransmit(addr_byter, SET_TEMPORARY_COLOUR_TEMPERATURE);
}

/**
 * @description: 
 * @param {*}
 * @return {*}
 */
void Cmd_QueryDimLevel(uint8_t short_addr)
{
    uint16_t addr_byte = (((uint16_t)short_addr) << 1) | 0x0001;

    Cmd_DALITransmit(addr_byte, QUERY_ACTUAL_LEVEL);
}

/**
 * @description: // 查询 fade and time rate XY. X: fade time  Y: fade rate
 * @param {*}
 * @return {*} 
 */
void Cmd_QueryFateAndTimeRate(uint8_t short_addr)
{
    uint16_t addr_byte = (((uint16_t)short_addr) << 1) | 0x0001;

    Cmd_DALITransmit(addr_byte, QUERY_FADE_TIME_AND_RATE);
}

/**
 * @description: 发送获取起亮等级命令
 *               Return the power up level
 * @param {type} 
 * @return {type} 
 */
void Cmd_QueryPowerOnLevel(uint8_t short_addr)
{
    uint16_t addr_byte = (((uint16_t)short_addr) << 1) | 0x0001; // S = 0x0001, CMDBYTE is command byte

    Cmd_DALITransmit(addr_byte, QUERY_POWERON_LEVEL);
}

/**
 * @description: 
 * @param {*}
 * @return {*}
 */
void Cmd_QueryDeviceType(uint8_t short_addr)
{
    uint16_t addr_byte = (((uint16_t)short_addr) << 1) | 0x0001;

    Cmd_DALITransmit(addr_byte, QUERY_DEVICE_TYPE);
}

/**
 * @description: 
 * @param {*}
 * @return {*}
 */
void Cmd_QueryContentDTR1(uint8_t short_addr)
{
    uint16_t addr_byter = (((uint16_t)short_addr) << 1) | 0x0001;

    Cmd_DALITransmit(addr_byter, QUERY_CONTENT_DTR1);
}

/**
 * @description: 
 * @param {*}
 * @return {*}
 */
void Cmd_QueryContentDTR2(uint8_t short_addr)
{
    uint16_t addr_byter = (((uint16_t)short_addr) << 1) | 0x0001;

    Cmd_DALITransmit(addr_byter, QUERY_CONTENT_DTR2);
}

/*************************************************************************
 *      Addressing commands
 *      1 STEP : INITIALISE 0xA5 XX     该命令将启动一个15分钟的定时，命令259~270只能在这个时间内进行处理
 *              XX: 00 所有镇流器均响应 
 *              XX: 0AAA AAA1 只有地址为AAAAAA的镇流器才会响应
 *              XX: 1111 1111 无地址的镇流器才会响应 
 * 
 * 
 * ************************************************************************/
void Cmd_InitialiseAddr(uint8_t short_addr)
{
    uint16_t addr_byte = (((uint16_t)short_addr) << 1) | 0x0001;

    Cmd_DALITransmit(INITIALISE_ADDR, addr_byte);
}

/**
 * @description: 发送镇流器物理选择模式
 * @param {*}
 * @return {*}
 */
void Cmd_Physical_Selection(void)
{
    uint16_t special_byte = 0x00;

    Cmd_DALITransmit(PHYSICAL_SELECTION, special_byte);
}

/**
 * @description: 删除被选中的镇流器短地址  B7 FF 
 *              被选中的含义： Cmd_InitialiseAddr可选中对应镇流器
 *                  * RANDOM_ADDRESS[H: M: L] == SEARCHADDR[H: M: L]
 *                  * 通过物理寻址来选择一个指定地址的镇流器
 * 删除短地址流程：1. 0xA5 0XXXXXX1  --> 2. 0xBD 0x00 --> 3. 0xB7 0xFF
 * @param {type} 
 * @return {type} 
 */
void Cmd_DelShortAddr(uint8_t short_addr)
{
    uint16_t special_byte = (uint16_t)PROGRAM_SHORT_ADDR;
    uint16_t cmd_byte = 0x00FF; // 0XFF 为删除镇流器地址; 0AAAAAA1 为将被选中的镇流器地址设置为 0AAAAAA1
    uint16_t dali_cmd = (special_byte << 8) | cmd_byte;

    Cmd_InitialiseAddr(short_addr); /*1. 选中指定short_addr 镇流器 */
    Cmd_Physical_Selection();       /*2. 命令镇流器进入物理选择模式 */
    master_send_data(dali_cmd);     /*3. 命令镇流器删除短地址 */
}

/**
 * @description: 已知短地址，修改短地址
 *              1. 将new_addr值存入 DTR中: A3 new_addr
 *              (1.1 读取DTR的值，与设置比较是否相等，若不相等错误处理)
 *              2. 将DRT的值作为该灯具的新地址
 * @param {*}   old_addr AAAAAA   0~63
 *              new_addr AAAAAA
 * @return {*}
 */
void Update_ShortAddr(uint8_t old_addr, uint8_t new_addr)
{
    uint8_t DTR_data = (((uint16_t)new_addr) << 1) | 0x0001;
    uint16_t addr_byte = (((uint16_t)old_addr) << 1) | 0x0001;
    uint16_t cmd_byte = STORE_DTR_AS_SHORT_ADDRESS;
    uint16_t dali_cmd = (addr_byte << 8) | cmd_byte; // 将DRT的值作为该灯具的新地址

    Cmd_StoreDataToDTR(DTR_data);
    master_send_data(dali_cmd);

    return;
}