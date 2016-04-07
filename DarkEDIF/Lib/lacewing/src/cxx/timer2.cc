
/* vim: set et ts=3 sw=3 ft=cpp:
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

#include "../common.h"

timer lacewing::timer_new (lacewing::pump pump)
{
   return (timer) lw_timer_new ((lw_pump) pump);
}

void lacewing::timer_delete (lacewing::timer timer)
{
   lw_timer_delete ((lw_timer) timer);
}

void _timer::start (long msec)
{
   lw_timer_start ((lw_timer) this, msec);
}

void _timer::stop ()
{
   lw_timer_stop ((lw_timer) this);
}

bool _timer::started ()
{
   return lw_timer_started ((lw_timer) this);
}

void _timer::force_tick ()
{
   lw_timer_force_tick ((lw_timer) this);
}

void _timer::on_tick (_timer::hook_tick on_tick)
{
   lw_timer_on_tick ((lw_timer) this, (lw_timer_hook_tick) on_tick);
}

void * _timer::tag ()
{
   return lw_timer_tag ((lw_timer) this);
}

void _timer::tag (void * tag)
{
   lw_timer_set_tag ((lw_timer) this, tag);
}

