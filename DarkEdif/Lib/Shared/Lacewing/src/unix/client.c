/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012 James McLaughlin et al.
 * Copyright (C) 2012-2026 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../common.h"
#include "../address.h"
#include "fdstream.h"

#define lw_client_flag_connecting	((lw_i8)1)
#define lw_client_flag_connected	((lw_i8)2)

// 3 second timeout
static const int lw_client_timeout_ms = 3 * 1000;

struct _lw_client
{
	struct _lw_fdstream fdstream;

	lw_client_hook_connect	  on_connect;
	lw_client_hook_disconnect on_disconnect;
	lw_client_hook_data		  on_data;
	lw_client_hook_error	  on_error;

	lw_i8 flags;

	lw_addr remote_address;
	lw_addr local_address;
	lw_ui32 ifidx;

	lwp_socket socket;
	//lw_bool connecting; // part of flags, see lw_client_flag_connecting
	lw_timer connect_timer;

	//lw_pump pump;
	//lw_pump_watch watch;
	// Binds to a specific port, reset to 0 during connect attempt
	lw_ui16 local_port_next_connect;
};

void lw_client_connect_timeout(lw_timer timer)
{
	lw_timer_stop (timer);

	lw_client client = (lw_client)lw_timer_tag(timer);
	if (lw_client_connected(client))
		return;

	if (client->on_error)
	{
		lw_error error = lw_error_new();
		lw_error_addf(error, "Connection timeout");
		lw_error_addf(error, "Error connecting");
		client->on_error(client, error);

		lw_error_delete(error);
	}

	lwp_close_socket(client->socket);
	client->flags &= ~lw_client_flag_connecting;
}

lw_client lw_client_new (lw_pump pump)
{
	lw_client ctx = (lw_client)calloc (sizeof (*ctx), 1);

	ctx->ifidx = -1;

	lwp_init ();

	lwp_fdstream_init (&ctx->fdstream, pump);

	ctx->connect_timer = lw_timer_new(pump, "lw_client connect timer");
	lw_timer_set_tag (ctx->connect_timer, ctx);
	lw_timer_on_tick (ctx->connect_timer, lw_client_connect_timeout);

	return ctx;
}

void lw_client_delete (lw_client ctx)
{
	if (!ctx)
		return;

	lw_timer_delete (ctx->connect_timer);
	ctx->connect_timer = NULL;

	lw_stream_close ((lw_stream) ctx, lw_true);

	lw_addr_delete(ctx->local_address);
	ctx->local_address = NULL;
	lw_addr_delete(ctx->remote_address);
	ctx->remote_address = NULL;
	ctx->ifidx = -1;

	lwp_deinit();
	free (ctx);
}

void lw_client_connect (lw_client ctx, const char * host, lw_ui16 port)
{
	lw_addr address = lw_addr_new_port (host, port);

	lw_client_connect_addr (ctx, address);
}

// Connection handshake is done
static void first_time_write_ready (void * tag)
{
	// Checks that a connect() is done.
	// Runs the first time a write is possible. Reads are queued and handled after lw_fdstream_set_fd, if any.

	lw_client ctx = (lw_client)tag;

	// This is sometimes true when connection fails instantly
	// assert (!(ctx->flags & lw_client_flag_connecting))

	lw_timer_stop (ctx->connect_timer);

	int errorNum;

	{	socklen_t error_len = sizeof (errorNum);
		getsockopt (ctx->socket, SOL_SOCKET, SO_ERROR, &errorNum, &error_len);
	}

	if (errorNum != 0)
	{
		/* Failed to connect */

		ctx->flags &= ~ lw_client_flag_connecting;

		lw_error error = lw_error_new ();
		lw_error_add (error, errorNum);
		lw_error_addf (error, "Error connecting");

		if (ctx->on_error)
		 ctx->on_error (ctx, error);

		lw_error_delete (error);

		return;
	}

	// Clear pending writes, in case user sent messages without checking connection was ready
	// TODO: If a pending write count is kept, akin to Windows, then reset it here.
	list_clear(ctx->fdstream.stream.back_queue);

	lw_fdstream_set_fd (&ctx->fdstream, ctx->socket, lw_true, lw_true);

	struct sockaddr_storage ss = lwp_socket_addr((lwp_socket)ctx->socket);
	ctx->local_address = lwp_addr_new_sockaddr((struct sockaddr*)&ss);
	ctx->ifidx = lwp_get_ifidx(&ss);

	ctx->flags &= ~ lw_client_flag_connecting;

	if (ctx->on_connect)
		ctx->on_connect (ctx);

	if (ctx->on_data)
		lw_stream_read ((lw_stream) ctx, SIZE_MAX);
}

