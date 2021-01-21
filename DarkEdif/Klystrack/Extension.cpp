#include "Common.h"

///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

Extension::Extension(RUNDATA * _rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr)
	: rdPtr(_rdPtr), rhPtr(_rdPtr->rHo.AdRunHeader), Runtime(_rdPtr)
{
	LinkAction(0, LoadSongFromFile);
	LinkAction(1, LoadSongFromMemory);
	LinkAction(2, CloseSong);
	LinkAction(3, CreatePlayer);
	LinkAction(4, SelectPlayer);
	LinkAction(5, ClosePlayer);
	LinkAction(6, SetCurrentPlayerQuality);
	LinkAction(7, SetCurrentPlayerLooping);
	LinkAction(8, SetCurrentPlayerVolume);
	LinkAction(9, PlaySongOnCurrentPlayer);
	LinkAction(10, SetPauseStateOnCurrentPlayer);
	LinkAction(11, StopCurrentPlayer);

	LinkCondition(0, OnError);
	LinkCondition(1, DoesPlayerNameExist);
	LinkCondition(2, DoesSongNameExist);
	LinkCondition(3, IsPlayerNamePlaying);
	LinkCondition(4, IsPlayerNamePaused);

	LinkExpression(0, GetError);
	LinkExpression(1, GetPlayerCurrentPos);
	LinkExpression(2, GetSongLength);
	LinkExpression(3, GetSongTitle);
	LinkExpression(4, GetSongNumInstruments);
	LinkExpression(5, GetSongNumChannels);
	LinkExpression(6, GetSongInstrumentName);

	curPlayer = nullptr;
}

void Extension::CreateError(const char * format, ...)
{
	va_list v;
	va_start(v, format);

	static char errorBuffer[512];
	vsprintf_s(errorBuffer, format, v);
	lastError = errorBuffer;

	Runtime.GenerateEvent(0);
}

Extension::~Extension()
{
	curPlayer = nullptr;
	for (size_t i = 0; i < players.size(); i++)
		delete players[i];
	players.clear();
}

Extension::ExtKPlayer::ExtKPlayer()
{
	memset(this, 0, sizeof(*this));
}
Extension::ExtKPlayer::~ExtKPlayer()
{
	for (size_t i = 0; i < songs.size(); i++)
		delete songs[i];
	KSND_FreePlayer(player);
	memset(this, 0, sizeof(*this));
}

Extension::ExtKSong::ExtKSong()
{
	memset(this, 0, sizeof(*this));
}
Extension::ExtKSong::~ExtKSong()
{
	free(songInfo.song_title);
	for (int i = 0; i < songInfo.n_instruments; i++)
		free(songInfo.instrument_name[i]);
	KSND_FreeSong(song);
	memset(this, 0, sizeof(*this));
}

REFLAG Extension::Handle()
{
	// Will not be called next loop
	return REFLAG::ONE_SHOT;
}


REFLAG Extension::Display()
{
	/*
		If you return REFLAG_DISPLAY in Handle() this routine will run.
	*/

	// Ok
	return REFLAG::NONE;
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


