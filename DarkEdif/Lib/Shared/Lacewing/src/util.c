/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "common.h"

void lwp_make_nonblocking(lwp_socket fd)
{
#ifndef _WIN32
	int orig = fcntl(fd, F_GETFL, 0);
	int e = errno;
	(void)e; // hide unused warnings
	assert(orig != -1);
	// Don't set to non-blocking if it is already. Gets OS upset.
	if (orig & O_NONBLOCK)
		lw_trace("Not setting socket/FD %d to non-blocking, already set to that.", fd);
	else
	{
		int newVal = fcntl(fd, F_SETFL, orig | O_NONBLOCK);
		(void)newVal;
		e = errno;
		assert(newVal != -1);
	}
#endif
}
void lwp_setsockopt2(lwp_socket fd, int level, int option, const char * optName, const char * value, socklen_t value_length)
{
	if (setsockopt(fd, level, option, value, value_length) != 0)
	{
		always_log("setsockopt for option %s failed with error %d, continuing", optName, errno);
	}
}

void lwp_disable_ipv6_only (lwp_socket socket)
{
	int no = 0;
	lwp_setsockopt(socket, IPPROTO_IPV6, IPV6_V6ONLY, (char *) &no, sizeof (no));
}

struct sockaddr_storage lwp_socket_addr (lwp_socket socket)
{
	struct sockaddr_storage addr;
	socklen_t addr_len;

	memset (&addr, 0, sizeof (addr));

	if (socket != -1)
	{
		addr_len = sizeof (addr);
		getsockname (socket, (struct sockaddr *) &addr, &addr_len);
	}

	return addr;
}

lw_ui16 lwp_socket_port (lwp_socket socket)
{
	struct sockaddr_storage addr = lwp_socket_addr (socket);

	return ntohs (addr.ss_family == AF_INET6 ?
				  ((struct sockaddr_in6 *) &addr)->sin6_port
					: ((struct sockaddr_in *) &addr)->sin_port);
}

lw_bool lwp_urldecode (const char * in, size_t in_length,
						char * out, size_t out_length, lw_bool plus_spaces)
{
	char n [3];
	size_t cur_in = 0, cur_out = 0;

	n [2] = 0;

	while (cur_in < in_length)
	{
		if (cur_out >= out_length)
			return lw_false;

		if( (n[0] = in [cur_in]) == '%')
		{
			if ((cur_in + 2) > in_length)
				return lw_false;

			n [0] = in [++ cur_in];
			n [1] = in [++ cur_in];

			out [cur_out ++] = (char) strtol (n, 0, 16);
		}
		else
		{
			out [cur_out ++] = (plus_spaces && n [0] == '+') ? ' ' : n [0];
		}

		++ cur_in;
	}

	out [cur_out] = 0;

	return lw_true;
}

lw_bool lwp_begins_with (const char * string, const char * substring)
{
	while (*substring)
	{
		if (*string ++ != *substring ++)
			return lw_false;
	}

	return lw_true;
}

void lwp_copy_string (char * dest, const char * source, size_t size)
{
	size_t length = strlen (source);

	if (length > -- size)
		length = size;

	memcpy (dest, source, length);
	dest [length] = 0;
}

lw_bool lwp_find_char (const char ** str, size_t * len, char c)
{
	while (*len > 0)
	{
		if (**str == c)
			return lw_true;

		++ (*str);
		-- (*len);
	}

	return lw_false;
}

void lwp_close_socket (lwp_socket socket)
{
	if (socket == -1)
		return;

	#ifdef _WIN32
		closesocket (socket);
		socket = (lwp_socket)INVALID_HANDLE_VALUE;
	#else
		close (socket);
	#endif
}

#ifdef __MINGW32__
	_CRTIMP int _vscprintf (const char * format, va_list argptr);
#endif

ssize_t lwp_format (char ** output, const char * format, va_list args)
{
	ssize_t count;

	#ifdef _WIN32

		count = _vscprintf (format, args);

		if (! (*output = (char *) malloc (count + 1)))
			return 0;

		if (vsprintf (*output, format, args) < 0)
		{
			free (*output);
			*output = 0;

			return 0;
		}

	#else

		/* TODO : Alternative for where vasprintf is not supported? */

		if ((count = vasprintf (output, format, args)) == -1)
			*output = 0;

	#endif

	return count;
}

