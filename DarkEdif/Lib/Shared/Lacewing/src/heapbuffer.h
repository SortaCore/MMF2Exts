/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#ifndef _lw_heap_buffer_h
#define _lw_heap_buffer_h

typedef struct _lwp_heapbuffer
{
	size_t length, allocated, offset;
	char buffer [1];

} * lwp_heapbuffer;

lw_bool lwp_heapbuffer_add (lwp_heapbuffer *, const char * buffer, size_t length);
void lwp_heapbuffer_addf (lwp_heapbuffer *, const char * format, ...);

void lwp_heapbuffer_trim_left (lwp_heapbuffer *, size_t);
void lwp_heapbuffer_trim_right (lwp_heapbuffer *, size_t);

void lwp_heapbuffer_reset (lwp_heapbuffer *);
size_t lwp_heapbuffer_length (lwp_heapbuffer *);

char * lwp_heapbuffer_buffer (lwp_heapbuffer *);

void lwp_heapbuffer_free (lwp_heapbuffer *);

#endif

