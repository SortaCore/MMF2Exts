
#include "Common.h"

bool Extension::OnError()
{
	// Triggered condition, no params, always true
	return true;
}
bool Extension::DoesPlayerNameExist(const char * playerName)
{
	return std::find_if(players.cbegin(), players.cend(), [=](const ExtKPlayer * p) {
		return !_stricmp(p->playerName.c_str(), playerName);
	}) != players.cend();
}

bool Extension::DoesSongNameExist(const char * songName)
{
	CheckForPlayer(false, "DoesSongNameExist", false);
	return std::find_if(curPlayer->songs.cbegin(), curPlayer->songs.cend(), [=](const ExtKSong * s) {
		return !_stricmp(s->songName.c_str(), songName);
	}) != curPlayer->songs.cend();
}
bool Extension::IsPlayerNamePlaying(const char * playerName)
{
	auto player = std::find_if(players.cbegin(), players.cend(), [=](const ExtKPlayer * p) {
		return !_stricmp(p->playerName.c_str(), playerName);
	});
	if (player == players.cend())
	{
		CreateError("IsPlayerNamePlaying: Can't find player with name %s.", playerName);
		return false;
	}
	return (**player).state == ExtKPlayer::Playing;
}
bool Extension::IsPlayerNamePaused(const char * playerName)
{
	auto player = std::find_if(players.cbegin(), players.cend(), [=](const ExtKPlayer * p) {
		return !_stricmp(p->playerName.c_str(), playerName);
	});
	if (player == players.cend())
	{
		CreateError("IsPlayerNamePaused: Can't find player with name %s.", playerName);
		return false;
	}
	return (**player).state == ExtKPlayer::Paused;
}
