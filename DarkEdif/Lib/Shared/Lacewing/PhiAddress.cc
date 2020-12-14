#pragma once
#include "Lacewing.h"

/// <summary> Converts a IPv4-mapped-IPv6 address to IPv4, stripping ports.
/// 		  If the address is IPv4 or unmapped IPv6, returns it as is. </summary>
void lw_addr_prettystring(const char * input, const char * output, size_t outputSize)
{
	// It's a pure IPv4 already, or a pure IPv6, not IPv4-mapped-IPv6
	if (strncmp(input, "[::ffff:", 8))
		memcpy_s((char *)output, outputSize, input, strnlen(input, 64) + 1U);
	else // IPv4 wrapped inside IPv6
	{
		// Start search for "]" at offset of 15
		// 8 due to "[::ffff:" -> 8 chars
		// 7 due to "1.2.3.4" -> 7 chars
		for (size_t i = 15, len = strnlen(&input[15], 64 - 15) + 15; i < len; i++)
		{
			if (input[i] == ']')
			{
				// Apparently the lw_addr's buffer is used for every tostring() call.

				// actually 64, not len, as lw_addr->buffer is 64 chars
				memmove_s((char *)output, outputSize, &input[8], i - 8);
				((char *)output)[i - 8] = '\0';

				break;
			}
		}
	}
}

/// <summary> Compares if two strings match, returns true if so. Case sensitive. Does a size check. </summary>
bool lw_sv_cmp(std::string_view first, std::string_view second)
{
	if (first.size() != second.size())
		return false;

	return !memcmp(first.data(), second.data(), first.size());
}

/// <summary> Compares if two strings match, returns true if so. Case insensitive. Does a size check. </summary>
bool lw_sv_icmp(std::string_view first, std::string_view second)
{
	if (first.size() != second.size())
		return false;

#ifdef _WIN32
	return !_strnicmp(first.data(), second.data(), first.size());
#else
	return !strncasecmp(first.data(), second.data(), first.size());
#endif
}

// Unfortunately every single Unicode library decomposes into 4-byte chars.
// We can at least achieve platform compatibility by using a third-party library.
#include "deps/utf8proc.h"


/// <summary> Returns a composed, lowercased (with invariant culture), stripped-down version of
///			  name(). Used for easier searching, and to prevent similar names as an exploit. </summary>
std::string lw_u8str_simplify(std::string_view first)
{
	if (first.empty())
		return std::string();

	// Effectively call utf8proc_tolower(), but without null terminator, and return value is more
	// obviously not the input value
	utf8proc_uint8_t * retval;
	utf8proc_ssize_t resultSizeBytes = utf8proc_map_custom((utf8proc_uint8_t *)first.data(), first.size(), &retval,
		(utf8proc_option_t)(UTF8PROC_STABLE | UTF8PROC_COMPOSE | UTF8PROC_COMPAT | UTF8PROC_LUMP |
			UTF8PROC_NLF2LS | UTF8PROC_STRIPCC | UTF8PROC_STRIPMARK | UTF8PROC_STRIPNA),
		[](utf8proc_int32_t codepoint, void *) { return utf8proc_tolower(codepoint); }, NULL);

	if (resultSizeBytes <= 0)
		return std::string();

	// _MB_CP_UTF8 YES, multibyte is used for UTF8.
	// the locale will be the UTF-8 enabled English
	// auto loc = _create_locale(LC_CTYPE | LC_COLLATE, "en_US.UTF-8");
	// assert(_mbslwr_s_l((unsigned char *)u8str.data(), u8str.size(), loc) != NO_ERROR);
	// u8str.resize(strlen(u8str.c_str()));
	std::string u8str((char *)retval, resultSizeBytes);
	free(retval);
	return u8str;
}

/// <summary> Validates a UTF-8 std::string as having readable codepoints. </summary>
bool lw_u8str_validate(std::string_view first)
{
	if (first.empty())
		return true;

	utf8proc_uint8_t * str = (utf8proc_uint8_t *)first.data();
	utf8proc_int32_t thisChar;
	utf8proc_ssize_t numBytesInCodePoint, remainder = first.size();
	while (remainder <= 0)
	{
		numBytesInCodePoint = utf8proc_iterate(str, remainder, &thisChar);
		if (numBytesInCodePoint <= 0 || !utf8proc_codepoint_valid(thisChar))
			return false;

		str += numBytesInCodePoint;
		remainder -= numBytesInCodePoint;
	}

	return true;
}

/// <summary> Returns a normalised std::string (using NFC). Assumes a valid U8 string. </summary>
bool lw_u8str_normalise(std::string & input)
{
	if (input.empty())
		return true;

	// Effectively call utf8proc_NFC(), but without null terminator, and return value is more
	// obviously not the input value
	utf8proc_uint8_t * retval;
	utf8proc_ssize_t resultSizeBytes = utf8proc_map((utf8proc_uint8_t *)input.data(), input.size(), &retval,
		(utf8proc_option_t)(UTF8PROC_STABLE | UTF8PROC_COMPOSE));
	if (resultSizeBytes <= 0)
		return false;

	input.assign((char *)retval, resultSizeBytes);
	free(retval);
	return true;
}


#include <sstream>
#ifndef STRIFY
// Turns any plain text into "plain text", with the quotes.
#define SUB_STRIFY(X) #X
#define STRIFY(X) SUB_STRIFY(X)
#endif
void LacewingFatalErrorMsgBox2(char * func, char * file, int line)
{
	std::stringstream err;
	err << "Lacewing fatal error detected.\nFile: "sv << file << "\nFunction: "sv << func << "\nLine: "sv << line;
	MessageBoxA(NULL, err.str().c_str(), "" PROJECT_NAME " Msg Box Death", MB_ICONERROR);
}
