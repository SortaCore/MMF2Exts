
#include "Common.h"
char Buffer [200];
std::string CriticalSection;
const static int X = -200; // Mystical placeholder
#define Remake(name) MessageBoxA(NULL, "Your "#name" actions need to be recreated.\r\n" \
									   "This is probably due to parameter changes.", "Lacewing Relay Client - DarkEDIF", MB_OK)
void Extension::RelayServer_Host(int port)
{
	if (Srv.hosting())
		CreateError("Cannot start hosting: already hosting a server.");
	else
		Srv.host(port);
}
void Extension::RelayServer_StopHosting()
{
	Srv.unhost();
}
void Extension::FlashServer_Host(char * path)
{
	if (FlashSrv->hosting())
		CreateError("Cannot start hosting flash policy: already hosting a flash policy.");
	else
		FlashSrv->host(path);
}
void Extension::FlashServer_StopHosting()
{
	FlashSrv->unhost();
}
void Extension::HTML5Server_EnableHosting()
{
	
}
void Extension::HTML5Server_DisableHosting()
{
	
}
void Extension::ChannelListing_Enable()
{
	Srv.setchannellisting(true);
}
void Extension::ChannelListing_Disable()
{
	Srv.setchannellisting(false);
}
void Extension::SetWelcomeMessage(char * Message)
{
	if (!Message)
		CreateError("SetWelcomeMessage() was called with a null message.");
	else
		Srv.setwelcomemessage(Message);
}
void Extension::EnableCondition_OnMessageToChannel()
{
	Srv.onmessage_channel(::OnChannelMessage);
}
void Extension::EnableCondition_OnMessageToPeer()
{
	Srv.onmessage_peer(::OnPeerMessage);
}
void Extension::OnInteractive_Deny(char * Reason)
{
	if (!Reason)
		CreateError("Can't deny action: Deny was called with a null reason.");
	else if (InteractivePending == InteractiveType::None)
		CreateError("Cannot change new peer name: No interactive action is pending.");
	else if ((InteractivePending & InteractiveType::DenyPermitted) != InteractiveType::DenyPermitted)
		CreateError("Cannot change new peer name: Interactive event is not compatible with this action.");
	else if (DenyReason)
		CreateError("Can't deny action: Deny was called more than once. Ignoring additional denies.");
	else
	{
		if (NewChannelName)
			CreateError("Warning: Already edited the name of the join channel request, deny request action has just overridden it.");
		if (NewPeerName)
			CreateError("Warning: Already edited the name of the client set name change, deny request action has just overridden it.");

		DenyReason = _strdup(Reason);
	}
}
void Extension::OnInteractive_ChangePeerName(char * NewName)
{
	if (!NewName || NewName[0] == '\0')
		CreateError("Cannot change new peer name: Cannot use a null or blank name.");
	else if (strlen(NewName) > 255)
		CreateError("Cannot change new peer name: Cannot use a name longer than 255 characters.");
	else if (InteractivePending == InteractiveType::None)
		CreateError("Cannot change new peer name: No interactive action is pending.");
	else if ((InteractivePending & InteractiveType::PeerName) != InteractiveType::PeerName)
		CreateError("Cannot change new peer name: Interactive event is not compatible with this action.");
	else if (DenyReason)
		CreateError("Cannot change new peer name: Name change has already been denied by the Deny Request action.");
	else
		NewPeerName = _strdup(NewName);
}
void Extension::OnInteractive_ChangeChannelName(char * NewName)
{
	if (!NewName || NewName[0] == '\0')
		CreateError("Cannot change joining channel name: Cannot use a null or blank name.");
	else if (strlen(NewName) > 255)
		CreateError("Cannot change joining channel name: Cannot use a name exceeding the max length of 255 characters.");
	else if (InteractivePending == InteractiveType::None)
		CreateError("Cannot change joining channel name: No interactive action is pending.");
	else if ((InteractivePending & InteractiveType::ChannelName) != InteractiveType::ChannelName)
		CreateError("Cannot change joining channel name: Interactive event is not compatible with this action.");
	else if (DenyReason)
		CreateError("Cannot change joining channel name: Channel name join has already been denied by the Deny Request action.");
	else
		NewChannelName = _strdup(NewName);
}
void Extension::OnInteractive_DropMessage()
{
	if (InteractivePending == InteractiveType::None)
		CreateError("Cannot deny the action: No interactive action is pending.");
	else if ((InteractivePending & InteractiveType::ChannelName) != InteractiveType::ChannelName)
		CreateError("Cannot change joining channel name: Interactive event is not compatible with this action.");
	else if (DropMessage)
		CreateError("Cannot drop message: Message already being dropped. Ignoring additional actions.");
	else
		DropMessage = true;
}
void Extension::Channel_SelectByName(char * Name)
{
	if (!Name || Name[0] == '\0')
		CreateError("Channel_SelectByName() was called with a null or blank name.");
	else if (strlen(Name) > 255)
		CreateError("Channel_SelectByName() was called with a name exceeding the max length of 255 characters.");
	else
	{
		ThreadData.Channel = nullptr;
		lacewing::relayserver::channel * i = Srv.firstchannel();
		while (i)
		{
			if (_stricmp(i->name(), Name))
			{
				ThreadData.Channel = i;
				return;
			}
			i = i->next();
		}
		
		CreateError("Selecting channel by name failed: A channel with that name doesn't exist.");
	}
}
void Extension::Channel_Close()
{
	if (!ThreadData.Channel)
		CreateError("Could not close channel: No channel selected.");
	else if (ThreadData.Channel->isclosed)
		CreateError("Could not close channel: Already closing.");
	else
		ThreadData.Channel->close();
}
void Extension::Channel_SelectMaster()
{
	if (!ThreadData.Channel)
		CreateError("Could not select channel master: No channel selected.");
	else if (ThreadData.Channel->isclosed)
		CreateError("Could not select channel master: Channel is closed.");
	else
		ThreadData.Client = ThreadData.Channel->channelmaster();
}
void Extension::Channel_LoopClients()
{
	if (!ThreadData.Channel)
		CreateError("Error: Loop Clients On Channel was called without a channel being selected.");
	else // You can loop a closed channel's clients, but it's read-only.
	{
		lacewing::relayserver::client * Stored = ThreadData.Client,
			*Selected = ThreadData.Channel->firstclient();
		while (Selected)
		{
			if (!Selected->isclosed)
			{
				SaveExtInfo &S = AddEvent(8);
				S.Client = Selected;
			}
			Selected = Selected->next();
		}
		SaveExtInfo &S = AddEvent(42);
		S.Client = Stored;
	}
}
void Extension::Channel_SetLocalData(char * Key, char * Value)
{
	if (!ThreadData.Channel)
		CreateError("Could not set channel local data: No channel selected.");
	else if (ThreadData.Channel->isclosed)
		CreateError("Could not set channel local data: Channel is closed.");
	else if (!Key || !Value)
		CreateError("Could not set channel local data: null parameter supplied.");
	else
	{
		auto i = std::find_if(Globals->channelsLocalData.begin(), Globals->channelsLocalData.end(),
			[&](const LocalData<lacewing::relayserver::channel> & s){ 
			return s.KeyAddr == ThreadData.Channel && !_stricmp(s.Key, Key); });

		// Blank value: Delete
		if (Value[0] == '\0')
		{
			if (i != Globals->channelsLocalData.end())
				Globals->channelsLocalData.erase(i);
			return;
		}
		free(i->Value);
		i->Value = _strdup(Value);
	}
}
void Extension::LoopAllChannels()
{
	lacewing::relayserver::channel * Stored = ThreadData.Channel;
	for (lacewing::relayserver::channel * i = Srv.firstchannel(); i; i = i->next())
	{
		ThreadData.Channel = i;
		Runtime.GenerateEvent(5);
	}

	ThreadData.Channel = Stored;
	Runtime.GenerateEvent(43);
}
void Extension::LoopAllChannelsWithName(char * LoopName)
{
	lacewing::relayserver::channel * Stored = ThreadData.Channel;
	for (lacewing::relayserver::channel * i = Srv.firstchannel(); i; i = i->next())
	{
		ClearThreadData();
		ThreadData.Channel = i;
		ThreadData.Loop.Name = LoopName;
		Runtime.GenerateEvent(35);
	}

	ClearThreadData();
	ThreadData.Channel = Stored;
	ThreadData.Loop.Name = LoopName;
	Runtime.GenerateEvent(40);
}
void Extension::Client_Disconnect()
{
	if (!ThreadData.Client)
		CreateError("Could not disconnect client: No client selected.");
	else if (!ThreadData.Client->isclosed)
		ThreadData.Client->disconnect();
}
void Extension::Client_SetLocalData(char * Key, char * Value)
{
	if (!ThreadData.Client)
		CreateError("Could not set client local data: No client selected.");
	else if (ThreadData.Client->isclosed)
		CreateError("Could not set client local data: Client is closed.");
	else if (!Key || !Value)
		CreateError("Could not set client local data: null parameter supplied.");
	else
	{
		auto i = std::find_if(Globals->clientsLocalData.begin(), Globals->clientsLocalData.end(),
			[&](const LocalData<lacewing::relayserver::client> & s){
			return s.KeyAddr == ThreadData.Client && !_stricmp(s.Key, Key); });

		// Blank value: Delete the key's data
		if (Value[0] == '\0')
		{
			if (i != Globals->clientsLocalData.end())
				Globals->clientsLocalData.erase(i);
			return;
		}
		free(i->Value);
		i->Value = Value;
	}
}
void Extension::Client_LoopJoinedChannels()
{
	if (!ThreadData.Client)
		CreateError("Cannot loop client's joined channels: No client selected.");
	else
	{
		// Store selected channel and revert after loop
		lacewing::relayserver::channel * Stored = ThreadData.Channel,
			*Selected = Srv.firstchannel();
		lacewing::relayserver::client * StoredCli = ThreadData.Client;
		while (Selected)
		{
			if (!Selected->isclosed)
			{
				ThreadData.Channel = Selected;
				ThreadData.Client = StoredCli;
				Runtime.GenerateEvent(6);
			}
			Selected = Selected->next();
		}
		ThreadData.Channel = Stored;
		ThreadData.Client = StoredCli;
		Runtime.GenerateEvent(45);
	}
}
void Extension::Client_LoopJoinedChannelsWithName(char * LoopName)
{
	if (!ThreadData.Client)
		CreateError("Cannot loop client's joined channels: No client selected.");
	else
	{
		// Store selected channel and revert after loop
		lacewing::relayserver::channel * Stored = ThreadData.Channel,
			*Selected = ThreadData.Client->firstchannel();
		lacewing::relayserver::client * StoredCli = ThreadData.Client;
		while (Selected)
		{
			if (!Selected->isclosed)
			{
				ThreadData.Channel = Selected;
				ThreadData.Client = StoredCli;
				ThreadData.Loop.Name = LoopName;
				Runtime.GenerateEvent(36);
			}
			Selected = Selected->next();
		}
		ThreadData.Channel = Stored;
		ThreadData.Client = StoredCli;
		ThreadData.Loop.Name = LoopName;
		Runtime.GenerateEvent(41);
		ThreadData.Loop.Name = nullptr;
	}
}
void Extension::Client_SelectByName(char * ClientName)
{
	if (!ClientName || ClientName[0] == '\0')
		CreateError("Error: Select Client On Channel By Name was called with a null parameter/blank name.");
	else if (!ThreadData.Channel)
		CreateError("Error: Select Client On Channel By Name was called without a channel being selected.");
	else
	{
		lacewing::relayserver::client * Selected = ThreadData.Channel->firstclient();
		while (Selected)
		{
			if (!_stricmp(Selected->name(), ClientName))
				break;
			Selected = Selected->next();
		}
		// Only modify ThreadData.Client if we found it
		if (Selected && !Selected->isclosed)
			ThreadData.Client = Selected;
		else
		{
			std::string Error = "Client not found:\r\n";
			Error += ClientName;
			Error += "\r\nOn channel:\r\n";
			Error += ThreadData.Channel->name();
			CreateError(Error.c_str());
		}
	}
}
void Extension::Client_SelectByID(int ClientID)
{
	if (ClientID < 0 || ClientID > 0xFFFF)
		CreateError("Could not select Client on channel, ID is below 0 or greater than 65535.");
	else
	{
		lacewing::relayserver::client * Selected = ThreadData.Channel->firstclient();
		while (Selected)
		{
			if (Selected->id() == ClientID)
				break;
			Selected = Selected->next();
		}
		
		if (Selected)
			ThreadData.Client = Selected;
		else if (Selected->isclosed)
		{
			char num[20];
			std::string Error = "Client ID ";
			if (_itoa_s(ClientID, num, 20, 10))
				Error += "> could not be copied <";
			else
				Error += &num[0];
			Error += " was not found on channel:";
			Error += ThreadData.Channel->name();
			CreateError(Error.c_str());
		}
	}
}
void Extension::Client_SelectSender()
{
	if (!ThreadData.SenderClient)
		CreateError("Cannot select sending client: No sending client variable available.");
	else
		ThreadData.Client = ThreadData.SenderClient;
}
void Extension::Client_SelectReceiver()
{
	if (!ThreadData.ReceivingClient)
		CreateError("Cannot select receiving client: No receiving client variable available.");
	else
		ThreadData.Client = ThreadData.ReceivingClient;
}
void Extension::LoopAllClients()
{
	SaveExtInfo * Stored = new SaveExtInfo(ThreadData);
	for (auto i = Srv.firstclient(); i; i = i->next())
	{
		ClearThreadData();
		ThreadData.Client = i;
		Runtime.GenerateEvent(7);
	}
	ClearThreadData();
	Runtime.GenerateEvent(44);

	ThreadData = *Stored;
	delete Stored;
}
void Extension::LoopAllClientsWithName(char * LoopName)
{
	SaveExtInfo * Stored = new SaveExtInfo(ThreadData);
	for (auto i = Srv.firstclient(); i; i = i->next())
	{
		ClearThreadData();
		ThreadData.Client = i;
		ThreadData.Loop.Name = LoopName;
		Runtime.GenerateEvent(37);
	}
	ClearThreadData();
	ThreadData.Loop.Name = LoopName;
	Runtime.GenerateEvent(50);

	ThreadData = *Stored;
	delete Stored;
}
void Extension::SendTextToChannel(int Subchannel, char * TextToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Send Text to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!TextToSend)
		CreateError("Error: Send Text to Channel was called with a null parameter.");
	else if (!ThreadData.Channel)
		CreateError("Error: Send Text to Channel was called without a channel being selected.");
	else if (ThreadData.Channel->isclosed)
		CreateError("Error: Send Text to Channel was called with a closed channel.");
	else
		ThreadData.Channel->send(Subchannel, TextToSend, strlen(TextToSend) + 1, 0);
}
void Extension::SendTextToClient(int Subchannel, char * TextToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Send Text to Client was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!TextToSend)
		CreateError("Error: Send Text to Client was called with a null parameter.");
	else if (!ThreadData.Client)
		CreateError("Error: Send Text to Client was called without a Client being selected.");
	else if (ThreadData.Client->isclosed)
		CreateError("Error: Send Text to Client was called with a closed Client.");
	else
		ThreadData.Client->send(Subchannel, TextToSend, strlen(TextToSend) + 1, 0);
}
void Extension::SendNumberToChannel(int Subchannel, int NumToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Send Number to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!ThreadData.Channel)
		CreateError("Error: Send Number to Channel was called without a channel being selected.");
	else if (ThreadData.Channel->isclosed)
		CreateError("Error: Send Number to Channel was called with a closed channel.");
	else
		ThreadData.Channel->send(Subchannel, (char *)&NumToSend, sizeof(int), 1);
}
void Extension::SendNumberToClient(int Subchannel, int NumToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Send Number to Client was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!ThreadData.Client)
		CreateError("Error: Send Number to Client was called without a Client being selected.");
	else if (ThreadData.Client->isclosed)
		CreateError("Error: Send Number to Client was called with a closed Client.");
	else
		ThreadData.Client->send(Subchannel, (char *)&NumToSend, sizeof(int), 1);
}
void Extension::SendBinaryToChannel(int Subchannel)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!ThreadData.Channel)
		CreateError("Error: Send Binary to Channel was called without a channel being selected.");
	else if (ThreadData.Channel->isclosed)
		CreateError("Error: Send Binary to Channel was called with a closed channel.");
	else
		ThreadData.Channel->send(Subchannel, SendMsg, SendMsgSize, 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::SendBinaryToClient(int Subchannel)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!ThreadData.Client)
		CreateError("Error: Send Binary to Client was called without a Client being selected.");
	else if (ThreadData.Client->isclosed)
		CreateError("Error: Send Binary to Client was called with a closed Client.");
	else
		ThreadData.Client->send(Subchannel, SendMsg, SendMsgSize, 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastTextToChannel(int Subchannel, char * TextToBlast)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Blast Text to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!TextToBlast)
		CreateError("Error: Blast Text to Channel was called with a null parameter.");
	else if (!ThreadData.Channel)
		CreateError("Error: Blast Text to Channel was called without a channel being selected.");
	else if (ThreadData.Channel->isclosed)
		CreateError("Error: Blast Text to Channel was called with a closed channel.");
	else
		ThreadData.Channel->blast(Subchannel, TextToBlast, strlen(TextToBlast) + 1, 0);
}
void Extension::BlastTextToClient(int Subchannel, char * TextToBlast)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Blast Text to Client was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!TextToBlast)
		CreateError("Error: Blast Text to Client was called with a null parameter.");
	else if (!ThreadData.Client)
		CreateError("Error: Blast Text to Client was called without a Client being selected.");
	else if (ThreadData.Client->isclosed)
		CreateError("Error: Blast Text to Client was called with a closed Client.");
	else
		ThreadData.Client->blast(Subchannel, TextToBlast, strlen(TextToBlast) + 1, 0);
}
void Extension::BlastNumberToChannel(int Subchannel, int NumToBlast)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Blast Number to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!ThreadData.Channel)
		CreateError("Error: Blast Number to Channel was called without a channel being selected.");
	else
		ThreadData.Channel->blast(Subchannel, (char *)&NumToBlast, 4, 1);
}
void Extension::BlastNumberToClient(int Subchannel, int NumToBlast)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Blast Number to Client was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!ThreadData.Client)
		CreateError("Error: Blast Number to Client was called without a Client being selected.");
	else if (ThreadData.Client->isclosed)
		CreateError("Error: Blast Number to Client was called with a closed Client.");
	else
		ThreadData.Client->blast(Subchannel, (char *)&NumToBlast, 4, 1);
}
void Extension::BlastBinaryToChannel(int Subchannel)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!ThreadData.Channel)
		CreateError("Error: Blast Binary to Channel was called without a channel being selected.");
	else if (ThreadData.Channel->isclosed)
		CreateError("Error: Blast Binary to Channel was called with a closed channel.");
	else
		ThreadData.Channel->blast(Subchannel, SendMsg, SendMsgSize, 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastBinaryToClient(int Subchannel)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!ThreadData.Client)
		CreateError("Error: Blast Binary to Client was called without a Client being selected.");
	else if (ThreadData.Client->isclosed)
		CreateError("Error: Blast Binary to Client was called with a closed Client.");
	else
		ThreadData.Client->blast(Subchannel, SendMsg, SendMsgSize, 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::AddByteText(char * Byte)
{
	if (!Byte || strnlen(Byte, 2) != 1)
		CreateError("Adding byte to stack failed: byte supplied was part of a string, not a single byte.");
	else
		AddToSend(Byte, 1);
}
void Extension::AddByteInt(int Byte)
{
	if (Byte > 255 || Byte < -128)
		CreateError("Error: Byte out of bounds.");
	else
	{
		if (Byte < 0)
		{
			char RealByte = (char)Byte;
			AddToSend(&RealByte, 1);
		}
		else
		{
			unsigned char RealByte = (unsigned char)Byte;
			AddToSend(&RealByte, 1);
		}
	}
}
void Extension::AddShort(int Short)
{
	if (Short > 65535 || Short < -32768)
		CreateError("Error: Short out of bounds.");
	else
	{
		if (Short < 0)
		{
			short RealShort = (short)Short;
			AddToSend(&RealShort, 2);
		}
		else
		{
			unsigned short RealShort = (unsigned short)Short;
			AddToSend(&RealShort, 2);
		}
	}
}
void Extension::AddInt(int Int)
{
	AddToSend(&Int, 4);
}
void Extension::AddFloat(float Float)
{
	AddToSend(&Float, 4);
}
void Extension::AddStringWithoutNull(char * String)
{
	if (String)
		AddToSend(String, strlen(String));
	else
		CreateError("Adding string without null failed: pointer was null.");
}
void Extension::AddString(char * String)
{
	if (String)
		AddToSend(String, strlen(String) + 1);
	else
		CreateError("Adding string failed: pointer was null.");
}
void Extension::AddBinary(void * Address, int Size)
{
	if (Size < 0)
		CreateError("Add binary failed: Size < 0.");
	else
	{
		if (Size != 0)
			AddToSend(Address, Size);
		// else do nothing
	}
}
void Extension::AddFileToBinary(char * Filename)
{
	if (!Filename || Filename[0] == '\0')
		CreateError("Cannot add file to send binary; filename is invalid.");
	else
	{
		FILE * File = NULL;

		// Open and deny other programs write priviledges
		if (!(File = _fsopen(Filename, "wb", _SH_DENYWR)))
		{
			char errorval[20];
			SaveExtInfo &S = AddEvent(0);
			std::string Error = "Cannot save binary to file, error ";
			if (_itoa_s(*_errno(), &errorval[0], 20, 10))
			{
				Error += " with opening the file, and with converting error number.";
			}
			else
			{
				Error += "number [";
				Error += &errorval[0];
				Error += "] occured with opening the file.";
			}
			Error += "\r\nThe message has not been modified.";
			S.Error.Text = _strdup(Error.c_str());
			return;
		}
		// Jump to end
		fseek(File, 0, SEEK_END);
		// Read current position as file size
		long filesize = ftell(File);
		// Go back to start
		fseek(File, 0, SEEK_SET);
		char * buffer = (char *)malloc(filesize);
		if (!buffer)
			CreateError("Couldn't reserve enough memory to add file into message.");
		else
		{
			size_t s;
			if ((s = fread_s(buffer, filesize, 1, filesize, File)) != filesize)
			{
				char sizeastext[20];
				SaveExtInfo &S = AddEvent(0);
				std::string Error = "Couldn't write full buffer to file, ";
				if (_itoa_s(s, &sizeastext[0], 20, 10))
				{
					Error += " and error copying size.";
				}
				else
				{
					Error += &sizeastext[0];
					Error += " bytes managed to be written.";
				}
				S.Error.Text = _strdup(Error.c_str());
			}
			AddToSend(buffer, s);
			free(buffer);
		}
		fclose(File);
	}
}
void Extension::ResizeBinaryToSend(int NewSize)
{
	if (NewSize < 0)
		CreateError("Cannot change size of binary to send: new size is under 0 bytes.");
	else
	{
		char * NewMsg = (char *)realloc(SendMsg, NewSize);
		if (!NewMsg)
		{
			return CreateError("Cannot change size of binary to send: reallocation of memory failed.\r\n"
				"Size has not been modified.");
		}
		// Clear new bytes to 0
		memset(NewMsg + SendMsgSize, 0, NewSize - SendMsgSize);

		SendMsg = NewMsg;
		SendMsgSize = NewSize;
	}
}
void Extension::CompressSendBinary()
{
	if (SendMsgSize <= 0)
		CreateError("Cannot compress; Message is too small.");
	else
	{
		int ret;
		z_stream strm = { 0 };
		ret = deflateInit_(&strm, 9, ZLIB_VERSION, sizeof(z_stream));
		if (ret)
		{
			std::stringstream error;
			error << "Error " << ret << "occured with initiating compression.";
			CreateError(error.str().c_str());
			return;
		}

		unsigned char * output_buffer = (unsigned char *)malloc(SendMsgSize + 256);
		if (!output_buffer)
		{
			std::stringstream error;
			error << "Error, could not allocate enough memory. Desired " << SendMsgSize + 256 << "bytes.";
			CreateError(error.str().c_str());
			deflateEnd(&strm);
			return;
		}
		strm.next_in = (unsigned char *)SendMsg;
		strm.avail_in = SendMsgSize;
		// Allocate memory for compression
		strm.avail_out = _msize(output_buffer);
		strm.next_out = output_buffer;
		ret = deflate(&strm, Z_FINISH);
		if (ret != Z_STREAM_END)
		{
			std::stringstream error;
			error << "Error with compression, deflate() returned " << ret << "! Text: " << strm.msg ? strm.msg : "";
			free(output_buffer);
			deflateEnd(&strm);
			CreateError(error.str().c_str());
			return;
		}
		deflateEnd(&strm);
		void * v = realloc(output_buffer, strm.total_out);
		if (!v)
		{
			free(output_buffer);
			CreateError("Error with compression, reallocating memory failed.");
			return;
		}
		free(SendMsg);

		SendMsg = (char *)v;
		SendMsgSize = strm.total_out;
	}
}
void Extension::ClearBinaryToSend()
{
	if (SendMsg)
	{
		free(SendMsg);
		SendMsg = NULL;
	}
	SendMsgSize = 0;
}
void Extension::DecompressReceivedBinary()
{
	if (ThreadData.ReceivedMsg.Size <= 0)
		CreateError("Cannot decompress; Message is too small.");
	else
	{
		int ret;
		z_stream strm = { 0 };
		ret = inflateInit_(&strm, ZLIB_VERSION, sizeof(z_stream));
		if (ret)
		{
			std::stringstream error;
			error << "Error " << ret << "occured with initiating decompression.";
			CreateError(error.str().c_str());
			return;
		}

		unsigned char * output_buffer = NULL, *output_buffer_pointer = NULL;
		strm.next_in = (unsigned char *)ThreadData.ReceivedMsg.Content;
		strm.avail_in = ThreadData.ReceivedMsg.Size;
		// run inflate() on input until output buffer not full, finish
		// compression if all of source has been read in
		do {
			// Allocate memory for compression
			output_buffer_pointer = (unsigned char *)realloc(output_buffer, (output_buffer ? _msize(output_buffer) : 0) + 1024);
			if (!output_buffer_pointer)
			{
				std::stringstream error;
				error << "Error, could not allocate enough memory. Desired " << (output_buffer ? _msize(output_buffer) : 0) + 1024 << "bytes.";
				if (output_buffer)
					free(output_buffer);

				CreateError(error.str().c_str());
				inflateEnd(&strm);
				return;
			}

			output_buffer = output_buffer_pointer;
			output_buffer_pointer += _msize(output_buffer) - 1024;
			strm.avail_out = 1024;
			strm.next_out = output_buffer_pointer;
			ret = inflate(&strm, Z_FINISH);
			if (ret < Z_OK)
			{
				std::stringstream error;
				error << "Error with decompression, inflate() returned " << ret << "! Text: " << strm.msg ? strm.msg : "";
				free(output_buffer);
				CreateError(error.str().c_str());
				inflateEnd(&strm);
				return;
			}

		} while (strm.avail_in != 0);

		if (ret < 0)
		{
			std::stringstream error;
			error << "Error with decompression: " << ret << "! Text: " << strm.msg ? strm.msg : "";
			CreateError(error.str().c_str());
			inflateEnd(&strm);
			return;
		}
		inflateEnd(&strm);
		char * ThisMsg = ThreadData.ReceivedMsg.Content;
		free(ThreadData.ReceivedMsg.Content);
		ThreadData.ReceivedMsg.Content = (char *)output_buffer;
		ThreadData.ReceivedMsg.Cursor = 0;
		ThreadData.ReceivedMsg.Size = _msize(output_buffer);
		for (std::vector<SaveExtInfo *>::iterator i = Saved.begin(); i != Saved.end(); ++i)
		{
			if (ThisMsg != (*i)->ReceivedMsg.Content)
				break;
			(*i)->ReceivedMsg.Content = (char *)output_buffer;
			(*i)->ReceivedMsg.Cursor = 0;
			(*i)->ReceivedMsg.Size = _msize(output_buffer);
		}
	}
}
void Extension::MoveReceivedBinaryCursor(int Position)
{
	if (Position < 0)
		CreateError("Cannot move cursor; Position less than 0.");
	else if (ThreadData.ReceivedMsg.Size - Position <= 0)
		CreateError("Cannot move cursor; Message is too small.");
	else
		ThreadData.ReceivedMsg.Cursor = Position;
}
void Extension::SaveReceivedBinaryToFile(int Position, int Size, char * Filename)
{
	if (Position < 0)
		CreateError("Cannot save received binary; Position less than 0.");
	else if (Size <= 0)
		CreateError("Cannot save received binary; Size equal or less than 0.");
	else if (!Filename || Filename[0] == '\0')
		CreateError("Cannot save received binary; filename is invalid.");
	else if (ThreadData.ReceivedMsg.Size - Size <= 0)
		CreateError("Cannot save received binary; Message is too small.");
	else
	{
		FILE * File = NULL;
		if (fopen_s(&File, Filename, "wb") || !File)
		{
			char errorval[20];
			SaveExtInfo &S = AddEvent(0);
			std::string Error = "Cannot save received binary to file, error ";
			if (_itoa_s(*_errno(), &errorval[0], 20, 10))
			{
				Error += " with opening the file, and with converting error number.";
			}
			else
			{
				Error += "number [";
				Error += &errorval[0];
				Error += "] occured with opening the file.";
			}
			Error += "\r\nThe message has not been modified.";
			S.Error.Text = _strdup(Error.c_str());
			return;
		}
		// Jump to end
		fseek(File, 0, SEEK_END);
		// Read current position as file size
		long long filesize = _ftelli64(File);
		// Go back to start
		fseek(File, 0, SEEK_SET);
		long l;
		if ((l = fwrite(ThreadData.ReceivedMsg.Content + Position, 1, Size, File)) != Size)
		{
			char sizeastext[20];
			SaveExtInfo &S = AddEvent(0);
			std::string Error = "Couldn't save the received binary to file, ";
			if (_itoa_s(errno, &sizeastext[0], 20, 10))
			{
				Error += " and error copying size.";
			}
			else
			{
				Error += &sizeastext[0];
				Error += " bytes managed to be written.";
			}
			S.Error.Text = _strdup(Error.c_str());
		}
		fclose(File);
	}
}
void Extension::AppendReceivedBinaryToFile(int Position, int Size, char * Filename)
{
	if (Position < 0)
		CreateError("Cannot append received binary; Position less than 0.");
	else if (Size <= 0)
		CreateError("Cannot append received binary; Size equal or less than 0.");
	else if (!Filename || Filename[0] == '\0')
		CreateError("Cannot append received binary; filename is invalid.");
	else if (ThreadData.ReceivedMsg.Size - Size <= 0)
		CreateError("Cannot append received binary; Message is too small.");
	else
	{
		// Open while denying write of other programs
		FILE * File = _fsopen(Filename, "ab", SH_DENYWR);
		if (!File)
		{
			char errorval[20];
			SaveExtInfo &S = AddEvent(0);
			std::string Error = "Cannot append received binary to file, error ";
			if (_itoa_s(*_errno(), &errorval[0], 20, 10))
			{
				Error += " with opening the file, and with converting error number.";
			}
			else
			{
				Error += "number [";
				Error += &errorval[0];
				Error += "] occured with opening the file.";
			}
			Error += "\r\nThe message has not been modified.";
			S.Error.Text = _strdup(Error.c_str());
			return;
		}
		long l;
		if ((l = fwrite(ThreadData.ReceivedMsg.Content + Position, 1, Size, File)) != Size)
		{
			char sizeastext[20];
			SaveExtInfo &S = AddEvent(0);
			std::string Error = "Couldn't append the received binary to file, ";
			if (_itoa_s(errno, &sizeastext[0], 20, 10))
			{
				Error += " and error copying size.";
			}
			else
			{
				Error += &sizeastext[0];
				Error += " bytes managed to be append.";
			}
			S.Error.Text = _strdup(Error.c_str());
		}
		fclose(File);
	}
}
