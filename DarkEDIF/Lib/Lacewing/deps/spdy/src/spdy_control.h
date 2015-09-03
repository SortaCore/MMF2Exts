
/* vim: set et ts=3 sw=3 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin et al.  All rights reserved.
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

#ifndef SPDY_CONTROL_FRAME_H
#define SPDY_CONTROL_FRAME_H

#define SPDY_CTRL_HEADER_SIZE 8

#define FLAG_FIN 1
#define FLAG_UNIDIRECTIONAL 2

enum spdy_control_frame
{
  SYN_STREAM = 1, SYN_REPLY = 2, RST_STREAM = 3, SETTINGS = 4,
  NOOP = 5, PING = 6, GOAWAY = 7, HEADERS = 8, WINDOW_UPDATE = 9,
  CREDENTIAL = 10
};

int spdy_proc_control_frame
    (spdy_ctx *, char * buffer, size_t size);

void spdy_build_control_header
    (spdy_ctx *, char * dest, int type, int8_t flags, size_t size);

int spdy_proc_syn_stream      (spdy_ctx *, int8_t flags, spdy_buffer *);
int spdy_proc_syn_reply       (spdy_ctx *, int8_t flags, spdy_buffer *);
int spdy_proc_rst_stream      (spdy_ctx *, int8_t flags, spdy_buffer *);
int spdy_proc_settings        (spdy_ctx *, int8_t flags, spdy_buffer *);
int spdy_proc_ping            (spdy_ctx *, int8_t flags, spdy_buffer *);
int spdy_proc_goaway          (spdy_ctx *, int8_t flags, spdy_buffer *);
int spdy_proc_headers         (spdy_ctx *, int8_t flags, spdy_buffer *);
int spdy_proc_window_update   (spdy_ctx *, int8_t flags, spdy_buffer *);
int spdy_proc_credential      (spdy_ctx *, int8_t flags, spdy_buffer *);

int spdy_emit_syn_stream
  (spdy_ctx *, int8_t flags, int32_t stream_id, int32_t assoc_stream_id,
   int8_t priority, int8_t slot, size_t num_headers, spdy_nv_pair * headers);

int spdy_emit_syn_reply
  (spdy_ctx *, int8_t flags, int32_t stream_id, size_t num_headers, spdy_nv_pair *);

void spdy_emit_rst_stream 
  (spdy_ctx *, int8_t flags, int32_t stream_id, int32_t status_code);

void spdy_emit_settings
  (spdy_ctx *, int8_t flags, spdy_bool returning_persisted, int num, spdy_setting *);

void spdy_emit_ping
  (spdy_ctx *, int8_t flags, int32_t id);

void spdy_emit_goaway
  (spdy_ctx *, int8_t flags, int32_t last_good_stream_id);

int spdy_emit_headers
  (spdy_ctx *, int8_t flags, int32_t stream_id, size_t num_headers, spdy_nv_pair *);

int spdy_emit_window_update
  (spdy_ctx *, int8_t flags, int32_t stream_id, int32_t delta_window_size);

#endif



