
#include "Edif.h"
class Extension
{
public:
	// Hide stuff requiring other headers
	SaveExtInfo ThreadData; // Must be first variable in Extension class
	std::vector<SaveExtInfo *> Saved;
	SaveExtInfo &AddEvent(int Event, bool UseLastData = false);
	void NewEvent(SaveExtInfo *);
	CRITICAL_SECTION Lock;

    RUNDATA * rdPtr;
    RunHeader * rhPtr;

    Edif::Runtime Runtime;

    static const int MinimumBuild = 251;
    static const int Version = 1;

    static const int OEFLAGS = 0; // Use OEFLAGS namespace
    static const int OEPREFS = 0; // Use OEPREFS namespace
    
    static const int WindowProcPriority = 100;

    Extension(RUNDATA * rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr);
    ~Extension();

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

	enum InteractiveType : unsigned char
	{
		None = 0x0,
		DenyPermitted = 0x1,
		PeerName = 0x2,
		ChannelName = 0x4,
		MessageDrop = 0x8,
	};


    /*  Add any data you want to store in your extension to this class
        (eg. what you'd normally store in rdPtr).
		
		For those using multi-threading, any variables that are modified
		by the threads should be in SaveExtInfo.
		See MultiThreading.h.

        Unlike rdPtr, you can store real C++ objects with constructors
        and destructors, without having to call them manually or store
        a pointer.
    */
	struct GlobalInfo {
		lacewing::eventpump		_ObjEventPump;
		lacewing::relayserver	_Server;
		lacewing::flashpolicy	_Policy;
		char *					_SendMsg,
			 *					_DenyReason,
			 *					_NewChannelName,
			 *					_NewPeerName;
		bool					_DropMessage,
								_AutomaticallyClearBinary;
		InteractiveType			_InteractivePending;
		size_t					_SendMsgSize;
		
		char *					_GlobalID;
		HANDLE					_Thread;

		std::list<LocalData<lacewing::relayserver::channel>> channelsLocalData;
		std::list<LocalData<lacewing::relayserver::client>> clientsLocalData;
		
		GlobalInfo() : _Server(_ObjEventPump), _Policy((lacewing::flashpolicy)flashpolicy_new(_ObjEventPump)),
			_SendMsg(NULL), _DenyReason(NULL), _NewChannelName(NULL),
			_NewPeerName(NULL), _InteractivePending(InteractiveType::None),
			_SendMsgSize(0), _AutomaticallyClearBinary(false), _GlobalID(NULL), _Thread(NULL) {}
	} * Globals;

	// This allows prettier and more readable access while maintaining global variables.
	#define ObjEventPump				Globals->_ObjEventPump
	#define Srv							Globals->_Server
	#define FlashSrv					Globals->_Policy
	#define SendMsg						Globals->_SendMsg
	#define DenyReason					Globals->_DenyReason
	#define SendMsgSize					Globals->_SendMsgSize
	#define AutomaticallyClearBinary	Globals->_AutomaticallyClearBinary
	#define GlobalID					Globals->_GlobalID
	#define NewChannelName				Globals->_NewChannelName
	#define NewPeerName					Globals->_NewPeerName
	#define DropMessage					Globals->_DropMessage
	#define InteractivePending			Globals->_InteractivePending
	
	

	void ClearThreadData();
	void CreateError(const char *);

	void AddToSend(void *, size_t);
	

	// Because Lacewing is singlethreaded, once we move the variables outside of its functions into DarkEDIF,
	// the data may be overwritten, causing crashes and other such nasties.
	// To work around this, we duplicate all the variables, and provide a special event number which will remove
	// all the pointers in SaveExtInfo after they should no longer be valid.
	// In this way, when a peer is disconnected then a channel left, both can be queried properly while as far as
	// Lacewing is concerned they no longer exist.

	std::vector<lacewing::relayserver::channel *> Channels;
	std::vector<lacewing::relayserver::client *> Clients;

    // int MyVariable;




    /*  Add your actions, conditions and expressions as real class member
        functions here. The arguments (and return type for expressions) must
        match EXACTLY what you defined in the JSON.

        Remember to link the actions, conditions and expressions to their
        numeric IDs in the class constructor (Extension.cpp)
    */



