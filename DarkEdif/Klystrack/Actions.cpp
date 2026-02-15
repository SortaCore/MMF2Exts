#include "Common.hpp"

// This is used in KSND, seems to be a GCC x64 -> x86 *nix weirdness.
extern "C" {
	__declspec (dllexport) unsigned long __udivdi3(unsigned long long int x, unsigned long int y)
	{
		return (unsigned long)x / y;
	}
	__declspec (dllexport) unsigned long __umoddi3(unsigned long long int x, unsigned long int y)
	{
		return (unsigned long)x % y;
	}
}

void Extension::LoadSongFromFile(const TCHAR * songName, const TCHAR * filePath)
{
	CheckForPlayer(true, "LoadSongFromFile");
	auto songIfAlreadyLoaded = std::find_if(curPlayer->songs.cbegin(), curPlayer->songs.cend(), [=](const std::unique_ptr<ExtKSong> &song) {
		return !_tcsicmp(song->songName.c_str(), songName);
	});
	if (songIfAlreadyLoaded != curPlayer->songs.cend())
		return CreateError(_T("LoadSongFromFile: Song with name %s is already in use."), songName);

	songIfAlreadyLoaded = std::find_if(curPlayer->songs.cbegin(), curPlayer->songs.cend(), [=](const std::unique_ptr<ExtKSong> & song) {
		return !_tcsicmp(song->filePath.c_str(), filePath);
	});
	if (songIfAlreadyLoaded != curPlayer->songs.cend())
		return CreateError(_T("LoadSongFromFile: File %s is already opened in this player, under song name %s."), filePath, (**songIfAlreadyLoaded).songName.c_str());

	// Good to go
	KSong * newSong = nullptr;
	try {
		newSong = KSND_LoadSong(curPlayer->player, DarkEdif::TStringToUTF8(filePath).c_str());
	}
	catch (...)
	{
		return CreateError(_T("LoadSongFromFile: Failed to load song."));
	}
	if (!newSong)
		return CreateError(_T("LoadSongFromFile: Failed to load song."));

	// This is used in KSND, but you'll get the KSND library unable to find it unless we use it ourselves.
	printf("yay");

	std::unique_ptr<ExtKSong> newExtSong = std::make_unique<ExtKSong>();
	newExtSong->filePath = filePath;
	newExtSong->songName = songName;
	newExtSong->song = newSong;
	newExtSong->length = KSND_GetSongLength(newSong);
	// Uses dodgy memory in strings from KSND, so copy it JIC
	KSND_GetSongInfo(newSong, &newExtSong->songInfo);
	newExtSong->songInfo.song_title = _strdup(newExtSong->songInfo.song_title);
	for (int i = 0; i < newExtSong->songInfo.n_instruments; ++i)
		newExtSong->songInfo.instrument_name[i] = _strdup(newExtSong->songInfo.instrument_name[i]);

	curPlayer->songs.push_back(std::move(newExtSong));
}
void Extension::LoadSongFromMemory(const TCHAR * songName, unsigned int address, int size)
{
CheckForPlayer(true, "LoadSongFromMemory");
	auto songIfAlreadyLoaded = std::find_if(curPlayer->songs.cbegin(), curPlayer->songs.cend(), [=](const std::unique_ptr<ExtKSong> & song) {
		return !_tcsicmp(song->songName.c_str(), songName);
	});
	if (songIfAlreadyLoaded != curPlayer->songs.cend())
		return CreateError(_T("LoadSongFromMemory: Song with name %s is already in use."), songName);

	// Good to go
	KSong * newSong = nullptr;
	try {
		newSong = KSND_LoadSongFromMemory(curPlayer->player, (void *)(long)address, size);
	}
	catch (...)
	{
		return CreateError(_T("LoadSongFromMemory: Failed to load song."));
	}
	if (!newSong)
		return CreateError(_T("LoadSongFromMemory: Failed to load song."));

	std::unique_ptr<ExtKSong> newExtSong = std::make_unique<ExtKSong>();
	newExtSong->songName = songName;
	newExtSong->song = newSong;
	newExtSong->length = KSND_GetSongLength(newSong);
	// Uses dodgy memory in strings from KSND, so copy it JIC
	KSND_GetSongInfo(newSong, &newExtSong->songInfo);
	newExtSong->songInfo.song_title = _strdup(newExtSong->songInfo.song_title);
	for (int i = 0; i < newExtSong->songInfo.n_instruments; ++i)
		newExtSong->songInfo.instrument_name[i] = _strdup(newExtSong->songInfo.instrument_name[i]);

	curPlayer->songs.push_back(std::move(newExtSong));
}
void Extension::CloseSong(const TCHAR * songName)
{
	CheckForPlayer(true, "CloseSong");
	auto song = std::find_if(curPlayer->songs.cbegin(), curPlayer->songs.cend(), [=](const std::unique_ptr<ExtKSong> &song) {
		return !_tcsicmp(song->songName.c_str(), songName);
	});
	if (song == curPlayer->songs.cend())
		return CreateError(_T("CloseSong: Song with name %s not found."), songName);
	curPlayer->songs.erase(song);
}

