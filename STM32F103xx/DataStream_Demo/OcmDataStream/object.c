/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.onecm.com
**
**------------------------------------File Info----------------------------------------------------------*/
/*
 * @Description: 
 * @version: 
 * @Author: Saber
 * @Date: 2020-08-25 11:03:39
 * @FilePath: \STM32F103C8_Demo\Core\OcmDataFlow\object.c
 * @LastEditors: Saber
 * @LastEditTime: 2020-08-27 16:21:50
 * @**
 * **********************************************************************************************************
 */
#include <string.h>

/*
 * define object_info for the number of ocm_object_container items.
 */
enum ocm_object_info_type
{
    OCM_Object_Info_Device = 0, /**< The object is a device */
    OCM_Object_Info_Unknown,    /**< The object is unknown. */
};

#define _OBJ_CONTAINER_LIST_INIT(c)                                                  \
    {                                                                                \
        &(rt_object_container[c].object_list), &(rt_object_container[c].object_list) \
    }

static struct ocm_object_information ocm_object_container[OCM_Object_Info_Unknown] = {
    /* initialize object container - device */
    {OCM_Object_Class_Device, _OBJ_CONTAINER_LIST_INIT(OCM_Object_Info_Device), sizeof(struct ocm_device)},
};

/**
 * @brief: This function will return the specified type of object information
 * @param type the type of object
 * @return the object type information or OCM_NULL
 */
struct ocm_object_information *
ocm_object_get_information(enum ocm_object_class_type type)
{
    ocm_size_t index;

    for (index = 0; index < OCM_Object_Info_Unknown; index++)
        if (ocm_object_container[index].type == type)
            return &ocm_object_container[index];

    return OCM_NULL;
}

/**
 * @brief: This function will initialize an object and add it to object 
 * @param {type} 
 * @return {type} 
 */
void ocm_object_init(struct ocm_object *object,
                     enum ocm_object_class_type type,
                     const char *name)
{
    register ocm_base_t temp;
    struct ocm_object_information *information;

    /* get object information */
    information = ocm_object_get_information(type);

    /* set objcet type to static */
    object->type = type | OCM_Object_Class_Static;

    /* copy name */
    strncpy(object->name, name, OCM_NAME_MAX);

    ocm_list_insert_after(&(information->object_list), &(object->list));
    /* lock interrup */
    temp = ocm_hw_interrupt_disable();

    /* unlock interrup */
    ocm_hw_interrupt_enable(temp);
}

/**
 * @brief: 
 * @param {type} 
 * @return {type} 
 */
void ocm_object_detach(ocm_object_t object)
{
    register ocm_base_t temp;

    /* object check */
    /* pass */

    object->type = 0;

    temp = ocm_hw_interrupt_disable();

    ocm_list_remove(&(object->list));

    ocm_hw_interrupt_enable(temp);
}

ocm_bool_t ocm_object_is_systemobject(ocm_object_t object)
{
    if (object->type & OCM_Object_Class_Static)
        return true;

    return false;
}

ocm_type_t ocm_object_get_type(ocm_object_t object)
{
    return object->type & (~OCM_Object_Class_Static);
}

/**
 * @brief: 
 * @param {type} 
 * @return {type} 
 */
ocm_object_t ocm_object_find(const char *name, ocm_uint8_t type)
{
    struct ocm_object *object = OCM_NULL;
    struct ocm_list_node *node = OCM_NULL;
    struct ocm_object_information *information = OCM_NULL;

    if ((name == OCM_NULL) || (type > OCM_Object_Class_Unknow))
        return OCM_NULL;

    /* try to find object */
    if (information == OCM_NULL)
    {
        information = ocm_object_get_information((enmu ocm_object_class_type)type);
    }

    for (node = information->object_list.next; node != &(information->object_list); node = node->next)
    {
        /* get object address*/
        object = ocm_list_entry(node, struct ocm_object, list);

        if (strncmp(object->name, name, OCM_NAME_MAX) == 0)
        {
            return object;
        }
    }

    return OCM_NULL;
}
