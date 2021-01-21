/* vim: set et ts=3 sw=3 ft=c:
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

#include "json.h"

#ifdef _MSC_VER
	#ifndef _CRT_SECURE_NO_WARNINGS
	  #define _CRT_SECURE_NO_WARNINGS
	#endif
#endif

#ifdef __cplusplus
	const struct _json_value json_value_none; /* zero-d by ctor */
#else
	const struct _json_value json_value_none = { 0 };
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>


static unsigned char hex_value (json_char c)
{
	if (c >= 'A' && c <= 'F')
	  return (c - 'A') + 10;

	if (c >= 'a' && c <= 'f')
	  return (c - 'a') + 10;

	if (c >= '0' && c <= '9')
	  return c - '0';

	return 0xFF;
}

static void * default_alloc (size_t size, int zero, void * user_data)
{
	return zero ? calloc (size, 1) : malloc (size);
}

static void default_free (void * ptr, void * user_data)
{
	free (ptr);
}

static void * json_alloc (json_state * state, unsigned long size, int zero)
{
	if ((state->ulong_max - state->used_memory) < size)
	  return 0;

	if (state->settings.max_memory
		 && (state->used_memory += size) > state->settings.max_memory)
	{
	  return 0;
	}

	return state->settings.mem_alloc (size, zero, state->settings.user_data);
}

static int new_value
	(json_state * state, json_value ** top, json_value ** root, json_value ** alloc, json_type type)
{
	json_value * value;
	int values_size;

	if (!state->first_pass)
	{
	  value = *top = *alloc;
	  *alloc = (*alloc)->_reserved.next_alloc;

	  if (!*root)
		 *root = value;

	  switch (value->type)
	  {
		 case json_array:

			if (! (value->u.array.values = (json_value **) json_alloc
				(state, value->u.array.length * sizeof (json_value *), 0)) )
			{
				return 0;
			}

			value->u.array.length = 0;
			break;

		 case json_object:

			values_size = sizeof (*value->u.object.values) * value->u.object.length;

			if (! ((*(void **) &value->u.object.values) = json_alloc
				  (state, values_size + ((unsigned long) value->u.object.values), 0)) )
			{
				return 0;
			}

			value->_reserved.object_mem = (*(char **) &value->u.object.values) + values_size;

			value->u.object.length = 0;
			break;

		 case json_string:

			if (! (value->u.string.ptr = (json_char *) json_alloc
				(state, (value->u.string.length + 1) * sizeof (json_char), 0)) )
			{
				return 0;
			}

			value->u.string.length = 0;
			break;

		 default:
			break;
	  };

	  return 1;
	}

	value = (json_value *) json_alloc (state, sizeof (json_value), 1);

	if (!value)
	  return 0;

	if (!*root)
	  *root = value;

	value->type = type;
	value->parent = *top;

	if (*alloc)
	  (*alloc)->_reserved.next_alloc = value;

	*alloc = *top = value;

	return 1;
}

#define e_off \
	((int) (i - cur_line_begin))

#define whitespace \
	case '\n': ++ cur_line;  cur_line_begin = i; \
	case ' ': case '\t': case '\r'

#define string_add(b)  \
	do { if (!state.first_pass) string [string_length] = b;  ++ string_length; } while (0);

const static long
	flag_next			 = 1 << 0,
	flag_reproc			= 1 << 1,
	flag_need_comma		= 1 << 2,
	flag_seek_value		= 1 << 3,
	flag_escaped		  = 1 << 4,
	flag_string			= 1 << 5,
	flag_need_colon		= 1 << 6,
	flag_done			 = 1 << 7,
	flag_num_negative	 = 1 << 8,
	flag_num_zero		 = 1 << 9,
	flag_num_e			= 1 << 10,
	flag_num_e_got_sign	= 1 << 11,
	flag_num_e_negative	= 1 << 12;

