/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012, 2013 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "common.h"

lw_ws_req lwp_ws_req_new (lw_ws ws,
						  lwp_ws_client client,
						  const lw_streamdef * def)
{
	lw_ws_req ctx = (lw_ws_req) calloc (sizeof (*ctx), 1);

	if (!ctx)
	  return 0;

	ctx->ws = ws;
	ctx->client = client;

	ctx->responded = lw_true;

	lwp_ws_req_clean (ctx);

	lwp_stream_init ((lw_stream) ctx, def, ws->pump);

	return ctx;
}

void lwp_ws_req_delete (lw_ws_req ctx)
{
	lwp_ws_req_clean (ctx);
	lw_stream_delete ((lw_stream) ctx);
}

void lwp_ws_req_clean (lw_ws_req ctx)
{
	ctx->responded = lw_true;
	ctx->parsed_post_data = lw_false;

	ctx->version_major = 0;
	ctx->version_minor = 0;

	list_each (struct _lw_ws_req_hdr, ctx->headers_in, header)
	{
	  free (header.name);
	  free (header.value);
	}

	list_each (struct _lw_ws_req_hdr, ctx->headers_out, header)
	{
	  free (header.name);
	  free (header.value);
	}

	list_clear (ctx->headers_in);
	list_clear (ctx->headers_out);

	if (ctx->cookies)
	{
	  lw_ws_req_cookie cookie, tmp;

	  HASH_ITER (hh, ctx->cookies, cookie, tmp)
	  {
		 HASH_DEL (ctx->cookies, cookie);

		 free (cookie->name);
		 free (cookie->value);

		 free (cookie);
	  }
	}

	lwp_nvhash_clear (&ctx->get_items);
	lwp_nvhash_clear (&ctx->post_items);

	ctx->cookies	 = 0;
	ctx->get_items	= 0;
	ctx->post_items  = 0;

	*ctx->method	 = 0;
	*ctx->url		= 0;
	*ctx->hostname	= 0;

	lwp_heapbuffer_free (&ctx->buffer);
}

void lwp_ws_req_before_handler (lw_ws_req ctx)
{
	/* Any preparation to be done immediately before calling the handler should
	* be in this function.
	*/

	lwp_heapbuffer_add (&ctx->buffer, "\0", 1); /* null terminate body */

	strcpy (ctx->status, "200 OK");

	list_each (struct _lw_ws_req_hdr, ctx->headers_out, header)
	{
	  free (header.name);
	  free (header.value);
	}

	list_clear (ctx->headers_out);

	lw_ws_req_add_header (ctx, "server", lw_version ());
	lw_ws_req_add_header (ctx, "content-type", "text/html; charset=UTF-8");

	if (ctx->client->secure)
	{
	  /* When the request is secure, add the "cache-control: public" header
		* by default.  DisableCache() called in the handler will remove this,
		* and should be used for any pages containing sensitive data that
		* shouldn't be cached.
		*/

	  lw_ws_req_add_header (ctx, "cache-control", "public");
	}

	assert (ctx->responded);

	ctx->responded = lw_false;
}

void lwp_ws_req_after_handler (lw_ws_req ctx)
{
	/* Anything to be done immediately after the hook has returned should be
	* in this function.
	*/

	if ((!ctx->responded) && ctx->ws->auto_finish)
	  lwp_ws_req_respond (ctx);
}

void lwp_ws_req_call_hook (lw_ws_req ctx)
{
	/* If the protocol doesn't want to call the hook itself (i.e. this is a
	* standard GET/POST/HEAD request), it will call this function to invoke
	* the appropriate handler automatically.
	*/

	lwp_ws_req_before_handler (ctx);

	do
	{
	  if (!strcmp (ctx->method, "GET"))
	  {
		 if (ctx->ws->on_get)
			ctx->ws->on_get (ctx->ws, ctx);

		 break;
	  }

	  if (!strcmp (ctx->method, "POST"))
	  {
		 if (ctx->ws->on_post)
			ctx->ws->on_post (ctx->ws, ctx);

		 break;
	  }

	  if (!strcmp (ctx->method, "HEAD"))
	  {
		 if (ctx->ws->on_head)
			ctx->ws->on_head (ctx->ws, ctx);

		 break;
	  }

	  lw_ws_req_status (ctx, 501, "Not Implemented");
	  lw_ws_req_finish (ctx);

	  return;

	} while(0);

	lwp_ws_req_after_handler (ctx);
}

