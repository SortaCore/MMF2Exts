#include "Common.hpp"

#define EventsToRun globals->_eventsToRun
#define Remake(name) DarkEdif::MsgBox::Error(_T("Remake action"), _T("Your "#name" actions need to be recreated.\r\n") \
						_T("This is probably due to parameter changes."))

static char errtext[1024];
static void ErrNoToErrText()
{
	int error = errno; // strerror_s may change errno
#ifdef _WIN32
	if (strerror_s(errtext, error))
	{
		strcpy_s(errtext, std::size(errtext), "errno failed to convert");
		errno = error;
	}
#else
	char * strError = strerror(error);
	if (strError == NULL)
	{
		strcpy(errtext, "errno failed to convert");
		errno = error;
	}
	else
		strcpy(errtext, strError);
#endif
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
	if (globals->unhostingInProgress)
		return CreateError("You're unhosting while unhosting. Don't, it messes up the cleanup. Second unhost cancelled.");
	globals->unhostingInProgress = true;
	Srv.unhost();
	globals->unhostingInProgress = false;
}
void Extension::FlashServer_Host(const TCHAR * path)
{
	if (FlashSrv->hosting())
		return CreateError("Cannot start hosting flash policy: already hosting a flash policy.");

	const std::tstring flashFileUnembedded = DarkEdif::MakePathUnembeddedIfNeeded(this, path);

	// Errors in DarkEdif::MakePathUnembeddedIfNeeded indicated by starting with '>' char
	if (flashFileUnembedded[0] == _T('>'))
		return CreateError("Cannot load flash policy file: %s.", DarkEdif::TStringToUTF8(flashFileUnembedded.substr(1)).c_str());

	// Pass through as UTF-8; it'll be converted back to wide by lacewing if Unicode Windows build
	FlashSrv->host(DarkEdif::TStringToUTF8(flashFileUnembedded).c_str());
}
void Extension::FlashServer_StopHosting()
{
	// Don't need to mess with globals->unhostingInProgress. No events will be triggered by leaving Flash clients,
	// as the Flash server hosts policy separately; the Flash clients connect to regular Relay server Srv, not FlashSrv
	FlashSrv->unhost();
}

void Extension::WebSocketServer_LoadHostCertificate_FromFile(const TCHAR* chainFile, const TCHAR* privKeyFile, const TCHAR* password)
{
	const std::tstring chainFileUnembedded = DarkEdif::MakePathUnembeddedIfNeeded(this, chainFile);

	// Errors in DarkEdif::MakePathUnembeddedIfNeeded indicated by starting with '>' char
	if (chainFileUnembedded[0] == _T('>'))
		return CreateError("Cannot load cert chain file: %s.", DarkEdif::TStringToUTF8(chainFileUnembedded.substr(1)).c_str());

	// Allow blank path for the second path param, in case first path's file has both cert and priv key within
	const std::tstring privKeyFileUnembedded = privKeyFile[0] == _T('\0') ? std::tstring() : DarkEdif::MakePathUnembeddedIfNeeded(this, privKeyFile);

	if (privKeyFileUnembedded[0] == _T('>'))
		return CreateError("Cannot load private key file: %s.", DarkEdif::TStringToUTF8(privKeyFileUnembedded.substr(1)).c_str());

	// Pass through as UTF-8; it'll be converted back to wide by lacewing if Unicode Windows build
	// this will return false if it fails - but we should get an error made anyway
	Srv.websocket->load_cert_file(
		DarkEdif::TStringToUTF8(chainFileUnembedded).c_str(),
		DarkEdif::TStringToUTF8(privKeyFileUnembedded).c_str(),
		DarkEdif::TStringToUTF8(password).c_str());
}
void Extension::WebSocketServer_LoadHostCertificate_FromSystemStore(const TCHAR* commonName, const TCHAR* location, const TCHAR* storeName)
{
#ifdef _WIN32
	// this will return false if it fails - but we should get an error made anyway
	Srv.websocket->load_sys_cert(DarkEdif::TStringToUTF8(commonName).c_str(), DarkEdif::TStringToUTF8(location).c_str(), DarkEdif::TStringToUTF8(storeName).c_str());
#else
	CreateError("Cannot load host certificate from system store: the system store is only available on Windows.");
#endif
}
void Extension::WebSocketServer_EnableHosting(int insecurePort, int securePort)
{
	if (insecurePort < -1 || insecurePort > UINT16_MAX)
		return CreateError("Cannot start WebSocket hosting: the passed insecure port %i is invalid.", insecurePort);
	if (securePort < -1 || securePort > UINT16_MAX)
		return CreateError("Cannot start WebSocket hosting: the passed secure port %i is invalid.", insecurePort);
	if (securePort == 0 && insecurePort == 0)
		return CreateError("Cannot start WebSocket hosting: you passed 0 for both insecure and secure ports; nothing to host.");
	if (securePort != 0 && !Srv.websocket->cert_loaded())
		return CreateError("Cannot start WebSocket hosting: Can't run secure server with no certificate loaded. See help file.");

	if (insecurePort != 0 && Srv.websocket->hosting())
		return CreateError("Cannot start WebSocket hosting: WebSocket Insecure server is already running on port %d.", Srv.websocket->port());
	if (securePort != 0 && Srv.websocket->hosting_secure())
		return CreateError("Cannot start WebSocket hosting: WebSocket Secure server is already running on port %d.", Srv.websocket->port_secure());

	Srv.host_websocket(insecurePort, securePort);
}
void Extension::WebSocketServer_DisableHosting(const TCHAR* whichParam)
{
	const std::string which = TStringToUTF8Simplified(whichParam);
	// i is converted to l as part of text simplifying, so it's actually lnsecure
	if (which == "both"sv || which == "lnsecure"sv || which == "secure"sv)
	{
		if (globals->unhostingInProgress)
			return CreateError("You're unhosting while unhosting. Don't, it messes up the cleanup. Second unhost cancelled.");
		globals->unhostingInProgress = true;
		Srv.unhost_websocket(
			which == "both"sv || which == "lnsecure"sv,
			which == "both"sv || which == "secure"sv);
		globals->unhostingInProgress = false;
	}
	else
		CreateError("Couldn't stop hosting WebSocket: server type \"%s\" unrecognised. Should be \"both\", \"insecure\" or \"secure\".", which.c_str());
}
void Extension::ChannelListing_Enable()
{
	Srv.setchannellisting(true);
}
void Extension::ChannelListing_Disable()
{
	Srv.setchannellisting(false);
}
void Extension::SetWelcomeMessage(const TCHAR * message)
{
	Srv.setwelcomemessage(DarkEdif::TStringToUTF8(message));
}


int FindAllowListFromName(const TCHAR * listToSet)
{
	static const std::tstring_view listNames[]{
		_T("clientnames"sv),
		_T("channelnames"sv),
		_T("receivedbyclient"sv),
		_T("receivedbyserver"sv),
		// Portuguese
		_T("nomedocliente"sv),
		_T("nomedocanal"sv),
		_T("recebidopelocliente"sv),
		_T("recebidopeloservidor"sv),
		// French
		_T("nomduclient"sv),
		_T("nomducanal"sv)
		_T("reçuparleclient"sv),
		_T("reçuparleserveur"sv)
	};

	std::tstring listToSetStr(listToSet);

	listToSetStr.erase(std::remove(listToSetStr.begin(), listToSetStr.end(), _T(' ')), listToSetStr.end());
	std::transform(listToSetStr.begin(), listToSetStr.end(), listToSetStr.begin(),
		[](const TCHAR c) { return ::_totlower(c); });

	for (std::size_t i = 0; i < std::size(listNames); ++i)
	{
		if (listNames[i] == listToSetStr)
			return (i % 4);
	}
	return -1;
}
void Extension::SetUnicodeAllowList(const TCHAR * listToSet, const TCHAR * allowListContents)
{
	const int listIndex = FindAllowListFromName(listToSet);
	if (listIndex == -1)
		return CreateError(R"(Unicode allow list %s does not exist, should be "client names", "channel names", "received by client" or "received by server".)", DarkEdif::TStringToUTF8(listToSet).c_str());

	const std::string err = Srv.setcodepointsallowedlist((lacewing::relayserver::codepointsallowlistindex)listIndex, DarkEdif::TStringToANSI(allowListContents));
	if (!err.empty())
		CreateError("Couldn't set Unicode %s allow list, %s.", DarkEdif::TStringToUTF8(listToSet).c_str(), err.c_str());
}


