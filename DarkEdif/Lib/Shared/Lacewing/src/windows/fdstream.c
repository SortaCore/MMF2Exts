/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012, 2013 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../common.h"
#include "fdstream.h"

extern const lw_streamdef def_fdstream;

static void issue_read (lw_fdstream);
static void read_completed (lw_fdstream);
static void write_completed (lw_fdstream);

static void add_pending_write (lw_fdstream ctx)
{
	if ((++ ctx->num_pending_writes) == 1)
	{
	/* Since writes are now pending, the stream must be retained. */

	lwp_retain (ctx, "fdstream pending write");
	}
}

static void remove_pending_write (lw_fdstream ctx)
{
	if ((-- ctx->num_pending_writes) == 0)
	{
	/* If any writes were pending, the stream was being retained.  Since the
		* last write has finished, we can release it now.
		*/

		lwp_release (ctx, "fdstream pending write");
	}
}

#define overlapped_type_read		1
#define overlapped_type_write		2
#define overlapped_type_transmitfile  3

static void completion (void * tag, OVERLAPPED * _overlapped,
						unsigned long bytes_transferred, int error)
{
	lw_fdstream ctx = (lw_fdstream) tag;
	fdstream_overlapped overlapped = (fdstream_overlapped) _overlapped;

	lwp_retain (ctx, "fdstream completion");

	switch (overlapped->type)
	{
	case overlapped_type_read:
		assert (overlapped == &ctx->read_overlapped);

		read_completed (ctx);

		if (error == ERROR_OPERATION_ABORTED)
			break;

		if (ctx->stream.flags & lwp_stream_flag_dead)
			break;

		if (error || !bytes_transferred)
		{
			// First, establish if connection has died, or temporarily had an error.
			// If it's died, we'll do an immediate shutdown, otherwise a graceful.

			lw_bool otherEndUnreachable =
				// Connection closed on other end nicely
				(error == NO_ERROR && bytes_transferred == 0) ||
				// Connection closed on other end
				error == ERROR_NETNAME_DELETED ||
				// Local network went down (something providing connectivity failed)
				error == WSAENETDOWN ||
				// Local network unreachable (no route to other side)
				error == WSAENETUNREACH || error == ERROR_NETWORK_UNREACHABLE ||
				// Local network was reset, and this connection was closed forcibly; e.g. network adapter reset
				error == WSAENETRESET ||
				// Local or remote end aborted their connection
				error == WSAECONNABORTED || error == ERROR_CONNECTION_ABORTED ||
				// Remote end reset their connection
				error == WSAECONNRESET || error == ERROR_CONNECTION_UNAVAIL ||
				// Local socket was shut down via shutdown()/closesocket() call (can also cause connection aborted)
				error == WSAESHUTDOWN ||
				// Local socket not connected
				error == WSAENOTCONN || error == ERROR_PORT_UNREACHABLE ||
				// Local socket was disconnected by network cable being unplugged (and a write was done and timed out)
				error == ERROR_SEM_TIMEOUT;
			lwp_trace("Closing stream %p, due to error %i. Unreachable: %s.\n", ctx, error, otherEndUnreachable ? "YES" : "NO");

			lw_stream_close ((lw_stream) ctx, otherEndUnreachable);
			break;
		}

		lw_stream_data ((lw_stream) ctx, ctx->buffer, bytes_transferred);

		issue_read (ctx);
		break;

	case overlapped_type_write:
		lw_sync_lock(ctx->pending_writes_sync);
		list_remove (fdstream_overlapped, ctx->pending_writes, overlapped);
		free (overlapped);

		write_completed (ctx);
		lw_sync_release(ctx->pending_writes_sync);

		break;

	case overlapped_type_transmitfile:
	{
		lw_sync_lock(ctx->pending_writes_sync);
		assert (overlapped == &ctx->transmitfile_overlapped);

		ctx->transmit_file_from->transmit_file_to = 0;
		write_completed (ctx->transmit_file_from);
		ctx->transmit_file_from = 0;

		write_completed (ctx);
		lw_sync_release(ctx->pending_writes_sync);

		break;
	}

	default:
		assert (lw_false);
	};

	lwp_release (ctx, "fdstream completion");
}

