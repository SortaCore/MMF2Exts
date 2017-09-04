
#include "Common.h"
#ifdef _DEBUG
std::stringstream CriticalSection;
#endif

const static int X = -200; // Mystical placeholder
#define Remake(name) MessageBoxA(NULL, "Your "#name" actions need to be recreated.\r\n" \
									   "This is probably due to parameter changes.", "Lacewing Blue Client - DarkEDIF", MB_OK)
#define Saved (Globals->_Saved)

void Extension::RemovedActionNoParams()
{
	CreateError("Action needs removing.");
}
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


static GlobalInfo::AutoResponse ConvToAutoResponse(int informFusion, int immediateRespondWith,
	char *& denyReason, GlobalInfo * Globals, const char * const funcName)
{
	static char err[256];

	// Settings:
	//	["Integer", "Approve immediately (0), deny immediately (1), or wait for Fusion (2)?"],
	// Auto approve, later inform Fusion [1, 0]
	// Auto deny, later inform Fusion [1, 1]
	// Wait for Fusion to say yay or nay [1, 2]
	// Auto approve, say nothing to Fusion [0, 0]
	// Auto deny, say nothing to Fusion [0, 1]
	// Do nothing, say nothing to Fusion [0, 2] -> not usable!
	
	if (informFusion < 0 || informFusion > 1)
		sprintf_s(err, sizeof(err), "Invalid \"Inform Fusion\" parameter passed to \"enable/disable condition: %s\".", funcName);
	else if (immediateRespondWith < 0 || immediateRespondWith > 2)
		sprintf_s(err, sizeof(err), "Invalid \"Immediate Respond With\" parameter passed to \"enable/disable condition: %s\".", funcName);
	else if (informFusion == 0 && immediateRespondWith == 2)
		sprintf_s(err, sizeof(err), "Invalid parameters passed to \"enable/disable condition: %s\"; with no immediate response"
			" and Fusion condition triggering off, the server wouldn't know what to do.", funcName);
	else 
	{ 
		// If we're not denying, replace deny parameter with null
		if (immediateRespondWith != 1)
			denyReason = nullptr;
		else if (!denyReason[0])
			denyReason = "No reason specified.";
		
		GlobalInfo::AutoResponse autoResponse = GlobalInfo::AutoResponse::Invalid;
		if (informFusion == 1)
		{
			if (immediateRespondWith == 0)
				autoResponse = GlobalInfo::AutoResponse::Approve_TellFusion;
			else if (immediateRespondWith == 1)
				autoResponse = GlobalInfo::AutoResponse::Deny_TellFusion;
			else /* immediateRespondWith == 2 */ 
				autoResponse = GlobalInfo::AutoResponse::WaitForFusion;
		}
		else /* informFusion == 0 */
		{
			if (immediateRespondWith == 0)
				autoResponse = GlobalInfo::AutoResponse::Approve_Quiet;
			else /* if (immediateRespondWith == 1) */
				autoResponse = GlobalInfo::AutoResponse::Deny_Quiet;
			/* immediateRespondWith == 2 is invalid with informFusion = 0 */;
		}

		return autoResponse;
	}

	Globals->CreateError(err);
	return GlobalInfo::AutoResponse::Invalid;
}


