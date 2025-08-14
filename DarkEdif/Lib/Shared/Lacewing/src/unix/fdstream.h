/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#ifndef _lw_fdstream_h
#define _lw_fdstream_h

#include "../stream.h"

struct _lw_fdstream
{
	struct _lw_stream stream;

	//lw_pump_watch watch;

	int fd;

	lw_i8 flags;

	size_t size;
	size_t reading_size;
};

#define lwp_fdstream_flag_nagle		((lw_i8)1)
#define lwp_fdstream_flag_is_socket	((lw_i8)2)
#define lwp_fdstream_flag_autoclose	((lw_i8)4)
#define lwp_fdstream_flag_reading	 ((lw_i8)8)

void lwp_fdstream_init (lw_fdstream, lw_pump);

#endif


