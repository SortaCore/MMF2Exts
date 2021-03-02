#include "Common.h"
#include <assert.h>
#include "MultiThreading.h"

///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

#define Ext (*globals->_ext)
#define EventsToRun globals->_eventsToRun

HANDLE AppWasClosed = NULL;
Extension::Extension(RUNDATA * _rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr) :
	rdPtr(_rdPtr), rhPtr(_rdPtr->rHo.AdRunHeader), Runtime(_rdPtr), FusionDebugger(this)
{
	// Nullify the thread-specific data
	ClearThreadData();

	/*
		Link all your action/condition/expression functions to their IDs to match the
		IDs in the JSON here
	*/
	{
		LinkAction(0, Replaced_Connect);
		LinkAction(1, Disconnect);
		LinkAction(2, SetName);
		LinkAction(3, Replaced_JoinChannel);
		LinkAction(4, LeaveChannel);
		LinkAction(5, SendTextToServer);
		LinkAction(6, SendTextToChannel);
		LinkAction(7, SendTextToPeer);
		LinkAction(8, SendNumberToServer);
		LinkAction(9, SendNumberToChannel);
		LinkAction(10, SendNumberToPeer);
		LinkAction(11, BlastTextToServer);
		LinkAction(12, BlastTextToChannel);
		LinkAction(13, BlastTextToPeer);
		LinkAction(14, BlastNumberToServer);
		LinkAction(15, BlastNumberToChannel);
		LinkAction(16, BlastNumberToPeer);
		LinkAction(17, SelectChannelWithName);
		LinkAction(18, ReplacedNoParams);
		LinkAction(19, LoopClientChannels);
		LinkAction(20, SelectPeerOnChannelByName);
		LinkAction(21, SelectPeerOnChannelByID);
		LinkAction(22, LoopPeersOnChannel);
		LinkAction(23, ReplacedNoParams);
		LinkAction(24, ReplacedNoParams);
		LinkAction(25, ReplacedNoParams);
		LinkAction(26, ReplacedNoParams);
		LinkAction(27, ReplacedNoParams);
		LinkAction(28, ReplacedNoParams);
		LinkAction(29, ReplacedNoParams);
		LinkAction(30, RequestChannelList);
		LinkAction(31, LoopListedChannels);
		LinkAction(32, ReplacedNoParams);
		LinkAction(33, ReplacedNoParams);
		LinkAction(34, ReplacedNoParams);
		LinkAction(35, SendBinaryToServer);
		LinkAction(36, SendBinaryToChannel);
		LinkAction(37, SendBinaryToPeer);
		LinkAction(38, BlastBinaryToServer);
		LinkAction(39, BlastBinaryToChannel);
		LinkAction(40, BlastBinaryToPeer);
		LinkAction(41, SendMsg_AddASCIIByte);
		LinkAction(42, SendMsg_AddByteInt);
		LinkAction(43, SendMsg_AddShort);
		LinkAction(44, SendMsg_AddInt);
		LinkAction(45, SendMsg_AddFloat);
		LinkAction(46, SendMsg_AddStringWithoutNull);
		LinkAction(47, SendMsg_AddString);
		LinkAction(48, SendMsg_AddBinaryFromAddress);
		LinkAction(49, SendMsg_Clear);
		LinkAction(50, RecvMsg_SaveToFile);
		LinkAction(51, RecvMsg_AppendToFile);
		LinkAction(52, SendMsg_AddFileToBinary);
		LinkAction(53, ReplacedNoParams);
		LinkAction(54, ReplacedNoParams);
		LinkAction(55, ReplacedNoParams);
		LinkAction(56, ReplacedNoParams);
		LinkAction(57, ReplacedNoParams);
		LinkAction(58, ReplacedNoParams);
		LinkAction(59, ReplacedNoParams);
		LinkAction(60, ReplacedNoParams);
		LinkAction(61, ReplacedNoParams);
		LinkAction(62, ReplacedNoParams);
		LinkAction(63, ReplacedNoParams);
		LinkAction(64, SelectChannelMaster);
		LinkAction(65, JoinChannel);
		LinkAction(66, SendMsg_CompressBinary);
		LinkAction(67, RecvMsg_DecompressBinary);
		LinkAction(68, RecvMsg_MoveCursor);
		LinkAction(69, LoopListedChannelsWithLoopName);
		LinkAction(70, LoopClientChannelsWithLoopName);
		LinkAction(71, LoopPeersOnChannelWithLoopName);
		LinkAction(72, ReplacedNoParams);
		LinkAction(73, Connect);
		LinkAction(74, SendMsg_Resize);
		LinkAction(75, SetDestroySetting);
	}
	{
		LinkCondition(0, AlwaysTrue /* OnError */);
		LinkCondition(1, AlwaysTrue /* OnConnect */);
		LinkCondition(2, AlwaysTrue /* OnConnectDenied */);
		LinkCondition(3, AlwaysTrue /* OnDisconnect */);
		LinkCondition(4, AlwaysTrue /* OnChannelJoin */);
		LinkCondition(5, AlwaysTrue /* OnChannelJoinDenied */);
		LinkCondition(6, AlwaysTrue /* OnNameSet */);
		LinkCondition(7, AlwaysTrue /* OnNameDenied */);
		LinkCondition(8, OnSentTextMessageFromServer);
		LinkCondition(9, OnSentTextMessageFromChannel);
		LinkCondition(10, AlwaysTrue /* OnPeerConnect */);
		LinkCondition(11, AlwaysTrue /* OnPeerDisonnect */);
		LinkCondition(12, AlwaysFalse /* Replaced_OnChannelJoin */);
		LinkCondition(13, AlwaysTrue /* OnChannelPeerLoop */);
		LinkCondition(14, AlwaysTrue /* OnClientChannelLoop */);
		LinkCondition(15, OnSentNumberMessageFromServer);
		LinkCondition(16, OnSentNumberMessageFromChannel);
		LinkCondition(17, AlwaysTrue /* OnChannelPeerLoopFinished */);
		LinkCondition(18, AlwaysTrue /* OnClientChannelLoopFinished */);
		LinkCondition(19, AlwaysFalse /* ReplacedCondNoParams */);
		LinkCondition(20, OnBlastedTextMessageFromServer);
		LinkCondition(21, OnBlastedNumberMessageFromServer);
		LinkCondition(22, OnBlastedTextMessageFromChannel);
		LinkCondition(23, OnBlastedNumberMessageFromChannel);
		LinkCondition(24, AlwaysFalse /* ReplacedCondNoParams */);
		LinkCondition(25, AlwaysFalse /* ReplacedCondNoParams */);
		LinkCondition(26, AlwaysTrue /* OnChannelListReceived */);
		LinkCondition(27, AlwaysTrue /* OnChannelListLoop */);
		LinkCondition(28, AlwaysTrue /* OnChannelListLoopFinished */);
		LinkCondition(29, AlwaysFalse /* ReplacedCondNoParams */);
		LinkCondition(30, AlwaysFalse /* ReplacedCondNoParams */);
		LinkCondition(31, AlwaysFalse /* ReplacedCondNoParams */);
		LinkCondition(32, OnSentBinaryMessageFromServer);
		LinkCondition(33, OnSentBinaryMessageFromChannel);
		LinkCondition(34, OnBlastedBinaryMessageFromServer);
		LinkCondition(35, OnBlastedBinaryMessageFromChannel);
		LinkCondition(36, OnSentTextMessageFromPeer);
		LinkCondition(37, OnSentNumberMessageFromPeer);
		LinkCondition(38, OnSentBinaryMessageFromPeer);
		LinkCondition(39, OnBlastedTextMessageFromPeer);
		LinkCondition(40, OnBlastedNumberMessageFromPeer);
		LinkCondition(41, OnBlastedBinaryMessageFromPeer);
		LinkCondition(42, IsConnected);
		LinkCondition(43, AlwaysTrue /* OnChannelLeave */);
		LinkCondition(44, AlwaysTrue /* OnChannelLeaveDenied */);
		LinkCondition(45, AlwaysTrue /* OnPeerChangedName */);
		LinkCondition(46, AlwaysFalse /* ReplacedCondNoParams */);
		LinkCondition(47, OnAnySentMessageFromServer);
		LinkCondition(48, OnAnySentMessageFromChannel);
		LinkCondition(49, OnAnySentMessageFromPeer);
		LinkCondition(50, OnAnyBlastedMessageFromServer);
		LinkCondition(51, OnAnyBlastedMessageFromChannel);
		LinkCondition(52, OnAnyBlastedMessageFromPeer);
		LinkCondition(53, AlwaysTrue /* OnNameChanged */);
		LinkCondition(54, ClientHasAName);
		LinkCondition(55, AlwaysFalse /* ReplacedCondNoParams */);
		LinkCondition(56, AlwaysFalse /* ReplacedCondNoParams */);
		LinkCondition(57, SelectedPeerIsChannelMaster);
		LinkCondition(58, YouAreChannelMaster);
		LinkCondition(59, OnChannelListLoopWithName);
		LinkCondition(60, OnChannelListLoopWithNameFinished);
		LinkCondition(61, OnPeerLoopWithName);
		LinkCondition(62, OnPeerLoopWithNameFinished);
		LinkCondition(63, OnClientChannelLoopWithName);
		LinkCondition(64, OnClientChannelLoopWithNameFinished);
		LinkCondition(65, OnSentTextChannelMessageFromServer);
		LinkCondition(66, OnSentNumberChannelMessageFromServer);
		LinkCondition(67, OnSentBinaryChannelMessageFromServer);
		LinkCondition(68, OnAnySentChannelMessageFromServer);
		LinkCondition(69, OnBlastedTextChannelMessageFromServer);
		LinkCondition(70, OnBlastedNumberChannelMessageFromServer);
		LinkCondition(71, OnBlastedBinaryChannelMessageFromServer);
		LinkCondition(72, OnAnyBlastedChannelMessageFromServer);
		LinkCondition(73, IsJoinedToChannel);
		LinkCondition(74, IsPeerOnChannel_Name);
		LinkCondition(75, IsPeerOnChannel_ID);
	}
	{
		LinkExpression(0, Error);
		LinkExpression(1, ReplacedExprNoParams);
		LinkExpression(2, Self_Name);
		LinkExpression(3, Self_ChannelCount);
		LinkExpression(4, Peer_Name);
		LinkExpression(5, RecvMsg_ReadAsString);
		LinkExpression(6, RecvMsg_ReadAsInteger);
		LinkExpression(7, RecvMsg_Subchannel);
		LinkExpression(8, Peer_ID);
		LinkExpression(9, Channel_Name);
		LinkExpression(10, Channel_PeerCount);
		LinkExpression(11, ReplacedExprNoParams);
		LinkExpression(12, ChannelListing_Name);
		LinkExpression(13, ChannelListing_PeerCount);
		LinkExpression(14, Self_ID);
		LinkExpression(15, ReplacedExprNoParams);
		LinkExpression(16, ReplacedExprNoParams);
		LinkExpression(17, ReplacedExprNoParams);
		LinkExpression(18, ReplacedExprNoParams);
		LinkExpression(19, ReplacedExprNoParams);
		LinkExpression(20, RecvMsg_StrASCIIByte);
		LinkExpression(21, RecvMsg_UnsignedByte);
		LinkExpression(22, RecvMsg_SignedByte);
		LinkExpression(23, RecvMsg_UnsignedShort);
		LinkExpression(24, RecvMsg_SignedShort);
		LinkExpression(25, RecvMsg_UnsignedInteger);
		LinkExpression(26, RecvMsg_SignedInteger);
		LinkExpression(27, RecvMsg_Float);
		LinkExpression(28, RecvMsg_StringWithSize);
		LinkExpression(29, RecvMsg_String);
		LinkExpression(30, RecvMsg_SizeInBytes);
		LinkExpression(31, Lacewing_Version);
		LinkExpression(32, SendBinaryMsg_Size);
		LinkExpression(33, Self_PreviousName);
		LinkExpression(34, Peer_PreviousName);
		LinkExpression(35, ReplacedExprNoParams);
		LinkExpression(36, ReplacedExprNoParams);
		LinkExpression(37, DenyReason);
		LinkExpression(38, Host_IP);
		LinkExpression(39, HostPort);
		LinkExpression(40, ReplacedExprNoParams);
		LinkExpression(41, WelcomeMessage);
		LinkExpression(42, RecvMsg_MemoryAddress);
		LinkExpression(43, RecvMsg_Cursor_StrASCIIByte);
		LinkExpression(44, RecvMsg_Cursor_UnsignedByte);
		LinkExpression(45, RecvMsg_Cursor_SignedByte);
		LinkExpression(46, RecvMsg_Cursor_UnsignedShort);
		LinkExpression(47, RecvMsg_Cursor_SignedShort);
		LinkExpression(48, RecvMsg_Cursor_UnsignedInteger);
		LinkExpression(49, RecvMsg_Cursor_SignedInteger);
		LinkExpression(50, RecvMsg_Cursor_Float);
		LinkExpression(51, RecvMsg_Cursor_StringWithSize);
		LinkExpression(52, RecvMsg_Cursor_String);
		LinkExpression(53, ReplacedExprNoParams);
		LinkExpression(54, SendBinaryMsg_MemoryAddress);
		LinkExpression(55, RecvMsg_DumpToString);
		LinkExpression(56, ChannelListing_ChannelCount);
		LinkExpression(57, ConvToUTF8_GetVisibleCharCount);
		LinkExpression(58, ConvToUTF8_GetCompleteCodePointCount);
		LinkExpression(59, ConvToUTF8_GetByteCount);
		LinkExpression(60, ConvToUTF8_TestAllowList);
	}

	// This is signalled by EndApp in Runtime.cpp. It's used to close the connection
	// when the application closes, from the timeout thread - assuming events or the
	// server hasn't done that already.
	if (!AppWasClosed)
		AppWasClosed = CreateEvent(NULL, TRUE, FALSE, NULL);

#if EditorBuild
	if (edPtr->eHeader.extSize < sizeof(EDITDATA))
	{
		MessageBoxA(NULL, "Properties are the wrong size. Please re-create the Lacewing Blue Client object in frame, "
			"and use \"Replace by another object\" in Event Editor.", "Lacewing Blue Client error", MB_OK | MB_ICONERROR);
	}
#endif

	isGlobal = edPtr->isGlobal;
	char msgBuff[500];
	sprintf_s(msgBuff, PROJECT_NAME " - Extension create: isGlobal=%i.\n", isGlobal ? 1 : 0);
	OutputDebugStringA(msgBuff);
	if (isGlobal)
	{
		const std::tstring id = UTF8ToTString(edPtr->edGlobalID) + _T("BlueClient"s);
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

			EnterCriticalSectionDebug(&globals->lock);

			if (globals->pendingDelete)
			{
				OutputDebugStringA(PROJECT_NAME " - Pending delete is true. Deleting.\n");
				LeaveCriticalSectionDebug(&globals->lock);
				delete globals;
				goto MakeNewGlobalInfo;
			}
			// If switching frames, the old ext will store selection here.
			// We'll keep it across frames for simplicity.
			if (!globals->lastDestroyedExtSelectedChannel.expired())
			{
				selChannel = globals->lastDestroyedExtSelectedChannel.lock();
				globals->lastDestroyedExtSelectedChannel.reset();
			}
			if (!globals->lastDestroyedExtSelectedPeer.expired())
			{
				selPeer = globals->lastDestroyedExtSelectedPeer.lock();
				globals->lastDestroyedExtSelectedPeer.reset();
			}

			globals->extsHoldingGlobals.push_back(this);
			if (!globals->_ext)
				globals->_ext = this;
			OutputDebugStringA(PROJECT_NAME " - Globals exists: added to extsHoldingGlobals.\n");

			if (globals->timeoutThread)
			{
				OutputDebugStringA(PROJECT_NAME " - Timeout thread is active: waiting for it to close.\n");
				SetEvent(AppWasClosed);
				WaitForSingleObject(globals->timeoutThread, 200);
				CloseHandle(globals->timeoutThread);
				globals->timeoutThread = NULL;
				ResetEvent(AppWasClosed);
				OutputDebugStringA(PROJECT_NAME " - Timeout thread has closed.\n");
			}

			LeaveCriticalSectionDebug(&globals->lock);
		}
	}
	else
	{
		OutputDebugStringA(PROJECT_NAME " - Non-Global object; creating Globals, not submitting to WriteGlobal.\n");
		globals = new GlobalInfo(this, edPtr);

		globals->_objEventPump->tick();
	}

	// Try to boot the Lacewing thread if multithreading and not already running
	if (edPtr->multiThreading && !globals->_thread && !(globals->_thread = CreateThread(NULL, NULL, LacewingLoopThread, this, NULL, NULL)))
	{
		CreateError("Error: failed to boot thread. Falling back to single-threaded interface.");
		Runtime.Rehandle();
	}
	else if (!edPtr->multiThreading)
		Runtime.Rehandle();

	// Set up Fusion debugger (it uses globals, so we have to do it after)
	// The client could be connected if this Extension() is being run after a Fusion frame switch.
	const auto connectedDebugItemReader = [](Extension *ext, std::tstring &writeTo) {
		if (ext->Cli.connecting())
			writeTo = _T("Connected: Connecting..."sv);
		else if (ext->Cli.connected())
			writeTo = _T("Connected: ") + ANSIToTString(ext->HostIP);
		else
			writeTo = _T("Connected: No connection"sv);
	};
	FusionDebugger.AddItemToDebugger(connectedDebugItemReader, NULL, 500, NULL);

	const auto clientNameDebugItemReader = [](Extension *ext, std::tstring &writeTo) {
		auto cliName = UTF8ToTString(ext->Cli.name());
		if (cliName.empty())
			cliName = _T("(unset)"sv);
		writeTo = _T("Name: ") + cliName;
	};
	FusionDebugger.AddItemToDebugger(clientNameDebugItemReader, NULL, 500, NULL);

	const auto channelCountDebugItemReader = [](Extension *ext, std::tstring &writeTo) {
		writeTo = _T("Channel count: ") + std::to_tstring(ext->Cli.channelcount());
	};
	FusionDebugger.AddItemToDebugger(channelCountDebugItemReader, NULL, 500, NULL);

	const auto selectedChannelDebugItemReader = [](Extension *ext, std::tstring &writeTo) {
		if (ext->selChannel)
			writeTo = _T("Selected channel: ") + UTF8ToTString(ext->selChannel->name());
		else
			writeTo = _T("Selected channel: (none)"sv);
	};
	FusionDebugger.AddItemToDebugger(selectedChannelDebugItemReader, NULL, 100, NULL);

	const auto numPeersDebugItemReader = [](Extension *ext, std::tstring &writeTo) {
		if (ext->selChannel)
			writeTo = _T("Peer count: ") + std::to_tstring(ext->selChannel->peercount());
		else
			writeTo = _T("Peer count: (no channel)"sv);
	};
	FusionDebugger.AddItemToDebugger(numPeersDebugItemReader, NULL, 100, NULL);

	const auto selectedPeerDebugItemReader = [](Extension *ext, std::tstring &writeTo) {
		if (ext->selPeer && ext->selChannel)
			writeTo = _T("Selected peer: ") + UTF8ToTString(ext->selPeer->name());
		else
			writeTo = _T("Selected peer: (none)"sv);
	};
	FusionDebugger.AddItemToDebugger(selectedPeerDebugItemReader, NULL, 100, NULL);
}

