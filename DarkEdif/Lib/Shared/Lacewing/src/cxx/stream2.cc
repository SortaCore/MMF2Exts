/* vim: set noet ts=4 sw=4 sts=4 ft=cpp:
 *
 * Copyright (C) 2012 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../common.h"

stream lacewing::stream_new (const lw_streamdef * def, lacewing::pump pump)
{
	return (stream) lw_stream_new (def, (lw_pump) pump);
}

void lacewing::stream_delete (lacewing::stream stream)
{
	lw_stream_delete ((lw_stream) stream);
}

void _stream::add_hook_data (hook_data hook, void * tag)
{
	lw_stream_add_hook_data ((lw_stream) this, (lw_stream_hook_data) hook, tag);
}

void _stream::remove_hook_data (hook_data hook, void * tag)
{
	lw_stream_remove_hook_data ((lw_stream) this, (lw_stream_hook_data) hook, tag);
}

void _stream::add_hook_close (hook_close hook, void * tag)
{
	lw_stream_add_hook_close ((lw_stream) this, (lw_stream_hook_close) hook, tag);
}

void _stream::remove_hook_close (hook_close hook, void * tag)
{
	lw_stream_remove_hook_close ((lw_stream) this, (lw_stream_hook_close) hook, tag);
}

size_t _stream::bytes_left ()
{
	return lw_stream_bytes_left ((lw_stream) this);
}

void _stream::read (size_t bytes)
{
	lw_stream_read ((lw_stream) this, bytes);
}

void _stream::begin_queue ()
{
	lw_stream_begin_queue ((lw_stream) this);
}

size_t _stream::queued ()
{
	return lw_stream_queued ((lw_stream) this);
}

void _stream::end_queue ()
{
	lw_stream_end_queue ((lw_stream) this);
}

void _stream::end_queue (int head_buffers,
						 const char ** buffers, size_t * lengths)
{
	lw_stream_end_queue_hb ((lw_stream) this, head_buffers, buffers, lengths);
}

void _stream::write (const char * buffer, size_t size)
{
	lw_stream_write ((lw_stream) this, buffer, size);
}

void _stream::writef (const char * format, ...)
{
	va_list args;
	va_start (args, format);

	lw_stream_writev ((lw_stream) this, format, args);

	va_end (args);
}

void _stream::write (stream s, size_t size, bool delete_when_finished)
{
	lw_stream_write_stream ((lw_stream) this, (lw_stream) s,
							size, delete_when_finished);
}

void _stream::write_file (const char * filename)
{
	lw_stream_write_file ((lw_stream) this, filename);
}

void _stream::add_filter_upstream (stream filter, bool delete_with_stream,
									bool close_together)
{
	lw_stream_add_filter_upstream ((lw_stream) this, (lw_stream) filter,
								  delete_with_stream, close_together);
}

void _stream::add_filter_downstream (stream filter, bool delete_with_stream,
									 bool close_together)
{
	lw_stream_add_filter_downstream ((lw_stream) this, (lw_stream) filter,
									delete_with_stream, close_together);
}

bool _stream::close (bool immediate)
{
	return lw_stream_close ((lw_stream) this, immediate);
}

lacewing::pump _stream::pump ()
{
	return (lacewing::pump) lw_stream_pump ((lw_stream) this);
}

void * _stream::tag ()
{
	return lw_stream_tag ((lw_stream) this);
}

void _stream::tag (void * tag)
{
	lw_stream_set_tag ((lw_stream) this, tag);
}

