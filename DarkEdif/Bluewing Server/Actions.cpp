
#include "Common.h"
#ifdef _DEBUG
std::stringstream CriticalSection;
#endif

const static int X = -200; // Mystical placeholder
#define Remake(name) MessageBoxA(NULL, "Your "#name" actions need to be recreated.\r\n" \
										"This is probably due to parameter changes.", "Lacewing Blue Client - DarkEdif", MB_OK)
#define Saved (globals->_Saved)

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
void Extension::SetWelcomeMessage(char * message)
{
	Srv.setwelcomemessage(message);
}


static GlobalInfo::AutoResponse ConvToAutoResponse(int informFusion, int immediateRespondWith,
	char *& denyReason, GlobalInfo * globals, const char * const funcName)
{
	static char err[256];

	// Settings:
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

	globals->CreateError(err);
	return GlobalInfo::AutoResponse::Invalid;
}


void Extension::EnableCondition_OnConnectRequest(int informFusion, int immediateRespondWith, char * autoDenyReason)
{
	GlobalInfo::AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		autoDenyReason, globals, "on connect request");
	if (resp == GlobalInfo::AutoResponse::Invalid)
		return;
	globals->AutoResponse_Connect = resp;
	free((char *)globals->AutoResponse_Connect_DenyReason);
	globals->AutoResponse_Connect_DenyReason = autoDenyReason ? _strdup(autoDenyReason) : nullptr;
	Srv.onconnect(::OnClientConnectRequest);
}
void Extension::EnableCondition_OnNameSetRequest(int informFusion, int immediateRespondWith, char * autoDenyReason)
{
	GlobalInfo::AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		autoDenyReason, globals, "on name set request");
	if (resp == GlobalInfo::AutoResponse::Invalid)
		return;
	globals->AutoResponse_NameSet = resp;
	free((char *)globals->AutoResponse_NameSet_DenyReason);
	globals->AutoResponse_NameSet_DenyReason = autoDenyReason ? _strdup(autoDenyReason) : nullptr;
	Srv.onnameset(resp != GlobalInfo::AutoResponse::Approve_Quiet ? ::OnNameSetRequest : nullptr);
}
void Extension::EnableCondition_OnJoinChannelRequest(int informFusion, int immediateRespondWith, char * autoDenyReason)
{
	GlobalInfo::AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		autoDenyReason, globals, "on join channel request");
	if (resp == GlobalInfo::AutoResponse::Invalid)
		return;
	globals->AutoResponse_ChannelJoin = resp;
	free((char *)globals->AutoResponse_ChannelJoin_DenyReason);
	globals->AutoResponse_ChannelJoin_DenyReason = autoDenyReason ? _strdup(autoDenyReason) : nullptr;
	Srv.onchannel_join(::OnJoinChannelRequest);
}
void Extension::EnableCondition_OnLeaveChannelRequest(int informFusion, int immediateRespondWith, char * autoDenyReason)
{
	GlobalInfo::AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith, 
		autoDenyReason, globals, "on join channel request");
	if (resp == GlobalInfo::AutoResponse::Invalid)
		return;
	globals->autoResponse_ChannelLeave = resp;
	free((char *)globals->AutoResponse_ChannelLeave_DenyReason);
	globals->AutoResponse_ChannelLeave_DenyReason = autoDenyReason ? _strdup(autoDenyReason) : nullptr;
	Srv.onchannel_leave(::OnLeaveChannelRequest);
}
void Extension::EnableCondition_OnMessageToPeer(int informFusion, int immediateRespondWith)
{
	char * dummyDenyReason = "X";
	GlobalInfo::AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		dummyDenyReason, globals, "on join channel request");
	if (resp == GlobalInfo::AutoResponse::Invalid)
		return;
	globals->AutoResponse_MessageClient = resp;
	Srv.onmessage_peer(resp != GlobalInfo::AutoResponse::Approve_Quiet ? ::OnPeerMessage : nullptr);
}
void Extension::EnableCondition_OnMessageToChannel(int informFusion, int immediateRespondWith)
{
	char * dummyDenyReason = "X";
	GlobalInfo::AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		dummyDenyReason, globals, "on join channel request");
	if (resp == GlobalInfo::AutoResponse::Invalid)
		return;
	globals->AutoResponse_MessageChannel = resp;
	Srv.onmessage_channel(resp != GlobalInfo::AutoResponse::Approve_Quiet ? ::OnChannelMessage : nullptr);
}
void Extension::EnableCondition_OnMessageToServer(int informFusion)
{
	if (informFusion < 0 || informFusion > 1)
		globals->CreateError("Invalid \"Inform Fusion\" parameter passed to \"enable/disable condition: on message to server\".");
	else
	{
		// This one's handled a bit differently; there is no auto approve/deny. 
		// The message is either read by Fusion or discarded immediately.
		globals->AutoResponse_MessageServer = informFusion == 1 ? GlobalInfo::WaitForFusion : GlobalInfo::Deny_Quiet;
		Srv.onmessage_server(informFusion == 1 ? ::OnServerMessage : nullptr);
	}
}
void Extension::OnInteractive_Deny(char * reason)
{
	if (InteractivePending == InteractiveType::None)
		CreateError("Cannot deny client's action: No interactive action is pending.");
	// All of the interactive events currently allow denying
	//else if ((InteractivePending & InteractiveType::DenyPermitted) != InteractiveType::DenyPermitted)
	//	CreateError("Cannot deny client's action: Interactive event is not compatible with this action.");
	else if (DenyReason)
		CreateError("Can't deny client's action: Set to auto-deny, or Deny was called more than once. Ignoring additional denies.");
	else
		DenyReason = _strdup(reason[0] ? reason : "No reason specified.");
}
void Extension::OnInteractive_ChangeClientName(char * newName)
{
	if (newName[0] == '\0')
		CreateError("Cannot change new client name: Cannot use a blank name.");
	else if (strnlen(newName, 256) == 256)
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
		NewClientName = _strdup(newName);
	}
}
void Extension::OnInteractive_ChangeChannelName(char * newName)
{
	if (newName[0] == '\0')
		CreateError("Cannot change joining channel name: Cannot use a blank name.");
	else if (strnlen(newName, 256U) == 256U)
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
		NewChannelName = _strdup(newName);
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
		//threadData.receivedMsg.size = strlen(NewText) + 1;
		//threadData.receivedMsg.content = (char *)realloc(threadData.receivedMsg.content, threadData.receivedMsg.size);
		//strcpy_s(threadData.receivedMsg.content, threadData.receivedMsg.size, NewText);
	}

}
void Extension::OnInteractive_ReplaceMessageWithNumber(int newNumber)
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

