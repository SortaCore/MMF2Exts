
#include "Common.h"

const char * Extension::Error()
{
	return Runtime.CopyString(ThreadData.Error.Text);
}
const char * Extension::ReplacedExprNoParams()
{
	return Runtime.CopyString("");
}
const char * Extension::Self_Name()
{
	return Runtime.CopyString(Cli.Name() ? Cli.Name() : "");
}
int Extension::Self_ChannelCount()
{
	return Cli.ChannelCount();
}
const char * Extension::Peer_Name()
{
	return Runtime.CopyString((!ThreadData.Peer || !ThreadData.Peer->Name()) ? "" : ThreadData.Peer->Name());
}
const char * Extension::ReceivedStr()
{
	if (ThreadData.ReceivedMsg.Content[ThreadData.ReceivedMsg.Size-1] != '\0') 
	{
		CreateError("Received$() was used on a message that is not null-terminated.");
		return Runtime.CopyString("");
	}
	else
		return Runtime.CopyString(ThreadData.ReceivedMsg.Content);
}
int Extension::ReceivedInt()
{
	if (ThreadData.ReceivedMsg.Size != 4) 
	{
		CreateError("Received() was used on a message that is not a number message.");
		return 0;
	}
	else
		return *(int *)ThreadData.ReceivedMsg.Content;
}
int Extension::Subchannel()
{
	return (int)ThreadData.ReceivedMsg.Subchannel;
}
int Extension::Peer_ID()
{
	return ThreadData.Peer ? ThreadData.Peer->ID() : -1;
}
const char * Extension::Channel_Name()
{
	return Runtime.CopyString(ThreadData.Channel ? ThreadData.Channel->Name() : ThreadData.Loop.Name ? ThreadData.Loop.Name : "");
}
int Extension::Channel_PeerCount()
{
	return ThreadData.Channel ? ThreadData.Channel->PeerCount() : -1;
}
const char * Extension::ChannelListing_Name()
{
	return Runtime.CopyString(ThreadData.ChannelListing ? ThreadData.Channel->Name() : "");
}
int Extension::ChannelListing_PeerCount()
{
	return ThreadData.Channel ? ThreadData.Channel->PeerCount() : -1;
}
int Extension::Self_ID()
{
	return Cli.ID();
}
const char * Extension::StrByte(int Index)
{
	if (Index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return Runtime.CopyString("");
	}
	else if (ThreadData.ReceivedMsg.Size - Index < sizeof(char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return Runtime.CopyString("");
	}
	else
		return Runtime.CopyString(std::string(ThreadData.ReceivedMsg.Content+Index, 1).c_str());
}
unsigned int Extension::UnsignedByte(int Index)
{
	if (Index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0;
	}
	else if (ThreadData.ReceivedMsg.Size - Index < sizeof(unsigned char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
		return (unsigned int)(*(unsigned char *)(ThreadData.ReceivedMsg.Content+Index));
}
int Extension::SignedByte(int Index)
{
	if (Index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0;
	}
	else if (ThreadData.ReceivedMsg.Size - Index < sizeof(char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
		return (int)(ThreadData.ReceivedMsg.Content+Index);
}
unsigned int Extension::UnsignedShort(int Index)
{
	if (Index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0;
	}
	else if (ThreadData.ReceivedMsg.Size - Index < sizeof(unsigned short))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
		return (unsigned int)(*(unsigned short *)(ThreadData.ReceivedMsg.Content + Index));
}
int Extension::SignedShort(int Index)
{
	if (Index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0;
	}
	else if (ThreadData.ReceivedMsg.Size - Index < sizeof(short))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
		return (int)(*(short *)(ThreadData.ReceivedMsg.Content + Index));
}
unsigned int Extension::UnsignedInteger(int Index)
{
	if (Index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0;
	}
	else if (ThreadData.ReceivedMsg.Size - Index < sizeof(unsigned int))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
		return (*(unsigned int *)(ThreadData.ReceivedMsg.Content + Index));
}
int Extension::SignedInteger(int Index)
{
	if (Index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0;
	}
	else if (ThreadData.ReceivedMsg.Size - Index < sizeof(int))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
		return (*(int *)(ThreadData.ReceivedMsg.Content + Index));
}
float Extension::Float(int Index)
{
	if (Index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0.0f;
	}
	else if (ThreadData.ReceivedMsg.Size - Index < sizeof(float))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0.0f;
	}
	else
		return (*(float *)(ThreadData.ReceivedMsg.Content + Index));
}
const char * Extension::StringWithSize(int Index, int Size)
{
	if (Index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return Runtime.CopyString("");
	}
	else if (ThreadData.ReceivedMsg.Size - Index < Size * sizeof(TCHAR))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return Runtime.CopyString("");
	}
	else
		return Runtime.CopyString(std::string(ThreadData.ReceivedMsg.Content+Index, Size).c_str());
}
const char * Extension::String(int Index)
{
	if (Index < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return Runtime.CopyString("");
	}
	else if (ThreadData.ReceivedMsg.Size - Index < 1)
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return Runtime.CopyString("");
	}
	else if (strnlen(ThreadData.ReceivedMsg.Content + Index, ThreadData.ReceivedMsg.Size - Index + 1) == ThreadData.ReceivedMsg.Size - Index + 1)
	{
		CreateError("Could not read null-terminated string; null terminator not found.");
		return Runtime.CopyString("");
	}
	else
		return Runtime.CopyString(&ThreadData.ReceivedMsg.Content[Index]);
}
int Extension::ReceivedBinarySize()
{
	return ThreadData.ReceivedMsg.Size;
}
const char * Extension::Lacewing_Version()
{
	return Runtime.CopyString(Lacewing::Version());
}
int Extension::SendBinarySize()
{
	return SendMsgSize;
}
const char * Extension::Self_PreviousName()
{
	return Cli.Tag ? (char *)Cli.Tag : "";
}
const char * Extension::Peer_PreviousName()
{
	return ThreadData.Peer->Tag ? (char *)ThreadData.Peer->Tag : "";
}
const char * Extension::DenyReason()
{
	return Runtime.CopyString(DenyReasonBuffer ? DenyReasonBuffer : "No reason specified.");
}
const char * Extension::HostIP()
{
	return Cli.ServerAddress().ToString();
}
int Extension::HostPort()
{
	return Cli.ServerAddress().Port();
}
const char * Extension::WelcomeMessage()
{
	return Cli.WelcomeMessage(); 
}
long Extension::ReceivedBinaryAddress()
{
	return (long)ThreadData.ReceivedMsg.Content;
}
const char * Extension::CursorStrByte()
{
	if (ThreadData.ReceivedMsg.Size - ThreadData.ReceivedMsg.Cursor < sizeof(char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return Runtime.CopyString("");
	}
	else
		return Runtime.CopyString(std::string(ThreadData.ReceivedMsg.Content+ThreadData.ReceivedMsg.Cursor, 1).c_str());
}
unsigned int Extension::CursorUnsignedByte()
{
	if (ThreadData.ReceivedMsg.Size - ThreadData.ReceivedMsg.Cursor < sizeof(unsigned char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read.");
		return 0;
	}
	else
	{
		++ThreadData.ReceivedMsg.Cursor;
		return (unsigned int)(*(unsigned char *)(ThreadData.ReceivedMsg.Content + (ThreadData.ReceivedMsg.Cursor-1)));
	}
}
int Extension::CursorSignedByte()
{
	if (ThreadData.ReceivedMsg.Size - ThreadData.ReceivedMsg.Cursor < sizeof(char))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read.");
		return 0;
	}
	else
	{
		++ThreadData.ReceivedMsg.Cursor;
		return (int)(ThreadData.ReceivedMsg.Content+ThreadData.ReceivedMsg.Cursor-1);
	}
}
unsigned int Extension::CursorUnsignedShort()
{
	if (ThreadData.ReceivedMsg.Size - ThreadData.ReceivedMsg.Cursor < sizeof(unsigned short))
	{
		CreateError("Could not read from received binary, index less than 0.");
		return 0;
	}
	else
	{
		ThreadData.ReceivedMsg.Cursor += 2;
		return (unsigned int)(*(unsigned short *)(ThreadData.ReceivedMsg.Content + ThreadData.ReceivedMsg.Size - 2));
	}
}
int Extension::CursorSignedShort()
{
	if (ThreadData.ReceivedMsg.Size - ThreadData.ReceivedMsg.Cursor < sizeof(short))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
	{
		ThreadData.ReceivedMsg.Cursor += 2;
		return (int)(*(short *)(ThreadData.ReceivedMsg.Content + ThreadData.ReceivedMsg.Cursor - 2));
	}
}
unsigned int Extension::CursorUnsignedInteger()
{
	if (ThreadData.ReceivedMsg.Size - ThreadData.ReceivedMsg.Cursor < sizeof(unsigned int))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
	{
		ThreadData.ReceivedMsg.Cursor += 4;
		return (*(unsigned int *)(ThreadData.ReceivedMsg.Content + ThreadData.ReceivedMsg.Cursor - 4));
	}
}
int Extension::CursorSignedInteger()
{
	if (ThreadData.ReceivedMsg.Size - ThreadData.ReceivedMsg.Cursor < sizeof(int))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0;
	}
	else
	{
		ThreadData.ReceivedMsg.Cursor += 4;
		return (*(int *)(ThreadData.ReceivedMsg.Content + ThreadData.ReceivedMsg.Cursor - 4));
	}
}
float Extension::CursorFloat()
{
	if (ThreadData.ReceivedMsg.Size - ThreadData.ReceivedMsg.Cursor < sizeof(float))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return 0.0f;
	}
	else
	{
		ThreadData.ReceivedMsg.Cursor += 4;
		return (*(float *)(ThreadData.ReceivedMsg.Content + ThreadData.ReceivedMsg.Cursor - 4));
	}
}
const char * Extension::CursorStringWithSize(int Size)
{
	if (ThreadData.ReceivedMsg.Size - ThreadData.ReceivedMsg.Cursor < Size * sizeof(TCHAR))
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return Runtime.CopyString("");
	}
	else
	{
		ThreadData.ReceivedMsg.Cursor += Size;
		return Runtime.CopyString(std::string(ThreadData.ReceivedMsg.Content + ThreadData.ReceivedMsg.Cursor - Size, Size).c_str());
	}
}
const char * Extension::CursorString()
{
	if (ThreadData.ReceivedMsg.Size - ThreadData.ReceivedMsg.Cursor < 1)
	{
		CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index.");
		return Runtime.CopyString("");
	}
	else if (strnlen(ThreadData.ReceivedMsg.Content + ThreadData.ReceivedMsg.Cursor, ThreadData.ReceivedMsg.Size - ThreadData.ReceivedMsg.Cursor + 1) == ThreadData.ReceivedMsg.Size - ThreadData.ReceivedMsg.Cursor + 1)
	{
		CreateError("Could not read null-terminated string; null terminator not found.");
		return Runtime.CopyString("");
	}
	else
	{
		size_t s = ThreadData.ReceivedMsg.Cursor;
		ThreadData.ReceivedMsg.Cursor += strlen(ThreadData.ReceivedMsg.Content + ThreadData.ReceivedMsg.Cursor)+1;
		return Runtime.CopyString(ThreadData.ReceivedMsg.Content+s);
	}
}
long Extension::SendBinaryAddress()
{
	return (long)SendMsg;
}
const char * Extension::DumpMessage(int Index, const char * Format)
{
	//
	if (!Format || Format[0] == '\0')
	{
		CreateError("Dumping message failed; format supplied was null or \"\".");
	}
	else
	{
		if (ThreadData.ReceivedMsg.Size - Index <= 0)
		{
			CreateError("Dumping message failed; index exceeds size of message.");
		}
		else
		{
			std::stringstream Output;
			size_t SizeOfFormat = strlen(Format);
			bool Signed;
			size_t Count = 0;
			const char * Msg = &ThreadData.ReceivedMsg.Content[Index];
			// +c10c20c
 			for (const char * i = Format; i < Format+SizeOfFormat;)
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
					if (ThreadData.ReceivedMsg.Size-Index < Count)
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
					if (ThreadData.ReceivedMsg.Size-Index < Count*sizeof(short))
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
						if (strnlen(Msg, ThreadData.ReceivedMsg.Size-Index+1) == ThreadData.ReceivedMsg.Size-Index+1)
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
					if (ThreadData.ReceivedMsg.Size-Index < Count*sizeof(int))
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
					if (ThreadData.ReceivedMsg.Size-Index < Count*sizeof(float))
					{
						CreateError("Could not dump; message was not large enough to contain variables.");
						return Runtime.CopyString("");
					}
					if (!Signed)
					{
						CreateError("'+' flag not expected next to 'f'; floats cannot be unsigned.");
					}
					else
					{
						for (unsigned int j = 0; j < Count; ++j)
							Output << "Float: " << ((float *)Msg)[j] << "\r\n";
					}
					Msg += Count*sizeof(float);
					continue;
				}

				// Did not find identifier; error out
				std::stringstream Error;
				Error << "Unrecognised variable in format: '" << i[0] << "'. Valid: c, h, s, i, f; operator +.";
				CreateError(Error.str().c_str());
				return Runtime.CopyString("");
			}

			return Runtime.CopyString(Output.str().c_str());
		}
	}

	return Runtime.CopyString("");
}
