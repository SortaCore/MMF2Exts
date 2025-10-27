/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011-2013 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "common.h"
#include "address.h"

struct _lw_filter
{
	lw_bool reuse, ipv6;

	lw_addr local, remote;
	long local_port, remote_port;

	void * tag;
};

lw_filter lw_filter_new ()
{
	lw_filter ctx = (lw_filter) malloc (sizeof (*ctx));
	if (!ctx)
		return NULL;

	ctx->local_port = 0;
	ctx->remote_port = 0;

	ctx->local = 0;
	ctx->remote = 0;

	ctx->reuse = lw_true;
	ctx->ipv6 = lw_true;

	return ctx;
}

lw_filter lw_filter_clone (lw_filter filter)
{
	lw_filter ctx = lw_filter_new ();

	lw_filter_set_ipv6 (ctx, lw_filter_ipv6 (filter));
	lw_filter_set_reuse (ctx, lw_filter_reuse (filter));

	lw_filter_set_local_port (ctx, lw_filter_local_port (filter));
	lw_filter_set_remote_port (ctx, lw_filter_remote_port (filter));

	lw_filter_set_local (ctx, lw_filter_local (filter));
	lw_filter_set_remote (ctx, lw_filter_remote (filter));

	return ctx;
}

void lw_filter_delete (lw_filter ctx)
{
	if (!ctx)
	  return;

	lw_addr_delete (ctx->local);
	lw_addr_delete (ctx->remote);

	free (ctx);
}

void lw_filter_set_remote (lw_filter ctx, lw_addr addr)
{
	if (addr)
	{
	  if (lw_addr_resolve (addr))
		 return;

	  lw_addr_delete (ctx->remote);
	  ctx->remote = lw_addr_clone (addr);

	  if (ctx->remote_port != 0)
		 lw_addr_set_port (ctx->remote, ctx->remote_port);
	}
	else
	{
	  if (ctx->remote)
		 ctx->remote_port = lw_addr_port (ctx->remote);

	  lw_addr_delete (ctx->remote);
	  ctx->remote = 0;
	}
}

lw_addr lw_filter_remote (lw_filter ctx)
{
	return ctx ? ctx->remote : NULL;
}

void lw_filter_set_local (lw_filter ctx, lw_addr addr)
{
	if (addr)
	{
	  if (lw_addr_resolve (addr))
		 return;

	  lw_addr_delete (ctx->local);
	  ctx->local = lw_addr_clone (addr);

	  if (ctx->local_port != 0)
		 lw_addr_set_port (ctx->local, ctx->local_port);
	}
	else
	{
	  if (ctx->local)
		 ctx->local_port = lw_addr_port (ctx->local);

	  lw_addr_delete (ctx->local);
	  ctx->local = 0;
	}
}

lw_addr lw_filter_local (lw_filter ctx)
{
	return ctx->local;
}

void lw_filter_set_reuse (lw_filter ctx, lw_bool enabled)
{
	ctx->reuse = enabled;
}

lw_bool lw_filter_reuse (lw_filter ctx)
{
	return ctx->reuse;
}

void lw_filter_set_ipv6 (lw_filter ctx, lw_bool enabled)
{
	ctx->ipv6 = enabled;
}

lw_bool lw_filter_ipv6 (lw_filter ctx)
{
	return ctx->ipv6;
}

long lw_filter_local_port (lw_filter ctx)
{
	return ctx->local ? lw_addr_port (ctx->local) : ctx->local_port;
}

void lw_filter_set_local_port (lw_filter ctx, long port)
{
	if (ctx->local)
	  lw_addr_set_port (ctx->local, port);
	else
	  ctx->local_port = port;
}

long lw_filter_remote_port (lw_filter ctx)
{
	return ctx->remote ? lw_addr_port (ctx->remote) : ctx->remote_port;
}

void lw_filter_set_remote_port (lw_filter ctx, long port)
{
	if (ctx->remote)
	  lw_addr_set_port (ctx->remote, port);
	else
	  ctx->remote_port = port;
}