DWORD WINAPI LacewingLoopThread(void * thisExt)
{
	// If the loop thread is terminated, very few bytes of memory will be leaked.
	// However, it is better to use PostEventLoopExit().

	GlobalInfo * G = ((Extension *)thisExt)->globals;
	try {
		lacewing::error error = G->_objEventPump->start_eventloop();

		// Can't error report if there's no extension to error-report to.
		// Worst case scenario CreateError calls Runtime.Rehandle which breaks because ext is gone.
		if (!error)
			OutputDebugStringA(PROJECT_NAME " - LacewingLoopThread closing gracefully.\n");
		else if (G->_ext)
		{
			std::string text = "Error returned by StartEventLoop(): ";
			text += error->tostring();
			G->CreateError(text.c_str());
		}

	}
	catch (...)
	{
		OutputDebugStringA(PROJECT_NAME " - LacewingLoopThread got an exception.\n");
		if (G->_ext)
			G->CreateError("StartEventLoop() killed by exception. Switching to single-threaded.");
	}
	G->_thread = NULL;
	OutputDebugStringA(PROJECT_NAME " - LacewingLoopThread has exited.\n");
	return 0;
}

void GlobalInfo::AddEvent1(std::uint16_t event1ID,
	std::shared_ptr<lacewing::relayclient::channel> channel,
	std::shared_ptr<lacewing::relayclient::channellisting> channelListing,
	std::shared_ptr<lacewing::relayclient::channel::peer> peer,
	std::string_view messageOrErrorText,
	lw_ui8 subchannel, lw_ui8 variant)
{
	return AddEventF(false, event1ID, 35353, channel, channelListing, peer, messageOrErrorText, subchannel, variant);
}
void GlobalInfo::AddEvent2(std::uint16_t event1ID, std::uint16_t event2ID,
	std::shared_ptr<lacewing::relayclient::channel> channel,
	std::shared_ptr<lacewing::relayclient::channellisting> channelListing,
	std::shared_ptr<lacewing::relayclient::channel::peer> peer,
	std::string_view messageOrErrorText,
	lw_ui8 subchannel, lw_ui8 variant)
{
	return AddEventF(true, event1ID, event2ID, channel, channelListing, peer, messageOrErrorText, subchannel, variant);
}
void GlobalInfo::AddEventF(bool twoEvents, std::uint16_t event1ID, std::uint16_t event2ID,
	std::shared_ptr<lacewing::relayclient::channel> channel /* = nullptr */,
	std::shared_ptr<lacewing::relayclient::channellisting> channelListing /* = nullptr */,
	std::shared_ptr<lacewing::relayclient::channel::peer> peer /* = nullptr */,
	std::string_view messageOrErrorText /* = std::string_view() */,
	lw_ui8 subchannel /* = 255 */, lw_ui8 variant /* = 255 */)
{
	/*
		Saves all variables returned by expressions in order to ensure two conditions, triggering simultaneously,
		do not cause reading of only the last condition's output. Only used in multi-threading.

		For example, if an error event changes an output variable, and immediately afterward a
		success event changes the same variable, only the success event's output can be read since it overwrote
		the last event. However, using MMF2 in single-threaded mode, overwriting is impossible to do
		provided you use GenerateEvent(). Since PushEvent() is not immediate, you can reproduce this
		multi-threaded bug in single-thread mode. But PushEvent() isn't recommended for use anyway;
		it's designed so extensions who don't have focus of MMF2 (gained by MMF2 calling, say, Rehandle)
		can still create events.
		With GenerateEvent() + multithreading, this would cause crashes as Fusion is forced into the extension
		at the wrong time.
		With PushEvent() + multithreading, this  would cause overwriting of old events and possibly access
		violations as variables are simultaneously written to by the ext and read from by Fusion at the same time.

		But in DarkEdif, you'll note all the GenerateEvents() are handled on a queue, and the queue is
		iterated through in Handle(), thus it is quite safe. But we still need to protect potentially several
		AddEvent() functions running at once and corrupting the memory at some point; so we need the
		CRITICAL_SECTION variable mentioned in Extension.h to ensure this will not happen.
	*/

	std::shared_ptr<EventToRun> newEvent = std::make_shared<EventToRun>();
	EventToRun &newEvent2 = *newEvent;

	// Initialise
	newEvent2.numEvents = twoEvents ? 2 : 1;
	newEvent2.condTrig[0] = (unsigned short)event1ID;
	newEvent2.condTrig[1] = (unsigned short)event2ID;
	// channel/channelListing overlap, as do message content and error text
	newEvent2.channel = channel;
	newEvent2.channelListing = channelListing;
	newEvent2.peer = peer;
	newEvent2.receivedMsg.content = messageOrErrorText;
	newEvent2.receivedMsg.subchannel = subchannel;
	newEvent2.receivedMsg.variant = variant;

	EnterCriticalSectionDebug(&lock); // Needed before we access Extension
#if 0
	// Copy Extension's data to vector
	if (memcpy_s(((char *)newEvent) + 5, sizeof(EventToRun) - 5, ((char *)&_ext->ThreadData) + 5, sizeof(EventToRun) - 5))
	{
		// Failed to copy memory (error in "errno")
		// delete newEvent; // Keep it for debugging
		LeaveCriticalSectionDebug(&lock);
		throw std::exception("Memory copy failed while doing a lacewing event.");
	}
#endif
	_eventsToRun.push_back(newEvent);

	LeaveCriticalSectionDebug(&lock); // We're done accessing Extension

	// Cause Handle() to be triggered, allowing EventsToRun to be parsed

	if (_ext != nullptr)
		_ext->Runtime.Rehandle();
}

