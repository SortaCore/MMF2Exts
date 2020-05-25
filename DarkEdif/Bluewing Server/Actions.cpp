
#include "Common.h"
#ifdef _DEBUG
std::stringstream CriticalSection;
#endif

#define Remake(name) MessageBoxA(NULL, "Your "#name" actions need to be recreated.\n" \
										"This is probably due to parameter changes.", "Lacewing Blue Server", MB_OK)
#define Saved (globals->_saved)

static char errtext[1024];
void ErrNoToErrText()
{
	int error = errno; // strerror_s may change errno
	if (strerror_s(errtext, error))
	{
		strcpy_s(errtext, "errno failed to convert");
		_set_errno(error);
	}
}


void Extension::RemovedActionNoParams()
{
	CreateError("Action needs removing.");
}
void Extension::RelayServer_Host(int port)
{
	if (Srv.hosting())
		return CreateError("Cannot start hosting: already hosting a server.");

	Srv.host(port);
}
void Extension::RelayServer_StopHosting()
{
	Srv.unhost();
}
void Extension::FlashServer_Host(char * path)
{
	if (FlashSrv->hosting())
		return CreateError("Cannot start hosting flash policy: already hosting a flash policy.");

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


static AutoResponse ConvToAutoResponse(int informFusion, int immediateRespondWith,
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
			denyReason = "";

		AutoResponse autoResponse = AutoResponse::Invalid;
		if (informFusion == 1)
		{
			if (immediateRespondWith == 0)
				autoResponse = AutoResponse::Approve_TellFusion;
			else if (immediateRespondWith == 1)
				autoResponse = AutoResponse::Deny_TellFusion;
			else /* immediateRespondWith == 2 */
				autoResponse = AutoResponse::WaitForFusion;
		}
		else /* informFusion == 0 */
		{
			if (immediateRespondWith == 0)
				autoResponse = AutoResponse::Approve_Quiet;
			else /* if (immediateRespondWith == 1) */
				autoResponse = AutoResponse::Deny_Quiet;
			/* immediateRespondWith == 2 is invalid with informFusion = 0 */;
		}

		return autoResponse;
	}

	globals->CreateError(err);
	return AutoResponse::Invalid;
}


