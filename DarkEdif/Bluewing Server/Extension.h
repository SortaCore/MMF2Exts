#include "Edif.h"

struct GlobalInfo;
struct ClientCopy;
struct ChannelCopy;

class Extension
{
public:
	// Hide stuff requiring other headers
	SaveExtInfo threadData; // Must be first variable in Extension class

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
	
	/*  Add any data you want to store in your extension to this class
		(eg. what you'd normally store in rdPtr).
		
		For those using multi-threading, any variables that are modified
		by the threads should be in SaveExtInfo.
		See MultiThreading.h.

		Unlike rdPtr, you can store real C++ objects with constructors
		and destructors, without having to call them manually or store
		a pointer.
	*/

	bool IsGlobal;
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
	#define Channels					globals->_channels
	#define Clients						globals->_Clients
	
	

	void ClearThreadData();
	void CreateError(const char *);

	void AddToSend(void *, size_t);
	



	// Because Bluewing is multithreaded, and uses a second queue, once we move the variables outside of its
	// functions into DarkEdif, the data may be overwritten, causing crashes and other such nasties.

	// To work around this, we duplicate all the variables, and provide a special event number which will remove
	// all the pointers in SaveExtInfo after they should no longer be valid.
	// In this way, when a client sends message then disconnects, on liblacewing's side that happens instantly,
	// and liblacewing cleans up its client variable instantly too.
	// We can't "deny" a disconnect, so we must accept it immediately.
	//
	// But since these events are queued, the "on client disconnect" may be called later, after liblacewing has deleted.
	// So we have to store a copy of the client so we can look up name and such when it's disconnected.
	// See the _channels and _Clients variables in GlobalInfo for our copies.
	
	// int MyVariable;




	/*  Add your actions, conditions and expressions as real class member
		functions here. The arguments (and return type for expressions) must
		match EXACTLY what you defined in the JSON.

		Remember to link the actions, conditions and expressions to their
		numeric IDs in the class constructor (Extension.cpp)
	*/



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
		void SetWelcomeMessage(char * Message);
		void EnableCondition_OnConnectRequest(int informFusion, int immediateRespondWith, char * autoDenyReason);
		void EnableCondition_OnNameSetRequest(int informFusion, int immediateRespondWith, char * autoDenyReason);
		void EnableCondition_OnJoinChannelRequest(int informFusion, int immediateRespondWith, char * autoDenyReason);
		void EnableCondition_OnLeaveChannelRequest(int informFusion, int immediateRespondWith, char * autoDenyReason);
		void EnableCondition_OnMessageToChannel(int informFusion, int immediateRespondWith);
		void EnableCondition_OnMessageToPeer(int informFusion, int immediateRespondWith);
		void EnableCondition_OnMessageToServer(int informFusion);
		void OnInteractive_Deny(char * Reason);
		void OnInteractive_ChangeClientName(char * NewName);
		void OnInteractive_ChangeChannelName(char * NewName);
		void OnInteractive_DropMessage();
		void OnInteractive_ReplaceMessageWithText(char * NewText);
		void OnInteractive_ReplaceMessageWithNumber(int NewNumber);
		void OnInteractive_ReplaceMessageWithSendBinary();

		void Channel_SelectByName(char * name);
		void Channel_Close();
		void Channel_SelectMaster();
		void Channel_LoopClients();
		void Channel_LoopClientsWithName(char * LoopName);
		void Channel_SetLocalData(char * Key, char * Value);
		void LoopAllChannels();
		void LoopAllChannelsWithName(char * LoopName);
		void Client_Disconnect();
		void Client_SetLocalData(char * Key, char * Value);
		void Client_LoopJoinedChannels();
		void Client_LoopJoinedChannelsWithName(char * LoopName);
		void Client_SelectByName(char * ClientName);
		void Client_SelectByID(int ClientID);
		void Client_SelectSender();
		void Client_SelectReceiver();
		void LoopAllClients();
		void LoopAllClientsWithName(char * LoopName);