static void close_fd (lw_fdstream ctx)
{
	lwp_trace ("FDStream %p with FD %d: close_fd", ctx, ctx->fd);

	if (ctx->fd == INVALID_HANDLE_VALUE)
	return;

	if (ctx->transmit_file_from)
	{
		ctx->transmit_file_from->transmit_file_to = 0;
		write_completed (ctx->transmit_file_from);
		ctx->transmit_file_from = 0;

		/* Not calling write_completed for this stream because the FD
		 * is closing anyway (and write_completed may result in our
		 * destruction, which would be annoying here.) */
	}

	if (ctx->transmit_file_to)
	{
		ctx->transmit_file_to->transmit_file_from = 0;
		write_completed (ctx->transmit_file_to);
		ctx->transmit_file_to = 0;
	}

	// No need to run CancelIoEx(), closesocket() will run it internally

	if (ctx->flags & lwp_fdstream_flag_auto_close)
	{
		if (ctx->flags & lwp_fdstream_flag_is_socket)
		{
			shutdown((SOCKET)ctx->fd, SD_BOTH);

			if (closesocket ((SOCKET) ctx->fd) == SOCKET_ERROR)
			{
				// There's no error reporting function here, and it's not worth killing the app over.
				#ifdef _lacewing_debug

				lw_error err = lw_error_new();
				lw_error_add(err, WSAGetLastError());
				lwp_trace("closesocket() returned %s", lw_error_tostring(err));
				lw_error_delete(err);

				#endif
			}
			else
				ctx->fd = INVALID_HANDLE_VALUE;
		}
		else
		{
			if (CloseHandle(ctx->fd) == FALSE)
			{
				// There's no error reporting function here, and it's not worth killing the app over.
				#ifdef _lacewing_debug

				lw_error err = lw_error_new();
				lw_error_add(err, GetLastError());
				lwp_trace("CloseHandle() returned %s", lw_error_tostring(err));
				lw_error_delete(err);

				#endif
			}
			else
				ctx->fd = INVALID_HANDLE_VALUE;
		}
	}

	//list_clear(ctx->pending_writes);
}

void write_completed (lw_fdstream ctx)
{
	lw_sync_lock(ctx->pending_writes_sync);
	remove_pending_write (ctx);

	if (ctx->num_pending_writes == 0)
	{
		// Were we trying to close?
		if ( (ctx->flags & lwp_fdstream_flag_close_asap) && !(ctx->flags & lwp_fdstream_flag_read_pending))
		{
			lw_sync_release(ctx->pending_writes_sync);
			close_fd (ctx);

			lw_stream_close ((lw_stream) ctx, lw_true);
			return;
		}
	}
	lw_sync_release(ctx->pending_writes_sync);
}

void issue_read (lw_fdstream ctx)
{
	if (ctx->fd == INVALID_HANDLE_VALUE)
		return;

	if ((ctx->flags & lwp_fdstream_flag_read_pending) != 0)
	//if ((ctx->flags & (lwp_fdstream_flag_read_pending | lwp_fdstream_flag_close_asap)) != 0 || (ctx->stream.flags & lwp_stream_flag_closeASAP) != 0)
		return; // Only one read pending on a stream at once

	ctx->flags |= lwp_fdstream_flag_read_pending;
	lwp_retain(ctx, "fdstream read");  /* retain the stream for the duration of the read op */

	memset (&ctx->read_overlapped, 0, sizeof (ctx->read_overlapped));

	ctx->read_overlapped.type = overlapped_type_read;

	ctx->read_overlapped.overlapped.Offset = ctx->offset.LowPart;
	ctx->read_overlapped.overlapped.OffsetHigh = ctx->offset.HighPart;

	DWORD to_read = sizeof (ctx->buffer);

	if (ctx->reading_size != -1 && to_read > ctx->reading_size)
	{
		to_read = (DWORD)ctx->reading_size;
		if (sizeof(ctx->reading_size) > 4)
			assert(ctx->reading_size < 0xFFFFFFFF);
	}

	// On error or running async, 0 will be returned.
	if (ReadFile (ctx->fd, ctx->buffer, to_read,
				  0, &ctx->read_overlapped.overlapped) == FALSE)
	{
		lw_ui32 error = GetLastError();
		// IO Pending is a non-error indicating ReadFile() is running asynchronously
		if (error == ERROR_IO_PENDING)
		{
			if (ctx->size != -1)
				ctx->offset.QuadPart += to_read;
			lwp_trace("FDStream %p; ReadFile returned async with no error", ctx);
			return;
		}

		// Otherwise, real error; this read op has been cancelled so run it as completed
		lwp_trace("FDStream %p; ReadFile returned error %u", ctx, error);
		read_completed(ctx);
		return;
	}
	// else ran synchronously with no error.

	if (ctx->size != -1)
		ctx->offset.QuadPart += to_read;
	lwp_trace("FDStream %p; ReadFile returned sync with no error", ctx);
}

