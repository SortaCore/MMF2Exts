#ifndef KSND_DLL_H
#define KSND_DLL_H

/*

Wrapper library for easy klystron song loading and playing.

*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KSong_t KSong;
typedef struct KPlayer_t KPlayer;
typedef struct 
{
	char *song_title;
	char *instrument_name[128];
	int n_instruments;
	int n_channels;
} KSongInfo;


#ifdef WIN32 
#ifdef DLLEXPORT
#define KLYSAPI _cdecl __declspec(dllexport)
#else
#define KLYSAPI 
#endif
#else
#define KLYSAPI 
#endif

// Song

KLYSAPI extern KSong* KSND_LoadSong(KPlayer* player, const char *path);
KLYSAPI extern KSong* KSND_LoadSongFromMemory(KPlayer* player, void *data, int data_size);
KLYSAPI extern void KSND_FreeSong(KSong *song);
KLYSAPI extern int KSND_GetSongLength(const KSong *song);
KLYSAPI extern const KSongInfo * KSND_GetSongInfo(KSong *song, KSongInfo *data);
KLYSAPI extern void KSND_SetLooping(KPlayer *player, int looping);

// Player

KLYSAPI extern KPlayer* KSND_CreatePlayer(int sample_rate);
KLYSAPI extern KPlayer* KSND_CreatePlayerUnregistered(int sample_rate);
KLYSAPI extern void KSND_SetPlayerQuality(KPlayer *player, int oversample);
KLYSAPI extern void KSND_FreePlayer(KPlayer *player);
KLYSAPI extern void KSND_PlaySong(KPlayer *player, KSong *song, int start_position);
KLYSAPI extern void KSND_FillBuffer(KPlayer *player, short int *buffer, int buffer_length);
KLYSAPI extern void KSND_Stop(KPlayer* player);
KLYSAPI extern void KSND_Pause(KPlayer *player, int state);
KLYSAPI extern int KSND_GetPlayPosition(KPlayer* player);
KLYSAPI extern void KSND_SetVolume(KPlayer *player, int volume);
KLYSAPI extern void KSND_GetVUMeters(KPlayer *player, int *dest, int n_channels);

#ifdef __cplusplus
}
#endif

#endif
