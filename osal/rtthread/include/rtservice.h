/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * @file        rtservice.h
 *
 * @brief       RT-Thread adaper macro definition header file.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#ifndef __RT_SERVICE_H__
#define __RT_SERVICE_H__

#include <rtdef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 ***********************************************************************************************************************
 * @def         rt_container_of
 *
 * @brief       Cast a member of a structure out to the containing structure.
 *
 * @param       ptr             The pointer to the member.
 * @param       type            The type of the container struct this is embedded in.
 * @param       member          The name of the member within the struct. 
 ***********************************************************************************************************************
 */
#define rt_container_of(ptr, type, member)  \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/* Initialize a list object */
#define RT_LIST_OBJECT_INIT(object)         \
    {                                       \
        &(object),                          \
        &(object)                           \
    }

/**
 ***********************************************************************************************************************
 * @brief           Initialize a list
 *
 * @param[in]       l               List to be initialized
 *
 * @return          No return value
 ***********************************************************************************************************************
 */
rt_inline void rt_list_init(rt_list_t *l)
{
    l->next = l->prev = l;
}

/**
 ***********************************************************************************************************************
 * @brief           Insert a node after a list
 *
 * @param[in]       l               List to insert it
 * @param[in]       n               New node to be inserted
 *
 * @return          No return value
 ***********************************************************************************************************************
 */
rt_inline void rt_list_insert_after(rt_list_t *l, rt_list_t *n)
{
    l->next->prev = n;
    n->next = l->next;

    l->next = n;
    n->prev = l;
}

/**
 * @brief insert a node before a list
 *
 * @param n new node to be inserted
 * @param l list to insert it
 */
rt_inline void rt_list_insert_before(rt_list_t *l, rt_list_t *n)
{
    l->prev->next = n;
    n->prev = l->prev;

    l->prev = n;
    n->next = l;
}

/**
 * @brief remove node from list.
 * @param n the node to remove from the list.
 */
rt_inline void rt_list_remove(rt_list_t *n)
{
    n->next->prev = n->prev;
    n->prev->next = n->next;

    n->next = n->prev = n;
}

/**
 * @brief tests whether a list is empty
 * @param l the list to test.
 */
rt_inline int rt_list_isempty(const rt_list_t *l)
{
    return l->next == l;
}

/**
 * @brief get the list length
 * @param l the list to get.
 */
rt_inline unsigned int rt_list_len(const rt_list_t *l)
{
    unsigned int len = 0;
    const rt_list_t *p = l;
    while (p->next != l)
    {
        p = p->next;
        len ++;
    }

    return len;
}

/**
 * @brief get the struct for this entry
 * @param node the entry point
 * @param type the type of structure
 * @param member the name of list in structure
 */
#define rt_list_entry(node, type, member)   rt_container_of(node, type, member)

/**
 * rt_list_for_each - iterate over a list
 * @pos:	the rt_list_t * to use as a loop cursor.
 * @head:	the head for your list.
 */
#define rt_list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * rt_list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:	the rt_list_t * to use as a loop cursor.
 * @n:		another rt_list_t * to use as temporary storage
 * @head:	the head for your list.
 */
#define rt_list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * rt_list_for_each_entry  -   iterate over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define rt_list_for_each_entry(pos, head, member) \
    for (pos = rt_list_entry((head)->next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = rt_list_entry(pos->member.next, typeof(*pos), member))

/**
 * rt_list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define rt_list_for_each_entry_safe(pos, n, head, member) \
    for (pos = rt_list_entry((head)->next, typeof(*pos), member), \
         n = rt_list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = n, n = rt_list_entry(n->member.next, typeof(*n), member))

/**
 * rt_list_first_entry - get the first element from a list
 * @ptr:    the list head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define rt_list_first_entry(ptr, type, member) \
    rt_list_entry((ptr)->next, type, member)

#define RT_SLIST_OBJECT_INIT(object) { RT_NULL }

/**
 * @brief initialize a single list
 *
 * @param l the single list to be initialized
 */
rt_inline void rt_slist_init(rt_slist_t *l)
{
    l->next = RT_NULL;
}

rt_inline void rt_slist_append(rt_slist_t *l, rt_slist_t *n)
{
    struct rt_slist_node *node;

    node = l;
    while (node->next) node = node->next;

    /* append the node to the tail */
    node->next = n;
    n->next = RT_NULL;
}

rt_inline void rt_slist_insert(rt_slist_t *l, rt_slist_t *n)
{
    n->next = l->next;
    l->next = n;
}

rt_inline unsigned int rt_slist_len(const rt_slist_t *l)
{
    unsigned int len = 0;
    const rt_slist_t *list = l->next;
    while (list != RT_NULL)
    {
        list = list->next;
        len ++;
    }

    return len;
}

rt_inline rt_slist_t *rt_slist_remove(rt_slist_t *l, rt_slist_t *n)
{
    /* remove slist head */
    struct rt_slist_node *node = l;
    while (node->next && node->next != n) node = node->next;

    /* remove node */
    if (node->next != (rt_slist_t *)0) node->next = node->next->next;

    return l;
}

rt_inline rt_slist_t *rt_slist_first(rt_slist_t *l)
{
    return l->next;
}

rt_inline rt_slist_t *rt_slist_tail(rt_slist_t *l)
{
    while (l->next) l = l->next;

    return l;
}

rt_inline rt_slist_t *rt_slist_next(rt_slist_t *n)
{
    return n->next;
}

rt_inline int rt_slist_isempty(rt_slist_t *l)
{
    return l->next == RT_NULL;
}

/**
 * @brief get the struct for this single list node
 * @param node the entry point
 * @param type the type of structure
 * @param member the name of list in structure
 */
#define rt_slist_entry(node, type, member) \
    rt_container_of(node, type, member)

/**
 * rt_slist_for_each - iterate over a single list
 * @pos:    the rt_slist_t * to use as a loop cursor.
 * @head:   the head for your single list.
 */
#define rt_slist_for_each(pos, head) \
    for (pos = (head)->next; pos != RT_NULL; pos = pos->next)

/**
 * rt_slist_for_each_entry  -   iterate over single list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your single list.
 * @member: the name of the list_struct within the struct.
 */
#define rt_slist_for_each_entry(pos, head, member) \
    for (pos = rt_slist_entry((head)->next, typeof(*pos), member); \
         &pos->member != (RT_NULL); \
         pos = rt_slist_entry(pos->member.next, typeof(*pos), member))

/**
 * rt_slist_first_entry - get the first element from a slist
 * @ptr:    the slist head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the slist_struct within the struct.
 *
 * Note, that slist is expected to be not empty.
 */
#define rt_slist_first_entry(ptr, type, member) \
    rt_slist_entry((ptr)->next, type, member)

/**
 * rt_slist_tail_entry - get the tail element from a slist
 * @ptr:    the slist head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the slist_struct within the struct.
 *
 * Note, that slist is expected to be not empty.
 */
#define rt_slist_tail_entry(ptr, type, member) \
    rt_slist_entry(rt_slist_tail(ptr), type, member)

#ifdef __cplusplus
}
#endif

#endif /* __RT_SERVICE_H__ */

