#include "Common.hpp"

///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

Extension::Extension(RunObject* const _rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr) :
	rdPtr(_rdPtr), rhPtr(_rdPtr->get_rHo()->get_AdRunHeader()), Runtime(this)
{
	/*
		Link all your action/condition/expression functions to their IDs to match the
		IDs in the JSON here
	*/

/// ACTIONS
	LinkAction(0, DEPRECATED_TestReportAndErrors);
	LinkAction(1, DEPRECATED_UsePopupMessages);
	// Client
	LinkAction(2, DEPRECATED_ClientInitialize_Basic);
	LinkAction(3, DEPRECATED_ClientInitialize_Advanced);
	LinkAction(4, ClientCloseSocket);
	LinkAction(5, DEPRECATED_ClientSend);
	LinkAction(6, REMOVED_ClientGoIndependent);
	LinkAction(7, ClientReceiveOnly);
	LinkAction(8, DEPRECATED_ClientLinkFileOutput);
	LinkAction(9, DEPRECATED_ClientUnlinkFileOutput);
	LinkAction(10, REMOVED_ClientFusionReport);
	// Server
	LinkAction(11, ServerInitialize_Basic);
	LinkAction(12, ServerInitialize_Advanced);
	LinkAction(13, ServerShutdown);
	LinkAction(14, DEPRECATED_ServerSend);
	LinkAction(15, REMOVED_ServerGoIndependent);
	LinkAction(16, ServerAutoAccept);
	LinkAction(17, DEPRECATED_ServerLinkFileOutput);
	LinkAction(18, DEPRECATED_ServerUnlinkFileOutput);
	LinkAction(19, REMOVED_ServerFusionReport);
	// Form packet
	LinkAction(20, PacketBeingBuilt_NewPacket);
	LinkAction(21, PacketBeingBuilt_ResizePacket);
	LinkAction(22, PacketBeingBuilt_SetByte);
	LinkAction(23, PacketBeingBuilt_SetShort);
	LinkAction(24, PacketBeingBuilt_SetInteger);
	LinkAction(25, DEPRECATED_PacketBeingBuilt_SetLong);
	LinkAction(26, PacketBeingBuilt_SetFloat);
	LinkAction(27, PacketBeingBuilt_SetDouble);
	LinkAction(28, DEPRECATED_PacketBeingBuilt_SetString);
	LinkAction(29, DEPRECATED_PacketBeingBuilt_SetWString);
	LinkAction(30, DEPRECATED_PacketBeingBuilt_SetBuffer);

	// New actions as of year 2022, build 4:
	LinkAction(31, ClientInitialize_Basic);
	LinkAction(32, ClientInitialize_Advanced);
	LinkAction(33, ClientSend);
	LinkAction(34, ServerSend);
	LinkAction(35, ServerShutdownPeerSocket); // server link new action, now removed
	LinkAction(36, PacketBeingBuilt_SetInt64);
	LinkAction(37, PacketBeingBuilt_SetString);
	LinkAction(38, PacketBeingBuilt_SetBuffer);
	LinkAction(39, PendingData_DiscardBytes);



/// CONDITIONS
	LinkCondition(0, AlwaysTrue /* On error */);
	LinkCondition(1, AlwaysTrue /* On any info */);

	LinkCondition(2, SocketIDCondition /* Client connected */);
	LinkCondition(3, SocketIDCondition /* Cient disconnected */);
	LinkCondition(4, SocketIDCondition /* Client received packet */);

	LinkCondition(5, SocketIDCondition /* Server received packet */);
	LinkCondition(6, SocketIDCondition /* Server socket done */);
	LinkCondition(7, SocketIDCondition /* Peer connected to server */);
	LinkCondition(8, ServerPeerDisconnected);
	LinkCondition(9, SocketIDCondition /* Peer attempting connection */);
	LinkCondition(10, SocketIDCondition /* Server started hosting */);

/// EXPRESSIONS
	LinkExpression(0, DEPRECATED_GetErrors);
	LinkExpression(1, DEPRECATED_GetReports);
	LinkExpression(2, GetCurrent_SocketID);
	LinkExpression(3, DEPRECATED_GetLastMessageText);
	LinkExpression(4, DEPRECATED_GetLastMessageAddress);
	LinkExpression(5, GetLastReceivedData_Size);
	LinkExpression(6, GetNewSocketID);
	LinkExpression(7, DEPRECATED_GetSocketIDForLastEvent);
	LinkExpression(8, GetPortFromType);

	LinkExpression(9, DEPRECATED_PacketBeingBuilt_GetAddress);
	LinkExpression(10, PacketBeingBuilt_GetSize);
	LinkExpression(11, PacketBeingBuilt_ICMPChecksum);

	LinkExpression(12, PendingData_GetByte);
	LinkExpression(13, PendingData_GetShort);
	LinkExpression(14, PendingData_GetInteger);
	LinkExpression(15, DEPRECATED_PendingData_GetLong);
	LinkExpression(16, PendingData_GetFloat);
	LinkExpression(17, PendingData_GetDouble);
	LinkExpression(18, DEPRECATED_PendingData_GetString);
	LinkExpression(19, DEPRECATED_PendingData_GetWString);

	// New actions as of year 2022, build 4:
	LinkExpression(20, GetCurrent_PeerSocketID);
	LinkExpression(21, PacketBeingBuilt_GetAddress);
	LinkExpression(22, PendingData_GetAddress);
	LinkExpression(23, PendingData_GetUnsignedByte);
	LinkExpression(24, PendingData_GetUnsignedShort);
	LinkExpression(25, PendingData_GetInt64);
	LinkExpression(26, PendingData_GetUnsignedInt64);
	LinkExpression(27, PendingData_GetString);
	LinkExpression(28, PendingData_FindIndexOfChar);
	LinkExpression(29, PendingData_ReverseFindIndexOfChar);
	LinkExpression(30, GetCurrent_RemoteAddress);
	LinkExpression(31, GetLastReceivedData_Offset);
	LinkExpression(32, Statistics_BytesIn);
	LinkExpression(33, Statistics_BytesOut);
	LinkExpression(34, Statistics_PacketsIn);
	LinkExpression(35, Statistics_PacketsOut);
	LinkExpression(36, GetErrorOrInfo);

	isGlobal = false; // edPtr->IsPropChecked(0); // IsGlobal
	this->curEvent = std::make_shared<EventToRun>(-1, nullptr, Conditions::Unset);

#if EditorBuild
	if (edPtr->eHeader.extSize < sizeof(EDITDATA))
	{
		DarkEdif::MsgBox::Error(_T("Property size mismatch"), _T("Properties are the wrong size (MFA size %lu, extension size %zu). "
			"Please re-create the DarkSocket object in frame, "
			"and use \"Replace by another object\" in Event Editor."), edPtr->eHeader.extSize, sizeof(EDITDATA));
	}
#endif

	// This uses the Bluewing pattern
	if (isGlobal)
	{
		const std::tstring id = edPtr->Props.GetPropertyStr("GlobalID") + _T("DarkSocket"s);
		void * globalVoidPtr = Runtime.ReadGlobal(id.c_str());
		if (!globalVoidPtr)
		{
		MakeNewGlobalInfo:
			globals = new GlobalInfo(this, edPtr);
			Runtime.WriteGlobal(id.c_str(), globals);
			OutputDebugStringA(PROJECT_NAME " - Created new Globals.\n");
		}
		else // Add this Extension to extsHoldingGlobals to inherit control later
		{
			globals = (GlobalInfo *)globalVoidPtr;

			globals->threadsafe.edif_lock();

			if (globals->pendingDelete)
			{
				OutputDebugStringA(PROJECT_NAME " - Pending delete is true. Deleting.\n");
				globals->threadsafe.edif_unlock();
				delete globals;
				goto MakeNewGlobalInfo;
			}

			globals->extsHoldingGlobals.push_back(this);
			OutputDebugStringA(PROJECT_NAME " - Globals exists: added to extsHoldingGlobals.\n");

			// globals->timeoutThread is now invalid
			std::thread timeoutThread(std::move(globals->timeoutThread));
			globals->threadsafe.edif_unlock(); // can't hold it while timeout thread tries to exit

			// If timeout thread, join to wait for it
			if (timeoutThread.joinable())
			{
				OutputDebugStringA(PROJECT_NAME " - Timeout thread is active: waiting for it to close.\n");
				globals->cancelTimeoutThread = true;
				timeoutThread.join(); // Wait for end
				OutputDebugStringA(PROJECT_NAME " - Timeout thread has closed.\n");
			}
		}
	}
	else
	{
		OutputDebugStringA(PROJECT_NAME " - Non-Global object; creating Globals, not submitting to WriteGlobal.\n");
		globals = new GlobalInfo(this, edPtr);
	}
}

