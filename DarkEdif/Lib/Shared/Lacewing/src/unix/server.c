/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012, 2013 James McLaughlin.
 * Copyright (C) 2012-2026 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../common.h"
#include <sys/ioctl.h>

#ifdef ENABLE_SSL
	#include "../openssl/sslclient.h"
#endif

#include "../address.h"

#include "fdstream.h"

static void on_client_close (lw_stream, void * tag);

static void on_client_data (lw_stream, void * tag, const char * buffer,
							size_t size);

#ifdef ENABLE_SSL
	static void on_ssl_handshook (lwp_sslclient ssl, void * tag);
#endif

struct _lw_server
{
	int socket;

	lw_pump pump;
	lw_pump_watch pump_watch;

	lw_server_hook_connect on_connect;
	lw_server_hook_disconnect on_disconnect;
	lw_server_hook_data on_data;
	lw_server_hook_error on_error;

	void * tag;

	#ifdef ENABLE_SSL
		SSL_CTX * ssl_context;
		char ssl_passphrase [128];
		time_t cert_expiry_time; // expiry time in UTC

		#ifdef _lacewing_npn
			unsigned char npn [128];
		#endif
	#endif

	lw_list (lw_server_client, clients);
};

struct _lw_server_client
{
	struct _lw_fdstream fdstream;

	lw_server server;

	lw_bool on_connect_called;
	lw_bool is_websocket;
	lw_bool is_hole_punch;

	void* relay_tag;

	#ifdef ENABLE_SSL
		lwp_sslclient ssl;
	#endif

	lw_addr remote_address;
	lw_addr local_address;
	lw_ui32 ifidx;

	lw_server_client * elem;
};

void on_ssl_error(lw_server_client client, lw_error error)
{
	lw_error_addf(error, "SSL error");

	if (client->server->on_error)
		client->server->on_error(client->server, error);

	// SSL errors are generally unrecoverable
	lw_stream_close((lw_stream)client, lw_true);
}

static lw_server_client lwp_server_client_new (lw_server ctx, lw_pump pump, int fd)
{
	lw_server_client client = (lw_server_client)calloc (sizeof (*client), 1);

	if (!client)
		return 0;

	client->server = ctx;

	lwp_fdstream_init (&client->fdstream, pump);

	/* We keep this reference right up until the client disconnects from
	* the server
	*/
	lwp_retain (client, "server_client_new");

	/* The first added close hook is always the last called.
	* This is important, because ours will destroy the client.
	*/
	lw_stream_add_hook_close ((lw_stream) client, on_client_close, client);

	#ifdef ENABLE_SSL

		if (ctx->ssl_context)
		{
			client->ssl = lwp_sslclient_new (ctx->ssl_context, client,
											on_ssl_handshook, client);
		}

	#endif

	lw_fdstream_set_fd (&client->fdstream, fd, 0, lw_true, lw_true);

	return client;
}

#ifdef ENABLE_SSL

 void on_ssl_handshook (lwp_sslclient ssl, void * tag)
 {
	lw_server_client client = (lw_server_client)tag;
	lw_server server = client->server;

	#ifdef _lacewing_npn
		lwp_trace ("on_ssl_handshook for %p, NPN is %s",
			 client, lwp_sslclient_npn (ssl));
	#endif

	client->on_connect_called = lw_true;

	lwp_retain (client, "on_ssl_handshook");

	if (server->on_connect)
		server->on_connect (server, client);

	if (lwp_release (client, "on_ssl_handshook") ||
			((lw_stream) client)->flags & lwp_stream_flag_dead)
	{
		/* Client was deleted by connect hook
		*/
		return;
	}

	list_push (lw_server_client, server->clients, client);
	client->elem = list_elem_back (lw_server_client, server->clients);
 }

#endif

lw_server lw_server_new (lw_pump pump)
{
	lwp_init ();

	lw_server ctx = (lw_server)calloc (sizeof (*ctx), 1);

	if (!ctx)
		return 0;

	ctx->pump = pump;

	#ifdef _lacewing_npn
		lwp_trace ("NPN is available");
	#else
		lwp_trace ("NPN is NOT available");
	#endif

	ctx->socket = -1;

	return ctx;
}

void lw_server_delete (lw_server ctx)
{
	if (!ctx)
	  return;

	lw_server_unhost (ctx);

#ifdef ENABLE_SSL
	if (ctx->ssl_context)
		SSL_CTX_free(ctx->ssl_context);
#endif

	free (ctx);
}