lw_bool lwp_ws_req_in_version (lw_ws_req ctx, size_t len, const char * version)
{
	if (len != 8)
	  return lw_false;

	if (memcmp (version, "HTTP/", 5) || version [6] != '.')
	  return lw_false;

	ctx->version_major = (char)(version [5] - '0');
	ctx->version_minor = (char)(version [7] - '0');

	if (ctx->version_major != 1)
	  return lw_false;

	if (ctx->version_minor != 0 && ctx->version_minor != 1)
	  return lw_false;

	return lw_true;
}

lw_bool lwp_ws_req_in_method (lw_ws_req ctx, size_t len, const char * method)
{
	if (len > (sizeof (ctx->method) - 1))
	  return lw_false;

	memcpy (ctx->method, method, len);
	ctx->method [len] = 0;

	return lw_true;
}

static lw_bool parse_cookie_header (lw_ws_req ctx, size_t header_len,
									const char * header)
{
	size_t name_begin = 0, name_len = 0, value_begin = 0, value_len = 0;

	int state = 0;

	for (size_t i = 0 ;; ++ i)
	{
	  switch (state)
	  {
		 case 0: /* seeking name */

			if (i >= header_len)
				return lw_true;

			if (header [i] == ' ' || header [i] == '\t')
				break;

			name_begin = i;
			name_len = 1;

			++ state;

			break;

		 case 1: /* seeking name/value separator */

			if (i >= header_len)
				return lw_false;

			if (header [i] == '=')
			{
				value_begin = i + 1;
				value_len = 0;

				++ state;
				break;
			}

			++ name_len;
			break;

		 case 2: /* seeking end of value */

			if (i >= header_len || header [i] == ';')
			{
				lwp_ws_req_set_cookie
				  (ctx, name_len, header + name_begin,
					value_len, header + value_begin, 0, "", lw_false);

				if (i >= header_len)
				  return lw_true;

				state = 0;
				break;
			}

			++ value_len;
	  };
	}
}

lw_bool lwp_ws_req_in_header (lw_ws_req ctx, size_t name_len, const char * name,
							  size_t value_len, const char * value)
{
	struct _lw_ws_req_hdr header;

	/* TODO : limit name_len/value_len */

	if (! (header.name = (char *) malloc (name_len + 1)))
		return lw_false;

	if (! (header.value = (char *) malloc (value_len + 1)))
		return lw_false;

	for (size_t i = 0; i < name_len; ++ i)
		header.name [i] = (char) tolower (name [i]);

	#ifdef _MSC_VER
		#pragma warning (suppress: 6386) // Not an overrun
	#endif
	header.name [name_len] = 0;

	name = header.name;

	memcpy (header.value, value, value_len);
	header.value [value_len] = 0;

	list_push (struct _lw_ws_req_hdr, ctx->headers_in, header);

	if (!strcmp (name, "cookie"))
		return parse_cookie_header (ctx, value_len, value);

	if (!strcmp (name, "host"))
	{
		/* The hostname gets stored separately with the port removed for
		* the hostname function.
		*/

		if (value_len > (sizeof (ctx->hostname) - 1))
			return lw_false; /* hostname too long */

		size_t host_len = value_len;

		for (size_t i = 0; i < value_len; ++ i)
		{
			if(i > 0 && value [i] == ':')
			{
				host_len = i;
				break;
			}
		}

		memcpy (ctx->hostname, value, host_len);
		ctx->hostname [host_len] = 0;

		return lw_true;
	}

	return lw_true;
}

