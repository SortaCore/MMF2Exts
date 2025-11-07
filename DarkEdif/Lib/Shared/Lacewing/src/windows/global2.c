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
#include "compat.h"
#include <IPHlpApi.h> // for NotifyAddrChange
#include <netioapi.h>

static int init_called = 0;
OVERLAPPED lwp_network_changed_overlapped;

static HANDLE networkChanged;

void WINAPI lwp_networkChangedCallback(void* CallerContext, PMIB_IPINTERFACE_ROW row, int type)
{
	if (type != MibDeleteInstance && type != MibAddInstance)
		return;

	// IPv6 address likely expired
	if (type == MibDeleteInstance && (!row || lwp_ipv6_public_fixed_interface_index == row->InterfaceIndex))
	{
		lwp_ipv6_public_fixed_addr = in6addr_any;
		lwp_ipv6_public_fixed_interface_index = -1;
	}
	if (type == MibAddInstance && lwp_ipv6_public_fixed_interface_index < 0)
	{
		// If failed to find IPv6 entirely, cos no IPv6, this will be -2; we might have new adapter and new system now
		if (lwp_ipv6_public_fixed_interface_index == -2)
			lwp_ipv6_public_fixed_interface_index = -1;
		lwp_trigger_public_address_hunt(lw_false);
	}

	lwp_on_network_changed ((lw_network_change_type) type);
}

lw_thread networkLostModeMonitorThread;
int __stdcall xpNetworkLostMode(void* p)
{
	while (1)
	{
		DWORD res = WSAWaitForMultipleEvents(1, &lwp_network_changed_overlapped.hEvent, TRUE, INFINITE, TRUE);
		if (res == WAIT_OBJECT_0)
		{
			lwp_ipv6_public_fixed_addr = in6addr_any;
			lwp_ipv6_public_fixed_interface_index = -1;
			lwp_trigger_public_address_hunt(lw_false);

			lwp_on_network_changed(lw_network_change_type_unspecified);

			// Re-register for an event
			WSAResetEvent(lwp_network_changed_overlapped.hEvent);
			if (NotifyAddrChange(NULL, &lwp_network_changed_overlapped) != ERROR_IO_PENDING)
			{
				always_log("Couldn't register network change handler, error %u.", GetLastError());
				break;
			}
			continue;
		}

		// else assume borked, perhaps deliberately
		break;
	};

	CancelIPChangeNotify(&lwp_network_changed_overlapped);
	WSACloseEvent(&lwp_network_changed_overlapped.hEvent);
	lwp_network_changed_overlapped.hEvent = NULL;
	CloseHandle(networkChanged);
	networkChanged = NULL;

	lwp_on_network_changed(lw_network_change_type_handlergone);
	return -1;
}

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

	lwp_network_change_init ();

	memset(&lwp_network_changed_overlapped, 0, sizeof(lwp_network_changed_overlapped));
	fn_NotifyIpInterfaceChange notif = compat_NotifyIpInterfaceChange();
	DWORD res;
	if (notif)
		res = notif(AF_UNSPEC, &lwp_networkChangedCallback, NULL, FALSE, &lwp_network_changed_overlapped.hEvent);
	else
	{
		lwp_network_changed_overlapped.hEvent = WSACreateEvent();
		res = NotifyAddrChange(&networkChanged, &lwp_network_changed_overlapped);
		if (res == 0 || res == WSA_IO_PENDING)
		{
			networkLostModeMonitorThread = lw_thread_new("Network status monitor", (void*)xpNetworkLostMode);
			lw_thread_start(networkLostModeMonitorThread, NULL);
		}
	}

	if (res != 0 && res != WSA_IO_PENDING)
	{
		always_log("Warning: Network notification could not be registered, error %u. IPv6 may fail after network change.", GetLastError());
		if (!notif)
			WSACloseEvent(lwp_network_changed_overlapped.hEvent);
	}
}

void lwp_deinit()
{
	if (--init_called == 0)
	{
		if (!compat_NotifyIpInterfaceChange())
		{
			CancelIPChangeNotify(&lwp_network_changed_overlapped);
			lw_thread_join(networkLostModeMonitorThread);
			lw_thread_delete(networkLostModeMonitorThread);
		}
		else
			compat_CancelMibChangeNotify2()(&networkChanged);

		lwp_network_change_deinit();

		WSACleanup();
	}
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