void Extension::Channel_SelectByName(char * name)
{
	if (name[0] == '\0')
		CreateError("Channel_SelectByName() was called with a blank name.");
	else
	{
		size_t nameLen = strnlen(name, 256U);
		if (nameLen == 256U)
			CreateError("Channel_SelectByName() was called with a name exceeding the max length of 255 characters.");
		else
		{
			threadData.channel = nullptr;
			for (auto i = Channels.begin(); i != Channels.end(); i++)
			{
				if (!_strnicmp((**i).name(), name, nameLen))
				{
					threadData.channel = *i;
					return;
				}
			}

			CreateError("Selecting channel by name failed: A channel with that name doesn't exist.");
		}
	}
}
void Extension::Channel_Close()
{
	if (!threadData.channel)
		CreateError("Could not close channel: No channel selected.");
	else if (threadData.channel->isclosed)
		CreateError("Could not close channel: Already closing.");
	else
		threadData.channel->close();
}
void Extension::Channel_SelectMaster()
{
	if (!threadData.channel)
		CreateError("Could not select channel master: No channel selected.");
	else if (threadData.channel->isclosed)
		CreateError("Could not select channel master: Channel is closed.");
	else
		threadData.client = threadData.channel->channelmaster();
}
void Extension::Channel_LoopClients()
{
	if (!threadData.channel)
		CreateError("Error: Loop Clients On Channel was called without a channel being selected.");
	else // You can loop a closed channel's clients, but it's read-only.
	{
		ChannelCopy * currentChannel = threadData.channel;

		char * stored = (char *)malloc(sizeof(SaveExtInfo));
		memcpy_s(stored, sizeof(SaveExtInfo), &threadData, sizeof(SaveExtInfo));
		
		auto& channelClients = currentChannel->getclients();
		for (auto selectedClient = channelClients.begin(); selectedClient != channelClients.end(); selectedClient++)
		{
			if (!(**selectedClient).isclosed)
			{
				ClearThreadData();
				threadData.channel = currentChannel;
				threadData.client = *selectedClient;
				Runtime.GenerateEvent(8);
			}
		}

		memcpy_s(&threadData, sizeof(SaveExtInfo), stored, sizeof(SaveExtInfo));
		free(stored);

		Runtime.GenerateEvent(44);
	}
}
void Extension::Channel_LoopClientsWithName(char * loopName)
{
	if (!threadData.channel)
		CreateError("Error: Loop Clients On Channel With Name was called without a channel being selected.");
	else // You can loop a closed channel's clients, but it's read-only.
	{
		ChannelCopy * currentChannel = threadData.channel;

		char * stored = (char *)malloc(sizeof(SaveExtInfo));
		memcpy_s(stored, sizeof(SaveExtInfo), &threadData, sizeof(SaveExtInfo));

		auto channelClients = currentChannel->getclients();
		for (auto selectedClient = channelClients.begin(); selectedClient != channelClients.end(); selectedClient++)
		{
			if (!(**selectedClient).isclosed)
			{
				ClearThreadData();
				threadData.channel = currentChannel;
				threadData.client = *selectedClient;
				threadData.loop.name = _strdup(loopName);
				Runtime.GenerateEvent(39);
			}
		}

		memcpy_s(&threadData, sizeof(SaveExtInfo), stored, sizeof(SaveExtInfo));
		free(stored);

		threadData.loop.name = _strdup(loopName);
		Runtime.GenerateEvent(40);
	}
}
void Extension::Channel_SetLocalData(char * key, char * value)
{
	if (!threadData.channel)
		CreateError("Could not set channel local data: No channel selected.");
	else if (threadData.channel->isclosed)
		CreateError("Could not set channel local data: Channel is closed.");
	else
		threadData.channel->SetLocalData(key, value);
}
void Extension::LoopAllChannels()
{
	char * stored = (char *)malloc(sizeof(SaveExtInfo));
	memcpy_s(stored, sizeof(SaveExtInfo), &threadData, sizeof(SaveExtInfo));

	for (auto selectedChannel = Channels.begin(); selectedChannel != Channels.end(); selectedChannel++)
	{
		ClearThreadData();
		threadData.channel = *selectedChannel;
		Runtime.GenerateEvent(5);
	}

	memcpy_s(&threadData, sizeof(SaveExtInfo), stored, sizeof(SaveExtInfo));
	free(stored);

	Runtime.GenerateEvent(45);
}
void Extension::LoopAllChannelsWithName(char * loopName)
{
	char * stored = (char *)malloc(sizeof(SaveExtInfo));
	memcpy_s(stored, sizeof(SaveExtInfo), &threadData, sizeof(SaveExtInfo));

	for (auto selectedChannel = Channels.begin(); selectedChannel != Channels.end(); selectedChannel++)
	{
		ClearThreadData();
		threadData.channel = *selectedChannel;
		threadData.loop.name = _strdup(loopName);
		Runtime.GenerateEvent(36);
	}

	memcpy_s(&threadData, sizeof(SaveExtInfo), stored, sizeof(SaveExtInfo));
	free(stored);

	threadData.loop.name = _strdup(loopName);
	Runtime.GenerateEvent(41);
}
void Extension::Client_Disconnect()
{
	if (!threadData.client)
		CreateError("Could not disconnect client: No client selected.");
	else if (!threadData.client->isclosed)
		threadData.client->disconnect(true);
}
void Extension::Client_SetLocalData(char * key, char * value)
{
	if (!threadData.client)
		CreateError("Could not set client local data: No client selected.");
	else if (threadData.client->isclosed)
		CreateError("Could not set client local data: Client is closed.");
	else
		threadData.client->SetLocalData(key, value);
}
void Extension::Client_LoopJoinedChannels()
{
	if (!threadData.client)
		CreateError("Cannot loop client's joined channels: No client selected.");
	else
	{
		char * stored = (char *)malloc(sizeof(SaveExtInfo));
		memcpy_s(stored, sizeof(SaveExtInfo), &threadData, sizeof(SaveExtInfo));
		
		ClientCopy * StoredCli = threadData.client;
		auto &channels = StoredCli->getchannels();
		for (auto selectedChannel = channels.begin(); selectedChannel != channels.end(); selectedChannel++)
		{
			if (!(**selectedChannel).isclosed)
			{
				ClearThreadData();
				threadData.channel = *selectedChannel;
				threadData.client = StoredCli;
				Runtime.GenerateEvent(6);
			}
		}

		memcpy_s(&threadData, sizeof(SaveExtInfo), stored, sizeof(SaveExtInfo));
		free(stored);

		Runtime.GenerateEvent(47);
	}
}
void Extension::Client_LoopJoinedChannelsWithName(char * loopName)
{
	if (!threadData.client)
		CreateError("Cannot loop client's joined channels: No client selected.");
	else
	{
		char * stored = (char *)malloc(sizeof(SaveExtInfo));
		memcpy_s(stored, sizeof(SaveExtInfo), &threadData, sizeof(SaveExtInfo));

		auto& channels = threadData.client->getchannels();
		ClientCopy * storedCli = threadData.client;
		for (auto selectedChannel = channels.begin(); selectedChannel != channels.end(); selectedChannel++)
		{
			if (!(**selectedChannel).isclosed)
			{
				ClearThreadData();
				threadData.channel = *selectedChannel;
				threadData.client = storedCli;
				threadData.loop.name = _strdup(loopName);
				Runtime.GenerateEvent(37);
			}
		}

		memcpy_s(&threadData, sizeof(SaveExtInfo), stored, sizeof(SaveExtInfo));
		free(stored);

		threadData.loop.name = _strdup(loopName);
		Runtime.GenerateEvent(43);
	}
}
void Extension::Client_SelectByName(char * clientName)
{
	if (clientName[0] == '\0')
		CreateError("Error: Select Client By Name was called with a null parameter/blank name.");
	else
	{
		auto Selected = Clients.begin();
		for (; Selected != Clients.end(); Selected++)
			if (!_stricmp((**Selected).name(), clientName))
				break;
		// Only modify threadData.client if we found it
		if (Selected != Clients.end() && !(**Selected).isclosed)
		{
			auto& ch = (**Selected).getchannels();
			if (std::find(ch.cbegin(), ch.cend(), threadData.channel) == ch.cend())
				threadData.channel = nullptr;
			threadData.client = *Selected;
		}
		else
		{
			std::stringstream error;
			error << "Client with name " << clientName <<" not found on server.";
			CreateError(error.str().c_str());
		}
	}
}
void Extension::Client_SelectByID(int clientID)
{
	if (clientID < 0 || clientID > 0xFFFF)
		CreateError("Could not select Client on channel, ID is below 0 or greater than 65535.");
	else
	{
		auto Selected = Clients.begin();
		for (; Selected != Clients.end(); Selected++)
			if ((**Selected).id() == clientID)
				break;
		
		if (Selected != Clients.end() && !(**Selected).isclosed)
			threadData.client = *Selected;
		else
		{
			std::stringstream error;
			error << "Client ID " << clientID << " was not found on server.";
			CreateError(error.str().c_str());
		}
	}
}
void Extension::Client_SelectSender()
{
	if (!threadData.senderClient)
		CreateError("Cannot select sending client: No sending client variable available.");
	else
		threadData.client = threadData.senderClient;
}
void Extension::Client_SelectReceiver()
{
	if (!threadData.ReceivingClient)
		CreateError("Cannot select receiving client: No receiving client variable available.");
	else
		threadData.client = threadData.ReceivingClient;
}
void Extension::LoopAllClients()
{
	char * stored = (char *)malloc(sizeof(SaveExtInfo));
	memcpy_s(stored, sizeof(SaveExtInfo), &threadData, sizeof(SaveExtInfo));
	
	for (auto selectedClient = Clients.begin(); selectedClient != Clients.end(); selectedClient++)
	{
		ClearThreadData();
		threadData.client = *selectedClient;
		Runtime.GenerateEvent(7);
	}

	memcpy_s(&threadData, sizeof(SaveExtInfo), stored, sizeof(SaveExtInfo));
	free(stored);

	Runtime.GenerateEvent(46);
}
void Extension::LoopAllClientsWithName(char * loopName)
{
	char * Stored = (char *)malloc(sizeof(SaveExtInfo));
	memcpy_s(Stored, sizeof(SaveExtInfo), &threadData, sizeof(SaveExtInfo));
	
	for (auto selectedClient = Clients.begin(); selectedClient != Clients.end(); selectedClient++)
	{
		ClearThreadData();
		threadData.client = *selectedClient;
		threadData.loop.name = _strdup(loopName);
		Runtime.GenerateEvent(38);
	}

	memcpy_s(&threadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
	free(Stored);

	threadData.loop.name = _strdup(loopName);
	Runtime.GenerateEvent(42);
}
void Extension::SendTextToChannel(int subchannel, char * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Text to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!threadData.channel)
		CreateError("Error: Send Text to Channel was called without a channel being selected.");
	else if (threadData.channel->isclosed)
		CreateError("Error: Send Text to Channel was called with a closed channel.");
	else
		threadData.channel->send(subchannel, textToSend, strlen(textToSend) + 1U, 0);
}
void Extension::SendTextToClient(int subchannel, char * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Text to Client was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!threadData.client)
		CreateError("Error: Send Text to Client was called without a Client being selected.");
	else if (threadData.client->isclosed)
		CreateError("Error: Send Text to Client was called with a closed Client.");
	else
		threadData.client->send(subchannel, textToSend, strlen(textToSend) + 1U, 0);
}
void Extension::SendNumberToChannel(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Number to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!threadData.channel)
		CreateError("Error: Send Number to Channel was called without a channel being selected.");
	else if (threadData.channel->isclosed)
		CreateError("Error: Send Number to Channel was called with a closed channel.");
	else
		threadData.channel->send(subchannel, (char *)&numToSend, sizeof(int), 1);
}
void Extension::SendNumberToClient(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Number to Client was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!threadData.client)
		CreateError("Error: Send Number to Client was called without a Client being selected.");
	else if (threadData.client->isclosed)
		CreateError("Error: Send Number to Client was called with a closed Client.");
	else
		threadData.client->send(subchannel, (char *)&numToSend, sizeof(int), 1);
}
void Extension::SendBinaryToChannel(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!threadData.channel)
		CreateError("Error: Send Binary to Channel was called without a channel being selected.");
	else if (threadData.channel->isclosed)
		CreateError("Error: Send Binary to Channel was called with a closed channel.");
	else
		threadData.channel->send(subchannel, SendMsg, SendMsgSize, 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::SendBinaryToClient(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!threadData.client)
		CreateError("Error: Send Binary to Client was called without a Client being selected.");
	else if (threadData.client->isclosed)
		CreateError("Error: Send Binary to Client was called with a closed Client.");
	else
		threadData.client->send(subchannel, SendMsg, SendMsgSize, 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastTextToChannel(int subchannel, char * textToBlast)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Text to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!threadData.channel)
		CreateError("Error: Blast Text to Channel was called without a channel being selected.");
	else if (threadData.channel->isclosed)
		CreateError("Error: Blast Text to Channel was called with a closed channel.");
	else
		threadData.channel->blast(subchannel, textToBlast, strlen(textToBlast) + 1U, 0);
}
void Extension::BlastTextToClient(int subchannel, char * textToBlast)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Text to Client was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!threadData.client)
		CreateError("Error: Blast Text to Client was called without a Client being selected.");
	else if (threadData.client->isclosed)
		CreateError("Error: Blast Text to Client was called with a closed Client.");
	else
		threadData.client->blast(subchannel, textToBlast, strlen(textToBlast) + 1U, 0);
}
void Extension::BlastNumberToChannel(int subchannel, int numToBlast)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Number to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!threadData.channel)
		CreateError("Error: Blast Number to Channel was called without a channel being selected.");
	else
		threadData.channel->blast(subchannel, (char *)&numToBlast, 4U, 1);
}
void Extension::BlastNumberToClient(int subchannel, int numToBlast)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Number to Client was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!threadData.client)
		CreateError("Error: Blast Number to Client was called without a Client being selected.");
	else if (threadData.client->isclosed)
		CreateError("Error: Blast Number to Client was called with a closed Client.");
	else
		threadData.client->blast(subchannel, (char *)&numToBlast, 4U, 1);
}
void Extension::BlastBinaryToChannel(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!threadData.channel)
		CreateError("Error: Blast Binary to Channel was called without a channel being selected.");
	else if (threadData.channel->isclosed)
		CreateError("Error: Blast Binary to Channel was called with a closed channel.");
	else
		threadData.channel->blast(subchannel, SendMsg, SendMsgSize, 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastBinaryToClient(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!threadData.client)
		CreateError("Error: Blast Binary to Client was called without a Client being selected.");
	else if (threadData.client->isclosed)
		CreateError("Error: Blast Binary to Client was called with a closed Client.");
	else
		threadData.client->blast(subchannel, SendMsg, SendMsgSize, 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::AddByteText(char * byte)
{
	if (!byte || strnlen(byte, 2) != 1)
		CreateError("Adding byte to stack failed: byte supplied was part of a string, not a single byte.");
	else
		AddToSend(byte, 1);
}
void Extension::AddByteInt(int byte)
{
	if (byte > MAXUINT8 || byte < MININT8)
		CreateError("Error: Byte out of bounds.");
	else
	{
		if (byte < 0)
		{
			char realByte = (char)byte;
			AddToSend(&realByte, 1);
		}
		else
		{
			unsigned char realByte = (unsigned char)byte;
			AddToSend(&realByte, 1);
		}
	}
}
void Extension::AddShort(int _short)
{
	if (_short > MAXUINT16 || _short < MININT16)
		CreateError("Error: Short out of bounds.");
	else
	{
		if (_short < 0)
		{
			short realShort = (short)_short;
			AddToSend(&realShort, 2);
		}
		else
		{
			unsigned short realShort = (unsigned short)_short;
			AddToSend(&realShort, 2);
		}
	}
}
void Extension::AddInt(int _int)
{
	AddToSend(&_int, 4);
}
void Extension::AddFloat(float _float)
{
	AddToSend(&_float, 4);
}
void Extension::AddStringWithoutNull(char * string)
{
	if (string)
		AddToSend(string, strlen(string));
	else
		CreateError("Adding string without null failed: pointer was null.");
}
void Extension::AddString(char * string)
{
	if (string)
		AddToSend(string, strlen(string) + 1);
	else
		CreateError("Adding string failed: pointer was null.");
}
void Extension::AddBinary(unsigned int address, int size)
{
	if (size < 0)
		CreateError("Add binary failed: Size less than 0.");
	else
	{
		if (size != 0)
			AddToSend((void *)(long)address, size);
		// else do nothing
	}
}
void Extension::AddFileToBinary(char * filename)
{
	if (filename[0] == '\0')
		CreateError("Cannot add file to send binary; filename is invalid.");
	else
	{
		// Open and deny other programs write priviledges
		FILE * file = _fsopen(filename, "rb", _SH_DENYWR);
		if (!file)
		{
			std::stringstream error;
			error << "Cannot add file to send binary, error number " << errno
				<< " occured with opening the file. The send binary has not been modified.";
			CreateError(error.str().c_str());
			return;
		}

		// Jump to end
		fseek(file, 0, SEEK_END);

		// Read current position as file size
		long filesize = ftell(file);

		// Go back to start
		fseek(file, 0, SEEK_SET);

		char * buffer = (char *)malloc(filesize);
		if (!buffer)
		{
			std::stringstream error;
			error << "Couldn't read file \"" << filename << "\" into binary to send; couldn't reserve enough memory "
				"to add file into message. The send binary has not been modified.";
			CreateError(error.str().c_str());
		}
		else
		{
			size_t amountRead;
			if ((amountRead = fread_s(buffer, filesize, 1U, filesize, file)) != filesize)
			{
				std::stringstream error;
				error << "Couldn't read file \"" << filename << "\" into binary to send, error number " << errno
					<< " occured with opening the file. The send binary has not been modified.";
				CreateError(error.str().c_str());
			}
			else
				AddToSend(buffer, amountRead);

			free(buffer);
		}
		fclose(file);
	}
}
void Extension::ResizeBinaryToSend(int newSize)
{
	if (newSize < 0)
		CreateError("Cannot change size of binary to send: new size is under 0 bytes.");
	else
	{
		char * NewMsg = (char *)realloc(SendMsg, newSize);
		if (!NewMsg)
		{
			return CreateError("Cannot change size of binary to send: reallocation of memory failed.\r\n"
				"Size has not been modified.");
		}
		// Clear new bytes to 0
		memset(NewMsg + SendMsgSize, 0, newSize - SendMsgSize);

		SendMsg = NewMsg;
		SendMsgSize = newSize;
	}
}
void Extension::CompressSendBinary()
{
	if (SendMsgSize <= 0)
		CreateError("Cannot compress send binary; Message is too small.");
	else
	{
		z_stream strm = { 0 };
		int ret = deflateInit(&strm, 9); // 9 = best compression ratio
		if (ret)
		{
			std::stringstream error;
			error << "Error " << ret << " occured with initiating compression.";
			CreateError(error.str().c_str());
			return;
		}

		unsigned char * output_buffer = (unsigned char *)malloc(SendMsgSize + 256U);
		if (!output_buffer)
		{
			std::stringstream error;
			error << "Error with compressing send binary, could not allocate enough memory. Desired " << (SendMsgSize + 256U) << " bytes.";
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
	if (threadData.receivedMsg.size <= 0)
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
		strm.next_in = (unsigned char *)threadData.receivedMsg.content;
		strm.avail_in = threadData.receivedMsg.size;
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
		char * ThisMsg = threadData.receivedMsg.content;
		free(threadData.receivedMsg.content);

		for (std::vector<SaveExtInfo *>::iterator i = Saved.begin(); i != Saved.end(); ++i)
		{
			if ((char *)output_buffer == (*i)->receivedMsg.content)
				continue;
			(*i)->receivedMsg.content = (char *)output_buffer;
			(*i)->receivedMsg.cursor = 0;
			(*i)->receivedMsg.size = _msize(output_buffer);
		}
	}
}
void Extension::MoveReceivedBinaryCursor(int position)
{
	if (position < 0)
		CreateError("Cannot move cursor; Position less than 0.");
	else if (threadData.receivedMsg.size - position <= 0)
		CreateError("Cannot move cursor; Message is too small.");
	else
		threadData.receivedMsg.cursor = position;
}
void Extension::SaveReceivedBinaryToFile(int position, int size, char * filename)
{
	if (position < 0)
		CreateError("Cannot save received binary; Position less than 0.");
	else if (size <= 0)
		CreateError("Cannot save received binary; Size equal or less than 0.");
	else if (filename[0] == '\0')
		CreateError("Cannot save received binary; filename is invalid.");
	else if (((unsigned int)position) + size > threadData.receivedMsg.size)
	{
		std::stringstream error;
		error << "Cannot save received binary to file; message doesn't have " << size
			<< " bytes at position " << position << " onwards.";
		CreateError(error.str().c_str());
		return;
	}
	else
	{
		FILE * File = _fsopen(filename, "wb", SH_DENYWR);
		if (!File)
		{
			std::stringstream error;
			error << "Cannot save received binary to file, error number " << errno
				<< " occured with opening the file. The message has not been modified.";
			CreateError(error.str().c_str());
			return;
		}

		size_t amountWritten;
		if ((amountWritten = fwrite(threadData.receivedMsg.content + position, 1, size, File)) != size)
		{
			std::stringstream error;
			error << "Cannot save received binary to file, error number " << errno
				<< " occured with writing the file. Wrote " << amountWritten
				<< " bytes total. The message has not been modified.";
			CreateError(error.str().c_str());
		}

		fclose(File);
	}
}
void Extension::AppendReceivedBinaryToFile(int position, int size, char * filename)
{
	if (position < 0)
		CreateError("Cannot append received binary to file; position less than 0.");
	else if (size <= 0)
		CreateError("Cannot append received binary to file; size equal or less than 0.");
	else if (filename[0] == '\0')
		CreateError("Cannot append received binary to file; filename is invalid.");
	else if (((unsigned int)position) + size > threadData.receivedMsg.size)
	{
		std::stringstream error;
		error << "Cannot append received binary to file; message doesn't have " << size
			<< " bytes at position " << position << " onwards.";
		CreateError(error.str().c_str());
		return;
	}
	else
	{
		// Open while denying write of other programs
		FILE * File = _fsopen(filename, "ab", SH_DENYWR);
		if (!File)
		{
			std::stringstream error;
			error << "Cannot append received binary to file, error number " << errno
				<< " occured with opening the file. The binary message has not been modified.";
			CreateError(error.str().c_str());
			return;
		}

		size_t amountWritten;
		if ((amountWritten = fwrite(threadData.receivedMsg.content + position, 1U, size, File)) != size)
		{
			fseek(File, 0, SEEK_END);
			long long filesize = _ftelli64(File);

			std::stringstream error;
			error << "Cannot append received binary to file, error number " << errno
				<< " occured with writing the file. Wrote " << amountWritten
				<< " bytes, leaving file at size " << filesize << " total. The binary message has not been modified.";
			CreateError(error.str().c_str());
		}

		fclose(File);
	}
}
