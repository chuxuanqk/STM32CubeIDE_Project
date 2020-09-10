/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.onecm.com
**
**------------------------------------File Info----------------------------------------------------------*/
/*
 * @Description: 相关属性及操作定义
 * @version: 
 * @Author: Saber
 * @Date: 2020-08-24 10:12:17
 * @FilePath: \OcmDataStream\ocm_def.h
 * @LastEditors: Saber
 * @LastEditTime: 2020-08-31 17:37:38
 * @**
 * **********************************************************************************************************
 */

#ifndef __OCMDEF_H__
#define __OCMDEF_H__

#include "stdint.h"
#include "stdbool.h"
#include "ocm_config.h"

/* basic data type definitions */
typedef int32_t ocm_base_t;   /**< base date type */
typedef uint32_t ocm_ubase_t; /**< base data type */
typedef uint8_t ocm_uint8_t;
typedef uint16_t ocm_uint16_t;

typedef bool ocm_bool_t;        /**< Type for bool */
typedef uint8_t ocm_type_t;     /**< Type for type */
typedef uint16_t ocm_flag_t;    /**< Type for flags */
typedef ocm_base_t ocm_err_t;   /**< Type for error number */
typedef ocm_ubase_t ocm_time_t; /**< Type for time stamp */
typedef ocm_ubase_t ocm_tick_t; /**< Type for tick count */
typedef ocm_ubase_t ocm_size_t; /**< Type for size number */
typedef ocm_base_t ocm_off_t;   /**< Type for offset */
typedef ocm_base_t ocm_cmd_t;   /**< Type for cmd */
typedef ocm_base_t ocm_event_t; /**< Type for event */

#define OCM_NULL ((void *)0)
#define ocm_inline static inline

/** 
 * 根据结构体成员计算结构体地址
 * ocm_container_of - return the member address of ptr, if the type of ptr is the
 * struct type.
 */
#define ocm_container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/**
 * Double List structure
 */
struct ocm_list_node
{
    struct ocm_list_node *next; /**< point to next node. */
    struct ocm_list_node *prev; /**< point to prev node. */
};
typedef struct ocm_list_node ocm_list_t; /**< Type for lists. */

enum ocm_object_class_type
{
    OCM_Object_Class_Null = 0,     /**< The object is not used. */
    OCM_Object_Class_Device,       /**< The object is a device */
    OCM_Object_Class_Unknow,       /**< The object is unknown. */
    OCM_Object_Class_Static = 0x80 /**< The object is a static object. */
};

/**
 * The information of the kernel object
 */
struct ocm_object_information
{
    enum ocm_object_class_type type; /**< object class type */
    ocm_list_t object_list;          /**< object list */
    ocm_size_t object_size;          /**< object size */
};

/*
* Base structure of object
*/
struct ocm_object
{
    char name[OCM_NAME_MAX];

    ocm_type_t type;
    ocm_flag_t flag;

    ocm_list_t list;
};
typedef struct ocm_object *ocm_object_t; /**<  */

/**
 * device flags defintions
 */
#define OCM_DEVICE_FLAG_DEACTIVATE 0x000 /**< device is not not initialized */

#define OCM_DEVICE_FLAG_RDONLY 0x001 /**< read only */
#define OCM_DEVICE_FLAG_WRONLY 0x002 /**< write only */
#define OCM_DEVICE_FLAG_RDWR 0x003   /**< read and write */

#define OCM_DEVICE_FLAG_REMOVABLE 0x004  /**< removable device */
#define OCM_DEVICE_FLAG_STANDALONE 0x008 /**< standalone device */
#define OCM_DEVICE_FLAG_ACTIVATED 0x010  /**< device is activated */
#define OCM_DEVICE_FLAG_SUSPENDED 0x020  /**< device is suspended */
#define OCM_DEVICE_FLAG_STREAM 0x040     /**< stream mode */

/* INT or DMA init */
#define OCM_DEVICE_FLAG_INT_RX 0x100 /**< INT mode on Rx */
#define OCM_DEVICE_FLAG_DMA_RX 0x200 /**< DMA mode on Rx */
#define OCM_DEVICE_FLAG_INT_TX 0x400 /**< INT mode on Tx */
#define OCM_DEVICE_FLAG_DMA_TX 0x800 /**< DMA mode on Tx */

#define OCM_DEVICE_OFLAG_CLOSE 0x000  /**< device is closed */
#define OCM_DEVICE_OFLAG_RDONLY 0x001 /**< read only access */
#define OCM_DEVICE_OFLAG_WRONLY 0x002 /**< write only access */
#define OCM_DEVICE_OFLAG_RDWR 0x003   /**< read and write */
#define OCM_DEVICE_OFLAG_OPEN 0x008   /**< device is opened */
#define OCM_DEVICE_OFLAG_MASK 0xf0f   /**< mask of open flag */

