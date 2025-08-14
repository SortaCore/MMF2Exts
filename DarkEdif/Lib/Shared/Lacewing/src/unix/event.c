/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012, 2013 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../common.h"

struct _lw_event
{
	int pipe_r, pipe_w;

	void * tag;
};

lw_event lw_event_new ()
{
	lw_event ctx = (lw_event) malloc (sizeof (*ctx));
	if (!ctx)
		return NULL;

	int p [2];
	if (pipe(p) == -1)
	{
		always_log ("error %d creating pipe for lw_event\n", errno);
		free(ctx);
		return NULL;
	}

	ctx->pipe_r = p [0];
	ctx->pipe_w = p [1];

	fcntl (ctx->pipe_r, F_SETFL, fcntl (ctx->pipe_r, F_GETFL, 0) | O_NONBLOCK);

	return ctx;
}

void lw_event_delete (lw_event ctx)
{
	if (!ctx)
		return;

	close (ctx->pipe_w);
	close (ctx->pipe_r);

	free (ctx);
}

lw_bool lw_event_signalled (lw_event ctx)
{
	fd_set set;

	FD_ZERO (&set);
	FD_SET (ctx->pipe_r, &set);

	struct timeval timeout = { 0 };

	return select (ctx->pipe_r + 1, &set, 0, 0, &timeout) > 0;
}

void lw_event_signal (lw_event ctx)
{
	ssize_t s = write (ctx->pipe_w, "", 1);
	(void)s; // we use it
	assert(s == 1);
}

void lw_event_unsignal (lw_event ctx)
{
	char buf [16];

	while (read (ctx->pipe_r, buf, sizeof (buf)) != -1)
	{
	}
}

lw_bool lw_event_wait (lw_event ctx, long timeout)
{
	fd_set set;

	FD_ZERO (&set);
	FD_SET (ctx->pipe_r, &set);

	if (timeout == -1)
	{
		return select (ctx->pipe_r + 1, &set, 0, 0, 0) > 0;
	}
	else
	{
		struct timeval tv;

		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;

		return select (ctx->pipe_r + 1, &set, 0, 0, &tv) > 0;
	}
}

void lw_event_set_tag (lw_event ctx, void * tag)
{
	ctx->tag = tag;
}

void * lw_event_tag (lw_event ctx)
{
	return ctx->tag;
}