json_value * json_parse_ex (json_settings * settings,
							const json_char * json,
							size_t length,
							char * error_buf,
							size_t error_buf_len)
{
	json_char error [128];
	unsigned int cur_line;
	const json_char * cur_line_begin, * i, * end;
	json_value * top, * root = 0, * alloc = 0;
	json_state state = { 0 };
	long flags;
	long num_digits, num_e;
	json_int_t num_fraction;

	error[0] = '\0';

	memcpy (&state.settings, settings, sizeof (json_settings));

	if (!state.settings.mem_alloc)
		state.settings.mem_alloc = default_alloc;

	if (!state.settings.mem_free)
		state.settings.mem_free = default_free;

	memset (&state.uint_max, 0xFF, sizeof (state.uint_max));
	memset (&state.ulong_max, 0xFF, sizeof (state.ulong_max));

	state.uint_max -= 8; /* limit of how much can be added before next check */
	state.ulong_max -= 8;

//	#pragma warning(disable:4133)

	if (!json_clean_comments (&json, &state, error, sizeof(error), &length))
		goto e_failed;
//	#pragma warning(default:4133)

	end = (json + length); // json_clean_comments changes length

	for (state.first_pass = 1; state.first_pass >= 0; -- state.first_pass)
	{
		json_uchar uchar;
		unsigned char uc_b1, uc_b2, uc_b3, uc_b4;
		json_char * string;
		unsigned int string_length;

		top = root = 0;
		flags = flag_seek_value;

		cur_line = 1;
		cur_line_begin = json;

		for (i = json ;; ++ i)
		{
			json_char b = (i == end ? 0 : *i);

			if (flags & flag_done)
			{
				if (!b)
					break;

				switch (b)
				{
					whitespace:
						continue;

					default:
						sprintf_s (error, "Line %d, char %d: Trailing garbage: `%c`", cur_line, e_off, b);
						goto e_failed;
				};
			}

			if (flags & flag_string)
			{
				if (!b)
				{	sprintf_s (error, "Unexpected EOF in string (at line %d, char %d)", cur_line, e_off);
					goto e_failed;
				}

				if (string_length > state.uint_max)
					goto e_overflow;

				if (flags & flag_escaped)
				{
					flags &= ~ flag_escaped;

					switch (b)
					{
						case 'b':  string_add ('\b');  break;
						case 'f':  string_add ('\f');  break;
						case 'n':  string_add ('\n');  break;
						case 'r':  string_add ('\r');  break;
						case 't':  string_add ('\t');  break;
						case 'u':

						if ((uc_b1 = hex_value (*++ i)) == 0xFF || (uc_b2 = hex_value (*++ i)) == 0xFF
								|| (uc_b3 = hex_value (*++ i)) == 0xFF || (uc_b4 = hex_value (*++ i)) == 0xFF)
						{
							sprintf_s (error, "Invalid character value `%c` (at line %d, char %d)", b, cur_line, e_off);
							goto e_failed;
						}

						uc_b1 = uc_b1 * 16 + uc_b2;
						uc_b2 = uc_b3 * 16 + uc_b4;

						uchar = ((json_char) uc_b1) * 256 + uc_b2;

						if (sizeof (json_char) >= sizeof (json_uchar) || (uc_b1 == 0 && uc_b2 <= 0x7F))
						{
							string_add ((json_char) uchar);
							break;
						}

						if (uchar <= 0x7FF)
						{
							if (state.first_pass)
								string_length += 2;
							else
							{	string [string_length ++] = 0xC0 | ((uc_b2 & 0xC0) >> 6) | ((uc_b1 & 0x7) << 2);
								string [string_length ++] = 0x80 | (uc_b2 & 0x3F);
							}

							break;
						}

						if (state.first_pass)
							string_length += 3;
						else
						{	string [string_length ++] = 0xE0 | ((uc_b1 & 0xF0) >> 4);
							string [string_length ++] = 0x80 | ((uc_b1 & 0xF) << 2) | ((uc_b2 & 0xC0) >> 6);
							string [string_length ++] = 0x80 | (uc_b2 & 0x3F);
						}

						break;

						default:
							string_add (b);
					};

					continue;
				}

				if (b == '\\')
				{
					flags |= flag_escaped;
					continue;
				}

				if (b == '"')
				{
					if (!state.first_pass)
						string [string_length] = 0;

					flags &= ~ flag_string;
					string = 0;

					switch (top->type)
					{
						case json_string:

							top->u.string.length = string_length;
							flags |= flag_next;

							break;

						case json_object:

							if (state.first_pass)
								(*(json_char **) &top->u.object.values) += string_length + 1;
							else
							{
								top->u.object.values [top->u.object.length].name
									= (json_char *) top->_reserved.object_mem;

								(*(json_char **) &top->_reserved.object_mem) += string_length + 1;
							}

							flags |= flag_seek_value | flag_need_colon;
							continue;

						default:
							break;
					};
				}
				else
				{
					string_add (b);
					continue;
				}
			}

			if (flags & flag_seek_value)
			{
				switch (b)
				{
					whitespace:
						continue;

					case ']':

						if (top->type == json_array)
							flags = (flags & ~ (flag_need_comma | flag_seek_value)) | flag_next;
						else if (!(state.settings.settings & json_relaxed_commas))
						{	sprintf_s (error, "Line %d, char %d: Unexpected ]", cur_line, e_off);
							goto e_failed;
						}

						break;

					default:

						if (flags & flag_need_comma)
						{
							if (b == ',')
							{	flags &= ~ flag_need_comma;
								continue;
							}
							else
							{	sprintf_s (error, "Line %d, char %d: Expected , before %c", cur_line, e_off, b);
								goto e_failed;
							}
						}

						if (flags & flag_need_colon)
						{
							if (b == ':')
							{	flags &= ~ flag_need_colon;
								continue;
							}
							else
							{	sprintf_s (error, "Line %d, char %d: Expected : before %c", cur_line, e_off, b);
								goto e_failed;
							}
						}

						flags &= ~ flag_seek_value;

						switch (b)
						{
							case '{':

								if (!new_value (&state, &top, &root, &alloc, json_object))
									goto e_alloc_failure;

								continue;

							case '[':

								if (!new_value (&state, &top, &root, &alloc, json_array))
									goto e_alloc_failure;

								flags |= flag_seek_value;
								continue;

							case '"':

								if (!new_value (&state, &top, &root, &alloc, json_string))
									goto e_alloc_failure;

								flags |= flag_string;

								string = top->u.string.ptr;
								string_length = 0;

								continue;

							case 't':

								if ((end - i) < 3 || strncmp(i, "true", 4))
									goto e_unknown_value;

								if (!new_value (&state, &top, &root, &alloc, json_boolean))
									goto e_alloc_failure;

								top->u.boolean = 1;

								i += 3; // NB: ++i in for() declaration
								flags |= flag_next;
								break;

							case 'f':

								if ((end - i) < 4 || strncmp(i, "false", 5))
									goto e_unknown_value;

								if (!new_value (&state, &top, &root, &alloc, json_boolean))
									goto e_alloc_failure;

								i += 4;
								flags |= flag_next;
								break;

							case 'n':

								if ((end - i) < 3 || strncmp(i, "null", 4))
									goto e_unknown_value;

								if (!new_value (&state, &top, &root, &alloc, json_null))
									goto e_alloc_failure;

								i += 3;
								flags |= flag_next;
								break;

							default:

								if (isdigit (b) || b == '-')
								{
									if (!new_value (&state, &top, &root, &alloc, json_integer))
										goto e_alloc_failure;

									if (!state.first_pass)
									{
										while (isdigit (b) || b == '+' || b == '-'
													 || b == 'e' || b == 'E' || b == '.')
										{
											b = *++ i;
										}

										flags |= flag_next | flag_reproc;
										break;
									}

									flags &= ~ (flag_num_negative | flag_num_e |
												flag_num_e_got_sign | flag_num_e_negative |
												flag_num_zero);

									num_digits = 0;
									num_fraction = 0;
									num_e = 0;

									if (b != '-')
									{
										flags |= flag_reproc;
										break;
									}

									flags |= flag_num_negative;
									continue;
								}
								else
								{	sprintf_s (error, "Line %d, char %d: Unexpected %c when seeking value", cur_line, e_off, b);
									goto e_failed;
								}
						};
				};
			}
			else
			{
				switch (top->type)
				{
					case json_object:
					{
						switch (b)
						{
							whitespace:
								continue;

							case '"':
								if (flags & flag_need_comma && (!(state.settings.settings & json_relaxed_commas)))
								{
									sprintf_s (error, "Line %d, char %d: Expected , before \"", cur_line, e_off);
									goto e_failed;
								}

								flags |= flag_string;

								string = (json_char *) top->_reserved.object_mem;
								string_length = 0;

								break;

							case '}':
								flags = (flags & ~ flag_need_comma) | flag_next;
								break;

							case ',':
								if (flags & flag_need_comma)
								{
									flags &= ~ flag_need_comma;
									break;
								}

							default:
								sprintf_s (error, b ? "Line %d, char %d: Unexpected `%c` in object" : "Line %d, char %d: Unexpected `\\0` in object", cur_line, e_off, b);
								goto e_failed;
						}
					};

					break;

					case json_integer:
					case json_double:
						if (isdigit (b))
						{
							++ num_digits;

							if (top->type == json_integer || flags & flag_num_e)
							{
								if (! (flags & flag_num_e))
								{
									if (flags & flag_num_zero)
									{	sprintf_s (error, "Line %d, char %d: Unexpected `0` before `%c`", cur_line, e_off, b);
										goto e_failed;
									}

									if (num_digits == 1 && b == '0')
										flags |= flag_num_zero;
								}
								else
								{
									flags |= flag_num_e_got_sign;
									num_e = (num_e * 10) + (b - '0');
									continue;
								}

								top->u.integer = (top->u.integer * 10) + (b - '0');
								continue;
							}

							num_fraction = (num_fraction * 10) + (b - '0');
							continue;
						}

						if (b == '+' || b == '-')
						{
							if ( (flags & flag_num_e) && !(flags & flag_num_e_got_sign))
							{
								flags |= flag_num_e_got_sign;
								if (b == '-')
									flags |= flag_num_e_negative;

								continue;
							}
						}
						else if (b == '.' && top->type == json_integer)
						{
							if (!num_digits)
							{	sprintf_s (error, "Line %d, char %d: Expected digit before `.`", cur_line, e_off);
								goto e_failed;
							}

							top->type = json_double;
							top->u.dbl = (double) top->u.integer;

							num_digits = 0;
							continue;
						}

						if (! (flags & flag_num_e))
						{
							if (top->type == json_double)
							{
								if (!num_digits)
								{	sprintf_s (error, "Line %d, char %d: Expected digit after `.`", cur_line, e_off);
									goto e_failed;
								}

								top->u.dbl += ((double) num_fraction) / (pow (10, (double) num_digits));
							}

							if (b == 'e' || b == 'E')
							{
								flags |= flag_num_e;

								if (top->type == json_integer)
								{
									top->type = json_double;
									top->u.dbl = (double) top->u.integer;
								}

								num_digits = 0;
								flags &= ~ flag_num_zero;

								continue;
							}
						}
						else
						{
							if (!num_digits)
							{	sprintf_s (error, "Line %d, char %d: Expected digit after `e`", cur_line, e_off);
								goto e_failed;
							}

							top->u.dbl *= pow (10, (double) (flags & flag_num_e_negative ? - num_e : num_e));
						}

						if (flags & flag_num_negative)
						{
							if (top->type == json_integer)
								top->u.integer = - top->u.integer;
							else
								top->u.dbl = - top->u.dbl;
						}

						flags |= flag_next | flag_reproc;
						break;

					default:
						break;
				}
			}

			if (flags & flag_reproc)
			{
				flags &= ~ flag_reproc;
				-- i;
			}

			if (flags & flag_next)
			{
				flags = (flags & ~ flag_next) | flag_need_comma;

				if (!top->parent)
				{
					/* root value done */

					flags |= flag_done;
					continue;
				}

				if (top->parent->type == json_array)
					flags |= flag_seek_value;

				if (!state.first_pass)
				{
					json_value * parent = top->parent;

					switch (parent->type)
					{
						case json_object:

							parent->u.object.values[parent->u.object.length].value = top;

							break;

						case json_array:

							parent->u.array.values[parent->u.array.length] = top;

							break;

						default:
							break;
					};
				}

				if ( (++ top->parent->u.array.length) > state.uint_max)
					goto e_overflow;
				top = top->parent;

				continue;
			}
		}

		alloc = root;
	}

	return root;

e_unknown_value:

	sprintf_s (error, "Line %d, char %d: Unknown value", cur_line, e_off);
	goto e_failed;

e_alloc_failure:

	strcpy_s (error, sizeof(error), "Memory allocation failure");
	goto e_failed;

e_overflow:

	sprintf_s (error, "Line %d, char %d: Too long (caught overflow)", cur_line, e_off);
	goto e_failed;

e_failed:

	if (error_buf)
		strcpy_s (error_buf, error_buf_len, (*error) ? error : "Unknown error");

	if (state.first_pass)
		alloc = root;

	while (alloc)
	{
		top = alloc->_reserved.next_alloc;
		state.settings.mem_free (alloc, state.settings.user_data);
		alloc = top;
	}

	if (!state.first_pass)
		json_value_free_ex (&state.settings, root);

	return 0;
}