time_t lwp_parse_time (const char * string)
{
	char copy [32];
	size_t length;
	char * month, * time;
	int day, year;
	int i = 0;
	struct tm tm;

	if ((length = strlen (string)) > (sizeof (copy) - 1))
		return 0;

	if (length < 8)
		return 0;

	memcpy (copy, string, length);
	copy [length] = 0;

	if (copy [3] == ',')
	{
		/* RFC 822/RFC 1123 - Sun, 06 Nov 1994 08:49:37 GMT */

		if (length < 29)
			return 0;

		copy [ 7] = 0;
		copy [11] = 0;
		copy [16] = 0;

		day = atoi (copy + 4);
		month = copy + 8;
		year = atoi (copy + 12);
		time = copy + 17;
	}
	else if (string[3] == ' ' || string[3] == '\t')
	{
		/* ANSI C asctime() format - Sun Nov  6 08:49:37 1994 */

		if (length < 24)
			return 0;

		copy [ 7] = 0;
		copy [10] = 0;
		copy [19] = 0;

		month = copy + 4;
		day = atoi (copy + 8);
		time = copy + 11;
		year = atoi (copy + 20);
	}
	else
	{
		/* RFC 850 date (Sunday, 06-Nov-94 08:49:37 GMT) - unsupported */

		return 0;
	}

	tm.tm_mday  = day;
	tm.tm_wday  = 0;
	tm.tm_year  = year - 1900;
	tm.tm_isdst = 0;
	tm.tm_yday  = 0;

	while (i < 12)
	{
		if (strcasecmp (lwp_months [i], month))
		{
			++ i;
			continue;
		}

		tm.tm_mon = i;

		if (strlen (time) < 8)
			return 0;

		time [2] = 0;
		time [5] = 0;

		tm.tm_hour = atoi (time);
		tm.tm_min  = atoi (time + 3);
		tm.tm_sec  = atoi (time + 6);

		#if defined(__ANDROID__)
			return timegm (&tm);
		#elif defined (_WIN32)
			#ifndef __MINGW_H
				return _mkgmtime64 (&tm);
			#else
				return compat_mkgmtime64 () (&tm);
			#endif
		#else
			#ifdef HAVE_TIMEGM
				return timegm (&tm);
			#else
				#error Cannot find a suitable way to convert a tm to a UTC UNIX time
			#endif
		#endif
	}

	return 0;
}

void lwp_to_lowercase (char * str)
{
	char * i;

	for (i = str; *i; ++ i)
		*i = (char)tolower (*i);
}

void * lw_realloc_or_exit (void * const origptr, const size_t newSize)
{
	void * newMem = realloc(origptr, newSize);
	if (!newMem)
		exit(ENOMEM);
	return newMem;
}

void * lw_malloc_or_exit(const size_t size)
{
	void * newMem = malloc (size);
	if (!newMem)
		exit (ENOMEM);
	return newMem;
}

void * lw_calloc_or_exit (const size_t count, const size_t size)
{
	void * newMem = calloc (count, size);
	if (!newMem)
		exit (ENOMEM);
	return newMem;
}


/* IPv6 address that can be used consistently for outgoing messages; used mainly for UDP IPv6 servers outgoing
   Valid only if lw_udp_public_fixed_interface_index is not -1 */
struct in6_addr lwp_ipv6_public_fixed_addr = { 0 };

/* Interface index that lwp_ipv6_public_fixed_addr is used with. -1 if addr is not populated, -2 if none.
   Call lw_udp_trigger_public_address_hunt to scan the network cards for a lookup. */
int lwp_ipv6_public_fixed_interface_index = -1;

static lw_thread lw_udp_public_hunter = NULL;