    /// Actions

		void RelayServer_Host(int port);
		void RelayServer_StopHosting();
		void FlashServer_Host(char * path);
		void FlashServer_StopHosting();
		void HTML5Server_EnableHosting();
		void HTML5Server_DisableHosting();
		void ChannelListing_Enable();
		void ChannelListing_Disable();
		void SetWelcomeMessage(char * Message);
		void EnableCondition_OnMessageToChannel();
		void EnableCondition_OnMessageToPeer();
		void OnInteractive_Deny(char * Reason);
		void OnInteractive_ChangePeerName(char * NewName);
		void OnInteractive_ChangeChannelName(char * NewName);
		void OnInteractive_DropMessage();

		void Channel_SelectByName(char * Name);
		void Channel_Close();
		void Channel_SelectMaster();
		void Channel_LoopClients();
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

		void SendTextToChannel(int Subchannel, char * TextToSend);
		void SendTextToClient(int Subchannel, char * TextToSend);
		void SendNumberToChannel(int Subchannel, int NumToSend);
		void SendNumberToClient(int Subchannel, int NumToSend);
		void SendBinaryToChannel(int Subchannel);
		void SendBinaryToClient(int Subchannel);

		void BlastTextToChannel(int Subchannel, char * TextToBlast);
		void BlastTextToClient(int Subchannel, char * TextToBlast);
		void BlastNumberToChannel(int Subchannel, int NumToBlast);
		void BlastNumberToClient(int Subchannel, int NumToBlast);
		void BlastBinaryToChannel(int Subchannel);
		void BlastBinaryToClient(int Subchannel);

		void AddByteText(char * Byte);
		void AddByteInt(int Byte);
		void AddShort(int Short);
		void AddInt(int Int);
		void AddFloat(float Float);
		void AddStringWithoutNull(char * String);
		void AddString(char * String);
		void AddBinary(void * Address, int Size);
		void AddFileToBinary(char * File);
		void ResizeBinaryToSend(int NewSize);
		void CompressSendBinary();
		void ClearBinaryToSend();

		void DecompressReceivedBinary();
		void MoveReceivedBinaryCursor(int Position);
		void SaveReceivedBinaryToFile(int Position, int Size, char * Filename);
		void AppendReceivedBinaryToFile(int Position, int Size, char * Filename);

    
	/// Conditions

