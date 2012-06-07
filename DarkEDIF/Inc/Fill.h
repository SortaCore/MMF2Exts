//************************************************
// Fill.h
// A.Delepo 
// 09/04/97
//************************************************

#ifndef __FILL__
#define __FILL__

//**************************
//**************************
// External headers
//**************************
//**************************

#include "colors.h"
#include "SurfaceDefs.h"

//**************************
//**************************
// Forwards
//**************************
//**************************

// external
class FAR cSurface;

// internal
class FAR CFillData;
	class FAR CFillFlat;
	class FAR CFillDir;
	class FAR CFillMosaic;

//**************************
//**************************
// Concrete classes
//**************************
//**************************

//************************************************
// CFillData class
//
// Interface of Fill Classes
//************************************************

class SURFACES_API CFillData
{	
public:
			CFillData(){};
	virtual ~CFillData(){};
	virtual DWORD SetFillOrg(int x, int y){return 0;}
	virtual BOOL Fill(cSurface FAR * pSf, int l, int t, int r, int b, BOOL bForceOpaqueBlack=FALSE) = 0;
	virtual BOOL Fill(cSurface FAR * pSf, int trueW = INT_MAX, int trueH = INT_MAX, BOOL bForceOpaqueBlack=FALSE);

protected:
	inline 	void VERIFY_BLACK(COLORREF FAR & cr){if (cr == BLACK) cr = OPAQUE_BLACK;}
};

//************************************************
// CFillFlat class
//************************************************

class SURFACES_API CFillFlat : public CFillData
{
public:
	// Constructors
	CFillFlat(COLORREF cr);
	CFillFlat(const CFillFlat FAR & scr);

private:
	// Attributes
	COLORREF m_cr;

public:

	void SetColor(COLORREF newCr){m_cr = newCr;}

	// Implementation
	virtual BOOL Fill(cSurface FAR * pSf, int l, int t, int r, int b, BOOL bForceOpaqueBlack=FALSE);
	COLORREF GetColor() {return m_cr;}
};

//************************************************
// CFillDir class
//************************************************

class SURFACES_API CFillDir : public CFillData
{
public:
	// Constructors
	CFillDir(COLORREF crFrom, COLORREF crTo, BOOL Vertical, int xOrg = 0, int yOrg = 0);
	CFillDir(const CFillDir FAR & src);

private:
	int m_xOrg;
	int m_yOrg;
	// Attributes
	COLORREF	m_crFrom;
	COLORREF	m_crTo;
	BOOL			m_vert;

public:
	void SetColors(COLORREF crFrom, COLORREF crTo) {m_crFrom = crFrom; m_crTo = crTo;}
	void SetDir(BOOL vert) { m_vert = vert; }
	// Implementation
	virtual DWORD SetFillOrg(int x, int y);
	virtual BOOL Fill(cSurface FAR * pSf, int l, int t, int r, int b, BOOL bForceOpaqueBlack=FALSE);
	virtual BOOL Fill(cSurface FAR * pSf, int trueW = INT_MAX, int trueH = INT_MAX, BOOL bForceOpaqueBlack=FALSE);
};

//************************************************
// CFillMosaic class
//************************************************

class SURFACES_API CFillMosaic : public CFillData
{
public:
	// Constructors
	CFillMosaic(cSurface FAR * psfMosaic, int xOrg = 0, int yOrg = 0);
	CFillMosaic(const CFillMosaic FAR & src);

private:
	// Attributes
	cSurface FAR * m_pSfTile;
	int m_xOrg;
	int m_yOrg;

public:
	// Implementation
	void SetTile(cSurface FAR * pSf){m_pSfTile = pSf;}
	virtual DWORD SetFillOrg(int x, int y);
	virtual BOOL Fill(cSurface FAR * pSf, int l, int t, int r, int b, BOOL bForceOpaqueBlack=FALSE);
};

#endif  //__FILL__
