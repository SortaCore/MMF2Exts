#include "Common.h"
#include <assert.h>

///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

#define Ext (*globals->_ext)
#define Saved globals->_Saved

HANDLE AppWasClosed = NULL;
Extension::Extension(RUNDATA * _rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr) :
	rdPtr(_rdPtr), rhPtr(_rdPtr->rHo.AdRunHeader), Runtime(_rdPtr)
{
	// Nullify the thread-specific data
	ClearThreadData();

	/*
        Link all your action/condition/expression functions to their IDs to match the
        IDs in the JSON here
    */
	{
		LinkAction(0, RelayServer_Host);
		LinkAction(1, RelayServer_StopHosting);
		LinkAction(2, SetWelcomeMessage);
		LinkAction(3, RemovedActionNoParams);
		LinkAction(4, RemovedActionNoParams);
		LinkAction(5, RemovedActionNoParams);
		LinkAction(6, RemovedActionNoParams);
		LinkAction(7, RemovedActionNoParams);
		LinkAction(8, RemovedActionNoParams);
		LinkAction(9, RemovedActionNoParams /* EnableCondition_OnMessageToChannel */);
		LinkAction(10, RemovedActionNoParams);
		LinkAction(11, RemovedActionNoParams /*EnableCondition_OnMessageToPeer */);
		LinkAction(12, RemovedActionNoParams);
		LinkAction(13, RemovedActionNoParams);
		LinkAction(14, RemovedActionNoParams);
		LinkAction(15, RemovedActionNoParams);
		LinkAction(16, RemovedActionNoParams);
		LinkAction(17, RemovedActionNoParams);
		LinkAction(18, RemovedActionNoParams);
		LinkAction(19, RemovedActionNoParams /* old deny action without reason */);
		LinkAction(20, OnInteractive_ChangeClientName);
		LinkAction(21, OnInteractive_ChangeChannelName);
		LinkAction(22, OnInteractive_ReplaceMessageWithText);
		LinkAction(23, OnInteractive_ReplaceMessageWithNumber);
		LinkAction(24, OnInteractive_ReplaceMessageWithSendBinary);
		LinkAction(25, Channel_Close);
		LinkAction(26, Channel_SelectMaster);
		LinkAction(27, Channel_SelectByName);
		LinkAction(28, LoopAllChannels);
		LinkAction(29, Client_Disconnect);
		LinkAction(30, Client_LoopJoinedChannels);
		LinkAction(31, Client_SelectByName);
		LinkAction(32, Client_SelectByID);
		LinkAction(33, LoopAllClients);
		LinkAction(34, SendTextToClient);
		LinkAction(35, SendTextToChannel);
		LinkAction(36, SendNumberToClient);
		LinkAction(37, SendNumberToChannel);
		LinkAction(38, SendBinaryToClient);
		LinkAction(39, SendBinaryToChannel);
		LinkAction(40, BlastTextToClient);
		LinkAction(41, BlastTextToChannel);
		LinkAction(42, BlastNumberToClient);
		LinkAction(43, BlastNumberToChannel);
		LinkAction(44, BlastBinaryToClient);
		LinkAction(45, BlastBinaryToChannel);
		LinkAction(46, AddByteText);
		LinkAction(47, AddByteInt);
		LinkAction(48, AddShort);
		LinkAction(49, AddInt);
		LinkAction(50, AddFloat);
		LinkAction(51, AddStringWithoutNull);
		LinkAction(52, AddString);
		LinkAction(53, AddBinary);
		LinkAction(54, AddFileToBinary);
		LinkAction(55, CompressSendBinary);
		LinkAction(56, ClearBinaryToSend);
		LinkAction(57, SaveReceivedBinaryToFile);
		LinkAction(58, AppendReceivedBinaryToFile);
		LinkAction(59, DecompressReceivedBinary);
		LinkAction(60, Channel_LoopClients);
		LinkAction(61, OnInteractive_DropMessage);
		LinkAction(62, Client_SelectSender);
		LinkAction(63, Client_SelectReceiver);
		LinkAction(64, LoopAllChannelsWithName);
		LinkAction(65, LoopAllClientsWithName);
		LinkAction(66, Client_LoopJoinedChannelsWithName);
		LinkAction(67, Channel_LoopClientsWithName);
		LinkAction(68, FlashServer_Host);
		LinkAction(69, FlashServer_StopHosting);
		LinkAction(70, Client_SetLocalData);
		LinkAction(71, MoveReceivedBinaryCursor);
		LinkAction(72, Channel_SetLocalData);
		LinkAction(73, ResizeBinaryToSend);
		// Added actions:
		LinkAction(74, EnableCondition_OnConnectRequest);
		LinkAction(75, EnableCondition_OnJoinChannelRequest);
		LinkAction(76, EnableCondition_OnLeaveChannelRequest);
		LinkAction(77, EnableCondition_OnMessageToChannel);
		LinkAction(78, EnableCondition_OnMessageToPeer);
		LinkAction(79, EnableCondition_OnMessageToServer);
		LinkAction(80, OnInteractive_Deny);
		LinkAction(81, EnableCondition_OnNameSetRequest);
		//LinkAction(82, HTML5Server_EnableHosting);
		//LinkAction(83, HTML5Server_DisableHosting);
	}
	{
		LinkCondition(0, AlwaysTrue /* OnError */);
		LinkCondition(1, AlwaysTrue /* OnClientConnectRequest */);
		LinkCondition(2, AlwaysTrue /* OnDisconnect */);
		LinkCondition(3, AlwaysTrue /* OnChannel_JoinRequest */);
		LinkCondition(4, AlwaysTrue /* OnChannel_LeaveRequest */);
		LinkCondition(5, AlwaysTrue /* AllChannelsLoop */);
		LinkCondition(6, AlwaysTrue /* OnClient_JoinedChannelsLoop */);
		LinkCondition(7, AlwaysTrue /* AllClientsLoop */);
		LinkCondition(8, AlwaysTrue /* OnChannel_ClientLoop */);
		LinkCondition(9, Client_IsChannelMaster);
		LinkCondition(10, AlwaysTrue /* OnClient_NameSetRequest */);
		LinkCondition(11, AlwaysFalse /* NOT USED: OnClient_NameChangeRequest */);
		LinkCondition(12, SubchannelMatches /* OnSentTextMessageToServer */);
		LinkCondition(13, SubchannelMatches /* OnSentNumberMessageToServer */);
		LinkCondition(14, SubchannelMatches /* OnSentBinaryMessageToServer */);
		LinkCondition(15, SubchannelMatches /* OnAnySentMessageToServer */);
		LinkCondition(16, SubchannelMatches /* OnSentTextMessageToChannel */);
		LinkCondition(17, SubchannelMatches /* OnSentNumberMessageToChannel */);
		LinkCondition(18, SubchannelMatches /* OnSentBinaryMessageToChannel */);
		LinkCondition(19, SubchannelMatches /* OnAnySentMessageToChannel */);
		LinkCondition(20, SubchannelMatches /* OnSentTextMessageToPeer */);
		LinkCondition(21, SubchannelMatches /* OnSentNumberMessageToPeer */);
		LinkCondition(22, SubchannelMatches /* OnSentBinaryMessageToPeer */);
		LinkCondition(23, SubchannelMatches /* OnAnySentMessageToPeer */);
		LinkCondition(24, SubchannelMatches /* OnBlastedTextMessageToPeer */);
		LinkCondition(25, SubchannelMatches /* OnBlastedNumberMessageToServer */);
		LinkCondition(26, SubchannelMatches /* OnBlastedBinaryMessageToServer */);
		LinkCondition(27, SubchannelMatches /* OnAnyBlastedMessageToServer */);
		LinkCondition(28, SubchannelMatches /* OnBlastedTextMessageToChannel */);
		LinkCondition(29, SubchannelMatches /* OnBlastedNumberMessageToChannel */);
		LinkCondition(30, SubchannelMatches /* OnBlastedBinaryMessageToChannel */);
		LinkCondition(31, SubchannelMatches /* OnAnyBlastedMessageToChannel */);
		LinkCondition(32, SubchannelMatches /* OnBlastedTextMessageToPeer */);
		LinkCondition(33, SubchannelMatches /* OnBlastedNumberMessageToPeer */);
		LinkCondition(34, SubchannelMatches /* OnBlastedBinaryMessageToPeer */);
		LinkCondition(35, SubchannelMatches /* OnAnyBlastedMessageToPeer */);
		LinkCondition(36, OnAllChannelsLoopWithName);
		LinkCondition(37, OnClientsJoinedChannelLoopWithName);
		LinkCondition(38, OnAllClientsLoopWithName);
		LinkCondition(39, OnChannelClientsLoopWithName);
		LinkCondition(40, OnChannelClientsLoopWithNameFinished);
		LinkCondition(41, OnAllChannelsLoopWithNameFinished);
		LinkCondition(42, OnAllClientsLoopWithNameFinished);
		LinkCondition(43, OnClientsJoinedChannelLoopWithNameFinished);
		LinkCondition(44, AlwaysTrue /* OnChannelsClientLoopFinished */);
		LinkCondition(45, AlwaysTrue /* OnAllChannelsLoopFinished */);
		LinkCondition(46, AlwaysTrue /* OnAllClientsLoopFinished */);
		LinkCondition(47, AlwaysTrue /* OnClientJoinedChannelLoopFinished */);
		LinkCondition(48, IsLacewingServerHosting);
		LinkCondition(49, IsFlashPolicyServerHosting);
		LinkCondition(50, ChannelIsHiddenFromChannelList);
		LinkCondition(51, ChannelIsSetToCloseAutomatically);
		LinkCondition(52, AlwaysFalseWithTextParam /* DUPLICATE IN ORIG, NOT USED IN BLUE: OnAllClientsLoopWithNameFinished */);
		// Added conditions:
		LinkCondition(53, IsClientOnChannel_ID);
		LinkCondition(54, IsClientOnChannel_Name);
	//	LinkCondition(55, IsHTML5Hosting);
	}
	{
		LinkExpression(0, error);
		LinkExpression(1, Lacewing_Version);
		LinkExpression(2, BinaryToSend_Size);
		LinkExpression(3, RequestedClientName);
		LinkExpression(4, RequestedChannelName);
		LinkExpression(5, Channel_Name);
		LinkExpression(6, Channel_ClientCount);
		LinkExpression(7, Client_Name);
		LinkExpression(8, Client_ID);
		LinkExpression(9, Client_IP);
		LinkExpression(10, Client_ConnectionTime);
		LinkExpression(11, Client_ChannelCount);
		LinkExpression(12, ReceivedStr);
		LinkExpression(13, ReceivedInt);
		LinkExpression(14, ReceivedBinarySize);
		LinkExpression(15, ReceivedBinaryAddress);
		LinkExpression(16, StrByte);
		LinkExpression(17, UnsignedByte);
		LinkExpression(18, SignedByte);
		LinkExpression(19, UnsignedShort);
		LinkExpression(20, SignedShort);
		LinkExpression(21, UnsignedInteger);
		LinkExpression(22, SignedInteger);
		LinkExpression(23, Float);
		LinkExpression(24, StringWithSize);
		LinkExpression(25, String);
		LinkExpression(26, Subchannel);
		LinkExpression(27, Channel_Count);
		LinkExpression(28, Client_GetLocalData);
		LinkExpression(29, CursorStrByte);
		LinkExpression(30, CursorUnsignedByte);
		LinkExpression(31, CursorSignedByte);
		LinkExpression(32, CursorUnsignedShort);
		LinkExpression(33, CursorSignedShort);
		LinkExpression(34, CursorUnsignedInteger);
		LinkExpression(35, CursorSignedInteger);
		LinkExpression(36, CursorFloat);
		LinkExpression(37, CursorStringWithSize);
		LinkExpression(38, CursorString);
		LinkExpression(39, Client_ProtocolImplementation);
		LinkExpression(40, Channel_GetLocalData);
		LinkExpression(41, Port);
		LinkExpression(42, BinaryToSend_Address);
		// Added expressions:
		LinkExpression(43, DumpMessage);
		LinkExpression(44, AllClientCount);
		LinkExpression(45, GetDenyReason);
	}
    
	/*
        This is where you'd do anything you'd do in CreateRunObject in the original SDK

        It's the only place you'll get access to edPtr at runtime, so you should transfer
        anything from edPtr to the extension class here.
    */

	// This is signalled by EndApp in General.cpp. It's used to unhost the server
	// when the application closes, from the timeout thread - assuming events
	// haven't done that already.
	if (!AppWasClosed)
		AppWasClosed = CreateEvent(NULL, TRUE, FALSE, NULL);

	IsGlobal = edPtr->Global;
	char msgBuff[500];
	sprintf_s(msgBuff, "Extension create: IsGlobal=%i.\n", IsGlobal ? 1 : 0);
	OutputDebugStringA(msgBuff);
	if (IsGlobal)
	{
		char * GlobalIDCopy = _strdup(edPtr->edGlobalID);
		void * v = Runtime.ReadGlobal(std::string(std::string("LacewingRelayServer") + GlobalIDCopy).c_str());
		if (!v)
		{
			globals = new GlobalInfo(this, edPtr);
			Runtime.WriteGlobal(std::string(std::string("LacewingRelayServer") + GlobalIDCopy).c_str(), globals);
			OutputDebugStringA("Created new globals.");
		}
		else
		{
			// Add this Extension to Refs
			globals = (GlobalInfo *)v;
			
			// If switching frames, the old ext will store selection here.
			// We'll keep it across frames for simplicity.
			threadData.channel = globals->LastDestroyedExtSelectedChannel;
			threadData.client = globals->LastDestroyedExtSelectedClient;

			globals->Refs.push_back(this);
			if (!globals->_ext)
				globals->_ext = this;
			OutputDebugStringA("globals exists: added to Refs.");
		}
	}
	else
	{
		OutputDebugStringA("Non-Global object; creating globals, not submitting to WriteGlobal.");
		globals = new GlobalInfo(this, edPtr);

		globals->_objEventPump->tick();
	}

#ifndef _DEBUG
	if (edPtr->MultiThreading)
	{
		MessageBoxA(NULL, "For stability reasons, Bluewing Server does not support multithreading. "
			"Please use the \"Enable conditions\" menu appropriately for optimum speed. See the help file.",
			"Lacewing Blue Server - Startup warning", MB_OK | MB_ICONWARNING);
		edPtr->MultiThreading = false;
		Runtime.Rehandle();
	}
#endif


	// Try to boot the Lacewing thread if multithreading and not already running
	if (edPtr->MultiThreading && !globals->_thread && !(globals->_thread = CreateThread(NULL, NULL, LacewingLoopThread, this, NULL, NULL)))
	{
		CreateError("Error: failed to boot thread. Falling back to single-threaded interface.");
		Runtime.Rehandle();
	}
	else if (!edPtr->MultiThreading)
		Runtime.Rehandle();
}