void lw_client_connect_addr (lw_client ctx, lw_addr address)
{
	if (lw_client_connected (ctx) || lw_client_connecting (ctx))
	{
		lw_error error = lw_error_new ();
		if (lw_client_connecting (ctx))
			lw_error_addf (error, "Already connecting to a server");
		else
			lw_error_addf (error, "Already connected to a server");

		if (ctx->on_error)
			ctx->on_error (ctx, error);

		lw_error_delete (error);

		return;
	}

	ctx->flags |= lw_client_flag_connecting;

	/* TODO : Resolve asynchronously? */

	{	lw_error error = lw_addr_resolve (address);

		if (error)
		{
			ctx->flags &= ~lw_client_flag_connecting;
			if (ctx->on_error)
				ctx->on_error (ctx, error);

			lw_error_delete (error);

			return;
		}
	}

	lw_addr_delete (ctx->remote_address);
	ctx->remote_address = lw_addr_clone (address);

	if (!ctx->remote_address->info)
	{
		ctx->flags &= ~lw_client_flag_connecting;

		lw_error error = lw_error_new();
		lw_error_addf(error, "The provided remote address is not ready for use");

		if (ctx->on_error)
			ctx->on_error(ctx, error);

		lw_error_delete(error);

		return;
	}

	if ((ctx->socket = socket (lw_addr_ipv6 (ctx->remote_address) ? AF_INET6 : AF_INET,
				SOCK_STREAM,
				IPPROTO_TCP)) == -1)
	{
		ctx->flags &= ~lw_client_flag_connecting;
		lw_error error = lw_error_new ();

		lw_error_add (error, errno);
		lw_error_addf (error, "Error creating socket");

		if (ctx->on_error)
			ctx->on_error (ctx, error);

		lw_error_delete (error);

		return;
	}

	lwp_make_nonblocking((lwp_socket)ctx->socket);

	// Android doesn't support AI_V4MAPPED, despite defining it.
	// Due to that, https://stackoverflow.com/questions/5587935/cant-turn-off-socket-option-ipv6-v6only#comment15775318_8213504
#if !defined(__ANDROID__) && !defined(__APPLE__)
	if (lw_addr_ipv6(ctx->remote_address))
		lwp_disable_ipv6_only((lwp_socket)ctx->socket);
#endif

	struct sockaddr_storage local_address = { 0 };

	// Lock to last outgoing local address if local port is being re-used,
	// which only happens for hole punch
	if (ctx->local_address && ctx->local_port_next_connect)
	{
		memcpy(&local_address, ctx->local_address->info->ai_addr, ctx->local_address->info->ai_addrlen);
		assert(lw_addr_ipv6(address) == lw_addr_ipv6(ctx->local_address));
	}
	// else bind to receiving from any address in remote's IPvX; possibly using a fixed local port,
	// but not using a fixed local address. Local port will be 0 by default, which is OS-pick local port.
	// inaddr_any/in6addr_any is all-zero, which we inited local_address to already.
	else if (lw_addr_ipv6(ctx->remote_address))
	{
		((struct sockaddr_in6 *)&local_address)->sin6_family = AF_INET6;
		((struct sockaddr_in6 *)&local_address)->sin6_port = htons(ctx->local_port_next_connect);
	}
	else
	{
		((struct sockaddr_in *)&local_address)->sin_family = AF_INET;
		((struct sockaddr_in *)&local_address)->sin_port = htons(ctx->local_port_next_connect);
	}

	// Reuse port or not, based on reserved port being non-zero
	const int was_locked_local = ctx->local_port_next_connect != 0 ? 1 : 0;
	lwp_setsockopt(ctx->socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&was_locked_local, sizeof(was_locked_local));
	ctx->local_port_next_connect = 0;

	if (bind(ctx->socket, (struct sockaddr *)&local_address,
		// sizeof sockaddr_storage doesn't work cross-platform
		lw_addr_ipv6(ctx->remote_address) ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in)) == -1)
	{
		ctx->flags &= ~lw_client_flag_connecting;
		close(ctx->socket);
		ctx->socket = -1;

		lw_error error = lw_error_new();

		lw_error_add(error, errno);
		lw_error_addf(error, "Error binding socket%s", was_locked_local != 0 ? " with fixed port" : "");

		if (ctx->on_error)
			ctx->on_error(ctx, error);

		lw_error_delete(error);

		return;
	}

	if (connect (ctx->socket, address->info->ai_addr,
			address->info->ai_addrlen) == -1)
	{
		lwp_trace("connect() done, error %d", errno);
		if (errno == EINPROGRESS)
		{
			// Start the connect timeout timer
			lw_timer_start(ctx->connect_timer, lw_client_timeout_ms);
			goto good; // No problem
		}

		close(ctx->socket);
		ctx->socket = -1;
		ctx->flags &= ~ lw_client_flag_connecting;

		lw_error error = lw_error_new ();

		lw_error_add (error, errno);
		lw_error_addf (error, "Error connecting to address");

		if (ctx->on_error)
			ctx->on_error (ctx, error);

		lw_error_delete (error);
		return;
	}

	// Else
	lwp_trace("connect() done, no error");
