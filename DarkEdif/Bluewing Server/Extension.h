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
	std::string_view loopName;

	// Selections local to this extension. When outside of events, these are used.
	// When events are run by Handle(), this selection is copied out, then altered to match event.
	// Following the event, the selection is restored.

	std::shared_ptr<lacewing::relayserver::channel> selChannel;
	std::shared_ptr<lacewing::relayserver::client> selClient;

	void ClearThreadData();
	void CreateError(_Printf_format_string_ const char *, ...);

	void AddToSend(void *, size_t);

	// called from Handle() when a Lacewing object is being destroyed (e.g. client disconnect, channel leave)
	void DeselectIfDestroyed(std::shared_ptr<SaveExtInfo> s);
	// called from Handle() when an interactive event needs to be responded to
	void HandleInteractiveEvent(std::shared_ptr<SaveExtInfo> s);


	// To work around this, we use a special event number which will deselect
	// all the pointers in SaveExtInfo after they should no longer be valid.
	// In this way, when a client sends message then disconnects, on liblacewing's side that happens instantly,
	// and liblacewing cleans up its client variable instantly too.
	// We can't "deny" a disconnect, so we must accept it immediately.
	
	/// Actions
		void RemovedActionNoParams();

		void RelayServer_Host(int port);
		void RelayServer_StopHosting();
		void FlashServer_Host(char * path);
		void FlashServer_StopHosting();
		void HTML5Server_EnableHosting();
		void HTML5Server_DisableHosting();
		void ChannelListing_Enable();
		void ChannelListing_Disable();
		void SetWelcomeMessage(char * message);
		void EnableCondition_OnConnectRequest(int informFusion, int immediateRespondWith, char * autoDenyReason);
		void EnableCondition_OnNameSetRequest(int informFusion, int immediateRespondWith, char * autoDenyReason);
		void EnableCondition_OnJoinChannelRequest(int informFusion, int immediateRespondWith, char * autoDenyReason);
		void EnableCondition_OnLeaveChannelRequest(int informFusion, int immediateRespondWith, char * autoDenyReason);
		void EnableCondition_OnMessageToChannel(int informFusion, int immediateRespondWith);
		void EnableCondition_OnMessageToPeer(int informFusion, int immediateRespondWith);
		void EnableCondition_OnMessageToServer(int informFusion);
		void OnInteractive_Deny(char * reason);
		void OnInteractive_ChangeClientName(char * newName);
		void OnInteractive_ChangeChannelName(char * newName);
		void OnInteractive_DropMessage();
		void OnInteractive_ReplaceMessageWithText(char * newText);
		void OnInteractive_ReplaceMessageWithNumber(int newNumber);
		void OnInteractive_ReplaceMessageWithSendBinary();

		void Channel_SelectByName(char * name);
		void Channel_Close();
		void Channel_SelectMaster();
		void Channel_LoopClients();
		void Channel_LoopClientsWithName(char * loopName);
		void Channel_SetLocalData(char * key, char * value);
		void LoopAllChannels();
		void LoopAllChannelsWithName(char * loopName);
		void Channel_CreateChannelWithMasterByName(char * channelName, int hidden, int autoclose, char * masterClientName);
		void Channel_CreateChannelWithMasterByID(char * channelName, int hidden, int autoclose, int masterClientID);
		void Channel_KickClientByName(char * clientName);
		void Channel_KickClientByID(int id);
		void Channel_JoinClientByName(char * clientName);
		void Channel_JoinClientByID(int id);
		void Client_Disconnect();
		void Client_SetLocalData(char * key, char * value);
		void Client_JoinToChannel(char * channelNamePtr);
		void Client_LoopJoinedChannels();
		void Client_LoopJoinedChannelsWithName(char * loopName);
		void Client_SelectByName(char * clientName);
		void Client_SelectByID(int clientID);
		void Client_SelectSender();
		void Client_SelectReceiver();
		void LoopAllClients();
		void LoopAllClientsWithName(char * loopName);

		void SendTextToChannel(int subchannel, char * textToSend);
		void SendTextToClient(int subchannel, char * textToSend);
		void SendNumberToChannel(int subchannel, int numToSend);
		void SendNumberToClient(int subchannel, int numToSend);
		void SendBinaryToChannel(int subchannel);
		void SendBinaryToClient(int subchannel);

		void BlastTextToChannel(int subchannel, char * TextToBlast);
		void BlastTextToClient(int subchannel, char * TextToBlast);
		void BlastNumberToChannel(int subchannel, int NumToBlast);
		void BlastNumberToClient(int subchannel, int NumToBlast);
		void BlastBinaryToChannel(int subchannel);
		void BlastBinaryToClient(int subchannel);

		void AddByteText(char * Byte);
		void AddByteInt(int Byte);
		void AddShort(int Short);
		void AddInt(int Int);
		void AddFloat(float Float);
		void AddStringWithoutNull(char * string);
		void AddString(char * string);
		void AddBinary(unsigned int address, int size);
		void AddFileToBinary(char * file);
		void ResizeBinaryToSend(int newSize);
		void CompressSendBinary();
		void ClearBinaryToSend();

		void DecompressReceivedBinary();
		void MoveReceivedBinaryCursor(int Position);
		void SaveReceivedBinaryToFile(int Position, int size, char * Filename);
		void AppendReceivedBinaryToFile(int Position, int size, char * Filename);

	
	/// Conditions

		const bool AlwaysTrue() { return true; }
		const bool AlwaysFalse() { return false; }
		const bool AlwaysFalseWithTextParam(const char *) { return false; }
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
		bool OnAllChannelsLoopWithName(char * loopName);
		bool OnClientsJoinedChannelLoopWithName(char * loopName);
		bool OnAllClientsLoopWithName(char * loopName);
		bool OnChannelClientsLoopWithName(char * loopName);
		bool OnChannelClientsLoopWithNameFinished(char * loopName);
		bool OnAllChannelsLoopWithNameFinished(char * loopName);
		bool OnClientsJoinedChannelLoopWithNameFinished(char * loopName);
		// AlwaysTrue	bool OnChannelsClientLoopFinished(char * loopName);
		// AlwaysTrue	bool OnAllChannelsLoopFinished();
		// AlwaysTrue	bool OnAllClientsLoopFinished();
		// AlwaysTrue	bool OnClientsJoinedChannelLoopFinished();
		bool IsLacewingServerHosting();
		bool IsFlashPolicyServerHosting();
		bool ChannelIsHiddenFromChannelList();
		bool ChannelIsSetToCloseAutomatically();
		bool OnAllClientsLoopWithNameFinished(char * loopName);
		// Added conditions:
		bool IsClientOnChannel_ID(int clientID, char * channelName);
		bool IsClientOnChannel_Name(char * clientName, char * channelName);
		bool DoesChannelNameExist(char * channelName);
		bool DoesChannelIDExist(int channelID);
		bool DoesClientNameExist(char * clientName);
		bool DoesClientIDExist(int clientID);
		bool IsHTML5Hosting();

	/// Expressions

		const char * Error();
		const char * Lacewing_Version();
		unsigned int BinaryToSend_Size();
		const char * RequestedClientName();
		const char * RequestedChannelName();
		const char * Channel_Name();
		unsigned int Channel_ClientCount();
		const char * Client_Name();
		unsigned int Client_ID();
		const char * Client_IP();
		unsigned int Client_ConnectionTime(); // NB: was removed in Lacewing, kept in Bluewing
		unsigned int Client_ChannelCount();
		const char * ReceivedStr();
		int ReceivedInt();
		unsigned int ReceivedBinarySize();
		unsigned int ReceivedBinaryAddress();
		const char * StrByte(int index);
		unsigned int UnsignedByte(int index);
		int SignedByte(int index);
		unsigned int UnsignedShort(int index);
		int SignedShort(int index);
		unsigned int UnsignedInteger(int index);
		int SignedInteger(int index);
		float Float(int index);
		const char * StringWithSize(int index, int size);
		const char * String(int index);
		unsigned int Subchannel();
		unsigned int Channel_Count();
		const char * Client_GetLocalData(char * key);
		const char * CursorStrByte();
		unsigned int CursorUnsignedByte();
		int CursorSignedByte();
		unsigned int CursorUnsignedShort();
		int CursorSignedShort();
		unsigned int CursorUnsignedInteger();
		int CursorSignedInteger();
		float CursorFloat();
		const char * CursorStringWithSize(int size);
		const char * CursorString();
		const char * Client_ProtocolImplementation();
		const char * Channel_GetLocalData(char * key);
		unsigned int Port();
		unsigned int BinaryToSend_Address();
		const char * Welcome_Message();
		// Added expressions:
		const char * DumpMessage(int index, const char * format);
		unsigned int AllClientCount();
		const char * GetDenyReason();
		
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
	lacewing::eventpump			_objEventPump;
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

	template<typename T> struct LocalData
	{
		std::shared_ptr<T> ptr;
		std::string key;
		std::string val;

		LocalData(std::shared_ptr<T> ptr, std::string key, std::string val)
			: ptr(ptr), key(key), val(val) { }
	};
	std::vector<LocalData<lacewing::relayserver::client>> clientLocal;
	std::vector<LocalData<lacewing::relayserver::channel>> channelLocal;

	const std::string& GetLocalData(std::shared_ptr<lacewing::relayserver::client> client, std::string key);
	const std::string& GetLocalData(std::shared_ptr<lacewing::relayserver::channel> channel, std::string key);
	void SetLocalData(std::shared_ptr<lacewing::relayserver::client> client, std::string key, std::string value);
	void SetLocalData(std::shared_ptr<lacewing::relayserver::channel> channel, std::string key, std::string value);
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
	void CreateError(_Printf_format_string_ const char * errorText, ...);
	void CreateError(_Printf_format_string_ const char * errorText, va_list);

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