void Extension::EnableCondition_OnConnectRequest(int informFusion, int immediateRespondWith, char * autoDenyReason)
{
	GlobalInfo::AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		autoDenyReason, Globals, "on connect request");
	if (resp == GlobalInfo::AutoResponse::Invalid)
		return;
	Globals->AutoResponse_Connect = resp;
	free((char *)Globals->AutoResponse_Connect_DenyReason);
	Globals->AutoResponse_Connect_DenyReason = autoDenyReason ? _strdup(autoDenyReason) : nullptr;
	Srv.onconnect(::OnClientConnectRequest);
}
void Extension::EnableCondition_OnNameSetRequest(int informFusion, int immediateRespondWith, char * autoDenyReason)
{
	GlobalInfo::AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		autoDenyReason, Globals, "on name set request");
	if (resp == GlobalInfo::AutoResponse::Invalid)
		return;
	Globals->AutoResponse_NameSet = resp;
	free((char *)Globals->AutoResponse_NameSet_DenyReason);
	Globals->AutoResponse_NameSet_DenyReason = autoDenyReason ? _strdup(autoDenyReason) : nullptr;
	Srv.onnameset(resp != GlobalInfo::AutoResponse::Approve_Quiet ? ::OnNameSetRequest : nullptr);
}
void Extension::EnableCondition_OnJoinChannelRequest(int informFusion, int immediateRespondWith, char * autoDenyReason)
{
	GlobalInfo::AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		autoDenyReason, Globals, "on join channel request");
	if (resp == GlobalInfo::AutoResponse::Invalid)
		return;
	Globals->AutoResponse_ChannelJoin = resp;
	free((char *)Globals->AutoResponse_ChannelJoin_DenyReason);
	Globals->AutoResponse_ChannelJoin_DenyReason = autoDenyReason ? _strdup(autoDenyReason) : nullptr;
	Srv.onchannel_join(::OnJoinChannelRequest);
}
void Extension::EnableCondition_OnLeaveChannelRequest(int informFusion, int immediateRespondWith, char * autoDenyReason)
{
	GlobalInfo::AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith, 
		autoDenyReason, Globals, "on join channel request");
	if (resp == GlobalInfo::AutoResponse::Invalid)
		return;
	Globals->AutoResponse_ChannelLeave = resp;
	free((char *)Globals->AutoResponse_ChannelLeave_DenyReason);
	Globals->AutoResponse_ChannelLeave_DenyReason = autoDenyReason ? _strdup(autoDenyReason) : nullptr;
	Srv.onchannel_leave(::OnLeaveChannelRequest);
}
void Extension::EnableCondition_OnMessageToPeer(int informFusion, int immediateRespondWith)
{
	char * dummyDenyReason = "X";
	GlobalInfo::AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		dummyDenyReason, Globals, "on join channel request");
	if (resp == GlobalInfo::AutoResponse::Invalid)
		return;
	Globals->AutoResponse_MessageClient = resp;
	Srv.onmessage_peer(resp != GlobalInfo::AutoResponse::Approve_Quiet ? ::OnPeerMessage : nullptr);
}
void Extension::EnableCondition_OnMessageToChannel(int informFusion, int immediateRespondWith)
{
	char * dummyDenyReason = "X";
	GlobalInfo::AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		dummyDenyReason, Globals, "on join channel request");
	if (resp == GlobalInfo::AutoResponse::Invalid)
		return;
	Globals->AutoResponse_MessageChannel = resp;
	Srv.onmessage_channel(resp != GlobalInfo::AutoResponse::Approve_Quiet ? ::OnChannelMessage : nullptr);
}
void Extension::EnableCondition_OnMessageToServer(int informFusion)
{
	if (informFusion < 0 || informFusion > 1)
		Globals->CreateError("Invalid \"Inform Fusion\" parameter passed to \"enable/disable condition: on message to server\".");
	else
	{
		// This one's handled a bit differently; there is no auto approve/deny. 
		// The message is either read by Fusion or discarded immediately.
		Globals->AutoResponse_MessageServer = informFusion == 1 ? GlobalInfo::WaitForFusion : GlobalInfo::Deny_Quiet;
		Srv.onmessage_server(informFusion == 1 ? ::OnServerMessage : nullptr);
	}
}
void Extension::OnInteractive_Deny(char * Reason)
{
	if (!Reason)
		CreateError("Can't deny client's action: Deny was called with a null reason.");
	else if (InteractivePending == InteractiveType::None)
		CreateError("Cannot deny client's action: No interactive action is pending.");
	// All of the interactive events currently allow denying
	//else if ((InteractivePending & InteractiveType::DenyPermitted) != InteractiveType::DenyPermitted)
	//	CreateError("Cannot deny client's action: Interactive event is not compatible with this action.");
	else if (DenyReason)
		CreateError("Can't deny client's action: Set to auto-deny, or Deny was called more than once. Ignoring additional denies.");
	else
		DenyReason = _strdup(Reason[0] ? Reason : "No reason specified.");
}
void Extension::OnInteractive_ChangeClientName(char * NewName)
{
	if (!NewName || NewName[0] == '\0')
		CreateError("Cannot change new client name: Cannot use a null or blank name.");
	else if (strnlen(NewName, 256) == 256)
		CreateError("Cannot change new client name: Cannot use a name longer than 255 characters.");
	else if (InteractivePending == InteractiveType::None)
		CreateError("Cannot change new client name: No interactive action is pending.");
	else if (InteractivePending != InteractiveType::ClientNameSet)
		CreateError("Cannot change new client name: Interactive event is not compatible with this action.");
	else if (DenyReason)
		CreateError("Cannot change new client name: Name change has already been denied by the Deny Request action.");
	else
	{
		free(NewClientName);
		NewClientName = _strdup(NewName);
	}
}
void Extension::OnInteractive_ChangeChannelName(char * NewName)
{
	if (!NewName || NewName[0] == '\0')
		CreateError("Cannot change joining channel name: Cannot use a null or blank name.");
	else if (strnlen(NewName, 256) == 256)
		CreateError("Cannot change joining channel name: Cannot use a name longer than 255 characters.");
	else if (InteractivePending == InteractiveType::None)
		CreateError("Cannot change joining channel name: No interactive action is pending.");
	else if ((InteractivePending & InteractiveType::ChannelJoin) != InteractiveType::ChannelJoin)
		CreateError("Cannot change joining channel name: Interactive event is not compatible with this action.");
	else if (DenyReason)
		CreateError("Cannot change joining channel name: Channel name join has already been denied by the Deny Request action.");
	else
	{
		free(NewChannelName);
		NewChannelName = _strdup(NewName);
	}
}
void Extension::OnInteractive_DropMessage()
{
	if (InteractivePending == InteractiveType::None)
		CreateError("Cannot deny the action: No interactive action is pending.");
	else if (InteractivePending != InteractiveType::ChannelMessageIntercept &&
			 InteractivePending != InteractiveType::ClientMessageIntercept)
		CreateError("Cannot change joining channel name: Interactive event is not compatible with this action.");
	else if (DropMessage)
		CreateError("Cannot drop message: Message already being dropped. Ignoring additional actions.");
	else
		DropMessage = true;
}
void Extension::OnInteractive_ReplaceMessageWithText(char * NewText)
{
	if (InteractivePending == InteractiveType::None)
		CreateError("Cannot deny the action: No interactive action is pending.");
	else if (InteractivePending != InteractiveType::ChannelMessageIntercept &&
			 InteractivePending != InteractiveType::ClientMessageIntercept)
		CreateError("Cannot replace message: Interactive event is not compatible with this action.");
	else if (DropMessage)
		CreateError("Cannot replace message: Message was dropped.");
	else
	{
		CreateError("Cannot replace message: Replacing messages not implemented.");
		return;
		//ThreadData.ReceivedMsg.Size = strlen(NewText) + 1;
		//ThreadData.ReceivedMsg.Content = (char *)realloc(ThreadData.ReceivedMsg.Content, ThreadData.ReceivedMsg.Size);
		//strcpy_s(ThreadData.ReceivedMsg.Content, ThreadData.ReceivedMsg.Size, NewText);
	}

}
void Extension::OnInteractive_ReplaceMessageWithNumber(int NewNumber)
{
	if (InteractivePending == InteractiveType::None)
		CreateError("Cannot deny the action: No interactive action is pending.");
	else if (InteractivePending != InteractiveType::ChannelMessageIntercept &&
			 InteractivePending != InteractiveType::ClientMessageIntercept)
		CreateError("Cannot replace message: Interactive event is not compatible with this action.");
	else if (DropMessage)
		CreateError("Cannot replace message: Message was dropped.");
	else
	{
		CreateError("Cannot replace message: Replacing messages not implemented.");
		return;
	}
}
void Extension::OnInteractive_ReplaceMessageWithSendBinary()
{
	if (InteractivePending == InteractiveType::None)
		CreateError("Cannot deny the action: No interactive action is pending.");
	else if (InteractivePending != InteractiveType::ChannelMessageIntercept &&
			 InteractivePending != InteractiveType::ClientMessageIntercept)
		CreateError("Cannot replace message: Interactive event is not compatible with this action.");
	else if (DropMessage)
		CreateError("Cannot replace message: Message was dropped.");
	else
	{
		CreateError("Cannot replace message: Replacing messages not implemented.");
		return;
	}
}

