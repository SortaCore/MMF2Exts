#pragma once
#include "WindowsSurface.hpp"

// Forwards
class CFillData;
class CFillFlat;
class CFillDir;
class CFillMosaic;

// CFillData class
//
// Interface of Fill Classes
//
// Note full non-inline EXORD members exist for some of these, but due to trivial implementation there's no point using those.

class FusionAPIImport CFillData
{
public:
	CFillData() {};
	virtual ~CFillData() {};
	virtual DWORD SetFillOrg(int x, int y) { return 0; }

	virtual BOOL Fill(cSurface * pSf, int l, int t, int r, int b, BOOL bForceOpaqueBlack = FALSE) = 0;
	// Default (non-pure) implementation, used when a subclass doesn't need custom trueW/trueH handling.
	EXORD(434) virtual BOOL Fill(cSurface * pSf, int trueW = INT_MAX, int trueH = INT_MAX, BOOL bForceOpaqueBlack = FALSE) EXDEF;

	// New functions to avoid dll export compatibility issues with Fusion (and use the same code...)
	// Pure virtual
	virtual BOOL fxFill(cSurface * pSf, int l, int t, int r, int b, BlitMode bm = BlitMode::BMODE_OPAQUE, COLORREF rgbaCoef = 0xFFFFFFFF, BOOL bForceOpaqueBlack = FALSE) = 0;

	CF25_292_02_REQUIRED(1113) virtual BOOL fxFill(cSurface * pSf, int trueW = INT_MAX, int trueH = INT_MAX, BlitMode bm = BlitMode::BMODE_OPAQUE, COLORREF rgbaCoef = 0xFFFFFFFF, BOOL bForceOpaqueBlack = FALSE) EXDEF;

protected:
	// EXORD2(444,771)
	inline void VERIFY_BLACK(COLORREF & cr) { if (cr == BLACK) cr = OPAQUE_BLACK; }
};

//************************************************
// CFillFlat class
//************************************************

class FusionAPIImport CFillFlat : public CFillData
{
public:
	// Constructors
	EXORD2(305, 234) CFillFlat(COLORREF cr) EXDEF;
	EXORD2(304, 233) CFillFlat(const CFillFlat & scr) EXDEF;

private:
	// Attributes
	COLORREF m_cr;

public:
	// Inline; matches the real DLL export (ordinal 427 MMF1 / 725 MMF2+) byte-for-byte.
	void SetColor(COLORREF newCr) { m_cr = newCr; }

	// Implementation
	EXORD(437) virtual BOOL Fill(cSurface * pSf, int l, int t, int r, int b, BOOL bForceOpaqueBlack = FALSE) EXDEF;
	CF25_292_02_REQUIRED(1116) virtual BOOL fxFill(cSurface * pSf, int l, int t, int r, int b, BlitMode bm = BlitMode::BMODE_OPAQUE, COLORREF rgbaCoef = 0xFFFFFFFF, BOOL bForceOpaqueBlack = FALSE) EXDEF;
	// EXORD2(371,477)
	COLORREF GetColor() { return m_cr; }
};

//************************************************
// CFillDir class
//************************************************

class FusionAPIImport CFillDir : public CFillData
{
public:
	// Constructors
	EXORD2(303, 232) CFillDir(COLORREF crFrom, COLORREF crTo, BOOL Vertical, int xOrg = 0, int yOrg = 0) EXDEF;
	EXORD2(302, 231) CFillDir(const CFillDir & src) EXDEF;

private:
	int m_xOrg;
	int m_yOrg;
	// Attributes
	COLORREF	m_crFrom;
	COLORREF	m_crTo;
	BOOL		m_vert;

public:
	// EXORD(726)
	// @remarks MMF1(428) took three parameters: COLORREF crFrom, COLORREF crTo, BOOL vertical
	void SetColors(COLORREF crFrom, COLORREF crTo) { m_crFrom = crFrom; m_crTo = crTo; }
	// EXORD(730)
	void SetDir(BOOL vert) { m_vert = vert; }
	// Implementation
	EXORD2(430, 733) virtual DWORD SetFillOrg(int x, int y) EXDEF;
	EXORD(436) virtual BOOL Fill(cSurface * pSf, int l, int t, int r, int b, BOOL bForceOpaqueBlack = FALSE) EXDEF;
	EXORD(435) virtual BOOL Fill(cSurface * pSf, int trueW = INT_MAX, int trueH = INT_MAX, BOOL bForceOpaqueBlack = FALSE) EXDEF;
	CF25_292_02_REQUIRED(1114) virtual BOOL fxFill(cSurface * pSf, int l, int t, int r, int b, BlitMode bm = BlitMode::BMODE_OPAQUE, COLORREF rgbaCoef = 0xFFFFFFFF, BOOL bForceOpaqueBlack = FALSE) EXDEF;
	CF25_292_02_REQUIRED(1115) virtual BOOL fxFill(cSurface * pSf, int trueW = INT_MAX, int trueH = INT_MAX, BlitMode bm = BlitMode::BMODE_OPAQUE, COLORREF rgbaCoef = 0xFFFFFFFF, BOOL bForceOpaqueBlack = FALSE) EXDEF;
};

//************************************************
// CFillMosaic class
//************************************************

class FusionAPIImport CFillMosaic : public CFillData
{
public:
	// Constructors
	EXORD2(307, 236) CFillMosaic(cSurface * psfMosaic, int xOrg = 0, int yOrg = 0) EXDEF;
	EXORD2(306, 235) CFillMosaic(const CFillMosaic & src) EXDEF;

private:
	// Attributes
	cSurface *	m_pSfTile;
	int			m_xOrg;
	int			m_yOrg;

public:
	// EXORD2(439,752)
	void SetTile(cSurface * pSf) { m_pSfTile = pSf; }
	EXORD2(431, 734) virtual DWORD SetFillOrg(int x, int y) EXDEF;
	EXORD(438) virtual BOOL Fill(cSurface * pSf, int l, int t, int r, int b, BOOL bForceOpaqueBlack = FALSE) EXDEF;
	CF25_292_02_REQUIRED(1117) virtual BOOL fxFill(cSurface * pSf, int l, int t, int r, int b, BlitMode bm = BlitMode::BMODE_OPAQUE, COLORREF rgbaCoef = 0xFFFFFFFF, BOOL bForceOpaqueBlack = FALSE) EXDEF;
};