		const bool AlwaysTrue() { return true; }
		const bool AlwaysFalse() { return false; }
		bool SubchannelMatches(int Subchannel);
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
		// MessageMatches:	bool OnSentTextMessageToServer(int Subchannel);
		// MessageMatches:	bool OnSentNumberMessageToServer(int Subchannel);
		// MessageMatches:	bool OnSentBinaryMessageToServer(int Subchannel);
		// MessageMatches:	bool OnAnySentMessageToServer(int Subchannel);
		// MessageMatches:	bool OnSentTextMessageToChannel(int Subchannel);
		// MessageMatches:	bool OnSentNumberMessageToChannel(int Subchannel);
		// MessageMatches:	bool OnSentBinaryMessageToChannel(int Subchannel);
		// MessageMatches:	bool OnAnySentMessageToChannel(int Subchannel);
		// MessageMatches:	bool OnSentTextMessageToPeer(int Subchannel);
		// MessageMatches:	bool OnSentNumberMessageToPeer(int Subchannel);
		// MessageMatches:	bool OnSentBinaryMessageToPeer(int Subchannel);
		// MessageMatches:	bool OnAnySentMessageToPeer(int Subchannel);
		// MessageMatches:	bool OnBlastedTextMessageToServer(int Subchannel);
		// MessageMatches:	bool OnBlastedNumberMessageToServer(int Subchannel);
		// MessageMatches:	bool OnBlastedBinaryMessageToServer(int Subchannel);
		// MessageMatches:	bool OnAnyBlastedMessageToServer(int Subchannel);
		// MessageMatches:	bool OnBlastedTextMessageToChannel(int Subchannel);
		// MessageMatches:	bool OnBlastedNumberMessageToChannel(int Subchannel);
		// MessageMatches:	bool OnBlastedBinaryMessageToChannel(int Subchannel);
		// MessageMatches:	bool OnAnyBlastedMessageToChannel(int Subchannel);
		// MessageMatches:	bool OnBlastedTextMessageToPeer(int Subchannel);
		// MessageMatches:	bool OnBlastedNumberMessageToPeer(int Subchannel);
		// MessageMatches:	bool OnBlastedBinaryMessageToPeer(int Subchannel);
		// MessageMatches:	bool OnAnyBlastedMessageToPeer(int Subchannel);
		bool OnAllChannelsLoopWithName(char * LoopName);
		bool OnClientsJoinedChannelLoopWithName(char * LoopName);
		bool OnAllClientsLoopWithName(char * LoopName);
		bool OnChannelClientsLoopWithName(char * LoopName);
		bool OnChannelClientsLoopWithNameFinished(char * LoopName);
		bool OnAllChannelsLoopWithNameFinished(char * LoopName);
		bool OnClientsJoinedChannelLoopWithNameFinished(char * LoopName);
		bool OnChannelsClientLoopFinished(char * LoopName);
		// AlwaysTrue	bool OnAllChannelsLoopFinished();
		// AlwaysTrue	bool OnAllClientsLoopFinished();
		// AlwaysTrue	bool OnClientsJoinedChannelLoopFinished();
		bool IsLacewingServerHosting();
		bool IsFlashPolicyServerHosting();
		bool ChannelIsHiddenFromChannelList();
		bool ChannelIsSetToCloseAutomatically();
		bool OnAllClientsLoopWithNameFinished(char * LoopName);
		// Added conditions:
		bool IsClientOnChannel_ID(int ClientID, char * ChannelName);
		bool IsClientOnChannel_Name(char * ClientName, char * ChannelName);
		bool IsHTML5Hosting();

    /// Expressions

		const char * Error();
		const char * Lacewing_Version();
		unsigned int BinaryToSend_Size();
		const char * RequestedPeerName();
		const char * RequestedChannelName();
		const char * Channel_Name();
		int Channel_ClientCount();
		const char * Client_Name();
		int Client_ID();
		const char * Client_IP();
		int Client_ConnectionTime(); // NB: was removed.
		int Client_ChannelCount();
		const char * ReceivedStr();
		int ReceivedInt();
		unsigned int ReceivedBinarySize();
		long ReceivedBinaryAddress();
		const char * StrByte(int Index);
		unsigned int UnsignedByte(int Index);
		int SignedByte(int Index);
		unsigned int UnsignedShort(int Index);
		int SignedShort(int Index);
		unsigned int UnsignedInteger(int Index);
		int SignedInteger(int Index);
		float Float(int Index);
		const char * StringWithSize(int Index, int Size);
		const char * String(int Index);
		int Subchannel();
		int Channel_Count();
		const char * Client_GetLocalData(char * Key);
		const char * CursorStrByte();
		unsigned int CursorUnsignedByte();
		int CursorSignedByte();
		unsigned int CursorUnsignedShort();
		int CursorSignedShort();
		unsigned int CursorUnsignedInteger();
		int CursorSignedInteger();
		float CursorFloat();
		const char * CursorStringWithSize(int Size);
		const char * CursorString();
		const char * Client_ProtocolImplementation();
		const char * Channel_GetLocalData(char * Key);
		int Port();
		long BinaryToSend_Address();
		// Added expressions:
		const char * DumpMessage(int Index, const char * Format);
		const char * Welcome_Message();
		const char * Client_Platform(); 
		
    /* These are called if there's no function linked to an ID */

    void Action(int ID, RUNDATA * rdPtr, long param1, long param2);
    long Condition(int ID, RUNDATA * rdPtr, long param1, long param2);
    long Expression(int ID, RUNDATA * rdPtr, long param);

	

    /*  These replace the functions like HandleRunObject that used to be
        implemented in Runtime.cpp. They work exactly the same, but they're
        inside the extension class.
    */

    short Handle();
    short Display();

    short Pause();
    short Continue();

    bool Save(HANDLE File);
    bool Load(HANDLE File);

};
