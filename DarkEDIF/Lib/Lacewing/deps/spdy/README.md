
I couldn't find an existing C library for SPDY that was suitable for my project,
so here's my take on one.  

It supports both draft 2 and draft 3.  If the remote endpoint sends the first
control frame, it will use whichever version they use.  If sending the first
control frame itself, the version passed to `spdy_ctx_new` will be used (draft
2 by default).

ANSI C, BSD licensed, requires zlib.

[![Build Status](https://secure.travis-ci.org/udp/spdy.png)](http://travis-ci.org/udp/spdy)


Usage
=====

For a description of each function/callback, refer to the comments in `spdy.h`.

* Each connection should be assigned a `spdy_ctx`, created with `spdy_ctx_new`.
  `spdy_ctx_new` receives a `spdy_config` structure, in which the application
  can set up callback functions.

* When data arrives from the network, `spdy_data` should be called with the
  buffer and size.  `spdy_data` will indicate how many bytes were consumed, so
  the application can calculate how many bytes must be kept and passed again
  later.

* When the library has data to send to the network, the `emit` callback will be
  called with a buffer and size.

As well as `spdy_ctx`, there is a `spdy_stream` structure, of which there may
be multiple instances per `spdy_ctx`.  `spdy_stream` instances are created
either by this side (with `spdy_stream_open`) or by the remote endpoint (first
seen in the `on_stream_create` callback).

To receive incoming headers or data from a `spdy_stream`, set a callback for
`on_stream_headers` or `on_stream_data`.  To send outgoing headers or data to
a `spdy_stream`, use the `spdy_stream_write_headers` and `spdy_stream_write_data`
functions.


Known issues
============

* Draft 3 CREDENTIAL frame not implemented
* Flow control not implemented