Extension::~Extension()
{
	globals->threadsafe.edif_lock();

	auto refIt = std::find(globals->extsHoldingGlobals.cbegin(), globals->extsHoldingGlobals.cend(), this);
	assert(refIt != globals->extsHoldingGlobals.cend() && "Couldn't find this ext in extsHoldingGlobals.");
	globals->extsHoldingGlobals.erase(refIt);

	globals->threadsafe.edif_unlock();

	// is deleted when app ends
	if (!isGlobal)
		delete globals;
}


// Runs every tick of Fusion's runtime, can be toggled off and back on
REFLAG Extension::Handle()
{
	std::size_t maxNumEvents = 10;
	for (std::size_t i = 0; i < maxNumEvents; ++i)
	{
		if (!globals->threadsafe.edif_try_lock())
			return REFLAG::NONE;

	getnext:
		if (globals->eventsToRun.empty())
		{
			globals->threadsafe.edif_unlock();
			break;
		}
		std::shared_ptr<EventToRun> cpy(std::move(globals->eventsToRun[0]));
		globals->eventsToRun.erase(globals->eventsToRun.cbegin());

		if (cpy->eventID == Conditions::CleanupSocket)
		{
			// It's safe to read from Extension without locking. Only main thread will write to Ext directly.
			auto thdIt = std::find_if(globals->socketThreadList.cbegin(), globals->socketThreadList.cend(),
				[&](const std::shared_ptr<Thread> t) {
					return t->fusionSocketID == cpy->fusionSocketID;
				}
			);
			if (thdIt != globals->socketThreadList.cend())
				globals->socketThreadList.erase(thdIt);
			goto getnext;
		}
		globals->threadsafe.edif_unlock();

		if (!cpy->msg.empty())
			cpy->source->pendingDataToRead.append(cpy->msg);

		// chance of crash here, if a generated event destroys the ext k?
		for (std::size_t k = 0; k < globals->extsHoldingGlobals.size(); ++k)
		{
			auto curEventBackup = globals->extsHoldingGlobals[k]->curEvent;
			globals->extsHoldingGlobals[k]->curEvent = cpy;
			globals->extsHoldingGlobals[k]->Runtime.GenerateEvent((int)cpy->eventID);
			globals->extsHoldingGlobals[k]->curEvent = curEventBackup;
		}
	}
	return REFLAG::NONE;
}

// These are called if there's no function linked to an ID

void Extension::UnlinkedAction(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedAction() called"), _T("Running a fallback for action ID %d. Make sure you ran LinkAction()."), ID);
}

long Extension::UnlinkedCondition(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedCondition() called"), _T("Running a fallback for condition ID %d. Make sure you ran LinkCondition()."), ID);
	return 0;
}

long Extension::UnlinkedExpression(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedExpression() called"), _T("Running a fallback for expression ID %d. Make sure you ran LinkExpression()."), ID);
	// Unlinked A/C/E is fatal error , but try not to return null string and definitely crash it
	if ((size_t)ID < Edif::SDK->ExpressionInfos.size() && Edif::SDK->ExpressionInfos[ID]->Flags.ef == ExpReturnType::String)
		return (long)Runtime.CopyString(_T(""));
	return 0;
}
