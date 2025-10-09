/* vim: set noet ts=4 sw=4 sts=4 ft=cpp:
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * Copyright (C) 2017-2025 Darkwire Software.
 * All rights reserved.
 *
 * https://opensource.org/license/mit
*/

#include "Lacewing.h"
#ifdef _MSC_VER
	// suppress complaints about utf8proc C enums not being C++ enum classes
	#pragma warning (push)
	#pragma warning (disable: 26812)
#endif

// Comments for all the below functions can be found in the header file.
// IntelliSense should display them anyway.

void lw_addr_prettystring(const char * input, char * const output, size_t outputSize)
{
	// It's a pure IPv4 already, or a pure IPv6, not IPv4-mapped-IPv6
	if (strncmp(input, "[::ffff:", 8) != 0)
	{
		// IPv6 is in "[address]:port" format, IPv4 is in "address:port" format
		const char * const portSepPos = strchr(input, input[0] == '[' ? ']' : ':');

		// No port, copy as-is
		if (portSepPos == NULL)
		{
			assert(outputSize > strlen(input) && "IP output buffer too small");
			strcpy(output, input);
		}
		else // Strip port
		{
			size_t portSepSize = portSepPos - input;
			if (input[0] == '[')
				++portSepSize; // include the ']' of IPv6

			assert(outputSize > portSepSize && "IP output buffer too small");
			memcpy(output, input, portSepSize);
			output[portSepSize] = '\0';
		}
	}
	else // IPv4 wrapped inside IPv6
	{
		// Start search for "]" at offset of 15
		// 8 due to "[::ffff:" -> 8 chars
		// 7 due to "1.2.3.4" -> 7 chars
		for (std::size_t i = 15, len = strnlen(&input[15], 64 - 15) + 15; i < len; ++i)
		{
			if (input[i] == ']')
			{
				// Skip the first 8 chars of "[::ffff:"
				assert(outputSize >= i - 8 && "IP output buffer too small");
				memmove(output, &input[8], i - 8);
				output[i - 8] = '\0';

				break;
			}
		}
	}
}

bool lw_sv_cmp(const std::string_view first, const std::string_view second)
{
	if (first.size() != second.size())
		return false;

	return !memcmp(first.data(), second.data(), first.size());
}

bool lw_u8str_icmp(const std::string_view first, const std::string_view second)
{
	// Assume the strings are already composed
	if (first.size() != second.size())
		return false;

	return lw_u8str_simplify(first, false, false) == lw_u8str_simplify(second, false, false);
}

