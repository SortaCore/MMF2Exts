/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012 James McLaughlin.
 * Copyright (C) 2012-2022 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/licenses/mit-license.php
*/

#include "../common.h"

struct _lw_thread
{
	void * proc, * param;
	char * name;

	HANDLE thread;

	void * tag;
};

lw_thread lw_thread_new (const char * name, void * proc)
{
	lw_thread ctx = (lw_thread) calloc (sizeof (*ctx), 1);

	if (!ctx)
	  return 0;

	ctx->proc = proc;
	ctx->thread = INVALID_HANDLE_VALUE;
	ctx->name = strdup (name);

	return ctx;
}

void lw_thread_delete (lw_thread ctx)
{
	if (!ctx)
	  return;

	lw_thread_join (ctx);

	free (ctx->name);
	free (ctx);
}

static int thread_proc (lw_thread ctx)
{
	struct
	{
	  DWORD dwType;
	  LPCSTR szName;
	  DWORD dwThreadID;
	  DWORD dwFlags;

	} thread_name_info;

	thread_name_info.dwFlags	 = 0;
	thread_name_info.dwType	  = 0x1000;
	thread_name_info.szName	  = ctx->name;
	thread_name_info.dwThreadID  = -1;

	/* TODO */

	/* __try
	{	RaiseException (0x406D1388, 0,
						sizeof (thread_name_info) / sizeof (ULONG),
						(ULONG *) &thread_name_info);
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	} */

	return ((int (*) (void *)) ctx->proc) (ctx->param);
}

void lw_thread_start (lw_thread ctx, void * param)
{
	if (lw_thread_started (ctx))
	  return;

	ctx->param = param;

	ctx->thread = (HANDLE) _beginthreadex (0, 0,
		 (unsigned (__stdcall *) (void *)) thread_proc, ctx, 0, 0);
}

lw_bool lw_thread_started (lw_thread ctx)
{
	return ctx->thread != INVALID_HANDLE_VALUE;
}

void * lw_thread_join (lw_thread ctx)
{
	if (!lw_thread_started (ctx))
	  return (void *) -1;

	DWORD exit_code = -1;

	if (WaitForSingleObject (ctx->thread, INFINITE) == WAIT_OBJECT_0)
	  GetExitCodeThread (ctx->thread, &exit_code);
	CloseHandle(ctx->thread);
	ctx->thread = INVALID_HANDLE_VALUE;

	return (void *) (size_t)exit_code;
}

void lw_thread_set_tag (lw_thread ctx, void * tag)
{
	ctx->tag = tag;
}

void * lw_thread_tag (lw_thread ctx)
{
	return ctx->tag;
}

