
#include "Common.h"

const TCHAR * Extension::Error()
{
	return Runtime.CopyString(UTF8ToTString(threadData->error.text).c_str());
}
const TCHAR * Extension::ReplacedExprNoParams()
{
	return Runtime.CopyString(_T(""));
}
const TCHAR * Extension::Self_Name()
{
	return Runtime.CopyString(UTF8ToTString(Cli.name()).c_str());
}
unsigned int Extension::Self_ChannelCount()
{
	return Cli.channelcount();
}
const TCHAR * Extension::Peer_Name()
{
	return Runtime.CopyString(selPeer ? UTF8ToTString(selPeer->name()).c_str() : _T(""));
}
const TCHAR * Extension::RecvMsg_ReadAsString()
{
	return Runtime.CopyString(UTF8ToTString(threadData->receivedMsg.content).c_str());
}
int Extension::RecvMsg_ReadAsInteger()
{
	if (threadData->receivedMsg.content.size() != sizeof(int))
	{
		CreateError("Received() was used on a message that is not a number message.");
		return 0;
	}

	return *(int *)threadData->receivedMsg.content.data();
}
unsigned int Extension::RecvMsg_Subchannel()
{
	return (unsigned int)threadData->receivedMsg.subchannel;
}
int Extension::Peer_ID()
{
	return selPeer ? selPeer->id() : -1;
}
const TCHAR * Extension::Channel_Name()
{
	return Runtime.CopyString(selChannel ? UTF8ToTString(selChannel->name()).c_str() : _T(""));
}
int Extension::Channel_PeerCount()
{
	return selChannel ? selChannel->peercount() : -1;
}
const TCHAR * Extension::ChannelListing_Name()
{
	return Runtime.CopyString(threadData->channelListing ? UTF8ToTString(threadData->channelListing->name()).c_str() : _T(""));
}
int Extension::ChannelListing_PeerCount()
{
	return threadData->channelListing ? threadData->channelListing->peercount() : -1;
}
int Extension::Self_ID()
{
	return Cli.id();
}
const TCHAR * Extension::RecvMsg_StrASCIIByte(int index)
{
	if (index < 0)
	{
		CreateError("Could not read ASCII text byte from received binary, index %i is less than 0.", index);
		return Runtime.CopyString(_T(""));
	}
	if (threadData->receivedMsg.content.size() - index < sizeof(char))
	{
		CreateError("Could not read ASCII text byte from received binary at position %i, amount of message remaining is smaller than variable to be read.", index);
		return Runtime.CopyString(_T(""));
	}

	const std::string_view partial(threadData->receivedMsg.content.data() + index, 1);

	// ASCII goes up to 127, ANSI continues to 255. ANSI is locale-dependent, and if we only
	// interpret it as a fixed one like "en-us", that may not be the sender's locale, muddying
	// the water further for a Fusion developer.
	if (reinterpret_cast<const std::uint8_t &>(partial[0]) > 127)
	{
		CreateError("ANSI char %u read at position %d, which is not an ASCII character. Use StringWithSize() with size 1 to ensure consistency.",
			(std::uint32_t)reinterpret_cast<const std::uint8_t &>(partial[0]), index);
		return Runtime.CopyString(_T(""));
	}
	// Also check character is displayable (aka "printable").
	if (!isprint(partial[0]))
	{
		CreateError("ASCII char %u read, which cannot be displayed. Check index %d is correct.",
			(std::uint32_t)reinterpret_cast<const std::uint8_t &>(partial[0]), index);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(ANSIToTString(partial).c_str());
}
unsigned int Extension::RecvMsg_UnsignedByte(int index)
{
	if (index < 0)
		return CreateError("Could not read unsigned byte from received binary, index %i is less than 0.", index), 0U;
	if (threadData->receivedMsg.content.size() - index < sizeof(unsigned char))
		return CreateError("Could not read unsigned byte from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0U;

	return (unsigned int)(*(unsigned char *)(threadData->receivedMsg.content.data() + index));
}
int Extension::RecvMsg_SignedByte(int index)
{
	if (index < 0)
		return CreateError("Could not read signed byte from received binary, index %i is less than 0.", index), 0;
	if (threadData->receivedMsg.content.size() - index < sizeof(char))
		return CreateError("Could not read signed byte from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	return (int)(*(threadData->receivedMsg.content.data() + index));
}
unsigned int Extension::RecvMsg_UnsignedShort(int index)
{
	if (index < 0)
		return CreateError("Could not read unsigned short from received binary, index %i is less than 0.", index), 0;
	if (threadData->receivedMsg.content.size() - index < sizeof(unsigned short))
		return CreateError("Could not read unsigned short from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	return (unsigned int)(*(unsigned short *)(threadData->receivedMsg.content.data() + index));
}
int Extension::RecvMsg_SignedShort(int index)
{
	if (index < 0)
		return CreateError("Could not read signed short from received binary, index %i is less than 0.", index), 0;
	if (threadData->receivedMsg.content.size() - index < sizeof(short))
		return CreateError("Could not read signed short from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	return (int)(*(short *)(threadData->receivedMsg.content.data() + index));
}
unsigned int Extension::RecvMsg_UnsignedInteger(int index)
{
	if (index < 0)
		return CreateError("Could not read unsigned integer from received binary, index %i is less than 0.", index), 0;
	if (threadData->receivedMsg.content.size() - index < sizeof(unsigned int))
		return CreateError("Could not read unsigned integer from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	return (*(unsigned int *)(threadData->receivedMsg.content.data() + index));
}
int Extension::RecvMsg_SignedInteger(int index)
{
	if (index < 0)
		return CreateError("Could not read signed integer from received binary, index %i is less than 0.", index), 0;
	if (threadData->receivedMsg.content.size() - index < sizeof(int))
		return CreateError("Could not read signed integer from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	return (*(int *)(threadData->receivedMsg.content.data() + index));
}
float Extension::RecvMsg_Float(int index)
{
	if (index < 0)
		return CreateError("Could not read float from received binary, index %i is less than 0.", index), 0.0f;
	if (threadData->receivedMsg.content.size() - index < sizeof(float))
		return CreateError("Could not read float from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0.0f;

	return (*(float *)(threadData->receivedMsg.content.data() + index));
}
const TCHAR * Extension::RecvMsg_StringWithSize(int index, int size)
{
	if (index < 0)
	{
		CreateError("Could not read string with size from received binary, index %i is less than 0.", index);
		return Runtime.CopyString(_T(""));
	}
	if (size < 0)
	{
		CreateError("Could not read string with size from received binary, supplied size %i is less than 0.", size);
		return Runtime.CopyString(_T(""));
	}
	if (threadData->receivedMsg.content.size() - index < (size_t)size)
	{
		CreateError("Could not read string with size from received binary at position %i, amount of message remaining is smaller than supplied string size %i.", index, size);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(RecvMsg_Sub_ReadString(index, size, false).c_str());
}
const TCHAR * Extension::RecvMsg_String(int index)
{
	if (index < 0)
	{
		CreateError("Could not read null-terminated string from received binary, index %i is less than 0.", index);
		return Runtime.CopyString(_T(""));
	}
	if (threadData->receivedMsg.content.size() - index < 1)
	{
		CreateError("Could not read null-terminated string from received binary at position %i, amount of message remaining is smaller than variable to be read.", index);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(RecvMsg_Sub_ReadString(index, -1, false).c_str());
}
unsigned int Extension::RecvMsg_SizeInBytes()
{
	return threadData->receivedMsg.content.size();
}
const TCHAR * Extension::Lacewing_Version()
{
	static TCHAR version[128] = {};
	if (version[0] == _T('\0'))
	{
		std::tstringstream str;
		str << lw_version() << _T(" / Bluewing ");
#ifdef _UNICODE
		str << _T("Unicode ");
#endif
		str << _T("reimpl b") << lacewing::relayclient::buildnum;
		_tcscpy_s(version, str.str().c_str());
	}
	return Runtime.CopyString(version);
}
unsigned int Extension::SendBinaryMsg_Size()
{
	return SendMsgSize;
}
const TCHAR * Extension::Self_PreviousName()
{
	return Runtime.CopyString(UTF8ToTString(PreviousName).c_str());
}
const TCHAR * Extension::Peer_PreviousName()
{
	return Runtime.CopyString(!selPeer ? _T("") : UTF8ToTString(selPeer->prevname()).c_str());
}
const TCHAR * Extension::DenyReason()
{
	return Runtime.CopyString(UTF8ToTString(DenyReasonBuffer).c_str());
}
const TCHAR * Extension::Host_IP()
{
	return Runtime.CopyString(UTF8ToTString(HostIP).c_str());
}
unsigned int Extension::HostPort()
{
	auto addr = Cli.serveraddress();
	return addr ? addr->port() : -1;
}
const TCHAR * Extension::WelcomeMessage()
{
	return Runtime.CopyString(UTF8ToTString(Cli.welcomemessage()).c_str());
}
unsigned int Extension::RecvMsg_MemoryAddress()
{
	return (unsigned int)threadData->receivedMsg.content.data();
}

const TCHAR * Extension::RecvMsg_Cursor_StrASCIIByte()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(char))
	{
		CreateError("Could not read ASCII byte from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor);
		return Runtime.CopyString(_T(""));
	}

	const std::string_view partial(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor, 1);
	++threadData->receivedMsg.cursor;

	// ASCII goes up to 127, ANSI continues to 255. ANSI is locale-dependent, and if we only
	// interpret it as a fixed one like "en-us", that may not be the sender's locale, muddying
	// the water further for a Fusion developer.
	if (reinterpret_cast<const std::uint8_t &>(partial[0]) > 127)
	{
		CreateError("ANSI char %u read from cursor position %u, which is not an ASCII character. Use CursorStringWithSize() with size 1 to ensure consistency.",
			(std::uint32_t)reinterpret_cast<const std::uint8_t &>(partial[0]), threadData->receivedMsg.cursor - 1);
		return Runtime.CopyString(_T(""));
	}
	// Also check character is displayable (aka "printable").
	if (!isprint(partial[0]))
	{
		CreateError("ASCII char %u read, which cannot be displayed. Check cursor index %u is correct.",
			(std::uint32_t)reinterpret_cast<const std::uint8_t &>(partial[0]), threadData->receivedMsg.cursor - 1);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(ANSIToTString(partial).c_str());
}
unsigned int Extension::RecvMsg_Cursor_UnsignedByte()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(unsigned char))
		return CreateError("Could not read unsigned byte from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0U;

	++threadData->receivedMsg.cursor;
	return (unsigned int)(*(unsigned char *)(threadData->receivedMsg.content.data() + (threadData->receivedMsg.cursor - sizeof(unsigned char))));
}
int Extension::RecvMsg_Cursor_SignedByte()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(char))
		return CreateError("Could not read signed byte from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0;

	++threadData->receivedMsg.cursor;
	return (int)(*(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - sizeof(char)));
}
unsigned int Extension::RecvMsg_Cursor_UnsignedShort()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(unsigned short))
		return CreateError("Could not read unsigned short from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0U;

	threadData->receivedMsg.cursor += sizeof(unsigned short);
	return (unsigned int)(*(unsigned short *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - sizeof(unsigned short)));
}
int Extension::RecvMsg_Cursor_SignedShort()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(short))
		CreateError("Could not read signed short from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0;

	threadData->receivedMsg.cursor += sizeof(short);
	return (int)(*(short *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - sizeof(short)));
}
unsigned int Extension::RecvMsg_Cursor_UnsignedInteger()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(unsigned int))
		return CreateError("Could not read unsigned integer from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0U;

	threadData->receivedMsg.cursor += sizeof(unsigned int);
	return (*(unsigned int *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - sizeof(unsigned int)));
}
int Extension::RecvMsg_Cursor_SignedInteger()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(int))
		return CreateError("Could not read signed integer from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0;

	threadData->receivedMsg.cursor += sizeof(int);
	return (*(int *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - sizeof(int)));
}
float Extension::RecvMsg_Cursor_Float()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(float))
		return CreateError("Could not read unsigned byte from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0.0f;

	threadData->receivedMsg.cursor += sizeof(float);
	return (*(float *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - sizeof(float)));
}
const TCHAR * Extension::RecvMsg_Cursor_StringWithSize(int size)
{
	if (size < 0)
	{
		CreateError("Could not read string with size from received binary at cursor position %u, supplied size %i is less than 0.", threadData->receivedMsg.cursor, size);
		return Runtime.CopyString(_T(""));
	}
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < (size_t)size)
	{
		CreateError("Could not read string with size from received binary at cursor position %u, amount of message remaining is less than supplied size %i.", threadData->receivedMsg.cursor, size);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(RecvMsg_Sub_ReadString(threadData->receivedMsg.cursor, size, true).c_str());
}
const TCHAR * Extension::RecvMsg_Cursor_String()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < 1)
	{
		CreateError("Could not read null-terminated string from received binary at cursor position %zu, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(RecvMsg_Sub_ReadString(threadData->receivedMsg.cursor, -1, true).c_str());
}
unsigned int Extension::SendBinaryMsg_MemoryAddress()
{
	return (unsigned int)(long)SendMsg;
}
const TCHAR * Extension::RecvMsg_DumpToString(int index, const TCHAR * formatTStr)
{
	// Verify format is not blank
	if (formatTStr[0] == _T('\0'))
	{
		CreateError("Dumping message failed; format supplied was \"\".");
		return Runtime.CopyString(_T(""));
	}
	if (threadData->receivedMsg.content.size() - index <= 0)
	{
		CreateError("Dumping message failed; index %i is at message end index %i or beyond it.", index, threadData->receivedMsg.content.size() - 1);
		return Runtime.CopyString(_T(""));
	}

	bool allValid;
	const std::string formatANSI = TStringToANSI(formatTStr, &allValid);
	if (!allValid)
	{
		CreateError("Dumping message failed; format supplied \"%s\" had unrecognised characters. See help file.", TStringToUTF8(formatTStr).c_str());
		return Runtime.CopyString(_T(""));
	}
	const char * format = formatANSI.c_str();

	std::stringstream Output;
	size_t SizeOfFormat = strlen(format);
	bool Signed;
	size_t Count = 0;
	const char * Msg = &threadData->receivedMsg.content[index];
	// +c10c20c
	for (const char * i = format; i < format + SizeOfFormat;)
	{
		// Skip past last loop's numbers to get to variable type letter
		while (isdigit(i[0]))
			++i;

		// Determine if variable should be signed or unsigned
		if (i[0] != '+')
			Signed = true;
		else
		{
			Signed = false;
			++i;
		}

		// Count number of expected variables
		Count = max(atoi(i + 1), 1);

		// Char
		if (i[0] == 'c')
		{
			++i;
			if (threadData->receivedMsg.content.size() - index < Count)
			{
				CreateError("Could not dump; message was not large enough to contain all variables.");
				return Runtime.CopyString(_T(""));
			}
			if (Signed)
			{
				for (unsigned int j = 0; j < Count; ++j)
					Output << "Signed char: "sv << (int)Msg[j] << "\r\n"sv;
			}
			else
			{
				for (unsigned int j = 0; j < Count; ++j)
					Output << "Unsigned char: "sv << (int)((unsigned char *)Msg)[j] << "\r\n"sv;
			}
			Msg += Count;

			continue;
		}

		// Short
		if (i[0] == 'h')
		{
			++i;
			if (threadData->receivedMsg.content.size() - index < Count * sizeof(short))
			{
				CreateError("Could not dump; message was not large enough to contain variables.");
				return Runtime.CopyString(_T(""));
			}
			if (Signed)
			{
				for (unsigned int j = 0; j < Count; ++j)
					Output << "Signed short: "sv << (int)((short *)Msg)[j] << "\r\n"sv;
			}
			else
			{
				for (unsigned int j = 0; j < Count; ++j)
					Output << "Unsigned short: "sv << (int)((unsigned short *)Msg)[j] << "\r\n"sv;
			}
			Msg += Count * sizeof(short);
			continue;
		}

		// String (null-terminated)
		if (i[0] == 's')
		{
			++i;
			if (Signed == false)
				CreateError("'+' flag not expected next to 's'; strings cannot be unsigned.");
			for (unsigned int j = 0; j < Count; ++j)
			{
				size_t u8StrSize = strnlen(Msg, threadData->receivedMsg.content.size() - index + 1);
				if (u8StrSize == threadData->receivedMsg.content.size() - index + 1)
				{
					CreateError("Could not dump; message was not large enough to contain variables.");
					return Runtime.CopyString(_T(""));
				}

				if (!lw_u8str_validate(std::string_view(Msg, u8StrSize)))
				{
					CreateError("Could not dump; the null-terminated string starting at message index %i, read as %i chars long, was not valid UTF-8 text.", index, u8StrSize);
					return Runtime.CopyString(_T(""));
				}

				Output << "String: "sv << Msg << "\r\n"sv;
				Msg += u8StrSize + 1;
			}
			continue;
		}

		// Integer
		if (i[0] == 'i')
		{
			++i;
			if (threadData->receivedMsg.content.size() - index < Count * sizeof(int))
			{
				CreateError("Could not dump; message was not large enough to contain variables.");
				return Runtime.CopyString(_T(""));
			}
			if (Signed)
			{
				for (unsigned int j = 0; j < Count; ++j)
					Output << "Signed integer: "sv << ((int *)Msg)[j] << "\r\n"sv;
			}
			else
			{
				for (unsigned int j = 0; j < Count; ++j)
					Output << "Unsigned integer: "sv << ((unsigned int *)Msg)[j] << "\r\n"sv;
			}
			Msg += Count * sizeof(int);
			continue;
		}

		// Floating-point
		if (i[0] == 'f')
		{
			++i;
			if (threadData->receivedMsg.content.size() - index < Count * sizeof(float))
			{
				CreateError("Could not dump; message was not large enough to contain variables.");
				return Runtime.CopyString(_T(""));
			}
			if (!Signed)
				CreateError("'+' flag not expected next to 'f'; floats cannot be unsigned.");
			else
			{
				for (unsigned int j = 0; j < Count; ++j)
					Output << "Float: "sv << ((float *)Msg)[j] << "\r\n"sv;
			}
			Msg += Count * sizeof(float);
			continue;
		}

		// Did not find identifier; error out
		CreateError("Unrecognised character in dump format: '%hc'. Valid : c, h, s, i, f; operator +.", i[0]);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(UTF8ToTString(Output.str()).c_str());
}
unsigned int Extension::ChannelListing_ChannelCount()
{
	return Cli.channellistingcount();
}

// Yoink Lacewing's UTF-8 library
#include "../Lib/Shared/Lacewing/deps/utf8proc.h"

/// <summary> Number of UTF-8 code points (including things like combining accents) </summary>
int Extension::ConvToUTF8_GetCompleteCodePointCount(const TCHAR * tStr)
{
	// Empty string
	if (tStr[0] == _T('\0'))
		return 0;

	std::string u8str = TStringToUTF8(tStr);
	if (u8str.empty())
		return -1;

	size_t numCodePoints = 0;

	utf8proc_uint8_t * str = (utf8proc_uint8_t *)u8str.data();
	utf8proc_int32_t thisChar;
	utf8proc_ssize_t numBytesInCodePoint, remainder = u8str.size();
	while (remainder > 0)
	{
		numBytesInCodePoint = utf8proc_iterate(str, remainder, &thisChar);
		if (numBytesInCodePoint <= 0 || !utf8proc_codepoint_valid(thisChar))
			return -1;
		++numCodePoints;

		str += numBytesInCodePoint;
		remainder -= numBytesInCodePoint;
	}

	return numCodePoints;
}
/// <summary> Get number of UTF-8 graphemes (distinct graphical characters) </summary>
int Extension::ConvToUTF8_GetVisibleCharCount(const TCHAR * tStr)
{
	// Empty string
	if (tStr[0] == _T('\0'))
		return 0;

	const std::string u8str = TStringToUTF8(tStr);
	if (u8str.empty())
		return -1;

	size_t numChars = 0;

	utf8proc_uint8_t * str = (utf8proc_uint8_t *)u8str.data();
	utf8proc_int32_t lastChar = 0, thisChar, state = 0;
	utf8proc_ssize_t numBytesInCodePoint, remainder = u8str.size();
	while (remainder > 0)
	{
		numBytesInCodePoint = utf8proc_iterate(str, remainder, &thisChar);
		if (numBytesInCodePoint <= 0 || !utf8proc_codepoint_valid(thisChar))
			return -1;
		// str[0] - str[numBytesInCodePoint - 1] is a char
		// if utf8proc_grapheme returns true, there is a new grapheme starting between lastChar and thisChar
		if (lastChar != 0 && utf8proc_grapheme_break_stateful(lastChar, thisChar, &state))
			++numChars;
		lastChar = thisChar;

		str += numBytesInCodePoint;
		remainder -= numBytesInCodePoint;
	}

	// Add 1 char for the thisChar at end
	return ++numChars;
}
/// <summary> Get number of bytes in a UTF-8 string </summary>
int Extension::ConvToUTF8_GetByteCount(const TCHAR * tStr)
{
	// Empty string
	if (tStr[0] == _T('\0'))
		return 0;

	// TODO: can make this a little faster by only measuring length of output in WideToMultiByte(),
	// instead of doing that + the conversion itself as TStringToUTF8() does internally.
	size_t u8size = TStringToUTF8(tStr).size();
	if (u8size <= 0)
		return -1;
	return (int)u8size;
}
/// <summary> Tests if the UTF-8 equivalent matches the passed allow list, and if allow list is valid.
///			  If so, blank is returned, otherwise the error or faulty character. </summary>
const TCHAR * Extension::ConvToUTF8_TestAllowList(const TCHAR * toTest, const TCHAR * allowList)
{
	lacewing::codepointsallowlist list;
	std::string err = list.setcodepointsallowedlist(TStringToUTF8(allowList));
	if (!err.empty())
		return Runtime.CopyString(UTF8ToTString(err).c_str());

	utf8proc_int32_t rejectedChar = -1;
	int idx = list.checkcodepointsallowed(TStringToUTF8(toTest), &rejectedChar);
	if (idx == -1)
		return Runtime.CopyString(_T(""));

	TCHAR output[256];
	_stprintf_s(output, _T("Code point at index %d does not match allowed list. Code point U+%0.4X, decimal %u; valid = %s, Unicode category = %hs."),
		idx, rejectedChar, rejectedChar, utf8proc_codepoint_valid(rejectedChar) ? _T("yes") : _T("no"), utf8proc_category_string(rejectedChar));
	return Runtime.CopyString(output);
}