DWORD WINAPI LacewingLoopThread(void * ThisExt)
{
	// If the loop thread is terminated, very few bytes of memory will be leaked.
	// However, it is better to use PostEventLoopExit().

	GlobalInfo * g = ((Extension *)ThisExt)->globals;
	try {
		lacewing::error error = g->_objEventPump->start_eventloop();

		// Can't error report if there's no extension to error-report to.
		// Worst case scenario CreateError calls Runtime.Rehandle which breaks because ext is gone.
		if (!error)
			OutputDebugStringA("LacewingLoopThread closing gracefully.");
		else if (g->_ext)
		{
			std::string Text = "Error returned by StartEventLoop(): ";
			Text += error->tostring();
			g->CreateError(Text.c_str());
		}

	}
	catch (...)
	{
		OutputDebugStringA("LacewingLoopThread got an exception.");
		if (g->_ext)
		{
			std::string Text = "StartEventLoop() killed by exception. Switching to single-threaded.";
			g->CreateError(Text.c_str());
		}
	}
	g->_thread = NULL;
	OutputDebugStringA("LacewingLoopThread has exited.");
	return 0;
}

GlobalInfo::GlobalInfo(Extension * e, EDITDATA * edPtr)
	: _objEventPump((lacewing::eventpump)lacewing::eventpump_new()),
	_server(_objEventPump),
	_sendMsg(nullptr), _sendMsgSize(0), _denyReason(nullptr), _newChannelName(nullptr),
	_newClientName(nullptr), _interactivePending(InteractiveType::None),
	_automaticallyClearBinary(edPtr->AutomaticClear), _globalID(nullptr), _thread(nullptr),
	LastDestroyedExtSelectedChannel(nullptr), LastDestroyedExtSelectedClient(nullptr),
	AutoResponse_Connect(Approve_Quiet), AutoResponse_Connect_DenyReason(nullptr),
	AutoResponse_NameSet(Approve_Quiet), AutoResponse_NameSet_DenyReason(nullptr),
	AutoResponse_ChannelJoin(Approve_Quiet), AutoResponse_ChannelJoin_DenyReason(nullptr),
	autoResponse_ChannelLeave(Approve_Quiet), AutoResponse_ChannelLeave_DenyReason(nullptr),
	AutoResponse_MessageServer(WaitForFusion),
	AutoResponse_MessageChannel(Approve_Quiet), AutoResponse_MessageClient(Approve_Quiet)
{
	_ext = e;
	Refs.push_back(e);
	if (edPtr->Global)
		_globalID = _strdup(edPtr->edGlobalID);
	TimeoutWarningEnabled = edPtr->TimeoutWarningEnabled;
	FullDeleteEnabled = edPtr->FullDeleteEnabled;

	_server.onerror(::OnError);
	_server.onconnect(::OnClientConnectRequest);
	_server.onnameset(::OnNameSetRequest);
	_server.onchannel_join(::OnJoinChannelRequest);
	_server.onchannel_leave(::OnLeaveChannelRequest);
	_server.onmessage_server(::OnServerMessage);
	_server.ondisconnect(::OnClientDisconnect);

	// Approve quiet needs no handler
	//_server.onmessage_channel(::OnChannelMessage);
	//_server.onmessage_peer(::OnPeerMessage);
	
	// RelayServer combines flash policy errors with regular errors.
	// So this is handled by srv.onerror
	//flashSrv->on_error(::OnFlashError);

	InitializeCriticalSection(&lock);

	// Useful so Lacewing callbacks can access Extension
	_server.tag = this;
}
GlobalInfo::~GlobalInfo() noexcept(false)
{
	if (!Refs.empty())
		throw std::exception("GlobalInfo dtor called prematurely.");
	free(_globalID);
	DeleteCriticalSection(&lock);

	// We're no longer responding to these events
	_server.onerror(nullptr);
	_server.ondisconnect(nullptr);
	_server.onconnect(nullptr);
	_server.onnameset(nullptr);
	_server.onchannel_join(nullptr);
	_server.onchannel_leave(nullptr);
	_server.onmessage_server(nullptr);
	_server.onmessage_channel(nullptr);
	_server.onmessage_peer(nullptr);

	// Cleanup all usages of GlobalInfo
	if (!_thread)
		_objEventPump->tick();

	_server.tag = nullptr; // was == this, now this is not usable

	if (_server.hosting())
	{
		_server.unhost();

		if (!_thread)
			_objEventPump->tick();
		Sleep(0U);
	}
	_objEventPump->post_eventloop_exit();

	if (_thread)
	{
		Sleep(0U);
		if (_thread)
			Sleep(50U);
		if (_thread) {
			TerminateThread(_thread, 0U);
			_thread = NULL;
		}
	}
	else
	{
		_objEventPump->tick();
		Sleep(0U);
	}
}