void Extension::Channel_SelectByName(char * Name)
{
	if (!Name || Name[0] == '\0')
		CreateError("Channel_SelectByName() was called with a null or blank name.");
	else if (strnlen(Name, 256) == 256)
		CreateError("Channel_SelectByName() was called with a name exceeding the max length of 255 characters.");
	else
	{
		ThreadData.Channel = nullptr;
		for (auto i = Channels.begin(); i != Channels.end(); i++)
		{
			if (!_stricmp((**i).name(), Name))
			{
				ThreadData.Channel = *i;
				return;
			}
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
		ChannelCopy * CurrentChannel = ThreadData.Channel;

		char * Stored = (char *)malloc(sizeof(SaveExtInfo));
		memcpy_s(Stored, sizeof(SaveExtInfo), &ThreadData, sizeof(SaveExtInfo));
		
		auto& clients = CurrentChannel->getclients();
		for (auto SelectedClient = clients.begin(); SelectedClient != clients.end(); SelectedClient++)
		{
			if (!(**SelectedClient).isclosed)
			{
				ClearThreadData();
				ThreadData.Channel = CurrentChannel;
				ThreadData.Client = *SelectedClient;
				Runtime.GenerateEvent(8);
			}
		}

		memcpy_s(&ThreadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
		free(Stored);

		Runtime.GenerateEvent(44);
	}
}
void Extension::Channel_LoopClientsWithName(char * LoopName)
{
	if (!ThreadData.Channel)
		CreateError("Error: Loop Clients On Channel With Name was called without a channel being selected.");
	else // You can loop a closed channel's clients, but it's read-only.
	{
		ChannelCopy * CurrentChannel = ThreadData.Channel;

		char * Stored = (char *)malloc(sizeof(SaveExtInfo));
		memcpy_s(Stored, sizeof(SaveExtInfo), &ThreadData, sizeof(SaveExtInfo));

		auto clients = CurrentChannel->getclients();
		for (auto SelectedClient = clients.begin(); SelectedClient != clients.end(); SelectedClient++)
		{
			if (!(**SelectedClient).isclosed)
			{
				ClearThreadData();
				ThreadData.Channel = CurrentChannel;
				ThreadData.Client = *SelectedClient;
				ThreadData.Loop.Name = _strdup(LoopName);
				Runtime.GenerateEvent(39);
			}
		}

		memcpy_s(&ThreadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
		free(Stored);

		ThreadData.Loop.Name = _strdup(LoopName);
		Runtime.GenerateEvent(40);
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
		ThreadData.Channel->SetLocalData(Key, Value);
}
void Extension::LoopAllChannels()
{
	char * Stored = (char *)malloc(sizeof(SaveExtInfo));
	memcpy_s(Stored, sizeof(SaveExtInfo), &ThreadData, sizeof(SaveExtInfo));

	for (auto SelectedChannel = Channels.begin(); SelectedChannel != Channels.end(); SelectedChannel++)
	{
		ClearThreadData();
		ThreadData.Channel = *SelectedChannel;
		Runtime.GenerateEvent(5);
	}

	memcpy_s(&ThreadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
	free(Stored);

	Runtime.GenerateEvent(45);
}
void Extension::LoopAllChannelsWithName(char * LoopName)
{
	char * Stored = (char *)malloc(sizeof(SaveExtInfo));
	memcpy_s(Stored, sizeof(SaveExtInfo), &ThreadData, sizeof(SaveExtInfo));

	for (auto SelectedChannel = Channels.begin(); SelectedChannel != Channels.end(); SelectedChannel++)
	{
		ClearThreadData();
		ThreadData.Channel = *SelectedChannel;
		ThreadData.Loop.Name = _strdup(LoopName);
		Runtime.GenerateEvent(36);
	}

	memcpy_s(&ThreadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
	free(Stored);

	ThreadData.Loop.Name = _strdup(LoopName);
	Runtime.GenerateEvent(41);
}
void Extension::Client_Disconnect()
{
	if (!ThreadData.Client)
		CreateError("Could not disconnect client: No client selected.");
	else if (!ThreadData.Client->isclosed)
		ThreadData.Client->disconnect(true);
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
		ThreadData.Client->SetLocalData(Key, Value);
}
void Extension::Client_LoopJoinedChannels()
{
	if (!ThreadData.Client)
		CreateError("Cannot loop client's joined channels: No client selected.");
	else
	{
		char * Stored = (char *)malloc(sizeof(SaveExtInfo));
		memcpy_s(Stored, sizeof(SaveExtInfo), &ThreadData, sizeof(SaveExtInfo));
		
		ClientCopy * StoredCli = ThreadData.Client;
		auto &channels = StoredCli->getchannels();
		for (auto SelectedChannel = channels.begin(); SelectedChannel != channels.end(); SelectedChannel++)
		{
			if (!(**SelectedChannel).isclosed)
			{
				ClearThreadData();
				ThreadData.Channel = *SelectedChannel;
				ThreadData.Client = StoredCli;
				Runtime.GenerateEvent(6);
			}
		}

		memcpy_s(&ThreadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
		free(Stored);

		Runtime.GenerateEvent(47);
	}
}
void Extension::Client_LoopJoinedChannelsWithName(char * LoopName)
{
	if (!ThreadData.Client)
		CreateError("Cannot loop client's joined channels: No client selected.");
	else
	{
		char * Stored = (char *)malloc(sizeof(SaveExtInfo));
		memcpy_s(Stored, sizeof(SaveExtInfo), &ThreadData, sizeof(SaveExtInfo));

		auto& channels = ThreadData.Client->getchannels();
		ClientCopy * StoredCli = ThreadData.Client;
		for (auto SelectedChannel = channels.begin(); SelectedChannel != channels.end(); SelectedChannel++)
		{
			if (!(**SelectedChannel).isclosed)
			{
				ClearThreadData();
				ThreadData.Channel = *SelectedChannel;
				ThreadData.Client = StoredCli;
				ThreadData.Loop.Name = _strdup(LoopName);
				Runtime.GenerateEvent(37);
			}
		}

		memcpy_s(&ThreadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
		free(Stored);

		ThreadData.Loop.Name = _strdup(LoopName);
		Runtime.GenerateEvent(43);
	}
}
void Extension::Client_SelectByName(char * ClientName)
{
	if (!ClientName || ClientName[0] == '\0')
		CreateError("Error: Select Client By Name was called with a null parameter/blank name.");
	else
	{
		auto Selected = Clients.begin();
		for (; Selected != Clients.end(); Selected++)
			if (!_stricmp((**Selected).name(), ClientName))
				break;
		// Only modify ThreadData.Client if we found it
		if (Selected != Clients.end() && !(**Selected).isclosed)
		{
			auto& ch = (**Selected).getchannels();
			if (std::find(ch.cbegin(), ch.cend(), ThreadData.Channel) == ch.cend())
				ThreadData.Channel = nullptr;
			ThreadData.Client = *Selected;
		}
		else
		{
			std::stringstream Error;
			Error << "Client with name " << ClientName <<" not found on server.";
			CreateError(Error.str().c_str());
		}
	}
}
void Extension::Client_SelectByID(int ClientID)
{
	if (ClientID < 0 || ClientID > 0xFFFF)
		CreateError("Could not select Client on channel, ID is below 0 or greater than 65535.");
	else
	{
		auto Selected = Clients.begin();
		for (; Selected != Clients.end(); Selected++)
			if ((**Selected).id() == ClientID)
				break;
		
		if (Selected != Clients.end() && !(**Selected).isclosed)
			ThreadData.Client = *Selected;
		else
		{
			std::stringstream Error;
			Error << "Client ID " << ClientID << " was not found on server.";
			CreateError(Error.str().c_str());
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
	char * Stored = (char *)malloc(sizeof(SaveExtInfo));
	memcpy_s(Stored, sizeof(SaveExtInfo), &ThreadData, sizeof(SaveExtInfo));
	
	for (auto SelectedClient = Clients.begin(); SelectedClient != Clients.end(); SelectedClient++)
	{
		ClearThreadData();
		ThreadData.Client = *SelectedClient;
		Runtime.GenerateEvent(7);
	}

	memcpy_s(&ThreadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
	free(Stored);

	Runtime.GenerateEvent(46);
}
void Extension::LoopAllClientsWithName(char * LoopName)
{
	char * Stored = (char *)malloc(sizeof(SaveExtInfo));
	memcpy_s(Stored, sizeof(SaveExtInfo), &ThreadData, sizeof(SaveExtInfo));
	for (auto SelectedClient = Clients.begin(); SelectedClient != Clients.end(); SelectedClient++)
	{
		ClearThreadData();
		ThreadData.Client = *SelectedClient;
		ThreadData.Loop.Name = _strdup(LoopName);
		Runtime.GenerateEvent(38);
	}
	memcpy_s(&ThreadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
	free(Stored);

	ThreadData.Loop.Name = _strdup(LoopName);
	Runtime.GenerateEvent(42);
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
		CreateError("Add binary failed: Size less than 0.");
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
		// Open and deny other programs write priviledges
		FILE * File = _fsopen(Filename, "rb", _SH_DENYWR);
		if (!File)
		{
			std::stringstream Error;
			Error << "Cannot add file to send binary, error number " << errno
				<< " occured with opening the file. The send binary has not been modified.";
			CreateError(Error.str().c_str());
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
		{
			std::stringstream Error;
			Error << "Couldn't read file \"" << Filename << "\" into binary to send; couldn't reserve enough memory "
				"to add file into message. The send binary has not been modified.";
			CreateError(Error.str().c_str());
		}
		else
		{
			size_t amountRead;
			if ((amountRead = fread_s(buffer, filesize, 1, filesize, File)) != filesize)
			{
				std::stringstream Error;
				Error << "Couldn't read file \"" << Filename << "\" into binary to send, error number " << errno
					<< " occured with opening the file. The send binary has not been modified.";
				CreateError(Error.str().c_str());
			}
			else
				AddToSend(buffer, amountRead);

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
		CreateError("Cannot compress send binary; Message is too small.");
	else
	{
		int ret;
		z_stream strm = { 0 };

		ret = deflateInit(&strm, 9);
		if (ret)
		{
			std::stringstream error;
			error << "Error " << ret << " occured with initiating compression.";
			CreateError(error.str().c_str());
			return;
		}

		unsigned char * output_buffer = (unsigned char *)malloc(SendMsgSize + 256);
		if (!output_buffer)
		{
			std::stringstream error;
			error << "Error with compressing send binary, could not allocate enough memory. Desired " << SendMsgSize + 256 << " bytes.";
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
			error << "Error with compressing send binary, deflate() returned " << ret << ". Zlib error: " << (strm.msg ? strm.msg : "");
			free(output_buffer);
			deflateEnd(&strm);
			CreateError(error.str().c_str());
			return;
		}

		deflateEnd(&strm);

		void * output_bufferResize = realloc(output_buffer, strm.total_out);
		if (!output_bufferResize)
		{
			free(output_buffer); // realloc will not free on error
			CreateError("Error with compressing send binary, reallocating memory to remove excess space after compression failed.");
			return;
		}
		free(SendMsg);

		SendMsg = (char *)output_bufferResize;
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
		CreateError("Cannot decompress; message is too small.");
	else
	{
		z_stream strm = { 0 };
		int ret = inflateInit_(&strm, ZLIB_VERSION, sizeof(z_stream));
		if (ret)
		{
			std::stringstream error;
			error << "Error " << ret << " occured with initiating decompression.";
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
				error << "Error with decompression, could not allocate enough memory. Desired "
					<< (output_buffer ? _msize(output_buffer) : 0) + 1024 << " bytes.";
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
				error << "Error with decompression, inflate() returned error " << ret
					<< ". Zlib error: " << (strm.msg ? strm.msg : "");
				free(output_buffer);
				inflateEnd(&strm);
				CreateError(error.str().c_str());
				return;
			}

		} while (strm.avail_in != 0);

		if (ret < 0)
		{
			std::stringstream error;
			error << "Error with decompression: " << ret << "! Text: " << strm.msg ? strm.msg : "";
			inflateEnd(&strm);
			CreateError(error.str().c_str());
			return;
		}
		inflateEnd(&strm);

		// Update all extensions with new message content.
		char * ThisMsg = ThreadData.ReceivedMsg.Content;
		free(ThreadData.ReceivedMsg.Content);

		for (std::vector<SaveExtInfo *>::iterator i = Saved.begin(); i != Saved.end(); ++i)
		{
			if ((char *)output_buffer == (*i)->ReceivedMsg.Content)
				continue;
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
	else if (((unsigned int)Position) + Size > ThreadData.ReceivedMsg.Size)
	{
		std::stringstream Error;
		Error << "Cannot save received binary to file; message doesn't have " << Size
			<< " bytes at position " << Position << " onwards.";
		CreateError(Error.str().c_str());
		return;
	}
	else
	{
		FILE * File = _fsopen(Filename, "wb", SH_DENYWR);
		if (!File)
		{
			std::stringstream Error;
			Error << "Cannot save received binary to file, error number " << errno
				<< " occured with opening the file. The message has not been modified.";
			CreateError(Error.str().c_str());
			return;
		}

		size_t amountWritten;
		if ((amountWritten = fwrite(ThreadData.ReceivedMsg.Content + Position, 1, Size, File)) != Size)
		{
			std::stringstream Error;
			Error << "Cannot save received binary to file, error number " << errno
				<< " occured with writing the file. Wrote " << amountWritten
				<< " bytes total. The message has not been modified.";
			CreateError(Error.str().c_str());
		}

		fclose(File);
	}
}
void Extension::AppendReceivedBinaryToFile(int Position, int Size, char * Filename)
{
	if (Position < 0)
		CreateError("Cannot append received binary to file; position less than 0.");
	else if (Size <= 0)
		CreateError("Cannot append received binary to file; size equal or less than 0.");
	else if (!Filename || Filename[0] == '\0')
		CreateError("Cannot append received binary to file; filename is invalid.");
	else if (((unsigned int)Position) + Size > ThreadData.ReceivedMsg.Size)
	{
		std::stringstream Error;
		Error << "Cannot append received binary to file; message doesn't have " << Size
			<< " bytes at position " << Position << " onwards.";
		CreateError(Error.str().c_str());
		return;
	}
	else
	{
		// Open while denying write of other programs
		FILE * File = _fsopen(Filename, "ab", SH_DENYWR);
		if (!File)
		{
			std::stringstream Error;
			Error << "Cannot append received binary to file, error number " << errno
				<< " occured with opening the file. The binary message has not been modified.";
			CreateError(Error.str().c_str());
			return;
		}

		size_t amountWritten;
		if ((amountWritten = fwrite(ThreadData.ReceivedMsg.Content + Position, 1, Size, File)) != Size)
		{
			fseek(File, 0, SEEK_END);
			long long filesize = _ftelli64(File);

			std::stringstream Error;
			Error << "Cannot append received binary to file, error number " << errno
				<< " occured with writing the file. Wrote " << amountWritten
				<< " bytes, leaving file at size " << filesize << " total. The binary message has not been modified.";
			CreateError(Error.str().c_str());
		}

		fclose(File);
	}
}
