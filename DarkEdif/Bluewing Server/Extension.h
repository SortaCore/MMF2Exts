#include "Edif.h"
#include <functional>

struct GlobalInfo;

class Extension
{
public:
	// Hide stuff requiring other headers
	std::shared_ptr<SaveExtInfo> threadData;

	RUNDATA * rdPtr;
	RunHeader * rhPtr;

	Edif::Runtime Runtime;

	static const int MinimumBuild = 251;
	static const int Version = lacewing::relayserver::buildnum;

	static const OEFLAGS OEFLAGS = OEFLAGS::NEVER_KILL | OEFLAGS::NEVER_SLEEP; // Use OEFLAGS namespace
	static const OEPREFS OEPREFS = OEPREFS::GLOBAL; // Use OEPREFS namespace

	static const int WindowProcPriority = 100;

	Extension(RUNDATA * rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr);
	~Extension();
	std::thread::id mainThreadID;

	DarkEdif::FusionDebugger FusionDebugger;

	bool isGlobal;
	GlobalInfo * globals;


	// This allows prettier and more readable access while maintaining global variables.
	#define ObjEventPump				globals->_objEventPump
	#define Srv							globals->_server
	#define FlashSrv					globals->_server.flash
	#define SendMsg						globals->_sendMsg
	#define DenyReason					globals->_denyReason
	#define SendMsgSize					globals->_sendMsgSize
	#define AutomaticallyClearBinary	globals->_automaticallyClearBinary
	#define GlobalID					globals->_globalID
	#define NewChannelName				globals->_newChannelName
	#define NewClientName				globals->_newClientName
	#define DropMessage					globals->_dropMessage
	#define InteractivePending			globals->_interactivePending
	#define GThread						globals->_thread
	std::tstring_view loopName;

	// Selections local to this extension. When outside of events, these are used.
	// When events are run by Handle(), this selection is copied out, then altered to match event.
	// Following the event, the selection is restored.

	std::shared_ptr<lacewing::relayserver::channel> selChannel;
	std::shared_ptr<lacewing::relayserver::client> selClient;

	void ClearThreadData();
	void CreateError(_Printf_format_string_ const char * errU8, ...);

	void AddToSend(const void *, size_t);

	// called from Handle() when a Lacewing object is being destroyed (e.g. client disconnect, channel leave)
	void DeselectIfDestroyed(std::shared_ptr<SaveExtInfo> s);
	// called from Handle() when an interactive event needs to be responded to
	void HandleInteractiveEvent(std::shared_ptr<SaveExtInfo> s);

	std::string TStringToUTF8Stripped(std::tstring);

	// Returns 0 if OK. -1 if cut off UTF-8 at front, 1 if cut off at end
	int CheckForUTF8Cutoff(std::string_view sv);

	// UTF-8 uses a bitmask to determine how many bytes are in the item.
	// Note that this does not verify the ending characters other than a size check; but any TString converter will throw them out
	int GetNumBytesInUTF8Char(std::string_view sv);

	// Reads string at given position of received binary. If size is -1, will expect a null.
	// isCursorExpression is used for error messages.
	std::tstring ReadStringFromRecvBinary(size_t index, int size, bool isCursorExpression);

	// To work around this, we use a special event number which will deselect
	// all the pointers in SaveExtInfo after they should no longer be valid.
	// In this way, when a client sends message then disconnects, on liblacewing's side that happens instantly,
	// and liblacewing cleans up its client variable instantly too.
	// We can't "deny" a disconnect, so we must accept it immediately.

	/// Actions
		void RemovedActionNoParams();

		void RelayServer_Host(int port);
		void RelayServer_StopHosting();
		void FlashServer_Host(const TCHAR * path);
		void FlashServer_StopHosting();
		void HTML5Server_EnableHosting();
		void HTML5Server_DisableHosting();
		void ChannelListing_Enable();
		void ChannelListing_Disable();
		void SetWelcomeMessage(const TCHAR * message);
		void EnableCondition_OnConnectRequest(int informFusion, int immediateRespondWith, const TCHAR * autoDenyReason);
		void EnableCondition_OnNameSetRequest(int informFusion, int immediateRespondWith, const TCHAR * autoDenyReason);
		void EnableCondition_OnJoinChannelRequest(int informFusion, int immediateRespondWith, const TCHAR * autoDenyReason);
		void EnableCondition_OnLeaveChannelRequest(int informFusion, int immediateRespondWith, const TCHAR * autoDenyReason);
		void EnableCondition_OnMessageToChannel(int informFusion, int immediateRespondWith);
		void EnableCondition_OnMessageToPeer(int informFusion, int immediateRespondWith);
		void EnableCondition_OnMessageToServer(int informFusion);
		void OnInteractive_Deny(const TCHAR * reason);
		void OnInteractive_ChangeClientName(const TCHAR * newName);
		void OnInteractive_ChangeChannelName(const TCHAR * newName);
		void OnInteractive_DropMessage();
		void OnInteractive_ReplaceMessageWithText(const TCHAR * newText);
		void OnInteractive_ReplaceMessageWithNumber(int newNumber);
		void OnInteractive_ReplaceMessageWithSendBinary();

