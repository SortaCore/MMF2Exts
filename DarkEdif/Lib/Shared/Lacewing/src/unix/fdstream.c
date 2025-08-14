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
#include "fdstream.h"
#include "eventpump.h"

extern const lw_streamdef def_fdstream;

/* FDStream makes the assumption that this will fail for anything but a regular
 * file (i.e. something that is always considered read ready)
 *
 * TODO: support more sendfile variants
 */

static lw_i64 lwp_sendfile (int source, int dest, lw_i64 size)
{
	#if defined (__linux__)

	 ssize_t sent = 0;

	 if ((sent = sendfile (dest, source, 0, (size_t)size)) == -1)
		 return errno == EAGAIN ? 0 : -1;

	 return sent;

	#elif defined (__FreeBSD__)

	 off_t sent = 0;

	 if (sendfile (source, dest, lseek (source, 0, SEEK_CUR),
					 size, 0, &sent, 0) != 0)
	 {
		if (errno != EAGAIN)
			return -1;
	 }

	 lseek (source, sent, SEEK_CUR);
	 return sent;

	#elif defined (__APPLE__)

	 off_t bytes = size;

	 if (sendfile (source, dest, lseek (source, 0, SEEK_CUR),
					 &bytes, 0, 0) != 0)
	 {
		if (errno != EAGAIN)
			return -1;
	 }

	 lseek (source, bytes, SEEK_CUR);
	 return bytes;

	#endif

	errno = EINVAL;
	return -1;
}

static void write_ready (void * tag)
{
	lw_stream_retry ((lw_stream) tag, lw_stream_retry_now);
}

static void read_ready (void * tag)
{
	lw_fdstream ctx = (lw_fdstream)tag;

	if (ctx->flags & lwp_fdstream_flag_reading)
		return;

	ctx->flags |= lwp_fdstream_flag_reading;

	lwp_retain (ctx, "fdstream read_ready");

	/* TODO : Use a buffer on the heap instead? */

	char buffer [lwp_default_buffer_size];

	lw_bool close_stream = lw_false;

	while (ctx->reading_size == SIZE_MAX || ctx->reading_size > 0)
	{
		if (ctx->fd == -1)
		 break;

		size_t to_read = sizeof (buffer);
		if (ctx->reading_size != SIZE_MAX && to_read > ctx->reading_size)
			to_read = ctx->reading_size;

		ssize_t bytes = read (ctx->fd, buffer, to_read);

		if (bytes == 0)
		{
			close_stream = lw_true;
			break;
		}

		if (bytes == -1)
		{
			if (errno == EAGAIN)
				break;

			lw_trace("read_ready: Abort for bytes == -1, errno %d (not -1), fd %d, ctx/tag %p (tag is fd_stream) - closing stream", errno, ctx->fd, ctx);
			close_stream = lw_true;
			break;
		}

		if (ctx->reading_size != SIZE_MAX)
		{
			if (bytes > (ssize_t)ctx->reading_size)
				ctx->reading_size = 0;
			else
				ctx->reading_size -= (size_t)bytes;
		}

		lw_stream_data ((lw_stream) ctx, buffer, (size_t)bytes);

		/* Calling Data or Close may result in destruction of the Stream -
		* see FDStream destructor.
		*/

		if (! (ctx->flags & lwp_fdstream_flag_reading))
		 break;
	}

	ctx->flags &= ~ lwp_fdstream_flag_reading;

	if (lwp_release(ctx, "fdstream read_ready") || ctx->stream.flags & lwp_stream_flag_dead)
		return;

	if (close_stream)
		lw_stream_close ((lw_stream) ctx, lw_true);
}

long lw_fdstream_get_fd_debug(lw_fdstream ctx)
{
	return ctx->fd;
}

