**
/****************************************Copyright (c)****************************************************
**                                 http://www.onecm.com
**
**------------------------------------File Info----------------------------------------------------------*/
/*
 * @Description: 
 * @version: 
 * @Author: Saber
 * @Date: 2020-08-25 11:10:27
 * @FilePath: \OcmDataStream\ocm_core.h
 * @LastEditors: Saber
 * @LastEditTime: 2020-09-01 16:11:56
 * @**
 * **********************************************************************************************************
 */

#ifndef __OCMCORE_H__
#define __OCMCORE_H__

#include "ocm_def.h"
#include "ocm_config.h"
#include "ocm_list.h"

#include "string.h"

 /**
 *  interrupt interface 
 */
 ocm_base_t
 ocm_hw_interrupt_disable(void);
void ocm_hw_interrupt_enable(ocm_base_t level);

/**
 *  object interface 
 */
struct ocm_object_information *ocm_object_get_information(enum ocm_object_class_type type);
void ocm_object_init(struct ocm_object *object,
                     enum ocm_object_class_type type,
                     const char *name);
void ocm_object_detach(ocm_object_t object);
#ifdef ALLOCATE_DELETE
ocm_object_t ocm_object_allocate(enum ocm_object_class_type type,
                                 const char *name);
void ocm_object_delete(ocm_object_t object);
#endif
ocm_bool_t ocm_object_is_systemobject(ocm_object_t object);
ocm_type_t ocm_object_get_type(ocm_object_t object);
ocm_object_t ocm_object_find(const char *name, ocm_uint8_t type);

/**
 *  device interface 
 */
ocm_device_t ocm_device_find(const char *name);

ocm_err_t ocm_device_register(ocm_device_t dev,
                              const char *name,
                              ocm_uint16_t flags);
ocm_err_t ocm_device_unregister(ocm_device_t dev);
ocm_device_t ocm_device_create(int type, int attach_size);
void ocm_device_destroy(ocm_device_t device);

ocm_err_t ocm_device_init(ocm_device_t dev);
ocm_err_t ocm_device_open(ocm_device_t dev, ocm_uint16_t oflag);
ocm_err_t ocm_device_close(ocm_device_t dev);
ocm_size_t ocm_device_read(ocm_device_t dev,
                           ocm_off_t pos,
                           void *buffer,
                           ocm_size_t size);
ocm_size_t ocm_device_write(ocm_device_t dev,
                            ocm_off_t pos,
                            const void *buffer,
                            ocm_size_t size);
ocm_err_t ocm_device_control(ocm_device_t dev, ocm_cmd_t cmd, void *arg);

/* 钩子函数，暂未实现 */
ocm_err_t
ocm_device_set_rx_indicate(ocm_device_t dev,
                           ocm_err_t (*rx_ind)(ocm_device_t dev, ocm_size_t size));
ocm_err_t
ocm_device_set_tx_complete(ocm_device_t dev,
                           ocm_err_t (*tx_done)(ocm_device_t dev, void *buffer));

#endif