void lw_server_set_tag (lw_server ctx, void * tag)
{
	ctx->tag = tag;
}

void * lw_server_tag (lw_server ctx)
{
	return ctx->tag;
}

static lw_bool add_client_internal(lw_server ctx,
	struct sockaddr_storage * remote_addr, lwp_socket fd, lw_bool accepted)
{
	lwp_trace("%s FD %d", accepted ? "hole punch adding" : "accepted", fd);

	lw_server_client client = lwp_server_client_new(ctx, ctx->pump, fd);

	if (!client)
	{
		lwp_trace("Failed allocating client");
		return lw_false;
	}

	struct sockaddr_storage local_addr = lwp_socket_addr(fd);
	client->remote_address = lwp_addr_new_sockaddr((struct sockaddr*)remote_addr);
	client->local_address = lwp_addr_new_sockaddr((struct sockaddr*)&local_addr);
	client->ifidx = lwp_get_ifidx(&local_addr);

	lw_bool should_read = lw_false;

	if (ctx->on_data)
	{
		lw_stream_add_hook_data((lw_stream)client, on_client_data, client);
		should_read = lw_true;
	}

#ifdef ENABLE_SSL
	if (!client->ssl)
	{
#endif

		client->on_connect_called = lw_true;

		lwp_retain(client, "on_connect");

		if (ctx->on_connect)
			ctx->on_connect(ctx, client);

		if (lwp_release(client, "on_connect") ||
			((lw_stream)client)->flags & lwp_stream_flag_dead)
		{
			if (ctx->on_disconnect)
				ctx->on_disconnect(ctx, client);
			/* Client was deleted by connect hook
			 */
			return lw_false;
		}

		list_push(lw_server_client, ctx->clients, client);
		client->elem = list_elem_back(lw_server_client, ctx->clients);

#ifdef ENABLE_SSL
	}
	else
	{
		should_read = lw_true;
	}
#endif

	if (should_read)
	{
		lwp_retain(client, "client initial read");

		lw_stream_read((lw_stream)client, SIZE_MAX);

		if (lwp_release(client, "client initial read") ||
			((lw_stream)client)->flags & lwp_stream_flag_dead)
		{
			/* Client was deleted when performing initial read
			 */
			return lw_false;
		}
	}
	return lw_true;
}

static void listen_socket_read_ready (void * tag)
{
	lw_server ctx = (lw_server)tag;

	struct sockaddr_storage address;
	socklen_t address_length = sizeof (address);

	for (;;)
	{
	  int fd;

	  lwp_trace ("Trying to accept...");

	  if ((fd = accept (ctx->socket, (struct sockaddr *) &address,
						&address_length)) == -1)
	  {
		 lwp_trace ("Failed to accept: %s", strerror (errno));
		 break;
	  }

	  if (add_client_internal(ctx, &address, fd, lw_true))
		  return;
	}
}

typedef struct _lw_hole_punch_params {
	lw_server server;
	lwp_socket sock;
	struct sockaddr_storage addr;
	socklen_t addrlen;
	lw_ui16 local_port;
	lw_pump_watch watch;
} lw_hole_punch_params;

static lw_callback void hole_punch_socket_first_data(lw_hole_punch_params * params)
{
	lw_pump_remove(params->server->pump, params->watch);
	add_client_internal(params->server, &params->addr, params->sock, lw_false);
	lwp_release(params->server, "hole punch");
}