void Extension::CreateError(_Printf_format_string_ const char * error, ...)
{
	va_list v;
	va_start(v, error);
	globals->CreateError(error, v);
	va_end(v);
}

void GlobalInfo::CreateError(_Printf_format_string_ const char * error, ...)
{
	va_list v;
	va_start(v, error);
	CreateError(error, v);
	va_end(v);
}
void GlobalInfo::CreateError(_Printf_format_string_ const char * errorU8, va_list v)
{
	std::stringstream errorDetailed;
	if (std::this_thread::get_id() != mainThreadID)
		errorDetailed << "[handler] "sv;
	// Use extsHoldingGlobals[0] because Ext is (rarely) not set when an error is being made.
	else if (extsHoldingGlobals[0])
		errorDetailed << "[Fusion event #"sv << DarkEdif::GetEventNumber(extsHoldingGlobals[0]->rhPtr->EventGroup) << "] "sv;

	char output[2048];
	try {
		if (vsprintf_s(output, errorU8, v) <= 0)
		{
			errorDetailed.str("vsprintf_s failed with errno "s);
			errorDetailed << errno << ", format ["sv << errorU8 << "]."sv;
		}
		else
			errorDetailed << output;
	}
	catch (...)
	{
		errorDetailed.str("vsprintf_s failed with crash, format ["s);
		errorDetailed << errorU8 << "]."sv;
	}

#ifdef _DEBUG
	const std::string errTextU8 = errorDetailed.str();
	const std::wstring errText = UTF8ToWide(errTextU8);
	OutputDebugStringW(errText.c_str());
	OutputDebugStringW(L"\n");
#endif
	AddEvent1(0, nullptr, nullptr, nullptr, errTextU8);
}

