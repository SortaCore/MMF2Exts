
/* vim: set et ts=3 sw=3 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _lw_stream_h
#define _lw_stream_h

#include "streamgraph.h"

/* BeginQueue has been called */
 #define lwp_stream_flag_queueing  1

/* Close was called with immediately = false, stream was busy */
 #define lwp_stream_flag_closeASAP  2

/* Currently in the process of an immediate Close (this is just to prevent
 * re-entrance to the Close routine)
 */
 #define lwp_stream_flag_closing  4

/* Deleted when user count was > 0 - waiting to be freed */
 #define lwp_stream_flag_dead 8

/* Currently attempting to drain the queues.  Because draining the queues
 * might cause another retry, this flag prevents re-entrance to the
 * write_queued proc.
 */
 #define lwp_stream_flag_draining_queues 16

typedef struct _lwp_stream_data_hook
{
   lw_stream_hook_data proc;

   lw_stream stream;
   void * tag;

} * lwp_stream_data_hook;

typedef struct _lwp_stream_close_hook
{
   lw_stream_hook_close proc;

   void * tag;

} * lwp_stream_close_hook;

#define lwp_stream_queued_data           1
#define lwp_stream_queued_stream         2
#define lwp_stream_queued_begin_marker   3

typedef struct _lwp_stream_queued
{
   char type;

   lwp_heapbuffer buffer;

   lw_stream stream;
   size_t stream_bytes_left;
   lw_bool delete_stream;

} * lwp_stream_queued;

typedef struct _lwp_stream_filterspec
{
   lw_stream stream;
   lw_stream filter;

   lw_bool delete_with_stream;
   lw_bool close_together;

   /* Having a pre-allocated link here saves StreamGraph from having to
    * allocate one for each filter each time the graph is expanded.
    */

   struct _lwp_streamgraph_link link;

} * lwp_stream_filterspec;

struct _lw_stream
{
    lwp_refcounted;

    const lw_streamdef * def;

    lw_pump pump;
    
    char flags;

    void * tag;

    list (lwp_stream_data_hook, data_hooks);
    list (struct _lwp_stream_close_hook, close_hooks);


    /* Filters affecting this stream (stream == this).  The filter should be
     * freed when removed from these lists.
     */

    list (lwp_stream_filterspec, filters_upstream);
    list (lwp_stream_filterspec, filters_downstream);


    /* Streams we are a filter for (filter == this) */

    list (lwp_stream_filterspec, filtering);


    /* StreamGraph::Expand sets head_upstream to the head of the expanded
     * first filter, and fills exp_data_hooks with the data hooks this
     * stream is responsible for calling.
     */

    lw_stream head_upstream;

    list (lwp_stream_data_hook, exp_data_hooks);


    /* The front queue is to be written before any more data from the current
     * source stream, and the back queue is to be written after writing from
     * the current source stream has finished.
     */

    list (struct _lwp_stream_queued, front_queue);
    list (struct _lwp_stream_queued, back_queue);


    int retry;

    lwp_streamgraph graph;

    list (lwp_streamgraph_link, prev);
    list (lwp_streamgraph_link, next);

    list (lwp_streamgraph_link, prev_expanded);
    list (lwp_streamgraph_link, next_expanded);

    int last_expand;

    lw_stream prev_direct;
    size_t direct_bytes_left;
};

void lwp_stream_init (lw_stream, const lw_streamdef *, lw_pump);


/* Returns true if this stream should be considered transparent, based on
 * whether the public IsTransparent returns true, no data hooks are
 * registered, and the queue is empty.
 */

 lw_bool lwp_stream_is_transparent (lw_stream);

    
/* Pushes data forward to any streams next in the graph.  buffer may be
 * 0, in which case this is used to indicate the success of a direct write
 */

 void lwp_stream_push (lw_stream, const char * buffer, size_t size);


/* Extended (internal) versions of lw_stream_write* */

#define lwp_stream_write_ignore_filters  1
#define lwp_stream_write_ignore_busy  2
#define lwp_stream_write_ignore_queue  4
#define lwp_stream_write_partial  8
#define lwp_stream_write_delete_stream  16

 void lwp_stream_write_stream
   (lw_stream, lw_stream source, size_t size, int flags);

 size_t lwp_stream_write
   (lw_stream, const char * buffer, size_t size, int flags);


/* Attempts to write data from PrevDirect, returning false on failure. If
 * successful, DirectBytesLeft will be adjusted.
 */

 lw_bool lwp_stream_write_direct (lw_stream);


/* Returns true if this stream is ready to be closed - i.e. nothing is
 * queued or currently being written.
 */

 lw_bool lwp_stream_may_close (lw_stream);


/* Closes this stream.  If immediate is true, the stream will be closed
 * regardless of what MayClose() returns.  If immediate is false and
 * MayClose() returns false, the stream will be closed as soon as all
 * pending data has been written (via CloseASAP flag).
 *
 * Returns true if the stream was closed immediately, false otherwise.
 */

 lw_bool lwp_stream_close (lw_stream, lw_bool immediate);

#endif