void read_completed (lw_fdstream ctx)
{
	ctx->flags &= ~ lwp_fdstream_flag_read_pending;
	lwp_release (ctx, "fdstream read");  /* matches retain in issue_read */

	// A graceful close is scheduled, and no read/writes remaining; close now
	if ((ctx->flags & lwp_fdstream_flag_close_asap) && ctx->num_pending_writes == 0)
	{
		lw_stream_close((lw_stream)ctx, lw_true);
		return;
	}
}

static void def_cleanup (lw_stream _ctx)
{
	lw_fdstream ctx = (lw_fdstream) _ctx;

	close_fd (ctx);
}

void lw_fdstream_set_fd (lw_fdstream ctx, HANDLE fd,
						lw_pump_watch watch, lw_bool auto_close, lw_bool is_socket)
{
	lwp_trace ("FDStream %p : set FD to %d, auto_close %d, is_socket %d", ctx, fd, (int) auto_close, (int) is_socket);

	if (ctx->stream.watch)
	{
		// Occasionally triggers during a disconnect/reconnect spam.
		// Simple recreation: "On connect > Disconnect; Connect to X"
		//
		// TODO: The client is unstable at that point, and will not properly disconnect from original connection
		// nor acknowledge the server on the new connection, resulting in ping disconnect.
		// Thankfully, after a ping disconnect, it should be back to functional again.
		assert(!"disconnecting and reconnecting too fast, now unstable.");

		// These following lines prevent an issue where the old callbacks/pump are sent to new callback,
		// which will result in accumulating On Disconnect events, apparently.
		lw_pump pump = lw_stream_pump((lw_stream)ctx);
		lw_pump_update_callbacks(pump, ctx->stream.watch, NULL, NULL);
		lw_pump_post_remove(pump, ctx->stream.watch);
		ctx->stream.watch = NULL;
	}

	ctx->fd = fd;

	if (fd == INVALID_HANDLE_VALUE)
		return;

	if (is_socket)
		ctx->flags |= lwp_fdstream_flag_is_socket;

	if (auto_close)
		ctx->flags |= lwp_fdstream_flag_auto_close;

	/*
	// PHI: Use this and your socket will remain open in CLOSE_WAIT for several minutes after
	// disconnect, expecting this duplicated socket info to be used in a second socket.
	// See Lacewing.h.
	// I've modified function to pass whether it's a socket instead.
	// Alternatively, https://stackoverflow.com/a/6574906 (NtQueryObject) could be used.

	WSAPROTOCOL_INFO info;
	if (WSADuplicateSocket ((SOCKET) ctx->fd,
							GetCurrentProcessId (),
							&info) == SOCKET_ERROR)
	{
		int error = WSAGetLastError ();

		if (error == WSAENOTSOCK || error == WSAEINVAL)
			ctx->flags &= ~ lwp_fdstream_flag_is_socket;
	}*/

	if (ctx->flags & lwp_fdstream_flag_is_socket)
	{
		ctx->size = -1;

		int b = (ctx->flags & lwp_fdstream_flag_nagle) ? 0 : 1;

		setsockopt ((SOCKET) ctx->fd, SOL_SOCKET, TCP_NODELAY,
					(char *) &b, sizeof (b));
	}
	else
	{
		LARGE_INTEGER size;

		if (!compat_GetFileSizeEx () (ctx->fd, &size))
			return;

		// If GetFileSizeEx returned true, this was set, so suppress the uninited var warning
		#ifdef _MSC_VER
			#pragma warning (suppress: 6001)
		#endif
		ctx->size = (size_t) size.QuadPart;

		assert (ctx->size != -1);
	}

	if (watch)
	{
		ctx->stream.watch = watch;

		lw_pump_update_callbacks (lw_stream_pump ((lw_stream) ctx),
								watch, ctx, completion);
	}
	else
	{
		ctx->stream.watch = lw_pump_add (lw_stream_pump ((lw_stream) ctx),
								fd, ctx, completion);
	}

	if (ctx->reading_size != 0)
		issue_read (ctx);
}

lw_bool lw_fdstream_valid (lw_fdstream ctx)
{
	return ctx->fd != INVALID_HANDLE_VALUE && !(ctx->flags & lwp_fdstream_flag_close_asap);
}
long  lw_fdstream_get_fd_debug(lw_fdstream ctx)
{
	return *(long *)&ctx->fd;
}

