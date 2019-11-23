
#ifndef SoundError_h
#define SoundError_h

//////////////////////////////////////////////////////////////////////////////
//
// Sound Library Error codes
//
enum {
	SND_OK = 0,
	SND_NOTENOUGHMEM,
	SND_BADWAVEFORMAT,
	SND_CANNOTOPENFILE,
	SND_NOTWAVEFILE,
	SND_CANNOTREADFILE,
	SND_CORRUPTWAVEFILE,
	SND_BADWAVEFILEFORMAT,
	SND_CANNOTOPENMEMSTREAM,
	SND_NOTAIFFFILE,
	SND_UNKNOWNERROR
};

#endif // SoundError_h
