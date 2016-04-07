
/* vim: set et ts=3 sw=3 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin et al.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
 *    have to be aware of the list
 *
 *  - The linked list logic (and internal representation) is separate from the
 *    macros
 *
 *  - The list head does not require initialisation other than being cleared
 *    with null bytes
 *
 *  - The macros are intuitive and do not require any unnecessary parameters.
 *    In particular, the list type does not need to be passed to each operation.
 *
 * To accomplish this we use a boatload of unholy macro tricks, rampant and
 * sadistic subversion of the C type system, and a liberal sprinkling of GCC
 * extensions to stamp out any remaining chance of portability.
 *
 * List operations:
 *
 *   list(type, name)                 Declare a list
 *   list_push(list, value)           Push value to back
 *   list_push_front(list, value)     Push value to front
 *   list_pop(list)                   Pop and return value from back
 *   list_pop_front(list)             Pop and return value from front
 *   list_length(list)                Returns the list length
 *   list_remove(list, value)         Remove first occurrence of value from list
 *   list_clear(list)                 Clear the list (freeing all memory)
 *
 * Element operations:
 *
 *   list_elem_front(list)            Returns element at the front of list
 *   list_elem_back(list)             Returns element at the back of list
 *   list_elem_next(elem)             Returns element after elem
 *   list_elem_prev(elem)             Returns element before elem
 *   list_elem_remove(elem)           Remove element elem
 *
 * Loops:
 * 
 *   list_each(list, elem) { ... }
 *
 *     Loops through each list element, front to back.  elem will be declared
 *     and set to the actual value of each element (not an elemator)
 *                      
 *   list_each_r(list, elem) { ... }
 *
 *     Loops through each list element, back to front.  elem will be declared
 *     and set to the actual value of each element (not an elemator)
 *
 *   list_each_elem(list, elem) { ... }
 *
 *     Loops through each list element, front to back.  elem will be declared
 *     and set to an elemator pointing to each element. 
 *                      
 *   list_each_r_elem(list, elem) { ... }
 *
 *     Loops through each list element, back to front.  elem will be declared
 *     and set to an elemator pointing to each element.
 */

#define list_type(type) \
   type *

#define list(type, name) \
   list_type (type) name 

#define list_push(list, value) do {                                           \
   typeof (*list) _v_copy = (typeof (*list)) value;                           \
   _list_push ((list_head **) &list, sizeof (_v_copy), &_v_copy);             \
} while (0)                                                                   \

#define list_push_front(list, value) do {                                     \
   typeof (*list) _v_copy = (typeof (*list)) value;                           \
   _list_push_front ((list_head **) &list, sizeof (_v_copy), &_v_copy);       \
} while (0)                                                                   \

#define list_elem(list)                                                       \
   typeof (list)

#define list_elem_front(list)                                                 \
   ((typeof (list)) _list_front ((list_head *) list))                         \

#define list_elem_back(list)                                                  \
   ((typeof (list)) _list_back ((list_head *) list))                          \

#define list_elem_next(elem)                                                  \
   ((typeof (elem)) _list_next ((list_element *) elem))                       \

#define list_elem_prev(elem)                                                  \
   ((typeof (elem)) _list_prev ((list_element *) elem))                       \

#define list_each_elem(list, e)                                               \
   for (typeof (list) e = list_elem_front (list),                             \
         e##_next = list_elem_next (e); e;                                    \
          e = e##_next, e##_next = list_elem_next (e))                        \

#define list_each_r_elem(list, e)                                             \
   for (typeof (list) e = list_elem_back (list),                              \
         e##_prev = list_elem_prev (e); e;                                    \
          e = e##_prev, e##_prev = list_elem_prev (e))                        \

#define list_each(list, e)                                                    \
   for (typeof (*list) e, * _##e = list_elem_front (list),                    \
        * _##e##_next = list_elem_next (_##e); _##e;                          \
          _##e = _##e##_next, _##e##_next = list_elem_next (_##e))            \
             if (!memcpy (&e, _##e, sizeof (e))) {} else                      \

#define list_each_r(list, e)                                                  \
   for (typeof (*list) e, * _##e = list_elem_back (list),                     \
        * _##e##_prev = list_elem_prev (_##e); _##e;                          \
          _##e = _##e##_prev, _##e##_prev = list_elem_prev (_##e))            \
             if (!memcpy (&e, _##e, sizeof (e))) {} else                      \

/* The only macro that has to use a statement expression.  As list_find has to
 * be able to create an l-value from value but also has to work as an
 * expression, I can't see a way to accomplish this without.
 *
 * Since MSVC doesn't support statement expressions, a template function is
 * used instead.
 */

#ifndef _MSC_VER
   #define list_find(list, value) ({                                          \
      list_elem (list) elem = list_elem_front (list);                         \
      for (; elem; elem = list_elem_next (elem))                              \
         if (*elem == value)                                                  \
            break;                                                            \
      elem;                                                                   \
   })                                                                           
#else
   template <class T> inline T * list_find (T * list, T value)
   {
      list_elem (list) elem = list_elem_front (list);
      for (; elem; elem = list_elem_next (elem))    
         if (*elem == value)                       
            return elem;
      return 0;
   }
#endif

#define list_remove(list, value)                                              \
   list_elem_remove (list_find (list, value))                                 \

#define list_elem_remove(elem)                                                \
   _list_remove ((list_element *) elem)                                       \

#define list_length(list)                                                     \
   _list_length ((list_head *) (list))                                        \

#define list_clear(list)                                                      \
   _list_clear ((list_head **) &(list), sizeof (*list))                       \

#define list_front(list)                                                      \
   (*list_elem_front (list))                                                  \

#define list_back(list)                                                       \
   (*list_elem_back (list))                                                   \

#define list_pop_front(list)                                                  \
   list_elem_remove (list_elem_front (list))                                  \

#define list_pop_back(list)                                                   \
   list_elem_remove (list_elem_back (list))                                   \

#ifdef __cplusplus
   extern "C"
   {
#endif

void          _list_push        (list_head **, size_t value_size, void * value);
void          _list_push_front  (list_head **, size_t value_size, void * value);
list_element* _list_front       (list_head *);
list_element* _list_back        (list_head *);
list_element* _list_next        (list_element *);
list_element* _list_prev        (list_element *);
size_t        _list_length      (list_head *);
void          _list_remove      (list_element *);
void          _list_clear       (list_head **, size_t value_size);

#ifdef __cplusplus
   }
#endif

#endif

