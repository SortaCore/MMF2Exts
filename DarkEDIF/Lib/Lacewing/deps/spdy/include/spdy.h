
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

#ifndef SPDY_H
#define SPDY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <sys/uio.h>

typedef int spdy_bool;


/* Error codes */

enum spdy_error_t
{
   SPDY_E_OK,                  /* no error */
   SPDY_E_VERSION_MISMATCH,    /* wrong version */
   SPDY_E_PROTOCOL,            /* we received some garbage */
   SPDY_E_MEM,                 /* memory allocation failure */
   SPDY_E_THRESHOLD,           /* error threshold reached */
   SPDY_E_INFLATE,             /* error w/ zlib inflate */
   SPDY_E_DEFLATE,             /* error w/ zlib deflate */
   SPDY_E_PARAM,               /* bad param */
   SPDY_E_SESSION_CLOSED       /* session closed */
};

const char * spdy_error_string (int status_code);


/* Status codes for on_stream_close/spdy_stream_close */

enum spdy_status_t
{
   SPDY_STATUS_PROTOCOL_ERROR          = 1,
   SPDY_STATUS_INVALID_STREAM          = 2,
   SPDY_STATUS_REFUSED_STREAM          = 3,
   SPDY_STATUS_UNSUPPORTED_VERSION     = 4,
   SPDY_STATUS_CANCEL                  = 5,
   SPDY_STATUS_INTERNAL_ERROR          = 6,
   SPDY_STATUS_FLOW_CONTROL_ERROR      = 7, /* last for draft 2 */
   SPDY_STATUS_STREAM_IN_USE           = 8, 
   SPDY_STATUS_STREAM_ALREADY_CLOSED   = 9,
   SPDY_STATUS_INVALID_CREDENTIALS     = 10,
   SPDY_STATUS_FRAME_TOO_LARGE         = 11 
};

const char * spdy_status_string (int status_code);


/* GOAWAY status codes */

enum spdy_goaway_status_t
{
   SPDY_GOAWAY_OK                = 0,
   SPDY_GOAWAY_PROTOCOL_ERROR    = 1,
   SPDY_GOAWAY_INTERNAL_ERROR    = 11
};

const char * spdy_goaway_status_string (int status_code);


/* Settings */

enum spdy_setting_t
{
   SPDY_SETTINGS_UPLOAD_BANDWIDTH                = 1,
   SPDY_SETTINGS_DOWNLOAD_BANDWIDTH              = 2,
   SPDY_SETTINGS_ROUND_TRIP_TIME                 = 3,
   SPDY_SETTINGS_MAX_CONCURRENT_STREAMS          = 4,
   SPDY_SETTINGS_CURRENT_CWND                    = 5,
   SPDY_SETTINGS_DOWNLOAD_RETRANS_RATE           = 6,
   SPDY_SETTINGS_INITIAL_WINDOW_SIZE             = 7, /* last for draft 2 */
   SPDY_SETTINGS_CLIENT_CERTIFICATE_VECTOR_SIZE  = 8,

   SPDY_NUM_SETTINGS = 9
};

typedef struct _spdy_setting
{
   spdy_bool persistant;

   long id, value;

} spdy_setting;


/* This flag may be passed to spdy_stream_open or spdy_stream_write_data.
 * It indicates that we do not intend to send any more data on the stream, and
 * the remote endpoint should consider it closed on our end.
 */

#define SPDY_FLAG_FIN 1


/* This flag may be passed to spdy_stream_open.
 * It indicates that we do not expect any incoming data on the stream, and the
 * remote endpoint should consider it closed on their end.
 */

#define SPDY_FLAG_UNIDIRECTIONAL 2


typedef struct _spdy_ctx spdy_ctx;
typedef struct _spdy_stream spdy_stream;


/* Functions to attach a custom pointer to a spdy_ctx or spdy_stream.
 * Useful for adding custom state or ferrying application context pointers to
 * the callbacks.
 */

void spdy_ctx_set_tag (spdy_ctx *, void * tag);
void * spdy_ctx_get_tag (spdy_ctx *);

void spdy_stream_set_tag (spdy_stream *, void * tag);
void * spdy_stream_get_tag (spdy_stream *);


/* Used to represent a name/value pair throughout the API. */

typedef struct _spdy_nv_pair
{
   size_t name_len;
   char * name;

   size_t value_len;
   char * value; /* may contain multiple values separated by 0 octets */

} spdy_nv_pair;


/* Used in the optional emitv callback to represent each buffer. */

#ifndef _WIN32
    #include <sys/uio.h>
    typedef struct iovec spdy_iovec;
#else
    typedef struct spdy_iovec
    {
       void * iov_base;
       size_t iov_len;
    } spdy_iovec;
#endif


