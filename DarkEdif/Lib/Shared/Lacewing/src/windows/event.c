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
	WSAEVENT event;

	void * tag;
};

lw_event lw_event_new ()
{
	lwp_init ();

	lw_event ctx = (lw_event) calloc (sizeof (*ctx), 1);

	if (!ctx)
	  return 0;

	ctx->event = WSACreateEvent ();

	return ctx;
}

void lw_event_delete (lw_event ctx)
{
	if (!ctx)
	  return;

	WSACloseEvent (ctx->event);
	lwp_deinit();

	free (ctx);
}

lw_bool lw_event_signalled (lw_event ctx)
{
	return WSAWaitForMultipleEvents
	(
	  1,
	  &ctx->event,
	  lw_true,
	  0,
	  lw_false

	) == WAIT_OBJECT_0;
}

void lw_event_signal (lw_event ctx)
{
	WSASetEvent (ctx->event);
}

void lw_event_unsignal (lw_event ctx)
{
	WSAResetEvent (ctx->event);
}

lw_bool lw_event_wait (lw_event ctx, long timeout)
{
	return WSAWaitForMultipleEvents
	(
	  1,
	  &ctx->event,
	  lw_true,
	  timeout == -1 ? INFINITE : timeout,
	  lw_false

	) == WAIT_OBJECT_0;
}

void lw_event_set_tag (lw_event ctx, void * tag)
{
	ctx->tag = tag;
}

void * lw_event_tag (lw_event ctx)
{
	return ctx->tag;
}

