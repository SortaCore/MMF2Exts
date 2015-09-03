
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

#include "../include/spdy.h"

const char * spdy_status_string (int status_code)
{
    switch (status_code)
    {
    case SPDY_STATUS_PROTOCOL_ERROR:
        return "PROTOCOL_ERROR";
    case SPDY_STATUS_INVALID_STREAM:
        return "INVALID_STREAM";
    case SPDY_STATUS_REFUSED_STREAM:
        return "REFUSED_STREAM";
    case SPDY_STATUS_UNSUPPORTED_VERSION:
        return "UNSUPPORTED_VERSION";
    case SPDY_STATUS_CANCEL:
        return "CANCEL";
    case SPDY_STATUS_INTERNAL_ERROR:
        return "INTERNAL_ERROR";
    case SPDY_STATUS_FLOW_CONTROL_ERROR:
        return "FLOW_CONTROL_ERROR";
    case SPDY_STATUS_STREAM_IN_USE:
        return "STREAM_IN_USE";
    case SPDY_STATUS_STREAM_ALREADY_CLOSED:
        return "STREAM_ALREADY_CLOSED";
    case SPDY_STATUS_INVALID_CREDENTIALS:
        return "INVALID_CREDENTIALS";
    case SPDY_STATUS_FRAME_TOO_LARGE:
        return "FRAME_TOO_LARGE";
    default:
        return "";
    };
}

const char * spdy_goaway_status_string (int status_code)
{
    switch (status_code)
    {
       case SPDY_GOAWAY_OK:
        return "SPDY_GOAWAY_OK";
       case SPDY_GOAWAY_PROTOCOL_ERROR:
        return "SPDY_GOAWAY_PROTOCOL_ERROR";
       case SPDY_GOAWAY_INTERNAL_ERROR:
        return "SPDY_GOAWAY_INTERNAL_ERROR";
    default:
        return "";
    };
}

const char * spdy_error_string (int code)
{
    switch (code)
    {
    case SPDY_E_OK:
       return "No error";
    case SPDY_E_VERSION_MISMATCH:
       return "The remote endpoint uses an unsupported protocol version";
    case SPDY_E_PROTOCOL:
       return "Protocol error";
    case SPDY_E_MEM:
       return "Memory allocation failed";
    case SPDY_E_THRESHOLD:
       return "Error threshold reached";
    case SPDY_E_INFLATE:
       return "zlib inflate error";
    case SPDY_E_DEFLATE:
       return "zlib deflate error";
    case SPDY_E_PARAM:
       return "Bad parameter";
    case SPDY_E_SESSION_CLOSED:
       return "Session closed";
    default:
        return "";
    };
}

