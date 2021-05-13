/*
 * keyboard_func.c
 *
 *  Created on: 2020年11月12日
 *      Author: Saber
 */

#include "config_device/config_par.h"
#include "protocol/modbus_port_adjust.h"
#include "exchange/keyboard_func.h"
#include "virtual_device/bsp_button.h"
#include "virtual_device/bsp_led_7seg.h"
#include "virtual_device/state_led.h"
#include "sys_device/sys_init.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

const char Key7Seg_Show_Band[7][LED_7SEG_DEC_LEN] = {
    {'*', '*', '*', '*'},
    {'*', '*', '*', 'S'},
    {'*', '*', 'C', 'S'},
    {'*', 'G', 'C', 'S'},
    {'o', 'G', 'C', 'S'},
    {'*', 'G', 'C', 'S'},
    {'o', 'G', 'C', 'S'},
};

const char Key7Seg_Show_Reset[6][LED_7SEG_DEC_LEN] = {
    {'L', 'L', 'I', 'K'},
    {'*', '*', '*', '*'},
    {'L', 'L', 'I', 'K'},
    {'*', '*', '*', '*'},
    {'L', 'L', 'I', 'K'},
    {'*', '*', '*', '*'},
};

/* 按键信息 */
struct KeySeg_Structure
{
    KEY_ID key_number;        // 按键ID
    KEY_STATE key_state;      // 按键状态
    uint8_t keyshortup_count; // 按键短按弹起次数
};

/* 按键模式 */
enum key_mode
{
    HOME_PAGE = 0,
    CONFIG_PAGE,
};

/* 按键实例 */
struct KeySeg_Structure KeySeg_Temp = {.key_number = KEY_ZERO, .key_state = KEY_NONE, .keyshortup_count = 0};

#define Set_Home_Page() (LED_7SEG_DEC_Set((char *)Key7Seg_Show_Band, sizeof(Key7Seg_Show_Band), \
                                          250, (char *)Key7Seg_Show_Band, 0, 250))

/**
 * @description: 数码管配置页面
 * @param {*}
 * @return {*}
 */
static void Set_Configure_Page(void)
{
    char tmp_7seg_show_array[4]; //需要显示的临时数据

    tmp_7seg_show_array[0] = '1';
    tmp_7seg_show_array[1] = '0';
    tmp_7seg_show_array[2] = '1';
    tmp_7seg_show_array[3] = 'P';

    //更新7段数码管显示
    LED_7SEG_DEC_Set(tmp_7seg_show_array, sizeof(tmp_7seg_show_array),
                     1000, tmp_7seg_show_array, 0, 1000);
}

/**
 * @description: 
 * @param {*}
 * @return {*}
 */
static void Set_Reset_Page(void)
{
    char tmp_7seg_show_array[4]; //需要显示的临时数据

    tmp_7seg_show_array[0] = '1';
    tmp_7seg_show_array[1] = '0';
    tmp_7seg_show_array[2] = '1';
    tmp_7seg_show_array[3] = 'P';

    LED_7SEG_DEC_Set((char *)Key7Seg_Show_Reset, sizeof(Key7Seg_Show_Reset),
                     500, (char *)tmp_7seg_show_array, sizeof(tmp_7seg_show_array), 500);
}

/**
 * @description: 按键模块 主动发送设备信息
 *               模式1 2 3 4申请分配NET_ID
 * @param {*}channle: 通道号
 *           key7seg_mode：   模式号
 * @return {*}
 */
static void KeySeg_Send_Module_Msg(uint8_t channle)
{
    uint8_t tmp_module_port = channle; // 通道号

    Send_Rec_Data_Structure *can_send = Get_Can1_Send();
    Send_Rec_Data_Structure *can_recv = Get_Can1_Receive();
    Config_Data_Str *cfg_data = Get_Config_data();

    can_send->mac_id = cfg_data->Net_IDs[tmp_module_port];
    can_send->group_net_id = cfg_data->Group_Net_IDs[tmp_module_port];
    can_send->module_port = tmp_module_port;

    CAN1_Send_Meg(CASE_MODULE_MSG, can_send, can_recv);
}

/******************************************************************************************************/

/**
 * @description: 首页
 * @param {*}
 * @return {*}
 */
static void KeySeg_Home_Page(enum key_mode *page, struct KeySeg_Structure *keyseg_tmp)
{
    KEY_ID key_num = keyseg_tmp->key_number;
    KEY_STATE key_state = keyseg_tmp->key_state;
    uint8_t led_num = 0;

    switch (key_num)
    {
    case KEY_SW6:
    {
        switch (key_state)
        {
        case KEY_Short_UP:
            /* 1. 进入配置界面*/
            *page = CONFIG_PAGE;
            Set_Configure_Page();
            break;

        default:
            break;
        }
        break;
    }

    default:
        *page = HOME_PAGE;

        break;
    }

    return;
}

/**
 * @description: 配置页
 * @param {*}
 * @return {*}
 */
static void keySeg_Config_Page(enum key_mode *page, struct KeySeg_Structure *keyseg_tmp)
{
    KEY_ID key_num = keyseg_tmp->key_number;
    KEY_STATE key_state = keyseg_tmp->key_state;

    switch (key_num)
    {

    case KEY_SW1:
    case KEY_SW2:
    case KEY_SW3:
    case KEY_SW4:
    {
        switch (key_state)
        {
        case KEY_Short_UP:
        {
            // SystemReset();
            break;
        }

        case KEY_LONG_DOWN: // 发送配置信息
        {
            net_led_sharp();
            KeySeg_Send_Module_Msg(0);
            break;
        }
        default:
            break;
        }

        break;
    }

    case KEY_SW6:
    {
        switch (key_state)
        {
        case KEY_Short_UP:
        {
            *page = HOME_PAGE;
            Set_Home_Page();
            break;
        }

        case KEY_LONG_8S_UP:
        {
            *page = CONFIG_PAGE;
            Set_Reset_Page();
            //            Config_Data_Clear_Even();
            break;
        }

        default:
            break;
        }

        break;
    }

    default:
        *page = CONFIG_PAGE;
        break;
    }

    return;
}

/**
 * @description: 按键主控制模块
 * @param {*}
 * @return {*}
 */
void KeySeg_Press_Control(void)
{
    uint16_t seg_keyvalue = bsp_KeyPro();
    static enum key_mode page = HOME_PAGE;

    if (seg_keyvalue > 0)
    {
        /* Read_KeyValue_Code = key_number + KEY_STATE * 256 */
        KeySeg_Temp.key_number = seg_keyvalue % 256;
        KeySeg_Temp.key_state = seg_keyvalue / 256;
        KeySeg_Temp.keyshortup_count = 1;

        switch (page)
        {
        case HOME_PAGE:
            KeySeg_Home_Page(&page, &KeySeg_Temp);
            break;

        case CONFIG_PAGE:
            keySeg_Config_Page(&page, &KeySeg_Temp);
            break;

        default:
            break;
        }
    }

    return;
}
