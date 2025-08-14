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

static int init_called = 0;

void lwp_init ()
{
	WSADATA winsock_data;

	if (++init_called != 1)
		return;

	int err = WSAStartup(MAKEWORD(2, 2), &winsock_data);
	if (err != 0 || winsock_data.wVersion < MAKEWORD(2, 2))
	{
		always_log("Couldn't startup WinSock v2.2, got error %d, ver %d.%d", err, HIBYTE(winsock_data.wVersion), LOBYTE(winsock_data.wVersion));
		exit(WSAVERNOTSUPPORTED);
		return;
	}
}
void lwp_deinit()
{
	if (--init_called == 0)
		WSACleanup();
}

lw_bool lw_file_exists (const char * filename)
{
#if _UNICODE
	wchar_t * wc = lw_char_to_wchar(filename, -1);
	lw_bool exists = wc ? (GetFileAttributesW(wc) & FILE_ATTRIBUTE_DIRECTORY) == 0 : lw_false;
	free(wc);
	return exists;
#else
	return (GetFileAttributesA (filename) & FILE_ATTRIBUTE_DIRECTORY) == 0;
#endif
}

lw_bool lw_path_exists (const char * filename)
{
	DWORD attr = GetFileAttributesA(filename);
	return attr != 0xFFFFFFFF && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

size_t lw_file_size (const char * filename)
{
	WIN32_FILE_ATTRIBUTE_DATA info;
	LARGE_INTEGER size;

	if (!GetFileAttributesExA (filename, GetFileExInfoStandard, &info))
		return 0;

	size.LowPart = info.nFileSizeLow;
	size.HighPart = info.nFileSizeHigh;

	return (size_t) size.QuadPart;
}

lw_i64 lw_last_modified (const char * filename)
{
	WIN32_FILE_ATTRIBUTE_DATA info;
	LARGE_INTEGER time;

	if (!GetFileAttributesExA (filename, GetFileExInfoStandard, &info))
		return 0;

	time.LowPart = info.ftLastWriteTime.dwLowDateTime;
	time.HighPart = info.ftLastWriteTime.dwHighDateTime;

	return (time_t) ((time.QuadPart - 116444736000000000ULL) / 10000000);
}

void lw_temp_path (char * buffer)
{
	GetTempPathA (lwp_max_path, buffer);
}


static HCRYPTPROV crypt_prov = 0;

static lw_bool crypt_init ()
{
	if (!crypt_prov)
		CryptAcquireContext (&crypt_prov, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);

	return crypt_prov != 0;
}

lw_bool lw_random (char * buffer, size_t size)
{
	if (!crypt_init ())
		return lw_false;

	if (size > 0xFFFFFFFF)
		return lw_false;

	if (!CryptGenRandom (crypt_prov, (DWORD) size, (PBYTE) buffer))
		return lw_false;

	return lw_true;
}

size_t lw_min_size_t(size_t a, size_t b)
{
	return a < b ? a : b;
}

void lw_md5 (char * output, const char * input, size_t length)
{
	HCRYPTPROV hash_prov;
	DWORD hash_length = 16;

	crypt_init ();

	CryptCreateHash (crypt_prov, CALG_MD5, 0, 0, &hash_prov);
	CryptHashData (hash_prov, (BYTE *) input, (DWORD) length, 0);
	CryptGetHashParam (hash_prov, HP_HASHVAL, (BYTE *) output, &hash_length, 0);
	CryptDestroyHash (hash_prov);
}

void lw_sha1 (char * output, const char * input, size_t length)
{
	HCRYPTPROV hash_prov;
	DWORD hash_length = 20;

	crypt_init ();

	CryptCreateHash (crypt_prov, CALG_SHA1, 0, 0, &hash_prov);
	CryptHashData (hash_prov, (BYTE *) input, (DWORD) length, 0);
	CryptGetHashParam (hash_prov, HP_HASHVAL, (BYTE *) output, &hash_length, 0);
	CryptDestroyHash (hash_prov);
}

