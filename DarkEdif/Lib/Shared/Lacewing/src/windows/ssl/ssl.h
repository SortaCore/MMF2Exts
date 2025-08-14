/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2013 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#ifndef _lw_ssl_h
#define _lw_ssl_h

#include "../../stream.h"

typedef struct _lwp_ssl
{
	DWORD status;
	lw_server_client client;
	void (*handle_error)(lw_server_client, lw_error);
	lw_bool handshake_complete;

	lw_bool got_context;
	CtxtHandle context;

	SecPkgContext_StreamSizes sizes;

	char * header, * trailer;

	size_t (* proc_handshake_data) (struct _lwp_ssl *,
									const char * buffer,
									size_t size);

	struct _lw_stream upstream;
	struct _lw_stream downstream;

} * lwp_ssl;

void lwp_ssl_init (lwp_ssl, lw_server_client socket);
void lwp_ssl_cleanup (lwp_ssl);

#endif


