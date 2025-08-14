/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012, 2013 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "common.h"

lw_ws_upload lwp_ws_upload_new (lw_ws_req request)
{
	lw_ws_upload ctx = (lw_ws_upload) calloc (sizeof (*ctx), 1);

	if (!ctx)
	  return 0;

	ctx->request = request;

	return ctx;
}

void lwp_ws_upload_delete (lw_ws_upload ctx)
{
	lwp_trace("Free upload!");

	lwp_nvhash_clear (&ctx->disposition);

	list_each (struct _lw_ws_upload_hdr, ctx->headers, header)
	{
	  free (header.name);
	  free (header.value);
	}

	list_clear (ctx->headers);

	lw_stream_delete ((lw_stream) ctx->autosave_file);
	free (ctx->autosave_filename);
}

const char * lw_ws_upload_filename (lw_ws_upload ctx)
{
	const char * slash, * filename =
	  lwp_nvhash_get (&ctx->disposition, "filename", "");

	/* Old versions of IE send the absolute path */

	if ((slash = strrchr (filename, '/')))
	  return slash + 1;

	if ((slash = strrchr (filename, '\\')))
	  return slash + 1;

	return filename;
}

const char * lw_ws_upload_form_el_name (lw_ws_upload ctx)
{
	return lwp_nvhash_get (&ctx->disposition, "name", "");
}

const char * lw_ws_upload_header (lw_ws_upload ctx, const char * name)
{
	list_each (struct _lw_ws_upload_hdr, ctx->headers, header)
	{
	  if (!strcasecmp (header.name, name))
		 return header.value;
	}

	return "";
}

lw_ws_upload_hdr lw_ws_upload_hdr_first (lw_ws_upload ctx)
{
	return list_elem_front (struct _lw_ws_upload_hdr, ctx->headers);
}

const char * lw_ws_upload_hdr_name (lw_ws_upload_hdr header)
{
	return header->name;
}

const char * lw_ws_upload_hdr_value (lw_ws_upload_hdr header)
{
	return header->value;
}

lw_ws_upload_hdr lw_ws_upload_hdr_next (lw_ws_upload_hdr header)
{
	return list_elem_next (struct _lw_ws_upload_hdr, header);
}

static void on_autosave_close (lw_stream stream, void * tag)
{
	lw_ws_upload upload = (lw_ws_upload) tag;

	lw_stream_delete ((lw_stream) upload->autosave_file);
	upload->autosave_file = 0;

	lwp_ws_multipart_call_hook (upload->request->client->multipart);
}

void lw_ws_upload_set_autosave (lw_ws_upload ctx)
{
	if (ctx->autosave_file)
	  return;

	ctx->autosave_file = lw_file_new (ctx->request->ws->pump);

	lw_stream_add_hook_close ((lw_stream) ctx->autosave_file,
							 on_autosave_close, ctx);

	lw_file_open_temp (ctx->autosave_file);

	free (ctx->autosave_filename);
	ctx->autosave_filename = strdup (lw_file_name (ctx->autosave_file));
}

const char * lw_ws_upload_autosave_fname (lw_ws_upload ctx)
{
	if (!ctx->autosave_filename)
	  return "";

	return ctx->autosave_filename;
}