std::string lw_u8str_simplify(const std::string_view first, bool destructive, bool extralumping)
{
	if (first.empty())
		return std::string();

	// Effectively call utf8proc_tolower(), but without null terminator, and return value is more
	// obviously not the input value.

	// This is an NFKC transformation, a stripping transformation, and by use of casefold,
	// optionally a lowercase transformation.
	const utf8proc_option_t nfkc = (utf8proc_option_t)(UTF8PROC_STABLE | UTF8PROC_COMPOSE | UTF8PROC_COMPAT |
		UTF8PROC_NLF2LS | UTF8PROC_STRIPCC | UTF8PROC_REJECTNA |
		(destructive ? (UTF8PROC_CASEFOLD | UTF8PROC_LUMP | UTF8PROC_STRIPMARK) : 0));

	utf8proc_uint8_t * retval;
	utf8proc_ssize_t resultSizeBytes = utf8proc_map((utf8proc_uint8_t *)first.data(), first.size(), &retval, nfkc);

	if (resultSizeBytes <= 0)
		return std::string();

	std::string u8str((char *)retval, resultSizeBytes);
	free(retval);

	// Skip additional lumping
	if (!destructive || !extralumping)
		return u8str;

	// Lots of the characters are lumped together by virtue of UTF8PROC_LUMP enum above.
	// These further things are not covered by the lumping, and are manual merging of similarly-displayed characters.
	for (size_t i = 0; i < u8str.size(); ++i)
	{
		char & c = u8str[i];
		// Don't allow '0' to be confused with 'O', could happen with some fonts
		if (c == '0')
		{
			c = 'o';
			continue;
		}
		// Pipe '|', 1, and uppercase I (converted to 'i' by case folding above)
		// Include 'l' for the |\| type of checks later
		if (c == '|' || c == '1' || c == 'i' || c == 'l')
		{
			c = 'l';

			// Someone faking a D with |) or the like
			if (u8str.size() > i + 1 && u8str[i + 1] == ')')
				u8str.erase(i + 1, 1);

			// Read backwards from ending | in these detections,
			// so the simplifcation of "|1il" => "l" has happened already
			// (so |\| and 1\1 can be looked for by l\l )
			if (i >= 2 && u8str[i - 2] == 'l')
			{
				// |\| (l\l to N (but due to lowercase, n)
				if (u8str[i - 1] == '\\')
				{
					u8str[i - 2] = 'n'; // N but lowercase
					u8str.erase(i - 2, 2); // remove "\l"
					i -= 2;
				}
				// |\/| (lvl) to M (note a "A\/B" will become "AvB" due to the \/ check later)
				else if (u8str[i - 1] == 'v')
				{
					u8str[i - 2] = 'm'; // M but lowercase
					u8str.erase(i - 1, 2); // remove "vl"
					i -= 2;
				}
			}
			continue;
		}
		// 5 and S are similar
		if (c == '5')
		{
			c = 's';
			continue;
		}
		// ( with C
		if (c == '(')
		{
			c = 'c';
			continue;
		}
		// horizontal ellipsis (U+2026) to "..."
		if (u8str.size() > i + 2 && c == ((char)0xE2)  && u8str[i + 1] == ((char)0x80) && u8str[i + 2] == ((char)0xA6))
		{
			u8str[i] = '.';
			u8str[++i] = '.';
			u8str[++i] = '.';
			continue;
		}
		// \/ to V (but due to lowercase, v)
		if (c == '\\' && u8str.size() > i + 1 && u8str[i + 1] == '/')
		{
			c = 'v';
			u8str.erase(i + 1, 1); // drop the '/'

			// fall thru deliberately for the vv to w comparison
		}
		// vv to w, just in case
		if ((c == 'v' || c == 'V') && u8str.size() > i + 1 && (u8str[i + 1] == 'v' || u8str[i + 1] == 'V'))
		{
			c = 'w';
			u8str.erase(i + 1, 1); // drop the second 'v'
		}
		// Box drawing characters are dumb. Anyone allowing those have brought problems upon themselves.
	}

	return u8str;
}

extern "C" bool lw_u8str_validate(const char* toValidate, size_t size)
{
	return lw_u8str_validate(std::string_view(toValidate, size));
}
bool lw_u8str_validate(const std::string_view toValidate)
{
	if (toValidate.empty())
		return true;

	const utf8proc_uint8_t * str = (utf8proc_uint8_t *)toValidate.data();
	utf8proc_int32_t thisChar;
	utf8proc_ssize_t numBytesInCodePoint, remainder = toValidate.size();
	while (remainder > 0)
	{
		numBytesInCodePoint = utf8proc_iterate(str, remainder, &thisChar);
		if (numBytesInCodePoint <= 0 || !utf8proc_codepoint_valid(thisChar))
			return false;

		str += numBytesInCodePoint;
		remainder -= numBytesInCodePoint;
	}

	return true;
}

bool lw_u8str_normalize(std::string & input)
{
	if (input.empty())
		return true;

	// Effectively call utf8proc_NFC(), but without null terminator, and return value is more
	// obviously not the input value
	utf8proc_uint8_t * retval;
	utf8proc_ssize_t resultSizeBytes = utf8proc_map((utf8proc_uint8_t *)input.data(), input.size(), &retval,
		(utf8proc_option_t)(UTF8PROC_STABLE | UTF8PROC_COMPOSE | UTF8PROC_NLF2LS | UTF8PROC_STRIPCC | UTF8PROC_REJECTNA));
	if (resultSizeBytes <= 0)
		return false;

	input.assign((char *)retval, resultSizeBytes);
	free(retval);
	return true;
}