typedef struct _spdy_config
{
   /* Set to non-zero to have this SPDY context act as a server, or zero to
    * have it act as a client.
    */

   spdy_bool is_server;


   /* The number of non-fatal errors that may occur before the fatal
    * SPDY_E_THRESHOLD will be returned and the connection should be
    * terminated.  Set to -1 to allow unlimited errors.
    */

   int error_threshold;


   /* Called when data is ready to be sent across the wire.
    */

   void (* emit) (spdy_ctx *, const char * buffer, size_t size);


   /* Called to send a sequence of buffers across the wire.
    *
    * emitv is an optional callback.  If emitv is not implemented, the library
    * will simply call emit multiple times.
    */

   void (* emitv) (spdy_ctx *, size_t num, spdy_iovec *);
                   

   /* Called when new settings have arrived.  If clear_persisted is non-zero,
    * the application must clear any previously persisted settings before
    * storing the new ones.
    */

   void (* on_settings_changed) (spdy_ctx * ctx, spdy_bool clear_persisted,
                                 int num_changed, spdy_setting * changed);


   /* Called when a new stream has been created.  This may be a stream
    * initiated by the remote endpoint, or it may be an acknowledgement
    * of one created by us with spdy_stream_open.
    *
    * spdy_stream_is_ours may be used to detect if the stream was initiated
    * by us or the remote endpoint.
    */

   void (* on_stream_create)
      (spdy_ctx *, spdy_stream *, size_t num_headers, spdy_nv_pair * headers);


   /* Called when new headers have arrived on a stream after creation.
    */

   void (* on_stream_headers)
      (spdy_ctx *, spdy_stream *, size_t num_headers, spdy_nv_pair * headers);


   /* Called when data has arrived on a stream.
    */

   void (* on_stream_data)
      (spdy_ctx *, spdy_stream *, const char * data, size_t size);


   /* Called when a stream has been closed.  If the status code is != 0,
    * the stream was terminated by the remote endpoint.  Otherwise, the
    * stream was closed normally by becoming half-closed on both ends.
    */

   void (* on_stream_close) (spdy_ctx *, spdy_stream *, int status_code); 

} spdy_config;


/* One spdy_ctx should be created for each physical connection.
 * Any settings persisted from a previous session should be passed here (see
 * the on_settings callback).
 *
 * The version parameter may be 2, 3 or 0 for the default (currently 2).  If
 * the remote endpoint is the first to send a control frame and it has a
 * different version, it will override the version given here.
 */

spdy_ctx * spdy_ctx_new (const spdy_config *, int version,
                         int num_persisted_settings,
                         spdy_setting * persisted_settings);

void spdy_ctx_delete (spdy_ctx *);


/* Call this when data has arrived on the physical connection.
 * 
 * After this function returns, *bytes will contain the number of bytes from
 * the beginning of the buffer that were successfully processed and should not
 * be passed again.  Anything left over should be retained by the application
 * and passed again when more data is received, with the new data appended.
 */

int spdy_data (spdy_ctx *, const char * buffer, size_t * bytes);


/* Returns the currently active version for a spdy_ctx.  This may be different
 * from the version passed to spdy_ctx_new.
 */

int spdy_active_version (spdy_ctx *);


/* Updates setting(s) for a spdy_ctx.
 *
 * If should_clear is non-zero and we are the server, the remote endpoint will
 * be requested to clear any previously persisted settings.
 */

void spdy_update_settings (spdy_ctx *, spdy_bool should_clear, int num_changed,
                           spdy_setting * changed);


/* Opens a new, independent stream.  When the stream is acknowledged by the
 * remote endpoint, on_stream_create will be called.
 *
 * The stream may be associated with an existing, open stream.  This is
 * mandatory with HTTP server push, when a server is pushing additional
 * resources (in that case, SPDY_FLAG_UNIDIRECTIONAL must also be set).
 *
 * flags may optionally contain SPDY_FLAG_FIN and/or SPDY_FLAG_UNIDIRECTIONAL.
 */

int spdy_stream_open (spdy_ctx *, spdy_stream ** stream, spdy_stream * assoc,
                      int flags, int priority, int cred_slot,
                      size_t num_headers, spdy_nv_pair * headers);


/* Returns the stream a stream is associated with or null.
 */

spdy_stream * spdy_stream_get_assoc (spdy_stream *);


/* Returns non-zero if the stream was initiated by us, or zero if the stream
 * was initiated by the remote endpoint.
 */

spdy_bool spdy_stream_is_ours (spdy_stream *);


/* Returns non-zero if the stream is currently open for us to send on, or zero
 * if it is half-closed to us.
 */

spdy_bool spdy_stream_open_here (spdy_stream *);


/* Returns non-zero if the stream is currently open for the remote endpoint
 * to send on, or zero if it is half-closed to them.
 */

spdy_bool spdy_stream_open_remote (spdy_stream *);


/* Writes headers to a stream.  If called within on_stream_create for a stream
 * initiated by the remote endpoint, they will be sent as part of the
 * acknowledgement SYN_REPLY frame.  Otherwise, they will be sent as an
 * independent HEADERS frame.
 *
 * flags may optionally contain SPDY_FLAG_FIN.
 */

int spdy_stream_write_headers
      (spdy_stream *, size_t num_headers, spdy_nv_pair * headers, int flags);


/* Writes data to a stream.  flags may optionally contain SPDY_FLAG_FIN, and it
 * is valid to send a 0 length data frame with SPDY_FLAG_FIN to half-close a
 * stream without any new data.
 */

void spdy_stream_write_data
      (spdy_stream *, const char * data, size_t size, int flags);


/* Closes a stream with immediate effect.  The stream pointer will no longer
 * be valid.
 */

void spdy_stream_close (spdy_stream *, int status_code);


#ifdef __cplusplus
}
#endif

#endif

