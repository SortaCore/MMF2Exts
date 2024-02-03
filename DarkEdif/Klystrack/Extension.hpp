#pragma once
class Extension
{
public:

	RunHeader* rhPtr;
	RunObjectMultiPlat rdPtr; // you should not need to access this

	Edif::Runtime Runtime;

	static const int MinimumBuild = 254;
	static const int Version = 5;

	static const OEFLAGS OEFLAGS = OEFLAGS::NONE;
	static const OEPREFS OEPREFS = OEPREFS::NONE;

	static const int WindowProcPriority = 100;

	Extension(RunObject* const rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr);
	~Extension();

	// Song

	struct ExtKSong {
		KSong * song = nullptr;
		KSongInfo songInfo = {};
		std::tstring songName; // user assigned, not the song title
		std::tstring filePath;
		// Length, in pattern rows.
		int length = 0;
		ExtKSong();
		~ExtKSong();
	};
	struct ExtKPlayer
	{
		std::tstring playerName;
		KPlayer * player = nullptr;
		std::vector<std::unique_ptr<ExtKSong>> songs;
		// Volume, 0 to 128, inclusive.
		int volume = 0;
		// Quality, 0 to 4, inclusive.
		int quality = 0;
		// 0 or 1
		int looping = 0;

		enum PlayState
		{
			Stopped,
			Playing,
			Paused
		} state = Stopped;
		ExtKPlayer();
		~ExtKPlayer();
	};
	std::shared_ptr<ExtKPlayer> curPlayer;

	std::vector<std::shared_ptr<ExtKPlayer>> players;
	void CreateError(PrintFHintInside const TCHAR * format, ...) PrintFHintAfter(2, 3);
	std::tstring lastError;

	/// Actions

		void LoadSongFromFile(const TCHAR * songName, const TCHAR * filePath);
		void LoadSongFromMemory(const TCHAR * songName, unsigned int address, int size);
		void CloseSong(const TCHAR * songName);

		void CreatePlayer(const TCHAR * playerName, int sampleRate);
		void SelectPlayer(const TCHAR * playerName);
		void ClosePlayer();
		void SetCurrentPlayerQuality(int oversample);
		void SetCurrentPlayerLooping(int looping);
		void SetCurrentPlayerVolume(int volume);
		void PlaySongOnCurrentPlayer(const TCHAR * songUserName, int position);
		void SetPauseStateOnCurrentPlayer(int pauseState);
		void StopCurrentPlayer();

	/// Conditions

		bool OnError();
		bool DoesPlayerNameExist(const TCHAR * playerName);
		bool DoesSongNameExist(const TCHAR * songName);
		bool IsPlayerNamePlaying(const TCHAR * playerName);
		bool IsPlayerNamePaused(const TCHAR * playerName);

	/// Expressions

		const TCHAR * GetError();
		int GetPlayerCurrentPos();
		int GetSongLength(const TCHAR * songName);
		const TCHAR * GetSongTitle(const TCHAR * songName);
		int GetSongNumInstruments(const TCHAR * songName);
		int GetSongNumChannels(const TCHAR * songName);
		const TCHAR * GetSongInstrumentName(const TCHAR * songName, int instrumentIndex);



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
