class Extension
{
public:

	#define MsgBox(text) MessageBoxA(NULL, text, "DarkSocket Object - Debug information", MB_OK|MB_ICONINFORMATION)
	#define MakeDelim() const char delim [] = "|" //Used for delimiters in tokenizing
	#define FatalBox()	MessageBoxA(NULL, "Fatal error has not been repaired; bypassing erroneous code.", "DarkSocket Object - Bypass notification", MB_OK|MB_ICONERROR);

    RUNDATA * rdPtr;
    RunHeader * rhPtr;

    Edif::Runtime Runtime;

    static const int MinimumBuild = 252;
    static const int Version = 3;

    static const int OEFLAGS = OEFLAG::VALUES;
    static const int OEPREFS = 0;
    
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
	tstring LastError;					// Contains the last error
	tstring CompleteStatus;				// Contains the last report & errors
	tstring LastLockFile;				// For the VS debugger: contains the filename of the last ThreadSafe_Start()
	int LastLockLine;					// For the VS debugger: contains the line of the last ThreadSafe_Start()
	volatile bool threadsafe;			// Handles whether Extension can be read from/written to.
	volatile bool nogevent;				// Handles whether GenerateEvent call is being handled.
	int LastReturnSocketID;				// Indicates what the last SocketID is.
	tstring LastReturnMsg;				// Handles last returned message
	bool LastReturnType;				// Indicates the last message was from a client or server
	vector<CarryMsg> Returns;			// Carries a message from the client/server -> MMF2
	vector<RevCarryMsg> Senders;		// Carries a message from MMF2 -> the client/server
	int NewSocketID;					// The new socket ID is assigned here.
	bool UsePopups;						// This defines whether Report/Explode use popups.
 	WSADATA wsaData;					// Holds WSAStartup() information.
#ifdef AllowTermination
	vector<HANDLE> SocketThreadList;	// Contains handles to threads for terminating later.
#endif
	void * PacketFormLocation;			// Contains the packet forming memory location
	size_t PacketFormSize;				// Contains the packet forming size in bytes
   
	/*  Add your actions, conditions and expressions as real class member
        functions here. The arguments (and return type for expressions) must
        match EXACTLY what you defined in the JSON.

        Remember to link the actions, conditions and expressions to their
        numeric IDs in the class constructor (Extension.cpp)
    */
	/// Functions (called by this object but not through a direct action)
		int Unreferenced_WorkOutAddressFamily(tchar * t);
		int Unreferenced_WorkOutSocketType(tchar * t);
		int Unreferenced_WorkOutProtocolType(tchar * t);
		void Unreferenced_Error(tchar * error, int SocketID);
		void Unreferenced_Report(tchar * report, int SocketID);
		void Unreferenced_ReturnToMMF(int ReturnAsI, int SocketID, void * Msg, int MsgLength);
		unsigned long Unreferenced_WorkOutInAddr(tchar * t);
	
    /// Actions
/* ID = 0 */	void TestReportAndExplode();
/* ID = 1 */	void UsePopupMessages(int OnOrOff);

/* ID = 2 */	void ClientInitialise_Basic(tchar * Hostname, int Port, tchar * Protocol, tchar * InitialText);
/* ID = 3 */	void ClientInitialise_Advanced(tchar * Hostname, int Port, tchar * Protocol, tchar * AddressFamily, tchar * SocketType, tchar * InitialText);
/* ID = 4 */	void ClientShutdownSocket(int SocketID);
/* ID = 5 */	void ClientSend(int SocketID, tchar * Message);
/* ID = 6 */	void ClientGoIndependent(int SocketID);
/* ID = 7 */	void ClientReceiveOnly(int SocketID);
/* ID = 8 */	void ClientLinkFileOutput(int SocketID, tchar * File);
/* ID = 9 */	void ClientUnlinkFileOutput(int SocketID);
/* ID = 10 */	void ClientMMF2Report(int SocketID, int OnOrOff);
		
/* ID = 11 */	void ServerInitialise_Basic(tchar * Protocol, int Port);
/* ID = 12 */	void ServerInitialise_Advanced(tchar * Protocol, tchar * AddressFamily, tchar * SocketType, int Port, tchar * InAddr);
/* ID = 13 */	void ServerShutdownSocket(int SocketID);
/* ID = 14 */	void ServerSend(int SocketID, tchar * Message);
/* ID = 15 */	void ServerGoIndependent(int SocketID);
/* ID = 16 */	void ServerAutoAccept(int SocketID, int OnOrOff);
/* ID = 17 */	void ServerLinkFileOutput(int SocketID, tchar * File);
/* ID = 18 */	void ServerUnlinkFileOutput(int SocketID);
/* ID = 19 */	void ServerMMF2Report(int SocketID, int OnOrOff);

/* ID = 20 */	void PacketForm_NewPacket(size_t Size);
/* ID = 21 */	void PacketForm_ResizePacket(size_t Size);
/* ID = 22 */	void PacketForm_SetByte(unsigned char Byte, size_t WhereTo);
/* ID = 23 */	void PacketForm_SetShort(unsigned short Short, size_t WhereTo, int RunHTON);
/* ID = 24 */	void PacketForm_SetInteger(unsigned int Integer, size_t WhereTo);
/* ID = 25 */	void PacketForm_SetLong(unsigned long LongP, size_t WhereTo, int RunHTON);
/* ID = 26 */	void PacketForm_SetFloat(float Float, size_t WhereTo);
/* ID = 27 */	void PacketForm_SetDouble(double Double, size_t WhereTo);
/* ID = 28 */	void PacketForm_SetString(size_t WhereFrom, size_t WhereTo, size_t SizeOfString);
/* ID = 29 */	void PacketForm_SetWString(size_t WhereFrom, size_t WhereTo, size_t SizeOfStringInChar);
/* ID = 30 */	void PacketForm_SetBankFromBank(size_t WhereFrom, size_t WhereTo, size_t SizeOfString);

		
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
        
/* ID = 0 */	tchar * GetErrors(int clear);
/* ID = 1 */	tchar * GetReports(int clear);

/* ID = 2 */	int GetLastMessageSocketID();
/* ID = 3 */	tchar * GetLastMessageText();
/* ID = 4 */	size_t GetLastMessageAddress();
/* ID = 5 */	size_t GetLastMessageSize();

/* ID = 6 */	int GetNewSocketID();
/* ID = 7 */	int GetSocketIDForLastEvent();
/* ID = 8 */	int GetPortFromType(tchar * t);

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