#ifdef _WIN32
#include <Winsock2.h>
#include <IPHlpApi.h>
#include "address.h"
extern lw_addr lwp_addr_new_sockaddr(struct sockaddr* sockaddr);
// Hunt for a public IPv6 address that does not have a time limit or is LAN-only
static DWORD WINAPI publicFixedIPv6AddressHunterThread(LPVOID data)
{
	PIP_ADAPTER_ADDRESSES addr = (PIP_ADAPTER_ADDRESSES)malloc(16 * 1024);
	if (!addr)
		return 0;
	ULONG size = 16 * 1024; // recommended
	DWORD errcode = (DWORD)GetAdaptersAddresses(AF_INET6,
		GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER,
		NULL, (PIP_ADAPTER_ADDRESSES)addr, &size);

	lwp_trace("Finding Public IPv6...\n");
	if (errcode != ERROR_SUCCESS)
	{
		lwp_trace("Couldn't read IPv6 remote address from adapters. Error %ud.\n", errcode);
		free(addr);
		return 0;
	}

	lwp_trace("OK. First adapter:\n");
	lw_addr curIPv6Addr = NULL;
	for (PIP_ADAPTER_ADDRESSES addrL = addr; addrL; addrL = addrL->Next)
	{
		lwp_trace("Adapter name: \"%ls\". Interface index: %u. IPv6 interface index: %u.\n",
			addrL->FriendlyName, addrL->IfIndex, addrL->Ipv6IfIndex);
		for (PIP_ADAPTER_UNICAST_ADDRESS unicast = addrL->FirstUnicastAddress; unicast; unicast = unicast->Next)
		{
			if (unicast->Address.lpSockaddr->sa_family != AF_INET6)
				continue;

			if (!curIPv6Addr)
				curIPv6Addr = lwp_addr_new_sockaddr((struct sockaddr*)unicast->Address.lpSockaddr);
			else
				lwp_addr_set_sockaddr(curIPv6Addr, (struct sockaddr*)unicast->Address.lpSockaddr);

			// Not sure why, but suffix and prefix are IpSuffixOriginLinkLayerAddress (4) for public IP.
			// However, actual link-layer are prefix 2, suffix 4, loopback are 2, 2.
			if ((unicast->SuffixOrigin == IpSuffixOriginLinkLayerAddress ||
				unicast->SuffixOrigin == IpSuffixOriginManual ||
				unicast->SuffixOrigin == IpSuffixOriginDhcp) &&
				(unicast->PrefixOrigin == IpPrefixOriginRouterAdvertisement ||
					unicast->PrefixOrigin == IpPrefixOriginManual ||
					unicast->PrefixOrigin == IpPrefixOriginDhcp))
			{
				// Windows XP does not have a OnLinkPrefixLength member, default to pretending it's good
				#if WINVER < 0x0600
				UINT8 onLinkPrefixLength = 64;
				if (unicast->Length >= sizeof(IP_ADAPTER_UNICAST_ADDRESS_LH))
					onLinkPrefixLength = ((PIP_ADAPTER_UNICAST_ADDRESS_LH)unicast)->OnLinkPrefixLength;
				#else // targeting Vista+ SDK
					UINT8 onLinkPrefixLength = unicast->OnLinkPrefixLength;
				#endif

				lwp_trace("\tFound possibly usable IPv6; OL prefix length %hhu, valid %u, prefer %u, lease %u:\n\t\t",
					onLinkPrefixLength, unicast->ValidLifetime, unicast->PreferredLifetime, unicast->LeaseLifetime);
				lwp_trace("\t\t%s\n", lw_addr_tostring(curIPv6Addr));
				// Note: permanent IPs do not have infinite lifetime, strangely. One was 200k.
				if (onLinkPrefixLength == 64)
				{
					if (lwp_ipv6_public_fixed_interface_index < 0)
					{
						lwp_trace("\tValid public IPv6; will be using ^ address.\n");
						lwp_ipv6_public_fixed_addr = ((struct sockaddr_in6*)unicast->Address.lpSockaddr)->sin6_addr;
						lwp_ipv6_public_fixed_interface_index = addrL->IfIndex;
					}
					else
						lwp_trace("\tValid public IPv6; skipping, already have a usable IPv6.\n");
				}
			}
			else
			{
				lwp_trace("\tFound bad IPv6. Prefix origin is %d; Suffix origin is %d.\n",
					(int)unicast->PrefixOrigin, (int)unicast->SuffixOrigin);
				lwp_trace("\t\t%s\n", lw_addr_tostring(curIPv6Addr));
			}
			lwp_trace("\tNext address...\n");
		}
		lwp_trace("\nAddresses done, next adapter...\n");
	}

	if (curIPv6Addr == NULL)
		lwp_ipv6_public_fixed_interface_index = -2;

#ifdef _lacewing_debug
	// If an IPv6 was found, reset back to it for the log
	if (curIPv6Addr)
	{
		struct sockaddr_in6 bob = { 0 };
		bob.sin6_addr = lwp_ipv6_public_fixed_addr;
		lwp_addr_set_sockaddr(curIPv6Addr, (struct sockaddr*)&bob);
	}
	lwp_trace("Results of IPv6 adapter lookup: %s\n", curIPv6Addr ? lw_addr_tostring(curIPv6Addr) : "no public ipv6 found!");
#endif
	lw_addr_delete(curIPv6Addr);
	free(addr);
	return 0;
}

