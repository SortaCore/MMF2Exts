
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
const TCHAR * Extension::ReceivedStr()
{
	return Runtime.CopyString(UTF8ToTString(threadData->receivedMsg.content).c_str());
}
int Extension::ReceivedInt()
{
	if (threadData->receivedMsg.content.size() != 4)
	{
		CreateError("Received() was used on a message that is not a number message.");
		return 0;
	}
	else
		return *(int *)threadData->receivedMsg.content.data();
}
unsigned int Extension::Subchannel()
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
const TCHAR * Extension::StrASCIIByte(int index)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0U;
	}
	if (threadData->receivedMsg.content.size() - index < sizeof(char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0U;
	}

	return Runtime.CopyString(ANSIToTString(std::string(threadData->receivedMsg.content.data() + index, 1)).c_str());
}
unsigned int Extension::UnsignedByte(int index)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0U;
	}
	if (threadData->receivedMsg.content.size() - index < sizeof(unsigned char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0U;
	}
	return (unsigned int)(*(unsigned char *)(threadData->receivedMsg.content.data() + index));
}
int Extension::SignedByte(int index)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0;
	}
	else if (threadData->receivedMsg.content.size() - index < sizeof(char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
		return (int)(*(threadData->receivedMsg.content.data() + index));
}
unsigned int Extension::UnsignedShort(int index)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0U;
	}
	else if (threadData->receivedMsg.content.size() - index < sizeof(unsigned short))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0U;
	}
	else
		return (unsigned int)(*(unsigned short *)(threadData->receivedMsg.content.data() + index));
}
int Extension::SignedShort(int index)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0;
	}
	else if (threadData->receivedMsg.content.size() - index < sizeof(short))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
		return (int)(*(short *)(threadData->receivedMsg.content.data() + index));
}
unsigned int Extension::UnsignedInteger(int index)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0U;
	}
	else if (threadData->receivedMsg.content.size() - index < sizeof(unsigned int))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0U;
	}
	else
		return (*(unsigned int *)(threadData->receivedMsg.content.data() + index));
}
int Extension::SignedInteger(int index)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0;
	}
	else if (threadData->receivedMsg.content.size() - index < sizeof(int))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
		return (*(int *)(threadData->receivedMsg.content.data() + index));
}
float Extension::Float(int index)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0.0f;
	}
	else if (threadData->receivedMsg.content.size() - index < sizeof(float))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0.0f;
	}
	else
		return (*(float *)(threadData->receivedMsg.content.data() + index));
}
const TCHAR * Extension::StringWithSize(int index, int size)
{
	return Runtime.CopyString(ReadStringFromRecvBinary(index, size, false).c_str());
}
const TCHAR * Extension::String(int index)
{
	return Runtime.CopyString(ReadStringFromRecvBinary(index, -1, false).c_str());
}
unsigned int Extension::ReceivedBinarySize()
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
unsigned int Extension::SendBinarySize()
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
unsigned int Extension::ReceivedBinaryAddress()
{
	return (unsigned int)threadData->receivedMsg.content.data();
}
const TCHAR * Extension::CursorStrASCIIByte()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return Runtime.CopyString(_T(""));
	}

	// greater than 0x7F isn't ASCII, it's ANSI. Although ANSI is locale-dependent, switching it to a fixed one
	// like en-us may not be the sender's locale, muddying the water further.
	std::string partial = threadData->receivedMsg.content.substr(threadData->receivedMsg.cursor - 1, 1);
	++threadData->receivedMsg.cursor;
	return Runtime.CopyString(ANSIToTString(partial).c_str());
}
unsigned int Extension::CursorUnsignedByte()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(unsigned char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read.");
		return 0;
	}
	else
	{
		++threadData->receivedMsg.cursor;
		return (unsigned int)(*(unsigned char *)(threadData->receivedMsg.content.data() + (threadData->receivedMsg.cursor-1)));
	}
}
int Extension::CursorSignedByte()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read.");
		return 0;
	}
	else
	{
		++threadData->receivedMsg.cursor;
		return (int)(*(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - 1));
	}
}
unsigned int Extension::CursorUnsignedShort()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(unsigned short))
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0;
	}
	else
	{
		threadData->receivedMsg.cursor += 2;
		return (unsigned int)(*(unsigned short *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - 2));
	}
}
int Extension::CursorSignedShort()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(short))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
	{
		threadData->receivedMsg.cursor += 2;
		return (int)(*(short *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - 2));
	}
}
unsigned int Extension::CursorUnsignedInteger()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(unsigned int))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
	{
		threadData->receivedMsg.cursor += 4;
		return (*(unsigned int *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - 4));
	}
}
int Extension::CursorSignedInteger()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(int))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
	{
		threadData->receivedMsg.cursor += 4;
		return (*(int *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - 4));
	}
}
float Extension::CursorFloat()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(float))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0.0f;
	}
	else
	{
		threadData->receivedMsg.cursor += 4;
		return (*(float *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - 4));
	}
}
const TCHAR * Extension::CursorStringWithSize(int size)
{
	return Runtime.CopyString(ReadStringFromRecvBinary(threadData->receivedMsg.cursor, size, true).c_str());
}
const TCHAR * Extension::CursorString()
{
	return Runtime.CopyString(ReadStringFromRecvBinary(threadData->receivedMsg.cursor, -1, true).c_str());
}
unsigned int Extension::SendBinaryAddress()
{
	return (unsigned int)(long)SendMsg;
}
const TCHAR * Extension::DumpMessage(int index, const TCHAR * formatTStr)
{
	// Verify format is not blank
	if (formatTStr[0] == _T('\0'))
	{
		CreateError("Dumping message failed; format supplied was \"\".");
		return Runtime.CopyString(_T(""));
	}
	if (threadData->receivedMsg.content.size() - index <= 0)
	{
		CreateError("Dumping message failed; index %i is beyond or at message end index %i.", index, threadData->receivedMsg.content.size() - 1);
		return Runtime.CopyString(_T(""));
	}


	const std::string formatANSI = TStringToANSI(formatTStr);
	std::stringstream output;
	const size_t sizeOfFormat = formatANSI.size();
	bool _signed;
	size_t count = 0;
	const char * Msg = &threadData->receivedMsg.content[index];
	// +c10c20c
 	for (const char * i = formatANSI.c_str(); i < formatANSI.c_str() + sizeOfFormat;)
	{
		// Skip past last loop's numbers to get to variable type letter
		while (isdigit(i[0]))
			++i;

		// Determine if variable should be signed or unsigned
		if (i[0] != '+')
			_signed = true;
		else
		{
			_signed = false;
			++i;
		}

		// count number of expected variables
		count = max(atoi(i+1),1);

		// Char
		if (i[0] == 'c')
		{
			++i;
			if (threadData->receivedMsg.content.size()-index < count)
			{
				CreateError("Could not dump; message was not large enough to contain variables.");
				return Runtime.CopyString(_T(""));
			}
			if (_signed)
			{
				for (unsigned int j = 0; j < count; ++j)
					output << "Signed ANSI char: "sv << (int)Msg[j] << "\r\n"sv;
			}
			else
			{
				for (unsigned int j = 0; j < count; ++j)
					output << "Unsigned ANSI char: "sv << (int)((unsigned char *)Msg)[j] << "\r\n"sv;
			}
			Msg += count;

			continue;
		}

		// Short
		if (i[0] == 'h')
		{
			++i;
			if (threadData->receivedMsg.content.size()-index < count*sizeof(short))
			{
				CreateError("Could not dump; message was not large enough to contain variables.");
				return Runtime.CopyString(_T(""));
			}
			if (_signed)
			{
				for (unsigned int j = 0; j < count; ++j)
					output << "Signed short: "sv << (int)((short *)Msg)[j] << "\r\n"sv;
			}
			else
			{
				for (unsigned int j = 0; j < count; ++j)
					output << "Unsigned short: "sv << (int)((unsigned short *)Msg)[j] << "\r\n"sv;
			}
			Msg += count*sizeof(short);
			continue;
		}

		// String (null-terminated)
		if (i[0] == 's')
		{
			++i;
			if (_signed == false)
				CreateError("'+' flag not expected next to 's'; strings cannot be unsigned.");
			for (size_t j = 0; j < count; ++j)
			{
				if (strnlen(Msg, threadData->receivedMsg.content.size() - index + 1) == threadData->receivedMsg.content.size() - index + 1)
				{
					CreateError("Could not dump; message was not large enough to contain variables.");
					return Runtime.CopyString(_T(""));
				}

				output << "String: "sv << Msg << "\r\n"sv;
				Msg += strlen(Msg)+1;
			}
			continue;
		}

		// Integer
		if (i[0] == 'i')
		{
			++i;
			if (threadData->receivedMsg.content.size()-index < count*sizeof(int))
			{
				CreateError("Could not dump; message was not large enough to contain variables.");
				return Runtime.CopyString(_T(""));
			}
			if (_signed)
			{
				for (unsigned int j = 0; j < count; ++j)
					output << "Signed integer: "sv << ((int *)Msg)[j] << "\r\n"sv;
			}
			else
			{
				for (unsigned int j = 0; j < count; ++j)
					output << "Unsigned integer: "sv << ((unsigned int *)Msg)[j] << "\r\n"sv;
			}
			Msg += count*sizeof(int);
			continue;
		}

		// Floating-point
		if (i[0] == 'f')
		{
			++i;
			if (threadData->receivedMsg.content.size()-index < count*sizeof(float))
			{
				CreateError("Could not dump; message was not large enough to contain variables.");
				return Runtime.CopyString(_T(""));
			}
			if (!_signed)
			{
				CreateError("'+' flag not expected next to 'f'; floats cannot be unsigned.");
			}
			else
			{
				for (unsigned int j = 0; j < count; ++j)
					output << "Float: "sv << ((float *)Msg)[j] << "\r\n"sv;
			}
			Msg += count*sizeof(float);
			continue;
		}

		// Did not find identifier; error out
		std::stringstream error;
		error << "Unrecognised variable in format: '"sv << i[0] << "'. Valid: c, h, s, i, f; and operator +."sv;
		CreateError(error.str().c_str());
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(UTF8ToTString(output.str()).c_str());
}
unsigned int Extension::ChannelListing_ChannelCount()
{
	return Cli.channellistingcount();
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
