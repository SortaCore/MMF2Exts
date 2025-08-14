/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012, 2013 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "common.h"

static char version [64] = { 0 };

const char * lw_version ()
{
	const char * platform;

	if (!*version)
	{
		#if defined (_WIN32)
		#ifdef _UNICODE
			#define LWS_UNI "Unicode "
		#else
			#define LWS_UNI "ANSI "
		#endif
			#ifdef _WIN64
				platform = "Windows " LWS_UNI "x64";
			#elif defined(_M_ARM64) // Soon(tm)
				platform = "Windows " LWS_UNI "ARM64";
			#else
				platform = "Windows " LWS_UNI "x86";
			#endif
		#undef LWS_UNI
		#elif defined (__ANDROID__)
			#ifdef __aarch64__
				platform = "Android ARM64";
			#elif defined(__ARM_ARCH_7A__)
				#ifdef __ARM_NEON__
					platform = "Android ARMv7/NEON";
				#else
					platform = "Android ARMv7";
				#endif
			#elif defined(__arm__)
				platform = "Android ARMv5";
			#elif defined(__i386__)
				platform = "Android x86";
			#elif defined(__x86_64__)
				platform = "Android x64";
			#else
				#error ABI not known, please amend code
			#endif
		#elif defined(__APPLE__)
			#if MacBuild
				#define prefix "Mac "
			#else
				#define prefix "iOS "
			#endif
			#ifdef __arm64e__
				platform = prefix "ARM64e";
			#elif defined(__aarch64__)
				platform = prefix "ARM64";
			#elif defined(__ARM_ARCH_6__)
				platform = prefix "armv6";
			#elif defined(__ARM_ARCH_7S__)
				platform = prefix "armv7s";
			#elif defined(__ARM_ARCH_7A__)
				platform = prefix "armv7";
			#elif defined(__i386__)
				platform = prefix "i386";
			#elif defined(__x86_64__)
				platform = prefix "x86_64";
			#else
				#error ABI not known, please amend code
			#endif
			#undef prefix
		#else
			struct utsname name;
			uname (&name);
			platform = name.sysname;
		#endif

		#ifndef PACKAGE_VERSION
			#define PACKAGE_VERSION "0.5.4"
		#endif

		sprintf (version, "liblacewing " PACKAGE_VERSION " (%s, %d-bit)",
						platform, ((int) sizeof(void *)) * 8);
	}

	return version;
}

const char * const lwp_weekdays [] =
	{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

const char * const lwp_months [] =
	{ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

void lw_dump (const char * buffer, size_t size)
{
	const size_t bytes_per_row = 8;
	const size_t text_offset = bytes_per_row * 3 + 8;

	unsigned char b;
	size_t i;
	size_t row_offset = 0, row_offset_c = 0, row = 0;

	if (size == SIZE_MAX)
		size = (lw_ui32) strlen (buffer);

	fprintf (stderr, "=== " lwp_fmt_size " bytes @ %p ===\n", size, buffer);

	while (size > 0)
	{
		i = row * bytes_per_row + row_offset;

		if (i >= size)
		{
			if (row_offset_c >= text_offset) /* printing text? */
				break;
			else
				row_offset = bytes_per_row; /* skip to printing text */
		}
		else
		{
			b = *(unsigned char *)&buffer [i];

			row_offset_c += row_offset_c >= text_offset
					? (size_t)(isprint (b) ? fprintf (stderr, "%c", b) : fprintf (stderr, ".") )
					: (size_t)fprintf (stderr, "%02hX ", (short) b);
		}

		if ((++ row_offset) >= bytes_per_row)
		{
			row_offset = 0;

			if (row_offset_c < text_offset)
			{
				while (row_offset_c < text_offset)
					row_offset_c += (size_t)fprintf (stderr, " ");
			}
			else
			{
				row_offset = row_offset_c = 0;
				++ row;

				fprintf (stderr, "\n");
			}
		}
	}

	fprintf (stderr, "\n===\n");
}

#ifdef ENABLE_SSL

 void lw_md5_hex (char * output, const char * input, size_t length)
 {
	 char hex [48];
	 int i = 0;

	 lw_md5 (output, input, length);

	 while (i < 16)
	 {
		 sprintf (hex + (i * 2), "%02x", ((unsigned char *) output) [i]);
		 ++ i;
	 }

	 strcpy (output, hex);
 }

 void lw_sha1_hex (char * output, const char * input, size_t length)
 {
	 char hex [48];
	 int i = 0;

	 lw_sha1 (output, input, length);

	 while (i < 20)
	 {
		 sprintf (hex + (i * 2), "%02x", ((unsigned char *) output) [i]);
		 ++ i;
	 }

	 strcpy (output, hex);
 }

#endif

#ifdef _lacewing_debug

lw_sync lw_trace_sync = 0;
void lw_trace (const char * format, ...)
{
	va_list args;
	char * data;
	ssize_t size;

	va_start (args, format);

	size = lwp_format (&data, format, args);

	if (size > 0)
	{
		if (!lw_trace_sync)
			lw_trace_sync = lw_sync_new ();

		lw_sync_lock (lw_trace_sync);

		#ifdef __ANDROID__
			__android_log_write (ANDROID_LOG_INFO, "liblacewing", data);
		#elif defined(COXSDK) && !defined(__APPLE__)
			OutputDebugStringA (data);
			OutputDebugStringA ("\n");
		#else
			fprintf (stderr, "[liblacewing] %s\n", data);
			fflush (stderr);
		#endif

		free (data);

		lw_sync_release (lw_trace_sync);
	}
	else if (data)
		free(data);

	va_end (args);
}


void lwp_refcount_log_retain(struct lwp_refcount * refcount)
{
	lw_trace ("refcount: %p %s count = %d, retain",
				refcount,
				refcount->name,
				(int) refcount->refcount);
}

void lwp_refcount_log_release(struct lwp_refcount * refcount)
{
	lw_trace ("refcount: %p %s count = %d, release",
				refcount,
				refcount->name,
				(int) refcount->refcount);
}
#else
void lw_trace(const char * format, ...)
{
	// Do nothing
}
#endif


