/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../common.h"

struct _lw_sync
{
	pthread_mutex_t mutex;
	pthread_mutexattr_t attr;
};

lw_sync lw_sync_new ()
{
	lw_sync ctx = (lw_sync)malloc (sizeof (*ctx));

	if (!ctx)
		return 0;

	pthread_mutexattr_init (&ctx->attr);
	pthread_mutexattr_settype (&ctx->attr, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_init (&ctx->mutex, &ctx->attr);

	return ctx;
}

void lw_sync_delete (lw_sync ctx)
{
	pthread_mutex_destroy (&ctx->mutex);
	pthread_mutexattr_destroy (&ctx->attr);
}

void lw_sync_lock (lw_sync ctx)
{
	pthread_mutex_lock (&ctx->mutex);
}

void lw_sync_release (lw_sync ctx)
{
	pthread_mutex_unlock (&ctx->mutex);
}