		void SendTextToChannel(int subchannel, char * TextToSend);
		void SendTextToClient(int subchannel, char * TextToSend);
		void SendNumberToChannel(int subchannel, int NumToSend);
		void SendNumberToClient(int subchannel, int NumToSend);
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
		void AddStringWithoutNull(char * String);
		void AddString(char * String);
		void AddBinary(unsigned int Address, int size);
		void AddFileToBinary(char * File);
		void ResizeBinaryToSend(int NewSize);
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

template <typename T>
struct LocalData
{
	T * KeyAddr;
	char * Key;
	char * Value;

	LocalData(T * KeyAddr, char * Key, char * Value)
	{
		this->KeyAddr = KeyAddr;
		this->Key = Key;
		this->Value = Value;
	}
	~LocalData()
	{
		free(Key);
		free(Value);
		KeyAddr = nullptr;
		Value = Key = nullptr;
	}
};

struct GlobalInfo {
	lacewing::eventpump			_objEventPump;
	lacewing::relayserver		_server;
	char *						_sendMsg,
		 *						_denyReason,
		 *						_newChannelName,
		 *						_newClientName;
	bool						_dropMessage,
								_automaticallyClearBinary;
	InteractiveType				_interactivePending;
	size_t						_sendMsgSize;

	char *						_globalID;
	HANDLE						_thread;
	Extension *					_ext;
	std::vector<SaveExtInfo *>	_Saved;
	std::vector<ChannelCopy *>	_channels;
	std::vector<ClientCopy *>	_Clients;

	ChannelCopy *				LastDestroyedExtSelectedChannel;
	ClientCopy *				LastDestroyedExtSelectedClient;

	CRITICAL_SECTION			lock;
	std::vector<Extension *>	Refs;
	bool						TimeoutWarningEnabled; // If no Lacewing exists, fuss after set time period
	bool						FullDeleteEnabled; // If no Bluewing exists after DestroyRunObject, clean up GlobalInfo

	void AddEvent1(int Event1,
		ChannelCopy * channel = nullptr,
		ClientCopy * client = nullptr,
		char * MessageOrErrorText = nullptr,
		size_t MessageSize = 0,
		unsigned char subchannel = 255,
		ClientCopy * ReceivingClient = nullptr,
		InteractiveType InteractiveType = InteractiveType::None,
		unsigned char variant = 255,
		bool blasted = false,
		bool channelCreate_Hidden = false, bool channelCreate_AutoClose = false);
	void AddEvent2(int Event1, int Event2,
		ChannelCopy * channel = nullptr,
		ClientCopy *client = nullptr,
		char * MessageOrErrorText = nullptr,
		size_t MessageSize = 0,
		unsigned char subchannel = 255,
		ClientCopy * ReceivingClient = nullptr,
		InteractiveType InteractiveType = InteractiveType::None,
		unsigned char variant = 255,
		bool blasted = false,
		bool channelCreate_Hidden = false, bool channelCreate_AutoClose = false);
	void AddEventF(bool twoEvents, int Event1, int Event2,
		ChannelCopy * channel = nullptr,
		ClientCopy * client = nullptr,
		char * MessageOrErrorText = nullptr,
		size_t MessageSize = 0,
		unsigned char subchannel = 255,
		ClientCopy * ReceivingClient = nullptr,
		InteractiveType InteractiveType = InteractiveType::None,
		unsigned char variant = 255,
		bool blasted = false,
		bool channelCreate_Hidden = false,
		bool channelCreate_AutoClose = false
	);
	void CreateError(const char * errorText);

	enum AutoResponse : char
	{
		Invalid = -1,
		Approve_Quiet = 0,
		Deny_Quiet,
		Approve_TellFusion,
		Deny_TellFusion,
		WaitForFusion
	};

	AutoResponse AutoResponse_Connect;
	const char * AutoResponse_Connect_DenyReason;
	AutoResponse AutoResponse_NameSet;
	const char * AutoResponse_NameSet_DenyReason;
	AutoResponse AutoResponse_ChannelJoin;
	const char * AutoResponse_ChannelJoin_DenyReason;
	AutoResponse autoResponse_ChannelLeave;
	const char * AutoResponse_ChannelLeave_DenyReason;
	AutoResponse AutoResponse_MessageClient;
	AutoResponse AutoResponse_MessageChannel;
	AutoResponse AutoResponse_MessageServer;

	GlobalInfo(Extension * e, EDITDATA * edPtr);
	~GlobalInfo() noexcept(false);
};