static AutoResponse ConvToAutoResponse(int informFusion, int immediateRespondWith,
	std::string &denyReason, Extension::GlobalInfo * globals, const char * const funcName)
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
		sprintf_s(err, std::size(err), "Invalid \"Inform Fusion\" parameter passed to \"enable/disable condition: %s\".", funcName);
	else if (immediateRespondWith < 0 || immediateRespondWith > 2)
		sprintf_s(err, std::size(err), "Invalid \"Immediate Respond With\" parameter passed to \"enable/disable condition: %s\".", funcName);
	else if (informFusion == 0 && immediateRespondWith == 2)
		sprintf_s(err, std::size(err), "Invalid parameters passed to \"enable/disable condition: %s\"; with no immediate response"
			" and Fusion condition triggering off, the server wouldn't know what to do.", funcName);
	else
	{
		// If we're not denying, replace deny parameter with blank
		if (immediateRespondWith != 1)
			denyReason.clear();

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

	globals->CreateError("%s", err);
	return AutoResponse::Invalid;
}

void Extension::EnableCondition_OnConnectRequest(int informFusion, int immediateRespondWith, const TCHAR * autoDenyReason)
{
	std::string autoDenyReasonU8 = DarkEdif::TStringToUTF8(autoDenyReason);
	AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		autoDenyReasonU8, globals, "on connect request");
	if (resp == AutoResponse::Invalid)
		return;
	globals->autoResponse_Connect = resp;
	globals->autoResponse_Connect_DenyReason = autoDenyReasonU8;
	Srv.onconnect(resp != AutoResponse::Approve_Quiet ? ::OnClientConnectRequest : nullptr);
}
void Extension::EnableCondition_OnNameSetRequest(int informFusion, int immediateRespondWith, const TCHAR * autoDenyReason)
{
	std::string autoDenyReasonU8 = DarkEdif::TStringToUTF8(autoDenyReason);
	AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		autoDenyReasonU8, globals, "on name set request");
	if (resp == AutoResponse::Invalid)
		return;
	globals->autoResponse_NameSet = resp;
	globals->autoResponse_NameSet_DenyReason = autoDenyReasonU8;
	Srv.onnameset(resp != AutoResponse::Approve_Quiet ? ::OnNameSetRequest : nullptr);
}
void Extension::EnableCondition_OnJoinChannelRequest(int informFusion, int immediateRespondWith, const TCHAR * autoDenyReason)
{
	std::string autoDenyReasonU8 = DarkEdif::TStringToUTF8(autoDenyReason);
	AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		autoDenyReasonU8, globals, "on join channel request");
	if (resp == AutoResponse::Invalid)
		return;
	globals->autoResponse_ChannelJoin = resp;
	globals->autoResponse_ChannelJoin_DenyReason = autoDenyReasonU8;
	Srv.onchannel_join(resp != AutoResponse::Approve_Quiet ? ::OnJoinChannelRequest : nullptr);
}
void Extension::EnableCondition_OnLeaveChannelRequest(int informFusion, int immediateRespondWith, const TCHAR * autoDenyReason)
{
	std::string autoDenyReasonU8 = DarkEdif::TStringToUTF8(autoDenyReason);
	AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		autoDenyReasonU8, globals, "on join channel request");
	if (resp == AutoResponse::Invalid)
		return;
	globals->autoResponse_ChannelLeave = resp;
	globals->autoResponse_ChannelLeave_DenyReason = autoDenyReasonU8;
	// Local data for channel is cleared by channel close function, not channel leave
	Srv.onchannel_leave(resp != AutoResponse::Approve_Quiet ? ::OnLeaveChannelRequest : nullptr);
}
void Extension::EnableCondition_OnMessageToPeer(int informFusion, int immediateRespondWith)
{
	std::string dummyDenyReason;
	AutoResponse resp = ConvToAutoResponse(informFusion, immediateRespondWith,
		dummyDenyReason, globals, "on join channel request");
	if (resp == AutoResponse::Invalid)
		return;
	globals->autoResponse_MessageClient = resp;
	Srv.onmessage_peer(resp != AutoResponse::Approve_Quiet ? ::OnPeerMessage : nullptr);
}
void Extension::EnableCondition_OnMessageToChannel(int informFusion, int immediateRespondWith)
{
	std::string dummyDenyReason;
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

	// Note: the Unicode allowlist for server messageis only tested if onmessage_server is set to non-null
	globals->autoResponse_MessageServer = informFusion == 1 ? AutoResponse::WaitForFusion : AutoResponse::Deny_Quiet;
	Srv.onmessage_server(informFusion == 1 ? ::OnServerMessage : nullptr);
}
void Extension::OnInteractive_Deny(const TCHAR * reason)
{
	if (InteractivePending == InteractiveType::None)
		return CreateError("Cannot deny client's action: No interactive action is pending.");
	// All of the interactive events currently allow denying
	//else if ((InteractivePending & InteractiveType::DenyPermitted) != InteractiveType::DenyPermitted)
	//	return CreateError("Cannot deny client's action: Interactive event is not compatible with this action.");
	if (!DenyReason.empty())
		return CreateError("Can't deny client's action: Set to auto-deny, or Deny was called more than once. Ignoring additional denies.");
	if ((InteractivePending == InteractiveType::ConnectRequest && globals->autoResponse_Connect != AutoResponse::WaitForFusion) ||
		(InteractivePending == InteractiveType::ClientNameSet && globals->autoResponse_NameSet != AutoResponse::WaitForFusion) ||
		(InteractivePending == InteractiveType::ChannelJoin && globals->autoResponse_ChannelJoin != AutoResponse::WaitForFusion) ||
		(InteractivePending == InteractiveType::ChannelLeave && globals->autoResponse_ChannelLeave != AutoResponse::WaitForFusion) ||
		(InteractivePending == InteractiveType::ChannelMessageIntercept && globals->autoResponse_MessageChannel != AutoResponse::WaitForFusion) ||
		(InteractivePending == InteractiveType::ClientMessageIntercept && globals->autoResponse_MessageClient != AutoResponse::WaitForFusion))
	{
		return CreateError("Can't deny client's action: Already auto-approved or auto-denied; response was already sent due to handling method. See help file.");
	}

	DenyReason = reason[0] ? DarkEdif::TStringToUTF8(reason) : "No reason specified."s;
}
void Extension::OnInteractive_ChangeClientName(const TCHAR * newName)
{
	if (InteractivePending == InteractiveType::None)
		return CreateError("Cannot change new client name: No interactive action is pending.");
	if (InteractivePending != InteractiveType::ClientNameSet)
		return CreateError("Cannot change new client name: Interactive event is not compatible with this action.");
	if (!DenyReason.empty())
		return CreateError("Cannot change new client name: Name change has already been denied, by the Deny Request action or by a rejected client name modification.");

	// Deny joining if the modified name is invalid; we can assume the server only wants the client name
	// set/changed if it's set to its modified variant, so the original should not be allowed
	const std::string_view serverModifiedNameError = "Server modified the client name and got an error on its end"sv;
	if (newName[0] == _T('\0'))
	{
		DenyReason = serverModifiedNameError;
		return CreateError("Cannot change new client name: Cannot use a blank name.");
	}

	std::string newNameU8 = DarkEdif::TStringToUTF8(newName);
	if (!lw_u8str_normalize(newNameU8) || lw_u8str_trim(newNameU8, true).empty())
	{
		DenyReason = serverModifiedNameError;
		return CreateError("Cannot change new client name: Name is invalid.");
	}
	if (newNameU8.size() > 255)
	{
		DenyReason = serverModifiedNameError;
		return CreateError("Cannot change new client name: Name is %zu bytes long after UTF-8 conversion (max is 255 bytes).", newNameU8.size());
	}

	int rejectedChar, charIndex = Srv.checkcodepointsallowed(lacewing::relayserver::codepointsallowlistindex::ClientNames, newNameU8, &rejectedChar);
	if (charIndex != -1)
	{
		DenyReason = serverModifiedNameError;
		return CreateError("Cannot change new client name: invalid code point U+%0.4X, decimal %u; valid Unicode point = %s, Unicode category = %s.",
			rejectedChar, rejectedChar, utf8proc_codepoint_valid(rejectedChar) ? "yes" : "no", utf8proc_category_string(rejectedChar));
	}

	// If NewClientName is non-empty, client->name() will be called, so check for an exact match
	// Also, this is the one invalid name case where we don't need to deny
	if (lw_sv_cmp(NewClientName, newNameU8))
		return CreateError("Cannot change new channel name: New name is same as original name.");

	NewClientName = newNameU8;
}
void Extension::OnInteractive_ChangeChannelName(const TCHAR * newName)
{
	if (InteractivePending == InteractiveType::None)
		return CreateError("Cannot change joining channel name: No interactive action is pending.");
	if (InteractivePending != InteractiveType::ChannelJoin)
		return CreateError("Cannot change joining channel name: Interactive event is not compatible with this action.");
	if (!DenyReason.empty())
		return CreateError("Cannot change joining channel name: Channel name join has already been denied, by the Deny Request action or by a rejected channel name modification.");

	// Deny joining if the modified name is invalid; we can assume the server only wants the channel to be joined
	// if the name is set to its modified variant, so the original should not be allowed
	const std::string_view serverModifiedNameError = "Server modified the channel name and got an error on its end"sv;
	if (newName[0] == _T('\0'))
	{
		DenyReason = serverModifiedNameError;
		return CreateError("Cannot change joining channel name: Cannot use a blank name.");
	}

	std::string newNameU8 = DarkEdif::TStringToUTF8(newName);
	if (newNameU8.empty() || !lw_u8str_normalize(newNameU8) || lw_u8str_trim(newNameU8, true).empty())
	{
		DenyReason = serverModifiedNameError;
		return CreateError("Cannot change joining channel name: name is invalid.");
	}
	if (newNameU8.size() > 255)
	{
		DenyReason = serverModifiedNameError;
		return CreateError("Cannot change joining channel name: Name is %zu bytes long after UTF-8 conversion (max is 255 bytes).", newNameU8.size());
	}

	// If NewChannelName is non-empty, channel->name() will be called, so check for an exact match
	// Also, this is the one invalid name case where we don't need to deny
	if (lw_sv_cmp(NewChannelName, newNameU8))
		return CreateError("Cannot change new channel name: New name is same as original name.");

	int rejectedChar, charIndex = Srv.checkcodepointsallowed(lacewing::relayserver::codepointsallowlistindex::ChannelNames, newNameU8, &rejectedChar);
	if (charIndex != -1)
	{
		DenyReason = serverModifiedNameError;
		return CreateError("Cannot change joining channel name: Code point at index %d does not match allowed list. Code point U+%0.4X, decimal %u; valid = %s, Unicode category = %s.",
			charIndex, rejectedChar, rejectedChar, utf8proc_codepoint_valid(rejectedChar) ? "yes" : "no", utf8proc_category_string(rejectedChar));
	}

	NewChannelName = newNameU8;
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
void Extension::OnInteractive_ReplaceMessageWithText(const TCHAR * NewText)
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

	// See the Decompress Received Binary for implementation. Also, see
	// !Srv.checkcodepointsallowed(lacewing::relayserver::codepointsallowlistindex::RecvByClientMessages, newNameU8)
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

void Extension::Channel_SelectByName(const TCHAR * channelNamePtr)
{
	if (channelNamePtr[0] == _T('\0'))
		return CreateError("Selecting channel by name failed: name cannot be blank.");

	const std::string channelName(DarkEdif::TStringToUTF8(channelNamePtr));
	if (channelName.size() > 255U)
		return CreateError("Selecting channel by name failed: channel name \"%s\" was %zu UTF-8 characters, exceeding max of 255 characters.", channelName.c_str(), channelName.size());

	const std::string channelNameSimplified = lw_u8str_simplify(channelName.c_str());
	selChannel = nullptr;
	{
		lacewing::readlock serverChannelListReadLock = Srv.lock_channellist.createReadLock();
		const auto& channels = Srv.getchannels();
		for (const auto &ch : channels)
		{
			if (lw_sv_cmp(ch->nameSimplified(), channelNameSimplified))
			{
				selChannel = ch;

				if (selClient == nullptr)
					return;

				// If selected client is on new channel, keep it selected, otherwise deselect client
				serverChannelListReadLock.lw_unlock();

				auto channelReadLock = ch->lock.createReadLock();
				const auto &clientsOnChannel = ch->getclients();
				if (std::find(clientsOnChannel.cbegin(), clientsOnChannel.cend(), selClient) == clientsOnChannel.cend())
					selClient = nullptr;
				return;
			}
		}
	}

	CreateError("Selecting channel by name failed: Channel with name \"%s\" not found on server.", DarkEdif::TStringToUTF8(channelNamePtr).c_str());
}
void Extension::Channel_SelectByID(int channelIDInt)
{
	if (channelIDInt < 0 || channelIDInt >= 0xFFFF)
		return CreateError("Selecting channel by ID failed: ID %i is less than 0 or greater than 65534.", channelIDInt);

	const lw_ui16 channelID = (lw_ui16)channelIDInt;
	selChannel = nullptr;
	{
		lacewing::readlock serverChannelListReadLock = Srv.lock_channellist.createReadLock();
		const auto& channels = Srv.getchannels();
		for (const auto& ch : channels)
		{
			// Channel IDs are not contiguous or ordered, so we can't look up by index,
			// or stop if read ID goes past. Example:
			// Channel IDs 0, 1, 2 made.
			// 1 is destroyed. New channel made: ID 1, but at index 2 in channels list.
			// If we stopped at ID 2 because it was past ID 1, we'd not read index 2.
			if (ch->id() == channelID)
			{
				selChannel = ch;

				if (selClient == nullptr)
					return;

				// If selected client is on new channel, keep it selected, otherwise deselect client
				serverChannelListReadLock.lw_unlock();

				auto channelReadLock = ch->lock.createReadLock();
				const auto& clientsOnChannel = ch->getclients();
				if (std::find(clientsOnChannel.cbegin(), clientsOnChannel.cend(), selClient) == clientsOnChannel.cend())
					selClient = nullptr;
				return;
			}
		}
	}

	return CreateError("Selecting channel by ID failed: channel ID %i does not exist.", channelIDInt);
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

	const auto origSelChannel = selChannel;
	const auto origSelClient = selClient;
	const auto origLoopName = loopName;

	std::vector<decltype(selClient)> channelClientListDup;
	{
		auto channelReadLock = origSelChannel->lock.createReadLock();
		channelClientListDup = origSelChannel->getclients();
	}

	for (const auto &cli : channelClientListDup)
	{
		selChannel = origSelChannel;
		selClient = cli;
		loopName = std::tstring_view();
		Runtime.GenerateEvent(8);
	}

	selChannel = origSelChannel;
	selClient = origSelClient;
	loopName = std::tstring_view();
	Runtime.GenerateEvent(44);

	loopName = origLoopName;
}
void Extension::Channel_LoopClientsWithName(const TCHAR * passedLoopName)
{
	if (!selChannel)
		return CreateError("Loop Clients On Channel With Name was called without a channel being selected.");
	if (passedLoopName[0] == _T('\0'))
		return CreateError("Cannot loop clients joined to channel \"%s\": invalid loop name \"\" supplied.", selChannel->name().c_str());

	// You can loop a closed channel's clients, but it's read-only.
	const auto origSelChannel = selChannel;
	const auto origSelClient = selClient;
	const auto origLoopName = loopName;
	const std::tstring_view loopNameDup(passedLoopName);

	std::vector<decltype(selClient)> channelClientListDup;
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
void Extension::Channel_SetLocalData(const TCHAR * key, const TCHAR * value)
{
	if (!selChannel)
		return CreateError("Could not set channel local data: No channel selected.");
	// if (selChannel->readonly())
	//	return CreateError("Could not set channel local data: Channel is read-only.");

	globals->SetLocalData(selChannel, key, value);
}
void Extension::Channel_CreateChannelWithMasterByName(const TCHAR * channelNamePtr, int hiddenInt, int autocloseInt, const TCHAR * masterClientName)
{
	if (channelNamePtr[0] == _T('\0'))
		return CreateError("Cannot create new channel; blank channel name supplied.");
	if (hiddenInt < 0 || hiddenInt > 1)
		return CreateError("Cannot create new channel; hidden channel setting is %i, should be 0 or 1.", hiddenInt);
	if (autocloseInt < 0 || autocloseInt > 1)
		return CreateError("Cannot create new channel; autoclose channel setting is %i, should be 0 or 1.", autocloseInt);

	const std::string channelNameU8 = DarkEdif::TStringToUTF8(channelNamePtr);
	if (channelNameU8.size() > 254)
		return CreateError("Cannot create new channel; channel name \"%s\" is too long (after UTF-8 conversion).", DarkEdif::TStringToUTF8(channelNamePtr).c_str());
	const std::string channelNameU8Simplified = lw_u8str_simplify(channelNameU8);

	const bool hidden = hiddenInt == 1, autoclose = autocloseInt == 1;

	{
		auto serverChannelListReadLock = Srv.lock_channellist.createReadLock();
		const auto & channels = Srv.getchannels();
		auto foundChIt = std::find_if(channels.cbegin(), channels.cend(),
			[&](const auto & cli) {
				return lw_sv_cmp(cli->nameSimplified(), channelNameU8Simplified); });
		if (foundChIt != channels.cend())
		{
			return CreateError("Error creating channel with name \"%s\"; channel already exists (matching channel ID %hu, name %s).",
				DarkEdif::TStringToUTF8(channelNamePtr).c_str(), (**foundChIt).id(), (**foundChIt).name().c_str());
		}
	}

	// Blank master client
	decltype(selClient) masterClientToUse;
	if (masterClientName[0] == _T('\0'))
	{
		// Autoclose means when master leaves, you leave. Since there's no "set master" action yet...
		// the channel will autoclose immediately on creation, which makes no sense.
		if (autoclose)
			return CreateError("Error creating channel; no master specified, and autoclose \"leave when master leaves\" setting cannot be enabled when there is no master to leave.");
	}
	else // Pick master client
	{
		const std::string masterClientNameU8Simplified = TStringToUTF8Simplified(channelNamePtr);
		auto serverClientListReadLock = Srv.lock_clientlist.createReadLock();
		const auto & clients = Srv.getclients();
		auto foundCliIt =
			std::find_if(clients.cbegin(), clients.cend(),
				[&](const auto & cli) { return lw_sv_cmp(cli->nameSimplified(), masterClientNameU8Simplified); });
		if (foundCliIt == clients.cend())
			return CreateError("Error creating channel; specified master client name \"%s\" not found on server.", DarkEdif::TStringToUTF8(masterClientName).c_str());
		selClient = *foundCliIt;
	}

	// Will submit joinchannel_response if needed; also adds to server channel list
	Srv.createchannel(channelNameU8, masterClientToUse, hidden, autoclose);
}
void Extension::Channel_CreateChannelWithMasterByID(const TCHAR * channelNamePtr, int hiddenInt, int autocloseInt, int masterClientID)
{
	if (channelNamePtr[0] == _T('\0'))
		return CreateError("Cannot create new channel; blank channel name supplied.");
	if (hiddenInt < 0 || hiddenInt > 1)
		return CreateError("Cannot create new channel; hidden channel setting is %i, should be 0 or 1.", hiddenInt);
	if (autocloseInt < 0 || autocloseInt > 1)
		return CreateError("Cannot create new channel; autoclose channel setting is %i, should be 0 or 1.", autocloseInt);
	if (masterClientID < -1 || masterClientID >= 65535)
		return CreateError("Cannot create new channel; master client ID %i is invalid. Use -1 for no master.", masterClientID);

	const std::string channelNameU8 = DarkEdif::TStringToUTF8(channelNamePtr);
	if (channelNameU8.size() > 254)
		return CreateError("Cannot create new channel; channel name \"%s\" is too long (after UTF-8 conversion).", DarkEdif::TStringToUTF8(channelNamePtr).c_str());
	const std::string channelNameU8Simplified = lw_u8str_simplify(channelNameU8);

	bool hidden = hiddenInt == 1, autoclose = autocloseInt == 1;

	{
		auto serverChannelListReadLock = Srv.lock_channellist.createReadLock();
		const auto & channels = Srv.getchannels();
		auto foundChIt =
			std::find_if(channels.cbegin(), channels.cend(),
				[=](const auto & cli) { return lw_sv_cmp(cli->nameSimplified(), channelNameU8Simplified); });
		if (foundChIt != channels.cend())
			return CreateError("Error creating channel with name \"%s\"; channel already exists.", DarkEdif::TStringToUTF8(channelNamePtr).c_str());
	}

	// Blank master client
	decltype(selClient) masterClientToUse;
	if (masterClientID == -1)
	{
		// Autoclose means when master leaves, you leave. Since there's no "set master" action yet... this makes no sense.
		if (autoclose)
			return CreateError("Error creating channel; no master specified, and autoclose \"leave when master leaves\" setting cannot be enabled when there is no master to leave.");
	}
	else // Pick master client
	{
		auto serverClientListReadLock = Srv.lock_clientlist.createReadLock();
		const auto & clients = Srv.getclients();
		auto foundCliIt =
			std::find_if(clients.cbegin(), clients.cend(),
				[=](const auto & cli) { return cli->id() == masterClientID; });
		if (foundCliIt == clients.cend())
			return CreateError("Error creating channel; specified master client ID %i not found on server.", masterClientID);
		selClient = *foundCliIt;
	}

	// Will submit joinchannel_response if needed; also adds to server channel list
	Srv.createchannel(channelNameU8, masterClientToUse, hidden, autoclose);
}
void Extension::Channel_JoinClientByID(int clientID)
{
	if (!selChannel)
		return CreateError("Cannot force client to join channel; no channel selected.");
	if (selChannel->readonly())
		return CreateError("Error forcing client to join channel; channel is read-only.");
	if (clientID < -1 || clientID >= 65535)
		return CreateError("Cannot join client to channel; supplied client ID %i is invalid. Use -1 for currently selected client.", clientID);

	// Note: if user attempts to connect to "abc", and Fusion dev decides, while handling the event,
	// to join the user to "abc", on server end, this will be auto-denied by sanity checks in second run of joinchannel_response.
	// On client end, this will cause a join channel success, followed by a "error joining channel, you're already on the channel"
	// join channel denied message.

	decltype(selClient) clientToUse = nullptr;
	if (clientID == -1)
	{
		if (!selClient)
			return CreateError("Error joining client to selected channel; ID -1 was supplied and no client currently selected.");
		clientToUse = selClient;
	}
	else
	{
		auto serverClientListReadLock = Srv.lock_clientlist.createReadLock();
		const auto & clients = Srv.getclients();
		auto foundCliIt =
			std::find_if(clients.cbegin(), clients.cend(),
				[=](const auto & cli) { return cli->id() == clientID; });
		if (foundCliIt == clients.cend())
			return CreateError("Error joining client with ID %i from channel; client with that ID not found on server.", clientID);
		clientToUse = *foundCliIt;
	}

	if (clientToUse->readonly())
	{
		return CreateError("Error joining client \"%s\" (ID %i) to channel \"%s\"; client is read-only.",
			clientToUse->name().c_str(), clientID, selChannel->name().c_str());
	}

	// Check that channel does not contain client, and client does not contain channel; there may be inconsistency if client is currently leaving.
	{
		auto chCliReadLock = selChannel->lock.createReadLock();
		const auto & clientsOnChannel = selChannel->getclients();
		if (std::find(clientsOnChannel.cbegin(), clientsOnChannel.cend(), selClient) != clientsOnChannel.cend())
		{
			return CreateError("Error joining client \"%s\" (ID %i) to channel \"%s\"; client is already on the channel.",
				clientToUse->name().c_str(), clientID, selChannel->name().c_str());
		}
	}
	{
		auto cliChReadLock = clientToUse->lock.createReadLock();
		const auto & channelsOnClient = clientToUse->getchannels();
		if (std::find(channelsOnClient.cbegin(), channelsOnClient.cend(), selChannel) != channelsOnClient.cend())
		{
			return CreateError("Error joining client \"%s\" (ID %i) to channel \"%s\"; client is already on the channel.",
				clientToUse->name().c_str(), clientID, selChannel->name().c_str());
		}
	}

	// All checks passed; make it happen
	Srv.joinchannel_response(selChannel, clientToUse, std::string_view());
}
void Extension::Channel_JoinClientByName(const TCHAR * clientNamePtr)
{
	if (!selChannel)
		return CreateError("Cannot join client to channel; no channel selected.");
	if (selChannel->readonly())
		return CreateError("Error joining client to selected channel; channel is read-only.");

	decltype(selClient) clientToUse = nullptr;
	if (clientNamePtr[0] == _T('\0'))
	{
		if (!selClient)
			return CreateError("Error joining client to selected channel; blank client name was supplied and no client currently selected.");
		clientToUse = selClient;
	}
	else
	{
		const std::string clientNameU8Simplified = TStringToUTF8Simplified(clientNamePtr);
		auto serverClientListReadLock = Srv.lock_clientlist.createReadLock();
		const auto & clients = Srv.getclients();
		auto foundCliIt =
			std::find_if(clients.cbegin(), clients.cend(),
				[&](const auto & cli) { return lw_sv_cmp(cli->nameSimplified(), clientNameU8Simplified); });
		if (foundCliIt == clients.cend())
			return CreateError("Error joining client with name \"%s\" to channel; client with that name not found on server.", DarkEdif::TStringToUTF8(clientNamePtr).c_str());
		clientToUse = *foundCliIt;
	}

	if (clientToUse->readonly())
	{
		return CreateError("Error joining client \"%s\" (ID %hu) to channel \"%s\"; client is read-only.",
			clientToUse->name().c_str(), clientToUse->id(), selChannel->name().c_str());
	}

	// Check that channel contains client, and client contains channel; there may be inconsistency if client is currently leaving.
	{
		auto chCliReadLock = selChannel->lock.createReadLock();
		const auto & clientsOnChannel = selChannel->getclients();
		if (std::find(clientsOnChannel.cbegin(), clientsOnChannel.cend(), selClient) != clientsOnChannel.cend())
		{
			return CreateError("Error joining client \"%s\" (ID %hu) to channel \"%s\"; client is already on the channel.",
				clientToUse->name().c_str(), clientToUse->id(), selChannel->name().c_str());
		}
	}
	{
		auto cliChReadLock = clientToUse->lock.createReadLock();
		const auto & channelsOnClient = clientToUse->getchannels();
		if (std::find(channelsOnClient.cbegin(), channelsOnClient.cend(), selChannel) != channelsOnClient.cend())
		{
			return CreateError("Error joining client \"%s\" (ID %hu) to channel \"%s\"; client is already on the channel.",
				clientToUse->name().c_str(), clientToUse->id(), selChannel->name().c_str());
		}
	}

	// All checks passed; make it happen
	Srv.joinchannel_response(selChannel, clientToUse, std::string_view());
}
void Extension::Channel_KickClientByID(int clientID)
{
	if (!selChannel)
		return CreateError("Cannot force client to leave channel; no channel selected.");
	if (selChannel->readonly())
		return CreateError("Error forcing client to leave channel; channel is read-only.");
	if (clientID < -1 || clientID >= 65535)
		return CreateError("Cannot kick client from channel; supplied client ID %i is invalid. Use -1 for currently selected client.", clientID);

	decltype(selClient) clientToUse = nullptr;

	if (clientID == -1)
	{
		if (!selClient)
			return CreateError("Error kicking client from selected channel; ID -1 was supplied but no client currently selected.");
		clientToUse = selClient;
	}
	else
	{
		auto serverClientListReadLock = Srv.lock_clientlist.createReadLock();
		const auto & clients = Srv.getclients();
		auto foundCliIt =
			std::find_if(clients.cbegin(), clients.cend(),
				[=](const auto & cli) { return cli->id() == clientID; });
		if (foundCliIt == clients.cend())
			return CreateError("Error kicking client with ID %i from channel; client with that ID not found on server.", clientID);
		clientToUse = *foundCliIt;
	}

	if (clientToUse->readonly())
	{
		return CreateError("Error kicking client \"%s\" (ID %i) from channel \"%s\"; client is read-only.",
			clientToUse->name().c_str(), clientID, selChannel->name().c_str());
	}

	// Check that channel contains client, and client contains channel; there may be inconsistency if client is currently leaving.
	{
		auto chCliReadLock = selChannel->lock.createReadLock();
		const auto & clientsOnChannel = selChannel->getclients();
		if (std::find(clientsOnChannel.cbegin(), clientsOnChannel.cend(), selClient) == clientsOnChannel.cend())
		{
			return CreateError("Error kicking client \"%s\" (ID %i) from channel \"%s\"; client is not on the channel.",
				clientToUse->name().c_str(), clientID, selChannel->name().c_str());
		}
	}
	{
		auto cliChReadLock = clientToUse->lock.createReadLock();
		const auto & channelsOnClient = clientToUse->getchannels();
		if (std::find(channelsOnClient.cbegin(), channelsOnClient.cend(), selChannel) == channelsOnClient.cend())
		{
			return CreateError("Error kicking client \"%s\" (ID %i) from channel \"%s\"; client is not on the channel.",
				clientToUse->name().c_str(), clientID, selChannel->name().c_str());
		}
	}

	// All checks passed; make it happen
	Srv.leavechannel_response(selChannel, clientToUse, std::string_view());
}
void Extension::Channel_KickClientByName(const TCHAR * clientNamePtr)
{
	if (!selChannel)
		return CreateError("Cannot force client to leave channel; no channel selected.");
	if (selChannel->readonly())
		return CreateError("Error forcing client to leave channel; channel is read-only.");

	decltype(selClient) clientToUse = nullptr;
	if (clientNamePtr[0] == _T('\0'))
	{
		if (!selClient)
			return CreateError("Error kicking client from selected channel; blank name was supplied but no client currently selected.");
		clientToUse = selClient;
	}
	else
	{
		const std::string clientNameU8Simplified = TStringToUTF8Simplified(clientNamePtr);
		auto serverClientListReadLock = Srv.lock_clientlist.createReadLock();
		const auto & clients = Srv.getclients();
		auto foundCliIt =
			std::find_if(clients.cbegin(), clients.cend(),
				[&](const auto & cli) { return lw_sv_cmp(cli->nameSimplified(), clientNameU8Simplified); });
		if (foundCliIt == clients.cend())
			return CreateError("Error kicking client with name \"%s\" from channel; client with that name not found on server.", DarkEdif::TStringToUTF8(clientNamePtr).c_str());
		clientToUse = *foundCliIt;
	}

	if (clientToUse->readonly())
	{
		return CreateError("Error kicking client \"%s\" (ID %hu) from channel \"%s\"; client is read-only.",
			clientToUse->name().c_str(), clientToUse->id(), selChannel->name().c_str());
	}

	// Check that channel contains client, and client contains channel; there may be inconsistency if client is currently leaving.
	{
		auto chCliReadLock = selChannel->lock.createReadLock();
		const auto & clientsOnChannel = selChannel->getclients();
		if (std::find(clientsOnChannel.cbegin(), clientsOnChannel.cend(), selClient) == clientsOnChannel.cend())
		{
			return CreateError("Error kicking client \"%s\" (ID %hu) from channel \"%s\"; client is not on the channel.",
				clientToUse->name().c_str(), clientToUse->id(), selChannel->name().c_str());
		}
	}
	{
		auto cliChReadLock = clientToUse->lock.createReadLock();
		const auto & channelsOnClient = clientToUse->getchannels();
		if (std::find(channelsOnClient.cbegin(), channelsOnClient.cend(), selChannel) == channelsOnClient.cend())
		{
			return CreateError("Error kicking client \"%s\" (ID %hu) from channel \"%s\"; client is not on the channel.",
				clientToUse->name().c_str(), clientToUse->id(), selChannel->name().c_str());
		}
	}

	// All checks passed; make it happen
	Srv.leavechannel_response(selChannel, clientToUse, std::string_view());
}
void Extension::LoopAllChannels()
{
	const auto origSelChannel = selChannel;
	const auto origSelClient = selClient;
	const auto origLoopName = loopName;

	std::vector<decltype(selChannel)> serverChannelListDup;
	{
		auto serverChannelListReadLock = Srv.lock_channellist.createReadLock();
		serverChannelListDup = Srv.getchannels();
	}

	for (const auto& ch : serverChannelListDup)
	{
		selChannel = ch;
		selClient = nullptr;
		loopName = std::tstring_view();
		Runtime.GenerateEvent(5);
	}

	selChannel = origSelChannel;
	selClient = origSelClient;
	loopName = std::tstring_view();
	Runtime.GenerateEvent(45);

	loopName = origLoopName;
}
void Extension::LoopAllChannelsWithName(const TCHAR * passedLoopName)
{
	if (passedLoopName[0] == _T('\0'))
		return CreateError("Cannot loop all channels on server: invalid loop name \"\" supplied.");

	const auto origSelChannel = selChannel;
	const auto origSelClient = selClient;
	const auto origLoopName = loopName;
	const std::tstring_view loopNameDup(passedLoopName);

	std::vector<decltype(selChannel)> serverChannelListDup;
	{
		auto serverChannelListReadLock = Srv.lock_channellist.createReadLock();
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
		selClient->disconnect(selClient);
}
void Extension::Client_SetLocalData(const TCHAR * key, const TCHAR * value)
{
	if (!selClient)
		return CreateError("Could not set client local data: No client selected.");
	// if (selClient->readonly())
	//	return CreateError("Could not set client local data: Client is read-only.");

	globals->SetLocalData(selClient, key, value);
}
void Extension::Client_JoinToChannel(const TCHAR * channelNamePtr)
{
	CreateError("Not implemented.");
}
void Extension::Client_LoopJoinedChannels()
{
	if (!selClient)
		return CreateError("Cannot loop client's joined channels: No client selected.");

	const auto origSelClient = selClient;
	const auto origSelChannel = selChannel;
	const auto origLoopName = loopName;

	std::vector<decltype(selChannel)> joinedChannelListDup;
	{
		auto selClientReadLock = origSelClient->lock.createReadLock();
		joinedChannelListDup = origSelClient->getchannels();
	}

	for (const auto &joinedCh : joinedChannelListDup)
	{
		selChannel = joinedCh;
		selClient = origSelClient;
		loopName = std::tstring_view();
		Runtime.GenerateEvent(6);
	}

	selChannel = origSelChannel;
	selClient = origSelClient;
	loopName = std::tstring_view();

	Runtime.GenerateEvent(47);
	loopName = origLoopName;
}
void Extension::Client_LoopJoinedChannelsWithName(const TCHAR * passedLoopName)
{
	if (!selClient)
		return CreateError("Cannot loop client's joined channels: No client selected.");
	if (passedLoopName[0] == _T('\0'))
		return CreateError("Cannot loop client ID %hu, name \"%s\" joined channels: invalid loop name \"\" supplied.", selClient->id(), selClient->name().c_str());

	const auto origSelClient = selClient;
	const auto origSelChannel = selChannel;
	const auto origLoopName = loopName;
	const std::tstring_view loopNameDup(passedLoopName);

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
void Extension::Client_SelectByName(const TCHAR * clientName)
{
	if (clientName[0] == _T('\0'))
		return CreateError("Select Client By Name was called with a blank name.");

	selClient = nullptr;
	{
		const std::string clientNameU8Simplified = TStringToUTF8Simplified(clientName);
		auto serverClientListReadLock = Srv.lock_clientlist.createReadLock();
		const auto &clients = Srv.getclients();
		auto foundCliIt =
			std::find_if(clients.cbegin(), clients.cend(),
				[&](const auto &cli) { return lw_sv_cmp(cli->nameSimplified(), clientNameU8Simplified); });
		if (foundCliIt == clients.cend())
			return CreateError("Client with name %s not found on server.", DarkEdif::TStringToUTF8(clientName).c_str());
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
	if (clientID < 0 || clientID >= 0xFFFF)
		return CreateError("Could not select client on channel, ID is below 0 or greater than 65535.");

	selClient = nullptr;
	{
		auto serverClientListReadLock = Srv.lock_clientlist.createReadLock();
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
	const auto origSelClient = selClient;
	const auto origSelChannel = selChannel;
	const auto origLoopName = loopName;

	std::vector<decltype(selClient)> clientListDup;
	{
		auto serverClientListReadLock = Srv.lock_clientlist.createReadLock();
		clientListDup = Srv.getclients();
	}

	for (const auto &selectedClient : clientListDup)
	{
		selChannel = nullptr;
		selClient = selectedClient;
		loopName = std::tstring_view();
		Runtime.GenerateEvent(7);
	}

	selChannel = origSelChannel;
	selClient = origSelClient;
	loopName = std::tstring_view();
	Runtime.GenerateEvent(46);

	loopName = origLoopName;
}
void Extension::LoopAllClientsWithName(const TCHAR * passedLoopName)
{
	if (passedLoopName[0] == _T('\0'))
		return CreateError("Cannot loop all clients: invalid loop name \"\" supplied.");

	const auto origSelClient = selClient;
	const auto origSelChannel = selChannel;
	const auto origLoopName = loopName;
	const std::tstring_view loopNameDup(passedLoopName);

	std::vector<decltype(selClient)> clientListDup;
	{
		auto serverClientListReadLock = Srv.lock_clientlist.createReadLock();
		clientListDup = Srv.getclients();
	}

	for (const auto &selectedClient : clientListDup)
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
void Extension::SendTextToChannel(int subchannel, const TCHAR * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		return CreateError("Send Text to Channel was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	if (!selChannel)
		return CreateError("Send Text to Channel was called without a channel being selected.");
	if (selChannel->readonly())
		return CreateError("Send Text to Channel was called with a read-only channel, name %s.", selChannel->name().c_str());

	selChannel->send(subchannel, DarkEdif::TStringToUTF8(textToSend), 0);
}
void Extension::SendTextToClient(int subchannel, const TCHAR * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		return CreateError("Send Text to Client was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	if (!selClient)
		return CreateError("Send Text to Client was called without a client being selected.");
	if (selClient->readonly())
		return CreateError("Send Text to Client was called with a read-only client ID %hu, name %s.", selClient->id(), selClient->name().c_str());

	selClient->send(subchannel, DarkEdif::TStringToUTF8(textToSend), 0);
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
		SendMsg_Clear();
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
		SendMsg_Clear();
}
void Extension::BlastTextToChannel(int subchannel, const TCHAR * textToBlast)
{
	if (subchannel > 255 || subchannel < 0)
		return CreateError("Blast Text to Channel was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	if (!selChannel)
		return CreateError("Blast Text to Channel was called without a channel being selected.");
	if (selChannel->readonly())
		return CreateError("Blast Text to Channel was called with a read-only channel, name %s.", selChannel->name().c_str());

	const std::string utf8Msg = DarkEdif::TStringToUTF8(textToBlast);
	if (utf8Msg.size() > globals->maxUDPSize)
		return CreateError("Blast Text to Channel was called with text too large (%zu bytes).", utf8Msg.size());

	selChannel->blast(subchannel, utf8Msg, 0);
}
void Extension::BlastTextToClient(int subchannel, const TCHAR * textToBlast)
{
	if (subchannel > 255 || subchannel < 0)
		return CreateError("Blast Text to Client was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	if (!selClient)
		return CreateError("Blast Text to Client was called without a client being selected.");
	if (selClient->readonly())
		return CreateError("Blast Text to Client was called with a read-only client: ID %hu, name %s.", selClient->id(), selClient->name().c_str());

	const std::string utf8Msg = DarkEdif::TStringToUTF8(textToBlast);
	if (utf8Msg.size() > globals->maxUDPSize)
		return CreateError("Blast Text to Client was called with text too large (%zu bytes).", utf8Msg.size());

	selClient->blast(subchannel, utf8Msg, 0);
}
void Extension::BlastNumberToChannel(int subchannel, int numToBlast)
{
	if (subchannel > 255 || subchannel < 0)
		return CreateError("Blast Number to Channel was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	if (!selChannel)
		return CreateError("Blast Number to Channel was called without a channel being selected.");
	if (selChannel->readonly())
		return CreateError("Blast Number to Channel was called with a read-only channel, name %s.", selChannel->name().c_str());

	selChannel->blast(subchannel, std::string_view((char *)&numToBlast, sizeof(int)), 1);
}
void Extension::BlastNumberToClient(int subchannel, int numToBlast)
{
	if (subchannel > 255 || subchannel < 0)
		return CreateError("Blast Number to Client was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	if (!selClient)
		return CreateError("Blast Number to Client was called without a client being selected.");
	if (selClient->readonly())
		return CreateError("Blast Number to Client was called with a read-only client: ID %hu, name %s.", selClient->id(), selClient->name().c_str());

	selClient->blast(subchannel, std::string_view((char *)&numToBlast, sizeof(int)), 1);
}
void Extension::BlastBinaryToChannel(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Blast Binary to Channel was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selChannel)
		CreateError("Blast Binary to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Blast Binary to Channel was called with a read-only channel, name %s.", selChannel->name().c_str());
	else if (SendMsgSize > globals->maxUDPSize)
		CreateError("Blast Binary to Channel was called with binary too large (%zu bytes).", SendMsgSize);
	else
		selChannel->blast(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		SendMsg_Clear();
}
void Extension::BlastBinaryToClient(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Blast Binary to Client was called with an invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selClient)
		CreateError("Blast Binary to Client was called without a client being selected.");
	else if (selClient->readonly())
		CreateError("Blast Binary to Client was called with a read-only client: ID %hu, name %s.", selClient->id(), selClient->name().c_str());
	else if (SendMsgSize > globals->maxUDPSize)
		CreateError("Blast Binary to Client was called with binary too large (%zu bytes).", SendMsgSize);
	else
		selClient->blast(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		SendMsg_Clear();
}
void Extension::SendMsg_AddASCIIByte(const TCHAR * byte)
{
	const std::string u8Str(DarkEdif::TStringToUTF8(byte));
	if (u8Str.size() != 1)
		return CreateError("Adding ASCII character to binary failed: byte \"%s\" supplied was part of a string, not a single byte.", u8Str.c_str());

	// ANSI byte, not ASCII; or not displayable, so probably a corrupt string.
	if (reinterpret_cast<const std::uint8_t &>(u8Str[0]) > 127 || !std::isprint(u8Str[0]))
		return CreateError("Adding ASCII character to binary failed: byte \"%u\" was not a valid ASCII character.", (unsigned int) reinterpret_cast<const std::uint8_t &>(u8Str[0]));

	SendMsg_Sub_AddData(u8Str.c_str(), sizeof(char));
}
void Extension::SendMsg_AddByteInt(int byte)
{
	if (byte > UINT8_MAX || byte < INT8_MIN)
	{
		return CreateError("Adding byte to binary (as int) failed: the supplied number %i will not fit in range "
			"%i to %i (signed byte) or range 0 to %i (unsigned byte).", byte, INT8_MIN, INT8_MAX, UINT8_MAX);
	}

	SendMsg_Sub_AddData(&byte, sizeof(char));
}
void Extension::SendMsg_AddShort(int _short)
{
	if (_short > UINT16_MAX || _short < INT16_MIN)
	{
		return CreateError("Adding short to binary failed: the supplied number %i will not fit in range "
			"%i to %i (signed short) or range 0 to %i (unsigned short).", _short, INT16_MIN, INT16_MAX, UINT16_MAX);
	}

	SendMsg_Sub_AddData(&_short, sizeof(short));
}
void Extension::SendMsg_AddInt(int _int)
{
	SendMsg_Sub_AddData(&_int, 4);
}
void Extension::SendMsg_AddFloat(float _float)
{
	SendMsg_Sub_AddData(&_float, 4);
}
void Extension::SendMsg_AddStringWithoutNull(const TCHAR * string)
{
	const std::string u8String = DarkEdif::TStringToUTF8(string);
	SendMsg_Sub_AddData(u8String.c_str(), u8String.size());
}
void Extension::SendMsg_AddString(const TCHAR * string)
{
	const std::string u8Str = DarkEdif::TStringToUTF8(string);
	SendMsg_Sub_AddData(u8Str.c_str(), u8Str.size() + 1U);
}
void Extension::SendMsg_AddBinaryFromAddress(unsigned int address, int size)
{
	// Address is checked in SendMsg_Sub_AddData()
	if (size < 0)
		return CreateError("Add binary from address failed: Size %i less than 0.", size);

	SendMsg_Sub_AddData((void *)(long)address, size);
}
void Extension::SendMsg_AddFileToBinary(const TCHAR * filenameParam)
{
	if (filenameParam[0] == _T('\0'))
		return CreateError("Cannot add file to send binary; filename \"\" is invalid.");

	// Unembed file if necessary
	const std::tstring filename = DarkEdif::MakePathUnembeddedIfNeeded(this, filenameParam);
	if (filename[0] == _T('>'))
		return CreateError("Cannot add file \"%s\" to send binary, error %s occurred with opening the file."
			" The send binary has not been modified.", DarkEdif::TStringToUTF8(filenameParam).c_str(), DarkEdif::TStringToUTF8(filename.substr(1)).c_str());

	// Open and deny other programs write privileges
#ifdef _WIN32
	FILE * file = _tfsopen(filename.c_str(), _T("rb"), SH_DENYWR);
#else
	FILE * file = fopen(filename.c_str(), "rb");
#endif
	if (!file)
	{
		ErrNoToErrText();
		return CreateError("Cannot add file \"%s\" (original \"%s\") to send binary, error %i (%s) occurred with opening the file."
			" The send binary has not been modified.", DarkEdif::TStringToUTF8(filename).c_str(), DarkEdif::TStringToUTF8(filenameParam).c_str(), errno, errtext);
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
			"to add file into message. The send binary has not been modified.",
			DarkEdif::TStringToUTF8(filenameParam).c_str());
	}
	else
		SendMsg_Sub_AddData(buffer.get(), amountRead);

	fclose(file);
}
void Extension::SendMsg_Resize(int newSize)
{
	if (newSize < 0)
		return CreateError("Cannot resize binary to send: new size %u bytes is negative.", newSize);

	char * NewMsg = (char *)realloc(SendMsg, newSize);
	if (!NewMsg)
	{
		return CreateError("Cannot resize binary to send: reallocation of memory into %u bytes failed.\r\n"
			"Binary to send has not been modified.", newSize);
	}
	// Clear new bytes to 0
	memset(NewMsg + SendMsgSize, 0, newSize - SendMsgSize);

	SendMsg = NewMsg;
	SendMsgSize = newSize;
}
void Extension::SendMsg_CompressBinary()
{
	if (SendMsgSize <= 0)
		return CreateError("Cannot compress send binary; binary is empty.");

	z_stream strm = {};
	int ret = deflateInit(&strm, 9); // 9 is maximum compression level
	if (ret)
		return CreateError("Compressing send binary failed, zlib error %i \"%s\" occurred with initiating compression.", ret, (strm.msg ? strm.msg : ""));

	// 4: precursor lw_ui32 with uncompressed size, required by Relay
	// 256: if compression results in larger message, it shouldn't be *that* much larger.

	std::uint8_t * output_buffer = (std::uint8_t *)malloc(4 + SendMsgSize + 256);
	if (!output_buffer)
	{
		CreateError("Compressing send binary failed, couldn't allocate enough memory. Desired %zu bytes.",
			(size_t)4 + SendMsgSize + 256);
		deflateEnd(&strm);
		return;
	}

	// Store size as precursor - required by Relay
	*(lw_ui32 *)output_buffer = (std::uint32_t)SendMsgSize;

	strm.next_in = (std::uint8_t *)SendMsg;
	strm.avail_in = (std::uint32_t)SendMsgSize;

	// Allocate memory for compression
	strm.avail_out = (std::uint32_t)SendMsgSize + 256;
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
void Extension::SendMsg_Clear()
{
	free(SendMsg);
	SendMsg = NULL;
	SendMsgSize = 0;
}
void Extension::RecvMsg_DecompressBinary()
{
	if (threadData->receivedMsg.content.size() <= 4)
	{
		return CreateError("Cannot decompress received binary; message is %zu bytes and too small to be a valid compressed message.",
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
	const std::string_view inputData(threadData->receivedMsg.content.data() + sizeof(lw_ui32),
		threadData->receivedMsg.content.size() - sizeof(lw_ui32));

	unsigned char * output_buffer = (unsigned char *)malloc(expectedUncompressedSize);
	if (!output_buffer)
	{
		inflateEnd(&strm);
		return CreateError("Decompression failed; couldn't allocate enough memory. Desired %u bytes.", expectedUncompressedSize);
	}

	strm.next_in = (unsigned char *)inputData.data();
	strm.avail_in = (std::uint32_t)inputData.size();
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

	// Used to assign all exts in a questionable way, but threadData is now std::shared_ptr, so no need.
	threadData->receivedMsg.content.assign((char *)output_buffer, expectedUncompressedSize);
	threadData->receivedMsg.cursor = 0;

	free(output_buffer); // .assign() copies the memory
}
void Extension::RecvMsg_MoveCursor(int position)
{
	if (position < 0)
		return CreateError("Cannot move cursor; index %d is less than 0.", position);
	if (threadData->receivedMsg.content.size() - position <= 0)
		return CreateError("Cannot move cursor to index %d; message indexes are 0 to %zu.", position, threadData->receivedMsg.content.size());

	threadData->receivedMsg.cursor = position;
}
void Extension::RecvMsg_SaveToFile(int passedPosition, int passedSize, const TCHAR * filename)
{
	if (passedPosition < 0)
		return CreateError("Cannot save received binary; position %i is less than 0.", passedPosition);
	if (passedSize <= 0)
		return CreateError("Cannot save received binary; size of %i is equal or less than 0.", passedSize);
	if (filename[0] == _T('\0'))
		return CreateError("Cannot save received binary; filename \"\" is invalid.");

	size_t position = (size_t)passedPosition;
	size_t size = (size_t)passedSize;

	if (position + size > threadData->receivedMsg.content.size())
	{
		return CreateError("Cannot save received binary to file \"%s\"; message doesn't have %zu"
			" bytes from position %zu onwards, it only has %zu bytes.",
			DarkEdif::TStringToUTF8(filename).c_str(), size, position, threadData->receivedMsg.content.size() - position);
	}
#ifdef _WIN32
	FILE * File = _tfsopen(filename, _T("wb"), SH_DENYWR);
#else
	FILE * File = fopen(filename, "wb");
#endif
	if (!File)
	{
		ErrNoToErrText();
		CreateError("Cannot save received binary to file \"%s\", error %i \"%s\""
			" occurred with opening the file.", DarkEdif::TStringToUTF8(filename).c_str(), errno, errtext);
		return;
	}

	size_t amountWritten;
	if ((amountWritten = fwrite(threadData->receivedMsg.content.data() + position, 1, size, File)) != size)
	{
		ErrNoToErrText();
		CreateError("Cannot save received binary to file \"%s\", error %i \"%s\""
			" occurred with writing the file. Wrote %zu bytes total.", DarkEdif::TStringToUTF8(filename).c_str(), errno, errtext, amountWritten);
		fclose(File);
		return;
	}

	if (fclose(File))
	{
		ErrNoToErrText();
		CreateError("Cannot save received binary to file \"%s\", error %i \"%s\""
			" occurred with writing the end of the file.", DarkEdif::TStringToUTF8(filename).c_str(), errno, errtext);
	}
}
void Extension::RecvMsg_AppendToFile(int passedPosition, int passedSize, const TCHAR * filename)
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
		return CreateError("Cannot append received binary to file \"%s\"; message doesn't have %zu"
			" bytes from position %zu onwards, it only has %zu bytes.",
			DarkEdif::TStringToUTF8(filename).c_str(), size, position, threadData->receivedMsg.content.size() - position);
	}
#ifdef _WIN32
	FILE * File = _tfsopen(filename, _T("ab"), SH_DENYWR);
#else
	FILE * File = fopen(filename, "ab");
#endif
	if (!File)
	{
		ErrNoToErrText();
		CreateError("Cannot append received binary to file \"%s\", error %i \"%s\""
			" occurred with opening the file.", DarkEdif::TStringToUTF8(filename).c_str(), errno, errtext);
		return;
	}

	size_t amountWritten;
	if ((amountWritten = fwrite(threadData->receivedMsg.content.data() + position, 1, size, File)) != size)
	{
		ErrNoToErrText();
		CreateError("Cannot append received binary to file \"%s\", error %i \"%s\""
			" occurred with writing the file. Wrote %zu bytes total.", DarkEdif::TStringToUTF8(filename).c_str(), errno, errtext, amountWritten);
		fclose(File);
		return;
	}

	if (fclose(File))
	{
		ErrNoToErrText();
		CreateError("Cannot append received binary to file \"%s\", error %i \"%s\""
			" occurred with writing the end of the file.", DarkEdif::TStringToUTF8(filename).c_str(), errno, errtext);
	}
}
void Extension::Relay_DoHolePunchToFutureClient(const TCHAR* clientIP, int localPort)
{
	if (localPort > 0xFFFF || localPort < 0)
		return CreateError("Cannot hole punch with local port %d; invalid port.", localPort);
	if (clientIP[0] == _T('\0'))
		return CreateError("Cannot hole punch with blank remote IP.");

	Srv.hole_punch(DarkEdif::TStringToANSI(clientIP).c_str(), (lw_ui16)localPort);
}
