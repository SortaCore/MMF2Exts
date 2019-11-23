
#ifndef _SoundFilterMgr_h
#define _SoundFilterMgr_h

#include "FilterMgr.h"
#include "SndDefs.h"

class CSoundManager;
class CSoundFilter;
class CInputFile;

////////////////////////////////////////////////
//
// Sound Filter Manager
//

class SNDMGR_API CSoundFilterMgr : public CFilterMgr
{
public:
	CSoundFilterMgr();
	virtual ~CSoundFilterMgr();

	static CSoundFilterMgr* CreateInstance();

	// List APIs and Filters
			void		Initialize(LPCSTR pFilterPath, DWORD dwFlags);	// non virtual function
			void		Initialize(LPCWSTR pFilterPath, DWORD dwFlags);	// non virtual function
	virtual void		Free();

	// Create sound manager
	CSoundManager*	CreateSoundManager(HWND hWnd);

	// Create sound filter
	CSoundFilter*	CreateFilter(CInputFile* pf);
};

#endif // _SoundFilterMgr_h
