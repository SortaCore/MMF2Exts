#include "Common.hpp"

const TCHAR * Extension::Error()
{
	// Not the On Error event (condition 0)
	if (threadData->condTrig[0] != 0)
		return Runtime.CopyString(_T(""));
	return Runtime.CopyString(DarkEdif::UTF8ToTString(threadData->AsC<ErrorEvent>().text).c_str());
}
const TCHAR * Extension::ReplacedExprNoParams()
{
	return Runtime.CopyString(_T(""));
}
const TCHAR * Extension::Self_Name()
{
	return Runtime.CopyString(DarkEdif::UTF8ToTString(Cli.name()).c_str());
}
unsigned int Extension::Self_ChannelCount()
{
	return (std::uint32_t)Cli.channelcount();
}
const TCHAR * Extension::Peer_Name()
{
	return Runtime.CopyString(selPeer ? DarkEdif::UTF8ToTString(selPeer->name()).c_str() : _T(""));
}
const TCHAR * Extension::RecvMsg_ReadAsString()
{
	if (!threadData->IsRecvMsg())
	{
		CreateError("Received$() was used on an event that is not a received message event.");
		return Runtime.CopyString(_T(""));
	}
	const auto& msg = threadData->GetRecvMsg();
	if (msg.variant != 0)
	{
		CreateError("Received$() was used on a message that is not a text message.");
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(DarkEdif::UTF8ToTString(msg.content).c_str());
}
int Extension::RecvMsg_ReadAsInteger()
{
	if (!threadData->IsRecvMsg())
	{
		CreateError("Received() was used on an event that is not a received message event.");
		return 0;
	}
	const auto& msg = threadData->GetRecvMsg();
	if (msg.variant != 1)
	{
		CreateError("Received() was used on a message that is not a number message.");
		return 0;
	}

	return *(int *)msg.content.data();
}
unsigned int Extension::RecvMsg_Subchannel()
{
	if (!threadData->IsRecvMsg())
	{
		CreateError("Subchannel() was used on an event that is not a received message event.");
		return 0;
	}
	return (unsigned int)threadData->GetRecvMsg().subchannel;
}
int Extension::Peer_ID()
{
	return selPeer ? selPeer->id() : -1;
}
const TCHAR * Extension::Channel_Name()
{
	if (selChannel)
		return Runtime.CopyString(DarkEdif::UTF8ToTString(selChannel->name()).c_str());

	// If channel join was denied (condition 5), selChannel will be null, and the denied channel name will be in denied.name
	if (threadData->condTrig[0] == 5)
		return Runtime.CopyString(DarkEdif::UTF8ToTString(threadData->AsC<NameDeniedEvent>().name).c_str());
	return Runtime.CopyString(_T(""));
}
int Extension::Channel_PeerCount()
{
	return selChannel ? selChannel->peercount() : -1;
}
const TCHAR * Extension::ChannelListing_Name()
{
	if (!channelListing)
		return Runtime.CopyString(_T(""));
	return Runtime.CopyString(DarkEdif::UTF8ToTString(channelListing->name()).c_str());
}
int Extension::ChannelListing_PeerCount()
{
	if (!channelListing)
		return -1;
	return channelListing->peercount();
}
int Extension::Self_ID()
{
	int id = Cli.id();
	return id == UINT16_MAX ? -1 : id;
}
const TCHAR * Extension::RecvMsg_StrASCIIByte(int index)
{
	if (!threadData->IsRecvMsg())
	{
		CreateError("Could not read ASCII text byte from received binary, not a received message event.");
		return Runtime.CopyString(_T(""));
	}
	if (index < 0)
	{
		CreateError("Could not read ASCII text byte from received binary, index %i is less than 0.", index);
		return Runtime.CopyString(_T(""));
	}
	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - index < sizeof(char))
	{
		CreateError("Could not read ASCII text byte from received binary at position %i, amount of message remaining is smaller than variable to be read.", index);
		return Runtime.CopyString(_T(""));
	}

	const std::string_view partial(msg.content.data() + index, 1);
	const std::uint8_t charUnsigned = reinterpret_cast<const std::uint8_t &>(partial[0]);

	// ASCII goes up to 127, ANSI continues to 255. ANSI is locale-dependent, and if we only
	// interpret it as a fixed one like "en-us", that may not be the sender's locale, muddying
	// the water further for a Fusion developer.
	if (charUnsigned > 127)
	{
		CreateError("ANSI char %hhu read at position %d, which is not an ASCII character. Use StringWithSize() with size 1 to ensure consistency.",
			charUnsigned, index);
		return Runtime.CopyString(_T(""));
	}
	// Also check character is displayable (aka "printable").
	if (!std::isprint(charUnsigned))
	{
		CreateError("ASCII char %hhu read, which cannot be displayed. Check index %d is correct.",
			charUnsigned, index);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(DarkEdif::ANSIToTString(partial).c_str());
}
unsigned int Extension::RecvMsg_UnsignedByte(int index)
{
	if (!threadData->IsRecvMsg())
		return CreateError("Could not read unsigned byte from received binary, not a received message event."), 0U;
	if (index < 0)
		return CreateError("Could not read unsigned byte from received binary, index %i is less than 0.", index), 0U;
	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - index < sizeof(unsigned char))
		return CreateError("Could not read unsigned byte from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0U;

	return (unsigned int)(*(unsigned char *)(msg.content.data() + index));
}

int Extension::RecvMsg_SignedByte(int index)
{
	if (!threadData->IsRecvMsg())
		return CreateError("Could not read signed byte from received binary, not a received message event."), 0;
	if (index < 0)
		return CreateError("Could not read signed byte from received binary, index %i is less than 0.", index), 0;
	
	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - index < sizeof(char))
		return CreateError("Could not read signed byte from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	// Note: on ARM arch, char pretends to be signed by default (IntelliSense and compiler checks say it's signed), but is actually unsigned.
	// https://developer.arm.com/documentation/dui0041/c/ARM-Procedure-Call-Standard/C-language-calling-conventions/Argument-representation?lang=en
	// This is overridden with -fsigned-char in FusionSDK.props

	return (int)(*(msg.content.data() + index));
}
unsigned int Extension::RecvMsg_UnsignedShort(int index)
{
	if (!threadData->IsRecvMsg())
		return CreateError("Could not read unsigned short from received binary, not a received message event."), 0U;
	if (index < 0)
		return CreateError("Could not read unsigned short from received binary, index %i is less than 0.", index), 0;
	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - index < sizeof(unsigned short))
		return CreateError("Could not read unsigned short from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	return (unsigned int)(*(unsigned short *)(msg.content.data() + index));
}
int Extension::RecvMsg_SignedShort(int index)
{
	if (!threadData->IsRecvMsg())
		return CreateError("Could not read signed short from received binary, not a received message event."), 0;
	if (index < 0)
		return CreateError("Could not read signed short from received binary, index %i is less than 0.", index), 0;
	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - index < sizeof(short))
		return CreateError("Could not read signed short from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	return (int)(*(short *)(msg.content.data() + index));
}
unsigned int Extension::RecvMsg_UnsignedInteger(int index)
{
	if (!threadData->IsRecvMsg())
		return CreateError("Could not read unsigned integer from received binary, not a received message event."), 0U;
	if (index < 0)
		return CreateError("Could not read unsigned integer from received binary, index %i is less than 0.", index), 0U;
	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - index < sizeof(unsigned int))
		return CreateError("Could not read unsigned integer from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0U;

	return (*(unsigned int *)(msg.content.data() + index));
}
int Extension::RecvMsg_SignedInteger(int index)
{
	if (!threadData->IsRecvMsg())
		return CreateError("Could not read signed integer from received binary, not a received message event."), 0;
	if (index < 0)
		return CreateError("Could not read signed integer from received binary, index %i is less than 0.", index), 0;
	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - index < sizeof(int))
		return CreateError("Could not read signed integer from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	return (*(int *)(msg.content.data() + index));
}
float Extension::RecvMsg_Float(int index)
{
	if (!threadData->IsRecvMsg())
		return CreateError("Could not read float from received binary, not a received message event."), 0.f;
	if (index < 0)
		return CreateError("Could not read float from received binary, index %i is less than 0.", index), 0.f;
	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - index < sizeof(float))
		return CreateError("Could not read float from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0.0f;

	return (*(float *)(msg.content.data() + index));
}
const TCHAR * Extension::RecvMsg_StringWithSize(int index, int size)
{
	if (!threadData->IsRecvMsg())
	{
		CreateError("Could not read string with size from received binary, not a received message event.");
		return Runtime.CopyString(_T(""));
	}
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
	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - index < (size_t)size)
	{
		CreateError("Could not read string with size from received binary at position %i, amount of message remaining (%u bytes) is smaller than supplied string size %i.",
			index, (std::uint32_t)(msg.content.size() - index), size);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(RecvMsg_Sub_ReadString(msg, index, size, false).c_str());
}
const TCHAR * Extension::RecvMsg_String(int index)
{
	if (!threadData->IsRecvMsg())
	{
		CreateError("Could not read string with size from received binary, not a received message event.");
		return Runtime.CopyString(_T(""));
	}
	if (index < 0)
	{
		CreateError("Could not read null-terminated string from received binary, index %i is less than 0.", index);
		return Runtime.CopyString(_T(""));
	}
	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - index < 1)
	{
		CreateError("Could not read null-terminated string from received binary at position %i, amount of message remaining is smaller than variable to be read.", index);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(RecvMsg_Sub_ReadString(msg, index, -1, false).c_str());
}
unsigned int Extension::RecvMsg_SizeInBytes()
{
	if (!threadData->IsRecvMsg())
	{
		CreateError("Could not read received message size, not a received message event.");
		return 0;
	}
	return (std::uint32_t)threadData->GetRecvMsg().content.size();
}
const TCHAR * Extension::Lacewing_Version()
{
	static TCHAR version[128] = {};
	if (version[0] == _T('\0'))
	{
		std::tstringstream str;
		str << DarkEdif::UTF8ToTString(lw_version()) << _T(" / Bluewing ");
#ifdef _UNICODE
		str << _T("Unicode ");
#endif
		str << _T("reimpl b") << lacewing::relayclient::buildnum;
		_tcscpy_s(version, std::size(version), str.str().c_str());
	}
	return Runtime.CopyString(version);
}
unsigned int Extension::SendBinaryMsg_Size()
{
	return (std::uint32_t)SendMsgSize;
}
const TCHAR * Extension::Self_PreviousName()
{
	return Runtime.CopyString(DarkEdif::UTF8ToTString(PreviousName).c_str());
}
const TCHAR * Extension::Peer_PreviousName()
{
	return Runtime.CopyString(!selPeer ? _T("") : DarkEdif::UTF8ToTString(selPeer->prevname()).c_str());
}
const TCHAR * Extension::DenyReason()
{
	// Not a Denied event, return blank.
	// These are condition IDs for connection denied, channel join denied,
	// name set denied, and channel leave denied, respectively.
	if (threadData->condTrig[0] != 2 && threadData->condTrig[0] != 5 &&
		threadData->condTrig[0] != 7 && threadData->condTrig[0] != 44)
	{
		return Runtime.CopyString(_T(""));
	}
	return Runtime.CopyString(DarkEdif::UTF8ToTString(DenyReasonBuffer).c_str());
}
const TCHAR * Extension::Host_IP()
{
	return Runtime.CopyString(DarkEdif::UTF8ToTString(HostIP).c_str());
}
unsigned int Extension::Host_Port()
{
	return HostPort;
}
const TCHAR * Extension::WelcomeMessage()
{
	return Runtime.CopyString(DarkEdif::UTF8ToTString(Cli.welcomemessage()).c_str());
}
unsigned int Extension::RecvMsg_MemoryAddress()
{
#if __aarch64__ || _M_AMD64 || __amd64__
	CreateError("Warning: can't properly return received message memory address; address is 64-bit and Fusion only deals in 32-bit integers.");
#endif
	if (!threadData->IsRecvMsg())
		return CreateError("Can't return received message memory address, not a received message event."), 0U;
	return (unsigned int)((unsigned long)threadData->GetRecvMsg().content.data());
}

const TCHAR * Extension::RecvMsg_Cursor_StrASCIIByte()
{
	if (!threadData->IsRecvMsg())
	{
		CreateError("Could not read cursor ASCII byte from received binary, not a received message event.");
		return Runtime.CopyString(_T(""));
	}
	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - msg.cursor < sizeof(char))
	{
		CreateError("Could not read ASCII byte from received binary at cursor position %u, amount of message remaining is smaller "
			"than variable to be read.", msg.cursor);
		return Runtime.CopyString(_T(""));
	}

	const std::string_view partial(msg.content.data() + msg.cursor, 1);
	const std::uint8_t charUnsigned = *(std::uint8_t *)(&partial[0]);
	++msg.cursor;

	// ASCII goes up to 127, ANSI continues to 255. ANSI is locale-dependent, and if we only
	// interpret it as a fixed one like "en-us", that may not be the sender's locale, muddying
	// the water further for a Fusion developer.
	if (charUnsigned > 127)
	{
		CreateError("ANSI char %hhu read from cursor position %u, which is not an ASCII character. Use CursorStringWithSize() with size 1 to ensure consistency.",
			charUnsigned, msg.cursor - 1);
		return Runtime.CopyString(_T(""));
	}
	// Also check character is displayable (aka "printable").
	if (!std::isprint(charUnsigned))
	{
		CreateError("ASCII char %hhu read, which cannot be displayed. Check cursor index %u is correct.",
			charUnsigned, msg.cursor - 1);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(DarkEdif::ANSIToTString(partial).c_str());
}
unsigned int Extension::RecvMsg_Cursor_UnsignedByte()
{
	if (!threadData->IsRecvMsg())
		return CreateError("Could not read cursor unsigned byte from received binary, not a received message event."), 0U;
	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - msg.cursor < sizeof(unsigned char))
		return CreateError("Could not read unsigned byte from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", msg.cursor), 0U;

	++msg.cursor;
	return (unsigned int)(*(unsigned char *)(msg.content.data() + (msg.cursor - sizeof(unsigned char))));
}
int Extension::RecvMsg_Cursor_SignedByte()
{
	if (!threadData->IsRecvMsg())
		return CreateError("Could not read cursor ASCII byte from received binary, not a received message event."), 0;

	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - msg.cursor < sizeof(char))
		return CreateError("Could not read signed byte from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", msg.cursor), 0;

	++msg.cursor;
	return (int)(*(msg.content.data() + msg.cursor - sizeof(char)));
}
unsigned int Extension::RecvMsg_Cursor_UnsignedShort()
{
	if (!threadData->IsRecvMsg())
		return CreateError("Could not read cursor unsigned short from received binary, not a received message event."), 0U;
	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - msg.cursor < sizeof(unsigned short))
		return CreateError("Could not read unsigned short from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", msg.cursor), 0U;

	msg.cursor += sizeof(unsigned short);
	return (unsigned int)(*(unsigned short *)(msg.content.data() + msg.cursor - sizeof(unsigned short)));
}
int Extension::RecvMsg_Cursor_SignedShort()
{
	if (!threadData->IsRecvMsg())
		return CreateError("Could not read cursor signed short from received binary, not a received message event."), 0;
	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - msg.cursor < sizeof(short))
		return CreateError("Could not read signed short from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", msg.cursor), 0;

	msg.cursor += sizeof(short);
	return (int)(*(short *)(msg.content.data() + msg.cursor - sizeof(short)));
}
unsigned int Extension::RecvMsg_Cursor_UnsignedInteger()
{
	if (!threadData->IsRecvMsg())
		return CreateError("Could not read cursor unsigned integer from received binary, not a received message event."), 0U;
	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - msg.cursor < sizeof(unsigned int))
		return CreateError("Could not read unsigned integer from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", msg.cursor), 0U;

	msg.cursor += sizeof(unsigned int);
	return (*(unsigned int *)(msg.content.data() + msg.cursor - sizeof(unsigned int)));
}
int Extension::RecvMsg_Cursor_SignedInteger()
{
	if (!threadData->IsRecvMsg())
		return CreateError("Could not read cursor signed integer from received binary, not a received message event."), 0;
	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - msg.cursor < sizeof(int))
		return CreateError("Could not read signed integer from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", msg.cursor), 0;

	msg.cursor += sizeof(int);
	return (*(int *)(msg.content.data() + msg.cursor - sizeof(int)));
}
float Extension::RecvMsg_Cursor_Float()
{
	if (!threadData->IsRecvMsg())
		return CreateError("Could not read cursor float from received binary, not a received message event."), 0.f;
	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - msg.cursor < sizeof(float))
		return CreateError("Could not read unsigned byte from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", msg.cursor), 0.f;

	msg.cursor += sizeof(float);
	return (*(float *)(msg.content.data() + msg.cursor - sizeof(float)));
}
const TCHAR * Extension::RecvMsg_Cursor_StringWithSize(int size)
{
	if (!threadData->IsRecvMsg())
	{
		CreateError("Could not read cursor string with size from received binary, not a received message event.");
		return Runtime.CopyString(_T(""));
	}
	const auto& msg = threadData->GetRecvMsg();
	if (size < 0)
	{
		CreateError("Could not read a string with size at cursor position %u, supplied size %i is less than 0.", msg.cursor, size);
		return Runtime.CopyString(_T(""));
	}
	if (msg.content.size() - msg.cursor < (size_t)size)
	{
		CreateError("Could not read a string with size from received binary at cursor position %u, amount of message remaining %u is less than supplied size %i.",
			msg.cursor, (std::uint32_t)(msg.content.size() - msg.cursor), size);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(RecvMsg_Sub_ReadString(msg, msg.cursor, size, true).c_str());
}
const TCHAR * Extension::RecvMsg_Cursor_String()
{
	if (!threadData->IsRecvMsg())
	{
		CreateError("Could not read cursor null-terminated string from received binary, not a received message event.");
		return Runtime.CopyString(_T(""));
	}
	const auto& msg = threadData->GetRecvMsg();
	if (msg.content.size() - msg.cursor < 1)
	{
		CreateError("Could not read null-terminated string from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", msg.cursor);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(RecvMsg_Sub_ReadString(msg, msg.cursor, -1, true).c_str());
}
unsigned int Extension::SendBinaryMsg_MemoryAddress()
{
#if __aarch64__ || _M_AMD64 || __amd64__
	CreateError("Warning: can't properly return message-to-send memory address; address is 64-bit and Fusion only deals in 32-bit integers.");
#endif
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
	if (!threadData->IsRecvMsg())
	{
		CreateError("Could not dump received message, not a received message event.");
		return Runtime.CopyString(_T(""));
	}
	const auto& recvMsg = threadData->GetRecvMsg();
	if (recvMsg.content.size() - index <= 0)
	{
		CreateError("Dumping message failed; index %i is beyond message end index %zu.", index, recvMsg.content.size() - 1);
		return Runtime.CopyString(_T(""));
	}

	bool allValid;
	const std::string formatANSI = DarkEdif::TStringToANSI(formatTStr, &allValid);
	if (!allValid)
	{
		CreateError("Dumping message failed; format supplied \"%s\" had unrecognised characters. See help file.", DarkEdif::TStringToUTF8(formatTStr).c_str());
		return Runtime.CopyString(_T(""));
	}
	const char * format = formatANSI.c_str();

	std::stringstream output;
	output << std::setfill('0') << std::uppercase;
	size_t sizeOfFormat = strlen(format);
	bool varSigned;
	size_t varCount = 0;
	const char * msg = &recvMsg.content[index];
	// Example, signed char x10: "+c10"
	for (const char * i = format; i < format + sizeOfFormat;)
	{
		// Skip last iteration's count to get to next variable type letter
		while (isdigit(i[0]))
			++i;

		// We skipped past digits into the end of string
		if (i == format + sizeOfFormat)
			break;

		// Determine if variable should be signed or unsigned
		if (i[0] != '+')
			varSigned = true;
		else
		{
			varSigned = false;
			++i;
		}

		// varCount number of expected variables
		varCount = atoi(i + 1);
		if (varCount < 1)
			varCount = 1;

		// Char
		if (i[0] == 'c')
		{
			++i;
			if (recvMsg.content.size() - index < varCount)
			{
				CreateError("Could not dump; message was not large enough to contain all variables.");
				return Runtime.CopyString(_T(""));
			}

			std::uint32_t curChar;
#ifdef __APPLE__
			int width = (int)output.width();
#else
			std::streamsize width = output.width();
#endif
			if (varSigned)
			{
				for (unsigned int j = 0; j < varCount; ++j)
				{
					curChar = *(std::uint8_t *)&msg[j];
					output << "Signed char: "sv;
					if (std::isprint(curChar))
						output << '\'' << msg[j] << '\'';
					else
						output << "(?)"sv;
					output << " ("sv << (int)msg[j] << ", 0x"sv << std::hex << std::setw(2) << curChar << std::dec << std::setw(width) << ")\r\n"sv;
				}
			}
			else
			{
				for (unsigned int j = 0; j < varCount; ++j)
				{
					curChar = *(std::uint8_t *)&msg[j];
					output << "Unsigned char: "sv << curChar << " (0x" << std::hex << std::setw(2) << curChar << std::dec << std::setw(width) << ")\r\n"sv;
				}
			}
			output.width(width);
			msg += varCount;

			continue;
		}

		// Short
		if (i[0] == 'h')
		{
			++i;
			if (recvMsg.content.size() - index < varCount * sizeof(short))
			{
				CreateError("Could not dump; message was not large enough to contain variables.");
				return Runtime.CopyString(_T(""));
			}
			if (varSigned)
			{
				for (unsigned int j = 0; j < varCount; ++j)
					output << "Signed short: "sv << (int)((short *)msg)[j] << "\r\n"sv;
			}
			else
			{
				for (unsigned int j = 0; j < varCount; ++j)
					output << "Unsigned short: "sv << (int)((unsigned short *)msg)[j] << "\r\n"sv;
			}
			msg += varCount * sizeof(short);
			continue;
		}

		// String (null-terminated)
		if (i[0] == 's')
		{
			++i;
			if (varSigned == false)
				CreateError("'+' flag not expected next to 's'; strings cannot be unsigned.");
			for (unsigned int j = 0; j < varCount; ++j)
			{
				size_t u8StrSize = strnlen(msg, recvMsg.content.size() - index + 1);
				if (u8StrSize == recvMsg.content.size() - index + 1)
				{
					CreateError("Could not dump; message was not large enough to contain variables.");
					return Runtime.CopyString(_T(""));
				}

				if (!lw_u8str_validate(std::string_view(msg, u8StrSize)))
				{
					CreateError("Could not dump; the null-terminated string starting at message index %i, read as %zu chars long, was not valid UTF-8 text.", index, u8StrSize);
					return Runtime.CopyString(_T(""));
				}

				output << "String: "sv << msg << "\r\n"sv;
				msg += u8StrSize + 1;
			}
			continue;
		}

		// Integer
		if (i[0] == 'i')
		{
			++i;
			if (recvMsg.content.size() - index < varCount * sizeof(int))
			{
				CreateError("Could not dump; message was not large enough to contain variables.");
				return Runtime.CopyString(_T(""));
			}
			if (varSigned)
			{
				for (unsigned int j = 0; j < varCount; ++j)
					output << "Signed integer: "sv << ((int *)msg)[j] << "\r\n"sv;
			}
			else
			{
				for (unsigned int j = 0; j < varCount; ++j)
					output << "Unsigned integer: "sv << ((unsigned int *)msg)[j] << "\r\n"sv;
			}
			msg += varCount * sizeof(int);
			continue;
		}

		// Floating-point
		if (i[0] == 'f')
		{
			++i;
			if (recvMsg.content.size() - index < varCount * sizeof(float))
			{
				CreateError("Could not dump; message was not large enough to contain variables.");
				return Runtime.CopyString(_T(""));
			}
			if (!varSigned)
				CreateError("'+' flag not expected next to 'f'; floats cannot be unsigned.");
			else
			{
				for (unsigned int j = 0; j < varCount; ++j)
					output << "Float: "sv << ((float *)msg)[j] << "\r\n"sv;
			}
			msg += varCount * sizeof(float);
			continue;
		}

		// Did not find identifier; error out
		CreateError("Unrecognised character in dump format: '%c'. Valid : c, h, s, i, f; operator +.", i[0]);
		return Runtime.CopyString(_T(""));
	}

	std::string outputTrim = output.str();
	outputTrim.resize(outputTrim.size() - 2);
	return Runtime.CopyString(DarkEdif::UTF8ToTString(outputTrim).c_str());
}
unsigned int Extension::ChannelListing_ChannelCount()
{
	return (std::uint32_t)Cli.channellistingcount();
}

// Number of UTF-8 code points (including things like combining accents)
int Extension::ConvToUTF8_GetCompleteCodePointCount(const TCHAR * tStr)
{
	// Empty string
	if (tStr[0] == _T('\0'))
		return 0;

	const std::string u8str = DarkEdif::TStringToUTF8(tStr);
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

	return (int)numCodePoints;
}
// Get number of UTF-8 graphemes (distinct graphical characters)
int Extension::ConvToUTF8_GetVisibleCharCount(const TCHAR * tStr)
{
	// Empty string
	if (tStr[0] == _T('\0'))
		return 0;

	const std::string u8str = DarkEdif::TStringToUTF8(tStr);
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
	return (int)++numChars;
}
// Get number of bytes in a UTF-8 string
int Extension::ConvToUTF8_GetByteCount(const TCHAR * tStr)
{
	// Empty string
	if (tStr[0] == _T('\0'))
		return 0;

	// TODO: can make this a little faster by only measuring length of output in WideToMultiByte(),
	// instead of doing that + the conversion itself as DarkEdif::TStringToUTF8() does internally.
	const size_t u8size = DarkEdif::TStringToUTF8(tStr).size();
	return u8size <= 0 ? -1 : (int)u8size;
}
// Tests if the UTF - 8 equivalent matches the passed allow list, and if allow list is valid.
// If so, blank is returned, otherwise the error or faulty character.
const TCHAR * Extension::ConvToUTF8_TestAllowList(const TCHAR * toTest, const TCHAR * allowList)
{
	lacewing::codepointsallowlist list;
	const std::string err = list.setcodepointsallowedlist(DarkEdif::TStringToUTF8(allowList));
	if (!err.empty())
		return Runtime.CopyString(DarkEdif::UTF8ToTString(err).c_str());

	utf8proc_int32_t rejectedChar = -1;
	const int idx = list.checkcodepointsallowed(DarkEdif::TStringToUTF8(toTest), &rejectedChar);
	if (idx == -1)
		return Runtime.CopyString(_T(""));

	TCHAR output[256];
	_stprintf_s(output, std::size(output), _T("Code point at index %d does not match allowed list. Code point U+%0.4X, decimal %u; valid = %s, Unicode category = %s."),
		idx, rejectedChar, rejectedChar, utf8proc_codepoint_valid(rejectedChar) ? _T("yes") : _T("no"), DarkEdif::UTF8ToTString(utf8proc_category_string(rejectedChar)).c_str());
	return Runtime.CopyString(output);
}

const TCHAR * Extension::NetScan_ServerIP()
{
	if (!threadData->Is<NetScanReplyEvent>())
	{
		CreateError("Reading network scan IP when not in an On Network Scan Reply event.");
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(DarkEdif::UTF8ToTString(threadData->AsC<NetScanReplyEvent>().remoteIP).c_str());
}

const TCHAR* Extension::NetScan_ServerVersion()
{
	if (!threadData->Is<NetScanReplyEvent>())
	{
		CreateError("Reading network scan server version when not in an On Network Scan Reply event.");
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(DarkEdif::UTF8ToTString(threadData->AsC<NetScanReplyEvent>().serverVersion).c_str());
}

const TCHAR* Extension::NetScan_ServerWelcomeMessage()
{
	// Condition 76: On network scan
	if (!threadData->Is<NetScanReplyEvent>())
	{
		CreateError("Reading network scan welcome message when not in an On Network Scan Reply event.");
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(DarkEdif::UTF8ToTString(threadData->AsC<NetScanReplyEvent>().welcomeMessage).c_str());
}
