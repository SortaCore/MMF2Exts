/* vim: set et ts=4 sw=4 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin et al.  All rights reserved.
 * https://github.com/udp/json-parser
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
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

#ifndef _JSON_H
#define _JSON_H

#ifndef json_char
	#define json_char char
#endif

#ifndef json_int_t
	#ifndef _MSC_VER
	  #include <inttypes.h>
	  #include <cstdint>
	  #define json_int_t std::int64_t
	#else
	  #define json_int_t __int64
	#endif
#endif

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus

	#include <string.h>

	extern "C"
	{

#endif


typedef unsigned short json_uchar;

typedef struct
{
	unsigned long max_memory;
	int settings;

	/* Custom allocator support (leave null to use malloc/free)
	*/

	void * (* mem_alloc) (size_t, int zero, void * user_data);
	void (* mem_free) (void *, void * user_data);

	void * user_data;  /* will be passed to mem_alloc and mem_free */

} json_settings;

#define json_relaxed_commas 1

typedef enum
{
	json_none,
	json_object,
	json_array,
	json_integer,
	json_double,
	json_string,
	json_boolean,
	json_null

} json_type;

extern const struct _json_value json_value_none;

typedef struct _json_value
{
	struct _json_value * parent;

	json_type type;

	union
	{
	  int boolean;
	  json_int_t integer;
	  double dbl;

	  struct
	  {
		 unsigned int length;
		 json_char * ptr; /* null terminated */

	  } string;

	  struct
	  {
		 unsigned int length;

		 struct
		 {
			json_char * name;
			struct _json_value * value;

		 } * values;

		 #if defined(__cplusplus) && __cplusplus >= 201103L
		 decltype(values) begin () const
		 {  return values;
		 }
		 decltype(values) end () const
		 {  return values + length;
		 }
		 #endif

	  } object;

	  struct
	  {
		 unsigned int length;
		 struct _json_value ** values;

		 #if defined(__cplusplus) && __cplusplus >= 201103L
		 decltype(values) begin () const
		 {  return values;
		 }
		 decltype(values) end () const
		 {  return values + length;
		 }
		 #endif

	  } array;

	} u;

	union
	{
	  struct _json_value * next_alloc;
	  void * object_mem;

	} _reserved;


	/* Some C++ operator sugar */

	#ifdef __cplusplus

	  public:

		 inline _json_value ()
		 {  memset (this, 0, sizeof (_json_value));
		 }

		 // 64-bit systems only, where size_t is 64-bit and we'll get complaints about ambiguity/lost detail otherwise
#if (defined(__LP64__) && __LP64__) || (defined(_WIN64) && defined(_WIN32))
		 inline const struct _json_value & operator [] (const std::uint64_t index) const {
			 return (*this)[(const int)index];
		 }
#endif

		 inline const struct _json_value &operator [] (const int index) const
		 {
			if (type != json_array || index < 0
					 || ((unsigned int) index) >= u.array.length)
			{
				return json_value_none;
			}

			return *u.array.values[index];
		 }

		 inline const struct _json_value &operator [] (const char * index) const
		 {
			if (type != json_object)
				return json_value_none;

			for (unsigned int i = 0; i < u.object.length; ++ i)
				if (!strcmp(u.object.values[i].name, index))
					return *u.object.values[i].value;

			return json_value_none;
		 }

		 inline operator const char * () const
		 {
			switch (type)
			{
				case json_string:
				  return u.string.ptr;

				default:
				  return "";
			};
		 }

		 inline operator json_int_t () const
		 {
			switch (type)
			{
				case json_integer:
				  return u.integer;

				case json_double:
				  return (json_int_t) u.dbl;

				default:
				  return 0;
			};
		 }

		 inline operator bool () const
		 {
			if (type != json_boolean)
				return false;

			return u.boolean != 0;
		 }

		 inline operator double () const
		 {
			switch (type)
			{
				case json_integer:
				  return (double) u.integer;

				case json_double:
				  return u.dbl;

				default:
				  return 0;
			};
		 }

	#endif

} json_value;

json_value * json_parse (const json_char * json,
						 size_t length);

typedef struct
{
	unsigned long used_memory;

	unsigned int uint_max;
	unsigned long ulong_max;

	json_settings settings;
	int first_pass;
} json_state;
typedef json_state json_state;

int json_clean_comments (const json_char ** json_input,
						 json_state * state,
						 json_char * const error,
						 size_t error_len,
						 size_t * size);

json_value * json_parse_ex (json_settings * settings,
							const json_char * json,
							size_t length,
							char * error_buf,
							size_t error_buf_len);

void json_value_free (json_value *);


/* Not usually necessary, unless you used a custom mem_alloc and now want to
 * use a custom mem_free.
 */
void json_value_free_ex (json_settings * settings,
						 json_value *);


#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif
