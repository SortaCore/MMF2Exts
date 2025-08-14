/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#ifndef _WIN32
#include <netdb.h>
#endif

struct _lw_addr
{
	lw_thread resolver_thread;

	char * hostname, * hostname_to_free;
	char service [64]; /* port or service name */

	int hints;

	struct addrinfo * info_list, * info, * info_to_free;

	lw_error error;

	char buffer [64]; /* for to_string */

	void * tag;
};

void lwp_addr_init (lw_addr ctx, const char * hostname,
					const char * service, int hints);

void lwp_addr_cleanup (lw_addr ctx);

lw_addr lwp_addr_new_sockaddr (struct sockaddr *);
void lwp_addr_set_sockaddr (lw_addr ctx, struct sockaddr *);

