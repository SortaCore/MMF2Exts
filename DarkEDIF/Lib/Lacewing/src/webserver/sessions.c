
/* vim: set et ts=3 sw=3 ft=c:
 *
 * Copyright (C) 2011, 2012 James McLaughlin et al.  All rights reserved.
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

#include "common.h"

const char * const session_cookie = "lw_session";

const char hex [] = "0123456789abcdef";

void lw_ws_req_session_write (lw_ws_req request, const char * key,
                              const char * value)
{
   const char * cookie = lw_ws_req_get_cookie (request, session_cookie);

   lw_ws_session session;

   if (*cookie)
   {   
      HASH_FIND (hh, request->ws->sessions, cookie, strlen (cookie), session);
   }
   else
   {
      session = 0;
   }

   if (!session)
   {
      char session_id [lwp_session_id_length];

      if (!lw_random (session_id, sizeof (session_id)))
      {
         assert (0);
      }

      session = (lw_ws_session) calloc (sizeof (*session), 1);

      for (int i = 0; i < lwp_session_id_length; ++ i)
      {
         session->id [i * 2] = hex [session_id [i] & 0x0F];
         session->id [i * 2 + 1] = hex [(session_id [i] & 0xF0) >> 4];
      }
      
      HASH_ADD_KEYPTR (hh, request->ws->sessions, session->id,
                           strlen (session->id), session);

      lw_ws_req_set_cookie (request, session_cookie, session->id);
   }

   lwp_nvhash_set (&session->data, key, value, lw_true);
}

const char * lw_ws_req_session_read (lw_ws_req request, const char * key)
{
   const char * cookie = lw_ws_req_get_cookie (request, session_cookie);

   if (!*cookie)
      return "";

   lw_ws_session session;
   HASH_FIND (hh, request->ws->sessions, cookie, strlen (cookie), session);

   if (!session)
      return "";

   return lwp_nvhash_get (&session->data, key, "");
}

void lw_ws_session_close (lw_ws ws, const char * id)
{
   lw_ws_session session;
   HASH_FIND (hh, ws->sessions, id, strlen (id), session);

   if (!session)
      return;

   lwp_nvhash_clear (&session->data);
   HASH_DEL (ws->sessions, session);
}

void lw_ws_req_session_close (lw_ws_req request)
{
   lw_ws_session_close (request->ws, lw_ws_req_session_id (request));
}

const char * lw_ws_req_session_id (lw_ws_req request)
{
   return lw_ws_req_get_cookie (request, session_cookie);
}

lw_ws_sessionitem lw_ws_req_session_first (lw_ws_req request)
{
   const char * cookie = lw_ws_req_get_cookie (request, session_cookie);

   if (!*cookie)
      return 0;

   lw_ws_session session;
   HASH_FIND (hh, request->ws->sessions, cookie, strlen (cookie), session);

   if (!session)
      return 0;

   return (lw_ws_sessionitem) session->data;
}

lw_ws_sessionitem lw_ws_sessionitem_next (lw_ws_sessionitem item)
{
   return (lw_ws_sessionitem) ((lwp_nvhash) item)->hh.next;
}

const char * lw_ws_sessionitem_name (lw_ws_sessionitem item)
{
   return ((lwp_nvhash) item)->key;
}

const char * lw_ws_sessionitem_value (lw_ws_sessionitem item)
{
   return ((lwp_nvhash) item)->key;
}

