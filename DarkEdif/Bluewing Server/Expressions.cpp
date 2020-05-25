
#include "Common.h"

const char * Extension::Error()
{
	return Runtime.CopyString(!threadData->error.text.empty() ? threadData->error.text.c_str() : "");
}
unsigned int Extension::Channel_Count()
{
	return Srv.channelcount();
}
const char * Extension::Client_Name()
{
	return Runtime.CopyString(!selClient ? "" : selClient->name().c_str());
}
const char * Extension::ReceivedStr()
{
	return Runtime.CopyString(threadData->receivedMsg.content.c_str());
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
const char * Extension::RequestedClientName()
{
	return Runtime.CopyString(threadData->requested.name.c_str());
}
const char * Extension::RequestedChannelName()
{
	// 4 = Leave Channel condition ID. There's no point rewriting the name in a Leave Channel request.
	if (threadData->CondTrig[0] == 4)
		CreateError("Requested channel name is not available in a Leave Channel Request. Use Channel_Name$() instead.");

	return Runtime.CopyString(threadData->requested.name.c_str());
}
unsigned int Extension::Client_ConnectionTime()
{
	return selClient ? (int)selClient->getconnecttime() : -1;
}
unsigned int Extension::Client_ChannelCount()
{
	return selClient ? selClient->channelcount() : -1;
}
const char * Extension::Client_GetLocalData(char * key)
{
	if (!key || !selClient)
		return Runtime.CopyString("");
	return Runtime.CopyString(globals->GetLocalData(selClient, key).c_str());
}
const char * Extension::Channel_GetLocalData(char * key)
{
	if (!key || !selChannel)
		return Runtime.CopyString("");
	return Runtime.CopyString(globals->GetLocalData(selChannel, key).c_str());
}
const char * Extension::Channel_Name()
{
	return Runtime.CopyString(selChannel ? selChannel->name().c_str() : "");
}
unsigned int Extension::Channel_ClientCount()
{
	return selChannel ? selChannel->clientcount() : -1;
}
const char * Extension::StrByte(int index)
{
	if (index < 0)
	{
		CreateError("Could not read text byte from received binary, index %i is less than 0.", index), 0;
		return Runtime.CopyString("");
	}
	else if (threadData->receivedMsg.content.size() - index < sizeof(char))
	{
		CreateError("Could not read text byte from received binary at position %i, amount of message remaining is smaller than variable to be read.", index);
		return Runtime.CopyString("");
	}
	else
		return Runtime.CopyString(threadData->receivedMsg.content.substr(index, 1).c_str());
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
	if (threadData->receivedMsg.content.size() - index < sizeof(unsigned char))
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
	if (threadData->receivedMsg.content.size() - index < sizeof(signed short))
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
const char * Extension::StringWithSize(int index, int size)
{
	if (index < 0)
	{
		CreateError("Could not read string with size from received binary, index %i is less than 0.", index);
		return Runtime.CopyString("");
	}
	if (size < 0)
	{
		CreateError("Could not read string with size from received binary, supplied size %i is less than 0.", size);
		return Runtime.CopyString("");
	}
	if (threadData->receivedMsg.content.size() - index < (size_t)size)
	{
		CreateError("Could not read float from received binary at position %i, amount of message remaining is smaller than supplied string size %i.", index, size);
		return Runtime.CopyString("");
	}

	return Runtime.CopyString(threadData->receivedMsg.content.substr(index, size).c_str());
}
const char * Extension::String(int index)
{
	if (index < 0)
	{
		CreateError("Could not read null-terminated string from received binary, index %i is less than 0.", index);
		return Runtime.CopyString("");
	}
	if (threadData->receivedMsg.content.size() - index < 1)
	{
		CreateError("Could not read null-terminated string from received binary at position %i, amount of message remaining is smaller than variable to be read.", index);
		return Runtime.CopyString("");
	}
	if (strnlen(threadData->receivedMsg.content.data() + index, threadData->receivedMsg.content.size() - index + 1) == threadData->receivedMsg.content.size() - index + 1)
	{
		CreateError("Could not read null-terminated string from received binary at position %i, null terminator not found.", index);
		return Runtime.CopyString("");
	}

	return Runtime.CopyString(&threadData->receivedMsg.content[index]);
}
unsigned int Extension::ReceivedBinarySize()
{
	return threadData->receivedMsg.content.size();
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
	if (selClient == nullptr)
	{
		CreateError("Could not read client IP, no client selected.");
		return Runtime.CopyString("");
	}

	char * addr = (char *)Runtime.Allocate(64U);
	lw_addr_prettystring(selClient->getaddress().data(), addr, 64);
	return addr;
}
unsigned int Extension::Port()
{
	return Srv.port();
}
const char * Extension::Welcome_Message()
{
	return Runtime.CopyString(Srv.getwelcomemessage().c_str()); 
}
unsigned int Extension::ReceivedBinaryAddress()
{
	return (unsigned int)(long)threadData->receivedMsg.content.data();
}
const char * Extension::CursorStrByte()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(char))
	{
		CreateError("Could not read text byte from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor);
		return Runtime.CopyString("");
	}

	return Runtime.CopyString(threadData->receivedMsg.content.substr(threadData->receivedMsg.cursor, 1).c_str());
}
unsigned int Extension::CursorUnsignedByte()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(unsigned char))
	{
		return CreateError("Could not read unsigned byte from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0U;
		return 0U;
	}

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
		return CreateError("Could not read signed short from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0;

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
const char * Extension::CursorStringWithSize(int size)
{
	if (size < 0)
	{
		CreateError("Could not read string with size from received binary at cursor position %u, supplied size %i is less than 0.", threadData->receivedMsg.cursor, size);
		return Runtime.CopyString("");
	}
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < (size_t)size)
	{
		CreateError("Could not read string with size from received binary at cursor position %u, amount of message remaining is less than supplied size %i.", threadData->receivedMsg.cursor, size);
		return Runtime.CopyString("");
	}

	threadData->receivedMsg.cursor += size;
	return Runtime.CopyString(threadData->receivedMsg.content.substr(threadData->receivedMsg.cursor - size, size).c_str());
}
const char * Extension::CursorString()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < 1)
	{
		CreateError("Could not read null-terminated string from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor);
		return Runtime.CopyString("");
	}

	size_t maxSizePlusOne = threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor + 1;
	if (strnlen(threadData->receivedMsg.content.c_str() + threadData->receivedMsg.cursor, maxSizePlusOne) == maxSizePlusOne)
	{
		CreateError("Could not read null-terminated string from received binary at cursor position %u, null terminator not found.", threadData->receivedMsg.cursor);
		return Runtime.CopyString("");
	}

	size_t s = threadData->receivedMsg.cursor;
	threadData->receivedMsg.cursor += strlen(threadData->receivedMsg.content.c_str() + threadData->receivedMsg.cursor) + 1U;
	return Runtime.CopyString(threadData->receivedMsg.content.c_str() + s);
}
const char * Extension::Client_ProtocolImplementation()
{
	if (selClient == nullptr)
	{
		CreateError("Could not read client protocol implementation, no client selected.");
		return Runtime.CopyString("");
	}
	return Runtime.CopyString(selClient->getimplementation());
}
unsigned int Extension::BinaryToSend_Address()
{
	return (unsigned int)(long)SendMsg;
}
const char * Extension::DumpMessage(int index, const char * format)
{
	if (!format || format[0] == '\0')
	{
		CreateError("Dumping message failed; format supplied was null or \"\".");
		return Runtime.CopyString("");
	}
	if (threadData->receivedMsg.content.size() - index <= 0)
	{
		CreateError("Dumping message failed; index %i exceeds size of message %i.", index, threadData->receivedMsg.content.size());
		return Runtime.CopyString("");
	}
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
			if (threadData->receivedMsg.content.size()-index < Count*sizeof(short))
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
				if (strnlen(Msg, threadData->receivedMsg.content.size()-index+1) == threadData->receivedMsg.content.size()-index+1)
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
			if (threadData->receivedMsg.content.size()-index < Count*sizeof(int))
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
			if (threadData->receivedMsg.content.size()-index < Count*sizeof(float))
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
unsigned int Extension::AllClientCount()
{
	return Srv.clientcount();
}
const char * Extension::GetDenyReason()
{
	return Runtime.CopyString(DenyReason.c_str());
}
