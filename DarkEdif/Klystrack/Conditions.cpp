
#include "Common.h"

bool Extension::OnError()
{
	// Triggered condition, no params, always true
	return true;
}
bool Extension::DoesPlayerNameExist(const TCHAR * playerName)
{
	return std::find_if(players.begin(), players.end(), [=](const std::shared_ptr<ExtKPlayer> &p) {
		return !_tcsicmp(p->playerName.c_str(), playerName);
	}) != players.end();
}

bool Extension::DoesSongNameExist(const TCHAR * songName)
{
	CheckForPlayer(false, "DoesSongNameExist", false);
	return std::find_if(curPlayer->songs.cbegin(), curPlayer->songs.cend(), [=](const std::unique_ptr<ExtKSong> &s) {
		return !_tcsicmp(s->songName.c_str(), songName);
	}) != curPlayer->songs.cend();
}
bool Extension::IsPlayerNamePlaying(const TCHAR * playerName)
{
	auto player = std::find_if(players.begin(), players.end(), [=](const std::shared_ptr<ExtKPlayer> &p) {
		return !_tcsicmp(p->playerName.c_str(), playerName);
	});
	if (player == players.end())
	{
		CreateError(_T("IsPlayerNamePlaying: Can't find player with name %s."), playerName);
		return false;
	}
	return (**player).state == ExtKPlayer::Playing;
}
bool Extension::IsPlayerNamePaused(const TCHAR * playerName)
{
	auto player = std::find_if(players.begin(), players.end(), [=](const std::shared_ptr<ExtKPlayer> &p) {
		return !_tcsicmp(p->playerName.c_str(), playerName);
	});
	if (player == players.end())
	{
		CreateError(_T("IsPlayerNamePaused: Can't find player with name %s."), playerName);
		return false;
	}
	return (**player).state == ExtKPlayer::Paused;
}