#ifdef MULTI_THREADING

void GlobalInfo::AddEvent1(int event1ID,
	ChannelCopy * channel /* = nullptr */,
	ClientCopy * client /* = nullptr */,
	char * messageOrErrorText /* = nullptr */,
	size_t messageSize /* = 0U */,
	unsigned char subchannel /* = 255 */,
	ClientCopy * receivingClient /* = nullptr */,
	InteractiveType interactiveType  /* = None */,
	unsigned char variant /* = 255 */,
	bool blasted /* = false */,
	bool channelCreate_Hidden /* = false */,
	bool channelCreate_AutoClose /* = false */
	)
{
	return AddEventF(false, event1ID, 35353,
		channel, client, messageOrErrorText, messageSize, subchannel,
		receivingClient, interactiveType, variant, blasted, channelCreate_Hidden, channelCreate_AutoClose);
}
void GlobalInfo::AddEvent2(int event1ID, int event2ID,
	ChannelCopy * channel /* = nullptr */,
	ClientCopy * client /* = nullptr */,
	char * messageOrErrorText /* = nullptr */,
	size_t messageSize /* = 0 */,
	unsigned char subchannel /* = 255 */,
	ClientCopy * receivingClient /* = nullptr */,
	InteractiveType interactiveType  /* = None */,
	unsigned char variant /* = 255 */,
	bool blasted /* = false */,
	bool channelCreate_Hidden /* = false */,
	bool channelCreate_AutoClose /* = false */)
{
	return AddEventF(true, event1ID, event2ID,
		channel, client, messageOrErrorText, messageSize, subchannel,
		receivingClient, interactiveType, variant, blasted, channelCreate_Hidden, channelCreate_AutoClose);
}
void GlobalInfo::AddEventF(bool twoEvents, int event1ID, int event2ID,
	ChannelCopy * channel /* = nullptr */,
	ClientCopy * client /* = nullptr */,
	char * messageOrErrorText /* = nullptr */,
	size_t messageSize /* = 0 */,
	unsigned char subchannel /* = 255 */,
	ClientCopy * receivingClient /* = nullptr */,
	InteractiveType interactiveType  /* = None */,
	unsigned char variant /* = 255 */,
	bool blasted /* = false */,
	bool channelCreate_Hidden /* = false */,
	bool channelCreate_AutoClose /* = false */)
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
		With PushEvent() + multithreading, this would cause overwriting of old events and possibly access
		violations as variables are simulatenously written to by the ext and read from by Fusion at the same time.
		
		But in DarkEDIF, you'll note all the GenerateEvents() are handled on a queue, and the queue is
		iterated through in Handle(), thus it is quite safe. But we still need to protect potentially several
		AddEvent() functions running at once and corrupting the memory at some point; so we need the
		CRITICAL_SECTION variable mentioned in Extension.h to ensure this will not happen.
	*/

	SaveExtInfo * newEvent = new SaveExtInfo();
	SaveExtInfo &newEvent2 = *newEvent;

	// Initialise
	newEvent2.numEvents = twoEvents ? 2 : 1;
	newEvent2.CondTrig[0] = (unsigned short)event1ID;
	newEvent2.CondTrig[1] = (unsigned short)event2ID;
	newEvent2.channel = (ChannelCopy *)channel;
	// Client and senderClient overlap
	newEvent2.client = client;
	newEvent2.ReceivingClient = receivingClient;
	// Message Content, Error Text, and Loop Name overlap
	newEvent2.receivedMsg.content = messageOrErrorText;
	newEvent2.receivedMsg.size = messageSize;
	newEvent2.receivedMsg.subchannel = subchannel;
	newEvent2.receivedMsg.variant = variant;
	newEvent2.receivedMsg.blasted = blasted;
	newEvent2.InteractiveType = interactiveType;
	newEvent2.channelCreate_Hidden = channelCreate_Hidden;
	newEvent2.channelCreate_AutoClose = channelCreate_AutoClose;

	EnterCriticalSectionDerpy(&lock); // Needed before we access Extension

	_Saved.push_back(newEvent);

	LeaveCriticalSectionDebug(&lock); // We're done accessing Extension

	// Cause Handle() to be triggered, allowing Saved to be parsed
	if (_ext != nullptr)
		_ext->Runtime.Rehandle();
}

