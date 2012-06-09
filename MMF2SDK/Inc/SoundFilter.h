
#ifndef _SoundFilter_h
#define	_SoundFilter_h

#include "SoundConverter.h"
#include "SndDefs.h"

//////////////////////////////////////////////////////////////////////////////
//
// Sound Filter base class
//

class CInputFile;

extern /*SNDMGR_API*/ DWORD WINAPI GetRecommendedBufferSize(LPWAVEFORMATEX pwfex);

class SNDMGR_API CSoundFilter
{
public:
	CSoundFilter();
	virtual ~CSoundFilter();

	virtual void	Delete() = 0;

	virtual int		Open(CInputFile * pf) = 0;
	virtual void	Close() = 0;

	virtual DWORD	GetLength() = 0;
	virtual DWORD	GetPos() = 0;
	virtual BOOL	SetPos(DWORD dwPos) = 0;
	virtual int		ReadData(LPBYTE lpDstBuffer, DWORD dwBufSize, LPDWORD dwRead) = 0;

	virtual void	SetOutputFormat(LPWAVEFORMATEX pStreamFormat);

	LPWAVEFORMATEX	GetInputFormat() { return(&m_WaveFormatIn); }
	LPWAVEFORMATEX	GetOutputFormat() { return(&m_WaveFormatOut); }
	int				Read(LPBYTE pDest, DWORD dwBufSize, LPDWORD pDWRead);

protected:

	WAVEFORMATEX	m_WaveFormatOut;	// format des données à envoyer au SoundManager
	WAVEFORMATEX	m_WaveFormatIn;	// format des données après lecture et décodage

	BOOL			m_bDoConvert;
	float			m_fSizeRatio;		// rapport (Out Size)/(Read Size)
	LPBYTE			m_lpReadWaveData;	// Données après lecture et décodage
	SAMPLECONVERTPROC m_pConvertProc;
	DWORD			m_nSrcFreqSample;
	DWORD			m_nDstWrite;
};

#endif	// _SoundFilter_h