void lw_server_hole_punch(lw_server ctx, lw_addr addr, lw_ui16 local_port)
{
	lwp_retain(ctx, "hole punch");
	lw_hole_punch_params* params = (lw_hole_punch_params*)lw_malloc_or_exit(sizeof(lw_hole_punch_params));
	params->server = ctx;
	params->addr = *(struct sockaddr_storage *)addr->info->ai_addr;
	params->addrlen = addr->info->ai_addrlen;
	params->local_port = local_port;
	params->sock = -1;
	params->watch = NULL;

	params->sock = socket(params->addr.ss_family, SOCK_STREAM, IPPROTO_TCP);
	char* errStr = NULL;
	do {
		if (params->sock == -1)
		{
			errStr = "Creating TCP hole punch failed";
			break;
		}

		// reuse addr on
		//lwp_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));
		struct sockaddr_storage s = { 0 };
		s.ss_family = params->addr.ss_family;
		// Port is at same offset in both sockaddr_in and sockaddr_in6
		((struct sockaddr_in6*)&s)->sin6_port = htons((lw_ui16)params->local_port);

		if (bind(params->sock, (struct sockaddr*)&s, params->addrlen) == -1)
		{
			errStr = "bind on TCP hole punch failed";
			break;
		}
		lwp_make_nonblocking(params->sock);
		ssize_t conRet = connect(params->sock, (struct sockaddr *)&params->addr, params->addrlen);
		if (conRet == -1 && errno != EWOULDBLOCK)
		{
			errStr = "connect on TCP hole punch failed";
			break;
		}

		params->watch = lw_pump_add(params->server->pump, params->sock, params, (lw_pump_callback)hole_punch_socket_first_data, NULL, lw_true);
	} while (0);

	if (errStr)
	{
		lw_error err = lw_error_new();
		lw_error_add(err, errno);
		lw_error_addf(err, "%s", errStr);
		if (params->sock != -1)
			lwp_close_socket(params->sock);
		if (params->server->on_error)
			params->server->on_error(params->server, err);
		lw_error_delete(err);
		free(params);
	}
}

void lw_server_host (lw_server ctx, long port)
{
	lw_filter filter = lw_filter_new ();
	lw_filter_set_local_port (filter, port);

	lw_server_host_filter (ctx, filter);
}

void lw_server_host_filter (lw_server ctx, lw_filter filter)
{
	lw_server_unhost (ctx);

	lw_error error = lw_error_new ();

	if ((ctx->socket = lwp_create_server_socket
			(filter, SOCK_STREAM, IPPROTO_TCP, NULL, error)) == -1)
	{
	  lwp_trace ("server: error hosting: %s", lw_error_tostring (error));

	  if (ctx->on_error)
		 ctx->on_error (ctx, error);

	  lw_error_delete (error);
	  return;
	}

	if (listen (ctx->socket, SOMAXCONN) == -1)
	{
	  lw_error error = lw_error_new ();

	  lw_error_add (error, errno);
	  lw_error_addf (error, "Error listening");

	  if (ctx->on_error)
		 ctx->on_error (ctx, error);

	  lw_error_delete (error);
	  return;
	}

	lwp_make_nonblocking(ctx->socket);

	ctx->pump_watch = lw_pump_add (ctx->pump, ctx->socket, ctx, listen_socket_read_ready, 0, lw_true);

	lw_error_delete (error);
}

void lw_server_unhost (lw_server ctx)
{
	if (!lw_server_hosting (ctx))
	  return;

	close (ctx->socket);
	ctx->socket = -1;

	lw_pump_remove(ctx->pump, ctx->pump_watch);
	ctx->pump_watch = NULL;

	// Not having this leaves lw_server_client connections open but server is closed
	list_each(lw_server_client, ctx->clients, e) {
		lw_stream_close(&e->fdstream.stream, lw_true);
	}
}

lw_bool lw_server_hosting (lw_server ctx)
{
	return ctx->socket != -1;
}

size_t lw_server_num_clients (lw_server ctx)
{
	return list_length (ctx->clients);
}

int lw_server_port (lw_server ctx)
{
	return lwp_socket_port (ctx->socket);
}

lw_bool lw_server_cert_loaded (lw_server ctx)
{
	#ifdef ENABLE_SSL
	  return ctx->ssl_context != 0;
	#else
	  return lw_false;
	#endif
}

time_t lw_server_cert_expiry_time (lw_server ctx)
{
	#ifdef ENABLE_SSL
	  return ctx->cert_expiry_time;
	#else
	  return 0;
	#endif
}

#ifdef ENABLE_SSL
static int ssl_password_callback (char * buffer, int size, int rwflag, void * tag)
{
	lw_server ctx = (lw_server)tag;

	int passSize = (int)strlen(ctx->ssl_passphrase);
	if (size < passSize)
	{
		lw_error error = lw_error_new ();
		lw_error_addf (error, "Password is %i bytes, too big for buffer of %i bytes", passSize, size);
		if (ctx->on_error)
			ctx->on_error (ctx, error);
		lw_error_delete (error);
		return -1;
	}

	strcpy (buffer, ctx->ssl_passphrase);
	return passSize;
}
#endif