		void Channel_SelectByName(const TCHAR * name);
		void Channel_Close();
		void Channel_SelectMaster();
		void Channel_LoopClients();
		void Channel_LoopClientsWithName(const TCHAR * loopName);
		void Channel_SetLocalData(const TCHAR * key, const TCHAR * value);
		void LoopAllChannels();
		void LoopAllChannelsWithName(const TCHAR * loopName);
		void Channel_CreateChannelWithMasterByName(const TCHAR * channelName, int hidden, int autoclose, const TCHAR * masterClientName);
		void Channel_CreateChannelWithMasterByID(const TCHAR * channelName, int hidden, int autoclose, int masterClientID);
		void Channel_KickClientByName(const TCHAR * clientName);
		void Channel_KickClientByID(int id);
		void Channel_JoinClientByName(const TCHAR * clientName);
		void Channel_JoinClientByID(int id);
		void Client_Disconnect();
		void Client_SetLocalData(const TCHAR * key, const TCHAR * value);
		void Client_LoopJoinedChannels();
		void Client_LoopJoinedChannelsWithName(const TCHAR * loopName);
		void Client_SelectByName(const TCHAR * clientName);
		void Client_SelectByID(int clientID);
		void Client_SelectSender();
		void Client_SelectReceiver();
		void Client_JoinToChannel(const TCHAR * channelNamePtr);
		void LoopAllClients();
		void LoopAllClientsWithName(const TCHAR * loopName);

		void SendTextToChannel(int subchannel, const TCHAR * textToSend);
		void SendTextToClient(int subchannel, const TCHAR * textToSend);
		void SendNumberToChannel(int subchannel, int numToSend);
		void SendNumberToClient(int subchannel, int numToSend);
		void SendBinaryToChannel(int subchannel);
		void SendBinaryToClient(int subchannel);

		void BlastTextToChannel(int subchannel, const TCHAR * TextToBlast);
		void BlastTextToClient(int subchannel, const TCHAR * TextToBlast);
		void BlastNumberToChannel(int subchannel, int NumToBlast);
		void BlastNumberToClient(int subchannel, int NumToBlast);
		void BlastBinaryToChannel(int subchannel);
		void BlastBinaryToClient(int subchannel);

		void AddByteText(const TCHAR * Byte);
		void AddByteInt(int Byte);
		void AddShort(int Short);
		void AddInt(int Int);
		void AddFloat(float Float);
		void AddStringWithoutNull(const TCHAR * string);
		void AddString(const TCHAR * string);
		void AddBinary(unsigned int address, int size);
		void AddFileToBinary(const TCHAR * file);
		void ResizeBinaryToSend(int newSize);
		void CompressSendBinary();
		void ClearBinaryToSend();

		void DecompressReceivedBinary();
		void MoveReceivedBinaryCursor(int Position);
		void SaveReceivedBinaryToFile(int Position, int size, const TCHAR * Filename);
		void AppendReceivedBinaryToFile(int Position, int size, const TCHAR * Filename);


	/// Conditions

