/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "list.h"
#include "common.h"

/* This file contains private helper functions for the macros defined in list.h.
 *
 * These functions provide the actual linked list logic, keeping it out of the
 * macros (and therefore out of anything but list.o).
 */

struct list_head
{
	list_element * first, * last;
	size_t length;
};

struct list_element
{
	list_head * list;
	list_element * prev, * next;
};

static list_element * get_element (list_element * elem)
{
	return elem ? (elem - 1) : 0;
}

static void * get_value_ptr (list_element * elem)
{
	return elem ? (elem + 1) : 0;
}

size_t _list_length (list_head * list)
{
	return list ? list->length : 0;
}

void _list_push (list_head ** p_list, size_t value_size, void * value)
{
	if (!*p_list)
		*p_list = (list_head*)lw_calloc_or_exit (sizeof(list_head), 1);

	list_head * list = *p_list;

	++ list->length;

	list_element * elem = (list_element *)
		lw_malloc_or_exit (sizeof (*elem) + value_size);

	memset (elem, 0, sizeof (*elem));
	memcpy (get_value_ptr (elem), value, value_size);

	elem->list = list;
	elem->prev = list->last;

	if (list->last)
	{
		list->last->next = elem;
		list->last = elem;
	}
	else
	{
		list->first = list->last = elem;
	}
}

void _list_push_front (list_head ** p_list, size_t value_size, void * value)
{
	if (!*p_list)
		*p_list = (list_head *)lw_calloc_or_exit(sizeof(list_head), 1);

	list_head * list = *p_list;

	++ list->length;

	list_element * elem = (list_element *)
		lw_malloc_or_exit (sizeof (*elem) + value_size);

	memset (elem, 0, sizeof (*elem));
	memcpy (get_value_ptr (elem), value, value_size);

	elem->list = list;
	elem->next = list->first;

	if (list->first)
	{
		list->first->prev = elem;
		list->first = elem;
	}
	else
	{
		list->first = list->last = elem;
	}
}

list_element * _list_front (list_head * list)
{
	return list ? (list_element *) get_value_ptr (list->first) : 0;
}

list_element * _list_back (list_head * list)
{
	return list ? (list_element *) get_value_ptr (list->last) : 0;
}

list_element * _list_next (list_element * elem)
{
	if (!elem || ! (elem = get_element (elem)->next))
	  return 0;

	return (list_element *) get_value_ptr (elem);
}

list_element * _list_prev (list_element * elem)
{
	if (!elem || ! (elem = get_element (elem)->prev))
	  return 0;

	return (list_element *) get_value_ptr (elem);
}

// Worth noting this does not free list_head, mostly because list_each and such
// complain if the list is edited during a loop. list_clear() does free list_head.
void _list_remove (list_element * elem)
{
	if (!elem)
		return;

	elem = get_element (elem);

	list_head * list = elem->list;

	-- list->length;

	if (elem->next)
		elem->next->prev = elem->prev;

	if (elem->prev)
		elem->prev->next = elem->next;

	if (elem == list->first)
		list->first = elem->next;

	if (elem == list->last)
		list->last = elem->prev;

	free (elem);
}

void _list_clear (list_head ** list, size_t value_size)
{
	if (!*list)
		return;

	list_element * elem = (*list)->first;

	while (elem)
	{
		list_element * next = elem->next;
		free (elem);
		elem = next;
	}

	free (*list);
	*list = 0;
}