void Extension::SendMsg_Sub_AddData(const void * data, size_t size)
{
	if (!IsValidPtr(data))
		return CreateError("Error adding to send binary: pointer %p supplied is invalid. The message has not been modified.", data);

	// Nothing to do
	if (!size)
		return;

	char * newptr = (char *)realloc(SendMsg, SendMsgSize + size);

	// Failed to reallocate memory
	if (!newptr)
		return CreateError("Received error number %u with reallocating memory to append to binary message. The message has not been modified.", errno);

	// memcpy_s does not allow copying from what's already inside SendMsg; memmove_s does.

	// If we failed to copy memory.
	if (memmove_s(newptr + SendMsgSize, size, data, size))
		return CreateError("Received error number %u with copying memory into newly allocated binary message. The message has not been modified.", errno);

	SendMsg = newptr;
	SendMsgSize += size;
}
bool Extension::IsValidPtr(const void * data)
{
	// Common error memory addresses; null pointer (0x0), uninitalized filler memory (0xCC/0xCD),
	// freed filler memory (0xDD). See https://stackoverflow.com/a/370362 .
	// I'm not expecting Bluewing to have issues with this, but other exts might pass bad memory to Blue.
	return data != 0x00000000 && (long)data != 0xCCCCCCCC && (long)data != 0xDDDDDDDD && (long)data != 0xCDCDCDCD;
}

