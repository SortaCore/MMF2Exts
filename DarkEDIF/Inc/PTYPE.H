//----------------------------------//
//		Declarations de types		//
//----------------------------------//
#ifndef	_ptype_h
#define	_ptype_h	1

#ifndef	_MSC_VER
#define	__segment	unsigned int
#endif

#define	ubyte	unsigned char
#define	uchar	unsigned char
#define	uint	unsigned int
#define	ushort	unsigned short
#define	ulong	unsigned long

#ifdef	WIN32
#define	_near
#define	_far
#endif

typedef void _near *npvoid;
typedef char _near *npchar;
typedef int _near *npint;
typedef short _near *npshort;
typedef long _near *nplong;

typedef unsigned char _near *npubyte;
typedef unsigned char _near *npuchar;
typedef unsigned int _near *npuint;
typedef unsigned short _near *npushort;
typedef unsigned long _near *npulong;

typedef void _far *fpvoid;
typedef char _far *fpchar;
typedef int _far *fpint;
typedef short _far *fpshort;
typedef long _far *fplong;

typedef unsigned char _far *fpubyte;
typedef unsigned char _far *fpuchar;
typedef unsigned int _far *fpuint;
typedef unsigned short _far *fpushort;
typedef unsigned long _far *fpulong;

#endif	// _ptype_h
