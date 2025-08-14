/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#ifndef _lw_fdstream_h
#define _lw_fdstream_h

#include "../stream.h"

typedef struct _fdstream_overlapped
{
	OVERLAPPED overlapped;

	char type;

	char data [1];

} * fdstream_overlapped;

struct _lw_fdstream
{
	struct _lw_stream stream;

	struct _fdstream_overlapped read_overlapped;
	struct _fdstream_overlapped transmitfile_overlapped;

	lw_fdstream transmit_file_from,
				transmit_file_to;

	char buffer [lwp_default_buffer_size];

	HANDLE fd;

	size_t size;
	size_t reading_size;

	LARGE_INTEGER offset;

	char flags;

	/* The number of pending writes.  May not be the same as
	* list_length(pending_writes) because transmitfile counts as a pending
	* write too, in both the source and dest stream.
	*/
	int num_pending_writes;

	lw_list (fdstream_overlapped, pending_writes);
	lw_sync pending_writes_sync;

	lw_fdstream transmitfile_from, transmitfile_to;
};

#define lwp_fdstream_flag_read_pending	 1
#define lwp_fdstream_flag_nagle			2
#define lwp_fdstream_flag_is_socket		4
#define lwp_fdstream_flag_close_asap		8  /* FD close pending on write? */
#define lwp_fdstream_flag_auto_close		16

void lwp_fdstream_init (lw_fdstream, lw_pump);

#endif
