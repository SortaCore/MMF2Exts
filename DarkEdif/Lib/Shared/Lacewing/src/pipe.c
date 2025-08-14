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
#include "stream.h"

static lw_bool def_is_transparent (lw_stream ctx)
{
	return lw_true;
}

const static lw_streamdef def_pipe =
{
	0, /* sink_data */
	0, /* sink_stream */
	0, /* retry */
	def_is_transparent,
	0, /* close */
	0, /* bytes_left */
	0, /* read */
	0  /* cleanup */
};

void lwp_pipe_init (lw_stream ctx, lw_pump pump)
{
	lwp_stream_init (ctx, &def_pipe, pump);
}

lw_stream lw_pipe_new (lw_pump pump)
{
	lw_stream pipe = (lw_stream) malloc (sizeof (*pipe));

	if (!pipe)
		return NULL;

	lwp_pipe_init (pipe, pump);

	return pipe;
}

