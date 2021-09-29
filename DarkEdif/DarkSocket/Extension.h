#pragma once
class Extension
{
public:

	#define MsgBox(text) ::DarkEdif::MsgBox::Info(_T("Debug info"), _T("%s"), UTF8ToTString(text))
	#define MakeDelim() const char delim [] = "|" // Used for delimiters in tokenizing
	#define FatalBox()	::DarkEdif::MsgBox::Error(_T("Bypass notification"), _T("Fatal error has not been repaired; bypassing erroneous code."));

	RUNDATA * rdPtr;
	RunHeader * rhPtr;

	Edif::Runtime Runtime;

	static const int MinimumBuild = 252;
	static const int Version = 4;

	static const OEFLAGS OEFLAGS = OEFLAGS::VALUES;
	static const OEPREFS OEPREFS = OEPREFS::NONE;

	static const int WindowProcPriority = 100;

	Extension(RUNDATA * rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr);
	~Extension();


	/*  Add any data you want to store in your extension to this class
		(eg. what you'd normally store in rdPtr)

		Unlike rdPtr, you can store real C++ objects with constructors
		and destructors, without having to call them manually or store
		a pointer.
	*/

	// int MyVariable;
	std::tstring LastError;					// Contains the last error
	std::tstring CompleteStatus;			// Contains the last report & errors
	std::tstring LastLockFile;				// For the VS debugger: contains the filename of the last ThreadSafe_Start()
	int LastLockLine;						// For the VS debugger: contains the line of the last ThreadSafe_Start()
	Edif::recursive_mutex threadsafe;		// Handles whether Extension can be read from/written to.
	std::atomic<bool> nogevent;				// Handles whether GenerateEvent call is being handled.
	int LastReturnSocketID;					// Indicates what the last SocketID is.
	std::string LastReturnMsg;				// Handles last returned message
	bool LastReturnType;					// Indicates the last message was from a client or server
	std::vector<CarryMsg> Returns;			// Carries a message from the client/server->Fusion
	std::vector<RevCarryMsg> Senders;		// Carries a message from Fusion->the client/server
	int NewSocketID;						// The new socket ID is assigned here.
 	WSADATA wsaData;						// Holds WSAStartup() information.
	std::vector<std::thread> SocketThreadList;	// Contains handles to threads for terminating later.
	std::string packetForm;

	/*  Add your actions, conditions and expressions as real class member
		functions here. The arguments (and return type for expressions) must
		match EXACTLY what you defined in the JSON.

		Remember to link the actions, conditions and expressions to their
		numeric IDs in the class constructor (Extension.cpp)
	*/
	/// Functions (called by this object but not through a direct action)
		int Internal_WorkOutAddressFamily(const TCHAR * addrFamAsText);
		int Internal_WorkOutSocketType(const TCHAR * sockTypeAsText);
		int Internal_WorkOutProtocolType(const TCHAR * protoTypeAsText);
		void Internal_Error(int SocketID, PrintFHintInside const TCHAR * error, ...) PrintFHintAfter(3,4);
		void Internal_Report(int SocketID, PrintFHintInside const TCHAR * report, ...) PrintFHintAfter(3, 4);
		void Internal_ReturnToFusion(int ReturnAsI, int SocketID, const void * Msg, int MsgLength);
		unsigned long Internal_WorkOutInAddr(const TCHAR * t);