#else // not _WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <dlfcn.h>
#include <sys/ioctl.h>
#include <net/if.h>
#define _WSACMSGHDR cmsghdr
#define WSA_CMSG_LEN(x) CMSG_LEN(x)
#define WSA_CMSG_DATA(x) CMSG_DATA(x)

// Complaints about various sign to unsigned, they're useless
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic push

#if !defined(__APPLE__) || (defined(__ANDROID__) && __ANDROID_API__ < 24)
// A Netlink-based workaround for missing getifaddrs, based on Android source:
// https://github.com/LISPmob/lispdroid/blob/master/modulebased/lispd/ifaddrs-android.h#L36
// Adapted by Phi from C++ to C style.
// iOS/Mac does not have Netlink at all.

#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

struct lw_ifaddrs
{
	// Pointer to next struct in list, or NULL at end.
	struct lw_ifaddrs* ifa_next;
	// Interface name.
	char* ifa_name;
	// Interface flags.
	unsigned int ifa_flags;
	// Interface network address.
	struct sockaddr* ifa_addr;
	// Interface netmask.
	struct sockaddr* ifa_netmask;
};

static lw_bool setNameAndFlagsByIndex(struct lw_ifaddrs* that, int family, unsigned int interfaceIndex)
{
	// Get the name.
	char buf[IFNAMSIZ];
	char* name = if_indextoname(interfaceIndex, buf);
	if (name == NULL)
	{
		lwp_trace("setNameAndFlagsByIndex > if_indextoname failed, %d\n", errno);
		return lw_false;
	}
	if (that->ifa_name)
		free(that->ifa_name);
	that->ifa_name = strdup(name);
	// Get the flags.
	int fd = socket(family, SOCK_DGRAM, 0);
	if (fd == -1)
	{
		lwp_trace("setNameAndFlagsByIndex > socket failed, %d\n", errno);
		return lw_false;
	}
	struct ifreq ifr = { 0 };
	strcpy(ifr.ifr_name, name);
	int rc = ioctl(fd, SIOCGIFFLAGS, &ifr);
	if (rc == -1)
	{
		lwp_trace("setNameAndFlagsByIndex > ioctl failed, %d\n", errno);
		close(fd);
		return lw_false;
	}
	that->ifa_flags = (unsigned int)ifr.ifr_flags;
	close(fd);
	return lw_true;
}
// Returns a pointer to the first byte in the address data (which is
// stored in network byte order).
static uint8_t* sockaddrBytes(unsigned short family, struct sockaddr_storage* ss)
{
	if (family == AF_INET) {
		struct sockaddr_in* ss4 = (struct sockaddr_in*)ss;
		return (uint8_t*)(&ss4->sin_addr);
	}
	else if (family == AF_INET6) {
		struct sockaddr_in6* ss6 = (struct sockaddr_in6*)ss;
		return (uint8_t*)(&ss6->sin6_addr);
	}
	lwp_trace("sockaddrBytes > failed, %d\n", family);
	return NULL;
}
// Netlink gives us the address family in the header, and the
// sockaddr_in or sockaddr_in6 bytes as the payload. We need to
// stitch the two bits together into the sockaddr that's part of
// our portable interface.
static void setAddress(struct lw_ifaddrs* const that, const unsigned short family, void* data, size_t byteCount)
{
	// Set the address proper...
	struct sockaddr_storage* ss = (struct sockaddr_storage*)calloc(sizeof(*ss), 1);
	that->ifa_addr = (struct sockaddr*)ss;
	ss->ss_family = family;
	uint8_t* dst = sockaddrBytes(family, ss);
	memcpy(dst, data, byteCount);
}
// Netlink gives us the prefix length as a bit count. We need to turn
// that into a BSD-compatible netmask represented by a sockaddr*.
static void setNetmask(struct lw_ifaddrs* const that, const unsigned short family, const size_t prefixLength)
{
	// ...and work out the netmask from the prefix length.
	struct sockaddr_storage* ss = (struct sockaddr_storage*)calloc(sizeof(*ss), 1);
	that->ifa_netmask = (struct sockaddr*)ss;
	ss->ss_family = family;
	uint8_t* dst = sockaddrBytes(family, ss);
	memset(dst, 0xff, prefixLength / 8);
	if ((prefixLength % 8) != 0)
		dst[prefixLength / 8] = (uint8_t)(0xff << (8 - (prefixLength % 8)));
}
struct addrReq_struct {
	struct nlmsghdr netlinkHeader;
	struct ifaddrmsg msg;
};
static lw_bool sendNetlinkMessage(int fd, const void* data, size_t byteCount)
{
	ssize_t sentByteCount = TEMP_FAILURE_RETRY(send(fd, data, byteCount, 0));
	return (sentByteCount == (ssize_t)(byteCount));
}
static ssize_t recvNetlinkMessage(int fd, char* buf, size_t byteCount)
{
	return TEMP_FAILURE_RETRY(recv(fd, buf, byteCount, 0));
}