#ifdef _lacewing_npn

	static int npn_advertise (SSL * ssl, const unsigned char ** data,
							 unsigned int * len, void * tag)
	{
	  lw_server ctx = (lw_server)tag;

	  *len = 0;

	  for (unsigned char * i = ctx->npn; *i; )
	  {
		 *len += 1u + *i;
		 i += 1 + *i;
	  }

	  *data = ctx->npn;

	  lwp_trace ("Advertising for NPN...");

	  return SSL_TLSEXT_ERR_OK;
	}

#endif

lw_bool lw_server_load_cert_file (lw_server ctx, const char * filename_cert_chain, const char* filename_privkey,
								  const char * passphrase)
{
	#ifndef ENABLE_SSL
	  return lw_false;
	#else

	SSL_load_error_strings ();

	ctx->ssl_context = SSL_CTX_new (SSLv23_server_method ());
	assert (ctx->ssl_context);

	strcpy (ctx->ssl_passphrase, passphrase);

	SSL_CTX_set_mode (ctx->ssl_context,

		SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER

		#ifdef SSL_MODE_RELEASE_BUFFERS
			 | SSL_MODE_RELEASE_BUFFERS
		#endif
	);

	#ifdef _lacewing_npn
		SSL_CTX_set_next_protos_advertised_cb
			(ctx->ssl_context, npn_advertise, ctx);
	#endif

	SSL_CTX_set_quiet_shutdown (ctx->ssl_context, 1);

	SSL_CTX_set_default_passwd_cb (ctx->ssl_context, ssl_password_callback);
	SSL_CTX_set_default_passwd_cb_userdata (ctx->ssl_context, ctx);

	if (SSL_CTX_use_certificate_chain_file (ctx->ssl_context, filename_cert_chain) != 1)
	{
		lw_error error = lw_error_new();
		lw_error_addf(error, "Failed to load certificate chain file: %s", ERR_error_string(ERR_get_error(), 0));
		if (ctx->on_error)
			ctx->on_error(ctx, error);
		lw_error_delete(error);

		SSL_CTX_free (ctx->ssl_context);
		ctx->ssl_context = 0;
		return lw_false;
	}

	if (SSL_CTX_use_PrivateKey_file (ctx->ssl_context, filename_privkey,
									 SSL_FILETYPE_PEM) != 1)
	{
		lw_error error = lw_error_new();
		lw_error_addf(error, "Failed to load private key file: %s", ERR_error_string(ERR_get_error(), 0));
		if (ctx->on_error)
			ctx->on_error(ctx, error);
		lw_error_delete(error);

		SSL_CTX_free (ctx->ssl_context);
		ctx->ssl_context = 0;
		return lw_false;
	}

	X509* x509 = SSL_CTX_get0_certificate(ctx->ssl_context);
	const ASN1_TIME* notAfter = X509_getm_notAfter(x509);
	struct tm tm;
	if (ASN1_TIME_to_tm(notAfter, &tm))
	{
		int day, sec;
		// time must be valid
		if (!ASN1_TIME_diff(&day, &sec, NULL, notAfter)) {
			assert(lw_false);
			abort();
		}

		char buff[50];
		// Unix, iOS: int return; Android: size_t return
		if (strftime(buff, sizeof(buff), "%I:%M:%S%p on %A %d %B %Y AD", &tm) <= 0)
			always_log("time conversion failed, error %d", errno);
		else
			always_log("TLS certificate will expire at %s (local time).", buff);

		if (day > 0 || sec > 0)
			ctx->cert_expiry_time = timegm(&tm);
		else
		{
			SSL_CTX_free(ctx->ssl_context);
			ctx->ssl_context = 0;

			lw_error error = lw_error_new ();
			lw_error_addf(error, "TLS certificate has already expired, at %s (local time)", buff);
			if (ctx->on_error)
				ctx->on_error(ctx, error);
			lw_error_delete(error);
			return lw_false;
		}
	}
	else
	{
		lw_error error = lw_error_new();
		lw_error_addf(error, "Failed to read certificate expiration time.");
		if (ctx->on_error)
			ctx->on_error(ctx, error);
		lw_error_delete(error);

		SSL_CTX_free(ctx->ssl_context);
		ctx->ssl_context = 0;
		return lw_false;
	}

	return lw_true;

	#endif
}

lw_bool lw_server_load_sys_cert (lw_server ctx,
								 const char * common_name,
								 const char * location,
								 const char * store_name)
{
	lw_error error = lw_error_new ();
	lw_error_addf (error, "System certificates are only supported on Windows");

	if (ctx->on_error)
	  ctx->on_error (ctx, error);

	return lw_false;
}