#endif // MULTI_THREADING


void Extension::ClearThreadData()
{
	memset(&threadData, 0, sizeof(SaveExtInfo));
}

void Extension::CreateError(const char * error)
{
	OutputDebugStringA(error);
	globals->AddEvent1(0, nullptr, nullptr, _strdup(error));
	//__asm int 3;
}

void GlobalInfo::CreateError(const char * error)
{
	OutputDebugStringA(error);
	AddEvent1(0, nullptr, nullptr, _strdup(error));
	//__asm int 3;
}

void Extension::AddToSend(void * data, size_t size)
{
	if (!data)
	{
		CreateError("Error adding to send binary: pointer supplied is invalid. "
			"The message has not been modified.");
		return;
	}
	if (!size)
		return;
	char * newptr = (char *)realloc(SendMsg, SendMsgSize + size);

	// Failed to reallocate memory
	if (!newptr)
	{
		std::stringstream error;
		error << "Received error number " << errno << " with reallocating memory to append to binary message. "
			<< "The message has not been modified.";
		CreateError(error.str().c_str());
		return;
	}
	SendMsg = newptr;
	SendMsgSize += size;

	// memcpy_s does not allow copying from what's already inside sendMsg; memmove_s does.
	// If we failed to copy memory.
	if (memmove_s(newptr + SendMsgSize - size, size, data, size))
	{
		std::stringstream error;
		error << "Received error number " << errno << " with reallocating memory to append to binary message. "
			<< "The message has not been modified.";
		CreateError(error.str().c_str());
		return;
	}
}