json_value * json_parse (const json_char * json, size_t length)
{
	json_settings settings = { 0 };
	return json_parse_ex (&settings, json, length, 0, 0);
}

void json_value_free_ex (json_settings * settings, json_value * value)
{
	json_value * cur_value;

	if (!value)
	  return;

	value->parent = 0;

	while (value)
	{
	  switch (value->type)
	  {
		 case json_array:

			if (!value->u.array.length)
			{
				settings->mem_free (value->u.array.values, settings->user_data);
				break;
			}

			value = value->u.array.values [-- value->u.array.length];
			continue;

		 case json_object:

			if (!value->u.object.length)
			{
				settings->mem_free (value->u.object.values, settings->user_data);
				break;
			}

			value = value->u.object.values [-- value->u.object.length].value;
			continue;

		 case json_string:

			settings->mem_free (value->u.string.ptr, settings->user_data);
			break;

		 default:
			break;
	  };

	  cur_value = value;
	  value = value->parent;
	  settings->mem_free (cur_value, settings->user_data);
	}
}

void json_value_free (json_value * value)
{
	json_settings settings = { 0 };
	settings.mem_free = default_free;
	json_value_free_ex (&settings, value);
}

// The goal of this function is a preprocessor that removes all comments then writes the new array to json_input.
int json_clean_comments (const json_char ** json_input, json_state * state, char * const error,
	size_t error_len, size_t * _size)
{
#if EditorBuild
	#pragma warning(push)
	#pragma warning(disable: 4133)
	#pragma warning(disable: 4018)
	unsigned int size = *_size;
	// Used as an indicator whether i is currently inside a string var.
	int string = 0;
	int comment = 0;
	const char * json = *json_input, * i = json;

	char * newJSON = (char *)json_alloc(state, size, 1);
	char * j = newJSON;

	const json_char * cur_line_begin = 0;
	unsigned int cur_line = 0;

	for (i = json; i - json < size; ++i)
	{
		// Keeps a track of line count
		if (*i == '\n')
		{
			++cur_line;
			*(j++) = *i;
			if (comment == 0)
				cur_line_begin = i + 1;
			continue;
		}

		// Inside /* */ comment
		if (comment == 1)
		{
			// ...which just ended
			if (*i == '*' && *(i + 1) == '/')
			{
				++i;		  // Skip past '*'
				comment = 0;
			}

			continue;
		}

		// Not a comment: copy char to duplicate JSON allocation
		*(j++) = *i;

		// Sets string marker if " not escaped
		if (*i == '"' && *(i - 1) != '\\')
		{
			string = 1 - string;
			continue;
		}

		// Skip comment check if inside string or not a '/' character.
		if (string || *i != '/')
			continue;

		// <-- type of comment (newline ends it)
		if (*(i + 1) == '/')
		{
			// Did not find line break at any part in remainder of file.
			int startPos = i - json;
			if (!(i = strchr(i, '\n')))
			{
				sprintf_s(error, error_len, "Line %d, char %d: Opened // comment but no newline encountered.", cur_line, startPos);
				return 0;
			}

			++cur_line;
			*(j-1)	= '\r';	// Note j++ earlier
			*(j++)	= '\n';

			continue;
		}

		/* <-- type of comment --> */
		if (*(i + 1) == '*')
		{
			comment = 1;
			--j; // Note j++ earlier
			continue;
		}
	}

	if (comment)
	{
		// /* */ not completed
		sprintf_s(error, error_len, "Line %d, char %d: Opened /* */ comment without closing it.", cur_line, newJSON - cur_line_begin);
		return 0;
	}
	size = j - newJSON + 1;		// After skipping comments the size of the new buffer will be different
	newJSON[size - 1] = '\0';	// Ensure new JSON ends with a null terminator (i.e. End Of File)

	free((void *) *json_input);
	*json_input = (const char *)newJSON;
	*_size = size;

	return 1;
	#pragma warning(pop)

#else // Runtime build
	// Runtime has minified JSON, generated by DarkEdifPreBuildTool, which has no comments or spacing,
	// and all items not used at runtime stripped. It starts with a one-liner comment with a UTC timestamp,
	// indicating the latest change time of Extension.h, Extension.cpp, or DarkExt.json; whichever is latest.
	const char* json = *json_input;
	size_t size = *_size;

	// Max Int64 is 20 digits
	const char* newlineAt = size < 2 + 20 + 1 ? NULL : (const char*)memchr(&json[2], '\n', size);

	if (json[0] != '/' || json[1] != '/' || !newlineAt)
		goto WrongFormat;

	for (const char* j = &json[2]; j < newlineAt; j++)
		if (!isdigit(*j))
			goto WrongFormat;

	*_size = size - ((newlineAt + 1) - (*json_input));
	*json_input = newlineAt + 1;
	return 1;

	WrongFormat:
	strcpy_s(error, error_len, "JSON was not minified; does not start with //.");
	return 0;
#endif

}