/* Note that this always swallows all of the data (unlike on *nix where it
 * might only be able to use some of it.)  Because Windows FDStream never
 * calls WriteReady(), it's important that nothing gets buffered in the
 * Stream.
 */

static size_t def_sink_data (lw_stream _ctx, const char * buffer, size_t size)
{
	lw_fdstream ctx = (lw_fdstream) _ctx;

	if (!size)
		return size; /* nothing to do */

	// Size must be storable in 32-bit
	if (sizeof(size) > 4)
		assert(size < 0xFFFFFFFF);

	if (_ctx->flags & lwp_fdstream_flag_close_asap)
		return size; /* stream closing, cancel the write */

	/* TODO : Pre-allocate a bunch of these and reuse them? */

	fdstream_overlapped overlapped = (fdstream_overlapped)malloc (sizeof (*overlapped) + size);

	if (!overlapped)
		return size;

#ifdef _MSC_VER
	// this can't be an overrun
	#pragma warning (suppress: 6386)
#endif
	memset (overlapped, 0, sizeof (*overlapped));
	overlapped->type = overlapped_type_write;

	((OVERLAPPED *) overlapped)->Offset = ctx->offset.LowPart;
	((OVERLAPPED *) overlapped)->OffsetHigh = ctx->offset.HighPart;

	/* TODO : Find a way to avoid copying the data. */

	memcpy (overlapped->data, buffer, size);

	/* TODO : Use WSASend if IsSocket == true?  (for better error messages.)
	* Same goes for ReadFile and WSARecv.
	*/

	// We add before write, because the IOCP thread writing it could free it underneath us
	// But... wouldn't this create an atomic issue? either way, two things are modifying pending_writes with no sync
	lw_sync_lock(ctx->pending_writes_sync);
	add_pending_write(ctx);
	list_push(fdstream_overlapped, ctx->pending_writes, overlapped);

	if (WriteFile(ctx->fd,
		overlapped->data,
		(DWORD)size,
		0,
		(OVERLAPPED*)overlapped) == FALSE)
	{
		int error = GetLastError();

		if (error != ERROR_IO_PENDING)
		{
			free(overlapped);

			// There's no error reporting function here, and not worth killing the app over
#ifdef _lacewing_debug
			lw_error err = lw_error_new();
			lw_error_add(err, error);
			lwp_trace("Failed to write to socket %p, got error %s", ctx, lw_error_tostring(err));
			lw_error_delete(err);
#endif
			lw_sync_release(ctx->pending_writes_sync);

			return size;
		}
	}

	if (ctx->size != -1)
		ctx->offset.QuadPart += size;

	lw_sync_release(ctx->pending_writes_sync);
	return size;
}

static lw_i64 def_sink_stream (lw_stream _dest, lw_stream _src, size_t size)
{
	if (lw_stream_get_def (_src) != &def_fdstream)
		return -1;

	if (size == -1)
	{
		size = lw_stream_bytes_left (_src);

		if (size == -1)
			return -1;
	}

	lw_fdstream source = (lw_fdstream) _src;
	lw_fdstream dest = (lw_fdstream) _dest;

	if (size >= (((size_t) 1024) * 1024 * 1024 * 2))
		return -1;

	/* TransmitFile can only send from a file to a socket */

	if (source->flags & lwp_fdstream_flag_is_socket)
		return -1;

	if (! (dest->flags & lwp_fdstream_flag_is_socket))
		return -1;

	if (dest->transmitfile_from || source->transmitfile_to)
		return -1;  /* source or dest stream already performing a TransmitFile */

	fdstream_overlapped overlapped = &dest->transmitfile_overlapped;

	memset (overlapped, 0, sizeof (*overlapped));
	overlapped->type = overlapped_type_transmitfile;

	/* Use the current offset from the source stream */

	((OVERLAPPED *) overlapped)->Offset = source->offset.LowPart;
	((OVERLAPPED *) overlapped)->OffsetHigh = source->offset.HighPart;

	/* TODO : Can we make use of the head buffers somehow?  Perhaps if data
	* is queued and preventing this TransmitFile from happening immediately,
	* the head buffers could be used to drain it.
	*/

	// Phi note: this sync and the pending_write count is only half-implemented with file transmit,
	// so redo it and make sure it's consistent if you're allowing it.
	lw_sync_lock(dest->pending_writes_sync);

	if (!TransmitFile ((SOCKET) dest->fd,
					source->fd,
					(DWORD) size,
					0,
					(OVERLAPPED *) overlapped,
					0,
					0))
	{
		int error = WSAGetLastError ();

		if (error != WSA_IO_PENDING)
		{
			lw_sync_release(dest->pending_writes_sync);
			return -1;
		}
	}

	/* OK, looks like the TransmitFile call succeeded. */

	dest->transmitfile_from = source;
	source->transmitfile_to = dest;

	if (source->size != -1)
		source->offset.QuadPart += size;

	add_pending_write (dest);
	add_pending_write (source);

	lw_sync_release(dest->pending_writes_sync);
	/* As far as stream is concerned, we've now written everything. */

	return size;
}

