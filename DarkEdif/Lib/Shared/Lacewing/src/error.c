/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012, 2013 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "common.h"

struct _lw_error
{
	char buffer [4096];
	char * begin;

	size_t size;

	void * tag;
};

static void lwp_error_add (lw_error ctx, const char * buffer)
{
	size_t length = strlen (buffer);

	if ((ctx->begin - length) < ctx->buffer)
		return;

	ctx->begin -= length;
	memcpy (ctx->begin, buffer, length);
}

void lw_error_addv (lw_error ctx, const char * format, va_list args)
{
	++ ctx->size;

	if (*ctx->begin)
		lwp_error_add (ctx, " - ");

	char * buffer = (char *) lw_malloc_or_exit (sizeof (ctx->buffer) + 1);

	assert(format != NULL);
	vsnprintf (buffer, sizeof (ctx->buffer), format, args);
	lwp_error_add (ctx, buffer);

	free (buffer);
}

lw_error lw_error_new ()
{
	lw_error ctx = (lw_error) malloc (sizeof (*ctx));

	if (!ctx)
		return 0;

	*(ctx->begin = ctx->buffer + sizeof (ctx->buffer) - 1) = 0;

	ctx->size = 0;
	ctx->tag = NULL;

	return ctx;
}

void lw_error_delete (lw_error ctx)
{
	if (!ctx)
		return;

	free (ctx);
}

const char * lw_error_tostring (lw_error ctx)
{
	return ctx->begin;
}

lw_error lw_error_clone (lw_error ctx)
{
	lw_error error = (lw_error) malloc (sizeof (*error));

	if (!error)
		return 0;

	memcpy (error, ctx, sizeof (*error));

	return error;
}

void lw_error_addf (lw_error ctx, const char * format, ...)
{
	va_list args;
	va_start (args, format);

	lw_error_addv (ctx, format, args);

	va_end (args);
}

void lw_error_add (lw_error ctx, int error)
{
	#ifdef _WIN32

		TCHAR message[512];

		if (FormatMessage
			(FORMAT_MESSAGE_FROM_SYSTEM,
				0,
				error,
				MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
				message,
				sizeof(message) / sizeof(*message),
				0))
		{
			// Sometimes FormatMessage adds a newline or punctuation
			int size = (int)_tcslen(message);
			char* message2 = (char *)message;
			if (message[size - 1] == _T('\n'))
				--size;
			if (message[size - 1] == _T('\r'))
				--size;
			if (message[size - 1] == _T('.'))
				--size;

			// In Unicode, convert from UTF-16 to UTF-8 for lw_error_addf
#ifdef _UNICODE
			char utf8Str[512];
			size = WideCharToMultiByte(CP_UTF8, 0, message, size, utf8Str, sizeof(utf8Str), NULL, NULL);
			if (size <= 0)
			{
				lw_error_addf(ctx, "(error %u printing error message)", GetLastError());
				utf8Str[0] = '\0';
			}
			message2 = utf8Str;
#endif
			lw_error_addf (ctx, error < 0 ? "%.*s (%08X)" : "%.*s (%d)",
				size, message2, error);
		}

	#else

		lw_error_addf (ctx, "%s (%ld)", strerror (error), error);

	#endif
}

size_t lw_error_size (lw_error ctx)
{
	return ctx->size;
}

void lw_error_set_tag (lw_error ctx, void * tag)
{
	ctx->tag = tag;
}

void * lw_error_tag (lw_error ctx)
{
	return ctx->tag;
}
