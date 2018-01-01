
#include "Common.h"

const char * Extension::GetError()
{
	return Runtime.CopyString(lastError.c_str());
}
int Extension::GetPlayerCurrentPos()
{
	CheckForPlayer(true, "GetPlayerCurrentPos", -1);
	try {
		return KSND_GetPlayPosition(curPlayer->player);
	}
	catch (...)
	{
		CreateError("Failed to get current player position.");
		return -1;
	}
}
int Extension::GetSongLength(const char * songName)
{
	CheckForPlayer(true, "GetSongLength", -1);
	auto song = std::find_if(curPlayer->songs.cbegin(), curPlayer->songs.cend(), [=](const ExtKSong *song) {
		return !_stricmp(song->songName.c_str(), songName);
	});

	if (song == curPlayer->songs.cend())
	{
		CreateError("PlaySongOnCurrentPlayer: Song with name %s not found.", songName);
		return -1;
	}

	return (**song).length;
}
const char * Extension::GetSongTitle(const char * songName)
{
	CheckForPlayer(true, "GetSongTitle", Runtime.CopyString(_T("")));
	auto song = std::find_if(curPlayer->songs.cbegin(), curPlayer->songs.cend(), [=](const ExtKSong *song) {
		return !_stricmp(song->songName.c_str(), songName);
	});

	if (song == curPlayer->songs.cend())
	{
		CreateError("GetSongTitle: Song with name %s not found.", songName);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString((**song).songInfo.song_title);
}
int Extension::GetSongNumInstruments(const char * songName)
{
	CheckForPlayer(true, "GetSongNumInstruments", -1);
	auto song = std::find_if(curPlayer->songs.cbegin(), curPlayer->songs.cend(), [=](const ExtKSong *song) {
		return !_stricmp(song->songName.c_str(), songName);
	});

	if (song == curPlayer->songs.cend())
	{
		CreateError("GetSongTitle: Song with name %s not found.", songName);
		return -1;
	}
	return (**song).songInfo.n_instruments;
}
int Extension::GetSongNumChannels(const char * songName)
{
	CheckForPlayer(true, "GetSongNumChannels", -1);
	auto song = std::find_if(curPlayer->songs.cbegin(), curPlayer->songs.cend(), [=](const ExtKSong *song) {
		return !_stricmp(song->songName.c_str(), songName);
	});

	if (song == curPlayer->songs.cend())
	{
		CreateError("GetSongNumChannels: Song with name %s not found.", songName);
		return -1;
	}
	return (**song).songInfo.n_channels;
}
const char * Extension::GetSongInstrumentName(const char * songName, int instrumentIndex)
{
	CheckForPlayer(true, "GetSongInstrumentName", Runtime.CopyString(_T("")));
	auto song = std::find_if(curPlayer->songs.cbegin(), curPlayer->songs.cend(), [=](const ExtKSong *song) {
		return !_stricmp(song->songName.c_str(), songName);
	});

	if (song == curPlayer->songs.cend())
	{
		CreateError("GetSongInstrumentName: Song with name %s not found.", songName);
		return Runtime.CopyString(_T(""));
	}
	if (instrumentIndex < 0)
	{
		CreateError("GetSongInstrumentName: Instrument index must be 0 or greater.", songName);
		return Runtime.CopyString(_T(""));
	}
	if (instrumentIndex > (**song).songInfo.n_instruments - 1)
	{
		CreateError("GetSongInstrumentName: Instrument index %i is beyond valid range 0 to %i.", instrumentIndex, (**song).songInfo.n_instruments - 1);
		return Runtime.CopyString(_T(""));
	}
	return Runtime.CopyString((**song).songInfo.instrument_name[instrumentIndex]);
}