void Extension::ClearThreadData()
{
	threadData = std::make_shared<EventToRun>();
}

std::string Extension::TStringToUTF8Simplified(std::tstring str)
{
	return lw_u8str_simplify(TStringToUTF8(str));
}

// Returns 0 if OK. -1 if cut off UTF-8 at front, 1 if cut off at end
int Extension::CheckForUTF8Cutoff(std::string_view sv)
{
	// Start char is invalid
	int res = GetNumBytesInUTF8Char(sv);

	// 0 = empty string; we can't do anything, return it.
	// -1 = UTF-8 start char, but cut off string; we can't do anything, return it.
	// -2 = UTF-8 non-start char, so start char is cut off.
	if (res <= 0)
		return res;

	// We don't know the sizeInCodePoints of end char; we'll try for a 1 byte-char at very end, and work backwards and up to max UTF-8 sizeInCodePoints, 4 bytes.
	for (int i = 0, j = (sv.size() < 4 ? sv.size() : 4); i < j; ++i)
	{
		// Cut off a char; go backwards
		res = GetNumBytesInUTF8Char(sv.substr(sv.size() - i));
		if (res == -2)
			continue;

		// Otherwise, we hit the last start char in the string

		// But it's been cut off; invalid UTF-8
		// 0 = empty string; we can't do anything, return it.
		// -1 = UTF-8 start char, but cut off string; we can't do anything, return it.
		// -2 = UTF-8 non-start char
		return res == -1 ? 1 : 0;
	}

	// Never found a start char; 5-byte/6-byte nonstandard UTF-8?
	return 1;
}
// UTF-8 uses a bitmask to determine how many bytes are in the item.
// Note that this does not verify the ending characters other than a size check; but any TString converter will throw them out.
int Extension::GetNumBytesInUTF8Char(std::string_view sv)
{
	if (sv.empty())
		return 0;

	std::uint8_t c = *(std::uint8_t *)&sv.front();
	// ASCII/UTF-8 1-byte char
	if (c <= 0x7F)
		return 1;

	// 2-byte indicator
	if (c >= 0xC2 && c <= 0xDF)
		return sv.size() >= 2 ? 2 : -1;

	// 3-byte indicator
	if (c >= 0xE0 && c <= 0xEF)
		return sv.size() >= 3 ? 3 : -1;

	// 4-byte indicator
	if (c >= 0xF0)
		return sv.size() >= 4 ? 4 : -1;

	// Non-first character in multibyte sequence; user is reading too far ahead
	if (c >= 0x80 && c <= 0xBF)
		return -2;

	// Note 5-byte and 6-byte variants are theoretically possible but were removed by UTF-8 standard.

	return -1;
}
// Called as a subfunction to read string at given position of received binary. If sizeInCodePoints is -1, will expect a null
// terminator. The isCursorExpression is used for error messages.
std::tstring Extension::RecvMsg_Sub_ReadString(size_t recvMsgStartIndex, int sizeInCodePoints, bool isCursorExpression)
{
	// User requested empty size, let 'em have it
	if (sizeInCodePoints == 0)
		return std::tstring();

	if (recvMsgStartIndex < 0)
	{
		CreateError("Could not read from received binary, index less than 0.");
		return std::tstring();
	}
	if (recvMsgStartIndex > threadData->receivedMsg.content.size())
	{
		CreateError("Could not read from received binary, index %zu is outside range of 0 to %zu.",
			recvMsgStartIndex, recvMsgStartIndex + threadData->receivedMsg.content.size());
		return std::tstring();
	}

	if (sizeInCodePoints < -1)
	{
		CreateError("Could not read string with size %d; size is too low.", sizeInCodePoints);
		return std::tstring();
	}
	const bool fixedSize = sizeInCodePoints != -1;

	size_t maxSizePlusOne = threadData->receivedMsg.content.size() - recvMsgStartIndex + 1;
	size_t actualStringSizeBytes = strnlen(threadData->receivedMsg.content.c_str() + recvMsgStartIndex, maxSizePlusOne);
	if (fixedSize)
	{
		// Size too small - we assumed every char was 1-byte for this, so it's way under
		if (sizeInCodePoints != -1 && (unsigned)sizeInCodePoints > maxSizePlusOne - 1)
		{
			CreateError("Could not read string with size %d at %hsstart index %zu, only %zu possible characters in message.",
				sizeInCodePoints, isCursorExpression ? "cursor's " : "", recvMsgStartIndex, maxSizePlusOne);
			return std::tstring();
		}

		// Null terminator found within string
		if (actualStringSizeBytes < (unsigned)sizeInCodePoints)
		{
			CreateError("Could not read string with size %d at %hsstart index %zu, found null byte within at index %zu.",
				sizeInCodePoints, isCursorExpression ? "cursor's " : "", recvMsgStartIndex, recvMsgStartIndex + actualStringSizeBytes);
			return std::tstring();
		}
	}
	// Not fixed size; if null terminator not found within remainder of text, then whoops.
	// We are expecting (actualStringSizeBytes < maxSizePlusOne - 1) if found.
	else if (actualStringSizeBytes == maxSizePlusOne - 1)
	{
		CreateError("Could not read null-terminated string from %hsstart index %zu; null terminator not found.",
			isCursorExpression ? "cursor's " : "", recvMsgStartIndex);
		return std::tstring();
	}

	// To make sure user hasn't cut off the start/end UTF-8 char, we'll do a quick check
	const std::string result = threadData->receivedMsg.content.substr(recvMsgStartIndex, actualStringSizeBytes);

	// Start char is invalid
	if (GetNumBytesInUTF8Char(result) < 0)
	{
		CreateError("Could not read text from received binary, UTF-8 char was cut off at %hsstart index %zu.",
			isCursorExpression ? "the cursor's " : "", threadData->receivedMsg.cursor);
		return std::tstring();
	}

	// We have the entire received message in result, we need to trim it to sizeInCodePoints

	// We don't know the sizeInCodePoints of end char; we'll try for a 1 byte-char at very end, and work backwards and up to max UTF-8 sizeInCodePoints, 4 bytes.
	for (int codePointIndex = 0, numBytesRead = 0, byteIndex = 0, remainder = result.size(); ; )
	{
		int numBytes = GetNumBytesInUTF8Char(result.substr(byteIndex, min(4, remainder)));

		// We checked for -2 in start char in previous if(), so the string isn't starting too early.
		// So, a -2 in middle of the string means it's a malformed UTF-8.
		// We'll catch both -1 and -2 as malformed UTF-8 errors.
		if (numBytes < 0)
			goto DeadChar;

		// otherwise, valid char
		++byteIndex;

		// loop all 2nd+ bytes used by it, validate 'em as UTF-8 continuations
		for (int i = 1; i < numBytes; ++i)
		{
			const std::uint8_t & c = reinterpret_cast<const std::uint8_t &>(result[byteIndex++]);
			if (c < 0x80 || c > 0xBF)
			{
				numBytes = -1;
				goto DeadChar;
			}
		}

		// Easier looping - note, numBytes == 0 for null terminator
		remainder -= numBytes;
		numBytesRead += numBytes;

		// Okay, we read a full character (aka a code point)
		++codePointIndex;

		// Either it's null byte-terminated and we're at the null; or got all the characters we need
		if ((!fixedSize && numBytes == 0) || (fixedSize && sizeInCodePoints == codePointIndex))
		{
			// lw_u8str_validate will do a more thorough investigation of characters, +1 for null terminator
			if (isCursorExpression)
				threadData->receivedMsg.cursor += numBytesRead + (fixedSize ? 0 : 1);

			const std::string_view resStr(result.data(), numBytesRead);
			if (lw_u8str_validate(resStr))
				return UTF8ToTString(resStr);

			CreateError("Could not read text from received binary, UTF-8 was malformed at index %u (attempted to read %d chars from %hsstart index %zu).",
				recvMsgStartIndex + byteIndex, byteIndex, isCursorExpression ? "the cursor's " : "", recvMsgStartIndex);
			return std::tstring();
		}

		// grab another character
		continue;

		// Reused error message
	DeadChar:
		CreateError("Could not read text from received binary, UTF-8 was malformed at index %u (attempted to read %d chars from %hsstart index %d).",
			recvMsgStartIndex + byteIndex, byteIndex, isCursorExpression ? "the cursor's " : "", recvMsgStartIndex);
		return std::tstring();
	}

	// Either no problems, and numBytesRead

	CreateError("Could not read text from received binary, UTF-8 char was cut off at the cursor's end index %zu.",
		threadData->receivedMsg.cursor + actualStringSizeBytes);
	return std::tstring();
}