void Extension::EnableCondition_OnConnectRequest(int informFusion, int immediateRespondWith, char * autoDenyReason)
{
	AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		autoDenyReason, globals, "on connect request");
	if (resp == AutoResponse::Invalid)
		return;
	globals->autoResponse_Connect = resp;
	globals->autoResponse_Connect_DenyReason = autoDenyReason;
	Srv.onconnect(::OnClientConnectRequest);
}
void Extension::EnableCondition_OnNameSetRequest(int informFusion, int immediateRespondWith, char * autoDenyReason)
{
	AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		autoDenyReason, globals, "on name set request");
	if (resp == AutoResponse::Invalid)
		return;
	globals->autoResponse_NameSet = resp;
	globals->autoResponse_NameSet_DenyReason = autoDenyReason;
	Srv.onnameset(resp != AutoResponse::Approve_Quiet ? ::OnNameSetRequest : nullptr);
}
void Extension::EnableCondition_OnJoinChannelRequest(int informFusion, int immediateRespondWith, char * autoDenyReason)
{
	AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		autoDenyReason, globals, "on join channel request");
	if (resp == AutoResponse::Invalid)
		return;
	globals->autoResponse_ChannelJoin = resp;
	globals->autoResponse_ChannelJoin_DenyReason = autoDenyReason;
	Srv.onchannel_join(resp != AutoResponse::Approve_Quiet ? ::OnJoinChannelRequest : nullptr);
}
void Extension::EnableCondition_OnLeaveChannelRequest(int informFusion, int immediateRespondWith, char * autoDenyReason)
{
	AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		autoDenyReason, globals, "on join channel request");
	if (resp == AutoResponse::Invalid)
		return;
	globals->autoResponse_ChannelLeave = resp;
	globals->autoResponse_ChannelLeave_DenyReason = autoDenyReason;
	// If local data for channel is used at all, we don't want it dangling, so make sure OnLeave is always ran.
	Srv.onchannel_leave(::OnLeaveChannelRequest);
}
void Extension::EnableCondition_OnMessageToPeer(int informFusion, int immediateRespondWith)
{
	char * dummyDenyReason = "X";
	AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		dummyDenyReason, globals, "on join channel request");
	if (resp == AutoResponse::Invalid)
		return;
	globals->autoResponse_MessageClient = resp;
	Srv.onmessage_peer(resp != AutoResponse::Approve_Quiet ? ::OnPeerMessage : nullptr);
}
void Extension::EnableCondition_OnMessageToChannel(int informFusion, int immediateRespondWith)
{
	char * dummyDenyReason = "X";
	AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		dummyDenyReason, globals, "on join channel request");
	if (resp == AutoResponse::Invalid)
		return;
	globals->autoResponse_MessageChannel = resp;
	Srv.onmessage_channel(resp != AutoResponse::Approve_Quiet ? ::OnChannelMessage : nullptr);
}
void Extension::EnableCondition_OnMessageToServer(int informFusion)
{
	if (informFusion < 0 || informFusion > 1)
		return globals->CreateError("Invalid \"Inform Fusion\" parameter passed to \"enable/disable condition: on message to server\".");

	// This one's handled a bit differently; there is no auto approve/deny.
	// The message is either read by Fusion or discarded immediately.
	globals->autoResponse_MessageServer = informFusion == 1 ? AutoResponse::WaitForFusion : AutoResponse::Deny_Quiet;
	Srv.onmessage_server(informFusion == 1 ? ::OnServerMessage : nullptr);
}
void Extension::OnInteractive_Deny(char * reason)
{
	if (InteractivePending == InteractiveType::None)
		return CreateError("Cannot deny client's action: No interactive action is pending.");
	// All of the interactive events currently allow denying
	//else if ((InteractivePending & InteractiveType::DenyPermitted) != InteractiveType::DenyPermitted)
	//	return CreateError("Cannot deny client's action: Interactive event is not compatible with this action.");
	if (!DenyReason.empty())
		return CreateError("Can't deny client's action: Set to auto-deny, or Deny was called more than once. Ignoring additional denies.");

	DenyReason = reason[0] ? reason : "No reason specified.";
}
void Extension::OnInteractive_ChangeClientName(char * newName)
{
	if (newName[0] == '\0')
		return CreateError("Cannot change new client name: Cannot use a blank name.");
	if (strnlen(newName, 256) == 256)
		return CreateError("Cannot change new client name: Cannot use a name longer than 255 characters.");
	if (InteractivePending == InteractiveType::None)
		return CreateError("Cannot change new client name: No interactive action is pending.");
	if (InteractivePending != InteractiveType::ClientNameSet)
		return CreateError("Cannot change new client name: Interactive event is not compatible with this action.");
	if (!DenyReason.empty())
		return CreateError("Cannot change new client name: Name change has already been denied by the Deny Request action.");
	if (!strcmp(NewClientName.c_str(), newName))
		return CreateError("Cannot change new channel name: New name is same as original name.");

	NewClientName = newName;
}
void Extension::OnInteractive_ChangeChannelName(char * newName)
{
	if (newName[0] == '\0')
		return CreateError("Cannot change joining channel name: Cannot use a blank name.");
	if (strnlen(newName, 256U) == 256U)
		return CreateError("Cannot change joining channel name: Cannot use a name longer than 255 characters.");
	if (InteractivePending == InteractiveType::None)
		return CreateError("Cannot change joining channel name: No interactive action is pending.");
	if ((InteractivePending & InteractiveType::ChannelJoin) != InteractiveType::ChannelJoin)
		return CreateError("Cannot change joining channel name: Interactive event is not compatible with this action.");
	if (!DenyReason.empty())
		return CreateError("Cannot change joining channel name: Channel name join has already been denied by the Deny Request action.");
	if (!strcmp(NewChannelName.c_str(), newName))
		return CreateError("Cannot change new channel name: New name is same as original name.");

	NewChannelName = newName;
}
void Extension::OnInteractive_DropMessage()
{
	if (InteractivePending == InteractiveType::None)
		return CreateError("Cannot deny the action: No interactive action is pending.");
	if (InteractivePending != InteractiveType::ChannelMessageIntercept &&
		InteractivePending != InteractiveType::ClientMessageIntercept)
	{
		return CreateError("Cannot drop message: Interactive event is not compatible with this action.");
	}
	if (DropMessage)
		return CreateError("Error dropping message: Message already being dropped.");
	DropMessage = true;
}
void Extension::OnInteractive_ReplaceMessageWithText(char * NewText)
{
	if (InteractivePending == InteractiveType::None)
		return CreateError("Cannot deny the action: No interactive action is pending.");
	if (InteractivePending != InteractiveType::ChannelMessageIntercept &&
		InteractivePending != InteractiveType::ClientMessageIntercept)
	{
		return CreateError("Cannot replace message: Interactive event is not compatible with this action.");
	}
	if (DropMessage)
		return CreateError("Cannot replace message: Message was dropped.");

	// See the Decompress Received Binary for implementation.
	return CreateError("Cannot replace message: Replacing messages not implemented.");
}
void Extension::OnInteractive_ReplaceMessageWithNumber(int newNumber)
{
	if (InteractivePending == InteractiveType::None)
		return CreateError("Cannot deny the action: No interactive action is pending.");
	if (InteractivePending != InteractiveType::ChannelMessageIntercept &&
		InteractivePending != InteractiveType::ClientMessageIntercept)
	{
		return CreateError("Cannot replace message: Interactive event is not compatible with this action.");
	}
	if (DropMessage)
		return CreateError("Cannot replace message: Message was dropped.");

	// See the Decompress Received Binary for implementation.
	return CreateError("Cannot replace message: Replacing messages not implemented.");
}
void Extension::OnInteractive_ReplaceMessageWithSendBinary()
{
	if (InteractivePending == InteractiveType::None)
		return CreateError("Cannot deny the action: No interactive action is pending.");
	if (InteractivePending != InteractiveType::ChannelMessageIntercept &&
		InteractivePending != InteractiveType::ClientMessageIntercept)
	{
		return CreateError("Cannot replace message: Interactive event is not compatible with this action.");
	}
	if (DropMessage)
		return CreateError("Cannot replace message: Message was dropped.");

	// See the Decompress Received Binary for implementation.
	CreateError("Cannot replace message: Replacing messages not implemented.");
}

