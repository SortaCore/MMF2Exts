#include "Common.h"

///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

Extension::Extension(RUNDATA * _rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr)
	: rdPtr(_rdPtr), rhPtr(_rdPtr->rHo.AdRunHeader), Runtime(&_rdPtr->rHo)
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
	for (int i = 0; i < songInfo.n_instruments; i++)
		free(songInfo.instrument_name[i]);
	KSND_FreeSong(song);
}

REFLAG Extension::Handle()
{
	// Will not call Extension::Handle() next loop
	return REFLAG::ONE_SHOT;
}


// Called when Fusion wants your extension to redraw, due to window scrolling/resize, etc,
// or from you manually causing it.
REFLAG Extension::Display()
{
	// Return REFLAG::DISPLAY in Handle() to run this manually, or use Runtime.Redisplay().

	return REFLAG::NONE;
}

// Called when Fusion runtime is pausing due to the menu option Pause or an extension causing it.
short Extension::FusionRuntimePaused() {
	return 0; // OK
}

// Called when Fusion runtime is resuming after a pause.
short Extension::FusionRuntimeContinued() {
	return 0; // OK
}

// Called when the Fusion runtime executes the "Storyboard > Frame position > Save frame position" action
bool Extension::SaveFramePosition(HANDLE File)
{
	bool OK = false;
	#if defined(_WIN32) && !defined(VITALIZE)
		// Use WriteFile() to save your data.
		OK = true;
	#endif
	return OK;
}

// Called when the Fusion runtime executes the "Storyboard > Frame position > Load frame/app position" action
bool Extension::LoadFramePosition(HANDLE File)
{
	bool OK = false;
	#if defined(_WIN32) && !defined(VITALIZE)
		// Use ReadFile() to read your data.
		OK = true;
	#endif
	return OK;
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
	if ((size_t)ID < ::SDK->ExpressionInfos.size() && ::SDK->ExpressionInfos[ID]->Flags.ef == ExpReturnType::String)
		return (long)Runtime.CopyString(_T(""));
	return 0;
}