static lw_bool get_field (const char * URL, struct http_parser_url * parsed,
						  int field, const char ** buf, size_t * length)
{
	if (! (parsed->field_set & (1 << field)))
		return lw_false;

	*buf = URL + parsed->field_data [field].off;
	*length = parsed->field_data [field].len;

	return lw_true;
}

lw_bool lwp_ws_req_in_url (lw_ws_req ctx, size_t length, const char * url)
{
	struct http_parser_url parsed;

	/* Check for any directory traversal in the URL. */

	for (size_t i = 0; i < (length - 1); ++ i)
	{
		if (url [i] == '.' && url [i + 1] == '.')
			return lw_false;
	}

	/* Now hand it over to the URL parser */

	if (http_parser_parse_url (url, length, 0, &parsed))
		return lw_false;


	/* Path */

	*ctx->url = 0;

	{
		const char * path;
		size_t path_length;

		if (get_field (url, &parsed, UF_PATH, &path, &path_length))
		{
			if (*path == '/')
			{
				++ path;
				-- path_length;
			}

			if (!lwp_urldecode (path, path_length,
								ctx->url, sizeof (ctx->url),
								lw_false))
			{
				return lw_false;
			}
		}
	}


	/* Host */

	{
		const char * host;
		size_t host_length;

		if (get_field (url, &parsed, UF_HOST, &host, &host_length))
		{
			if (parsed.field_set & (1 << UF_PORT))
			{
				/* TODO : Is this robust? */

				host_length += 1u + parsed.field_data [UF_PORT].off;
			}

			lwp_ws_req_in_header (ctx, 4, "host", host_length, host);
		}
	}


	/* GET data */

	{
		const char * data;
		size_t length;

		if (get_field (url, &parsed, UF_QUERY, &data, &length))
		{
			for (;;)
			{
				const char * name = data;

				if (!lwp_find_char (&data, &length, '='))
					break;

				size_t name_length = (size_t)(data - name);

				/* skip the = character */

				++ data;
				-- length;

				const char * value = data;
				size_t value_length = length;

				if (lwp_find_char (&data, &length, '&'))
				{
					value_length = (size_t)(data - value);

					/* skip the & character */

					++ data;
					-- length;
				}

				char * name_decoded = (char *) lw_malloc_or_exit (name_length + 1),
					 * value_decoded = (char *) lw_malloc_or_exit (value_length + 1);

				if (!lwp_urldecode (name, name_length, name_decoded, name_length + 1, lw_true)
					|| !lwp_urldecode (value, value_length, value_decoded, value_length + 1, lw_true))
				{
					free (name_decoded);
					free (value_decoded);
				}
				else
				{
					lwp_nvhash_set (&ctx->get_items, name_decoded, value_decoded, lw_false);
				}
			}
		}
	}

	return lw_true;
}

void lwp_ws_req_respond (lw_ws_req ctx)
{
	assert (!ctx->responded);

	/* James note: Respond may delete us w/ SPDY blah blah
	   Phi note: SPDY was deprecated in favour of HTTP/2 in 2021, and so removed from liblacewing */

	ctx->client->respond (ctx->client, ctx);
}

lw_addr lw_ws_req_addr (lw_ws_req ctx)
{
	return lw_server_client_addr (ctx->client->socket);
}