Extension::~Extension()
{
	char msgBuff[500];
	sprintf_s(msgBuff, PROJECT_NAME " ~Extension called; extsHoldingGlobals count is %u.\n", globals->extsHoldingGlobals.size());
	OutputDebugStringA(msgBuff);

	EnterCriticalSectionDebug(&globals->lock);
	// Remove this Extension from liblacewing usage.
	auto i = std::find(globals->extsHoldingGlobals.cbegin(), globals->extsHoldingGlobals.cend(), this);
	bool wasBegin = i == globals->extsHoldingGlobals.cbegin();
	globals->extsHoldingGlobals.erase(i);

	ClearThreadData();

	// Shift secondary event management to other Extension, if any
	if (!globals->extsHoldingGlobals.empty())
	{
		OutputDebugStringA(PROJECT_NAME " - Note: Switched Lacewing instances.\n");

		// Switch Handle ticking over to next Extension visible.
		if (wasBegin)
		{
			globals->_ext = globals->extsHoldingGlobals.front();
			LeaveCriticalSectionDebug(&globals->lock);

			globals->_ext->Runtime.Rehandle();
		}
		else // This extension wasn't even the main event handler (for Handle()/globals).
		{
			LeaveCriticalSectionDebug(&globals->lock);
		}
	}
	// Last instance of this object; if global and not full-delete-enabled, do not cleanup.
	// In single-threaded instances, this will cause a dirty timeout; the lower-level protocols,
	// e.g. TCP, will close the connection after a certain amount of not-responding.
	// In multi-threaded instances, messages will continue to be queued, and this will retain
	// the connection indefinitely.
	else
	{
		if (!globals->_client.connected())
			OutputDebugStringA(PROJECT_NAME " - Not connected, nothing important to retain, closing Globals info.\n");
		else if (!isGlobal)
			OutputDebugStringA(PROJECT_NAME " - Not global, closing Globals info.\n");
		else if (globals->fullDeleteEnabled)
			OutputDebugStringA(PROJECT_NAME " - Full delete enabled, closing Globals info.\n");
		// Wait for 0ms returns immediately as per spec
		else if (WaitForSingleObject(AppWasClosed, 0U) == WAIT_OBJECT_0)
			OutputDebugStringA(PROJECT_NAME " - App was closed, closing Globals info.\n");
		else // !globals->fullDeleteEnabled
		{
			OutputDebugStringA(PROJECT_NAME " - Last instance dropped, and currently connected - "
				"Globals will be retained until a Disconnect is called.\n");
			globals->_ext = nullptr;
			globals->lastDestroyedExtSelectedChannel = selChannel;
			globals->lastDestroyedExtSelectedPeer = selPeer;
			LeaveCriticalSectionDebug(&globals->lock);

			sprintf_s(msgBuff, PROJECT_NAME " - Timeout thread started. If no instance has reclaimed ownership in 3 seconds,%s.\n",
				globals->timeoutWarningEnabled
				? "a warning message will be shown"
				: "the connection will terminate and all pending messages will be discarded");
			OutputDebugStringA(msgBuff);

			// Note the timeout thread does not delete globals. It can't, as Runtime.WriteGlobal() requires a valid Extension.
			// Instead, the thread marks it as pending delete, and in ReadGlobal in Extension ctor, it checks if it's
			// pending delete and deletes there.
			globals->timeoutThread = CreateThread(NULL, 0, ObjectDestroyTimeoutFunc, globals, NULL, NULL);
			ClearThreadData();
			selPeer = nullptr;
			selChannel = nullptr;
			return;
		}
		const std::tstring id = UTF8ToTString(globals->_globalID) + _T("BlueClient"s);
		Runtime.WriteGlobal(id.c_str(), nullptr);
		LeaveCriticalSectionDebug(&globals->lock);
		delete globals; // Disconnects and closes event pump, deletes lock
		globals = nullptr;
	}

	selPeer = nullptr;
	selChannel = nullptr;
}