void lw_fdstream_set_fd (lw_fdstream ctx, lw_fd fd, lw_pump_watch watch,
						 lw_bool auto_close, lw_bool is_socket)
{
	if (ctx->stream.watch)
	{
		lw_pump_remove (lw_stream_pump ((lw_stream) ctx), ctx->stream.watch);
		ctx->stream.watch = 0;
	}

	if ( (ctx->flags & lwp_fdstream_flag_autoclose) && ctx->fd != -1)
		lw_stream_close ((lw_stream) ctx, lw_true);

	ctx->fd = fd;

	if (auto_close)
		ctx->flags |= lwp_fdstream_flag_autoclose;
	else
		ctx->flags &= ~ lwp_fdstream_flag_autoclose;

	if (ctx->fd == -1)
		return;

	#ifdef HAVE_DECL_SO_NOSIGPIPE
	{	int b = 1;
		lwp_setsockopt (fd, SOL_SOCKET, SO_NOSIGPIPE, (char *) &b, sizeof (b));
	}
	#endif

	struct stat stat;
	fstat (fd, &stat);

	if (is_socket /* S_ISSOCK(stat.st_mode) */)
	{
		ctx->flags |= lwp_fdstream_flag_is_socket;

		lwp_make_nonblocking(fd);
	}
	else
	{
		ctx->flags &= ~ lwp_fdstream_flag_is_socket;

		if ((ctx->size = (size_t)stat.st_size) > 0)
			return;

		/* Not a socket, and size is 0.	Is it really just an empty file? */

		if (S_ISREG (stat.st_mode))
			return;
	}

	/* Assuming this is something we can watch for readiness. */

	ctx->size = SIZE_MAX;

	lw_pump pump = lw_stream_pump ((lw_stream) ctx);

	if (watch)
	{
		/* Given an existing pump watch - change it to use our callbacks */

		ctx->stream.watch = watch;

		lw_pump_update_callbacks (pump, ctx->stream.watch, ctx,
			read_ready, write_ready, lw_true);
	}
	else
	{
		ctx->stream.watch = lw_pump_add (pump, fd, ctx, read_ready,
			write_ready, lw_true);
	}
}

lw_bool lw_fdstream_valid (lw_fdstream ctx)
{
	return ctx->fd != -1;
}

void lw_fdstream_cork (lw_fdstream ctx)
{
	#ifdef lw_cork
		int enabled = 1;
		lwp_setsockopt (((lw_fdstream) ctx)->fd, IPPROTO_TCP,
						lw_cork, (const char *)&enabled, sizeof (enabled));
	#endif
}

void lw_fdstream_uncork (lw_fdstream ctx)
{
	#ifdef lw_cork
		int enabled = 0;
		lwp_setsockopt (((lw_fdstream) ctx)->fd, IPPROTO_TCP,
						lw_cork, (const char *)&enabled, sizeof (enabled));
	#endif
}

void lw_fdstream_nagle (lw_fdstream ctx, lw_bool enabled)
{
	if (enabled)
		ctx->flags |= lwp_fdstream_flag_nagle;
	else
		ctx->flags &= ~ lwp_fdstream_flag_nagle;

	if (ctx->fd != -1)
	{
		int b = enabled ? 0 : 1;
		lwp_setsockopt (ctx->fd, SOL_SOCKET, TCP_NODELAY, (char *) &b, sizeof (b));
	}
}

static size_t def_sink_data (lw_stream stream, const char * buffer, size_t size)
{
	lw_fdstream ctx = (lw_fdstream) stream;

	lwp_trace ("fdstream sink " lwp_fmt_size " bytes", size);

	ssize_t written;

	#ifdef HAVE_DECL_SO_NOSIGPIPE
		written = write (ctx->fd, buffer, size);
	#else
		if (ctx->flags & lwp_fdstream_flag_is_socket)
			written = send (ctx->fd, buffer, size, MSG_NOSIGNAL);
		else
			written = write (ctx->fd, buffer, size);
	#endif

	if (written == -1)
	{
		lwp_trace ("fdstream sank nothing!	write failed: %d", errno);
		return 0;
	}

	lwp_trace ("fdstream sank " lwp_fmt_size " of " lwp_fmt_size " bytes",
				(size_t)written, size);

	return (size_t)written;
}

