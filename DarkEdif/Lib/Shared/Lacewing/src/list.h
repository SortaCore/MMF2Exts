/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#ifndef _list_h
#define _list_h

typedef struct list_head list_head;
typedef struct list_element list_element;

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

/* This file, along with list.c, implements a simple generic linked list with
 * the following characteristics:
 *
 *  - "Object in list" rather than "list in object": the listed objects do not
 *	have to be aware of the list
 *
 *  - The linked list logic (and internal representation) is separate from the
 *	macros
 *
 *  - The list head does not require initialisation other than being cleared
 *	with null bytes
 *
 *  - The macros are intuitive and do not require any unnecessary parameters.
 *	In particular, the list type does not need to be passed to each operation.
 *
 * To accomplish this we use a boatload of unholy macro tricks, rampant and
 * sadistic subversion of the C type system, and a liberal sprinkling of GCC
 * extensions to stamp out any remaining chance of portability.
 *
 * List operations:
 *
 *	list(type, name)				Declare a list
 *	list_push(list, value)			Push value to back
 *	list_push_front(list, value)	Push value to front
 *	list_pop(list)					Pop and return value from back
 *	list_pop_front(list)			Pop and return value from front
 *	list_length(list)				Returns the list length
 *	list_remove(list, value)		Remove first occurrence of value from list
 *	list_clear(list)				Clear the list (freeing all memory)
 *
 * Element operations:
 *
 *	list_elem_front(list)			Returns element at the front of list
 *	list_elem_back(list)			Returns element at the back of list
 *	list_elem_next(elem)			Returns element after elem
 *	list_elem_prev(elem)			Returns element before elem
 *	list_elem_remove(elem)			Remove element elem
 *
 * Loops:
 *
 *	list_each(list, elem) { ... }
 *
 *	 Loops through each list element, front to back.  elem will be declared
 *	 and set to the actual value of each element (not an elemator)
 *
 *	list_each_r(list, elem) { ... }
 *
 *	 Loops through each list element, back to front.  elem will be declared
 *	 and set to the actual value of each element (not an elemator)
 *
 *	list_each_elem(list, elem) { ... }
 *
 *	 Loops through each list element, front to back.  elem will be declared
 *	 and set to an elemator pointing to each element.
 *
 *	list_each_r_elem(list, elem) { ... }
 *
 *	 Loops through each list element, back to front.  elem will be declared
 *	 and set to an elemator pointing to each element.
 */

#define list_type(type) \
	type *

#define lw_list(type, name) \
	list_type (type) name

#define list_push(type, list, value) do {											\
	type _v_copy = value;						\
	_list_push ((list_head **) &list, sizeof (_v_copy), &_v_copy);			\
} while (0)

#define list_push_front(type, list, value) do {									\
	type _v_copy = value;						\
	_list_push_front ((list_head **) &list, sizeof (_v_copy), &_v_copy);	\
} while (0)

#define list_elem(type, list)														\
	type

#define list_elem_front(type, list)												\
	((type*) _list_front ((list_head *) list))

#define list_elem_back(type, list)												\
	((type*) _list_back ((list_head *) list))

#define list_elem_next(type, elem)												\
	((type*) _list_next ((list_element *) elem))

#define list_elem_prev(type, elem)												\
	((type*) _list_prev ((list_element *) elem))

#define list_each_elem(type, list, e)												\
	for (type* e = list_elem_front (type, list),							\
		* e##_next = list_elem_next (type, e); e;									\
		  e = e##_next, e##_next = list_elem_next (type, e))

#define list_each_r_elem(type, list, e)											\
	for (type e = list_elem_back (type, list),							\
		 e##_prev = list_elem_prev (type, e); e;									\
		  e = e##_prev, e##_prev = list_elem_prev (type, e))

#define list_each(type, list, e)													\
	for (type e, * _##e = list_elem_front (type, list),					\
		* _##e##_next = list_elem_next (type, _##e); _##e;						\
		  _##e = _##e##_next, _##e##_next = list_elem_next (type, _##e))			\
			 if (!memcpy (&e, _##e, sizeof (e))) {} else

#define list_each_r(type, list, e)												\
	for (type e, * _##e = list_elem_back (type, list),					\
		* _##e##_prev = list_elem_prev (type, _##e); _##e;						\
		  _##e = _##e##_prev, _##e##_prev = list_elem_prev (type, _##e))			\
			 if (!memcpy (&e, _##e, sizeof (e))) {} else

	#define list_find(type, list, value) \
	  type* elem = list_elem_front (type, list);						\
	  for (; elem; elem = list_elem_next (type, elem))							\
		 if (*elem == value)												\
			break;															\

#define list_elem_remove(elem)												\
	_list_remove ((list_element *) elem)

#define list_remove(type, list, value)											\
	{list_find (type, list, value);\
	list_elem_remove (elem);}

#define list_length(list)													\
	_list_length ((list_head *) (list))

#define list_clear(list)													\
	_list_clear ((list_head **) &(list), sizeof (*list))

#define list_front(type, list)													\
	(*list_elem_front (type, list))

#define list_back(type, list)														\
	(*list_elem_back (type, list))

#define list_pop_front(type, list)												\
	list_elem_remove (list_elem_front (type, list))

#define list_pop_back(list)													\
	list_elem_remove (list_elem_back (list))

#ifdef __cplusplus
	extern "C"
	{
#endif

void		  _list_push		(list_head **, size_t value_size, void * value);
void		  _list_push_front  (list_head **, size_t value_size, void * value);
list_element* _list_front		(list_head *);
list_element* _list_back		(list_head *);
list_element* _list_next		(list_element *);
list_element* _list_prev		(list_element *);
size_t		_list_length	  (list_head *);
void		  _list_remove	  (list_element *);
void		  _list_clear		(list_head **, size_t value_size);

#ifdef __cplusplus
	}
#endif

#endif

