/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin et al.
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
	struct _lw_fdstream ctx;

	char name [lwp_max_path];
};

lw_file lw_file_new (lw_pump pump)
{
	lw_file ctx = (lw_file) malloc (sizeof (*ctx));

	if (!ctx)
	  return 0;

	*ctx->name = 0;

	lwp_fdstream_init ((lw_fdstream) ctx, pump);

	return ctx;
}

lw_file lw_file_new_open (lw_pump pump, const char * filename, const char * mode)
{
	lw_file ctx = lw_file_new (pump);

	if (!ctx)
	  return 0;

	lw_file_open (ctx, filename, mode);

	return ctx;
}

lw_bool lw_file_open (lw_file ctx, const char * filename, const char * mode)
{
	*ctx->name = 0;

	lw_fdstream_set_fd ((lw_fdstream) ctx, INVALID_HANDLE_VALUE, 0, lw_true, lw_false);

	DWORD dwDesiredAccess, dwShareMode, dwCreationDisposition;

	switch (*mode ++)
	{
	  case 'r':
		 dwDesiredAccess = GENERIC_READ;
		 dwCreationDisposition = OPEN_EXISTING;
		 dwShareMode = FILE_SHARE_READ;
		 break;

	  case 'w':
		 dwDesiredAccess = GENERIC_WRITE;
		 dwCreationDisposition = CREATE_ALWAYS;
		 dwShareMode = 0;
		 break;

	  case 'a':
		 dwDesiredAccess = GENERIC_WRITE;
		 dwCreationDisposition = OPEN_ALWAYS;
		 dwShareMode = 0;
		 break;

	  default:
		 return lw_false;
	};

	if (*mode == 'b')
	  ++ mode;

	if (*mode == '+')
	{
	  dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
	  ++ mode;
	}

	if (*mode == 'b')
	  ++ mode;

	if (*mode == 'x')
	{
	  if (dwDesiredAccess == GENERIC_READ)
		 return lw_false;

	  dwCreationDisposition = CREATE_NEW;
	}

	HANDLE fd = INVALID_HANDLE_VALUE;
	const void * filename2 = filename;
#if defined(_WIN32) && defined(_UNICODE)
	filename2 = lw_char_to_wchar(filename, -1);
	if (filename != NULL)
	{
#endif
		fd = CreateFile ((LPCTSTR)filename,
						 dwDesiredAccess,
						 dwShareMode,
						 0,
						 dwCreationDisposition,
						 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
						 0);
#if defined(_WIN32) && defined(_UNICODE)
		free((void *)filename2);
	}
#endif

	if (fd == INVALID_HANDLE_VALUE)
	  return lw_false;

	lw_fdstream_set_fd ((lw_fdstream) ctx, fd, 0, lw_true, lw_false);

	if (lw_fdstream_valid ((lw_fdstream) ctx))
	{
	  strcpy (ctx->name, filename);
	  return lw_true;
	}

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

