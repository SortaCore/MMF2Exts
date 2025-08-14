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

struct _lw_thread
{
	void * proc, * param;
	char * name;

	pthread_t thread;
	lw_bool started;

	void * tag;
};

lw_thread lw_thread_new (const char * name, void * proc)
{
	lw_thread ctx = (lw_thread)calloc (sizeof (*ctx), 1);

	if (!ctx)
	  return 0;

	ctx->name = strdup (name);
	ctx->proc = proc;

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
	#ifdef HAVE_DECL_PR_SET_NAME
	  prctl (PR_SET_NAME, (unsigned long) ctx->name, 0, 0, 0);
	#endif

	int exit_code = ((int (*) (void *)) ctx->proc) (ctx->param);

	ctx->started = lw_false;

	return exit_code;
}

void lw_thread_start (lw_thread ctx, void * param)
{
	if (lw_thread_started (ctx))
	  return;

	ctx->param = param;

	ctx->started = pthread_create
	  (&ctx->thread, 0, (void * (*) (void *)) thread_proc, ctx) == 0;
}

lw_bool lw_thread_started (lw_thread ctx)
{
	return ctx->started;
}

void * lw_thread_join (lw_thread ctx)
{
	if (!lw_thread_started (ctx))
	  return (void *) -1;

	void * exit_code;

	if (pthread_join (ctx->thread, &exit_code))
	  return (void *) -1;

	return exit_code;
}

void lw_thread_set_tag (lw_thread ctx, void * tag)
{
	ctx->tag = tag;
}

void * lw_thread_tag (lw_thread ctx)
{
	return ctx->tag;
}