std::string_view lw_u8str_trim(std::string_view toTrim, bool abortOnTrimNeeded)
{
	const utf8proc_uint8_t * str = (utf8proc_uint8_t *)toTrim.data();
	utf8proc_int32_t thisChar;
	utf8proc_ssize_t numBytesInCodePoint, remainder = toTrim.size();

	// ...Nothing to do
	if (remainder == 0)
		return std::string_view();

	const utf8proc_uint8_t * lastGoodEndChar = NULL;
	const utf8proc_uint8_t * firstGoodStartChar = NULL;
	utf8proc_ssize_t lastGoodEndCharLen = 0;
	bool lnpMarkSymbol = false;
	while (remainder > 0)
	{
		numBytesInCodePoint = utf8proc_iterate(str, remainder, &thisChar);
		if (numBytesInCodePoint <= 0 || !utf8proc_codepoint_valid(thisChar))
			return std::string_view();

		// Allow only 1-5, 9-18 on first one; that includes letters, numbers, punctuation.
		const utf8proc_category_t thisCharCat = utf8proc_category(thisChar);
		const bool letterNumPunc = (thisCharCat >= 1 && thisCharCat <= 5) || (thisCharCat >= 9 && thisCharCat <= 18);

		// Left hand trim - no non-space start character noted yet
		if (firstGoodStartChar == NULL)
		{
			// Bad start char, either abort or skip char in start
			if (!letterNumPunc)
			{
				if (abortOnTrimNeeded)
					return std::string_view();

				// Go to next char and try making that the start
				goto cont;
			}

			// Good start char, we'll mark this as start, then try to find end
			// TODO: May be possible to reverse-iterate, but honestly, with surrogates and stuff, can't do it.
			lastGoodEndChar = firstGoodStartChar = str;
			lastGoodEndCharLen = numBytesInCodePoint;
			lnpMarkSymbol = true; // Init to true for one-char strings
			goto cont;
		}

		// else middle or end of string.
		// It's not inStart here. It's either middle of string (anything goes) or end of string.
		// Unicode allowlist worries about the middle of string.

		// End of string allows letters, numbers, punc (as above), plus marks, symbols.
		lnpMarkSymbol = letterNumPunc || ((thisCharCat >= 6 && thisCharCat <= 8) || (thisCharCat >= 19 && thisCharCat <= 22));
		if (lnpMarkSymbol)
		{
			lastGoodEndChar = str;
			lastGoodEndCharLen = numBytesInCodePoint;
		}
		// else bad end char, which since we're in middle of string, don't know if it IS the end char.
		// So we'll keep lastGoodEndChar from last iteration

		cont:
		str += numBytesInCodePoint;
		remainder -= numBytesInCodePoint;
	}

	// Bad end char, and abort is set; die!
	if (abortOnTrimNeeded && !lnpMarkSymbol)
		return std::string_view();

	// Never hit a good start char
	if (firstGoodStartChar == NULL)
		return std::string_view();

	return std::string_view((const char *)firstGoodStartChar, ((lastGoodEndChar + lastGoodEndCharLen) - firstGoodStartChar));
}


#include <sstream>
#ifndef STRIFY
// Turns any plain text into "plain text", with the quotes.
#define SUB_STRIFY(X) #X
#define STRIFY(X) SUB_STRIFY(X)
#endif
void LacewingFatalErrorMsgBox2(const char * const func, const char * const file, const int line)
{
	// Remove the repository name
	const char * fileSub = strstr(file, "Lacewing\\");
	fileSub = fileSub ? fileSub : file;

	std::stringstream err;
	err << "Lacewing fatal error detected.\nFile: "sv << fileSub << "\nFunction: "sv << func << "\nLine: "sv << line;
#ifdef _WIN32
	MessageBoxA(NULL, err.str().c_str(), "" PROJECT_NAME " fatal error", MB_ICONERROR);
	std::abort();
#else
	char output[512];
	strcpy(output, err.str().c_str());
	assert(false && "Fatal error. Attach debugger and view output variable.");
#endif
}

#ifdef _MSC_VER
	#pragma warning (pop)
#endif