void Extension::Channel_SelectByName(char * channelNamePtr)
{
	if (channelNamePtr[0] == '\0')
		return CreateError("Channel_SelectByName() was called with a blank name.");
	size_t nameLen = strnlen(channelNamePtr, 256U);
	if (nameLen == 256U)
		return CreateError("Channel_SelectByName() was called with a name exceeding the max length of 255 characters.");

	std::string_view channelName(channelNamePtr, nameLen);

	selChannel = nullptr;

	{
		lacewing::readlock serverReadLock = Srv.lock.createReadLock();
		const auto& channels = Srv.getchannels();
		for (const auto &ch : channels)
		{
			if (lw_sv_icmp(ch->name(), channelName))
			{
				selChannel = ch;

				if (selClient == nullptr)
					return;

				// If selected client is on new channel, keep it selected, otherwise deselect client
				serverReadLock.lw_unlock();

				auto channelReadLock = ch->lock.createReadLock();
				const auto &clientsOnChannel = ch->getclients();
				if (std::find(clientsOnChannel.cbegin(), clientsOnChannel.cend(), selClient) == clientsOnChannel.cend())
					selClient = nullptr;
				return;
			}
		}
	}

	CreateError("Selecting channel by name failed: Channel with name %s not found on server.", channelNamePtr);
}
void Extension::Channel_Close()
{
	if (!selChannel)
		return CreateError("Could not close channel: No channel selected.");
	if (selChannel->readonly())
		return CreateError("Could not close channel: Already closing.");

	selChannel->close();
}
void Extension::Channel_SelectMaster()
{
	if (!selChannel)
		return CreateError("Could not select channel master: No channel selected.");

	selClient = selChannel->channelmaster();
}
void Extension::Channel_LoopClients()
{
	if (!selChannel)
		return CreateError("Loop Clients On Channel was called without a channel being selected.");

	auto origSelChannel = selChannel;
	auto origSelClient = selClient;
	auto origLoopName = loopName;

	std::vector<decltype(origSelClient)> channelClientListDup;
	{
		auto channelReadLock = origSelChannel->lock.createReadLock();
		channelClientListDup = origSelChannel->getclients();
	}

	for (const auto &cli : channelClientListDup)
	{
		selChannel = origSelChannel;
		selClient = cli;
		loopName = std::string_view();
		Runtime.GenerateEvent(8);
	}
	selChannel = origSelChannel;
	selClient = origSelClient;
	loopName = std::string_view();

	Runtime.GenerateEvent(44);
	loopName = origLoopName;
}
void Extension::Channel_LoopClientsWithName(char * passedLoopName)
{
	if (!selChannel)
		return CreateError("Loop Clients On Channel With Name was called without a channel being selected.");

	// You can loop a closed channel's clients, but it's read-only.
	auto origSelChannel = selChannel;
	auto origSelClient = selClient;
	auto origLoopName = loopName;

	std::string_view loopNameDup(passedLoopName);
	std::vector<decltype(origSelClient)> channelClientListDup;
	{
		auto channelReadLock = origSelChannel->lock.createReadLock();
		channelClientListDup = origSelChannel->getclients();
	}

	for (const auto &cli : channelClientListDup)
	{
		selChannel = origSelChannel;
		selClient = cli;
		loopName = loopNameDup;
		Runtime.GenerateEvent(39);
	}
	selChannel = origSelChannel;
	selClient = origSelClient;
	loopName = loopNameDup;

	Runtime.GenerateEvent(40);
	loopName = origLoopName;
}
void Extension::Channel_SetLocalData(char * key, char * value)
{
	if (!selChannel)
		return CreateError("Could not set channel local data: No channel selected.");
	// if (selChannel->readonly())
	//	return CreateError("Could not set channel local data: Channel is read-only.");

	globals->SetLocalData(selChannel, key, value);
}
void Extension::LoopAllChannels()
{
	auto origSelChannel = selChannel;
	auto origSelClient = selClient;
	auto origLoopName = loopName;

	std::vector<decltype(origSelChannel)> serverChannelListDup;
	{
		auto serverReadLock = Srv.lock.createReadLock();
		serverChannelListDup = Srv.getchannels();
	}

	for (const auto& ch : serverChannelListDup)
	{
		selChannel = ch;
		selClient = nullptr;
		loopName = std::string_view();
		Runtime.GenerateEvent(5);
	}
	selChannel = origSelChannel;
	selClient = origSelClient;
	loopName = std::string_view();

	Runtime.GenerateEvent(45);
	loopName = origLoopName;
}
void Extension::LoopAllChannelsWithName(char * passedLoopName)
{
	auto origSelChannel = selChannel;
	auto origSelClient = selClient;
	auto origLoopName = loopName;

	std::string_view loopNameDup(passedLoopName);
	std::vector<decltype(origSelChannel)> serverChannelListDup;
	{
		auto serverReadLock = Srv.lock.createReadLock();
		serverChannelListDup = Srv.getchannels();
	}

	for (const auto& ch : serverChannelListDup)
	{
		selChannel = ch;
		selClient = nullptr;
		loopName = loopNameDup;
		Runtime.GenerateEvent(36);
	}
	selChannel = origSelChannel;
	selClient = origSelClient;
	loopName = loopNameDup;

	Runtime.GenerateEvent(41);
	loopName = origLoopName;
}
void Extension::Client_Disconnect()
{
	if (!selClient)
		return CreateError("Could not disconnect client: No client selected.");

	if (!selClient->readonly())
		selClient->disconnect();
}
void Extension::Client_SetLocalData(char * key, char * value)
{
	if (!selClient)
		return CreateError("Could not set client local data: No client selected.");
	// if (selClient->readonly())
	//	return CreateError("Could not set client local data: Client is read-only.");

	globals->SetLocalData(selClient, key, value);
}
void Extension::Client_LoopJoinedChannels()
{
	if (!selClient)
		return CreateError("Cannot loop client's joined channels: No client selected.");

	auto origSelClient = selClient;
	auto origSelChannel = selChannel;
	auto origLoopName = loopName;
	std::vector<decltype(selChannel)> joinedChannelListDup;
	{
		auto selClientReadLock = origSelClient->lock.createReadLock();
		joinedChannelListDup = origSelClient->getchannels();
	}

	for (const auto &joinedCh : joinedChannelListDup)
	{
		selChannel = joinedCh;
		selClient = origSelClient;
		loopName = std::string_view();
		Runtime.GenerateEvent(6);
	}

	selChannel = origSelChannel;
	selClient = origSelClient;
	loopName = std::string_view();

	Runtime.GenerateEvent(47);
	loopName = origLoopName;
}
void Extension::Client_LoopJoinedChannelsWithName(char * passedLoopName)
{
	if (!selClient)
		return CreateError("Cannot loop client's joined channels: No client selected.");

	auto origSelClient = selClient;
	auto origSelChannel = selChannel;
	auto origLoopName = loopName;
	std::string_view loopNameDup(passedLoopName);
	std::vector<decltype(selChannel)> joinedChannelListDup;
	{
		auto selClientReadLock = origSelClient->lock.createReadLock();
		joinedChannelListDup = origSelClient->getchannels();
	}

	for (const auto &joinedCh : joinedChannelListDup)
	{
		selChannel = joinedCh;
		selClient = origSelClient;
		loopName = loopNameDup;
		Runtime.GenerateEvent(37);
	}

	selChannel = origSelChannel;
	selClient = origSelClient;
	loopName = loopNameDup;

	Runtime.GenerateEvent(43);
	loopName = origLoopName;
}
void Extension::Client_SelectByName(char * clientName)
{
	if (clientName[0] == '\0')
		return CreateError("Select Client By Name was called with a blank name.");

	selClient = nullptr;
	{
		auto serverReadLock = Srv.lock.createReadLock();
		const auto &clients = Srv.getclients();
		auto foundCliIt =
			std::find_if(clients.cbegin(), clients.cend(),
				[&](const auto &cli) { return lw_sv_icmp(cli->name(), clientName); });
		if (foundCliIt == clients.cend())
			return CreateError("Client with name %s not found on server.", clientName);
		selClient = *foundCliIt;
	}

	// If client is joined to originally selected channel, then keep that channel selected
	if (!selChannel)
		return;

	auto cliReadLock = selClient->lock.createReadLock();
	const auto & cliJoinChs = selClient->getchannels();
	if (std::find(cliJoinChs.cbegin(), cliJoinChs.cend(), selChannel) == cliJoinChs.cend())
		selChannel = nullptr;
}
void Extension::Client_SelectByID(int clientID)
{
	if (clientID < 0 || clientID > 0xFFFF)
		return CreateError("Could not select client on channel, ID is below 0 or greater than 65535.");

	selClient = nullptr;
	{
		auto serverReadLock = Srv.lock.createReadLock();
		const auto &clients = Srv.getclients();
		auto foundCliIt =
			std::find_if(clients.cbegin(), clients.cend(),
				[=](const auto & cli) { return cli->id() == clientID; });
		if (foundCliIt == clients.cend())
			return CreateError("Client with ID %i not found on server.", clientID);
		selClient = *foundCliIt;
	}

	// If client is joined to originally selected channel, then keep that channel selected with new client
	if (!selChannel)
		return;

	auto cliReadLock = selClient->lock.createReadLock();
	const auto& cliJoinChs = selClient->getchannels();
	if (std::find(cliJoinChs.cbegin(), cliJoinChs.cend(), selChannel) == cliJoinChs.cend())
		selChannel = nullptr;
}
void Extension::Client_SelectSender()
{
	if (!threadData->senderClient)
		return CreateError("Cannot select sending client: No sending client variable available.");

	selClient = threadData->senderClient;
}
void Extension::Client_SelectReceiver()
{
	if (!threadData->receivingClient)
		return CreateError("Cannot select receiving client: No receiving client variable available.");

	selClient = threadData->receivingClient;
}
void Extension::LoopAllClients()
{
	auto origSelClient = selClient;
	auto origSelChannel = selChannel;
	auto origLoopName = loopName;

	// Let user write to clients if necessary by duping
	std::vector<decltype(selClient)> clientListDup;
	{
		auto serverReadLock = Srv.lock.createReadLock();
		clientListDup = Srv.getclients();
	}

	for (const auto & selectedClient : clientListDup)
	{
		selChannel = nullptr;
		selClient = selectedClient;
		loopName = std::string_view();
		Runtime.GenerateEvent(7);
	}

	selChannel = origSelChannel;
	selClient = origSelClient;
	loopName = std::string_view();
	Runtime.GenerateEvent(46);
	loopName = origLoopName;
}
void Extension::LoopAllClientsWithName(char * passedLoopName)
{
	auto origSelClient = selClient;
	auto origSelChannel = selChannel;
	auto origLoopName = loopName;
	std::string_view loopNameDup(passedLoopName);
	loopName = loopNameDup;

	// Let user write to clients if necessary by duping
	std::vector<decltype(selClient)> clientListDup;
	{
		auto serverReadLock = Srv.lock.createReadLock();
		clientListDup = Srv.getclients();
	}

	for (const auto & selectedClient : clientListDup)
	{
		selChannel = nullptr;
		selClient = selectedClient;
		loopName = loopNameDup;
		Runtime.GenerateEvent(38);
	}

	selChannel = origSelChannel;
	selClient = origSelClient;
	loopName = loopNameDup;
	Runtime.GenerateEvent(42);

	loopName = origLoopName;
}
void Extension::SendTextToChannel(int subchannel, char * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		return CreateError("Send Text to Channel was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	if (!selChannel)
		return CreateError("Send Text to Channel was called without a channel being selected.");
	if (selChannel->readonly())
		return CreateError("Send Text to Channel was called with a read-only channel, name %s.", selChannel->name().c_str());

	selChannel->send(subchannel, std::string_view(textToSend, strlen(textToSend) + 1U), 0);
}
void Extension::SendTextToClient(int subchannel, char * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		return CreateError("Send Text to Client was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	if (!selClient)
		return CreateError("Send Text to Client was called without a client being selected.");
	if (selClient->readonly())
		return CreateError("Send Text to Client was called with a read-only client ID %hu, name %s.", selClient->id(), selClient->name().c_str());

	selClient->send(subchannel, std::string_view(textToSend, strlen(textToSend) + 1U), 0);
}
void Extension::SendNumberToChannel(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		return CreateError("Send Number to Channel was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	if (!selChannel)
		return CreateError("Send Number to Channel was called without a channel being selected.");
	if (selChannel->readonly())
		return CreateError("Send Number to Channel was called with a read-only channel, name %s.", selChannel->name().c_str());

	selChannel->send(subchannel, std::string_view((char *)&numToSend, sizeof(int)), 1);
}
void Extension::SendNumberToClient(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		return CreateError("Send Number to Client was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	if (!selClient)
		return CreateError("Send Number to Client was called without a client being selected.");
	if (selClient->readonly())
		return CreateError("Send Number to Client was called with a read-only client ID %hu, name %s.", selClient->id(), selClient->name().c_str());

	selClient->send(subchannel, std::string_view((char *)&numToSend, sizeof(int)), 1);
}
void Extension::SendBinaryToChannel(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Send Binary to Channel was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selChannel)
		CreateError("Send Binary to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Send Binary to Channel was called with a read-only channel, name %s.", selChannel->name().c_str());
	else
		selChannel->send(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::SendBinaryToClient(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Send Binary to Client was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selClient)
		CreateError("Send Binary to Client was called without a client being selected.");
	else if (selClient->readonly())
		CreateError("Send Binary to Client was called with a read-only client: ID %hu, name %s.", selClient->id(), selClient->name().c_str());
	else
		selClient->send(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastTextToChannel(int subchannel, char * textToBlast)
{
	if (subchannel > 255 || subchannel < 0)
		return CreateError("Blast Text to Channel was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	if (!selChannel)
		return CreateError("Blast Text to Channel was called without a channel being selected.");
	if (selChannel->readonly())
		return CreateError("Blast Text to Channel was called with a read-only channel, name %s.", selChannel->name().c_str());

	selChannel->blast(subchannel, std::string_view(textToBlast, strlen(textToBlast) + 1U), 0);
}
void Extension::BlastTextToClient(int subchannel, char * textToBlast)
{
	if (subchannel > 255 || subchannel < 0)
		return CreateError("Blast Text to Client was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	if (!selClient)
		return CreateError("Blast Text to Client was called without a client being selected.");
	if (selClient->readonly())
		return CreateError("Blast Text to Client was called with a read-only client: ID %hu, name %s.", selClient->id(), selClient->name().c_str());

	selClient->blast(subchannel, std::string_view(textToBlast, strlen(textToBlast) + 1U), 0);
}
void Extension::BlastNumberToChannel(int subchannel, int numToBlast)
{
	if (subchannel > 255 || subchannel < 0)
		return CreateError("Blast Number to Channel was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	if (!selChannel)
		return CreateError("Blast Number to Channel was called without a channel being selected.");
	if (selChannel->readonly())
		return CreateError("Blast Number to Channel was called with a read-only channel, name %s.", selChannel->name().c_str());

	selChannel->blast(subchannel, std::string_view((char *)&numToBlast, 4U), 1);
}
void Extension::BlastNumberToClient(int subchannel, int numToBlast)
{
	if (subchannel > 255 || subchannel < 0)
		return CreateError("Blast Number to Client was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	if (!selClient)
		return CreateError("Blast Number to Client was called without a client being selected.");
	if (selClient->readonly())
		return CreateError("Blast Number to Client was called with a read-only client: ID %hu, name %s.", selClient->id(), selClient->name().c_str());

	selClient->blast(subchannel, std::string_view((char *)&numToBlast, 4U), 1);
}
void Extension::BlastBinaryToChannel(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Blast Binary to Channel was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selChannel)
		CreateError("Blast Binary to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Blast Binary to Channel was called with a read-only channel, name %s.", selChannel->name().c_str());
	else
		selChannel->blast(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastBinaryToClient(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Blast Binary to Client was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selClient)
		CreateError("Blast Binary to Client was called without a client being selected.");
	else if (selClient->readonly())
		CreateError("Blast Binary to Client was called with a read-only client: ID %hu, name %s.", selClient->id(), selClient->name().c_str());
	else
		selClient->blast(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::AddByteText(char * byte)
{
	if (!byte)
		return CreateError("Adding byte to binary failed: pointer was null.");
	if (strnlen(byte, 2) != 1)
		return CreateError("Adding byte to binary failed: text supplied \"%s\" was not a single text character.", byte);

	AddToSend(byte, 1);
}
void Extension::AddByteInt(int byte)
{
	if (byte > MAXUINT8 || byte < MININT8)
	{
		return CreateError("Adding byte to binary (as int) failed: the supplied number %i will not fit in range "
			"%i to %i (signed byte) or range 0 to %i (unsigned byte).", byte, MININT8, MAXINT8, MAXUINT8);
	}

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
void Extension::AddShort(int _short)
{
	if (_short > MAXUINT16 || _short < MININT16)
	{
		return CreateError("Adding short to binary failed: the supplied number %i will not fit in range "
			"%i to %i (signed short) or range 0 to %i (unsigned short).", _short, MININT16, MAXINT16, MAXUINT16);
	}

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
	if (!string)
		return CreateError("Adding string without null failed: pointer was null.");

	AddToSend(string, strlen(string));
}
void Extension::AddString(char * string)
{
	if (!string)
		return CreateError("Adding string failed: pointer was null.");

	AddToSend(string, strlen(string) + 1);
}

void Extension::AddBinary(unsigned int address, int size)
{
	if (size < 0)
		return CreateError("Add binary failed: Size less than 0.");

	if (size != 0)
		AddToSend((void *)(long)address, size);
	// else do nothing
}
void Extension::AddFileToBinary(char * filename)
{
	if (filename[0] == '\0')
		return CreateError("Cannot add file to send binary; filename \"\" is invalid.");

	// Open and deny other programs write privileges
	FILE * file = _fsopen(filename, "rb", _SH_DENYWR);
	if (!file)
	{
		ErrNoToErrText();
		return CreateError("Cannot add file \"%s\" to send binary, error %i (%s) occurred with opening the file."
			" The send binary has not been modified.", filename, errno, errtext);
	}

	// Jump to end
	fseek(file, 0, SEEK_END);

	// Read current position as file size
	long filesize = ftell(file);

	// Go back to start
	fseek(file, 0, SEEK_SET);

	std::unique_ptr<char[]> buffer = std::make_unique<char[]>(filesize);
	size_t amountRead;
	if ((amountRead = fread_s(buffer.get(), filesize, 1U, filesize, file)) != filesize)
	{
		CreateError("Couldn't read file \"%s\" into binary to send; couldn't reserve enough memory "
			"to add file into message. The send binary has not been modified.", filename);
	}
	else
		AddToSend(buffer.get(), amountRead);

	fclose(file);
}
void Extension::ResizeBinaryToSend(int newSize)
{
	if (newSize < 0)
		return CreateError("Cannot resize binary to send: new size %u bytes is negative.", newSize);

	char * NewMsg = (char *)realloc(SendMsg, newSize);
	if (!NewMsg)
	{
		return CreateError("Cannot reisze of binary to send: reallocation of memory into %u bytes failed.\r\n"
			"Binary to send has not been modified.", newSize);
	}
	// Clear new bytes to 0
	memset(NewMsg + SendMsgSize, 0, newSize - SendMsgSize);

	SendMsg = NewMsg;
	SendMsgSize = newSize;
}
void Extension::CompressSendBinary()
{
	if (SendMsgSize <= 0)
		return CreateError("Cannot compress send binary; binary is empty.");

	z_stream strm = {};
	int ret = deflateInit(&strm, 9); // 9 is maximum compression level
	if (ret)
		return CreateError("Compressing send binary failed, zlib error %i \"%s\" occurred with initiating compression.", ret, (strm.msg ? strm.msg : ""));

	// 4: precursor lw_ui32 with uncompressed size, required by Relay
	// 256: if compression results in larger message, it shouldn't be *that* much larger.

	unsigned char * output_buffer = (unsigned char *)malloc(4 + SendMsgSize + 256);
	if (!output_buffer)
	{
		CreateError("Compressing send binary failed, couldn't allocate enough memory. Desired %u bytes.",
			(size_t)4 + SendMsgSize + 256);
		deflateEnd(&strm);
		return;
	}

	// Store size as precursor - required by Relay
	*(lw_ui32 *)output_buffer = SendMsgSize;

	strm.next_in = (unsigned char *)SendMsg;
	strm.avail_in = SendMsgSize;

	// Allocate memory for compression
	strm.avail_out = SendMsgSize - 4;
	strm.next_out = output_buffer + 4;

	ret = deflate(&strm, Z_FINISH);
	if (ret != Z_STREAM_END)
	{
		free(output_buffer);
		CreateError("Compressing send binary failed, zlib compression call returned error %u \"%s\".",
			ret, (strm.msg ? strm.msg : ""));
		deflateEnd(&strm);
		return;
	}

	deflateEnd(&strm);

	char * output_bufferResize = (char *)realloc(output_buffer, 4 + strm.total_out);
	if (!output_bufferResize)
		return CreateError("Compressing send binary failed, reallocating memory to remove excess space after compression failed.");

	free(SendMsg);

	SendMsg = (char *)output_bufferResize;
	SendMsgSize = 4 + strm.total_out;
}
void Extension::ClearBinaryToSend()
{
	free(SendMsg);
	SendMsg = NULL;
	SendMsgSize = 0;
}
void Extension::DecompressReceivedBinary()
{
	if (threadData->receivedMsg.content.size() <= 4)
	{
		return CreateError("Cannot decompress received binary; message is %u bytes and too small to be a valid compressed message.",
			threadData->receivedMsg.content.size());
	}

	z_stream strm = { };
	int ret = inflateInit(&strm);
	if (ret)
	{
		return CreateError("Compressing send binary failed, zlib error %i \"%s\" occurred with initiating decompression.",
			ret, (strm.msg ? strm.msg : ""));
	}

	// Lacewing provides a precursor to the compressed data, with uncompressed size.
	lw_ui32 expectedUncompressedSize = *(lw_ui32 *)threadData->receivedMsg.content.data();
	std::string_view inputData(threadData->receivedMsg.content.data() + 4, threadData->receivedMsg.content.size() - 4);

	unsigned char * output_buffer = (unsigned char *)malloc(expectedUncompressedSize);
	if (!output_buffer)
	{
		inflateEnd(&strm);
		return CreateError("Decompression failed; couldn't allocate enough memory. Desired %u bytes.", expectedUncompressedSize);
	}

	strm.next_in = (unsigned char *)inputData.data();
	strm.avail_in = inputData.size();
	strm.avail_out = expectedUncompressedSize;
	strm.next_out = output_buffer;
	ret = inflate(&strm, Z_FINISH);
	if (ret < Z_OK)
	{
		free(output_buffer);
		CreateError("Decompression failed; zlib decompression call returned error %i \"%s\".",
			ret, (strm.msg ? strm.msg : ""));
		inflateEnd(&strm);
		return;
	}

	inflateEnd(&strm);

	// Update all extensions with new message content.
	threadData->receivedMsg.content.assign((char *)output_buffer, expectedUncompressedSize);
	threadData->receivedMsg.cursor = 0;

	EnterCriticalSectionDebug(&globals->lock);
	for (auto& i : Saved)
	{
		if (threadData == i)
			continue;
		(*i).receivedMsg.content.assign((char *)output_buffer, expectedUncompressedSize);
		(*i).receivedMsg.cursor = 0;
	}
	LeaveCriticalSectionDebug(&globals->lock);

	free(output_buffer); // .assign() copies the memory
}
void Extension::MoveReceivedBinaryCursor(int position)
{
	if (position < 0)
		return CreateError("Cannot move cursor; Position less than 0.");
	if (threadData->receivedMsg.content.size() - position <= 0)
	{
		return CreateError("Cannot move cursor to index %i; message is too small. Valid cursor index range is 0 to %i.",
			position, max(threadData->receivedMsg.content.size() - 1, 0));
	}

	threadData->receivedMsg.cursor = position;
}
void Extension::SaveReceivedBinaryToFile(int passedPosition, int passedSize, char * filename)
{
	if (passedPosition < 0)
		return CreateError("Cannot save received binary; position %i is less than 0.", passedPosition);
	if (passedSize <= 0)
		return CreateError("Cannot save received binary; size of %i is equal or less than 0.", passedSize);
	if (filename[0] == '\0')
		return CreateError("Cannot save received binary; filename \"\" is invalid.");

	size_t position = (size_t)passedPosition;
	size_t size = (size_t)passedSize;

	if (position + size > threadData->receivedMsg.content.size())
	{
		return CreateError("Cannot save received binary to file \"%s\"; message doesn't have %i"
			" bytes from position %u onwards, it only has %u bytes.",
			filename, size, position, threadData->receivedMsg.content.size() - position);
	}
	FILE * File = _fsopen(filename, "wb", SH_DENYWR);
	if (!File)
	{
		ErrNoToErrText();
		CreateError("Cannot save received binary to file \"%s\", error %i \"%s\""
			" occurred with opening the file.", filename, errno, errtext);
		return;
	}

	size_t amountWritten;
	if ((amountWritten = fwrite(threadData->receivedMsg.content.data() + position, 1, size, File)) != size)
	{
		ErrNoToErrText();
		CreateError("Cannot save received binary to file \"%s\", error %i \"%s\""
			" occurred with writing the file. Wrote %u bytes total.", filename, errno, errtext, amountWritten);
		fclose(File);
		return;
	}

	if (fclose(File))
	{
		ErrNoToErrText();
		CreateError("Cannot save received binary to file \"%s\", error %i \"%s\""
			" occurred with writing the end of the file. Wrote %u bytes total.", filename, errno, errtext, amountWritten);
	}
}
void Extension::AppendReceivedBinaryToFile(int passedPosition, int passedSize, char * filename)
{
	if (passedPosition < 0)
		return CreateError("Cannot append received binary; position %i is less than 0.", passedPosition);
	if (passedSize <= 0)
		return CreateError("Cannot append received binary; size of %i is equal or less than 0.", passedSize);
	if (filename[0] == '\0')
		return CreateError("Cannot append received binary; filename \"\" is invalid.");

	size_t position = (size_t)passedPosition;
	size_t size = (size_t)passedSize;
	if (position + size > threadData->receivedMsg.content.size())
	{
		return CreateError("Cannot append received binary to file \"%s\"; message doesn't have %i"
			" bytes from position %u onwards, it only has %u bytes.",
			filename, size, position, threadData->receivedMsg.content.size() - position);
	}
	FILE * File = _fsopen(filename, "ab", SH_DENYWR);
	if (!File)
	{
		ErrNoToErrText();
		CreateError("Cannot append received binary to file \"%s\", error %i \"%s\""
			" occurred with opening the file.", filename, errno, errtext);
		return;
	}

	size_t amountWritten;
	if ((amountWritten = fwrite(threadData->receivedMsg.content.data() + position, 1, size, File)) != size)
	{
		ErrNoToErrText();
		CreateError("Cannot append received binary to file \"%s\", error %i \"%s\""
			" occurred with writing the file. Wrote %u bytes total.", filename, errno, errtext, amountWritten);
		fclose(File);
		return;
	}

	if (fclose(File))
	{
		ErrNoToErrText();
		CreateError("Cannot append received binary to file \"%s\", error %i \"%s\""
			" occurred with writing the end of the file. Wrote %u bytes total.", filename, errno, errtext, amountWritten);
	}
}