void lw_ws_req_disconnect (lw_ws_req ctx, unsigned int websocket_exit_reason)
{
	if (!ctx->client->websocket)
		lw_stream_close ((lw_stream) ctx->client->socket, lw_true);
	else
	{
		// Worth noting these close code checks mean that subsequent local disconnects do nothing
		// There's a timeout activated, so there's no need to rush, regardless.

		// Close message not sent already
		if (ctx->client->local_close_code == -1)
		{
			// if no error code, report normal closure reason
			if (websocket_exit_reason == 0)
				websocket_exit_reason = 1000;

			ctx->client->local_close_code = (lw_i16)websocket_exit_reason;

			lw_ui8 opcode = 0b10001000; // fin, connection close
			lw_ui16 exit_reason = htons((lw_ui16)websocket_exit_reason);
			char maskAndLen = (char)sizeof(exit_reason);
			char close_msg[] = { *(char*)&opcode, maskAndLen, ((char*)&exit_reason)[0], ((char*)&exit_reason)[1] };
			lwp_stream_write((lw_stream)ctx->client->socket, close_msg, sizeof(close_msg), lwp_stream_write_ignore_busy);

			ctx->client->timeout = 5; // WebSocket client has 5 seconds to acknowledge our sent close packet
		}

		// We already got a close packet, and since we just sent one, it's safe to exit now
		// lw_stream_close(..., lw_false) results in "non-clean" close for browser
		if (ctx->client->remote_close_code != -1)
			lw_stream_close((lw_stream)ctx->client->socket, lw_true);
	}
}

void lw_ws_req_guess_mimetype (lw_ws_req ctx, const char * filename)
{
	lw_ws_req_set_mimetype (ctx, lw_guess_mimetype (filename));
}

void lw_ws_req_set_mimetype_ex (lw_ws_req ctx, const char * mimetype, const char * charset)
{
	if (!*charset)
	{
		lw_ws_req_set_header (ctx, "content-type", mimetype);
		return;
	}

	char type [256];

	lwp_snprintf (type, sizeof (type), "%s; charset=%s", mimetype, charset);
	lw_ws_req_set_header (ctx, "content-type", type);
}

void lw_ws_req_set_mimetype (lw_ws_req ctx, const char * mimetype)
{
	lw_ws_req_set_mimetype_ex (ctx, mimetype, "");
}

void lw_ws_req_set_redirect (lw_ws_req ctx, const char * url)
{
	lw_ws_req_status  (ctx, 303, "See Other");
	lw_ws_req_set_header (ctx, "location", url);
}

void lw_ws_req_disable_cache (lw_ws_req ctx)
{
	lw_ws_req_set_header (ctx, "cache-control", "no-cache");
}

void lw_ws_req_set_header (lw_ws_req ctx, const char * name, const char * value)
{
	list_each_elem (struct _lw_ws_req_hdr, ctx->headers_out, header)
	{
		if (!strcasecmp (header->name, name))
		{
			free (header->value);
			header->value = strdup (value);

			return;
		}
	}

	lw_ws_req_add_header (ctx, name, value);
}

void lw_ws_req_add_header (lw_ws_req ctx, const char * name, const char * value)
{
	size_t name_len = strlen (name);

	struct _lw_ws_req_hdr header;

	header.name = (char *) malloc (name_len + 1);
	header.name [name_len] = 0;

	for (size_t i = 0; i < name_len; ++ i)
	{
		#ifdef _MSC_VER
			#pragma warning (suppress: 6386) // Not an overrun
		#endif
		header.name [i] = (char)tolower (name [i]);
	}

	header.value = strdup (value);

	list_push (struct _lw_ws_req_hdr, ctx->headers_out, header);
}

void lw_ws_req_set_cookie (lw_ws_req ctx, const char * name, const char * value)
{
	lw_ws_req_set_cookie_attr (ctx, name, value,
		 lw_ws_req_secure (ctx) ? "Secure; HttpOnly" : "HttpOnly");
}

void lw_ws_req_set_cookie_attr (lw_ws_req ctx, const char * name,
								const char * value, const char * attr)
{
	lwp_ws_req_set_cookie (ctx,
							strlen (name), name,
							strlen (value), value,
							strlen (attr), attr,
							lw_true);
}

