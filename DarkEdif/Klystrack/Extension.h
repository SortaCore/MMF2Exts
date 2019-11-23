class Extension
{
public:

	RUNDATA * rdPtr;
	RunHeader *	rhPtr;

	Edif::Runtime Runtime;

	static const int MinimumBuild = 254;
	static const int Version = 2;

	static const OEFLAGS OEFLAGS = OEFLAGS::NONE;
	static const OEPREFS OEPREFS = OEPREFS::NONE;
	
	static const int WindowProcPriority = 100;

	Extension(RUNDATA * rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr);
	~Extension();

	// Song

	struct ExtKSong {
		KSong * song;
		KSongInfo songInfo;
		std::string songName; // user assigned, not the song title
		std::string filePath;
		// Length, in pattern rows.
		int length;
		ExtKSong();
		~ExtKSong();
	};
	struct ExtKPlayer
	{
		std::string playerName;
		KPlayer * player;
		std::vector<ExtKSong *> songs;
		// Volume, 0 to 128, inclusive.
		int volume;
		// Quality, 0 to 4, inclusive.
		int quality;
		// 0 or 1
		int looping;

		enum PlayState
		{
			Stopped,
			Playing,
			Paused
		} state;
		ExtKPlayer();
		~ExtKPlayer();
	} *curPlayer;

	std::vector<ExtKPlayer *> players;
	void CreateError(const char * format, ...);
	std::string lastError;

	/// Actions

		void LoadSongFromFile(const char * songName, const char * filePath);
		void LoadSongFromMemory(const char * songName, unsigned int address, int size);
		void CloseSong(const char * songName);

		void CreatePlayer(const char * playerName, int sampleRate);
		void SelectPlayer(const char * playerName);
		void ClosePlayer();
		void SetCurrentPlayerQuality(int oversample);
		void SetCurrentPlayerLooping(int looping);
		void SetCurrentPlayerVolume(int volume);
		void PlaySongOnCurrentPlayer(const char * songUserName, int position);
		void SetPauseStateOnCurrentPlayer(int pauseState);
		void StopCurrentPlayer();

	/// Conditions

		bool OnError();
		bool DoesPlayerNameExist(const char * playerName);
		bool DoesSongNameExist(const char * songName);
		bool IsPlayerNamePlaying(const char * playerName);
		bool IsPlayerNamePaused(const char * playerName);

	/// Expressions
		
		const char * GetError();
		int GetPlayerCurrentPos();
		int GetSongLength(const char * songName);
		const char * GetSongTitle(const char * songName);
		int GetSongNumInstruments(const char * songName);
		int GetSongNumChannels(const char * songName);
		const char * GetSongInstrumentName(const char * songName, int instrumentIndex);



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
