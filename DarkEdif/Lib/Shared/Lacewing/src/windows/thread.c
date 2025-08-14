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

static const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

static void SetThreadName(DWORD dwThreadID, const char* threadName) {
	THREADNAME_INFO info = { 0x1000, threadName, dwThreadID, 0 };
#pragma warning(push)
#pragma warning(disable: 6320 6322)
	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
	}
#pragma warning(pop)

	// Windows 10, version 1607, and later; requires a debugger OS just as recent
	fn_SetThreadDescription setThreadDesc = compat_SetThreadDescription();
	if (setThreadDesc)
	{
		wchar_t* wideVer = lw_char_to_wchar(threadName, -1);
		assert(wideVer);
		setThreadDesc(GetCurrentThread(), wideVer);
		free(wideVer);
	}
}

static int thread_proc (lw_thread ctx)
{
	SetThreadName(GetCurrentThreadId(), ctx->name);

	return ((int (__stdcall *) (void *)) ctx->proc) (ctx->param);
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

