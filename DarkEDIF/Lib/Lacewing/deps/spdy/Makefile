
CFLAGS = -std=c89 -ansi -pedantic -Wall

OBJECTS = src/spdy_bytes.o src/spdy_control.o src/spdy_credential.o src/spdy_ctx.o \
		  src/spdy_goaway.o src/spdy_headers.o src/spdy_nv_block.o src/spdy_ping.o \
		  src/spdy_rst_stream.o src/spdy_settings.o src/spdy_stream.o src/spdy_syn_reply.o \
		  src/spdy_syn_stream.o src/spdy_zlib.o src/spdy_strings.o src/spdy_window_update.o

all: libspdy.a

libspdy.a: $(OBJECTS)
	ar rcs $@ $(OBJECTS)

.c.o:
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -f $(OBJECTS) libspdy.a

.PHONY: all clean
.SUFFIXES: .c .o .h