good:

	// Set Nagle. We can't do this in first_time_write_ready(), it causes EPERM on Android
	{	int b = (ctx->flags & lwp_fdstream_flag_nagle) ? 0 : 1;
	lwp_setsockopt(ctx->socket, SOL_SOCKET, TCP_NODELAY, (char *)&b, sizeof(b));
	}

	if (ctx->fdstream.stream.watch)
		lw_pump_update_callbacks(ctx->fdstream.stream.pump, ctx->fdstream.stream.watch,
			"lw_client_connect_addr resetting to first_time_write_ready", ctx, 0, first_time_write_ready, lw_true);
	else
		ctx->fdstream.stream.watch = lw_pump_add(ctx->fdstream.stream.pump, ctx->socket, "lw_client_connect_addr first init with first_time_write_ready", ctx, 0, first_time_write_ready, lw_true);
}

void lw_client_set_local_port (lw_client ctx, lw_ui16 localport)
{
	ctx->local_port_next_connect = localport;
}

lw_bool lw_client_connected (lw_client ctx)
{
	return lw_fdstream_valid ((lw_fdstream) ctx);
}

lw_bool lw_client_connecting (lw_client ctx)
{
	return ctx->flags & lw_client_flag_connecting;
}

lw_addr lw_client_server_addr (lw_client ctx)
{
	if (!lw_addr_ready(ctx->remote_address))
	{
		lw_log_if_debug("lw_client server addr not ready; client %p, address %p.", ctx, ctx->remote_address);
		lw_error err = lw_addr_resolve(ctx->remote_address);
		if (err)
		{
			lw_error_addf(err, "lw_client_server_addr()");
			if (ctx->on_error)
				ctx->on_error(ctx, err);
			lw_error_delete(err);
		}
	}
	return ctx->remote_address;
}
lw_addr lw_client_local_addr (lw_client ctx)
{
	return ctx->local_address;
}
lw_ui32 lw_client_ifidx (lw_client ctx)
{
	return ctx->ifidx;
}

static void on_stream_data (lw_stream stream, void * tag,
							const char * buffer, size_t length)
{
	lw_client ctx = (lw_client)tag;

	ctx->on_data (ctx, buffer, length);
}

void lw_client_on_data (lw_client ctx, lw_client_hook_data on_data)
{
	ctx->on_data = on_data;

	if (on_data)
	{
		lw_stream_add_hook_data ((lw_stream) ctx, on_stream_data, ctx);
		lw_stream_read ((lw_stream) ctx, SIZE_MAX);
	}
	else
	{
		lw_stream_remove_hook_data ((lw_stream) ctx, on_stream_data, ctx);
	}
}

static void on_close (lw_stream stream, void * tag)
{
	lw_client ctx = (lw_client)tag;

	ctx->on_disconnect (ctx);
}

void lw_client_on_disconnect (lw_client ctx,
								lw_client_hook_disconnect on_disconnect)
{
	ctx->on_disconnect = on_disconnect;

	if (on_disconnect)
		lw_stream_add_hook_close ((lw_stream) ctx, on_close, ctx);
	else
		lw_stream_remove_hook_close ((lw_stream) ctx, on_close, ctx);
}

lwp_def_hook (client, connect)
lwp_def_hook (client, error)

