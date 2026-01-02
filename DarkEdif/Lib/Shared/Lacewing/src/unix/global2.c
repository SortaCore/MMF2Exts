/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012, 2013 James McLaughlin et al.
 * Copyright (C) 2012-2026 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../common.h"

static lw_i32 init_called = 0;
// If true, this server is running under WSL.
// @remarks It doesn't matter if this is faked, as it's only used to fix a WSL host Windows -> Linux server UDP routing problem.
lw_bool lw_in_wsl = lw_false;

// IPv6 mapped IPv4 localhost address, seen in WSL host Windows connecting to WSL server.
// Text translation: [::ffff:127.0.0.1].
// @remarks WSL can't route UDP to host properly if you reply via sendmsg() using fixed local IP.
// sendto() still works for 127.0.0.1.
// However, sendto() fails too if you connect from host with other 127 variants like 127.0.0.2.
const struct in6_addr in6addr_loopback_wsl = { { { 0,0,0,0,0,0,0,0,0,0,255,255,127,0,0,1 } } };

void lwp_init ()
{
	if (++init_called != 1)
		return;

	#ifdef ENABLE_SSL

		STACK_OF (SSL_COMP) * comp_methods;

		SSL_library_init ();

		SSL_load_error_strings ();
		ERR_load_crypto_strings ();

		comp_methods = SSL_COMP_get_compression_methods ();
		sk_SSL_COMP_zero (comp_methods);

	#endif

	// WSL requires UDP workarounds, see lw_udp_send()
	lw_in_wsl = lw_file_exists("/proc/sys/fs/binfmt_misc/WSLInterop");
}
void lwp_deinit()
{
	// There is no SSL de-init for OpenSSL on Unix; it happens automatically
}


lw_bool lw_file_exists (const char * filename)
{
	struct stat attr;

	if (stat(filename, &attr) == 0)
		return !S_ISDIR(attr.st_mode);

	lwp_trace("%s stat failed, errno %d: %s", filename, errno, strerror(errno));
	return lw_false;
}

lw_bool lw_path_exists (const char * filename)
{
	struct stat attr;

	if (stat (filename, &attr) == 0)
		return S_ISDIR (attr.st_mode);

	return lw_false;
}

size_t lw_file_size (const char * filename)
{
	struct stat attr;

	if (stat (filename, &attr) == 0)
		return (size_t)attr.st_size;

	return 0;
}

lw_i64 lw_last_modified (const char * filename)
{
	struct stat attr;

	if (stat (filename, &attr) == 0)
		return attr.st_mtime;

	return 0;
}

void lw_temp_path (char * buffer)
{
	char * path = getenv ("TMPDIR");

	if (path)
		strcpy (buffer, path);
	else
#ifdef P_tmpdir
		strcpy (buffer, P_tmpdir);
#else
		strcpy (buffer, "/tmp/");
#endif

	if (*buffer && buffer [strlen (buffer) - 1] != '/')
		strcat (buffer, "/");

	return;
}

lw_bool lw_random (char * buffer, size_t size)
{
	static int dev_random = -1;

	if (dev_random == -1)
		dev_random = open ("/dev/urandom", O_RDONLY);

	if (dev_random == -1)
	{
		lwp_trace ("Error opening random: %s", strerror (errno));
		return lw_false;
	}

	if (read (dev_random, buffer, size) != (ssize_t)size)
	{
		lwp_trace ("Error reading from random: %s", strerror (errno));
		return lw_false;
	}

	return lw_true;
}

size_t lw_min_size_t(size_t a, size_t b)
{
	return a < b ? a : b;
}

#ifdef ENABLE_SSL

 void lw_md5 (char * output, const char * input, size_t length)
 {
	MD5_CTX context;

	MD5_Init (&context);
	MD5_Update (&context, input, length);
	MD5_Final ((unsigned char *) output, &context);
 }

 void lw_sha1 (char * output, const char * input, size_t length)
 {
	SHA_CTX context;

	SHA1_Init (&context);
	SHA1_Update (&context, input, length);
	SHA1_Final ((unsigned char *) output, &context);
 }

#endif