namespace lacewing { struct channelinternal; struct peerinternal; }
REFLAG Extension::Handle()
{
	// If thread is not working, use Tick functionality. This may add events, so do it before the event-loop check.
	if (!globals->_thread)
	{
		lacewing::error e = ObjEventPump->tick();
		if (e != nullptr)
		{
			CreateError(e->tostring());
			return REFLAG::NONE; // Run next loop
		}
	}

	// AddEvent() was called and not yet handled
	// (note all code that accesses Saved must have ownership of lock)

	// If Thread is not available, we have to tick() on Handle(), so
	// we have to run next loop even if there's no events in Saved() to deal with.
	bool RunNextLoop = !globals->_thread;

	for (size_t maxTrig = 0; maxTrig < 5; maxTrig++)
	{
		// Attempt to Enter, break if we can't get it instantly
		if (!TryEnterCriticalSection(&globals->lock))
		{
			RunNextLoop = true;
			break; // lock already occupied; leave it and run next event loop
		}
		// At this point we have effectively run EnterCriticalSection
#ifdef _DEBUG
		::CriticalSection << "Thread " << GetCurrentThreadId() << " : Entered on "
			<< __FILE__ << ", line " << __LINE__ << ".\r\n";
#endif

		if (Saved.empty())
		{
			LeaveCriticalSectionDebug(&globals->lock);
			break;
		}
		SaveExtInfo * s = Saved.front();
		Saved.erase(Saved.begin());

		#define DenyReasonSwitch(a) { free(DenyReason); DenyReason = a ? _strdup(a) : nullptr; }

		InteractivePending = s->InteractiveType;
		if (s->InteractiveType == InteractiveType::ConnectRequest)
			DenyReasonSwitch(globals->AutoResponse_Connect_DenyReason)
		else if (s->InteractiveType == InteractiveType::ChannelLeave)
			DenyReasonSwitch(globals->AutoResponse_ChannelLeave_DenyReason)
		else if (s->InteractiveType == InteractiveType::ClientNameSet)
		{
			NewClientName = _strdup(s->Requested.name);
			DenyReasonSwitch(globals->AutoResponse_NameSet_DenyReason)
		}
		else if (s->InteractiveType == InteractiveType::ChannelJoin)
		{
			NewChannelName = _strdup(s->Requested.name);
			DenyReasonSwitch(globals->AutoResponse_ChannelJoin_DenyReason)
		}
		else if (s->InteractiveType == InteractiveType::ChannelMessageIntercept)
		{
			DropMessage = globals->AutoResponse_MessageChannel == GlobalInfo::Deny_Quiet ||
				globals->AutoResponse_MessageChannel == GlobalInfo::Deny_TellFusion;
		}
		else if (s->InteractiveType == InteractiveType::ClientMessageIntercept)
		{
			DropMessage = globals->AutoResponse_MessageClient == GlobalInfo::Deny_Quiet ||
				globals->AutoResponse_MessageClient == GlobalInfo::Deny_TellFusion;
		}

		#undef DenyReasonSwitch

		LeaveCriticalSectionDebug(&globals->lock);

		// Trigger all stored events (more than one may be stored by calling AddEvent(***, true) )
		for (unsigned char u = 0; u < s->numEvents; ++u)
		{
			if (s->CondTrig[u] != 0xFFFF)
			{
				for (auto i : globals->Refs)
				{
					if (s->receivedMsg.content != nullptr)
					{
						i->threadData.receivedMsg.content = s->receivedMsg.content;
						i->threadData.receivedMsg.size = s->receivedMsg.size;
						i->threadData.receivedMsg.cursor = s->receivedMsg.cursor;
						i->threadData.receivedMsg.subchannel = s->receivedMsg.subchannel;
						// We only need these two for message intercepting ability
						i->threadData.receivedMsg.blasted = s->receivedMsg.blasted;
						i->threadData.receivedMsg.variant = s->receivedMsg.variant;
					}
					if (s->channel != nullptr)
						i->threadData.channel = s->channel;
					if (s->client != nullptr)
						i->threadData.client = s->client;

					i->threadData.ReceivingClient = s->ReceivingClient;
					i->threadData.InteractiveType = s->InteractiveType;
					i->threadData.channelCreate_Hidden = s->channelCreate_Hidden;
					i->threadData.channelCreate_AutoClose = s->channelCreate_AutoClose;
					i->Runtime.GenerateEvent((int)s->CondTrig[u]);
				}

				// We need to access these locks in multithreaded in case the user prevents
				// us from handling the interactive event properly by their actions,
				// e.g. don't want to allow the user to close midway through a send.
				// In single threaded the tick runs before this triggers, so it's not a problem.
				
				#define EnterSectionIfMultiThread(x) if (GThread) { EnterCriticalSectionDerpy(x); Sleep(0); }
				#define LeaveSectionIfMultiThread(x) if (GThread) LeaveCriticalSectionDebug(x)

				// If multiple events are triggering, only do this on the last one
				if (u == s->numEvents - 1)
				{
					// If interactive event, check for responses.
					if (s->InteractiveType != InteractiveType::None)
					{
						if (s->InteractiveType == InteractiveType::ConnectRequest)
						{
							if (globals->AutoResponse_Connect == GlobalInfo::WaitForFusion
								&& !s->client->isclosed)
							{
								EnterSectionIfMultiThread(&globals->lock);
								Srv.connect_response(s->client->orig(), DenyReason);

								// Disconnected by Fusion: reflect in copy
								if (DenyReason)
								{
									// Mark as disconnected
									s->client->disconnect(false);

									// Trick this for loop into running a second event to cleanup immediately
									assert(s->numEvents == 1); // connect req must be one event for this to work
									s->CondTrig[1] = 0xFFFF;
									s->numEvents = 2;
								}
								LeaveSectionIfMultiThread(&globals->lock);
							}
						}
						else if (s->InteractiveType == InteractiveType::ClientNameSet)
						{
							if (globals->AutoResponse_NameSet == GlobalInfo::WaitForFusion
								&& !s->client->isclosed)
							{
								EnterSectionIfMultiThread(&globals->lock);
								Srv.nameset_response(s->client->orig(), NewClientName, DenyReason);

								if (!DenyReason)
									s->client->name(NewClientName);
								LeaveSectionIfMultiThread(&globals->lock);
							}
							free(NewClientName);
							NewClientName = nullptr;
						}
						else if (s->InteractiveType == InteractiveType::ChannelJoin)
						{
							if (globals->AutoResponse_ChannelJoin == GlobalInfo::WaitForFusion
								&& !s->channel->isclosed && !s->client->isclosed)
							{
								EnterSectionIfMultiThread(&globals->lock);
								const char * passedNewChannelName = NewChannelName;
								char * denyReason = DenyReason;
								bool channelmasterset = false;

								if (!denyReason)
								{
									// Server owner ran "Change requested name" and passed the same name.
									// Replace channel name with null.
									const char * newChannelName = NewChannelName;
									if (newChannelName && !strcmp(s->channel->name(), passedNewChannelName))
										newChannelName = nullptr;

									// channel name was changed in join request
									if (newChannelName)
									{
										// New name sucks
										if (newChannelName[0] == 0 || strnlen(newChannelName, 256) == 256)
										{
											std::stringstream str;
											str << "Replacement channel name \"" << newChannelName <<
												"\" was blank or too long. Maximum channel name length is 255 bytes. "
												"Denying join channel request.";
											CreateError(str.str().c_str());
											denyReason = "Server replaced joining channel name to something invalid.";
										}
										// If this new name does not exist, we're going to create it.
										// If it does exist, we're going to join client to it.
										// Which is not the one we're joining this client to.
										else
										{
											auto newNameCh = std::find_if(Channels.cbegin(), Channels.cend(),
												[=](ChannelCopy * const c) {
													return c != s->channel && !_stricmp(c->name(), newChannelName);
											});

											// New name exists. Switch channel.
											if (newNameCh != Channels.cend())
											{
												// If the pre-name-change channel is not in server list, it should be deleted as this
												// is the last access point for the memory.
												// If it is in the list, the server will handle deleting it when needed.

												// If user is creating a channel, it won't be in server channel list until it's
												// create approved. (The creating user won't be in the channel's client list until
												// create approved either.)
												// If user had created it with autoclose off, and then left, the channel will exist
												// when a second client goes to join it.							

												// If the channel has clients, we're joining, in which case we don't need to delete
												bool foundPreNameChangeChannel = (*newNameCh)->clientcount() != 0U;
												if (!foundPreNameChangeChannel)
												{
													for (auto col = Srv.firstchannel(); col; col = col->next())
													{
														if (col == &s->channel->orig())
														{
															foundPreNameChangeChannel = true;
															break;
														}
													}
												}

												if (!foundPreNameChangeChannel)
												{
													s->channel->close();
													delete s->channel;
												}

												s->channel = *newNameCh;
											}
											else // new name not found.
											{
												// Old channel was being created by this user: rename it to new name
												if (s->channel->newlycreated())
													s->channel->name(newChannelName);
												else // Old channel was being joined (new name created); so we won't do anything with old channel.
													s->channel = new ChannelCopy(Srv.createchannel(newChannelName, s->client->orig(), !s->channelCreate_Hidden, s->channelCreate_AutoClose));

												channelmasterset = true;
											}
										}
									}
									// Created new channel, now it's approved, add it to Fusion list
									// It'll be added to Relay Server's list when joinchannel_response is run.
									else if (s->channel->newlycreated())
										channelmasterset = true;
								} // a deny reason was passed by server

								// Deny, create or join channel
								//  called by joinchannel_response
								Srv.joinchannel_response(s->channel->orig(), s->client->orig(), denyReason);

								if (denyReason)
								{
									// Renamed and then denied a channel; delete it
									// TODO: This check may not be necessary.
									if (s->channel->newlycreated())
									{
										s->channel->close();
										delete s->channel;
									}
								}
								else
								{
									s->channel->addclient(s->client, false);
									if (channelmasterset)
										s->channel->setchannelmaster(s->client);
								}
								LeaveSectionIfMultiThread(&globals->lock);
							} // is any closed

							free(NewChannelName);
							NewChannelName = nullptr;
						}
						else if (s->InteractiveType == InteractiveType::ChannelLeave)
						{
							if (globals->autoResponse_ChannelLeave == GlobalInfo::WaitForFusion
								&& !s->channel->isclosed && !s->client->isclosed)
							{
								EnterSectionIfMultiThread(&globals->lock);
								Srv.leavechannel_response(s->channel->orig(), s->client->orig(), DenyReason);

								if (!DenyReason)
								{
									// Left channel, approved by Fusion: reflect in copy
									s->channel->removeclient(s->client, false);

									// Trick this for loop into running a second event to cleanup immediately
									assert(s->numEvents == 1); // leave channel req must be one event for this to work
									s->CondTrig[1] = 0xFFFF;
									s->numEvents = 2;

									// Valid channel + client with 0xFFFF is seen as "client leaving channel".
									// If the channel closed, we need to do close channel, which is channel + no client with 0xFFFF.
									if (s->channel->isclosed)
										s->client = nullptr;
								}
								LeaveSectionIfMultiThread(&globals->lock);
							}
						}
						else if (s->InteractiveType == InteractiveType::ChannelMessageIntercept)
						{
							if (globals->AutoResponse_MessageChannel == GlobalInfo::WaitForFusion)
							{
								if (!DropMessage && (s->channel->isclosed || s->senderClient->isclosed))
									DropMessage = true;

								// threadData.senderClient may not be accurate, since user selection will alter Client,
								// and it overlaps with Client. Refer to original event "s".
								EnterSectionIfMultiThread(&globals->lock);
								Srv.channelmessage_permit(s->senderClient->orig(), s->channel->orig(),
									threadData.receivedMsg.blasted, threadData.receivedMsg.subchannel,
									threadData.receivedMsg.content, threadData.receivedMsg.size,
									threadData.receivedMsg.variant, !DropMessage);
								LeaveSectionIfMultiThread(&globals->lock);
							}

							// Replace messages not programmed.
							// Look like Lacewing Relay was going to use Send Binary, though, but if they replace with
							// text/number the send binary should stay intact.
							// We may have to use a 3rd message struct as we don't want to use the sendMsg data.
							//srv.channelmessage_permit(*threadData.channel, *threadData.senderClient,
							//	threadData.receivedMsg.content, threadData.MsgToSend.size, threadData.receivedMsg.subchannel);
						}
						else if (s->InteractiveType == InteractiveType::ClientMessageIntercept)
						{
							if (globals->AutoResponse_MessageClient == GlobalInfo::WaitForFusion)
							{
								if (!DropMessage && (threadData.channel->isclosed ||
									threadData.senderClient->isclosed || threadData.ReceivingClient->isclosed))
								{
									DropMessage = true;
								}

								EnterSectionIfMultiThread(&globals->lock);

								Srv.clientmessage_permit(threadData.senderClient->orig(), threadData.channel->orig(),
									threadData.ReceivingClient->orig(),
									threadData.receivedMsg.blasted, threadData.receivedMsg.subchannel,
									threadData.receivedMsg.content, threadData.receivedMsg.size,
									threadData.receivedMsg.variant, !DropMessage);

								LeaveSectionIfMultiThread(&globals->lock);
							}
						}
						else
							globals->CreateError("Interactive type not recognised!");
					}

					// Free memory for received message
					threadData.Free();
				}

				#undef EnterSectionIfMultiThread
				#undef LeaveSectionIfMultiThread
			}
			// Remove copies if this particular event number is used
			else
			{
				// If channel, it's a channel leave or peer leaving channel
				if (s->channel)
				{
					// channel, no client: channel closing.
					// Worth noting this is not called for non-autoclose channels.
					if (!s->client)
					{
						assert(s->channel->isclosed);

						for (auto u = Channels.begin(); u != Channels.end(); ++u)
						{
							if (*u == s->channel)
							{
								Channels.erase(u);
								delete s->channel;
								break;
							}
						}

						for (auto dropExt : globals->Refs)
							if (dropExt->threadData.channel == s->channel)
								dropExt->threadData.channel = nullptr;
					}
					// channel, client: peer leaving channel.
					else
					{
						// Client and channel are already fully updated.
						// Unlike RelayClient, clients aren't copied per channel, so we
						// don't need to delete.
						
						// We just make sure user doesn't have them selected.
						// At least, not within the channel they just left.
						for (auto dropExt : globals->Refs)
						{
							if (dropExt->threadData.channel == s->channel &&
								dropExt->threadData.client == s->client)
								dropExt->threadData.client = nullptr;
						}
					}
				}
				// No channel, client: client is disconnecting.
				else if (s->client)
				{
					assert(s->client->isclosed);

					// Drop client
					for (auto u = Clients.begin(); u != Clients.end(); ++u)
					{
						if (*u == s->client)
						{
							Clients.erase(u);
							delete s->client;
							break;
						}
					}

					// Make sure user doesn't have them selected.
					for (auto dropExt : globals->Refs)
					{
						if (dropExt->threadData.client == s->client)
							dropExt->threadData.client = nullptr;
					}
				}
				// No channel, no peer: server has unhosted, clear everything
				else 
				{
					for (auto i : Channels)
					{
						assert(i->isclosed);
						i->close();
						delete i;
					}
					for (auto i : Clients)
					{
						assert(i->isclosed);
						delete i;
					}
					for (auto dropExt : globals->Refs)
					{
						dropExt->threadData.channel = nullptr;
						dropExt->threadData.client = nullptr;
						dropExt->threadData.ReceivingClient = nullptr;
					}

					Channels.clear();
					Clients.clear();
				}
			}
		}
	}

	// Will not be called next loop if RunNextLoop is false
	return RunNextLoop ? REFLAG::NONE : REFLAG::ONE_SHOT;
}


