#pragma once
#include "Lacewing.h"

namespace lacewing {

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
		for (int i = 15, len = strnlen(&input[15], 64 - 15) + 15; i < len; i++)
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

}