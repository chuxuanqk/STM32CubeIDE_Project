/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.onecm.com
**
**------------------------------------File Info----------------------------------------------------------*/
/*
 * @Description: 
 * @version: 
 * @Author: Saber
 * @Date: 2020-08-25 11:16:29
 * @FilePath: \STM32F103C8_Demo\Core\OcmDataFlow\device.c
 * @LastEditors: Saber
 * @LastEditTime: 2020-08-26 16:51:55
 * @**
 * **********************************************************************************************************
 */
#include "ocm_core.h"
#include "ocm_device.h"

#include "stdlib.h"

#define device_init (dev->init)
#define device_open (dev->open)
#define device_close (dev->close)
#define device_read (dev->read)
#define device_write (dev->write)
#define device_control (dev->control)

/**
 * @brief: 
 * @param {type} 
 * @return {type} 
 */
ocm_err_t ocm_device_register(ocm_device_t dev,
                              const char *name,
                              ocm_flag_t flags)
{
    if (dev == OCM_NULL)
        return -OCM_ERROR;

    if (ocm_device_find(name) != OCM_NULL)
        return -OCM_ERROR;

    ocm_object_init(&(dev->parent), OCM_Object_Class_Device, name);

    dev->flag = flags;
    dev->open_flag = 0;

    return OCM_EOK;
}

/**
 * @brief: 
 * @param {type} 
 * @return {type} 
 */
ocm_err_t ocm_device_unregister(ocm_device_t dev)
{
    ocm_object_detach(&*(dev->parent));

    return OCM_EOK;
}

/**
 * @brief: 
 * @param {type} 
 * @return {type} 
 */
ocm_device_t ocm_device_find(const char *name)
{
    struct ocm_object *object;
    struct ocm_list_node *node;
    struct ocm_object_infomation *information;

    information = ocm_object_get_information(OCM_Object_Class_Device);

    for (node = information->object_list.next; // 遍历链表，查找设备
         node != &(information->object_list);
         node = node->next)
    {
        object = ocm_list_entry(node, struct ocm_object, list);

        if (strncmp(object->name, name, OCM_NAME_MAX) == 0)
        {
            return (ocm_device_t)object;
        }
    }

    return OCM_NULL;
}

/**
 * @brief: 
 * @param {type} 
 * @return {type} 
 */
ocm_device_t ocm_device_create(int type, int attach_size)
{
    int size;
    ocm_device_t device;

    size = sizeof(struct ocm_device);

    /*use the totoal size*/
    size += attach_size;

    device = (ocm_device_t)malloc(size);
    if (device)
    {
        memset(device, 0x0, sizeof(struct ocm_device));
        device->type = (enum ocm_device_class_type)type;
    }

    return device;
}

/**
 * @brief: 
 * @param {type} 
 * @return {type} 
 */
void ocm_device_destroy(ocm_device_t dev)
{
    ocm_object_detach(&(dev->parent));

    free(dev);
}

/**
 * @brief: 
 * @param {type} 
 * @return {type} 
 */
ocm_err_t ocm_device_init(ocm_device_t dev)
{
    ocm_err_t result = OCM_EOK;

    /* get device init handler */
    if (device_init != OCM_NULL)
    {
        if (!(dev->flag & OCM_DEVICE_FLAG_ACTIVATED))
        {
            result = device_init(dev);
            if (result != OCM_EOK)
            {
                /* error reduce code */
            }
            else
            {
                dev->flag |= OCM_DEVICE_FLAG_ACTIVATED;
            }
        }
    }

    return result;
}

/**
 * @brief: 
 * @param {type} 
 * @return {type} 
 */
ocm_device_t ocm_device_open(ocm_device_t dev, ocm_flag_t oflag)
{
    ocm_err_t result = OCM_EOK;

    /* if device is not initialized, initialize it. */
    if (!(dev->flag & OCM_DEVICE_FLAG_ACTIVATED))
    {
        if (device_init != OCM_NULL)
        {
            result = device_init(dev);
            if (result != OCM_EOK)
            {
                /* error reduce code */
                return result;
            }
            else
            {
                dev->flag |= OCM_DEVICE_FLAG_ACTIVATED;
            }
        }
    }

    /*  device is a opened */
    if ((dev->open_flag & OCM_DEVICE_OFLAG_OPEN))
        return -OCM_EBUSY;

    /* call device open interface */
    if (device_open != OCM_NULL)
    {
        result = device_open(dev, oflag);
    }
    else
    {
        dev->open_flag = (oflag & OCM_DEVICE_OFLAG_MASK);
    }

    if (result == OCM_EOK)
    {
        dev->open_flag |= OCM_DEVICE_OFLAG_OPEN;

        dev->ref_count++;
    }

    return result;
}

ocm_err_t ocm_device_close(ocm_device_t dev)
{
    ocm_err_t result = OCM_EOK;

    if (dev->ref_count == 0)
        return -OCM_ERROR;

    dev->ref_count--;

    if (dev->ref_count != 0)
        return OCM_EOK;

    /* call deivce close interface */
    if (device_close != OCM_NULL)
    {
        result = device_close(dev);
    }

    /* set open flag */
    if (result == OCM_EOK)
        dev->open_flag = OCM_DEVICE_OFLAG_CLOSE;

    return result;
}

/**
 * @brief: 
 * @param {type} 
 * @return {type} 
 */
ocm_size_t ocm_device_read(ocm_device_t dev,
                           ocm_off_t pos,
                           void *buffer,
                           ocm_size_t size)
{
    if (dev->ref_count == 0)
    {
        return OCM_ERROR;
    }

    /* call device read interface */
    if (device_read != OCM_NULL)
    {
        retrun device_read(dev, pos, buffer, size);
    }

    return OCM_ERROR;
}

/**
 * @brief: 
 * @param {type} 
 * @return {type} 
 */
ocm_size_t ocm_device_write(ocm_device_t dev,
                            ocm_off_t pos,
                            const void *buffer
                                ocm_size_t size)
{
    if (dev->ref_count == 0)
    {
        return OCM_ERROR;
    }

    if (device_write != OCM_NULL)
    {
        return device_write(dev, pos, buffer, size);
    }

    return OCM_ERROR;
}

/**
 * @brief: 
 * @param {type} 
 * @return {type} 
 */
ocm_err_t ocm_device_control(ocm_device_t dev, ocm_cmd_t cmd, void *arg)
{
    if (dev->ref_count == 0)
    {
        return OCM_ERROR;
    }

    if (device_control != OCM_NULL)
        return device_control(dev, cmd, arg);

    return OCM_ERROR;
}