DWORD WINAPI ObjectDestroyTimeoutFunc(void * ThisGlobalsInfo)
{
	GlobalInfo& G = *(GlobalInfo *)ThisGlobalsInfo;

	// If the user has created a new object which is receiving events from Bluewing
	// it's cool, just close silently
	if (!G.Refs.empty())
		return 0U;

	// If not hosting, no clients to worry about dropping
	if (!G._server.hosting())
		return 0U;

	// App closed within next 3 seconds: unhost by default
	if (WaitForSingleObject(AppWasClosed, 3000U) == WAIT_OBJECT_0)
		return 0U;

	// 3 seconds have passed: if we now have an ext, or server was unhosted, we're good
	if (!G.Refs.empty())
		return 0U;

	if (!G._server.hosting())
		return 0U;

	if (G.TimeoutWarningEnabled)
	{
		// Otherwise, fuss at them.
		MessageBoxA(NULL, "Bluewing Server Warning!\r\n"
			"All Bluewing Server objects have been destroyed and some time has passed; but "
			"the server is still hosting in the background, unused, but still available.\r\n"
			"If this is intended behaviour, disable the Timeout warning in the object properties.\r\n"
			"If you want to unhost if no instances remain open, use the FullCleanup action on the Bluewing object.",
			"Bluewing Server Warning",
			MB_OK | MB_DEFBUTTON1 | MB_ICONEXCLAMATION | MB_TOPMOST);
	}
	delete &G; // Cleanup!
	return 0U;
}

