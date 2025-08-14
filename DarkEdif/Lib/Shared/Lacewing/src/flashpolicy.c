/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012, 2013 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "common.h"
#include "flashpolicy.h"

static void on_data (lw_server server, lw_server_client client,
					 const char * buffer, size_t size)
{
	lw_flashpolicy ctx = (lw_flashpolicy) lw_server_tag (server);

	for (size_t i = 0; i < size; ++ i)
	{
		if (!buffer [i])
		{
			lw_stream_write ((lw_stream) client, ctx->buffer, ctx->size);
			lw_stream_write ((lw_stream) client, "", 1);

			return;
		}
	}
}

static void on_error (lw_server server, lw_error error)
{
	lw_flashpolicy ctx = (lw_flashpolicy) lw_server_tag (server);

	lw_error_addf (error, "Socket error");

	if (ctx->on_error)
		ctx->on_error (ctx, error);
}

lw_flashpolicy lw_flashpolicy_new (lw_pump pump)
{
	lw_flashpolicy ctx = (lw_flashpolicy) calloc (sizeof (*ctx), 1);

	if (!ctx)
		return NULL;

	ctx->server = lw_server_new (pump);
	
	if (!ctx->server)
	{
		free(ctx);
		return NULL;
	}
	
	lw_server_set_tag(ctx->server, ctx);

	lw_server_on_error (ctx->server, on_error);
	lw_server_on_data (ctx->server, on_data);

	return ctx;
}

void lw_flashpolicy_delete (lw_flashpolicy ctx)
{
	if (!ctx)
		return;

	lw_server_delete (ctx->server);

	free (ctx->buffer);
	free (ctx);
}

void lw_flashpolicy_host (lw_flashpolicy ctx, const char * filename)
{
	lw_filter filter = lw_filter_new ();
	lw_flashpolicy_host_filter (ctx, filename, filter);
	lw_filter_delete (filter);
}

void lw_flashpolicy_host_filter (lw_flashpolicy ctx, const char * filename,
								 lw_filter filter)
{
	lw_flashpolicy_unhost (ctx);

	lw_filter_set_local_port (filter, 843);

#if defined(_WIN32) && defined(_UNICODE)
	FILE * file = NULL;
	wchar_t * res = lw_char_to_wchar(filename, -1);
	if (res != NULL)
	{
		file = _wfopen(res, L"rb");
		free(res);
	}
#else
	FILE * file = fopen (filename, "rb");
#endif

	if (!file)
	{
		lw_error error = lw_error_new ();

		lw_error_add (error, lwp_last_socket_error);
		lw_error_addf (error, "Error opening file: %s", filename);

		if (ctx->on_error)
			ctx->on_error (ctx, error);

		return;
	}

	fseek (file, 0, SEEK_END);

	ctx->size = (size_t)ftell (file);
	ctx->buffer = (char *) lw_malloc_or_exit (ctx->size);

	fseek (file, 0, SEEK_SET);

	size_t bytes = fread (ctx->buffer, 1, ctx->size, file);

	if (bytes != ctx->size)
	{
		lw_error error = lw_error_new ();

		if (ferror (file))
			lw_error_add (error, lwp_last_error);
		else
			lw_error_addf(error, "Only read %zu bytes", bytes);
		lw_error_addf (error, "Error reading flash policy file: %s", filename);

		if (ctx->on_error)
			ctx->on_error (ctx, error);

		free (ctx->buffer);
		ctx->buffer = NULL;

		fclose (file);
		return;
	}

	fclose (file);

	lw_server_host_filter (ctx->server, filter);
}

void lw_flashpolicy_unhost (lw_flashpolicy ctx)
{
	lw_server_unhost (ctx->server);

	free (ctx->buffer);
	ctx->buffer = 0;
}

lw_bool lw_flashpolicy_hosting (lw_flashpolicy ctx)
{
	return lw_server_hosting (ctx->server);
}

void lw_flashpolicy_set_tag (lw_flashpolicy ctx, void * tag)
{
	ctx->tag = tag;
}

void * lw_flashpolicy_tag (lw_flashpolicy ctx)
{
	return ctx->tag;
}

lwp_def_hook (flashpolicy, error);