	/// Actions
/* ID = 0 */	void TestReportAndExplode();
/* ID = 1 */	void UsePopupMessages(int OnOrOff);

/* ID = 2 */	void ClientInitialise_Basic(const TCHAR * Hostname, int Port, const TCHAR * Protocol, const TCHAR * InitialText);
/* ID = 3 */	void ClientInitialise_Advanced(const TCHAR * Hostname, int Port, const TCHAR * Protocol, const TCHAR * AddressFamily, const TCHAR * SocketType, const TCHAR * InitialText);
/* ID = 4 */	void ClientShutdownSocket(int SocketID);
/* ID = 5 */	void ClientSend(int SocketID, TCHAR * Message);
/* ID = 6 */	void ClientGoIndependent(int SocketID);
/* ID = 7 */	void ClientReceiveOnly(int SocketID);
/* ID = 8 */	void ClientLinkFileOutput(int SocketID, TCHAR * File);
/* ID = 9 */	void ClientUnlinkFileOutput(int SocketID);
/* ID = 10 */	void ClientFusionReport(int SocketID, int OnOrOff);

/* ID = 11 */	void ServerInitialise_Basic(const TCHAR * Protocol, int Port);
/* ID = 12 */	void ServerInitialise_Advanced(const TCHAR * Protocol, const TCHAR * AddressFamily, const TCHAR * SocketType, int Port, const TCHAR * InAddr);
/* ID = 13 */	void ServerShutdownSocket(int SocketID);
/* ID = 14 */	void ServerSend(int SocketID, const TCHAR * Message);
/* ID = 15 */	void ServerGoIndependent(int SocketID);
/* ID = 16 */	void ServerAutoAccept(int SocketID, int OnOrOff);
/* ID = 17 */	void ServerLinkFileOutput(int SocketID, const TCHAR * File);
/* ID = 18 */	void ServerUnlinkFileOutput(int SocketID);
/* ID = 19 */	void ServerFusionReport(int SocketID, int OnOrOff);

/* ID = 20 */	void PacketForm_NewPacket(int Size);
/* ID = 21 */	void PacketForm_ResizePacket(int Size);
/* ID = 22 */	void PacketForm_SetByte(int Byte, int WhereTo);
/* ID = 23 */	void PacketForm_SetShort(int Short, int WhereTo, int RunHTON);
/* ID = 24 */	void PacketForm_SetInteger(int Integer, int WhereTo);
/* ID = 25 */	void PacketForm_SetLong(int LongP, int WhereTo, int RunHTON);
/* ID = 26 */	void PacketForm_SetFloat(float Float, int WhereTo);
/* ID = 27 */	void PacketForm_SetDouble(float Double, int WhereTo);
/* ID = 28 */	void DEPRECATED_PacketForm_SetString(int WhereFrom, int WhereTo, int SizeOfString);
/* ID = 29 */	void DEPRECATED_PacketForm_SetWString(int WhereFrom, int WhereTo, int SizeOfStringInChar);
/* ID = 30 */	void PacketForm_SetBankFromBank(int WhereFrom, int WhereTo, int SizeOfString);
/* ID = 31 */	void PacketForm_SetString(int WhereFrom, int WhereTo, int includeNull);
/* ID = 32 */	void PacketForm_SetWString(int WhereFrom, int WhereTo, int includeNull);


	/// Conditions

/* ID = 0 */	bool OnError();
/* ID = 1 */	bool OnNewStatus();

/* ID = 8 */	bool ClientSocketConnected(int SocketID);
/* ID = 3 */	bool ClientSocketDisconnected(int SocketID);
/* ID = 2 */	bool ClientReturnedMessage(int SocketID);

/* ID = 4 */	bool ServerReturnedMessage(int SocketID);
/* ID = 5 */	bool ServerSocketDone(int SocketID);
/* ID = 6 */	bool ServerPeerConnected(int SocketID);
/* ID = 7 */	bool ServerPeerDisconnected(int SocketID);



	/// Expressions

/* ID = 0 */	TCHAR * GetErrors(int clear);
/* ID = 1 */	TCHAR * GetReports(int clear);

/* ID = 2 */	int GetLastMessageSocketID();
/* ID = 3 */	TCHAR * GetLastMessageText();
/* ID = 4 */	size_t GetLastMessageAddress();
/* ID = 5 */	size_t GetLastMessageSize();

/* ID = 6 */	int GetNewSocketID();
/* ID = 7 */	int GetSocketIDForLastEvent();
/* ID = 8 */	int GetPortFromType(TCHAR * t);

/* ID = 9 */	size_t PacketForm_GetAddress();
/* ID = 10 */	size_t PacketForm_GetSize();
/* ID = 11 */	unsigned short PacketForm_RunOnesComplement(size_t WhereFrom, size_t SizeOfBank);
/* ID = 12 */	char LastMessage_GetByte(size_t WhereFrom);
/* ID = 13 */	short LastMessage_GetShort(size_t WhereFrom);
/* ID = 14 */	int LastMessage_GetInteger(size_t WhereFrom);
/* ID = 15 */	long LastMessage_GetLong(size_t WhereFrom);
/* ID = 16 */	float LastMessage_GetFloat(size_t WhereFrom);
/* ID = 17 */	double LastMessage_GetDouble(size_t WhereFrom);
/* ID = 18 */	char * LastMessage_GetString(size_t WhereFrom, size_t SizeOfString);
/* ID = 19 */	wchar_t * LastMessage_GetWString(size_t WhereFrom, size_t SizeOfString);

	/* These are called if there's no function linked to an ID */

	void UnlinkedAction(int ID);
	long UnlinkedCondition(int ID);
	long UnlinkedExpression(int ID);


	/*  These replace the functions like HandleRunObject that used to be
		implemented in Runtime.cpp. They work exactly the same, but they're
		inside the extension class.
	*/

	REFLAG Handle();
	REFLAG Display();

	short FusionRuntimePaused();
	short FusionRuntimeContinued();

	bool SaveFramePosition(HANDLE File);
	bool LoadFramePosition(HANDLE File);

};