/**
 * general device commands
 */
#define OCM_DEVICE_CTRL_RESUME 0x01  /**< resume device */
#define OCM_DEVICE_CTRL_SUSPEND 0x02 /**< suspend device */
#define OCM_DEVICE_CTRL_CONFIG 0x03  /**< configure device */

#define OCM_DEVICE_CTRL_SET_INT 0x10 /**< set interrupt */
#define OCM_DEVICE_CTRL_CLR_INT 0x11 /**< clear interrupt */
#define OCM_DEVICE_CTRL_GET_INT 0x12 /**< get interrupt status */

/**
 * special device commands
 */

/**
 * function calss type
 */
enum ocm_func_calss_type
{
    OCM_FUNC_MODULE_RESPON = 1,         // 应答总线占用命令
    OCM_FUNC_MODULE_MSG,                // 反馈从机信息命令,发送设备信息，包括Net_ID,设备类型，设备固件版本，设备物理地址
    OCM_FUNC_MODULE_SET_ID,             // 设置网络ID,组播ID
    OCM_FUNC_CHANGE_PORT,               // 设备功能，改变输出状
    OCM_FUNC_Read_PORT,                 // 设备功能，读取输出状态
    OCM_FUNC_SET_KEY_LIGHTS_STATE,      // 主机设置面板按键LED及背光状态
    OCM_FUNC_READ_KEY_LIGHTS_STATE,     // 主机读取面板按键LED及背光状态
    OCM_FUNC_SET_CONFIG,                // 设置模块功能,主要是设置是否允许主动发送
    OCM_FUNC_READ_CONFIG,               // 读取模块设置参数
    OCM_FUNC_SET_KEY_COMMEND,           // 主机掉线下面板主动控制设备
    OCM_FUNC_READ_KEY_COMMEND,          // 主机读取面板主动控制设备指令内容
    OCM_FUNC_SEND_KEY_COMMEND,          // 发送保存的按键命令
    OCM_FUNC_SENT_LUX_AND_HUMAN_REMOTE, // 面板发送传感器获取到的信号
    OCM_FUNC_READ_LUX_AND_HUMAN_REMOTE, // 主机读取面板传感器信号
};

/**
 * device (I/O) class type
 */
enum ocm_device_class_type
{
    Ocm_Device_Class_USART = 1, /**< character device */
    Ocm_Device_Class_Block,     /**< block device */
    Ocm_Device_Class_MTD,       /**< memory device */
    Ocm_Device_Class_CAN,       /**< CAN device */
    Ocm_Device_Class_RTC,       /**< RTC device */
    Ocm_Device_Class_Timer,     /**< Timer device */
    Ocm_Device_Class_VDF,       /**< Virtual DataFlow device*/
};

typedef struct ocm_device *ocm_device_t;
/*
* operations set for device ops,This is supply for application layer ops
*/
struct ocm_device_ops
{
    /* common device interface */
    ocm_err_t (*init)(ocm_device_t dev);
    ocm_err_t (*open)(ocm_device_t dev, ocm_flag_t oflag);
    ocm_err_t (*close)(ocm_device_t dev);
    ocm_size_t (*read)(ocm_device_t dev, ocm_off_t pos, void *buffer, ocm_size_t size);
    ocm_size_t (*write)(ocm_device_t dev, ocm_off_t pos, const void *buffer, ocm_size_t size);
    ocm_err_t (*control)(ocm_device_t dev, ocm_cmd_t cmd, void *args);
};

/*
* device structure
*/
struct ocm_device
{
    struct ocm_object parent; /**< inherit from ocm_object */

    enum ocm_device_class_type type; /**< device type */
    ocm_flag_t flag;                 /**< device flag */
    bool open_flag;                  /**< device open flag */

    uint8_t ref_count; /**< reference count */
    uint8_t device_id; /**< 0 - 255 */

    /* device call back */
    ocm_err_t (*rx_indicate)(ocm_device_t dev, ocm_size_t size);
    ocm_err_t (*tx_complete)(ocm_device_t dev, void *buffer);

    const struct ocm_device_ops *ops; /* common device interface */

    void *user_data; /**< device private data */
};

/* error code definitions */
#define OCM_EOK 0
#define OCM_ERROR 1
#define OCM_TIMEOUT 2
#define OCM_EFULL 3
#define OCM_EEMPTY 4
#define OCM_EBUSY 5

#endif