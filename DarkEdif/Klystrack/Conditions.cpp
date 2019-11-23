
#include "Common.h"

bool Extension::OnError()
{
	// Triggered condition, no params, always true
	return true;
}
bool Extension::DoesPlayerNameExist(const char * playerName)
{
	return std::find_if(players.begin(), players.end(), [=](const ExtKPlayer * p) {
		return !_stricmp(p->playerName.c_str(), playerName);
	}) != players.end();
}

bool Extension::DoesSongNameExist(const char * songName)
{
	CheckForPlayer(false, "DoesSongNameExist", false);
	return std::find_if(curPlayer->songs.begin(), curPlayer->songs.end(), [=](const ExtKSong * s) {
		return !_stricmp(s->songName.c_str(), songName);
	}) != curPlayer->songs.end();
}
bool Extension::IsPlayerNamePlaying(const char * playerName)
{
	auto player = std::find_if(players.begin(), players.end(), [=](const ExtKPlayer * p) {
		return !_stricmp(p->playerName.c_str(), playerName);
	});
	if (player == players.end())
	{
		CreateError("IsPlayerNamePlaying: Can't find player with name %s.", playerName);
		return false;
	}
	return (**player).state == ExtKPlayer::Playing;
}
bool Extension::IsPlayerNamePaused(const char * playerName)
{
	auto player = std::find_if(players.begin(), players.end(), [=](const ExtKPlayer * p) {
		return !_stricmp(p->playerName.c_str(), playerName);
	});
	if (player == players.end())
	{
		CreateError("IsPlayerNamePaused: Can't find player with name %s.", playerName);
		return false;
	}
	return (**player).state == ExtKPlayer::Paused;
}