lwp_socket lwp_create_server_socket (lw_filter filter, int type,
									 int protocol, lw_error error)
{
	lwp_socket s;
	socklen_t addr_len;
	struct sockaddr_storage addr;
	lw_bool ipv6;
	int reuse;

	ipv6 = lw_filter_ipv6 (filter);

	if (!ipv6)
	{
	  if (  (filter->local && lw_addr_ipv6 (filter->local))
			|| (filter->remote && lw_addr_ipv6 (filter->remote)) )
	  {
		 lw_error_addf (error,
				"Filter has IPv6 disabled, but one of the attached addresses is an IPv6 address.  "
				"Try using HINT_IPv4 when creating the address.");

		 return -1;
	  }
	}

	#ifdef _WIN32

		if (ipv6)
		{
			if ((s = WSASocket
				(AF_INET6, type, protocol, 0, 0, WSA_FLAG_OVERLAPPED)) == -1)
			{
				if (WSAGetLastError () != WSAEAFNOSUPPORT)
				{
					lw_error_add (error, WSAGetLastError ());
					lw_error_addf (error, "Error creating socket");

					return -1;
				}

				ipv6 = lw_false;
			}
			else
				goto ipv6success;
		}

		if (!ipv6)
		{
			if ((s = WSASocket
				(AF_INET, type, protocol, 0, 0, WSA_FLAG_OVERLAPPED)) == -1)
			{
				lw_error_add (error, WSAGetLastError ());
				lw_error_addf (error, "Error creating socket");

				return -1;
			}
		}

	#else

		if (ipv6)
		{
			if ((s = socket (AF_INET6, type, protocol)) == -1)
			{
				if (errno != EAFNOSUPPORT)
				{
					lw_error_add (error, errno);
					lw_error_addf (error, "Error creating socket");

					return -1;
				}

				ipv6 = lw_false;
			}
			else
				goto ipv6success;
		}

		if (!ipv6)
		{
			if ((s = socket (AF_INET, type, protocol)) == -1)
			{
				lw_error_add (error, errno);
				lw_error_addf (error, "Error creating socket");

				return -1;
			}
		}

		lwp_make_nonblocking(s);

	#endif

	ipv6success:
	if (ipv6)
	  lwp_disable_ipv6_only (s);

	reuse = lw_filter_reuse (filter) ? 1 : 0;
	lwp_setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));

	memset (&addr, 0, sizeof (addr));

	addr_len = 0;

	if (lw_filter_local (filter))
	{
	  struct addrinfo * info = lw_filter_local (filter)->info;

	  if (info)
	  {
		 memcpy (&addr, info->ai_addr, info->ai_addrlen);
		 addr_len = (socklen_t)info->ai_addrlen;
	  }
	}

	if (!addr_len)
	{
	  if (ipv6)
	  {
		 addr_len = sizeof (struct sockaddr_in6);

		 ((struct sockaddr_in6 *) &addr)->sin6_family = AF_INET6;
		 ((struct sockaddr_in6 *) &addr)->sin6_addr = in6addr_any;

		 ((struct sockaddr_in6 *) &addr)->sin6_port
			= lw_filter_local_port (filter) ?
				htons ((unsigned short) lw_filter_local_port (filter)) : 0;
	  }
	  else
	  {
		 addr_len = sizeof (struct sockaddr_in);

		 ((struct sockaddr_in *) &addr)->sin_family = AF_INET;
		 ((struct sockaddr_in *) &addr)->sin_addr.s_addr = INADDR_ANY;

		 ((struct sockaddr_in *) &addr)->sin_port
			= lw_filter_local_port (filter) ?
				htons ((unsigned short) lw_filter_local_port (filter)) : 0;
	  }
	}


	#ifdef _WIN32
		#pragma warning (suppress: 6385) // No, it's not over-reading
	#endif

	// always bind local address; either to wildcard, or to specific
	if (bind (s, (struct sockaddr *) &addr, (socklen_t) addr_len) == -1)
	{
	  lw_error_add (error, lwp_last_socket_error);

#if _WIN32
		if (lwp_last_socket_error == 10013 || lwp_last_socket_error == 10048)
			lw_error_addf(error, "Port is in use already?");
#else
		if (lwp_last_socket_error == EACCES)
		{
			if (lw_filter_local_port(filter) > 0 && lw_filter_local_port(filter) < 1024)
				lw_error_addf(error, "Priviledged port, needs sudo/iptables port redirect; or port is in use already?");
			else
				lw_error_addf(error, "Port is in use already?");
		}
#endif

	  lw_error_addf (error, "Error binding socket (port %li)", lw_filter_local_port(filter));

	  lwp_close_socket (s);

	  return -1;
	}

	return s;
}

void * lw_filter_tag (lw_filter ctx)
{
	return ctx->tag;
}

void lw_filter_set_tag (lw_filter ctx, void * tag)
{
	ctx->tag = tag;
}

