
#include "Common.h"

const char * Extension::error()
{
	return Runtime.CopyString(threadData.error.text ? threadData.error.text : "");
}
const char * Extension::ReplacedExprNoParams()
{
	return Runtime.CopyString("");
}
const char * Extension::Self_Name()
{
	return Runtime.CopyString(Cli.name() ? Cli.name() : "");
}
unsigned int Extension::Self_ChannelCount()
{
	return Channels.size();
}
const char * Extension::Peer_Name()
{
	return Runtime.CopyString(threadData.peer ? threadData.peer->name() : "");
}
const char * Extension::ReceivedStr()
{
	if (threadData.receivedMsg.content == nullptr || threadData.receivedMsg.size == 0) 
		return Runtime.CopyString("");
	else
		return Runtime.CopyString(std::string(threadData.receivedMsg.content, threadData.receivedMsg.size).c_str());
}
int Extension::ReceivedInt()
{
	if (threadData.receivedMsg.size != 4) 
	{
		CreateError("Received() was used on a message that is not a number message.");
		return 0;
	}
	else
		return *(int *)threadData.receivedMsg.content;
}
unsigned int Extension::subchannel()
{
	return (unsigned int)threadData.receivedMsg.subchannel;
}
int Extension::Peer_ID()
{
	return threadData.peer ? threadData.peer->id() : -1;
}
const char * Extension::Channel_Name()
{
	return Runtime.CopyString(threadData.channel ? threadData.channel->name() : "");
}
int Extension::Channel_PeerCount()
{
	return threadData.channel ? threadData.channel->peercount() : -1;
}
const char * Extension::ChannelListing_Name()
{
	return Runtime.CopyString(threadData.channelListing ? threadData.channelListing->name : "");
}
int Extension::ChannelListing_PeerCount()
{
	return threadData.channelListing ? threadData.channelListing->peercount : -1;
}
int Extension::Self_ID()
{
	return Cli.id();
}
const char * Extension::StrByte(int index)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return Runtime.CopyString("");
	}
	else if (threadData.receivedMsg.size - index < sizeof(char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return Runtime.CopyString("");
	}
	else
		return Runtime.CopyString(std::string(threadData.receivedMsg.content + index, 1U).c_str());
}
unsigned int Extension::UnsignedByte(int index)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0U;
	}
	else if (threadData.receivedMsg.size - index < sizeof(unsigned char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0U;
	}
	else
		return (unsigned int)(*(unsigned char *)(threadData.receivedMsg.content + index));
}
int Extension::SignedByte(int index)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0;
	}
	else if (threadData.receivedMsg.size - index < sizeof(char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
		return (int)(threadData.receivedMsg.content + index);
}
unsigned int Extension::UnsignedShort(int index)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0U;
	}
	else if (threadData.receivedMsg.size - index < sizeof(unsigned short))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0U;
	}
	else
		return (unsigned int)(*(unsigned short *)(threadData.receivedMsg.content + index));
}
int Extension::SignedShort(int index)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0;
	}
	else if (threadData.receivedMsg.size - index < sizeof(short))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
		return (int)(*(short *)(threadData.receivedMsg.content + index));
}
unsigned int Extension::UnsignedInteger(int index)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0U;
	}
	else if (threadData.receivedMsg.size - index < sizeof(unsigned int))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0U;
	}
	else
		return (*(unsigned int *)(threadData.receivedMsg.content + index));
}
int Extension::SignedInteger(int index)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0;
	}
	else if (threadData.receivedMsg.size - index < sizeof(int))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
		return (*(int *)(threadData.receivedMsg.content + index));
}
float Extension::Float(int index)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0.0f;
	}
	else if (threadData.receivedMsg.size - index < sizeof(float))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0.0f;
	}
	else
		return (*(float *)(threadData.receivedMsg.content + index));
}
const char * Extension::StringWithSize(int index, int size)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return Runtime.CopyString("");
	}
	else if (threadData.receivedMsg.size - index < size * sizeof(TCHAR))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return Runtime.CopyString("");
	}
	else
		return Runtime.CopyString(std::string(threadData.receivedMsg.content+index, size).c_str());
}
const char * Extension::String(int index)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return Runtime.CopyString("");
	}
	else if (threadData.receivedMsg.size - index < 1)
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return Runtime.CopyString("");
	}
	else if (strnlen(threadData.receivedMsg.content + index, threadData.receivedMsg.size - index + 1) == threadData.receivedMsg.size - index + 1)
	{
		CreateError("Could not read null-terminated string; null terminator not found.");
		return Runtime.CopyString("");
	}
	else
		return Runtime.CopyString(&threadData.receivedMsg.content[index]);
}
unsigned int Extension::ReceivedBinarySize()
{
	return threadData.receivedMsg.size;
}
const char * Extension::Lacewing_Version()
{
	static const char * version = nullptr;
	if (version == nullptr)
	{
		std::stringstream str;
		str << lw_version() << " / Bluewing reimpl b" << lacewing::relayserver::buildnum;
		version = _strdup(str.str().c_str());
	}
	return Runtime.CopyString(version);
}
unsigned int Extension::SendBinarySize()
{
	return SendMsgSize;
}
const char * Extension::Self_PreviousName()
{
	return Runtime.CopyString(PreviousName ? PreviousName : "");
}
const char * Extension::Peer_PreviousName()
{
	return Runtime.CopyString((!threadData.peer || !threadData.peer->prevname()) ? "" : threadData.peer->prevname());
}
const char * Extension::DenyReason()
{
	return Runtime.CopyString(DenyReasonBuffer ? DenyReasonBuffer : "No reason specified.");
}
const char * Extension::HostIP()
{
	char * fusionAddr = (char *)Runtime.Allocate(64);
	lacewing::lw_addr_prettystring(Cli.serveraddress()->tostring(), fusionAddr, 64U);
	return fusionAddr; // Allocate is equivalent to CopyString, but doesn't copy initially.
}
unsigned int Extension::HostPort()
{
	return Cli.serveraddress()->port();
}
const char * Extension::WelcomeMessage()
{
	return Runtime.CopyString(Cli.welcomemessage()); 
}
unsigned int Extension::ReceivedBinaryAddress()
{
	return (unsigned int)threadData.receivedMsg.content;
}
const char * Extension::CursorStrByte()
{
	if (threadData.receivedMsg.size - threadData.receivedMsg.cursor < sizeof(char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return Runtime.CopyString("");
	}
	else
	{
		++threadData.receivedMsg.cursor;
		return Runtime.CopyString(std::string(threadData.receivedMsg.content+threadData.receivedMsg.cursor - 1, 1).c_str());
	}
}
unsigned int Extension::CursorUnsignedByte()
{
	if (threadData.receivedMsg.size - threadData.receivedMsg.cursor < sizeof(unsigned char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read.");
		return 0;
	}
	else
	{
		++threadData.receivedMsg.cursor;
		return (unsigned int)(*(unsigned char *)(threadData.receivedMsg.content + (threadData.receivedMsg.cursor-1)));
	}
}
int Extension::CursorSignedByte()
{
	if (threadData.receivedMsg.size - threadData.receivedMsg.cursor < sizeof(char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read.");
		return 0;
	}
	else
	{
		++threadData.receivedMsg.cursor;
		return (int)(threadData.receivedMsg.content+threadData.receivedMsg.cursor-1);
	}
}
unsigned int Extension::CursorUnsignedShort()
{
	if (threadData.receivedMsg.size - threadData.receivedMsg.cursor < sizeof(unsigned short))
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0;
	}
	else
	{
		threadData.receivedMsg.cursor += 2;
		return (unsigned int)(*(unsigned short *)(threadData.receivedMsg.content + threadData.receivedMsg.size - 2));
	}
}
int Extension::CursorSignedShort()
{
	if (threadData.receivedMsg.size - threadData.receivedMsg.cursor < sizeof(short))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
	{
		threadData.receivedMsg.cursor += 2;
		return (int)(*(short *)(threadData.receivedMsg.content + threadData.receivedMsg.cursor - 2));
	}
}
unsigned int Extension::CursorUnsignedInteger()
{
	if (threadData.receivedMsg.size - threadData.receivedMsg.cursor < sizeof(unsigned int))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
	{
		threadData.receivedMsg.cursor += 4;
		return (*(unsigned int *)(threadData.receivedMsg.content + threadData.receivedMsg.cursor - 4));
	}
}
int Extension::CursorSignedInteger()
{
	if (threadData.receivedMsg.size - threadData.receivedMsg.cursor < sizeof(int))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
	{
		threadData.receivedMsg.cursor += 4;
		return (*(int *)(threadData.receivedMsg.content + threadData.receivedMsg.cursor - 4));
	}
}
float Extension::CursorFloat()
{
	if (threadData.receivedMsg.size - threadData.receivedMsg.cursor < sizeof(float))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0.0f;
	}
	else
	{
		threadData.receivedMsg.cursor += 4;
		return (*(float *)(threadData.receivedMsg.content + threadData.receivedMsg.cursor - 4));
	}
}
const char * Extension::CursorStringWithSize(int size)
{
	if (threadData.receivedMsg.size - threadData.receivedMsg.cursor < size * sizeof(TCHAR))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return Runtime.CopyString("");
	}
	else
	{
		threadData.receivedMsg.cursor += size;
		return Runtime.CopyString(std::string(threadData.receivedMsg.content + threadData.receivedMsg.cursor - size, size).c_str());
	}
}
const char * Extension::CursorString()
{
	if (threadData.receivedMsg.size - threadData.receivedMsg.cursor < 1)
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return Runtime.CopyString("");
	}
	else if (strnlen(threadData.receivedMsg.content + threadData.receivedMsg.cursor, threadData.receivedMsg.size - threadData.receivedMsg.cursor + 1) == threadData.receivedMsg.size - threadData.receivedMsg.cursor + 1)
	{
		CreateError("Could not read null-terminated string; null terminator not found.");
		return Runtime.CopyString("");
	}
	else
	{
		size_t s = threadData.receivedMsg.cursor;
		threadData.receivedMsg.cursor += strlen(threadData.receivedMsg.content + threadData.receivedMsg.cursor)+1;
		return Runtime.CopyString(threadData.receivedMsg.content+s);
	}
}
unsigned int Extension::SendBinaryAddress()
{
	return (unsigned int)(long)SendMsg;
}
const char * Extension::DumpMessage(int index, const char * format)
{
	//
	if (format[0] == '\0')
	{
		CreateError("Dumping message failed; format supplied was null or \"\".");
	}
	else if (threadData.receivedMsg.size - index <= 0)
	{
		CreateError("Dumping message failed; index exceeds size of message.");
	}
	else
	{
		std::stringstream output;
		size_t sizeOfFormat = strlen(format);
		bool _signed;
		size_t count = 0;
		const char * Msg = &threadData.receivedMsg.content[index];
		// +c10c20c
 		for (const char * i = format; i < format + sizeOfFormat;)
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
				if (threadData.receivedMsg.size-index < count)
				{
					CreateError("Could not dump; message was not large enough to contain variables.");
					return Runtime.CopyString("");
				}
				if (_signed)
				{
					for (unsigned int j = 0; j < count; ++j)
						output << "Signed char: " << (int)Msg[j] << "\r\n";
				}
				else
				{
					for (unsigned int j = 0; j < count; ++j)
						output << "Unsigned char: " << (int)((unsigned char *)Msg)[j] << "\r\n";
				}
				Msg += count;
					
				continue;
			}
				
			// Short
			if (i[0] == 'h')
			{
				++i;
				if (threadData.receivedMsg.size-index < count*sizeof(short))
				{
					CreateError("Could not dump; message was not large enough to contain variables.");
					return Runtime.CopyString("");
				}
				if (_signed)
				{
					for (unsigned int j = 0; j < count; ++j)
						output << "Signed short: " << (int)((short *)Msg)[j] << "\r\n";
				}
				else
				{
					for (unsigned int j = 0; j < count; ++j)
						output << "Unsigned short: " << (int)((unsigned short *)Msg)[j] << "\r\n";
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
				for (unsigned int j = 0; j < count; ++j)
				{
					if (strnlen(Msg, threadData.receivedMsg.size - index + 1) == threadData.receivedMsg.size - index + 1)
					{
						CreateError("Could not dump; message was not large enough to contain variables.");
						return Runtime.CopyString("");
					}

					output << "String: " << Msg << "\r\n";
					Msg += strlen(Msg)+1;
				}
				continue;
			}

			// Integer
			if (i[0] == 'i')
			{
				++i;
				if (threadData.receivedMsg.size-index < count*sizeof(int))
				{
					CreateError("Could not dump; message was not large enough to contain variables.");
					return Runtime.CopyString("");
				}
				if (_signed)
				{
					for (unsigned int j = 0; j < count; ++j)
						output << "Signed integer: " << ((int *)Msg)[j] << "\r\n";
				}
				else
				{
					for (unsigned int j = 0; j < count; ++j)
						output << "Unsigned integer: " << ((unsigned int *)Msg)[j] << "\r\n";
				}
				Msg += count*sizeof(int);
				continue;
			}

			// Floating-point
			if (i[0] == 'f')
			{
				++i;
				if (threadData.receivedMsg.size-index < count*sizeof(float))
				{
					CreateError("Could not dump; message was not large enough to contain variables.");
					return Runtime.CopyString("");
				}
				if (!_signed)
				{
					CreateError("'+' flag not expected next to 'f'; floats cannot be unsigned.");
				}
				else
				{
					for (unsigned int j = 0; j < count; ++j)
						output << "Float: " << ((float *)Msg)[j] << "\r\n";
				}
				Msg += count*sizeof(float);
				continue;
			}

			// Did not find identifier; error out
			std::stringstream error;
			error << "Unrecognised variable in format: '" << i[0] << "'. Valid: c, h, s, i, f; and operator +.";
			CreateError(error.str().c_str());
			return Runtime.CopyString("");
		}

		return Runtime.CopyString(output.str().c_str());
	}

	return Runtime.CopyString("");
}
unsigned int Extension::ChannelListing_ChannelCount()
{
	return Cli.channellistingcount();
}