REFLAG Extension::Handle()
{
	// If thread is not working, use Tick functionality. This may add events, so do it before the event-loop check.
	if (!globals->_thread)
	{
		lacewing::error e = ObjEventPump->tick();
		if (e != nullptr)
		{
			e->add("(in Extension::Handle -> tick())");
			CreateError(e->tostring());
			return REFLAG::NONE; // Run next loop
		}
	}

	// AddEvent() was called and not yet handled
	// (note all code that accesses EventsToRun must have ownership of lock)


	// If Thread is not available, we have to tick() on Handle(), so
	// we have to run next loop even if there's no events in EventsToRun to deal with.
	bool runNextLoop = !globals->_thread;
	size_t remainingCount = 0;
	constexpr size_t maxNumEventsPerEventLoop = 10;

	for (size_t maxTrig = 0; maxTrig < maxNumEventsPerEventLoop; maxTrig++)
	{
		// Attempt to Enter, break if we can't get it instantly
		if (!TryEnterCriticalSection(&globals->lock))
		{
			runNextLoop = true;
			break; // lock already occupied; leave it and run next event loop
		}
		// At this point we have effectively run EnterCriticalSection
#ifdef _DEBUG
		::CriticalSection << "Thread "sv << GetCurrentThreadId() << " : Entered on "sv
			<< __FILE__ << ", line "sv << __LINE__ << ".\r\n"sv;
#endif

		if (EventsToRun.empty())
		{
			LeaveCriticalSectionDebug(&globals->lock);
			isOverloadWarningQueued = false;
			break;
		}
		std::shared_ptr<EventToRun> s = EventsToRun.front();
		EventsToRun.erase(EventsToRun.begin());
		remainingCount = EventsToRun.size();

		LeaveCriticalSectionDebug(&globals->lock);


		for (auto i : globals->extsHoldingGlobals)
		{
			// Trigger all stored events (more than one may be stored by calling AddEvent(***, true) )
			for (std::uint8_t u = 0; u < s->numEvents; ++u)
			{
				if (s->condTrig[u] != 0xFFFF)
				{
					auto origSelChannel = selChannel;
					auto origSelPeer = selPeer;
					auto origTData = i->threadData; // may not be needed

					i->threadData = s;
					i->selChannel = s->channel;
					i->selPeer = s->peer;

					i->Runtime.GenerateEvent((int)s->condTrig[u]);

					// Restore old selection - if there was a selection
					i->threadData = origTData;
					if (origSelChannel)
						selChannel = origSelChannel;
					if (origSelPeer)
						selPeer = origSelPeer;
				}
				// Remove copies if this particular event number is used
				else
				{
					// On disconnect, clear everyting
					if (!s->channel)
					{
						// After On Disconnect is triggered (cond ID 3), 0xFFFF is triggered.
						// Invalidate the cached server's host IP, old prev name, and old deny reason.
						HostIP.clear();
						PreviousName.clear();
						DenyReasonBuffer.clear();
					}

					// No channel: full clear of all channels/peers
					// Channel and no peer: this client leaving channel
					if (!s->channel || (s->channel && !s->peer))
					{
						for (auto& dropExt : globals->extsHoldingGlobals)
						{
							if (!s->channel || dropExt->selChannel == s->channel)
							{
								dropExt->selChannel = nullptr;
								dropExt->selPeer = nullptr;
							}
						}
					}

					// No channel: full clear of all channels/peers
					// Channel and peer: this peer leaving channel
					if (!s->channel || (s->channel && s->peer))
						for (auto& dropExt : globals->extsHoldingGlobals)
							if (!s->channel || dropExt->selPeer == s->peer)
								dropExt->selPeer = nullptr;
				}
			}
		}
	}

	if (!isOverloadWarningQueued && remainingCount > maxNumEventsPerEventLoop * 3)
	{
		EnterCriticalSectionDebug(&globals->lock);
		char error[300];
		sprintf_s(error, "You're receiving too many messages for the application to process. Max of "
			"%u events per event loop, currently %u messages in queue.",
			maxNumEventsPerEventLoop, EventsToRun.size());

		// Create an error and move it to the front of the queue
		CreateError(error);
		auto errEvt = EventsToRun.back();
		EventsToRun.erase(--EventsToRun.cend());
		EventsToRun.insert(EventsToRun.cbegin(), errEvt);
		isOverloadWarningQueued = true;

		LeaveCriticalSectionDebug(&globals->lock);
	}

	// Will not be called next loop if runNextLoop is false
	return runNextLoop ? REFLAG::NONE : REFLAG::ONE_SHOT;
}

DWORD WINAPI ObjectDestroyTimeoutFunc(void * ThisGlobalsInfo)
{
	OutputDebugStringA(PROJECT_NAME " - timeout thread: startup.\n");

	GlobalInfo* G = (GlobalInfo *)ThisGlobalsInfo;

	// If the user has created a new object which is receiving events from Bluewing
	// it's cool, just close silently
	if (!G->extsHoldingGlobals.empty())
		return OutputDebugStringA(PROJECT_NAME " - timeout thread: pre timeout refs not empty, exiting.\n"), 0U;

	// If disconnected, no connection to worry about
	if (!G->_client.connected())
		return OutputDebugStringA(PROJECT_NAME " - timeout thread: pre timeout client not connected, exiting.\n"), 0U;

	// App closed within next 3 seconds: close connection by default.
	// This is also triggered by main thread after a frame switch finishes, to kick the timeout thread
	// out of the wait early. So app could be closed, or there's a new Ext to take over; we'll check.

	bool appCloseHandleTriggered = false;
	if (WaitForSingleObject(AppWasClosed, 3000U) == WAIT_OBJECT_0)
	{
		OutputDebugStringA(PROJECT_NAME " - timeout thread: waitforsingleobject triggered, may be app exit or new ext available.\n");
		appCloseHandleTriggered = true;
	}

	EnterCriticalSectionDebug(&G->lock);

	// 3 seconds have passed: if we now have an ext, or client was disconnected, we're good
	if (!G->extsHoldingGlobals.empty())
	{
		LeaveCriticalSectionDebug(&G->lock);
		return OutputDebugStringA(PROJECT_NAME " - timeout thread: post timeout refs not empty, exiting.\n"), 0U;
	}

	if (!G->_client.connected())
	{
		OutputDebugStringA(PROJECT_NAME " - timeout thread: post timeout client not connected, killing globals safely.\n"), 0U;
		goto killGlobals;
	}

	if (!appCloseHandleTriggered && G->timeoutWarningEnabled)
	{
		OutputDebugStringA(PROJECT_NAME " - timeout thread: timeout warning message.\n");
		// Otherwise, fuss at them.
		MessageBoxA(NULL, "Bluewing Client warning!\r\n"
			"All Bluewing Client objects have been destroyed and some time has passed; but "
			"the connection has been left open in the background, unused, but still connected.\r\n"
			"If you want to close the connection if no Bluewing Client objects are around, use the Disconnect action or "
			"disable the timeout warning in the Bluewing Client object properties.\r\n",
			"Bluewing Client Warning",
			MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_TOPMOST);
	}

	killGlobals:
	OutputDebugStringA(PROJECT_NAME " - timeout thread: Faux-deleting globals.\n");

	// Don't delete GlobalInfo, as we don't know what main thread is doing,
	// and we can't destroy GlobalInfo in case a thread is waiting to use it

	if (!appCloseHandleTriggered)
		G->MarkAsPendingDelete();
	LeaveCriticalSectionDebug(&G->lock);

	// App was closed, we can completely delete the memory
	if (appCloseHandleTriggered)
		delete G;

	OutputDebugStringA(PROJECT_NAME " - timeout thread: Globals faux-deleted, closing timeout thread.\n");
	return 0U;
}

