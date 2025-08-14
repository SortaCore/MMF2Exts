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

struct _lw_file
{
	struct _lw_fdstream fdstream;

	char name [lwp_max_path];
};

void lwp_file_init (lw_file ctx, lw_pump pump)
{
	*ctx->name = 0;

	lwp_fdstream_init (&ctx->fdstream, pump);
}

lw_file lw_file_new (lw_pump pump)
{
	lw_file ctx = (lw_file)malloc (sizeof (*ctx));
	if (!ctx)
		return NULL;
	lwp_file_init (ctx, pump);

	return ctx;
}

lw_file lw_file_new_open (lw_pump pump,
						  const char * filename,
						  const char * mode)
{
	lw_file ctx = lw_file_new (pump);

	if (!ctx)
		return 0;

	if (!lw_file_open (ctx, filename, mode))
	{
		lw_stream_delete ((lw_stream) ctx);
		return 0;
	}

	return ctx;
}

static int get_flags (const char * mode)
{
	/* Based on what FreeBSD does to convert the mode string for fopen(3) */

	int flags;

	switch (*mode ++)
	{
		case 'r':
			flags = O_RDONLY;
			break;

		case 'w':
			flags = O_WRONLY | O_CREAT | O_TRUNC;
			break;

		case 'a':
			flags = O_WRONLY | O_CREAT | O_APPEND;
			break;

		default:
			return -1;
	};

	if (*mode == 'b')
		++ mode;

	if (*mode == '+')
	{
		flags |= O_RDWR;
		++ mode;
	}

	if (*mode == 'b')
		++ mode;

	if (*mode == 'x')
	{
		if (flags & O_RDONLY)
			return -1;

		flags |= O_EXCL;
	}

	return flags;
}

lw_bool lw_file_open (lw_file ctx,
					  const char * filename,
					  const char * mode)
{
	lwp_trace ("%p : lw_file_open \"%s\", \"%s\"", ctx, filename, mode);

	*ctx->name = 0;

	int flags = get_flags (mode);

	if (flags == -1)
	{
		lwp_trace ("Error parsing mode");
		return lw_false;
	}

	int fd = open (filename, flags, S_IRWXU);

	if (fd == -1)
	{
		lwp_trace ("open() failed: %s", strerror (errno));
		return lw_false;
	}

	lw_fdstream_set_fd ((lw_fdstream) ctx, fd, 0, lw_true, lw_false);

	if (lw_fdstream_valid ((lw_fdstream) ctx))
	{
		strcpy (ctx->name, filename);
		return lw_true;
	}

	lwp_trace ("valid() returned false");

	return lw_false;
}

lw_bool lw_file_open_temp (lw_file ctx)
{
	char name [lwp_max_path];
	char random [8];
	size_t i = 0;

	lw_temp_path (name);
	lw_random (random, sizeof (random));

	while (i < sizeof (random))
	{
		sprintf (name + strlen (name), "%02x", random [i]);
		++ i;
	}

	lwp_trace ("Opening temp file: %s", name);

	return lw_file_open (ctx, name, "wb");
}

const char * lw_file_name (lw_file ctx)
{
	return ctx->name;
}