		const bool AlwaysTrue() { return true; }
		const bool AlwaysFalse() { return false; }
		const bool AlwaysFalseWithTextParam(const TCHAR *) { return false; }
		bool SubchannelMatches(int subchannel);
		// AlwaysTrue:	bool OnError();
		// AlwaysTrue:	bool OnConnectRequest();
		// AlwaysTrue:	bool OnDisconnect();
		// AlwaysTrue:	bool OnChannel_JoinRequest();
		// AlwaysTrue:	bool OnChannel_LeaveRequest();
		// AlwaysTrue:	bool AllChannelsLoop();
		// AlwaysTrue:	bool OnClient_JoinedChannelsLoop();
		// AlwaysTrue:	bool AllClientsLoop();
		// AlwaysTrue:	bool OnChannel_ClientLoop();
		bool Client_IsChannelMaster();
		// AlwaysTrue:	bool OnClient_NameSetRequest();
		// MessageMatches:	bool OnSentTextMessageToServer(int subchannel);
		// MessageMatches:	bool OnSentNumberMessageToServer(int subchannel);
		// MessageMatches:	bool OnSentBinaryMessageToServer(int subchannel);
		// MessageMatches:	bool OnAnySentMessageToServer(int subchannel);
		// MessageMatches:	bool OnSentTextMessageToChannel(int subchannel);
		// MessageMatches:	bool OnSentNumberMessageToChannel(int subchannel);
		// MessageMatches:	bool OnSentBinaryMessageToChannel(int subchannel);
		// MessageMatches:	bool OnAnySentMessageToChannel(int subchannel);
		// MessageMatches:	bool OnSentTextMessageToPeer(int subchannel);
		// MessageMatches:	bool OnSentNumberMessageToPeer(int subchannel);
		// MessageMatches:	bool OnSentBinaryMessageToPeer(int subchannel);
		// MessageMatches:	bool OnAnySentMessageToPeer(int subchannel);
		// MessageMatches:	bool OnBlastedTextMessageToServer(int subchannel);
		// MessageMatches:	bool OnBlastedNumberMessageToServer(int subchannel);
		// MessageMatches:	bool OnBlastedBinaryMessageToServer(int subchannel);
		// MessageMatches:	bool OnAnyBlastedMessageToServer(int subchannel);
		// MessageMatches:	bool OnBlastedTextMessageToChannel(int subchannel);
		// MessageMatches:	bool OnBlastedNumberMessageToChannel(int subchannel);
		// MessageMatches:	bool OnBlastedBinaryMessageToChannel(int subchannel);
		// MessageMatches:	bool OnAnyBlastedMessageToChannel(int subchannel);
		// MessageMatches:	bool OnBlastedTextMessageToPeer(int subchannel);
		// MessageMatches:	bool OnBlastedNumberMessageToPeer(int subchannel);
		// MessageMatches:	bool OnBlastedBinaryMessageToPeer(int subchannel);
		// MessageMatches:	bool OnAnyBlastedMessageToPeer(int subchannel);
		bool OnAllChannelsLoopWithName(const TCHAR * loopName);
		bool OnClientsJoinedChannelLoopWithName(const TCHAR * loopName);
		bool OnAllClientsLoopWithName(const TCHAR * loopName);
		bool OnChannelClientsLoopWithName(const TCHAR * loopName);
		bool OnChannelClientsLoopWithNameFinished(const TCHAR * loopName);
		bool OnAllChannelsLoopWithNameFinished(const TCHAR * loopName);
		bool OnClientsJoinedChannelLoopWithNameFinished(const TCHAR * loopName);
		// AlwaysTrue	bool OnChannelsClientLoopFinished(const TCHAR * loopName);
		// AlwaysTrue	bool OnAllChannelsLoopFinished();
		// AlwaysTrue	bool OnAllClientsLoopFinished();
		// AlwaysTrue	bool OnClientsJoinedChannelLoopFinished();
		bool IsLacewingServerHosting();
		bool IsFlashPolicyServerHosting();
		bool ChannelIsHiddenFromChannelList();
		bool ChannelIsSetToCloseAutomatically();
		bool OnAllClientsLoopWithNameFinished(const TCHAR * loopName);
		// Added conditions:
		bool IsClientOnChannel_ID(int clientID, const TCHAR * channelName);
		bool IsClientOnChannel_Name(const TCHAR * clientName, const TCHAR * channelName);
		bool DoesChannelNameExist(const TCHAR * channelName);
		bool DoesChannelIDExist(int channelID);
		bool DoesClientNameExist(const TCHAR * clientName);
		bool DoesClientIDExist(int clientID);
		bool IsHTML5Hosting();

	/// Expressions