REFLAG Extension::Display()
{
	/*
		If you return REFLAG::DISPLAY in Handle() this routine will run.
	*/

	// Ok
	return REFLAG::NONE;
}

short Extension::Pause()
{

	// Ok
	return 0;
}

short Extension::Continue()
{

	// Ok
	return 0;
}

bool Extension::Save(HANDLE File)
{
	bool OK = false;

	#ifndef VITALIZE

		// Save the object's data here

		OK = true;

	#endif

	return OK;
}

bool Extension::Load(HANDLE File)
{
	bool OK = false;

	#ifndef VITALIZE

		// Load the object's data here

		OK = true;

	#endif

	return OK;
}


// These are called if there's no function linked to an ID

void Extension::Action(int ID, RUNDATA * rdPtr, long param1, long param2)
{

}

long Extension::Condition(int ID, RUNDATA * rdPtr, long param1, long param2)
{
	return false;
}

long Extension::Expression(int ID, RUNDATA * rdPtr, long param)
{
	return 0;
}

GlobalInfo::GlobalInfo(Extension * e, EDITDATA * edPtr)
	: _objEventPump(lacewing::eventpump_new(), eventpumpdeleter),
	_client(_objEventPump.get()),
	_sendMsg(nullptr), _sendMsgSize(0),
	_automaticallyClearBinary(edPtr->automaticClear), _thread(nullptr),
	lastDestroyedExtSelectedChannel(), lastDestroyedExtSelectedPeer()
{
	// GlobalInfos are always created by Extension, which are always created by Fusion main thread
	mainThreadID = std::this_thread::get_id();

	_ext = e;
	extsHoldingGlobals.push_back(e);
	if (edPtr->isGlobal)
		_globalID = edPtr->edGlobalID;
	timeoutWarningEnabled = edPtr->timeoutWarningEnabled;
	fullDeleteEnabled = edPtr->fullDeleteEnabled;

	_client.onchannellistreceived(::OnChannelListReceived);
	_client.onmessage_channel(::OnChannelMessage);
	_client.onconnect(::OnConnect);
	_client.onconnectiondenied(::OnConnectDenied);
	_client.ondisconnect(::OnDisconnect);
	_client.onerror(::OnError);
	_client.onchannel_join(::OnJoinChannel);
	_client.onchannel_joindenied(::OnJoinChannelDenied);
	_client.onchannel_leave(::OnLeaveChannel);
	_client.onchannel_leavedenied(::OnLeaveChannelDenied);
	_client.onname_changed(::OnNameChanged);
	_client.onname_denied(::OnNameDenied);
	_client.onname_set(::OnNameSet);
	_client.onpeer_changename(::OnPeerNameChanged);
	_client.onpeer_connect(::OnPeerConnect);
	_client.onpeer_disconnect(::OnPeerDisconnect);
	_client.onmessage_peer(::OnPeerMessage);
	_client.onmessage_serverchannel(::OnServerChannelMessage);
	_client.onmessage_server(::OnServerMessage);

	InitializeCriticalSection(&lock);

	// Useful so Lacewing callbacks can access Extension
	_client.tag = this;
}

GlobalInfo::~GlobalInfo() noexcept(false)
{
	if (!extsHoldingGlobals.empty())
		throw std::exception("GlobalInfo dtor called prematurely.");

	if (!pendingDelete)
		MarkAsPendingDelete();

	// Holders trying to use MarkAsPendingDelete() secure themselves with this lock,
	// so we don't delete in the MarkAsPendingDelete() but in dtor only
	DeleteCriticalSection(&lock);
}
void GlobalInfo::MarkAsPendingDelete()
{
	if (pendingDelete)
		return;

	// Let the memory be freed later by _client dtor.
	this->lastDestroyedExtSelectedChannel.reset();
	this->lastDestroyedExtSelectedPeer.reset();

	auto clientWriteLock = _client.lock.createWriteLock();

	// We're no longer responding to these events
	_client.onchannellistreceived(nullptr);
	_client.onmessage_channel(nullptr);
	_client.onconnect(nullptr);
	_client.onconnectiondenied(nullptr);
	_client.ondisconnect(nullptr);
	_client.onerror(nullptr);
	_client.onchannel_join(nullptr);
	_client.onchannel_joindenied(nullptr);
	_client.onchannel_leave(nullptr);
	_client.onchannel_leavedenied(nullptr);
	_client.onname_changed(nullptr);
	_client.onname_denied(nullptr);
	_client.onname_set(nullptr);
	_client.onpeer_changename(nullptr);
	_client.onpeer_connect(nullptr);
	_client.onpeer_disconnect(nullptr);
	_client.onmessage_peer(nullptr);
	_client.onmessage_serverchannel(nullptr);
	_client.onmessage_server(nullptr);
	_client.tag = nullptr; // was == this, now this is not usable

	// Cleanup all usages of GlobalInfo
	if (!_thread)
		_objEventPump->tick();

	if (_client.connected() || _client.connecting())
	{
		_client.disconnect();

		if (!_thread)
			_objEventPump->tick();
		Sleep(0U);
	}

	_objEventPump->post_eventloop_exit();

	// Multithreading mode; wait for thread to end
	auto threadHandle = _thread;
	if (_thread)
	{
		Sleep(0U);
		if (WaitForSingleObject(threadHandle, 5000U) != WAIT_OBJECT_0 && _thread)
		{
			TerminateThread(_thread, 0U);
			_thread = NULL;
		}
	}
	else // single-threaded; tick so all pending events are parsed, like the eventloop exit
	{
		lacewing::error err = _objEventPump->tick();
		if (err != NULL)
		{
			// No way to report it; the last ext is being destroyed.
			std::stringstream errStr;
			errStr << PROJECT_NAME " - Pump closed with error \"" << err->tostring() << "\".\n";
			OutputDebugStringA(errStr.str().c_str());
		}
		OutputDebugStringA(PROJECT_NAME " - Pump should be closed.\n");
		Sleep(0U);
	}

	OutputDebugStringA(PROJECT_NAME " - ~GlobalInfo end\n");
}

void eventpumpdeleter(lacewing::eventpump pump)
{
	OutputDebugStringA(PROJECT_NAME " - Pump deleting...\n");
	lacewing::pump_delete(pump);
	OutputDebugStringA(PROJECT_NAME " - Pump deleted.\n");
	_CrtCheckMemory();
}