static int netlink_getifaddrs(struct ifaddrs** resultP)
{
	struct lw_ifaddrs** const result = (struct lw_ifaddrs**)resultP;
	// Simplify cleanup for callers.
	*result = NULL;
	// Create a netlink socket.
	int fd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
	if (fd < 0)
	{
		lwp_trace("socket af_netlink couldn't create, %d\n", errno);
		return -1;
	}
	// Ask for the address information.
	struct addrReq_struct addrRequest = { 0 };
	addrRequest.netlinkHeader.nlmsg_flags = NLM_F_REQUEST | NLM_F_MATCH;
	addrRequest.netlinkHeader.nlmsg_type = RTM_GETADDR;
	addrRequest.netlinkHeader.nlmsg_len = NLMSG_ALIGN(NLMSG_LENGTH(sizeof(addrRequest)));
	addrRequest.msg.ifa_family = AF_UNSPEC; // All families.
	addrRequest.msg.ifa_index = 0; // All interfaces.
	if (!sendNetlinkMessage(fd, &addrRequest, addrRequest.netlinkHeader.nlmsg_len)) {
		lwp_trace("send netlink couldn't go thru, %d\n", errno);
		close(fd);
		return -1;
	}
	// Read the responses.
	char * buf = (char *)calloc(65536, 1);
	ssize_t bytesRead;
	while ((bytesRead = recvNetlinkMessage(fd, &buf[0], 65536)) > 0) {
		struct nlmsghdr* hdr = (struct nlmsghdr*)(&buf[0]);
		for (; NLMSG_OK(hdr, (size_t)bytesRead); hdr = NLMSG_NEXT(hdr, bytesRead)) {
			switch (hdr->nlmsg_type)
			{
				case NLMSG_DONE:
					close(fd);
					free(buf);
					return 0;
				case NLMSG_ERROR:
					lwp_trace("NLMSG_ERROR, %d\n", errno);
					close(fd);
					free(buf);
					return -1;
				case RTM_NEWADDR:
				{
					struct ifaddrmsg* const address = (struct ifaddrmsg*)(NLMSG_DATA(hdr));
					struct rtattr* rta = IFA_RTA(address);
					size_t ifaPayloadLength = IFA_PAYLOAD(hdr);
					while (RTA_OK(rta, ifaPayloadLength)) {
						if (rta->rta_type == IFA_LOCAL || rta->rta_type == IFA_ADDRESS) {
							const unsigned short family = address->ifa_family;
							if (family == AF_INET6 || family == AF_INET) {
								struct lw_ifaddrs* const last = *result;
								*result = (struct lw_ifaddrs*)calloc(sizeof(**result), 1);
								(*result)->ifa_next = last;

								if (!setNameAndFlagsByIndex(*result, family, address->ifa_index)) {
									lwp_trace("setNameAndFlagsByIndex error, %d\n", errno);
									close(fd);
									free(buf);
									return -1;
								}
								setAddress(*result, family, RTA_DATA(rta), RTA_PAYLOAD(rta));
								setNetmask(*result, family, address->ifa_prefixlen);
							}
						}
						rta = RTA_NEXT(rta, ifaPayloadLength);
					}
				}
				break; // out of case
			} // switch type of info
		} // for info in packet
	} // while data remaining

	// We only get here if recv fails before we see a NLMSG_DONE.
	lwp_trace("recv failed, %d\n", errno);
	close(fd);
	free(buf);
	return -1;
}
static void netlink_freeifaddrs(struct ifaddrs * addressesP)
{
	struct lw_ifaddrs* const addresses = (struct lw_ifaddrs *)addressesP;
	for (struct lw_ifaddrs* addr = addresses, *next; addr; addr = next)
	{
		next = addr->ifa_next;
		free(addr->ifa_addr);
		free(addr->ifa_name);
		free(addr->ifa_netmask);
		free(addr);
	}
}
#endif
typedef int (*getifaddrs_func)(struct ifaddrs**);
typedef void (*freeifaddrs_func)(struct ifaddrs*);

