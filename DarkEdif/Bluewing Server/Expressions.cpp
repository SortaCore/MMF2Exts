
#include "Common.h"

const TCHAR * Extension::Error()
{
	return Runtime.CopyString(!threadData->error.text.empty() ? UTF8ToTString(threadData->error.text).c_str() : _T(""));
}
unsigned int Extension::Channel_Count()
{
	return Srv.channelcount();
}
const TCHAR * Extension::Client_Name()
{
	return Runtime.CopyString(!selClient ? _T("") : UTF8ToTString(selClient->name()).c_str());
}
const TCHAR * Extension::ReceivedStr()
{
	return Runtime.CopyString(ReadStringFromRecvBinary(0, -1, false).c_str());
}
int Extension::ReceivedInt()
{
	if (threadData->receivedMsg.content.size() != 4)
		return CreateError("Received() was used on a message that is not a number message."), 0;

	return *(int *)threadData->receivedMsg.content.data();
}
unsigned int Extension::Subchannel()
{
	return (int)threadData->receivedMsg.subchannel;
}
unsigned int Extension::Client_ID()
{
	return selClient ? selClient->id() : -1;
}
const TCHAR * Extension::RequestedClientName()
{
	return Runtime.CopyString(UTF8ToTString(threadData->requested.name).c_str());
}
const TCHAR * Extension::RequestedChannelName()
{
	// 4 = Leave Channel condition ID. There's no point rewriting the name in a Leave Channel request.
	if (threadData->CondTrig[0] == 4)
		CreateError("Requested channel name is not available in a Leave Channel Request. Use Channel_Name$() instead.");

	return Runtime.CopyString(UTF8ToTString(threadData->requested.name).c_str());
}
unsigned int Extension::Client_ConnectionTime()
{
	return selClient ? (int)selClient->getconnecttime() : -1;
}
unsigned int Extension::Client_ChannelCount()
{
	return selClient ? selClient->channelcount() : -1;
}
const TCHAR * Extension::Client_GetLocalData(const TCHAR * key)
{
	if (!key || !selClient)
		return Runtime.CopyString(_T(""));
	return Runtime.CopyString(globals->GetLocalData(selClient, key).c_str());
}
const TCHAR * Extension::Channel_GetLocalData(const TCHAR * key)
{
	if (!key || !selChannel)
		return Runtime.CopyString(_T(""));
	return Runtime.CopyString(globals->GetLocalData(selChannel, key).c_str());
}
const TCHAR * Extension::Channel_Name()
{
	return Runtime.CopyString(selChannel ? UTF8ToTString(selChannel->name()).c_str() : _T(""));
}
unsigned int Extension::Channel_ClientCount()
{
	return selChannel ? selChannel->clientcount() : -1;
}
const TCHAR * Extension::StrANSIByte(int index)
{
	if (index < 0)
	{
		CreateError("Could not read text byte from received binary, index %i is less than 0.", index);
		return Runtime.CopyString(_T(""));
	}
	if (threadData->receivedMsg.content.size() - index < sizeof(char))
	{
		CreateError("Could not read ASCII text byte from received binary at position %i, amount of message remaining is smaller than variable to be read.", index);
		return Runtime.CopyString(_T(""));
	}
	std::string ansiStr = threadData->receivedMsg.content.substr(index, 1);
	return Runtime.CopyString(ANSIToTString(ansiStr).c_str());
}
unsigned int Extension::UnsignedByte(int index)
{
	if (index < 0)
		return CreateError("Could not read unsigned byte from received binary, index %i is less than 0.", index), 0U;
	if (threadData->receivedMsg.content.size() - index < sizeof(unsigned char))
		return CreateError("Could not read unsigned byte from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0U;

	return (unsigned int)(*(unsigned char *)(threadData->receivedMsg.content.data() + index));
}
int Extension::SignedByte(int index)
{
	if (index < 0)
		return CreateError("Could not read signed byte from received binary, index %i is less than 0.", index), 0;
	if (threadData->receivedMsg.content.size() - index < sizeof(signed char))
		return CreateError("Could not read signed byte from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	return (int)(*(threadData->receivedMsg.content.data() + index));
}
unsigned int Extension::UnsignedShort(int index)
{
	if (index < 0)
		return CreateError("Could not read unsigned short from received binary, index %i is less than 0.", index), 0;
	if (threadData->receivedMsg.content.size() - index < sizeof(unsigned short))
		return CreateError("Could not read unsigned short from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	return (unsigned int)(*(unsigned short *)(threadData->receivedMsg.content.data() + index));
}
int Extension::SignedShort(int index)
{
	if (index < 0)
		return CreateError("Could not read signed short from received binary, index %i is less than 0.", index), 0;
	if (threadData->receivedMsg.content.size() - index < sizeof(signed short))
		return CreateError("Could not read signed short from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	return (int)(*(short *)(threadData->receivedMsg.content.data() + index));
}
unsigned int Extension::UnsignedInteger(int index)
{
	if (index < 0)
		return CreateError("Could not read unsigned integer from received binary, index %i is less than 0.", index), 0;
	if (threadData->receivedMsg.content.size() - index < sizeof(unsigned int))
		return CreateError("Could not read unsigned integer from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	return (*(unsigned int *)(threadData->receivedMsg.content.data() + index));
}
int Extension::SignedInteger(int index)
{
	if (index < 0)
		return CreateError("Could not read signed integer from received binary, index %i is less than 0.", index), 0;
	if (threadData->receivedMsg.content.size() - index < sizeof(signed short))
		return CreateError("Could not read signed integer from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	return (*(int *)(threadData->receivedMsg.content.data() + index));
}
float Extension::Float(int index)
{
	if (index < 0)
		return CreateError("Could not read float from received binary, index %i is less than 0.", index), 0.0f;
	if (threadData->receivedMsg.content.size() - index < sizeof(signed short))
		return CreateError("Could not read float from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0.0f;

	return (*(float *)(threadData->receivedMsg.content.data() + index));
}
const TCHAR * Extension::StringWithSize(int index, int size)
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
		CreateError("Could not read float from received binary at position %i, amount of message remaining is smaller than supplied string size %i.", index, size);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(ReadStringFromRecvBinary(index, size, false).c_str());
}
const TCHAR * Extension::String(int index)
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
	if (strnlen(threadData->receivedMsg.content.data() + index, threadData->receivedMsg.content.size() - index + 1) == threadData->receivedMsg.content.size() - index + 1)
	{
		CreateError("Could not read null-terminated string from received binary at position %i, null terminator not found.", index);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(ReadStringFromRecvBinary(index, 1, false).c_str());
}
unsigned int Extension::ReceivedBinarySize()
{
	return threadData->receivedMsg.content.size();
}
const TCHAR * Extension::Lacewing_Version()
{
	static const TCHAR * version = nullptr;
	if (version == nullptr)
	{
		std::stringstream str;
		str << lw_version() << " / Bluewing reimpl b"sv << lacewing::relayserver::buildnum;
		version = _tcsdup(UTF8ToTString(str.str()).c_str());
	}
	return Runtime.CopyString(version);
}
unsigned int Extension::BinaryToSend_Size()
{
	return SendMsgSize;
}
const TCHAR * Extension::Client_IP()
{
	if (selClient == nullptr)
	{
		CreateError("Could not read client IP, no client selected.");
		return Runtime.CopyString(_T(""));
	}

	char addr[64];
	lw_addr_prettystring(selClient->getaddress().data(), addr, 64);
	return Runtime.CopyString(ANSIToTString(addr).c_str());
}
unsigned int Extension::Port()
{
	return Srv.port();
}
const TCHAR * Extension::Welcome_Message()
{
	return Runtime.CopyString(UTF8ToTString(Srv.getwelcomemessage()).c_str());
}
unsigned int Extension::ReceivedBinaryAddress()
{
	return (unsigned int)(long)threadData->receivedMsg.content.data();
}
const TCHAR * Extension::CursorStrByte()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(char))
	{
		CreateError("Could not read text byte from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(ReadStringFromRecvBinary(threadData->receivedMsg.cursor, 1, true).c_str());
}
unsigned int Extension::CursorUnsignedByte()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(unsigned char))
		return CreateError("Could not read unsigned byte from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0U;

	++threadData->receivedMsg.cursor;
	return (unsigned int)(*(unsigned char *)(threadData->receivedMsg.content.data() + (threadData->receivedMsg.cursor - 1)));
}
int Extension::CursorSignedByte()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(char))
		return CreateError("Could not read signed byte from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0;

	++threadData->receivedMsg.cursor;
	return (int)(*(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - 1));
}
unsigned int Extension::CursorUnsignedShort()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(unsigned short))
		return CreateError("Could not read unsigned short from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0U;

	threadData->receivedMsg.cursor += 2U;
	return (unsigned int)(*(unsigned short *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - 2));
}
int Extension::CursorSignedShort()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(short))
		CreateError("Could not read signed short from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0;

	threadData->receivedMsg.cursor += 2;
	return (int)(*(short *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - 2));
}
unsigned int Extension::CursorUnsignedInteger()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(unsigned int))
		return CreateError("Could not read unsigned integer from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0U;

	threadData->receivedMsg.cursor += 4;
	return (*(unsigned int *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - 4));
}
int Extension::CursorSignedInteger()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(int))
		return CreateError("Could not read signed integer from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0;

	threadData->receivedMsg.cursor += 4;
	return (*(int *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - 4));
}
float Extension::CursorFloat()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(float))
		return CreateError("Could not read unsigned byte from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0.0f;

	threadData->receivedMsg.cursor += 4;
	return (*(float *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - 4));
}
const TCHAR * Extension::CursorStringWithSize(int size)
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

	return Runtime.CopyString(ReadStringFromRecvBinary(threadData->receivedMsg.cursor, size, true).c_str());
}
const TCHAR * Extension::CursorString()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < 1)
	{
		CreateError("Could not read null-terminated string from received binary at cursor position %zu, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(ReadStringFromRecvBinary(threadData->receivedMsg.cursor, -1, true).c_str());
}
const TCHAR * Extension::Client_ProtocolImplementation()
{
	if (selClient == nullptr)
	{
		CreateError("Could not read client protocol implementation, no client selected.");
		return Runtime.CopyString(_T(""));
	}
	return Runtime.CopyString(UTF8ToTString(selClient->getimplementation()).c_str());
}
unsigned int Extension::BinaryToSend_Address()
{
	return (unsigned int)(long)SendMsg;
}
const TCHAR * Extension::DumpMessage(int index, const TCHAR * formatTStr)
{
	if (!formatTStr || formatTStr[0] == _T('\0'))
	{
		CreateError("Dumping message failed; format supplied was null or \"\".");
		return Runtime.CopyString(_T(""));
	}
	if (threadData->receivedMsg.content.size() - index <= 0)
	{
		CreateError("Dumping message failed; passed index %i exceeds end index in message %i.", index, threadData->receivedMsg.content.size() - 1);
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
 	for (const char * i = format; i < format+SizeOfFormat;)
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
		Count = max(atoi(i+1),1);

		// Char
		if (i[0] == 'c')
		{
			++i;
			if (threadData->receivedMsg.content.size()-index < Count)
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
			if (threadData->receivedMsg.content.size()-index < Count*sizeof(short))
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
			Msg += Count*sizeof(short);
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
				if (u8StrSize == threadData->receivedMsg.content.size()-index+1)
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
			if (threadData->receivedMsg.content.size()-index < Count*sizeof(int))
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
			Msg += Count*sizeof(int);
			continue;
		}

		// Floating-point
		if (i[0] == 'f')
		{
			++i;
			if (threadData->receivedMsg.content.size()-index < Count*sizeof(float))
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
			Msg += Count*sizeof(float);
			continue;
		}

		// Did not find identifier; error out
		CreateError("Unrecognised character in dump format: '%hc'. Valid : c, h, s, i, f; operator +.", i[0]);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(UTF8ToTString(Output.str()).c_str());
}
unsigned int Extension::AllClientCount()
{
	return Srv.clientcount();
}
const TCHAR * Extension::GetDenyReason()
{
	return Runtime.CopyString(UTF8ToTString(DenyReason).c_str());
}

// Yoink Lacewing's UTF8 library
#include "../Lib/Shared/Lacewing/deps/utf8proc.h"

/// <summary> Number of UTF-8 characters (including things like combining accents) </summary>
int Extension::ConvToUTF8_GetCompleteCharCount(const TCHAR * tStr)
{
	// Empty string
	if (tStr[0] == _T('\0'))
		return 0;

	std::string u8str = TStringToUTF8(tStr);
	if (u8str.empty())
		return -1;

	size_t numChars = 0;

	utf8proc_uint8_t * str = (utf8proc_uint8_t *)u8str.data();
	utf8proc_int32_t thisChar;
	utf8proc_ssize_t numBytesInCodePoint, remainder = u8str.size();
	while (remainder <= 0)
	{
		numBytesInCodePoint = utf8proc_iterate(str, remainder, &thisChar);
		if (numBytesInCodePoint <= 0 || !utf8proc_codepoint_valid(thisChar))
			return -1;
		++numChars;

		str += numBytesInCodePoint;
		remainder -= numBytesInCodePoint;
	}

	return numChars;
}
/// <summary> Get number of UTF-8 graphemes (distinct graphical characters) </summary>
int Extension::ConvToUTF8_GetVisibleCharCount(const TCHAR * tStr)
{
	// Empty string
	if (tStr[0] == _T('\0'))
		return 0;

	std::string u8str = TStringToUTF8(tStr);
	if (u8str.empty())
		return -1;

	size_t numChars = 0;

	utf8proc_uint8_t * str = (utf8proc_uint8_t *)u8str.data();
	utf8proc_int32_t lastChar = 0, thisChar, state = 0;
	utf8proc_ssize_t numBytesInCodePoint, remainder = u8str.size();
	while (remainder <= 0)
	{
		numBytesInCodePoint = utf8proc_iterate(str, remainder, &thisChar);
		if (numBytesInCodePoint <= 0 || !utf8proc_codepoint_valid(thisChar))
			return -1;
		// str[0] - str[numBytesInCodePoint - 1] is a char
		// if utf8proc_graphme returns true, there is a new graphme starting between lastChar and thisChar
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
