此核心处理模块，借鉴rt_thread设计风格

# 1.数据流抽象层

数据流抽象层主要负责定义上层application的调用接口，以及底层数据传输驱动的接口定义，主要目的实现数据的流处理。

# 2 程序结构

## 2.1 文件目录树：

```c
/*
/<* core 
ocm_def.h		核心数据结构，以及相关类型定义
ocm_core.h		核心函数声明
ocm_config.h	配置信息

object.c		核心对象模块，对象管理
device.c		设备对象模块，设备管理,继承于object

/<* datastream(or channle)
datastream_core.c	数据流核心模块，抽象数据管理，继承于device
datastream_core.h	

datastream_can.c	can数据流模块，实现抽象的CAN协议通信管理，继承于dataflow_core
datastream_can.h

datastream_usart.c	usart数据流模块，实现抽象的485设备协议通信管理，继承于dataflow_core
datastream_usart.h

/<* app
application_xx.c	用户应用模块，具体实现相关操作
application_xx.h
*/
```

## 2.2主要的数据结构如下：

```c
/*
* Base structure of object，所有数据结构的基类
*/
struct ocm_object		
{
    char name[ONE_NAME_MAX];

    ocm_type_t type;
    ocm_flag_t flag;
};

/*
* device structure，抽象设备类型
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


/*
 * dataflow structure，数据流层抽象为一个设备
 */
struct ocm_dataflow_device
{
    struct ocm_device parent; /**< inherit from ocm_device */

    ocm_flag_t flag;
    ocm_time_t time_s;                 /**< time stamp*/
    enum ocm_dataflow_class_type type; /**< dataflow type.eg:usart  can */

    const struct ocm_dataflow_ops *ops;// 数据流层所需要的操作，由底层驱动提供
};
```



## 2.3 函数定义

| File_Name | Function_Name              | Description |
| --------- | -------------------------- | ----------- |
| object.c  | 供device. 调用             |             |
|           | ocm_object_get_information |             |
|           | ocm_object_init            |             |
|           | ocm_object_detach          |             |
|           | ocm_object_allocate        |             |
|           | ocm_object_delete          |             |
|           | ocm_object_is_systemobject |             |
|           | ocm_object_get_type        |             |
|           | ocm_object_find            |             |
|           |                            |             |
| device.c  | 提供应用层调用             |             |
|           | ocm_device_init            |             |
|           | ocm_device_open            |             |
|           | ocm_device_close           |             |
|           | ocm_device_read            |             |
|           | ocm_device_write           |             |
|           | ocm_device_control         |             |
|           |                            |             |
|           |                            |             |
|           |                            |             |
|           |                            |             |





# 内存管理

数据内存分配采用静态分配。











# 参考文献

[浅谈流处理](https://juejin.im/post/6844903743603867661)