lw_bool lw_server_can_npn (lw_server ctx)
{
	#ifdef _lacewing_npn
	  return lw_true;
	#endif

	return lw_false;
}

void lw_server_add_npn (lw_server ctx, const char * protocol)
{
	#ifdef _lacewing_npn

	  size_t length = strlen (protocol);

	  if (length > 0xFF)
	  {
		 lwp_trace ("NPN protocol too long: %s", protocol);
		 return;
	  }

	  unsigned char * end = ctx->npn;

	  while (*end)
		 end += 1 + *end;

	  if ((end + length + 2) > (ctx->npn + sizeof (ctx->npn)))
	  {
		 lwp_trace ("NPN list would have overflowed adding %s", protocol);
		 return;
	  }

	  *end ++ = ((unsigned char) length);
	  memcpy (end, protocol, length + 1);

	#endif
}

const char * lw_server_client_npn (lw_server_client client)
{
	#ifndef _lacewing_npn
	  return "";
	#else

	  if (client->ssl)
		 return lwp_sslclient_npn (client->ssl);

	  return "";

	#endif
}

lw_addr lw_server_client_remote_addr (lw_server_client client)
{
	return client->remote_address;
}
lw_addr lw_server_client_local_addr (lw_server_client client)
{
	return client->local_address;
}
lw_ui32 lw_server_client_ifidx (lw_server_client client)
{
	return client->ifidx;
}

lw_bool lw_server_client_is_websocket(lw_server_client client)
{
	return client->is_websocket;
}

lw_bool lw_server_client_is_hole_punch(lw_server_client client)
{
	return client->is_hole_punch;
}

void* lw_server_client_get_relay_tag(lw_server_client client)
{
	return client->relay_tag;
}

void lw_server_client_set_relay_tag(lw_server_client client, void* ptr)
{
	client->relay_tag = ptr;
}

void lw_server_client_set_websocket(lw_server_client client, lw_bool isWebSocket)
{
	client->is_websocket = isWebSocket;
}

lw_server_client lw_server_client_next (lw_server_client client)
{
	lw_server_client * next_client = list_elem_next (lw_server_client, client->elem);

	if (!next_client)
	  return NULL;

	return *next_client;
}

lw_server_client lw_server_client_first (lw_server ctx)
{
	if (list_length (ctx->clients) == 0)
	  return NULL;

	return list_front (lw_server_client, ctx->clients);
}

void on_client_data (lw_stream stream, void * tag, const char * buffer, size_t size)
{
	lw_server_client client = (lw_server_client)tag;
	lw_server server = client->server;

	#ifdef ENABLE_SSL
	  assert ( (!client->ssl) || lwp_sslclient_handshook (client->ssl) );
	#endif

	assert (server->on_data);

	server->on_data (server, client, buffer, size);
}

void on_client_close (lw_stream stream, void * tag)
{
	lw_server_client client = (lw_server_client)tag;

	lw_server ctx = client->server;

	lwp_trace ("Close %d", client->fdstream.fd);

	client->fdstream.fd = -1;

	if (client->on_connect_called)
	{
	  if (ctx->on_disconnect)
		 ctx->on_disconnect (ctx, client);
	}

	if (client->elem)
	{
		list_elem_remove(client->elem);
		client->elem = NULL;
	}

	#ifdef ENABLE_SSL
	  if (client->ssl)
		 lwp_sslclient_delete (client->ssl);
	#endif

	lw_stream_delete ((lw_stream) client);

	lwp_release (client, "server_client_new");
}

void lw_server_on_data (lw_server ctx, lw_server_hook_data on_data)
{
	ctx->on_data = on_data;

	if (on_data)
	{
	  /* Setting on_data to a handler */

	  if (!ctx->on_data)
	  {
		 list_each (lw_server_client, ctx->clients, client)
		 {
			lw_stream_add_hook_data ((lw_stream) client, on_client_data, client);
			lw_stream_read ((lw_stream) client, SIZE_MAX);
		 }
	  }

	  return;
	}

	/* Setting on_data to 0 */

	list_each (lw_server_client, ctx->clients, client)
	{
	  lw_stream_remove_hook_data ((lw_stream) client, on_client_data, client);
	}
}

lwp_def_hook (server, connect)
lwp_def_hook (server, disconnect)
lwp_def_hook (server, error)