// Hunt for a public IPv6 address that does not have a time limit or is LAN-only
static void * publicFixedIPv6AddressHunterThread(void * data)
{
	struct ifaddrs* ifaddr = NULL, * ifa;
	char addr_str[INET6_ADDRSTRLEN];

	// Android SDK may target too early, in which case OS libc has getifaddrs,
	// but we can't link to it at build time
	getifaddrs_func my_getifaddrs = NULL, my_getifaddrs2 = NULL;
	freeifaddrs_func my_freeifaddrs = NULL, my_freeifaddrs2 = NULL;
	void* libc = NULL;
#if !defined(__APPLE__) || (defined(__ANDROID__) && __ANDROID_API__ < 24)
	my_getifaddrs2 = netlink_getifaddrs;
	my_freeifaddrs2 = netlink_freeifaddrs;
#endif
#if !defined(__ANDROID__) || __ANDROID_API__ >= 24
	// iOS, Mac, Linux, and Android SDK 24+: link getifaddrs directly
	my_getifaddrs = getifaddrs;
	my_freeifaddrs = freeifaddrs;
#else
	libc = dlopen("libc.so", RTLD_LAZY);
	if (!libc) {
		lwp_trace("Couldn't read IPv6 remote address from adapters (getifaddrs). SO file could not be dynamically opened. Error %d.", errno);
	}
	else
	{
		my_getifaddrs = (getifaddrs_func)dlsym(libc, "getifaddrs");
		my_freeifaddrs = (freeifaddrs_func)dlsym(libc, "freeifaddrs");

		if (!my_getifaddrs || !my_freeifaddrs)
		{
			lwp_trace("Couldn't read IPv6 remote address from adapters (getifaddrs). Function could not be dynamically linked. Error %d.", errno);
			dlclose(libc);
			libc = NULL;
		}
	}
#endif

	if (!my_getifaddrs || my_getifaddrs(&ifaddr) == -1)
	{
		if (my_getifaddrs) {
			lwp_trace("getifaddrs failed with error %d", errno);
		}
		if (libc)
		{
			dlclose(libc);
			libc = NULL;
		}

		if (!my_getifaddrs2)
			return (void*)-1;

		if (my_getifaddrs2(&ifaddr) == -1)
		{
			lwp_trace("netlink_getifaddrs also failed with error %d", errno);
			return (void*)-1;
		}

		my_freeifaddrs = my_freeifaddrs2;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (!ifa->ifa_addr)
		{
			lwp_trace("\tSkipping due to null.\n");
			continue;
		}

		struct sockaddr_in6* sin6 = (struct sockaddr_in6*)ifa->ifa_addr;

		// Convert to text form
		if (inet_ntop(sin6->sin6_family, sin6->sin6_family == AF_INET6 ? &sin6->sin6_addr
			: (struct in6_addr *) &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, addr_str, sizeof(addr_str)))
		{
			lwp_trace("Interface: %s, IP Address: %s\n", ifa->ifa_name, addr_str);
		}

		if (sin6->sin6_family != AF_INET6)
		{
			lwp_trace("\tSkipping due to wrong address family (%d).\n", sin6->sin6_family);
			continue;
		}

		// Skip link-local addresses (LAN), and loopback
		if (IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr) ||
			IN6_IS_ADDR_LOOPBACK(&sin6->sin6_addr) ||
			IN6_IS_ADDR_MULTICAST(&sin6->sin6_addr)) {
			lwp_trace("\tSkipping due to multicast/loopback/linklocal.\n");
			continue;
		}
		// Android does not use RFC 4941 privacy addresses by default, but it still has multiple addresses,
		// and we want the server IPv6 address to be consistent
		// Ensure network interface is up and capable of broadcast - only wifi does broadcast
		const unsigned int desired_flags = (IFF_UP | IFF_RUNNING | IFF_BROADCAST);
		if ((ifa->ifa_flags & desired_flags) != desired_flags) {
			lwp_trace("\tSkipping due to being down, or not capable of broadcast (flag %i, 0x%x).\n",
				ifa->ifa_flags, ifa->ifa_flags);
			continue;
		}

		if (lwp_ipv6_public_fixed_interface_index == -1)
		{
			lwp_ipv6_public_fixed_interface_index = (int)if_nametoindex(ifa->ifa_name);
			lwp_ipv6_public_fixed_addr = sin6->sin6_addr;
			lwp_trace("\tPicked that address.\n");
		}
		else
			lwp_trace("\tSkipping as a valid address is already picked.\n");
	}

	if (lwp_ipv6_public_fixed_interface_index > -1)
	{
		inet_ntop(AF_INET6, &lwp_ipv6_public_fixed_addr, addr_str, sizeof(addr_str));
		lwp_trace("Results of IPv6 adapter lookup: \"%s\".\n", addr_str);
	}
	else
		lwp_trace("Results of IPv6 adapter lookup: none found\n");

	my_freeifaddrs(ifaddr);
	if (libc)
		dlclose(libc);
	return 0;
}

