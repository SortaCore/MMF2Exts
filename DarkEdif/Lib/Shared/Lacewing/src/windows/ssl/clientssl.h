/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2013 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#ifndef _lw_ssl_client_h
#define _lw_ssl_client_h

#include "ssl.h"

typedef struct _lwp_clientssl
{
	struct _lwp_ssl ssl;

	CredHandle server_creds;

	lw_bool got_context;

} * lwp_clientssl;

void lwp_clientssl_init (lwp_clientssl,
						 CredHandle server_creds,
						 lw_server_client socket);

void lwp_clientssl_cleanup (lwp_clientssl);

#endif

