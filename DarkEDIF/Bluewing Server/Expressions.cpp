
#include "Common.h"

const char * Extension::error()
{
	return Runtime.CopyString(threadData.error.text ? threadData.error.text : "");
}
unsigned int Extension::Channel_Count()
{
	return Srv.channelcount();
}
const char * Extension::Client_Name()
{
	return Runtime.CopyString((!threadData.client || !threadData.client->name()) ? "" : threadData.client->name());
}
const char * Extension::ReceivedStr()
{
	if (threadData.receivedMsg.content[threadData.receivedMsg.size - 1] != '\0') 
		return Runtime.CopyString(std::string(threadData.receivedMsg.content, threadData.receivedMsg.size).c_str());
	else
		return Runtime.CopyString(threadData.receivedMsg.content);
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
unsigned int Extension::Subchannel()
{
	return (int)threadData.receivedMsg.subchannel;
}
unsigned int Extension::Client_ID()
{
	return threadData.client ? threadData.client->id() : -1;
}
const char * Extension::RequestedClientName()
{
	return Runtime.CopyString(threadData.Requested.name ? threadData.Requested.name : "");
}
const char * Extension::RequestedChannelName()
{
	return Runtime.CopyString(threadData.Requested.name ? threadData.Requested.name : "");
}
unsigned int Extension::Client_ConnectionTime()
{
	return threadData.client ? (int)threadData.client->connecttime() : -1;
}
unsigned int Extension::Client_ChannelCount()
{
	return threadData.client ? threadData.client->channelcount() : -1;
}
const char * Extension::Client_GetLocalData(char * key)
{
	if (!key || !threadData.client)
		return Runtime.CopyString("");
	return Runtime.CopyString(threadData.client->GetLocalData(key).c_str());
}
const char * Extension::Channel_GetLocalData(char * key)
{
	if (!key || !threadData.channel)
		return Runtime.CopyString("");
	return Runtime.CopyString(threadData.channel->GetLocalData(key).c_str());
}
const char * Extension::Channel_Name()
{
	return Runtime.CopyString(threadData.channel ? threadData.channel->name() : "");
}
unsigned int Extension::Channel_ClientCount()
{
	return threadData.channel ? threadData.channel->clientcount() : -1;
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
		return Runtime.CopyString(std::string(threadData.receivedMsg.content + index, 1).c_str());
}
unsigned int Extension::UnsignedByte(int index)
{
	if (index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0;
	}
	else if (threadData.receivedMsg.size - index < sizeof(unsigned char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
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
		return 0;
	}
	else if (threadData.receivedMsg.size - index < sizeof(unsigned short))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
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
		return 0;
	}
	else if (threadData.receivedMsg.size - index < sizeof(unsigned int))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
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
		return Runtime.CopyString(std::string(threadData.receivedMsg.content + index, size).c_str());
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
unsigned int Extension::BinaryToSend_Size()
{
	return SendMsgSize;
}
const char * Extension::Client_IP()
{
	if (threadData.client == nullptr)
	{
		CreateError("Could not read client IP, no client selected.");
		return Runtime.CopyString("");
	}

	char * addr = (char *)Runtime.Allocate(64U);
	lw_addr_prettystring(threadData.client->getaddress(), addr, 64);
	return addr;
}
unsigned int Extension::Port()
{
	return Srv.port();
}
const char * Extension::Welcome_Message()
{
	return Runtime.CopyString(Srv.getwelcomemessage()); 
}
unsigned int Extension::ReceivedBinaryAddress()
{
	return (unsigned int)(long)threadData.receivedMsg.content;
}
const char * Extension::CursorStrByte()
{
	if (threadData.receivedMsg.size - threadData.receivedMsg.cursor < sizeof(char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return Runtime.CopyString("");
	}
	else
		return Runtime.CopyString(std::string(threadData.receivedMsg.content + threadData.receivedMsg.cursor, 1).c_str());
}
unsigned int Extension::CursorUnsignedByte()
{
	if (threadData.receivedMsg.size - threadData.receivedMsg.cursor < sizeof(unsigned char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read.");
		return 0U;
	}
	else
	{
		++threadData.receivedMsg.cursor;
		return (unsigned int)(*(unsigned char *)(threadData.receivedMsg.content + (threadData.receivedMsg.cursor - 1)));
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
		return (int)(threadData.receivedMsg.content + threadData.receivedMsg.cursor - 1);
	}
}
unsigned int Extension::CursorUnsignedShort()
{
	if (threadData.receivedMsg.size - threadData.receivedMsg.cursor < sizeof(unsigned short))
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0U;
	}
	else
	{
		threadData.receivedMsg.cursor += 2U;
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
		threadData.receivedMsg.cursor += strlen(threadData.receivedMsg.content + threadData.receivedMsg.cursor) + 1U;
		return Runtime.CopyString(threadData.receivedMsg.content + s);
	}
}
const char * Extension::Client_ProtocolImplementation()
{
	if (threadData.client == nullptr)
	{
		CreateError("Could not read client protocol implementation, no client selected.");
		return Runtime.CopyString("");
	}
	return Runtime.CopyString(threadData.client->getimplementation());
}
unsigned int Extension::BinaryToSend_Address()
{
	return (unsigned int)(long)SendMsg;
}
const char * Extension::DumpMessage(int index, const char * format)
{
	//
	if (!format || format[0] == '\0')
		CreateError("Dumping message failed; format supplied was null or \"\".");
	else if (threadData.receivedMsg.size - index <= 0)
		CreateError("Dumping message failed; index exceeds size of message.");
	else
	{
		std::stringstream Output;
		size_t SizeOfFormat = strlen(format);
		bool Signed;
		size_t Count = 0;
		const char * Msg = &threadData.receivedMsg.content[index];
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
				if (threadData.receivedMsg.size-index < Count)
				{
					CreateError("Could not dump; message was not large enough to contain variables.");
					return Runtime.CopyString("");
				}
				if (Signed)
				{
					for (unsigned int j = 0; j < Count; ++j)
						Output << "Signed char: " << (int)Msg[j] << "\r\n";
				}
				else
				{
					for (unsigned int j = 0; j < Count; ++j)
						Output << "Unsigned char: " << (int)((unsigned char *)Msg)[j] << "\r\n";
				}
				Msg += Count;
					
				continue;
			}
				
			// Short
			if (i[0] == 'h')
			{
				++i;
				if (threadData.receivedMsg.size-index < Count*sizeof(short))
				{
					CreateError("Could not dump; message was not large enough to contain variables.");
					return Runtime.CopyString("");
				}
				if (Signed)
				{
					for (unsigned int j = 0; j < Count; ++j)
						Output << "Signed short: " << (int)((short *)Msg)[j] << "\r\n";
				}
				else
				{
					for (unsigned int j = 0; j < Count; ++j)
						Output << "Unsigned short: " << (int)((unsigned short *)Msg)[j] << "\r\n";
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
					if (strnlen(Msg, threadData.receivedMsg.size-index+1) == threadData.receivedMsg.size-index+1)
					{
						CreateError("Could not dump; message was not large enough to contain variables.");
						return Runtime.CopyString("");
					}

					Output << "String: " << Msg << "\r\n";
					Msg += strlen(Msg)+1;
				}
				continue;
			}

			// Integer
			if (i[0] == 'i')
			{
				++i;
				if (threadData.receivedMsg.size-index < Count*sizeof(int))
				{
					CreateError("Could not dump; message was not large enough to contain variables.");
					return Runtime.CopyString("");
				}
				if (Signed)
				{
					for (unsigned int j = 0; j < Count; ++j)
						Output << "Signed integer: " << ((int *)Msg)[j] << "\r\n";
				}
				else
				{
					for (unsigned int j = 0; j < Count; ++j)
						Output << "Unsigned integer: " << ((unsigned int *)Msg)[j] << "\r\n";
				}
				Msg += Count*sizeof(int);
				continue;
			}

			// Floating-point
			if (i[0] == 'f')
			{
				++i;
				if (threadData.receivedMsg.size-index < Count*sizeof(float))
				{
					CreateError("Could not dump; message was not large enough to contain variables.");
					return Runtime.CopyString("");
				}
				if (!Signed)
					CreateError("'+' flag not expected next to 'f'; floats cannot be unsigned.");
				else
				{
					for (unsigned int j = 0; j < Count; ++j)
						Output << "Float: " << ((float *)Msg)[j] << "\r\n";
				}
				Msg += Count*sizeof(float);
				continue;
			}

			// Did not find identifier; error out
			std::stringstream error;
			error << "Unrecognised variable in format: '" << i[0] << "'. Valid: c, h, s, i, f; operator +.";
			CreateError(error.str().c_str());
			return Runtime.CopyString("");
		}

		return Runtime.CopyString(Output.str().c_str());
	}

	return Runtime.CopyString("");
}
unsigned int Extension::AllClientCount()
{
	return Srv.clientcount();
}
const char * Extension::GetDenyReason()
{
	return Runtime.CopyString(DenyReason ? DenyReason : "");
}