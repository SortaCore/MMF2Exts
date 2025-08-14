/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../common.h"

// Allows files to include c file all the time and conditionally define ENABLE_SSL
#ifdef ENABLE_SSL

#include "sslclient.h"
#include "../stream.h"

#define lwp_sslclient_flag_handshook	((lw_i8)1)
#define lwp_sslclient_flag_pumping		((lw_i8)2)
#define lwp_sslclient_flag_dead			((lw_i8)4)

struct _lwp_sslclient
{
	struct ssl_st * ssl;
	SSL_CTX * server_context;

	BIO * bio_internal, * bio_external;

	int write_condition;

	lw_i8 flags;

	void * tag;
	lwp_sslclient_on_handshook on_handshook;
	lw_server_client client;
	void (*on_error)(lw_server_client client, lw_error error);

	#ifdef _lacewing_npn
	  unsigned char npn [32];
	#endif

	struct _lw_stream upstream;
	struct _lw_stream downstream;
};

extern lw_streamdef def_upstream;
extern lw_streamdef def_downstream;

static void pumpclient (lwp_sslclient);

lwp_sslclient lwp_sslclient_new (SSL_CTX * server_context, lw_server_client socket,
								 lwp_sslclient_on_handshook on_handshook,
								 void * tag)
{
	lwp_sslclient ctx = (lwp_sslclient)calloc (sizeof (*ctx), 1);

	if (!ctx)
	  return 0;

	ctx->write_condition = -1;

	#ifdef _lacewing_npn
	  *ctx->npn = 0;
	#endif

	ctx->server_context = server_context;
	ctx->ssl = SSL_new (server_context);

	ctx->on_handshook = on_handshook;
	ctx->tag = tag;

	/* TODO : I'm not really happy with the extra layer of buffering
	* that BIO pairs introduce.  Is there a better way to do this?
	*/

	BIO_new_bio_pair (&ctx->bio_internal, 0, &ctx->bio_external, 0);
	SSL_set_bio (ctx->ssl, ctx->bio_internal, ctx->bio_internal);

	SSL_set_accept_state (ctx->ssl);

	lwp_stream_init (&ctx->upstream, &def_upstream, 0);
	lwp_stream_init (&ctx->downstream, &def_downstream, 0);

	/* Retain our streams indefinitely, since we'll be in charge of releasing
	* their memory.  This doesn't stop stream_delete from working.
	*/
	lwp_retain (&ctx->upstream, "lwp_sslclient upstream");
	lwp_retain (&ctx->downstream, "lwp_sslclient downstream");

	lw_stream_add_filter_upstream
	  ((lw_stream)socket, &ctx->upstream, lw_false, lw_false);

	lw_stream_add_filter_downstream
	  ((lw_stream)socket, &ctx->downstream, lw_false, lw_false);

	pumpclient (ctx);

	assert (! (ctx->flags & lwp_sslclient_flag_dead));

	return ctx;
}

void lwp_sslclient_delete (lwp_sslclient ctx)
{
	if (!ctx)
	  return;

	if (ctx->flags & lwp_sslclient_flag_pumping)
	{
	  ctx->flags |= lwp_sslclient_flag_dead;
	  return;
	}

	SSL_free (ctx->ssl);
	BIO_free (ctx->bio_external);

	lw_stream_delete (&ctx->upstream);
	lw_stream_delete (&ctx->downstream);

	free (ctx);
}

lw_bool lwp_sslclient_handshook (lwp_sslclient ctx)
{
	return ctx->flags & lwp_sslclient_flag_handshook;
}

const char * lwp_sslclient_npn (lwp_sslclient ctx)
{
	#ifdef _lacewing_npn
	  return (const char *)ctx->npn;
	#else
	  return "";
	#endif
}

static size_t downstream_sink_data (lw_stream downstream,
									const char * buffer, size_t size)
{
	lwp_sslclient ctx = container_of
	  (downstream, struct _lwp_sslclient, downstream);

	int bytes = BIO_write (ctx->bio_external, buffer, (int)size);

	pumpclient (ctx);

	if (ctx->flags & lwp_sslclient_flag_dead)
	{
	  lwp_sslclient_delete (ctx);
	  return size;
	}

	if (bytes < 0)
	{
		lw_error err = lw_error_new();
		lw_error_addf(err, "SSL downstream write error %d", SSL_get_error(ctx->ssl, bytes));

		if (ctx->on_error)
			ctx->on_error(ctx->client, err);
		lw_error_delete (err);

	  lw_stream_close (&ctx->downstream, lw_true);

	  return size;
	}

	assert (bytes == size);

	/* If OpenSSL was waiting for some more incoming data before we could
	* write something, signal Stream::WriteReady to have Put called again.
	*/

	if (ctx->write_condition == SSL_ERROR_WANT_READ)
	{
	  ctx->write_condition = -1;

	  lw_stream_retry (&ctx->upstream, lw_stream_retry_now);
	}

	return (size_t)bytes;
}