void Extension::CreatePlayer(const TCHAR * playerName, int sampleRate)
{
	auto alreadyPlayer = std::find_if(players.cbegin(), players.cend(), [=](const std::shared_ptr<ExtKPlayer> &player) {
		return !_tcsicmp(player->playerName.c_str(), playerName);
	});
	if (alreadyPlayer != players.cend())
		return CreateError(_T("CreatePlayer: Player with name %s already exists."), playerName);
	if (sampleRate <= 0)
		return CreateError(_T("CreatePlayer: Sample rate %i is invalid, must be greater than 0."), sampleRate);

	KPlayer * newPlayer = KSND_CreatePlayer(sampleRate);
	if (newPlayer == nullptr)
		return CreateError(_T("CreatePlayer: Failed to create player with name %s, sample rate %i."), playerName, sampleRate);

	std::shared_ptr<ExtKPlayer> newExtPlayer = std::make_shared<ExtKPlayer>();
	newExtPlayer->player = newPlayer;
	newExtPlayer->playerName = playerName;
	newExtPlayer->volume = 128; // I presume
	players.push_back(newExtPlayer);
}
void Extension::SelectPlayer(const TCHAR * playerName)
{
	auto player = std::find_if(players.cbegin(), players.cend(), [=](const std::shared_ptr<ExtKPlayer> &player) {
		return !_tcsicmp(player->playerName.c_str(), playerName);
	});
	if (player == players.cend())
		return CreateError(_T("SelectPlayer: Player with name %s doesn't exist."), playerName);
	curPlayer = *player;
}
void Extension::ClosePlayer()
{
	if (!curPlayer)
		return CreateError(_T("ClosePlayer: No player selected to close."));

	curPlayer.reset();
}
void Extension::SetCurrentPlayerQuality(int oversample)
{
	CheckForPlayer(true, "SetCurrentPlayerQuality");
	if (oversample < 0 || oversample > 4)
		return CreateError(_T("SetCurrentPlayerQuality: Quality must be between 0 and 4, inclusive."));
	try
	{
		KSND_SetPlayerQuality(curPlayer->player, oversample);
		curPlayer->quality = oversample;
	}
	catch (...)
	{
		return CreateError(_T("SetCurrentPlayerQuality: Failed to set quality of current player to %i."), oversample);
	}
}
void Extension::SetCurrentPlayerLooping(int looping)
{
	CheckForPlayer(true, "SetCurrentPlayerLooping");
	if (looping < 0 || looping > 1)
		return CreateError(_T("SetCurrentPlayerLooping: Looping param must be either 0 or 1."));

	try
	{
		KSND_SetLooping(curPlayer->player, looping);
		curPlayer->looping = looping == 1;
	}
	catch (...)
	{
		return CreateError(_T("SetCurrentPlayerLooping: Failed to set looping of current player to %i."), looping);
	}
}
void Extension::SetCurrentPlayerVolume(int volume)
{
	CheckForPlayer(true, "SetCurrentPlayerVolume");
	if (volume < 0 || volume > 128)
		return CreateError(_T("SetCurrentPlayerVolume: Volume must be between 0 and 128, inclusive."));

	try
	{
		KSND_SetVolume(curPlayer->player, volume);
		curPlayer->volume = volume;
	}
	catch (...)
	{
		return CreateError(_T("SetCurrentPlayerVolume: Failed to set volume of current player to %i."), volume);
	}
}
void Extension::PlaySongOnCurrentPlayer(const TCHAR * songUserName, int position)
{
	CheckForPlayer(true, "PlaySongOnCurrentPlayer");
	auto song = std::find_if(curPlayer->songs.cbegin(), curPlayer->songs.cend(), [=](const std::unique_ptr<ExtKSong> &song) {
		return !_tcsicmp(song->songName.c_str(), songUserName);
	});

	if (song == curPlayer->songs.cend())
		return CreateError(_T("PlaySongOnCurrentPlayer: Song with name %s not found."), songUserName);
	if (position < 0)
		return CreateError(_T("PlaySongOnCurrentPlayer: The position must be 0 or greater."));
	if (position > (**song).length)
		return CreateError(_T("PlaySongOnCurrentPlayer: Position %i is beyond length %i of song %s."), position, (**song).length, songUserName);

	try
	{
		KSND_PlaySong(curPlayer->player, (**song).song, position);
		curPlayer->state = ExtKPlayer::Playing;
	}
	catch (...)
	{
		return CreateError(_T("PlaySongOnCurrentPlayer: Failed to play song name %s at position %i."), songUserName, position);
	}
}
void Extension::SetPauseStateOnCurrentPlayer(int pauseState)
{
	CheckForPlayer(true, "SetPauseStateOnCurrentPlayer");
	if (pauseState < 0 || pauseState > 1)
		return CreateError(_T("SetPauseStateOnCurrentPlayer: Pause state must be 0 or 1."));

	try
	{
		KSND_Pause(curPlayer->player, pauseState);
		curPlayer->state = pauseState == 0 ?
			curPlayer->songs.empty() ? ExtKPlayer::Stopped : ExtKPlayer::Playing : ExtKPlayer::Paused;
	}
	catch (...)
	{
		return CreateError(_T("SetPauseStateOnCurrentPlayer: Failed to set pause state of current player."));
	}
}
void Extension::StopCurrentPlayer()
{
	CheckForPlayer(true, "StopCurrentPlayer");
	try
	{
		KSND_Stop(curPlayer->player);
	}
	catch (...)
	{
		return CreateError(_T("StopCurrentPlayer: Failed to stop."));
	}
}
