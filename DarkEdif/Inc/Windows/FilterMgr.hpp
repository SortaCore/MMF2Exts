#pragma once

#ifndef _FilterMgr_h
#define _FilterMgr_h

typedef	LPVOID LPFILTERINFOARRAY;

// Forwards
class CInputFile;

///////////////////////////////////////////////////
//
// Filter Manager base class
//

#define FILTERID_PNG	DWORDSTR('P','N','G',' ')

#if defined(_WIN32) && !defined(_UNICODE)

// Fusion 2.0 ANSI version of CFilterMgr
class FusionAPIImport CFilterMgrANSI20
{
	// Public
public:
	CFilterMgrANSI20();
	virtual ~CFilterMgrANSI20();

	static CFilterMgrANSI20* CreateInstance();

	// Initialization
	virtual void		Initialize(LPCSTR pFilterPath, LPCSTR pFileType, DWORD dwFlags);
	virtual void		Free();
	virtual void		Delete();

	// Filter info
	virtual int			GetFilterCount();
	virtual LPCSTR		GetFilterNameA(int nIndex);
	virtual LPCSTR		GetFilterFileNameA(int nIndex);
	virtual DWORD		GetFilterID(int nIndex);
	virtual int			GetFilterIndex(DWORD dwID);
	virtual LPCSTR* GetFilterExtsA(int nIndex);
	virtual LPCSTR* GetFilterDependenciesA(int nIndex);
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
#endif

// Filter Manager
class FusionAPIImport CFilterMgr
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
	virtual LPCSTR		GetFilterNameA(int nIndex);
	virtual LPCSTR		GetFilterFileNameA(int nIndex);
	virtual DWORD		GetFilterID(int nIndex);
	virtual int			GetFilterIndex(DWORD dwID);
	virtual LPCSTR*		GetFilterExtsA(int nIndex);
	virtual LPCSTR*		GetFilterDependenciesA(int nIndex);
	virtual BOOL		CanReadFile(int nIndex, CInputFile* pif);

	// Filter object creation
	virtual LPVOID		CreateFilterObject(int nIndex);
	virtual LPVOID		CreateFilterObject(DWORD dwID);
	virtual LPVOID		CreateFilterObject(CInputFile* pif);

	// Update
	virtual void		RemoveFilter(int nIndex);

	// Unicode
	virtual void		Initialize(LPCWSTR pFilterPath, LPCWSTR pFileType, DWORD dwFlags);
	virtual LPCWSTR		GetFilterNameW(int nIndex);
	virtual LPCWSTR		GetFilterFileNameW(int nIndex);
	virtual LPCWSTR*	GetFilterExtsW(int nIndex);
	virtual LPCWSTR*	GetFilterDependenciesW(int nIndex);

	#ifdef _UNICODE
		#define GetFilterName GetFilterNameW
		#define GetFilterFileName GetFilterFileNameW
		#define GetFilterExts GetFilterExtsW
		#define GetFilterDependencies GetFilterDependenciesW
	#else
		#define GetFilterName GetFilterNameA
		#define GetFilterFileName GetFilterFileNameA
		#define GetFilterExts GetFilterExtsA
		#define GetFilterDependencies GetFilterDependenciesA
	#endif

	// Data
public:
	LPFILTERINFOARRAY	m_pFilterArray;
};

#endif // _FilterMgr_h