static size_t upstream_sink_data (lw_stream upstream,
								  const char * buffer, size_t size)
{
	lwp_sslclient ctx = container_of
	  (upstream, struct _lwp_sslclient, upstream);

	int bytes = SSL_write (ctx->ssl, buffer, (int)size);
	int error = bytes < 0 ? SSL_get_error (ctx->ssl, bytes) : -1;

	pumpclient (ctx);

	if (ctx->flags & lwp_sslclient_flag_dead)
	{
	  lwp_sslclient_delete (ctx);
	  return size;
	}

	if (error != -1)
	{
	  if (error == SSL_ERROR_WANT_READ)
		 ctx->write_condition = error;
	  else
	  {
		  lw_error err = lw_error_new();
		  lw_error_addf(err, "SSL upstream write error %d", error);

		  if (ctx->on_error)
			  ctx->on_error(ctx->client, err);
		  lw_error_delete(err);
	  }

	  return 0;
	}

	assert (bytes == size);

	return (size_t)bytes;
}

void pumpclient (lwp_sslclient ctx)
{
	if (ctx->flags & lwp_sslclient_flag_pumping)
	  return; /* prevent stack overflow (we loop anyway) */

	ctx->flags |= lwp_sslclient_flag_pumping;

	{  char buffer [lwp_default_buffer_size];
	  int bytes;

	  for (;;)
	  {
		 if (! (ctx->flags & lwp_sslclient_flag_handshook))
		 {
			if (SSL_do_handshake (ctx->ssl) > 0)
			{
				ctx->flags |= lwp_sslclient_flag_handshook;

				#ifdef _lacewing_npn

				  const unsigned char * npn = 0;
				  unsigned int npn_length = 0;

				  SSL_get0_next_proto_negotiated (ctx->ssl, &npn, &npn_length);

				  if (npn)
				  {
					 if (npn_length >= sizeof (ctx->npn))
						npn_length = sizeof (ctx->npn) - 1;

					 memcpy (ctx->npn, npn, npn_length);
					 ctx->npn [npn_length] = 0;
				  }

				#endif

				if (ctx->on_handshook)
				  ctx->on_handshook (ctx, ctx->tag);
			}
		 }

		 lw_bool exit = lw_true;

		 /* First check for any new data destined for the network */

		 bytes = BIO_read (ctx->bio_external, buffer, sizeof (buffer));

		 lwp_trace ("Pump: BIO_read returned %d", bytes);

		 if (bytes > 0)
		 {
			exit = lw_false;

			lw_stream_data (&ctx->upstream, buffer, (size_t)bytes);

			/* Pushing data may end up destroying the SSLClient user, which
			 * will then set the _dead flag.
			 */

			if (ctx->flags & lwp_sslclient_flag_dead)
				return;
		 }
		 else
		 {
			if (!BIO_should_retry (ctx->bio_external))
			{
				lwp_trace ("BIO_should_retry = false");
			}
		 }


		 /* Now check for any data that's been decrypted and is ready to use */

		 bytes = SSL_read (ctx->ssl, buffer, sizeof (buffer));

		 lwp_trace ("Pump: SSL_read returned %d", bytes);

		 if (bytes > 0)
		 {
			exit = lw_false;
			lw_stream_data (&ctx->downstream, buffer, (size_t)bytes);

			if (ctx->flags & lwp_sslclient_flag_dead)
				return;
		 }
		 else
		 {
			if (!bytes)
			{
				lwp_trace ("SSL shutdown!");

				lw_stream_close (&ctx->downstream, lw_true);
			}
			else
			{
				int error = SSL_get_error (ctx->ssl, bytes);

				if (error == SSL_ERROR_WANT_WRITE)
				  continue;

				if (error != SSL_ERROR_WANT_READ)
				{
				  always_log ("SSL error: %s", ERR_error_string ((unsigned long)error, 0));
				  // TODO: SSL failure should kill this client's connection, but not sure how to do that from here
				  // They should eventually be kicked anyway by inactivity timeouts, like relayserver uses
				  // Worth noting error 1 (SSL_ERROR_SSL) will happen if you use a HTTP non-secure connection to a secure port
				}
			}
		 }

		 if (BIO_ctrl_pending (ctx->bio_external) > 0)
			continue;

		 if (exit)
			break;
	  }
	}

	ctx->flags &= ~ lwp_sslclient_flag_pumping;
}

lw_streamdef def_upstream =
{
	.sink_data = upstream_sink_data
};

lw_streamdef def_downstream =
{
	.sink_data = downstream_sink_data
};

#endif // ENABLE_SSL