		const TCHAR * Error();
		const TCHAR * Lacewing_Version();
		unsigned int BinaryToSend_Size();
		const TCHAR * RequestedClientName();
		const TCHAR * RequestedChannelName();
		const TCHAR * Channel_Name();
		unsigned int Channel_ClientCount();
		const TCHAR * Client_Name();
		unsigned int Client_ID();
		const TCHAR * Client_IP();
		unsigned int Client_ConnectionTime(); // NB: was removed in Lacewing, kept in Bluewing
		unsigned int Client_ChannelCount();
		const TCHAR * ReceivedStr();
		int ReceivedInt();
		unsigned int ReceivedBinarySize();
		unsigned int ReceivedBinaryAddress();
		const TCHAR * StrANSIByte(int index);
		unsigned int UnsignedByte(int index);
		int SignedByte(int index);
		unsigned int UnsignedShort(int index);
		int SignedShort(int index);
		unsigned int UnsignedInteger(int index);
		int SignedInteger(int index);
		float Float(int index);
		const TCHAR * StringWithSize(int index, int size);
		const TCHAR * String(int index);
		unsigned int Subchannel();
		unsigned int Channel_Count();
		const TCHAR * Client_GetLocalData(const TCHAR * key);
		const TCHAR * CursorStrByte();
		unsigned int CursorUnsignedByte();
		int CursorSignedByte();
		unsigned int CursorUnsignedShort();
		int CursorSignedShort();
		unsigned int CursorUnsignedInteger();
		int CursorSignedInteger();
		float CursorFloat();
		const TCHAR * CursorStringWithSize(int size);
		const TCHAR * CursorString();
		const TCHAR * Client_ProtocolImplementation();
		const TCHAR * Channel_GetLocalData(const TCHAR * key);
		unsigned int Port();
		unsigned int BinaryToSend_Address();
		const TCHAR * Welcome_Message();
		// Added expressions:
		const TCHAR * DumpMessage(int index, const TCHAR * format);
		unsigned int AllClientCount();
		const TCHAR * GetDenyReason();
		int ConvToUTF8_GetVisibleCharCount(const TCHAR * tStr);
		int ConvToUTF8_GetCompleteCharCount(const TCHAR * tStr);
		int ConvToUTF8_GetByteCount(const TCHAR * tStr);

	/* These are called if there's no function linked to an ID */

	void Action(int ID, RUNDATA * rdPtr, long param1, long param2);
	long Condition(int ID, RUNDATA * rdPtr, long param1, long param2);
	long Expression(int ID, RUNDATA * rdPtr, long param);



	/*  These replace the functions like HandleRunObject that used to be
		implemented in Runtime.cpp. They work exactly the same, but they're
		inside the extension class.
	*/

	REFLAG Handle();
	REFLAG Display();

	short Pause();
	short Continue();

	bool Save(HANDLE File);
	bool Load(HANDLE File);

};

void eventpumpdeleter(lacewing::eventpump);

enum class AutoResponse : char
{
	Invalid = -1,
	Approve_Quiet = 0,
	Deny_Quiet,
	Approve_TellFusion,
	Deny_TellFusion,
	WaitForFusion
};

struct GlobalInfo
{
	std::unique_ptr<lacewing::_eventpump, std::function<decltype(eventpumpdeleter)>>	_objEventPump;
	lacewing::relayserver		_server;
	char *						_sendMsg = nullptr;
	std::string					_denyReason,
		 						_newChannelName,
		 						_newClientName;
	bool						_dropMessage = false,
								_automaticallyClearBinary = true;
	InteractiveType				_interactivePending = InteractiveType::None;
	size_t						_sendMsgSize = 0U;

	char *						_globalID = nullptr;
	HANDLE						_thread = NULL;
	Extension *					_ext = nullptr;
	std::string_view			_loopName;
	HANDLE						timeoutThread = NULL;
	std::weak_ptr<lacewing::relayserver::channel>		lastDestroyedExtSelectedChannel;
	std::weak_ptr<lacewing::relayserver::client>		lastDestroyedExtSelectedClient;

	std::vector<std::shared_ptr<SaveExtInfo>>	_saved;

	// update with UpdateAcceptableChars() only. Expects lock held if multithreaded.
	std::vector<std::int32_t> acceptableCharCategories;
	std::vector<std::int32_t> acceptableSpecificChars;
	std::vector<std::pair<std::int32_t, std::int32_t>> acceptableCharRanges;
	void UpdateAcceptableChars(std::string str);

