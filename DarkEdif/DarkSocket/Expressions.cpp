#include "Common.h"

// ============================================================================
//
// EXPRESSIONS
//
// ============================================================================
const TCHAR* Extension::GetErrorOrInfo() {
	if (curEvent->eventID == Conditions::OnError || curEvent->eventID == Conditions::OnInfo)
		return Runtime.CopyString(curEvent->info.c_str());

	globals->CreateError(-1, _T("Using the GetErrorOrInfo$() expression in a non-logging event."));
	return Runtime.CopyString(_T(""));
}
const TCHAR* Extension::DEPRECATED_GetErrors(int clear)
{
	// If they're not correctly listening to errors, no point reporting it via CreateError - you'll just make an infinite loop
	DarkEdif::MsgBox::Error(_T("Invalid error handling"), _T("You're using the deprecated Error$(\"DarkSocket\") text expression in Fusion event %d."),
		DarkEdif::GetCurrentFusionEventNum(this));
	return Runtime.CopyString(_T("<DEPRECATED EXPRESSION>"));
}
const TCHAR * Extension::DEPRECATED_GetReports(int clear)
{
	globals->CreateError(-1, _T("Using the deprecated GetReports$() expression in a non-logging event."));
	return Runtime.CopyString(_T("<DEPRECATED EXPRESSION>"));
}
int Extension::GetCurrent_SocketID()
{
	return curEvent->fusionSocketID;
}
int Extension::GetCurrent_PeerSocketID()
{
	return curEvent->source ? curEvent->source->peerSocketID : -1;
}
const TCHAR * Extension::DEPRECATED_GetLastMessageText()
{
	// We don't know the whole text is usable, or that it's in a particular encoding.
	globals->CreateError(-1, _T("GetLastMessageText was deprecated, as it doesn't specify text encoding. Use the Get String expression instead."));
	return Runtime.CopyString(_T("<DEPRECATED>"));
}
unsigned int Extension::DEPRECATED_GetLastMessageAddress()
{
	globals->CreateError(-1, _T("Returning address as integer won't work on 64-bit systems. Re-create the expression to use the string version."));
	return -1;
}
const TCHAR * Extension::PendingData_GetAddress()
{
	if (curEvent->source == nullptr || curEvent->source->pendingDataToRead.empty())
	{
		globals->CreateError(-1, _T("CRASH LIKELY: Tried to read pending data in an event with no pending data!"));
		return Runtime.CopyString(_T(""));
	}
	return Runtime.CopyString(std::to_tstring((std::uint64_t)curEvent->source->pendingDataToRead.data()).c_str());
}
unsigned int Extension::GetLastReceivedData_Size()
{
	return (unsigned int)curEvent->source->pendingDataToRead.size();
}
int Extension::GetNewSocketID()
{
	globals->threadsafe.edif_lock();
	int temp = globals->newSocketID;
	globals->threadsafe.edif_unlock();
	return temp;
}
int Extension::DEPRECATED_GetSocketIDForLastEvent()
{
	globals->CreateError(curEvent->fusionSocketID, _T("Last event socket ID is deprecated. Use current source socket ID instead."));
	return curEvent->fusionSocketID;
}
int Extension::GetPortFromType(const TCHAR * typeStr)
{
	const struct servent * const service = getservbyname(DarkEdif::TStringToUTF8(typeStr).c_str(), NULL);
	if (service == nullptr)
	{
		globals->CreateError(-1, _T("Couldn't find service \"%s\" in /etc/services file."), typeStr);
		return -1;
	}
	return service->s_port;
}
unsigned int Extension::DEPRECATED_PacketBeingBuilt_GetAddress()
{
	globals->CreateError(-1, _T("Returning address as integer won't work on 64-bit systems. Re-create the expression to use the string version."));
	return UINT32_MAX; // turns into -1 because Fusion can't handle unsigned 32-bit
}
const TCHAR * Extension::PacketBeingBuilt_GetAddress()
{
	return Runtime.CopyString(std::to_tstring((std::uint64_t)packetBeingBuilt.data()).c_str());
}
unsigned int Extension::PacketBeingBuilt_GetSize()
{
	return (std::uint32_t)packetBeingBuilt.size();
}
int Extension::PendingData_GetByte(int readFromIndex)
{
	if (readFromIndex < 0 || (curEvent->source->pendingDataToRead.size() - readFromIndex) < sizeof(std::int8_t))
		return globals->CreateError(-1, _T("Could not retrieve byte at index %d: reading beyond the end of the pending data."), readFromIndex), 0;

	const std::int8_t i = *((std::int8_t *)&curEvent->source->pendingDataToRead[readFromIndex]);
	curEvent->source->pendingDataToReadCursor += sizeof(std::int8_t);
	return (int)i;
}
int Extension::PendingData_GetUnsignedByte(int readFromIndex)
{
	if (readFromIndex < 0 || (curEvent->source->pendingDataToRead.size() - readFromIndex) < sizeof(std::uint8_t))
		return globals->CreateError(-1, _T("Could not retrieve unsigned byte at index %d: reading beyond the end of the pending data."), readFromIndex), 0;

	const std::uint8_t i = *((std::uint8_t *)&curEvent->source->pendingDataToRead[readFromIndex]);
	curEvent->source->pendingDataToReadCursor += sizeof(std::uint8_t);
	return (int)i;
}
int Extension::PendingData_GetShort(int readFromIndex)
{
	if (readFromIndex < 0 || (curEvent->source->pendingDataToRead.size() - readFromIndex) < sizeof(short))
		return globals->CreateError(-1, _T("Could not retrieve unsigned short at index %d: reading beyond the end of the pending data."), readFromIndex), 0;

	const short i = *((short *)&curEvent->source->pendingDataToRead[readFromIndex]);
	curEvent->source->pendingDataToReadCursor += sizeof(short);
	return (int)i;
}
int Extension::PendingData_GetUnsignedShort(int readFromIndex)
{
	if (readFromIndex < 0 || (curEvent->source->pendingDataToRead.size() - readFromIndex) < sizeof(unsigned short))
		return globals->CreateError(-1, _T("Could not retrieve unsigned short at index %d: reading beyond the end of the pending data."), readFromIndex), 0;

	const unsigned short i = *((unsigned short *)&curEvent->source->pendingDataToRead[readFromIndex]);
	curEvent->source->pendingDataToReadCursor += sizeof(unsigned short);
	return (int)i;
}
int Extension::PendingData_GetInteger(int readFromIndex)
{
	if (readFromIndex < 0 || (curEvent->source->pendingDataToRead.size() - readFromIndex) < sizeof(int))
		return globals->CreateError(-1, _T("Could not retrieve integer at index %d: reading beyond the end of the pending data."), readFromIndex), 0;

	const int i = *((int *)&curEvent->source->pendingDataToRead[readFromIndex]);
	curEvent->source->pendingDataToReadCursor += sizeof(int);
	return i;
}
int Extension::DEPRECATED_PendingData_GetLong(int readFromIndex)
{
	// useless; long is either signed int32, in which case GetInteger works fine,
	// or it's an int64, in which case Fusion can't handle it returned as an integer.
	globals->CreateError(-1, _T("Deprecated expression \"read long\" used. Recreate the expression."));
	return 0;
}
const TCHAR * Extension::PendingData_GetInt64(int readFromIndex)
{
	if (readFromIndex < 0 || (curEvent->source->pendingDataToRead.size() - (size_t)readFromIndex) < sizeof(std::int64_t))
		return globals->CreateError(-1, _T("Could not retrieve int64 at index %d: reading beyond the end of the pending data."), readFromIndex), 0;

	const std::int64_t i64 = *((std::int64_t *)&curEvent->source->pendingDataToRead[readFromIndex]);
	curEvent->source->pendingDataToReadCursor += sizeof(std::int64_t);
	return Runtime.CopyString(std::to_tstring(i64).c_str());
}
const TCHAR * Extension::PendingData_GetUnsignedInt64(int readFromIndex)
{
	if (readFromIndex < 0 || (curEvent->source->pendingDataToRead.size() - (size_t)readFromIndex) < sizeof(std::uint64_t))
		return globals->CreateError(-1, _T("Could not retrieve uint64 at index %d: reading beyond the end of the pending data."), readFromIndex), 0;

	const std::uint64_t ui64 = *((std::int64_t *)&curEvent->source->pendingDataToRead[readFromIndex]);
	curEvent->source->pendingDataToReadCursor += sizeof(std::uint64_t);
	return Runtime.CopyString(std::to_tstring(ui64).c_str());
}
float Extension::PendingData_GetFloat(int readFromIndex)
{
	if (readFromIndex < 0 || (curEvent->source->pendingDataToRead.size() - (size_t)readFromIndex) < sizeof(float))
		return globals->CreateError(-1, _T("Could not retrieve float at index %d: reading beyond the end of the pending data."), readFromIndex), 0.0f;

	const float f = *((float *)&curEvent->source->pendingDataToRead[readFromIndex]);
	curEvent->source->pendingDataToReadCursor += sizeof(float);
	return f;
}
float Extension::PendingData_GetDouble(int readFromIndex)
{
	if (readFromIndex < 0 || (curEvent->source->pendingDataToRead.size() - (size_t)readFromIndex) < sizeof(double))
		return globals->CreateError(-1, _T("Could not retrieve double at index %d: reading beyond the end of the pending data."), readFromIndex), 0.0f;

	// Fusion doesn't allow double-precision returns, but it's reasonable to expect a double in a received message.
	// If user really wants it unmodified, then they can just copy the bytes using PendingData_Address.
	const double d = *((double *)&curEvent->source->pendingDataToRead[readFromIndex]);
	curEvent->source->pendingDataToReadCursor += sizeof(double);
	const float dAsFloat = (float)d;
	if ((double)dAsFloat != d)
		globals->ReportInfo(-1, _T("Retrieving double-width %f to single-width %f resulted in inaccuracy. See the DarkSocket help file."), d, (double)dAsFloat);
	return dAsFloat;
}
const TCHAR * Extension::DEPRECATED_PendingData_GetString(int readFromIndex, int sizeOfString)
{
	globals->CreateError(-1, _T("Deprecated expression \"read ASCII string\" used. Recreate the expression."));
	return Runtime.CopyString(_T(""));
}
const TCHAR * Extension::DEPRECATED_PendingData_GetWString(int readFromIndex, int sizeOfString)
{
	globals->CreateError(-1, _T("Deprecated expression \"read Unicode string\" used. Recreate the expression."), readFromIndex);
	return Runtime.CopyString(_T(""));
}
int Extension::PacketBeingBuilt_ICMPChecksum(int readFromIndex, int sizeToChecksum)
{
	// 1's complement of VAR = bitwise NOT of VAR, or ~VAR in C++.

	/* Sum as a uint16 array of ints, into an uint32.
	 * If size of bank is odd-numbered, add remaining uint8.
	 * Fold the sum uint32 into a uint16.
	 * Run bitwise NOT on the uint16; this is the answer. */
	if (sizeToChecksum < 0)
		return globals->CreateError(-1, _T("Can't run ICMP checksum on %d bytes."), sizeToChecksum), -1;
	if (readFromIndex < 0 || readFromIndex + sizeToChecksum > (int)packetBeingBuilt.size())
		return globals->CreateError(-1, _T("Can't run ICMP checksum at start point %d, of %d bytes, invalid range."), readFromIndex, sizeToChecksum), -1;

	// Pair as 16-bit integers
	std::uint16_t * addr = (std::uint16_t *)&packetBeingBuilt[readFromIndex];
	std::uint32_t sum = 0;
	while (sizeToChecksum > 1)
	{
		sum += *addr;
		sizeToChecksum -= 2;
		++addr;
	}

	//  Add left-over byte, if any
	if (sizeToChecksum % 2 == 1)
		sum += *((std::uint8_t *)addr);

	//  Fold the 32-bit ulong sum to 16 bits short
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);

	// Get the one's complement of the sum
	return (std::uint16_t)(~sum & 0xFFFF);
}
const TCHAR * Extension::PendingData_GetString(const TCHAR * encodingParam, int readFromIndex, int sizeOfStringInBytesOrMinusOne)
{
	// note PendingData_FindIndexOfChar() can be used to find the sizeOfStringInBytes
	const std::string encoding = DarkEdif::TStringToUTF8(encodingParam);
	size_t sizeToUse;
	int maxOutAt = (int)curEvent->source->pendingDataToRead.size() - readFromIndex;

	if (readFromIndex < 0 || (size_t)readFromIndex + 1 >= curEvent->source->pendingDataToRead.size())
	{
		return globals->CreateError(-1, _T("Reading at index %i, outside of packet indexes 0 to %d."),
			readFromIndex, (int)curEvent->source->pendingDataToRead.size() - 1), Runtime.CopyString(_T(""));
	}
	if (sizeOfStringInBytesOrMinusOne < -1 || sizeOfStringInBytesOrMinusOne == 0)
		return globals->CreateError(-1, _T("String size in bytes can't be %i."), sizeOfStringInBytesOrMinusOne), Runtime.CopyString(_T(""));

	// Null-terminator size; find it
	if (sizeOfStringInBytesOrMinusOne == -1)
	{
		// Not enough space for a full null terminator
		if (maxOutAt < (encoding == "UTF-16"sv ? 2 : 1) - 1)
			return globals->CreateError(-1, _T("Reading beyond end of packet.")), Runtime.CopyString(_T(""));

#if 0
		const void * foundAt;
		if (encoding != "UTF-16"sv)
		{
			foundAt = memchr(&curEvent->msg[readFromIndex], '\0',
				curEvent->msg.size() - (size_t)readFromIndex);
		}
		else
		{
			#if _WIN32
				// The >> 1 divides size by 2 - sizeof(wchar_t) - while rounding down
				foundAt = wmemchr((const wchar_t *)&curEvent->msg[readFromIndex], L'\0',
					(curEvent->msg.size() - (size_t)readFromIndex) >> 1);
			#else // !_WIN32
				// UTF16 ends with two-byte null, but wmemchr() uses UTF-32 on Android/iOS, so we can't use.
				for (const unsigned short * us = (const unsigned short *)&curEvent->msg[readFromIndex];
					(const char *)us < &curEvent->msg[curEvent->msg.size() - 1]; ++us)
				{
					if (us == 0)
					{
						foundAt = us;
						break;
					}
				}
			#endif
		}
		if (foundAt == NULL)
			return globals->CreateError(-1, _T("Couldn't find the null terminator in pending data (reading from index %i)."), readFromIndex), Runtime.CopyString(_T(""));
		sizeToUse = (const char *)foundAt - &curEvent->msg[readFromIndex];
#else
		sizeToUse = curEvent->source->pendingDataToRead.size() - (size_t)readFromIndex;
#endif
	}
	// Make sure there's enough room left for the separator
	else // if (sizeOfStringInBytesOrMinusOne > 0)
	{
		sizeToUse = sizeOfStringInBytesOrMinusOne;

		if (readFromIndex + sizeToUse > curEvent->source->pendingDataToRead.size())
		{
			globals->CreateError(-1, _T("Couldn't get string: reading past the end of the string (reading from index %i to %zu)."), readFromIndex, (size_t)readFromIndex + sizeToUse);
			return Runtime.CopyString(_T("Apples"));
		}
	}

	const std::string_view packetBytes(&curEvent->source->pendingDataToRead.data()[readFromIndex], sizeToUse);
	std::tstring output;
	if (!Internal_GetTextWithEncoding(encoding, packetBytes, output, curEvent->fusionSocketID))
		return Runtime.CopyString(_T(""));
	assert(output.find(_T('\0')) == std::string::npos);
	LOGI(_T("Note; returning text \"%s\".\n"), output.c_str());
	return Runtime.CopyString(output.c_str());
}
int Extension::PendingData_FindIndexOfChar(int charToFind, int numBytesInChar, int searchStartIndex, int textEndBytes)
{
	// This expression is meant for searching for a single byte, or single Unicode character, including null.
	// For more complicated searches, the user is expected to extract the full text, and use Fusion's Find().
	//
	// This should have been a single-byte search, but it'll be unusable in UTF-16 searching for null byte,
	// as UTF-16 has null byte, char byte, null byte, etc, so numBytesInChar avoids that issue.
	// There was a clever & 0xFF trick to figure out how many bytes to search, but endianness made it unusable too.
	// Use scenario: pass the character as its byte value (or Unicode code point value).

	if (searchStartIndex < 0 || (size_t)searchStartIndex > curEvent->source->pendingDataToRead.size() - numBytesInChar)
	{
		globals->CreateError(curEvent->fusionSocketID, _T("Can't search for char %d, search start index %d is out of range 0 to %zu."), charToFind, textEndBytes,
			charToFind, searchStartIndex, curEvent->source->pendingDataToRead.size() - numBytesInChar);
		return -1;
	}
	if (numBytesInChar < 1 || numBytesInChar > 4)
		return globals->CreateError(curEvent->fusionSocketID, _T("Can't search for char %d, a %d-byte character is impossible."), charToFind, numBytesInChar), -1;

	if (textEndBytes == -1)
		textEndBytes = curEvent->source->pendingDataToRead.size() - searchStartIndex;
	else if (textEndBytes <= 0)
		return globals->CreateError(curEvent->fusionSocketID, _T("Can't search for char %d, the \"text ends after\" parameter (%d bytes) is invalid."), charToFind, textEndBytes), -1;
	// LHS: how many bytes are in the remainder of msg after searchStartIndex is skipped
	else if (curEvent->source->pendingDataToRead.size() - (size_t)searchStartIndex < (size_t)textEndBytes)
	{
		globals->CreateError(curEvent->fusionSocketID, _T("Can't search for char %d, the \"text ends after\" parameter (%d bytes) was too high."
			" Pending data ends at %d bytes from start index %d."), charToFind, textEndBytes,
			curEvent->source->pendingDataToRead.size() - searchStartIndex, searchStartIndex);
		return -1;
	}

	const std::string_view searchIn1Byte(&curEvent->source->pendingDataToRead[searchStartIndex], textEndBytes);
	const std::string_view charToFindStr(((const char*)&charToFind) + (numBytesInChar - 1), numBytesInChar);
	const size_t indexOf = searchIn1Byte.find(charToFindStr);

	if (indexOf != std::string_view::npos)
		return searchStartIndex + indexOf;
	return -1;
}
int Extension::PendingData_ReverseFindIndexOfChar(int charToFind, int numBytesInChar, int searchStartIndex, int textEndBytes)
{
	if (searchStartIndex < 0 || (size_t)searchStartIndex > curEvent->source->pendingDataToRead.size())
	{
		globals->CreateError(curEvent->fusionSocketID, _T("Can't search for char %d, search start index %d is out of range 0 to %zu."), charToFind, textEndBytes,
			charToFind, searchStartIndex, curEvent->source->pendingDataToRead.size() - 1U);
		return -1;
	}
	if (numBytesInChar < 1 || numBytesInChar > 4)
		return globals->CreateError(curEvent->fusionSocketID, _T("Can't search for char %d, a %d-byte character is impossible."), charToFind, numBytesInChar), -1;

	if (textEndBytes == -1)
		textEndBytes = curEvent->source->pendingDataToRead.size() - searchStartIndex;
	else if (textEndBytes <= 0)
		return globals->CreateError(curEvent->fusionSocketID, _T("Can't search for char %d, the \"text ends after\" parameter (%d bytes) is invalid."), charToFind, textEndBytes), -1;
	// LHS: how many bytes are in the remainder of msg after searchStartIndex is skipped
	else if (curEvent->source->pendingDataToRead.size() - (size_t)searchStartIndex < (size_t)textEndBytes)
	{
		globals->CreateError(curEvent->fusionSocketID, _T("Can't search for char %d, the \"text ends after\" parameter (%d bytes) was too high."
			" Pending data ends at %d bytes from start index %d."), charToFind, textEndBytes,
			curEvent->source->pendingDataToRead.size() - searchStartIndex, searchStartIndex);
		return -1;
	}

	const std::string_view searchIn1Byte(&curEvent->source->pendingDataToRead[searchStartIndex], textEndBytes);
	const std::string_view charToFindStr(((const char*)&charToFind) + (numBytesInChar - 1), numBytesInChar);
	size_t indexOf = searchIn1Byte.rfind(charToFindStr);

	if (indexOf != std::string_view::npos)
		return searchStartIndex + indexOf;
	return -1;
}
int Extension::GetLastReceivedData_Offset()
{
	if (curEvent->msg.empty() || !curEvent->msgIsFullyInPendingData)
		return -1;
	// It's a simple spell, but quite unbreakable.
	return std::max(-1, (int)curEvent->source->pendingDataToRead.size() - (int)curEvent->msg.size());
}
const TCHAR * Extension::GetCurrent_RemoteAddress()
{
	return Runtime.CopyString(Internal_GetIPFromSockaddr(&curEvent->source->peerSockAddress, curEvent->source->peerSockAddressSize).c_str());
}

int Extension::Statistics_BytesIn(int socketID, int peerSocketID)
{
	auto sockSrc = globals->GetSocketSource("Statistic bytes in", socketID, peerSocketID);
	return sockSrc ? sockSrc->bytesIn : -1;
}
int Extension::Statistics_BytesOut(int socketID, int peerSocketID)
{
	auto sockSrc = globals->GetSocketSource("Statistic bytes out", socketID, peerSocketID);
	return sockSrc ? sockSrc->bytesOut : -1;
}
int Extension::Statistics_PacketsIn(int socketID, int peerSocketID)
{
	auto sockSrc = globals->GetSocketSource("Statistic packets in", socketID, peerSocketID);
	return sockSrc ? sockSrc->numPacketsIn : -1;
}
int Extension::Statistics_PacketsOut(int socketID, int peerSocketID)
{
	auto sockSrc = globals->GetSocketSource("Statistic packets out", socketID, peerSocketID);
	return sockSrc ? sockSrc->numPacketsOut : -1;
}
