/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012, 2013 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../../common.h"
#include "../../stream.h"
#include "serverssl.h"
#include "../fdstream.h"

static size_t proc_handshake_data (lwp_ssl ssl, const char * buffer, size_t size);

void lwp_serverssl_init (lwp_serverssl ctx,
						 CredHandle server_creds,
						 lw_server_client socket)
{
	ctx->server_creds = server_creds;
	ctx->socket = socket;
	lwp_ssl_init (&ctx->ssl, socket);

	ctx->ssl.proc_handshake_data = proc_handshake_data;
}

void lwp_serverssl_cleanup (lwp_serverssl ctx)
{
	lwp_ssl_cleanup (&ctx->ssl);
}

size_t proc_handshake_data (lwp_ssl ssl, const char * buffer, size_t size)
{
	lwp_serverssl ctx = (lwp_serverssl) ssl;

	SecBuffer in [2];

	  in [0].BufferType = SECBUFFER_TOKEN;
	  in [0].pvBuffer = (BYTE *) buffer;
	  in [0].cbBuffer = (unsigned long)size;

	  in [1].BufferType = SECBUFFER_EMPTY;
	  in [1].pvBuffer = 0;
	  in [1].cbBuffer = 0;

	SecBuffer out [2];

	  out [0].BufferType = SECBUFFER_TOKEN;
	  out [0].pvBuffer = 0;
	  out [0].cbBuffer = 0;

	  out [1].BufferType = SECBUFFER_EMPTY;
	  out [1].pvBuffer = 0;
	  out [1].cbBuffer = 0;

	SecBufferDesc in_desc = {0};

	in_desc.ulVersion = SECBUFFER_VERSION,
	in_desc.pBuffers = in;
	in_desc.cBuffers = 2;

	SecBufferDesc out_desc = {0};

	out_desc.ulVersion = SECBUFFER_VERSION,
	out_desc.pBuffers = out;
	out_desc.cBuffers = 2;

	int flags = ASC_REQ_SEQUENCE_DETECT | ASC_REQ_REPLAY_DETECT |
	  ASC_REQ_CONFIDENTIALITY | ASC_REQ_EXTENDED_ERROR |
	  ASC_REQ_ALLOCATE_MEMORY | ASC_REQ_STREAM;

	unsigned long out_flags;
	TimeStamp expiry_time;

	ctx->ssl.status = AcceptSecurityContext
	(
	  &ctx->server_creds,
	  ctx->ssl.got_context ? &ctx->ssl.context : 0,
	  &in_desc,
	  flags,
	  SECURITY_NATIVE_DREP,
	  ctx->ssl.got_context ? 0 : &ctx->ssl.context,
	  &out_desc,
	  &out_flags,
	  &expiry_time
	);

	ctx->ssl.got_context = lw_true;

	if (FAILED (ctx->ssl.status))
	{
	  if (ctx->ssl.status == SEC_E_INCOMPLETE_MESSAGE)
		 return 0; /* need more data */

	  lw_error error = lw_error_new();
	  lw_error_add(error, GetLastError() );
	  lw_error_addf(error, "Secure handshake failure");

	  if (ctx->ssl.handle_error)
		 ctx->ssl.handle_error(ctx->socket, error);

	  lw_error_delete(error);

	  return size;
	}

	if (ctx->ssl.status == SEC_E_OK || ctx->ssl.status == SEC_I_CONTINUE_NEEDED)
	{
	  /* Did AcceptSecurityContext give us back a response to send?
		*/
	  if (out [0].cbBuffer && out [0].pvBuffer)
	  {
		 lw_stream_data (&ctx->ssl.upstream, (char *) out [0].pvBuffer, out [0].cbBuffer);

		 FreeContextBuffer (out [0].pvBuffer);
	  }

	  /* Is there any data left over?
		*/
	  if (in [1].BufferType == SECBUFFER_EXTRA)
		 size -= in [1].cbBuffer;

	  if (ctx->ssl.status == SEC_E_OK)
		 ctx->ssl.handshake_complete = lw_true;
	}

	return size;
}

