#include "Common.hpp"

///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

Extension::Extension(RunObject* const _rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr) :
	rdPtr(_rdPtr), rhPtr(_rdPtr->get_rHo()->get_AdRunHeader()), Runtime(this)
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

void Extension::CreateError(PrintFHintInside const TCHAR * format, ...)
{
	va_list v;
	va_start(v, format);

	static TCHAR errorBuffer[512];
	_vstprintf_s(errorBuffer, format, v);
	lastError = errorBuffer;

	Runtime.GenerateEvent(0);
}

Extension::~Extension()
{
	curPlayer.reset();
	players.clear();
}

Extension::ExtKPlayer::ExtKPlayer()
{
}
Extension::ExtKPlayer::~ExtKPlayer()
{
	songs.clear();
	KSND_FreePlayer(player);
}

Extension::ExtKSong::ExtKSong()
{
}
Extension::ExtKSong::~ExtKSong()
{
	free(songInfo.song_title);
	for (int i = 0; i < songInfo.n_instruments; ++i)
		free(songInfo.instrument_name[i]);
	KSND_FreeSong(song);
}

// Runs every tick of Fusion's runtime, can be toggled off and back on
REFLAG Extension::Handle()
{
	// Will not call Extension::Handle() next loop
	return REFLAG::ONE_SHOT;
}


// These are called if there's no function linked to an ID

void Extension::UnlinkedAction(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedAction() called"), _T("Running a fallback for action ID %d. Make sure you ran LinkAction()."), ID);
}

long Extension::UnlinkedCondition(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedCondition() called"), _T("Running a fallback for condition ID %d. Make sure you ran LinkCondition()."), ID);
	return 0;
}

long Extension::UnlinkedExpression(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedExpression() called"), _T("Running a fallback for expression ID %d. Make sure you ran LinkExpression()."), ID);
	// Unlinked A/C/E is fatal error , but try not to return null string and definitely crash it
	if ((size_t)ID < Edif::SDK->ExpressionInfos.size() && Edif::SDK->ExpressionInfos[ID]->Flags.ef == ExpReturnType::String)
		return (long)Runtime.CopyString(_T(""));
	return 0;
}