static void def_read (lw_stream _ctx, size_t bytes)
{
	lw_fdstream ctx = (lw_fdstream) _ctx;

	lw_bool was_reading = ctx->reading_size != 0;

	if (bytes == -1)
		ctx->reading_size = lw_stream_bytes_left ((lw_stream) ctx);
	else
		ctx->reading_size += bytes;

	if (!was_reading)
		issue_read (ctx);
}

static size_t def_bytes_left (lw_stream _ctx)
{
	lw_fdstream ctx = (lw_fdstream) _ctx;

	if (!lw_fdstream_valid (ctx))
		return -1;

	if (ctx->size == -1)
		return -1;

	return ctx->size - ((size_t) ctx->offset.QuadPart);
}

/* Unlike with *nix, there's the possibility that data might actually be
 * pending in the FDStream, rather than just the Stream.
 */


static lw_bool def_close (lw_stream _ctx, lw_bool immediate)
{
	lw_fdstream ctx = (lw_fdstream) _ctx;

	// If ordered to close immediately, or we can anyway, do so.
	if (immediate /*|| ctx->num_pending_writes == 0*/)
	{
		lwp_retain (ctx, "fdstream close");

		close_fd (ctx);

		lwp_release (ctx, "fdstream close");

		return lw_true;
	}
	else
	{
		ctx->flags |= lwp_fdstream_flag_close_asap;
		shutdown((SOCKET)ctx->fd, SD_RECEIVE);

		if (compat_CancelIoEx())
			compat_CancelIoEx()(ctx->fd, &ctx->read_overlapped.overlapped);
		return lw_false;
	}
}

void lw_fdstream_nagle (lw_fdstream ctx, lw_bool enabled)
{
	if (enabled)
		ctx->flags |= lwp_fdstream_flag_nagle;
	else
		ctx->flags &= ~ lwp_fdstream_flag_nagle;

	if (lw_fdstream_valid (ctx))
	{
		int b = enabled ? 0 : 1;

		setsockopt ((SOCKET) ctx->fd, SOL_SOCKET,
				TCP_NODELAY, (char *) &b, sizeof (b));
	}
}

/* TODO : Can we do anything here on Windows? */

void lw_fdstream_cork (lw_fdstream ctx)
{
}

void lw_fdstream_uncork (lw_fdstream ctx)
{
}

const lw_streamdef def_fdstream =
{
	def_sink_data,
	def_sink_stream,
	0, /* retry */
	0, /* is_transparent */
	def_close,
	def_bytes_left,
	def_read,
	def_cleanup
};

void lw_fdstream_dealloc(lw_fdstream ctx)
{
	// No refs, so there should be no pending writes
	assert(list_length(ctx->pending_writes) == 0);
	list_clear(ctx->pending_writes);
	lw_sync_delete(ctx->pending_writes_sync);

	free(ctx);
}
void lwp_fdstream_init (lw_fdstream ctx, lw_pump pump)
{
	memset (ctx, 0, sizeof (*ctx));

	ctx->fd	= INVALID_HANDLE_VALUE;
	ctx->flags  = lwp_fdstream_flag_nagle;
	ctx->size	= -1;

	ctx->pending_writes_sync = lw_sync_new();

	lwp_stream_init ((lw_stream) ctx, &def_fdstream, pump);
	lwp_set_dealloc_proc(ctx, lw_fdstream_dealloc);
}

lw_fdstream lw_fdstream_new (lw_pump pump)
{
	lw_fdstream ctx = (lw_fdstream) malloc (sizeof (*ctx));

	if (!ctx)
		return 0;

	lwp_init ();

	lwp_fdstream_init (ctx, pump);

	return ctx;
}
