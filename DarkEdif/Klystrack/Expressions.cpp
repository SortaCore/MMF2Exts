#include "Common.hpp"

const TCHAR * Extension::GetError()
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
		CreateError(_T("Failed to get current player position."));
		return -1;
	}
}
int Extension::GetSongLength(const TCHAR * songName)
{
	CheckForPlayer(true, "GetSongLength", -1);
	auto song = std::find_if(curPlayer->songs.cbegin(), curPlayer->songs.cend(), [=](const std::unique_ptr<ExtKSong> &song) {
		return !_tcsicmp(song->songName.c_str(), songName);
	});

	if (song == curPlayer->songs.cend())
	{
		CreateError(_T("PlaySongOnCurrentPlayer: Song with name %s not found."), songName);
		return -1;
	}

	return (**song).length;
}
const TCHAR * Extension::GetSongTitle(const TCHAR * songName)
{
	CheckForPlayer(true, "GetSongTitle", Runtime.CopyString(_T("")));
	auto song = std::find_if(curPlayer->songs.cbegin(), curPlayer->songs.cend(), [=](const std::unique_ptr<ExtKSong> &song) {
		return !_tcsicmp(song->songName.c_str(), songName);
	});

	if (song == curPlayer->songs.cend())
	{
		CreateError(_T("GetSongTitle: Song with name %s not found."), songName);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(DarkEdif::UTF8ToTString((**song).songInfo.song_title).c_str());
}
int Extension::GetSongNumInstruments(const TCHAR * songName)
{
	CheckForPlayer(true, "GetSongNumInstruments", -1);
	auto song = std::find_if(curPlayer->songs.cbegin(), curPlayer->songs.cend(), [=](const std::unique_ptr<ExtKSong> &song) {
		return !_tcsicmp(song->songName.c_str(), songName);
	});

	if (song == curPlayer->songs.cend())
	{
		CreateError(_T("GetSongTitle: Song with name %s not found."), songName);
		return -1;
	}
	return (**song).songInfo.n_instruments;
}
int Extension::GetSongNumChannels(const TCHAR * songName)
{
	CheckForPlayer(true, "GetSongNumChannels", -1);
	auto song = std::find_if(curPlayer->songs.cbegin(), curPlayer->songs.cend(), [=](const std::unique_ptr<ExtKSong> &song) {
		return !_tcsicmp(song->songName.c_str(), songName);
	});

	if (song == curPlayer->songs.cend())
	{
		CreateError(_T("GetSongNumChannels: Song with name %s not found."), songName);
		return -1;
	}
	return (**song).songInfo.n_channels;
}
const TCHAR * Extension::GetSongInstrumentName(const TCHAR * songName, int instrumentIndex)
{
	CheckForPlayer(true, "GetSongInstrumentName", Runtime.CopyString(_T("")));
	auto song = std::find_if(curPlayer->songs.cbegin(), curPlayer->songs.cend(), [=](const std::unique_ptr<ExtKSong> &song) {
		return !_tcsicmp(song->songName.c_str(), songName);
	});

	if (song == curPlayer->songs.cend())
	{
		CreateError(_T("GetSongInstrumentName: Song with name %s not found."), songName);
		return Runtime.CopyString(_T(""));
	}
	if (instrumentIndex < 0)
	{
		CreateError(_T("GetSongInstrumentName: Instrument index must be 0 or greater."), songName);
		return Runtime.CopyString(_T(""));
	}
	if (instrumentIndex > (**song).songInfo.n_instruments - 1)
	{
		CreateError(_T("GetSongInstrumentName: Instrument index %i is beyond valid range 0 to %i."), instrumentIndex, (**song).songInfo.n_instruments - 1);
		return Runtime.CopyString(_T(""));
	}
	return Runtime.CopyString(DarkEdif::UTF8ToTString((**song).songInfo.instrument_name[instrumentIndex]).c_str());
}