void lwp_ws_req_set_cookie (lw_ws_req ctx,
		 size_t name_len,	const char * name,
		 size_t value_len,  const char * value,
		 size_t attr_len,	const char * attr,
		 lw_bool changed)
{
	lw_ws_req_cookie cookie;

	HASH_FIND (hh, ctx->cookies, name, name_len, cookie);

	if (!cookie)
	{
		cookie = (lw_ws_req_cookie) lw_calloc_or_exit (sizeof (*cookie), 1);

		cookie->changed = changed;

		cookie->name = (char *) lw_malloc_or_exit (name_len + 1);
		memcpy (cookie->name, name, name_len);
		cookie->name [name_len] = 0;

		cookie->value = (char *) lw_malloc_or_exit (value_len + 1);
		memcpy (cookie->value, value, value_len);
		cookie->value [value_len] = 0;

		cookie->attr = (char *) lw_malloc_or_exit (attr_len + 1);
		memcpy (cookie->attr, attr, attr_len);
		cookie->attr [attr_len] = 0;

		HASH_ADD_KEYPTR (hh, ctx->cookies, cookie->name, name_len, cookie);

		return;
	}

	cookie->changed = changed;

	cookie->value = (char *) lw_realloc_or_exit (cookie->value, value_len + 1);
	memcpy (cookie->value, value, value_len);
	cookie->value [value_len] = 0;

	cookie->attr = (char *) lw_realloc_or_exit (cookie->attr, attr_len + 1);
	memcpy (cookie->attr, attr, attr_len);
	cookie->attr [attr_len] = 0;
}

void lw_ws_req_status (lw_ws_req ctx, long code, const char * message)
{
	/* TODO : prevent buffer overrun */

	sprintf (ctx->status, "%d %s", (int) code, message);
}

void lw_ws_req_set_unmodified (lw_ws_req ctx)
{
	lw_ws_req_status (ctx, 304, "Not Modified");
}

void lw_ws_req_set_last_modified (lw_ws_req ctx, lw_i64 _time)
{
	struct tm tm;
	time_t time;

	time = (time_t) _time;

	#ifdef _WIN32
		memcpy (&tm, gmtime (&time), sizeof (struct tm));
	#else
		gmtime_r (&time, &tm);
	#endif

	char modified [128];

	sprintf (modified, "%s, %02d %s %d %02d:%02d:%02d GMT",
				lwp_weekdays [tm.tm_wday],
				tm.tm_mday,
				lwp_months [tm.tm_mon],
				tm.tm_year + 1900,
				tm.tm_hour,
				tm.tm_min,
				tm.tm_sec);

	lw_ws_req_set_header (ctx, "last-modified", modified);
}

/* TODO: merge with respond */

void lw_ws_req_finish (lw_ws_req ctx)
{
	lwp_ws_req_respond (ctx);
}

const char * lw_ws_req_header (lw_ws_req ctx, const char * name)
{
	list_each (struct _lw_ws_req_hdr, ctx->headers_in, header)
	{
		if (!strcasecmp (header.name, name))
			return header.value;
	}

	return "";
}

lw_ws_req_hdr lw_ws_req_hdr_first (lw_ws_req ctx)
{
	return list_elem_front (struct _lw_ws_req_hdr, ctx->headers_in);
}

const char * lw_ws_req_hdr_name (lw_ws_req_hdr header)
{
	return header->name;
}

const char * lw_ws_req_hdr_value (lw_ws_req_hdr header)
{
	return header->value;
}

lw_ws_req_hdr lw_ws_req_hdr_next (lw_ws_req_hdr header)
{
	return list_elem_next (struct _lw_ws_req_hdr, header);
}

const char * lw_ws_req_get_cookie (lw_ws_req ctx, const char * name)
{
	lw_ws_req_cookie cookie;

	HASH_FIND_STR (ctx->cookies, name, cookie);

	return cookie ? cookie->value : "";
}

lw_ws_req_cookie lw_ws_req_cookie_first (lw_ws_req ctx)
{
	return ctx->cookies;
}