	template<typename T> struct LocalData
	{
		std::shared_ptr<T> ptr;
		std::tstring key;
		std::tstring val;

		LocalData(std::shared_ptr<T> ptr, std::tstring key, std::tstring val)
			: ptr(ptr), key(key), val(val) { }
	};
	std::vector<LocalData<lacewing::relayserver::client>> clientLocal;
	std::vector<LocalData<lacewing::relayserver::channel>> channelLocal;

	const std::tstring& GetLocalData(std::shared_ptr<lacewing::relayserver::client> client, std::tstring key);
	const std::tstring& GetLocalData(std::shared_ptr<lacewing::relayserver::channel> channel, std::tstring key);
	void SetLocalData(std::shared_ptr<lacewing::relayserver::client> client, std::tstring key, std::tstring value);
	void SetLocalData(std::shared_ptr<lacewing::relayserver::channel> channel, std::tstring key, std::tstring value);
	void ClearLocalData(std::shared_ptr<lacewing::relayserver::client> client);
	void ClearLocalData(std::shared_ptr<lacewing::relayserver::channel> channel);

	std::shared_ptr<lacewing::relayserver::channel>				selChannel = nullptr;
	std::shared_ptr<lacewing::relayserver::client>				selClient = nullptr;

	CRITICAL_SECTION			lock = {};
	std::vector<Extension *>	Refs;
	bool						timeoutWarningEnabled = true; // If no Lacewing exists, fuss after set time period
	bool						fullDeleteEnabled = true; // If no Bluewing exists after DestroyRunObject, clean up GlobalInfo

	void AddEvent1(int event1,
		std::shared_ptr<lacewing::relayserver::channel> channel = nullptr,
		std::shared_ptr<lacewing::relayserver::client> senderClient = nullptr,
		std::string_view messageOrErrorText = std::string_view(),
		lw_ui8 subchannel = 255,
		std::shared_ptr<lacewing::relayserver::client> receivingClient = nullptr,
		InteractiveType interactiveType = InteractiveType::None,
		lw_ui8 variant = 255,
		bool blasted = false,
		bool channelCreate_Hidden = false, bool channelCreate_AutoClose = false);
	void AddEvent2(int event1, int event2,
		std::shared_ptr<lacewing::relayserver::channel> channel = nullptr,
		std::shared_ptr<lacewing::relayserver::client> senderClient = nullptr,
		std::string_view messageOrErrorText = std::string_view(),
		lw_ui8 subchannel = 255,
		std::shared_ptr<lacewing::relayserver::client> receivingClient = nullptr,
		InteractiveType interactiveType = InteractiveType::None,
		lw_ui8 variant = 255,
		bool blasted = false,
		bool channelCreate_Hidden = false, bool channelCreate_AutoClose = false);
	void AddEventF(bool twoEvents, int Event1, int Event2,
		std::shared_ptr<lacewing::relayserver::channel> channel = nullptr,
		std::shared_ptr<lacewing::relayserver::client> senderClient = nullptr,
		std::string_view MessageOrErrorText = std::string_view(),
		lw_ui8 subchannel = 255,
		std::shared_ptr<lacewing::relayserver::client> receivingClient = nullptr,
		InteractiveType interactiveType = InteractiveType::None,
		lw_ui8 variant = 255,
		bool blasted = false,
		bool channelCreate_Hidden = false,
		bool channelCreate_AutoClose = false
	);
	void CreateError(_Printf_format_string_ const char * errorTextU8, ...);
	void CreateError(_Printf_format_string_ const char * errorTextU8, va_list);

	AutoResponse autoResponse_Connect = AutoResponse::Approve_Quiet;
	std::string autoResponse_Connect_DenyReason;
	AutoResponse autoResponse_NameSet = AutoResponse::Approve_Quiet;
	std::string autoResponse_NameSet_DenyReason;
	AutoResponse autoResponse_ChannelJoin = AutoResponse::Approve_Quiet;
	std::string autoResponse_ChannelJoin_DenyReason;
	AutoResponse autoResponse_ChannelLeave = AutoResponse::Approve_Quiet;
	std::string autoResponse_ChannelLeave_DenyReason;
	AutoResponse autoResponse_MessageClient = AutoResponse::Approve_Quiet;
	AutoResponse autoResponse_MessageChannel = AutoResponse::Approve_Quiet;
	AutoResponse autoResponse_MessageServer = AutoResponse::WaitForFusion;

	GlobalInfo(Extension * e, EDITDATA * edPtr);
	~GlobalInfo() noexcept(false);
};