static lw_i64 def_sink_stream (lw_stream _dest,
								lw_stream _src,
								size_t size)
{
	if (lw_stream_get_def (_src) != &def_fdstream)
		return -1;

	if (size == SIZE_MAX)
	{
		size = lw_stream_bytes_left (_src);

		if (size == SIZE_MAX)
		 return -1;
	}

	lw_fdstream source = (lw_fdstream) _src;
	lw_fdstream dest = (lw_fdstream) _dest;

	lw_i64 sent = lwp_sendfile (source->fd, dest->fd, (lw_i64)size);

	lwp_trace ("lwp_sendfile sent " lwp_fmt_size " of " lwp_fmt_size,
					 ((size_t) sent), (size_t) size);

	return sent;
}

static void def_cleanup (lw_stream _ctx)
{
	/* All of our cleanup work will already have been done by close, so there's
	* nothing to do here.
	*/
}

static void def_read (lw_stream _ctx, size_t bytes)
{
	lw_fdstream ctx = (lw_fdstream) _ctx;

	lwp_trace ("FDStream %p (FD %d) read " lwp_fmt_size, ctx, ctx->fd, bytes);

	lw_bool was_reading = ctx->reading_size != 0;

	if (bytes == SIZE_MAX)
		ctx->reading_size = lw_stream_bytes_left ((lw_stream) ctx);
	else
		ctx->reading_size += bytes;

	if (!was_reading)
		read_ready (ctx);
}

static size_t def_bytes_left (lw_stream _ctx)
{
	lw_fdstream ctx = (lw_fdstream) _ctx;

	if (ctx->fd == -1)
		return SIZE_MAX; /* not valid */

	if (ctx->size == SIZE_MAX)
		return SIZE_MAX;

	lwp_trace ("lseek for FD %d, size " lwp_fmt_size, ctx->fd, ctx->size);

	/* TODO : lseek64? */

	return ctx->size - (size_t)lseek (ctx->fd, 0, SEEK_CUR);
}

static lw_bool def_close (lw_stream stream_ctx, lw_bool immediate)
{
	lw_fdstream ctx = (lw_fdstream) stream_ctx;

	lwp_trace ("FDStream::Close for FD %d", ctx->fd);

	lwp_retain (ctx, "fdstream close");

	/* Remove the watch to make sure we don't get any more
	* callbacks from the pump.
	*/

	if (ctx->stream.watch)
	{
		lw_pump_remove (lw_stream_pump ((lw_stream) ctx), ctx->stream.watch);
		ctx->stream.watch = 0;
	}

	int fd = ctx->fd;

	ctx->fd = -1;

	if (fd != -1)
	{
		if (ctx->flags & lwp_fdstream_flag_autoclose)
		{
		 shutdown (fd, SHUT_RDWR);
		 close (fd);
		}
	}

	lwp_release (ctx, "fdstream close");

	return lw_true;
}

const lw_streamdef def_fdstream =
{
	.sink_data	 = def_sink_data,
	.sink_stream = def_sink_stream,
	.close		 = def_close,
	.bytes_left	 = def_bytes_left,
	.read		 = def_read,
	.cleanup	 = def_cleanup
};

void lwp_fdstream_init (lw_fdstream ctx, lw_pump pump)
{
	ctx->fd = -1;
	ctx->flags = lwp_fdstream_flag_nagle;
	ctx->size = ctx->reading_size = 0;

	lwp_stream_init (&ctx->stream, &def_fdstream, pump);
}

lw_fdstream lw_fdstream_new (lw_pump pump)
{
	lw_fdstream ctx = (lw_fdstream)malloc (sizeof (*ctx));
	if (!ctx)
		return NULL;
	lwp_fdstream_init (ctx, pump);

	return ctx;
}