lw_ws_req_cookie lw_ws_req_cookie_next (lw_ws_req_cookie cookie)
{
	return (lw_ws_req_cookie) cookie->hh.next;
}

const char * lw_ws_req_cookie_name (lw_ws_req_cookie cookie)
{
	return cookie->name;
}

const char * lw_ws_req_cookie_value (lw_ws_req_cookie cookie)
{
	return cookie->value;
}

const char * lw_ws_req_body (lw_ws_req ctx)
{
	return lwp_heapbuffer_buffer (&ctx->buffer);
}

const char * lw_ws_req_GET (lw_ws_req ctx, const char * name)
{
	return lwp_nvhash_get (&ctx->get_items, name, "");
}

static void parse_post_data (lw_ws_req ctx)
{
	if (ctx->parsed_post_data)
		return;

	ctx->parsed_post_data = lw_true;

	if (!lwp_begins_with (lw_ws_req_header (ctx, "content-type"),
			"application/x-www-form-urlencoded"))
	{
		return;
	}

	char * post_data = lwp_heapbuffer_buffer (&ctx->buffer),
		* end = post_data + lwp_heapbuffer_length (&ctx->buffer),
		b = *end;

	*end = 0;

	for (;;)
	{
		char * name = post_data, * value = strchr (post_data, '=');

		size_t name_length = (size_t)(value - name);

		if (!value ++)
			break;

		char * next = strchr (value, '&');

		size_t value_length = next ? (size_t)(next - value) : strlen (value);

		char * name_decoded = (char *) lw_malloc_or_exit (name_length + 1),
			* value_decoded = (char *) lw_malloc_or_exit (value_length + 1);

		if(!lwp_urldecode (name, name_length, name_decoded, name_length, lw_true)
			|| !lwp_urldecode (value, value_length, value_decoded, value_length, lw_true))
		{
			free (name_decoded);
			free (value_decoded);
		}
		else
		{
			lwp_nvhash_set (&ctx->post_items, name_decoded, value_decoded, lw_false);
		}

		if (!next)
			break;

		post_data = next + 1;
	}

	*end = b;
}

const char * lw_ws_req_POST (lw_ws_req ctx, const char * name)
{
	parse_post_data (ctx);

	return lwp_nvhash_get (&ctx->post_items, name, "");
}

lw_ws_req_param lw_ws_req_GET_first (lw_ws_req ctx)
{
	return (lw_ws_req_param) ctx->get_items;
}

lw_ws_req_param lw_ws_req_POST_first (lw_ws_req ctx)
{
	parse_post_data (ctx);

	return (lw_ws_req_param) ctx->post_items;
}

lw_ws_req_param lw_ws_req_param_next (lw_ws_req_param param)
{
	return (lw_ws_req_param) ((lwp_nvhash) param)->hh.next;
}

const char * lw_ws_req_param_name (lw_ws_req_param param)
{
	return ((lwp_nvhash) param)->key;
}

const char * lw_ws_req_param_value (lw_ws_req_param param)
{
	return ((lwp_nvhash) param)->value;
}

lw_i64 lw_ws_req_last_modified (lw_ws_req ctx)
{
	const char * modified = lw_ws_req_header (ctx, "if-modified-since");

	if (*modified)
		return lwp_parse_time (modified);

	return 0;
}

lw_bool lw_ws_req_secure (lw_ws_req ctx)
{
	return ctx->client->secure;
}

lw_bool lw_ws_req_websocket (lw_ws_req ctx)
{
	return ctx->client->websocket;
}

const char * lw_ws_req_hostname (lw_ws_req ctx)
{
	return ctx->hostname;
}

const char * lw_ws_req_url (lw_ws_req ctx)
{
	return ctx->url;
}

long lw_ws_req_idle_timeout (lw_ws_req ctx)
{
	return ctx->client->timeout;
}

void lw_ws_req_set_idle_timeout (lw_ws_req ctx, long seconds)
{
	ctx->client->timeout = seconds;
}

