
#ifndef _FilterMgr_h
#define _FilterMgr_h

#include "StdDefs.h"
#if !defined(STDDLL_IMPORTS)
	#include "FilterInfo.h"
#else
	typedef	LPVOID LPFILTERINFOARRAY;
#endif

// Forwards
class CInputFile;

///////////////////////////////////////////////////
//
// Filter Manager base class
//

// Filter Manager
class STDDLL_API CFilterMgr
{
// Public
public:
	CFilterMgr ();
	virtual ~CFilterMgr();

	static CFilterMgr* CreateInstance();

	// Initialization
	virtual void		Initialize(LPCSTR pFilterPath, LPCSTR pFileType, DWORD dwFlags);
	virtual void		Free();
	virtual void		Delete();

	// Filter info
	virtual int			GetFilterCount();
	virtual LPCSTR		GetFilterName(int nIndex);
	virtual LPCSTR		GetFilterFileName(int nIndex);
	virtual DWORD		GetFilterID(int nIndex);
	virtual int			GetFilterIndex(DWORD dwID);
	virtual LPCSTR*		GetFilterExts(int nIndex);
	virtual LPCSTR*		GetFilterDependencies(int nIndex);
	virtual BOOL		CanReadFile(int nIndex, CInputFile* pif);

	// Filter object creation
	virtual LPVOID		CreateFilterObject(int nIndex);
	virtual LPVOID		CreateFilterObject(DWORD dwID);
	virtual LPVOID		CreateFilterObject(CInputFile* pif);

	// Update
	virtual void		RemoveFilter(int nIndex);

	// Data
public:
	LPFILTERINFOARRAY	m_pFilterArray;
};

#endif // _FilterMgr_h
