/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.onecm.com
**
**------------------------------------File Info----------------------------------------------------------*/
/*
 * @Description: 双向链表
 * @version: 
 * @Author: Saber
 * @Date: 2020-08-25 15:01:17
 * @FilePath: \STM32F103C8_Demo\Core\OcmDataStream\ocm_list.h
 * @LastEditors: Saber
 * @LastEditTime: 2020-08-31 09:57:41
 * @**
 * **********************************************************************************************************
 */
#include "ocm_core.h"

/**
 * initialize a list object
 */
#define OCM_LIST_OBJECT_INIT(object) \
    {                                \
        &(object), &(object)         \
    }

/**
 *  list function defined
 */
ocm_inline void ocm_list_init(ocm_list_t *l)
{
    l->next = l->prev = l;
}

ocm_inline void ocm_list_insert_after(ocm_list_t *l, ocm_list_t *n)
{
    l->next->prev = n;
    n->next = l->next;
    l->next = n;
    n->prev = l;
}

ocm_inline void ocm_list_insert_before(ocm_list_t *l, ocm_list_t *n)
{
    l->prev->next = n;
    n->prev = l->prev;
    l->prev = n;
    n->next = l;
}

// 从链表中移除，并没有释放占用的内存
ocm_inline void ocm_list_remove(ocm_list_t *n)
{
    n->prev->next = n->next;
    n->next->prev = n->prev;

    n->next = n->prev = n;
}

ocm_inline void ocm_list_isempty(const ocm_list_t *l)
{
    return l->next == l;
}

ocm_inline ocm_size_t ocm_list_len(const ocm_list_t *l)
{
    ocm_size_t len = 0;
    const ocm_list_t *p = l;
    while (p->next != l)
    {
        p = p->next;
        len++;
    }
    return len;
}

/**
 *  list function defined
 */
/**
 * @brief get the struct for this entry
 * @param node the entry point
 * @param type the type of structure
 * @param member the name of list in structure
 */
#define ocm_list_entry(node, type, member) \
    ocm_container_of(node, type, member)

/**
 * @brief: iterate over a list
 * @param pos the rt_list_t * to use as a loop cursor
 * @param head the head for your list
 * @return {type} 
 */
#define ocm_list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * @brief: get the first element from a list
 * @param  ptr the list head to take the element from.
 * @param  type the type of the struct this is embedded in.
 * @param  member the name of the list_struct within the struct.
 * @return {type} 
 */
#define ocm_list_first_entry(ptr, type, member) \
    ocm_list_entry((ptr)->next, type, member)