// restore sign conversion complaints
#pragma GCC diagnostic pop

#endif // _WIN32

// Attempts to populate lwp_ipv6_public_fixed_interface_index + lwp_ipv6_public_fixed_addr
void lwp_trigger_public_address_hunt(lw_bool block)
{
	if (lwp_ipv6_public_fixed_interface_index != -1)
		return;

	if (!lw_udp_public_hunter)
	{
		lw_udp_public_hunter = lw_thread_new("IPv6 adapter address finder", (void*)publicFixedIPv6AddressHunterThread);
		lw_thread_start(lw_udp_public_hunter, NULL);
	}
	if (block)
		lw_thread_join(lw_udp_public_hunter);
}

lw_bool lwp_set_ipv6pktinfo_cmsg(void * cmsg2)
{
	struct _WSACMSGHDR * cmsg = (struct _WSACMSGHDR *)cmsg2;
	// Trigger a lookup, or wait for existing lookup to finish
	if (lwp_ipv6_public_fixed_interface_index == -1)
		lwp_trigger_public_address_hunt(lw_true);

	if (lwp_ipv6_public_fixed_interface_index < 0)
		return lw_false;

	cmsg->cmsg_level = IPPROTO_IPV6;
	cmsg->cmsg_type = IPV6_PKTINFO;
	cmsg->cmsg_len = WSA_CMSG_LEN(sizeof(struct in6_pktinfo));
	struct in6_pktinfo* pktinfo = (struct in6_pktinfo*)WSA_CMSG_DATA(cmsg);
	pktinfo->ipi6_addr = lwp_ipv6_public_fixed_addr;
	pktinfo->ipi6_ifindex = (unsigned int)lwp_ipv6_public_fixed_interface_index;
	return lw_true;
}
