/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../common.h"

struct _lw_sync
{
	CRITICAL_SECTION critical_section;
};

lw_sync lw_sync_new ()
{
	lw_sync ctx = (lw_sync) malloc (sizeof (*ctx));

	if (!ctx)
		return 0;

	InitializeCriticalSection (&ctx->critical_section);

	return ctx;
}

void lw_sync_delete (lw_sync ctx)
{
	if (!ctx)
		return;

	DeleteCriticalSection (&ctx->critical_section);

	free (ctx);
}

void lw_sync_lock (lw_sync ctx)
{
	EnterCriticalSection (&ctx->critical_section);
}

void lw_sync_release (lw_sync ctx)
{
	LeaveCriticalSection (&ctx->critical_section);
}

