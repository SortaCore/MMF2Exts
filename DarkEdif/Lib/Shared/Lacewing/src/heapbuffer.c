/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "common.h"

void lwp_heapbuffer_free (lwp_heapbuffer * ctx)
{
	if (!*ctx)
	  return;

	free (*ctx);
	*ctx = 0;
}

lw_bool lwp_heapbuffer_add (lwp_heapbuffer * ctx, const char * buffer, size_t length)
{
	/* TODO: discard data before the offset (might save a realloc) */

	if (length == SIZE_MAX)
	  length = strlen (buffer);

	if (length == 0)
	  return lw_true;  /* nothing to do */

	if (!*ctx)
	{
	  size_t init_alloc = (length * 3);

	  if (! (*ctx = (lwp_heapbuffer) malloc (sizeof (**ctx) + init_alloc)))
		 return lw_false;

	#ifdef _MSC_VER
		#pragma warning (suppress: 6386) // No, it's not overrunning
	#endif
	  memset (*ctx, 0, sizeof (**ctx));

	  (*ctx)->allocated = init_alloc;
	}
	else
	{
	  size_t new_length = (*ctx)->length + length;

	  if (new_length > (*ctx)->allocated)
	  {
		 while (new_length > (*ctx)->allocated)
			(*ctx)->allocated *= 3;

		 if (! (*ctx = (lwp_heapbuffer) lw_realloc_or_exit
					(*ctx, sizeof (**ctx) + (*ctx)->allocated)))
		 {
			return lw_false;
		 }
	  }
	}

	memcpy ((*ctx)->buffer + (*ctx)->length, buffer, length);
	(*ctx)->length += length;

	return lw_true;
}

void lwp_heapbuffer_addf (lwp_heapbuffer * ctx, const char * format, ...)
{
	va_list args;
	va_start (args, format);

	char * buffer;
	ssize_t length = lwp_format (&buffer, format, args);

	if (length > 0)
	{
	  lwp_heapbuffer_add (ctx, buffer, (size_t)length);
	  free (buffer);
	}

	va_end (args);
}

void lwp_heapbuffer_reset (lwp_heapbuffer * ctx)
{
	if (!*ctx)
	  return;

	(*ctx)->length = (*ctx)->offset = 0;
}

size_t lwp_heapbuffer_length (lwp_heapbuffer * ctx)
{
	if (!*ctx)
	  return 0;

	return (*ctx)->length - (*ctx)->offset;
}

char * lwp_heapbuffer_buffer (lwp_heapbuffer * ctx)
{
	if (!*ctx)
	  return 0;

	return (*ctx)->buffer + (*ctx)->offset;
}

void lwp_heapbuffer_trim_left (lwp_heapbuffer * ctx, size_t length)
{
	if (!*ctx)
	  return;

	(*ctx)->offset += length;
}

void lwp_heapbuffer_trim_right (lwp_heapbuffer * ctx, size_t length)
{
	if (!*ctx)
	  return;

	(*ctx)->length -= length;
}