Extension::~Extension()
{
	char msgBuff[500];
	sprintf_s(msgBuff, "~Extension called; Refs count is %u.\n", globals->Refs.size());
	OutputDebugStringA(msgBuff);

	EnterCriticalSectionDerpy(&globals->lock);
	// Remove this Extension from liblacewing usage.
	auto i = std::find(globals->Refs.cbegin(), globals->Refs.cend(), this);
	bool wasBegin = i == globals->Refs.cbegin();
	globals->Refs.erase(i);

	// Shift secondary event management to other Extension, if any
	if (!globals->Refs.empty())
	{
		OutputDebugStringA("Note: Switched Lacewing instances.\n");

		// Switch Handle ticking over to next Extension visible.
		if (wasBegin)
		{
			globals->_ext = globals->Refs.front();
			LeaveCriticalSectionDebug(&globals->lock);

			globals->_ext->Runtime.Rehandle();
		}
		else // This extension wasn't even the main event handler (for Handle()/globals).
		{
			LeaveCriticalSectionDebug(&globals->lock);
		}
	}
	// Last instance of this object; if global and not full-delete-enabled, do not cleanup.
	// In single-threaded server instances, this will cause a dirty timeout from the client side;
	// the lower-level protocols, e.g. TCP, will close the connection after a certain amount of
	// not-responding.
	// In multi-threaded instances, messages and events will continue to be handled, and no visible
	// difference is made to the clients, other than the events explicitly handled by Fusion events.
	else
	{
		if (!globals->_server.hosting())
			OutputDebugStringA("Note: Not hosting, nothing important to retain, closing globals info.\n");
		else if (!IsGlobal)
			OutputDebugStringA("Note: Not global, closing globals info.\n");
		else if (globals->FullDeleteEnabled)
			OutputDebugStringA("Note: Full delete enabled, closing globals info.\n");
		// Wait for 0ms returns immediately as per spec
		else if (WaitForSingleObject(AppWasClosed, 0U) == WAIT_OBJECT_0)
			OutputDebugStringA("Note: App was closed, closing globals info.\n"); 
		else // !globals->FullDeleteEnabled
		{
			OutputDebugStringA("Note: Last instance dropped, and currently hosting - "
				"globals will be retained until a Unhost is called.\n");
			globals->_ext = nullptr;
			globals->LastDestroyedExtSelectedChannel = threadData.channel;
			globals->LastDestroyedExtSelectedClient = threadData.client;
			LeaveCriticalSectionDebug(&globals->lock);

			sprintf_s(msgBuff, "Timeout thread started. If no instance has reclaimed ownership in 3 seconds,%s.\n",
				globals->TimeoutWarningEnabled
				? "a warning message will be shown"
				: "the hosting will terminate and all pending messages will be discarded");
			OutputDebugStringA(msgBuff);

			CreateThread(NULL, 0, ObjectDestroyTimeoutFunc, globals, NULL, NULL);
			ClearThreadData();
			return;
		}

		std::string id = std::string(std::string("LacewingRelayServer") + (globals->_globalID ? globals->_globalID : ""));
		Runtime.WriteGlobal(id.c_str(), nullptr);
		LeaveCriticalSectionDebug(&globals->lock);
		delete globals; // Unhosts and closes event pump, deletes lock
		globals = nullptr;
	}

	ClearThreadData();
}

short Extension::Display()
{
    /*
       If you return REFLAG::DISPLAY in Handle() this routine will run.
    */

    // Ok
    return 0;
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
