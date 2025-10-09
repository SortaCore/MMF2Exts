#pragma once
#include "Common.hpp"

class Extension final
{
public:
	// ======================================
	// Required variables + functions
	// Variables here must not be moved or swapped around or it can cause future issues
	// ======================================
	RunHeader* rhPtr;
	RunObjectMultiPlatPtr rdPtr;

	Edif::Runtime Runtime;

	static const int MinimumBuild = 254;
	static const int Version = 5;

	static constexpr OEFLAGS OEFLAGS = OEFLAGS::NONE;
	static constexpr OEPREFS OEPREFS = OEPREFS::NONE;

	Extension(RunObject* const rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr);
	~Extension();

	// ======================================
	// Extension data
	// ======================================

	struct ExtKSong final
	{
		KSong * song = nullptr;
		KSongInfo songInfo = {};
		std::tstring songName; // user assigned, not the song title
		std::tstring filePath;
		// Length, in pattern rows.
		int length = 0;
		ExtKSong();
		~ExtKSong();
	};
	struct ExtKPlayer final
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

	// Actions
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

	// Conditions
	bool OnError();
	bool DoesPlayerNameExist(const TCHAR * playerName);
	bool DoesSongNameExist(const TCHAR * songName);
	bool IsPlayerNamePlaying(const TCHAR * playerName);
	bool IsPlayerNamePaused(const TCHAR * playerName);

	// Expressions
	const TCHAR * GetError();
	int GetPlayerCurrentPos();
	int GetSongLength(const TCHAR * songName);
	const TCHAR * GetSongTitle(const TCHAR * songName);
	int GetSongNumInstruments(const TCHAR * songName);
	int GetSongNumChannels(const TCHAR * songName);
	const TCHAR * GetSongInstrumentName(const TCHAR * songName, int instrumentIndex);

	// Runs every tick of Fusion's runtime, can be toggled off and back on
	REFLAG Handle();

	// These are called if there's no function linked to an ID

	void UnlinkedAction(int ID);
	long UnlinkedCondition(int ID);
	long UnlinkedExpression(int ID);
};
