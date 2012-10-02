
#include <stdio.h> 
#include <tchar.h>
#include <windows.h>

#define fancyenum(a) namespace a { \
						const enum

#include "Surface.h"
#include "Props.h"

struct CTransition;
struct CDebugger;
#ifdef HWABETA
	struct CEffectEx;
	struct CPList;
#endif

// Surface.h pre-declaration
class cSurface;
struct cSurfaceImplementation;
struct CFillData;
class CInputFile;
struct sMask;

#define bit1  0x00000001
#define bit2  0x00000002
#define bit3  0x00000004
#define bit4  0x00000008
#define bit5  0x00000010
#define bit6  0x00000020
#define bit7  0x00000040
#define bit8  0x00000080
#define bit9  0x00000100
#define bit10 0x00000200
#define bit11 0x00000400
#define bit12 0x00000800
#define bit13 0x00001000
#define bit14 0x00002000
#define bit15 0x00004000
#define bit16 0x00008000
#define bit17 0x00010000
#define bit18 0x00020000
#define bit19 0x00040000
#define bit20 0x00080000
#define bit21 0x00100000
#define bit22 0x00200000
#define bit23 0x00400000
#define bit24 0x00800000
#define bit25 0x01000000
#define bit26 0x02000000
#define bit27 0x04000000
#define bit28 0x08000000
#define bit29 0x10000000
#define bit30 0x20000000
#define bit31 0x40000000
#define bit32 0x80000000

// PictEdDefs.h
fancyenum(PICTEDOPT) {
	FIXEDIMGSIZE		= bit1,		// User cannot change the image size
	HOTSPOT				= bit2,		// Image has a hot spot
	ACTIONPOINT			= bit3,		// Image has an action point
	_16COLORS			= bit4,		// 16 colors image
	FIXEDNFRAMES		= bit5,		// User cannot add / remove frames (animation editor)
	NUMBERS				= bit6,		// Numbers (internal)
	NOTRANSPARENCY		= bit7,		// The image has no transparent color
	NOALPHACHANNEL		= bit8,		// The image cannot have an alpha channel
	APPICON				= bit9,		// Internal usage
	ICONPALETTE			= bit10,	// Internal usage
	CANBETRANSPARENT	= bit11,	// The image can be empty (if this option is not specified, MMF refuses to close the picture editor if the image is empty)
	CANNOTMOVEFRAMES	= bit12,	// Frames cannot be moved
};}
// Marquage des ObjectInfo qualifiers
#define	OIFLAG_QUALIFIER			bit16
#define	NDQUALIFIERS				100
#define MAX_EVENTPROGRAMS			256


// General errors
fancyenum(CFCERROR) {
	NOT_ENOUGH_MEM = 0x40000000,
	READ_ERROR,
	END_OF_FILE,
	WRITE_ERROR,
	DISK_FULL,
	CANNOT_OPEN_FILE,
	CANNOT_CREATE_FILE,
	BUFFER_TOO_SMALL,
	CANNOT_SET_FILESIZE,
	UNKNOWN,					// Internal error
	MAX = 0x40010000,
};}

// Surface errors
#define	SURFACEERROR_MIN	0x40010000
#define	SURFACEERROR_MAX	0x40020000
enum 
{
	SURFACEERROR_NOERR = 0,
	SURFACEERROR_NOT_SUPPORTED = SURFACEERROR_MIN,
	SURFACEERROR_SURFACE_NOT_CREATED,
	SURFACEERROR_INTERNAL
};

__declspec(dllimport) HFILE			WINAPI	File_OpenA(const char * fname, int mode);
__declspec(dllimport) HFILE			WINAPI	File_CreateA(const char * fname);
__declspec(dllimport) int			WINAPI	File_Read(HFILE hf, void * buf, unsigned int len);
__declspec(dllimport) int			WINAPI	File_ReadAndCount(HFILE hf, void * buf, unsigned int len);
__declspec(dllimport) int			WINAPI	File_ReadShortIntelData(HFILE hf, void ** pBuf);
__declspec(dllimport) int			WINAPI	File_ReadShortIntelString(HFILE hf, void ** pBuf);
__declspec(dllimport) int			WINAPI	File_ReadLongIntelData(HFILE hf, void ** pBuf);
__declspec(dllimport) int			WINAPI	File_Write(HFILE hf, void * buf, unsigned int len);
__declspec(dllimport) unsigned int	WINAPI	File_GetPosition(HFILE hf);
__declspec(dllimport) unsigned int	WINAPI	File_SeekBegin(HFILE hf, long pos);
__declspec(dllimport) unsigned int	WINAPI	File_SeekCurrent(HFILE hf, long pos);
__declspec(dllimport) unsigned int	WINAPI	File_SeekEnd(HFILE hf, long pos);
__declspec(dllimport) long			WINAPI	File_GetLength( HFILE hf );
__declspec(dllimport) void			WINAPI	File_Close(HFILE hf);
__declspec(dllimport) BOOL			WINAPI	File_ExistA(const char * pName);

__declspec(dllimport) HFILE		WINAPI	File_OpenW(const wchar_t * fname, int mode);
__declspec(dllimport) HFILE		WINAPI	File_CreateW(const wchar_t * fname);
__declspec(dllimport) BOOL			WINAPI	File_ExistW(const wchar_t * pName);

#ifdef _UNICODE
#define File_Open File_OpenW
#define File_Create File_CreateW
#define File_Exist File_ExistW
#else
#define File_Open File_OpenA
#define File_Create File_CreateA
#define File_Exist File_ExistA
#endif

#define	File_ReadIntelWord(h,p,l)	File_Read(h,p,l)
#define	File_ReadIntelDWord(h,p,l)	File_Read(h,p,l)

// Input file - abstract class
class __declspec(dllimport) CInputFile
{
public:
	virtual			~CInputFile() {};

	virtual void	Delete();

	virtual	int		Read(unsigned char * dest, unsigned long lsize) = 0;
	virtual	int		Read(unsigned char * dest, unsigned long lsize, LPDWORD pRead) = 0;
	virtual	int		ReadByte(unsigned char * dest) = 0;
	virtual	int		ReadIntelWord(LPWORD dest);
	virtual	int		ReadIntelDWord(LPDWORD dest);
	virtual	int		ReadMacWord(LPWORD dest);
	virtual	int		ReadMacDWord(LPDWORD dest);
	virtual	int		ReadIntelFloat(PFLOAT dest);
	virtual	int		ReadMacFloat(PFLOAT dest);
	virtual	int		ReadIntelWordArray(LPWORD dest, int count);
	virtual	int		ReadIntelDWordArray(LPDWORD dest, int count);
	virtual	int		ReadIntelFloatArray(PFLOAT dest, int count);
	virtual	int		ReadMacWordArray(LPWORD dest, int count);
	virtual	int		ReadMacDWordArray(LPDWORD dest, int count);
	virtual	int		ReadMacFloatArray(PFLOAT dest, int count);

	virtual long	GetPosition() = 0;
	virtual long	GetLength() = 0;
	virtual	long	Seek(long pos, int method) = 0;

	virtual	int		Attach(HANDLE hnd) = 0;
	virtual	HANDLE	Detach() = 0;

	virtual unsigned char *	GetBuffer(UINT nSize);
	virtual void	FreeBuffer(unsigned char * buf);

	virtual	char *	GetFileNameA() = 0;
	virtual	wchar_t *	GetFileNameW() = 0;

	#ifdef _CFCFILE_UNICODE_DEFS
		#if defined(_UNICODE)
			#define GetFileName GetFileNameW
		#else
			#define GetFileName GetFileNameA
		#endif
	#endif
};
//typedef CInputFile * LPINPUTFILE;

// Bufferized input file
class __declspec(dllimport) CInputBufFile : public CInputFile
{
	public:
								CInputBufFile();
		virtual					~CInputBufFile();

		static CInputBufFile *	NewInstance();

		int						Create(HFILE hf);
		int						Create(HFILE hf, unsigned int dwOffset, unsigned int dwSize);
		int						Create(const char * filename);
		int						Create(const char * filename, unsigned int dwOffset, unsigned int dwSize);
		int						Create(const wchar_t * filename);
		int						Create(const wchar_t * filename, unsigned int dwOffset, unsigned int dwSize);

		virtual	int				Read(unsigned char * dest, unsigned int lsize);
		virtual	int				Read(unsigned char * dest, unsigned int lsize, LPDWORD pRead);
		virtual	int				ReadByte(unsigned char * dest);
//		virtual	int				ReadWord(LPWORD dest);
//		virtual	int				ReadDWord(LPDWORD dest);
		virtual long			GetPosition();
		virtual long			GetLength();
		virtual	long			Seek(long pos, int method);

		virtual	int				Attach(HANDLE hnd);
		virtual	HANDLE			Detach();

//		virtual unsigned char *	GetBuffer(UINT nSize);
//		virtual void			FreeBuffer(unsigned char * buf);

		virtual	char *			GetFileNameA();
		virtual	wchar_t *		GetFileNameW();

	protected:
		int						Attach(HANDLE hnd, unsigned int dwOffset, unsigned int dwSize);
	private:
		HFILE					m_hf;
		unsigned int			m_curpos;
		unsigned char *			m_buffer;
		unsigned char *			m_bufcurr;
		unsigned int			m_remains;
		wchar_t *				m_fnameW;
		char *					m_fnameA;

		unsigned int			m_startOffset;
		unsigned int			m_length;
};
typedef	CInputBufFile * LPINPUTBUFFILE;

#define	BUFFILE_BUFFER_SIZE	16384

// Memory input file
class __declspec(dllimport) CInputMemFile : public CInputFile
{
	public:
								CInputMemFile();
		virtual					~CInputMemFile();

		static CInputMemFile *	NewInstance();

		int						Create(unsigned char * buffer, unsigned long lsize);
		int						Create(unsigned long lsize);
		unsigned char *			GetMemBuffer();

		virtual	int				Read(unsigned char * dest, unsigned long lsize);
		virtual	int				Read (unsigned char * dest, unsigned long lsize, LPDWORD pRead);
		virtual	int				ReadByte(unsigned char * dest);
//		virtual	int				ReadWord(LPWORD dest);
//		virtual	int				ReadDWord(LPDWORD dest);
		virtual long			GetPosition();
		virtual long			GetLength();
		virtual	long			Seek(long pos, int method);
	
		virtual	int				Attach(HANDLE hnd);
		virtual	HANDLE			Detach();

		virtual unsigned char *	GetBuffer(UINT nSize);
		virtual void			FreeBuffer(unsigned char * buf);

		virtual	char *			GetFileNameA() { return NULL; }
		virtual	wchar_t *		GetFileNameW() { return NULL; }

	private:
		unsigned char *			m_buffer;
		unsigned char *			m_bufcurr;
		unsigned int			m_curpos;
		unsigned int			m_remains;
		BOOL					m_bAutoDelete;
};
//typedef	CInputMemFile * LPINPUTMEMFILE;

// Output file: base class
class __declspec(dllimport) COutputFile
{
	public:
								COutputFile() {};
		virtual					~COutputFile() {};

//		virtual int				WriteByte(BYTE b) = 0;
//		virtual int				WriteWord(WORD b) = 0;

		int						WriteIntelWord(LPWORD pw);
		int						WriteIntelDWord(LPDWORD pdw);
		int						WriteIntelFloat(PFLOAT dest);
		int						WriteMacWord(LPWORD pw);
		int						WriteMacDWord(LPDWORD pdw);

		virtual int				Write(unsigned char * pb, UINT sz) = 0;
		virtual int				Flush() = 0;
		virtual unsigned int	GetLength() = 0;

		virtual long			GetPosition() = 0;
		virtual long			Seek(long pos, int method) = 0;

		virtual	char *			GetFileNameA() = 0;
		virtual	wchar_t *		GetFileNameW() = 0;
};
//typedef COutputFile * LPOUTPUTFILE;

// Memory output file
class __declspec(dllimport) COutputMemFile : public COutputFile
{
	public:
								COutputMemFile();
		virtual					~COutputMemFile();

		int						Create(UINT nBlockSize = 512);
		int						Create(unsigned char * buffer, unsigned int nBufferSize = 0x7FFFFFF);
		unsigned char *			GetBuffer();
		unsigned char *			DetachBuffer();
		static void				FreeBuffer(unsigned char * pBuffer);	// car il faut libérer à l'interieur de la DLL

		virtual int				Write(unsigned char * pb, UINT sz);
		virtual int				Flush();
		virtual unsigned int	GetLength();

		virtual long			GetPosition();
		virtual long			Seek(long pos, int method);

		virtual	char *			GetFileNameA() { return NULL; }
		virtual	wchar_t *		GetFileNameW() { return NULL; }

	private:
		unsigned char *			m_buffer;
		unsigned char *			m_curptr;
		unsigned int			m_totalsize;
		unsigned int			m_cursize;
		unsigned int			m_blocksize;
		BOOL					m_bReallocable;
};

// Bufferized output file
class __declspec(dllimport) COutputBufFile : public COutputFile
{
	public:
								COutputBufFile();
		virtual					~COutputBufFile();

		int						Create(HFILE hf, UINT nBufferSize = 4096);
		int						Create(const char * fname, UINT nBufferSize = 4096);
		int						Create(const wchar_t * fname, UINT nBufferSize = 4096);

		virtual int				Write(unsigned char * pb, UINT sz);
		virtual int				Flush();
		virtual unsigned int	GetLength();

		virtual long			GetPosition();
		virtual long			Seek(long pos, int method);

		virtual	char *			GetFileNameA();
		virtual	wchar_t *		GetFileNameW();

	private:
		HFILE					m_hf;
		wchar_t *				m_fnameW;
		unsigned char *			m_buffer;
		unsigned char *			m_curptr;
		unsigned int			m_cursize;
		unsigned int			m_buffersize;
		BOOL					m_bBuffered;
		char *					m_fnameA;
};

// Possible states of the application, see LApplication::SetRunningState()
fancyenum(GAMEON) {
	EMPTY,
	UNLOADED,
	TIME0,
	PAUSED,
	RUNNING,
	STEPPING,
};}

// Changes the structure alignment
// See http://stupefydeveloper.blogspot.co.uk/2009/01/c-alignment-of-structure.html
#ifndef	_H2INC  
	#pragma pack(push, mmf_master_header)
	#pragma pack(2) 
#endif

// This declaration list cannot be cut down
class CImageFilterMgr;
struct CSoundFilterMgr;
struct CSoundManager;
struct CRunApp;
struct CRunFrame;
struct CEditApp;
struct CEditFrame;
struct EDITDATA;
struct ObjectInfo;
struct dllTrans;
struct kpj;
struct drawRoutine;
struct LevelObject;
struct objInfoList;
struct HeaderObject;
struct qualToOi;
struct pev;
struct mv;
struct CRunMvt;
struct CMvt;
struct CDemoRecord;
struct CIPhoneJoystick;
struct CIPhoneAd;

struct Obj
{
	unsigned int	Size,		// Taille de l'objet: 0 = fin objets
					PrevSize,	// Taille objet precedent (0 = 1er objet)
					Type,		// Type d'objet (1=vide,2=appli,3=fenetre,...)
					Prev,		// Adresse objet precedent de meme type (0 = first)
					Next;		// Adresse objet suivant de meme type (0 = last)
};
//typedef Obj *npObj;
//typedef Obj *fpObj;

struct Spr
{
	#ifndef SPRLIST
		Obj			Obj;
	#endif
	unsigned int	Flags;	// Flags
	unsigned short	Layer;	// Sprite plane (layer)
	short			Angle;	// Angle
	int				ZOrder,	// Z-order value
					X,		// X Position
					Y,		// Y Position
					X1,		// Bounding box, top left corner X pos
					Y1,		// Bounding box, top left corner Y pos
					X2,		// Bounding box, bottom right corner X pos
					Y2,		// Bounding box, bottom right corner Y pos
					Xnew,	// New X Position
					Ynew,	// New Y Position
					X1new,	// New bounding box, top left corner X pos
					Y1new,	// New bounding box, top left corner Y pos
					X2new,	// New bounding box, bottom right corner X pos
					Y2new,	// New bounding box, bottom right corner Y pos
					X1z,	// Background bounding box, top left corner X pos
					Y1z,	// Background bounding box, top left corner Y pos
					X2z,	// Background bounding box, bottom right corner X pos
					Y2z;	// Background bounding box, bottom right corner Y pos
	float			XScale,
					YScale;
	// Temporary values for collisions
	unsigned short	TempImg;	// TODO: use unsigned intlater?
	short			TempAngle;
	float			TempScaleX,
					TempScaleY;
	// Image or owner-draw routine
	union {
		struct	{
			unsigned int Img;             // Numero d'image
			unsigned int ImgNew;          // Nouvelle image
		};
	LPARAM Rout;			// Ownerdraw callback routine
	};

	// Ink effect
	unsigned int	Effect;			// 0 = normal, 1 = semi-transparent, > 16 = routine
	LPARAM			EffectParam;	// parametre effet (coef transparence, etc...)

	// Fill color (wipe with color mode)
	COLORREF	BackColor;

	// Surfaces
	cSurfaceImplementation *	BackSurf;	// Background surface, if no general background surface

	cSurfaceImplementation *	Sf;			// Surface (if stretched or rotated)
	sMask*					ColMask;		// Collision mask (if stretched or rotated)

	cSurfaceImplementation*	TempSf;		// Temp surface (if stretched or rotated)
	sMask*					TempColMask;	// Temp collision mask (if stretched or rotated)

	// User data
	LPARAM		ExtraInfo;

	// Colliding ites
	int					CollisList[2];	// liste de ites entrant en collisions

};

// Maximum number of parameters
#define EVI_MAXPARAMS				16

// Structure de definition des conditions / actions POUR LES EXTENSIONS V1
struct infosEvents { 
	short 	code;					// Le numero de type + code event
	short	flags;					// Les flags a mettre dans l'event
	short	param[2];				// Le type des parametres
	short	paramTitle[2];			// Le titre de chacun des params
};
//typedef	infosEvents *			LPINFOEVENTS;
//typedef	infosEvents *				PINFOEVENTS;

// Event information structure
// ---------------------------
struct eventInformations {
	short		menu;	// Menu identifier
	short		string;	// String identifier
	infosEvents	infos;	// Sub structure
};
//typedef	eventInformations *		LPEVENTINFOS;
//typedef	eventInformations *				NPEVENTINFOS;

short ReadParameterType(const char *, bool &);
short ReadExpressionParameterType(const char *, bool &);
//typedef	eventInformations2 *	eventInformations2 *;

//#define EVINFO2_NEXT(p)			((eventInformations2 *)((unsigned char *)p + sizeof(eventInformations2) + p->infos.nParams * 2 * sizeof(short)))
//#define	EVINFO2_PARAM(p,n)		(*(unsigned short *)((unsigned char *)p + sizeof(eventInformations2) + n * sizeof(short)))
//#define	EVINFO2_PARAMTITLE(p,n)	(*(unsigned short *)((unsigned char *)p + sizeof(eventInformations2) + p->infos.nParams * sizeof(short) + n * sizeof(short)))


// CNCF.H


// Definitions for extensions
#define TYPE_LONG	0x0000
#define TYPE_INT	TYPE_LONG
#define TYPE_STRING	bit1
#define TYPE_FLOAT	bit2				// Pour les extensions
#define TYPE_DOUBLE bit2

struct CValue {
	unsigned int m_type,
				 m_paddle;
	union
	{
		long m_long;
		double m_double;
		TCHAR * m_pString;
	};
};

typedef short *				LPSHORT;

//class CValue;
// Structure for SaveRect
struct saveRect {
	unsigned char * pData;
	RECT			rc;
};
//typedef saveRect* fpSaveRect;

// Number of values
#define ALTERABLE_VALUES_COUNT 26
#define ALTERABLE_STRINGS_COUNT 10

typedef short ITEMTYPE;
typedef short OINUM;
typedef short HFII;
// LOGFONT 16 structure for compatibility with old extensions
/////////////////////////////////////////////////////////////
struct LOGFONTV1 {
	short			Height,
					Width,
					Escapement,
					Orientation,
					Weight;
	unsigned char	Italic,
					Underline,
					StrikeOut,
					CharSet,
					OutPrecision,
					ClipPrecision,
					Quality,
					PitchAndFamily;
	TCHAR			FaceName[32];
};

//////////////////////////////////////////////////////////////////////////////

//
// ObjectsCommon - Dynamic items
//
class Objects_Common {
public:

	unsigned long	Size;			// Total size of the structures

	unsigned short	Movements,		// Offset of the movements
					Animations, 	// Offset of the animations
					Version,		// For version versions > MOULI 
					Counter,		// Pointer to COUNTER structure
					Data,			// Pointer to DATA structure
					Free;			// IGNORE: Padding the shorts to 4 bytes
	unsigned long	OEFlags;		// New flags

	unsigned short	Qualifiers[8],	// Qualifier list (Max 8 qualifiers)
					Extension,		// Extension structure 
					Values,			// Values structure
					Strings,		// String structure
					
					Flags2,			// New news flags, before it was ocEvents
					OEPrefs;		// Automatically modifiable flags
	unsigned long	Identifier;		// Identifier d'objet

	COLORREF		BackColor;		// Background color
	unsigned long	FadeIn,			// Offset fade in 
					FadeOut,		// Offset fade out 
					ValueNames,		// For the debugger
					StringNames;

};
//typedef OC * LPOC;
//typedef OC * fpoc;

struct OCValues
{
	unsigned short	number;
	long			values[1];
};
// typedef	OCValues *			LPOCVALUES;

struct OCStrings
{
	unsigned short	number;
	TCHAR			str[2];
};
// typedef	OCStringsA*			LPOCSTRINGS;
// typedef	OCStringsW*			LPOCSTRINGS;



typedef struct
{
	unsigned short	number;
	char	str[2];

} OCValueNames;
typedef	OCValueNames*		LPOCVALUENAMES;

#define	OCFLAGS2_DONTSAVEBKD		bit1
#define	OCFLAGS2_SOLIDBKD			bit2
#define	OCFLAGS2_COLBOX				bit3
#define	OCFLAGS2_VISIBLEATSTART		bit4
#define	OCFLAGS2_OBSTACLESHIFT		bit3
#define	OCFLAGS2_OBSTACLEMASK		0x0030
#define	OCFLAGS2_OBSTACLE_SOLID		bit5
#define	OCFLAGS2_OBSTACLE_PLATFORM	bit6
#define	OCFLAGS2_OBSTACLE_LADDER	0x0030
#define	OCFLAGS2_AUTOMATICROTATION	bit7

//////////////////////////////////////////////////////////////////////////////
//
// Counter
//
struct counter {
	unsigned short	ctSize;
	long			ctInit,				// Initial value
					ctMini,				// Minimal value
					ctMaxi;				// Maximal value
};
//typedef counter tagCT;
//typedef counter	* fpct;
//typedef counter	* fpCounter;

//////////////////////////////////////////////////////////////////////////////
//
// Animations
//

// Anim header
struct AnimHeader {
	unsigned short	Size,
					AnimMax;			// New V2, number of entries in offset table
	short			OffsetToAnim[16];	// Minimum 16 animations, can be larger!
};
//typedef AnimHeader * fpAnimHeader;
//typedef AnimHeader * fpah;
//typedef AnimHeader * LPAH;

// Animation structure - collection of AnimDirections
struct Animation {
	short	OffsetToDir[32];	// Offset to animations with a direction (32 dirs max)
};
//typedef Animation *	fpAnim;
//typedef Animation *	fpan;
//typedef Animation *	LPAN;

// AnimDirection - Info about a specific direction in a specific animation
struct AnimDirection {
	unsigned char	MinSpeed;		// Minimum speed
	unsigned char	MaxSpeed;		// Maximum speed
	short			Repeat,			// Number of loops
					RepeatFrame,	// Where to loop
					NumberOfFrame,	// Number of frames
					Frame[1];		// Frames
};
#define sizeof_AnimDirection	(sizeof(AnimDirection)-2)
//typedef AnimDirection *	fpAnimDir;
//typedef AnimDirection *	fpad;
//typedef AnimDirection *	LPAD;

// Chunks de sauvegarde
#define EVTFILECHUNK_HEAD			B2L('E','R','>','>')
#define EVTFILECHUNK_EVTHEAD		B2L('E','R','e','s')
#define EVTFILECHUNK_EVENTS			B2L('E','R','e','v')
#define EVTFILECHUNK_NEWITEMS		B2L('>','N','I','<')
#define EVTFILECHUNK_END			B2L('<','<','E','R')

// Marquage des OI qualifiers
#define	OIFLAG_QUALIFIER			bit16
#define	NDQUALIFIERS				100
#define MAX_EVENTPROGRAMS			256

// COMMUNICATION STRUCTURE BETWEEN RUNTIME AND EDITOR
struct ComStructure
{
	unsigned int		command;

	unsigned short		runMode;
	unsigned int		timer;

	unsigned short		code;
	OINUM		oi;
	unsigned int		param[2];
	char		path[MAX_PATH];

};
const enum
{
	RUNCOMMAND_INITCOM=1,
	RUNCOMMAND_NEWEVENT,
	RUNCOMMAND_NEWSOUND,
	RUNCOMMAND_NEWOBJECT,
	RUNCOMMAND_NEWEVENTS,
	RUNCOMMAND_SETRUNMODE,
	RUNCOMMAND_CONTINUE,
	RUNCOMMAND_RESTART,
	RUNCOMMAND_QUIT,
	HANDLEMESSAGE_WAITFORCONFIRM,
	HANDLEMESSAGE_WAITFORCOMMAND,
	HANDLEMESSAGE_OK,
	HANDLEMESSAGE_QUIT,
	HANDLEMESSAGE_TIMEOUT
};

// DIFFERENTS MODES OF RUN
fancyenum(RUNMODE)
{
	RUNMODE_NONE,
	RUNMODE_STOPPED,
	RUNMODE_RUNNING,
	RUNMODE_PLAYING,
	RUNMODE_STEPPING,
	RUNMODE_PAUSED,
	RUNMODE_WAITING,
	RUNMODE_WAITINGQUIT,
};}


// -------------------------------------------------------------------------
// MOVEMENT DEFINITIONS
// -------------------------------------------------------------------------

// Definition of animation codes
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
fancyenum(ANIMID)
{
	STOP,
	WALK,
	RUN,
	APPEAR,
	DISAPPEAR,
	BOUNCE,
	SHOOT,
	JUMP,
	FALL,
	CLIMB,
	CROUCH,
	UNCROUCH,
	USER_DEFINED,
};}
//#define  	ANIMID_MAX				16
#define		ANIMNAME_MAXCHAR		64

// Definition of direction codes
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define		DIRF_ALL				0xFFFFFFFF
#define		DIRID_STATIC			32
#define		DIRID_MAX				32
#define		DIRID_SEE				30
#define		DIRID_SE				28
#define		DIRID_SSE				26
#define		DIRID_S					24
#define		DIRID_SSW				22
#define		DIRID_SW				20
#define		DIRID_SWW				18
#define		DIRID_W					16
#define		DIRID_NWW				14
#define		DIRID_NW				12
#define		DIRID_NNW				10
#define		DIRID_N					8
#define		DIRID_NNE				6
#define		DIRID_NE				4
#define		DIRID_NEE				2
#define		DIRID_E					0

// Definition of control types
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define		MVCONT_COMPUTER			0
#define		MVCONT_PLAYER1			1
#define		MVCONT_PLAYER2			2
#define		MVCONT_PLAYER3			3
#define		MVCONT_PLAYER4			4
#define		NB_MVCONT				5
#define		MAX_PLAYERS				4

// Definition of joystick controls
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define		JOYSTICK_UP				0b00000001
#define		JOYSTICK_DOWN			0b00000010
#define		JOYSTICK_LEFT			0b00000100
#define		JOYSTICK_RIGHT			0b00001000
#define		JOYSTICK_FIRE1			0b00010000
#define		JOYSTICK_FIRE2			0b00100000
#define		JOYSTICK_FIRE3			0b01000000
#define		JOYSTICK_FIRE4			0b10000000

// Definition of movement types
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define		MVTYPE_STATIC			0
#define		MVTYPE_MOUSE			1
#define		MVTYPE_RACE				2
#define		MVTYPE_GENERIC			3
#define		MVTYPE_BALL				4
#define		MVTYPE_TAPED			5
#define		MVTYPE_INTELLIGENT		6
#define		MVTYPE_PINBALL			7
#define		MVTYPE_LIST				8
#define		MVTYPE_PLATFORM			9
#define		MVTYPE_GOMOVEMENT		10
#define		MVTYPE_DISAPPEAR		11
#define		MVTYPE_APPEAR			12
#define		MVTYPE_BULLET			13
#define		MVTYPE_EXT				14
#define		MVTYPE_MAX				15

#define		MVTACTION_SAVEPOSITION	0x1010
#define		MVTACTION_LOADPOSITION	0x1011

////////////////////////////////////////////////////////////////////////////////
// EVENTS
////////////////////////////////////////////////////////////////////////////////

// Eventgroup structure, before conditions and actions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct eventGroup {
	short	evgSize;				// 0 Size of the group (<=0)
	unsigned char	evgNCond;		// 2 Number of conditions
	unsigned char	evgNAct;		// 3 Number of actions
	unsigned short	evgFlags;		// 4 Flags 
	short	evgInhibit;				// 6 If the group is inhibited
	unsigned short	evgInhibitCpt;	// 8 Counter
	unsigned short	evgIdentifier;	// 10 Unique identifier
	short 	evgUndo;				// 12 Identifier for UNDO
};
typedef		eventGroup	*		PEVG;
typedef		eventGroup	*		LPEVG;
#define		EVG_SIZE				14

// MACRO: next group
#define		EVGNEXT(evgPtr)		   	((LPEVG)((char *)evgPtr-evgPtr->evgSize))
// MACRO: first event
#define		EVGFIRSTEVT(p)	   		((LPEVT)((char *)p+sizeof(eventGroup)))
// MACRO: number of events in the group
#define		EVGNEVENTS(p)	 		(p->evgNCond+p->evgNAct)

// Internal flags of eventgroups
#define		EVGFLAGS_ONCE			bit1
#define		EVGFLAGS_NOTALWAYS		bit2
#define		EVGFLAGS_REPEAT			bit3
#define		EVGFLAGS_NOMORE			bit4
#define		EVGFLAGS_SHUFFLE		bit5
#define		EVGFLAGS_EDITORMARK		bit6
#define		EVGFLAGS_UNDOMARK		bit7
#define		EVGFLAGS_COMPLEXGROUP	bit8
#define		EVGFLAGS_BREAKPOINT		bit9
#define		EVGFLAGS_ALWAYSCLEAN	bit10
#define		EVGFLAGS_ORINGROUP		bit11
//#define		EVGFLAGS_2MANYACTIONS	bit11
#define		EVGFLAGS_STOPINGROUP	bit12
//#define		EVGFLAGS_NOTASSEMBLED	bit13
#define		EVGFLAGS_ORLOGICAL		bit13
#define		EVGFLAGS_GROUPED		bit14
#define		EVGFLAGS_INACTIVE		bit15
#define		EVGFLAGS_NOGOOD			bit16
#define		EVGFLAGS_LIMITED		(EVGFLAGS_SHUFFLE+EVGFLAGS_NOTALWAYS+EVGFLAGS_REPEAT+EVGFLAGS_NOMORE)
#define		EVGFLAGS_DEFAULTMASK	(EVGFLAGS_BREAKPOINT+EVGFLAGS_GROUPED)
//#define		EVGFLAGS_FADE			(EVGFLAGS_FADEIN|EVGFLAGS_FADEOUT)
	

// Condition and action structures
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct eventV1 {
	short	Size;				// 0 Size of the event
	union
	{
		short		Code;		// 2 Code (hi:NUM lo:TYPE)
		struct
		{
			char	Type,		// 2 Type of object
					Num;		// 3 Number of action/condition
		};
	};
	OINUM	Oi;					// 4 Object Identifier (if normal object)
	short	OiList;				// 6 Pointer
	char	Flags,				// 8 Flags
			Flags2,				// 9 Flags II
			NParams,			// 10 Number of parameters
			DefType;			// 11 If default, type
	// Conditions only
	short	Identifier;	  		// 12 Event identifier
								// 14 = total
};
// typedef	eventV1	*	LPEVTV1;

#define	CND_SIZEV1					sizeof(eventV1)
#define	ACT_SIZEV1					(sizeof(event)-2) // Ignore Identifier
#define	EVGFIRSTEVTV1(p)	   		((LPEVTV1)((char *)p+sizeof(eventGroup)))
#define	EVTPARAMSV1(p) 				((LPEVP)( p->evtCode<0 ? (unsigned char * )p+CND_SIZEV1 : (unsigned char * )p+ACT_SIZEV1) )
#define	EVTNEXTV1(p)		 		((LPEVTV1)((unsigned char * )p+p->evtSize))



// Differs from eventV1 by union variable types
struct event {
	short	evtSize;				// 0 Size of the event
	union
	{
		long		evtCode;		// 2 Code (hi:NUM lo:TYPE)
		struct
		{
			short	evtType,		// 2 Type of object
					evtNum;			// 4 Number of action/condition
		};
	};
	OINUM	evtOi;					// 6 Object Identifier (if normal object)
	short	evtOiList;				// 8 Pointer
	char	evtFlags,				// 10 Flags
			evtFlags2,				// 11 Flags II
			evtNParams,				// 12 Number of parameters
			evtDefType;				// 13 If default, type
	// Conditions
	short	evtIdentifier;	  		// 14 Event identifier
									// 16
};						
//typedef	event	*	PEVT;
//typedef	event	*	LPEVT;

#define	CND_SIZE					sizeof(event)
#define	ACT_SIZE					(sizeof(event)-2) // Ignore Identifier

// Definition of conditions / actions flags
fancyenum(EVFLAGS) {
	REPEAT = bit1,
	DONE = bit2,
	DEFAULT = bit3,
	DONEBEFOREFADEIN = bit4,
	NOTDONEINSTART = bit5,
	ALWAYS = bit6,
	BAD = bit7,
	BADOBJECT = bit8,
	DEFAULTMASK	= (ALWAYS+REPEAT+DEFAULT+DONEBEFOREFADEIN+NOTDONEINSTART),
};}

#define		ACTFLAGS_REPEAT			bit1


// For flags II
// -------------                    
#define		EVFLAG2_NOT			bit1
#define		EVFLAG2_NOTABLE		bit2
#define		EVFLAGS_NOTABLE		(bit2 << 8)
#define		EVFLAGS_MONITORABLE	bit3
#define		EVFLAGS_TODELETE	bit4
#define		EVFLAGS_NEWSOUND	bit5
#define		EVFLAG2_MASK		(EVFLAG2_NOT|EVFLAG2_NOTABLE|EVFLAGS_MONITORABLE)

// MACRO: Returns the code for an extension
#define		EXTCONDITIONNUM(i)		(-((short)(i>>16))-1)           
#define		EXTACTIONNUM(i)			((short)(i>>16))

// PARAM Structure
// ~~~~~~~~~~~~~~~
class EventParam { 
public:
	short			Size,
					Code;
	union {
		struct {
			short	W[8];
		};
		struct {
			long	L[4];
		};
	} evp;
};
//typedef	eventParam	*			PEVP;
//typedef	eventParam	*			LPEVP;
//typedef	eventParam	*			fpevp;

// MACRO: next parameter
#define		EVPNEXT(p)		   		((EventParam *)((char *)p+p->evpSize))


// Operators / expressions parameters
// --------------------------------------------------
#define		MIN_LONG				(-10000000L)
#define		MAX_LONG				(10000000L)
fancyenum(ExpParams) {
	Long = 1,
	Integer = Long,
	Float = Integer,
	GlobalVariable,
	String,
	AlterableValue,
	Flag,
};}
fancyenum(ExpReturns) {
	Long,
	Integer = Long,
	Float,
	String,
};}

#define		EXP_STOP				-1
#define		OPERATOR_START			0x00000000
#define		EXPL_END				0x00000000	
#define		EXPL_PLUS				0x00020000
#define		EXPL_MOINS				0x00040000
#define		EXPL_MULT				0x00060000
#define		EXPL_DIV				0x00080000                                         
#define		EXPL_MOD				0x000A0000
#define		EXPL_POW				0x000C0000
#define		EXPL_AND				0x000E0000
#define		EXPL_OR					0x00100000
#define		EXPL_XOR				0x00120000
#define		OPERATOR_END			0x00140000
struct expression {
	union {
		struct {
			long	Code;		// 2 Code (hi:NUM lo:TYPE)
		};
		struct {
			short	Type;		// 2 Type of object
			short	Num;			// 3 Expression number
		};
	};
	short	Size;
	union	
	{
		struct {
			short	Oi;
			short	OiList;
		} o;
		struct {
			long	LParam;
		} l;
		struct {
			double	Double;
			float	Float;
		} d;
		struct {
			short	WParam0;
			short	WParam1;
		} w;
		struct {
			short	Oi;
			short	OiList;
			short	Num;
		} v;
		struct {
			long	ExtCode;
			short	ExtNumber;
		} c;
	} u;
};
//typedef	expression *	LPEXP;
//typedef	expression *	PEXP;

#define		CMPOPE_EQU				(0)
#define		CMPOPE_DIF				(bit1)
#define		CMPOPE_LOWEQU			(bit2)
#define		CMPOPE_LOW				(CMPOPE_LOWEQU+CMPOPE_DIF)
#define		CMPOPE_GREEQU			(bit3)
#define		CMPOPE_GRE				(CMPOPE_GREEQU+CMPOPE_DIF)
#define		MAX_CMPOPE				6
#define		EXPNEXT(expPtr)			((expression *)((char *)expPtr+expPtr->expSize))
#define		EXPFLAG_STRING			bit1
#define		EXPFLAG_DOUBLE			bit2

struct expressionV1 {
	union
	{
		struct {
			short	Code;
		};
		struct {
			char	Type,
					Num;	
		};
	};
	short	Size;
	union {
		struct {
			short	Oi;
			short	OiList;
		} o;
		struct {
			long	LParam;
		} l;
		struct {
			double	Double;
			float	Float;
		} d;
		struct {
			short	WParam0;
			short	WParam1;
		} w;
		struct {
			short	Oi;
			short	OiList;
			short	Num;
		} v;
		struct {
			long	ExtCode;
			short	ExtNumber;
		} c;
	} u;
};
//typedef	expressionV1 *			LPEXPV1;

#define		EXPNEXTV1(expPtr)		((expressionV1 *)((char *)expPtr + expPtr->expSize))

/*
#define		Q_SPR	bit9
#define		Q_TXT	bit10
#define		Q_QST	bit11
#define		Q_ARE	bit12
#define		Q_CNT	bit13
#define		Q_PLA	bit14
#define		Q_GAM	bit15
#define		Q_TIM	bit16
#define		Q_COL	bit1
#define		Q_ZNE	bit2
#define		Q_MVT	bit3
#define		Q_ANI	bit4
#define		Q_OBJ	bit5
#define		Q_KEY	bit6
#define		Q_SYS	bit7
*/

// Information structure
// ----------------------------------------------
typedef struct tagEVO {                                  
	short		evoConditions;	  	// Conditions
	short		evoActions;			// Actions
	short		evoExpressions;		// Expressions
	short		evoMsgMenus;		// Menu strings for actions/conditions
	short		evoMsgDisplay;		// Display strings for actions/conditions
	short		evoMsgExpressions; 	// Menu/display strings for expressions
	short		evoMsgExpParams;	// Parameter string
	short		evoNConditions;		// Number of conditions
	} eventInfosOffsets;
typedef	eventInfosOffsets *		PEVO;
typedef	eventInfosOffsets *		LPEVO;

// Definition of the different system objects (TYPE<0)
// ----------------------------------------------------------
#define		TYPE_DIRECTION			-127
#define		TYPE_QUALIFIER			-126
#define		NUMBER_OF_SYSTEM_TYPES	7
fancyenum(OBJ) {
	// Built-in objects, included in all programs
	PLAYER = -7,
	KEYBOARD,
	CREATE,
	TIMER,
	GAME,
	STORYBOARD = GAME, // I think?
	SPEAKER,
	SYSTEM,
	// Built-in objects, have to be added to frame
	QUICK_BACKGROUND,
	BACKGROUND,
	ACTIVE,
	TEXT,
	QUESTION_AND_ANSWER,
	SCORE,
	LIVES,
	COUNTER,
	// Built-in, separated for some reason
	FIRST_C_OBJECT,			// First custom object (mfx)
	RTF = FIRST_C_OBJECT,	// NOT Rich Edit object, the other one.
	SUB_APPLICATION,
	SYS_OBJ,				// ?
	LAST = SYS_OBJ,
};}

// ------------------------------------------------------------
// EXTENSION OBJECT DATA ZONE
// ------------------------------------------------------------

// Flags 
fancyenum(OEFLAG) {
	DISPLAY_IN_FRONT		= bit1,
	BACKGROUND				= bit2,
	BACK_SAVE				= bit3,
	RUN_BEFORE_FADE_IN		= bit4,
	MOVEMENTS				= bit5,
	ANIMATIONS				= bit6,
	TAB_STOP				= bit7,
	WINDOW_PROC				= bit8,
	VALUES					= bit9,
	SPRITES					= bit10,
	INTERNAL_BACK_SAVE		= bit11,
	SCROLLING_INDEPENDENT	= bit12,
	QUICK_DISPLAY			= bit13,
	NEVER_KILL				= bit14,
	NEVER_SLEEP				= bit15,
	MANUAL_SLEEP			= bit16,
	TEXT					= 0x10000,
	DONT_CREATE_AT_START	= 0x20000,
};}

// Flags modifiable by the program
fancyenum(OEPREFS) {
	BACK_SAVE				= bit1,
	SCROLLING_INDEPENDENT	= bit2,
	QUICK_DISPLAY			= bit3,
	SLEEP					= bit4,
	LOAD_ON_CALL			= bit5,
	GLOBAL					= bit6,
	BACK_EFFECTS			= bit7,
	KILL					= bit8,
	INK_EFFECTS				= bit9,
	TRANSITIONS				= bit10,
	FINE_COLLISIONS			= bit11,
	APPLET_PROBLEMS			= bit12,
};}

// Running flags
fancyenum(REFLAG) {
	ONE_SHOT = bit1,
	DISPLAY = bit2,
	MSG_HANDLED = bit3,
	MSG_CATCHED = bit4,
	MSG_DEF_PROC = bit5,
	// ?
	MSGRETURNVALUE = bit7,
};}

fancyenum(CPF) {
	DIRECTION = bit1,
	ACTION = bit2,
	INITIALDIR = bit3,
	DEFAULTDIR = bit4
};}

///////////////////////////////////////////////////////////////////////
//
// DEFINITION OF THE DIFFERENT PARAMETERS
//
///////////////////////////////////////////////////////////////////////

fancyenum(Params) {
	Object = 1,						// ParamObject
	Time,							// ParamTime
	Border,							// ParamBorder
	Direction,						// ParamDir
	Integer,						// ParamInt
	Sample,							// ParamSound
	Music,							// ParamSound also
	Position,						// ParamPosition
	Create,							// ParamCreate
	Animation,						// ParamAnimation
	NoP,							// No struct (no data)
	Player,							// ParamPlayer
	Every,							// ParamEvery
	Key,							// ParamKey
	Speed,							// ParamSpeed
	NewPosition,					// ParamNewPosition
	Joystick_Direction, 			// ParamNewPosition also
	Shoot,							// ParamShoot
	Playfield_Zone,					// ParamZone
	System_Create,					// ParamCreate
	Expression = 22,				// ParamExpression
	Comparison,						// ParamComparison
	Colour,							// ParamColour
	Buffer,							// ParamBuffer
	Frame,							// ParamFrame - Storyboard frame number 
	Sample_Loop,					// ParamSoundLoop
	Music_Loop,						// ParamSoundLoop also
	New_Direction,					// ParamNewDir
	Text_Number,					// ParamTextNum
	Click,							// ParamClick
	Program = 33,					// ParamProgram
	Old_GlobalVariable, 			// DEPRECATED - DO NOT USE
	Condition_Sample,				// ParamCondSound
	Condition_Music,				// ParamCondSound also
	Editor_Comment,					// ParamEditorComment - Event editor comment
	Group,							// ParamGroup
	Group_Pointer,					// ParamGroupPointer
	Filename,						// ParamFilename
	String,							// ParamString
	Compare_Time,					// ParamCmpTime
	Paste_Sprite,					// ParamPasteSprite
	Virtual_Key_Code,				// ParamVKCode
	String_Expression,				// ParamStringExp
	String_Comparison,				// ParamStringExp also
	Ink_Effect,						// ParamInkEffect
	Menu,							// ParamMenu
	Global_Variable,				// ParamVariable
	Alterable_Value,				// ParamVariable also
	Flag,							// ParamVariable also also!
	Global_Variable_Expression, 	// ParamExpression
	Alterable_Value_Expression,		// ParamExpression also
	Flag_Expression,				// ParamExpression also also!
	Extension,						// ParamExtension
	_8Dirs,							// Param8Dirs
	Movement,						// ParamMvt
	Global_String,					// ParamVariable
	Global_String_Expression,		// ParamExpression also also also!!
	Program_2,						// ParamProgram2
	Alterable_String,				// ParamVariable
	Alterable_String_Expression,	// ParamExpression
	Filename_2,						// ParamFilename2 - allows filters of extensions or something.
	Effect,							// ParamEffect - HWA effect?
	Custom_Base = 1000,				// Base number for custom returns
};}
struct ParamObject {
	unsigned short	OffsetListOI,	//
					Number,			//
					Type;			// Version > FVERSION_NEWOBJECTS
};
struct ParamTime {
	long	Timer,		// Timer
			EqivLoops;	// Equivalent loops
};
struct ParamBorder {
	short BorderType;
	enum BorderFlags {
		LEFT	= bit1,
		RIGHT	= bit2,
		TOP		= bit3,
		BOTTOM	= bit4,
	};
};
struct ParamDir {
	short Direction;	// Direction, 0-31 presumably.
};
struct ParamInt  {
	short	Base,		// Short (or base)
			Maximum;	// Nothing (or maximum)
};
struct ParamSound {
	short	Handle,
			Flags;
	TCHAR	Name[64]; // Max sound name
	enum SoundFlags {
		UNINTERRUPTABLE = bit1,
		BAD = bit2,
		IPHONE_AUDIOPLAYER = bit3,
		IPHONE_OPENAL = bit4
	};
};
struct ParamPosition {
	short	Parent_ObjInfoNum,
			Flags,
			X, Y,
			Slope,
			Angle;				// Angle (forced short, 0-360?)
	long	Dir;				// Direction
	short	TypeParent,			//
			ObjInfoList,		//
			Layer;				//
};
struct ParamCreate {
	ParamPosition	Pos;	// Position structure
	HFII			FII;	// FrameItemInstance number
	OINUM			Oi;		// OI of the object to create
//	unsigned int	FII;	// No longer used
	unsigned int	Free;	// Ignore - Given due to no longer used FII
};
struct ParamShoot {
	ParamPosition	Pos;	// Position structure
	HFII			FII;	// FrameItemInstance number
	OINUM			Oi;		// OI of the object to shoot
	short			Speed;	// Speed
};
struct ParamAnimation {
	short	Number;
	TCHAR *	Name;
};
struct ParamNoP {
	short Unused;
};
struct ParamPlayer {
	short Number;
};
struct ParamEvery {
	long	Delay,
			Counter;
};
struct ParamKey {
	unsigned short VK_Code;	// Virtual Key Code
};
struct ParamSpeed {
	int Speed;
};
struct ParamNewPosition {
	unsigned short Direction; // The rest of the position variables are specified elsewhere
};
struct ParamZone {
	short	X1, Y1,	// Top-left corner
			X2, Y2;	// Bottom-right corner
};
struct ParamExpression {
	short	ID,
			Unknown[6];
};
struct ParamColour {
	COLORREF	RGB;
	long		ID;
};
struct ParamBuffer {
	void * Buffer; // Or it could be size, I dunno.
};
struct ParamFrame {
	unsigned short Number;
};
struct ParamSoundLoop {
	short NumOfLoops;
};
struct ParamNewDir {
	unsigned short	OldDir,	// No idea what's inside this struct, all we know is it's 4 bytes.
					NewDir;
};
struct ParamTextNum {
	int Number;
};
struct ParamClick {
	int Value;
	enum Masks {
		ClickMask = 0x00FF,
		DoubleClick = 0x0100
	};
};
struct ParamProgram {
	short	Flags;				// Default flags
	TCHAR	Path[MAX_PATH],		// Name of the program
			Command[108];		// Command line
	
	enum Masks {
		Wait = bit1,
		Hide = bit2,
	};
};
struct ParamCondSound {
	unsigned short	Number,
					Flags,
					Loops;
	TCHAR *			Name;
};
struct ParamEditorComment {
	LOGFONTV1		LogFont;			// Font 
	COLORREF		ColourFont,			// Text color
					ColourBack;			// Background color
	short			Align;				// Alignement flags
	unsigned short	TextId;				// Text number in the buffer
	TCHAR			Style[40];			// Style
};
struct ParamGroup {
	short			Flags,			// Active / Inactive?
					ID;				// Group identifier
	TCHAR			Title[80],		// Title (max 80? chars)
					Password[16];	// Protection (max 16? chars)
	unsigned long	Checksum;		// Checksum
	enum Masks {
		INACTIVE		= bit1,
		CLOSED			= bit2,
		PARENT_INACTIVE	= bit3,
		GROUP_INACTIVE	= bit4,
		GLOBAL			= bit5,
		/* Old flags
		FADE_IN			= bit3,
		FADE_OUT		= bit4,
		UNICODE			= bit5,*/
	};
};
#define GETEVPGRP(evpPtr) (paramGroup *)&evpPtr->evp.evp0
struct ParamGroupPointer {
	void *	PointTo;
	short	ID;
};
struct ParamFilename {
	TCHAR * FileName;
};
struct ParamString {
	TCHAR * String;
};
struct ParamCmpTime {
	long	Timer,
			Loops;
	short Comparison;
};
struct ParamPasteSprite {
	short	Flags,
			Security; // == padding?
};
struct ParamVKCode {
	short Code;
};
struct ParamStringExp {
	short	ID,
			Unknown[6];	// Assuming 6 from ParamExpression
};
struct ParamInkEffect {
	short	ID,			// ID of effect
			Parameter;	// Effect parameter
	long	Free;		// Ignore - free
};
struct ParamMenu {
	long	ID,
			Security; // == Padding?
};
struct ParamVariable {
	long Value; // Global variable, alterable value, flag
};
struct ParamExtension
{
	short	Size,
			Type,
			Code;
	char	Data[2];	// MaxSize = 512, Size = 12, not sure if those are related
};
#define		PARAM_EXTBASE			1000
struct Param8Dirs {
	long Flags;
	enum Mask {
		// Todo! Use bitXX.
	};
};
struct ParamMvt {
	short	mvtNumber;
	TCHAR	mvtName[32];	// Max movement name = 32 bytes
};
struct ParamProgram2 {
	short	Flags; // Default flags
	enum Masks {
		Wait = bit1,
		Hide = bit2,
	};
};
struct ParamEffect {
	TCHAR *	Name;
};

///////////////////////////////////////////////////////////////
// STRUCTURE FOR FAST LOOPS
///////////////////////////////////////////////////////////////////////
struct FastLoop
{
	TCHAR *			Next;
	TCHAR			Name[64];	// Max fast loop name (64 bytes)
	unsigned short	Flags;
	long			Index;
	enum Masks {
		Stop = bit1,
	};
};
//typedef	FastLoop *	LPFL;

///////////////////////////////////////////////////////////////
// DEBUGGER
///////////////////////////////////////////////////////////////
fancyenum(DEBUGGER) {
	DEBUGGER_RUNNING,
	DEBUGGER_TO_RUN,
	DEBUGGER_PAUSE,
	DEBUGGER_TO_PAUSE,
	DEBUGGER_STEP,
};}

// TREE identification
fancyenum(DBTYPE)
{
	SYSTEM,
	OBJECT,
	HO,
	MOVEMENT,
	ANIMATION,
	VALUES,
	COUNTER,
	SCORE,
	LIVES,
	TEXT,
	EXTENSION
};}

// Generic entries in the tree
enum
{
	DB_END = 0xFFFF,
	DB_PARENT = bit16
};
#define DB_EDITABLE		0x80

// Communication buffer size
#define DB_BUFFERSIZE			256

#define DB_MAXGLOBALVALUES		1000	// Maximum number of global values displayed in the debugger
#define DB_MAXGLOBALSTRINGS		1000

// System tree entries
fancyenum(SYSTEM_DB_TREE)
{
	DB_SYSTEM,
	DB_TIMER,
	DB_FPS,
	DB_FRAMENUMBER,
	DB_GLOBALVALUE,
	DB_GLOBALSTRING,
	DB_GVALUE,
	DB_GSTRING,
};}
// Headerobject tree entries
enum
{
	DB_XY,
	DB_SIZE
};

// Movements tree entries
enum
{
	DB_MOVEMENTS,
	DB_MVTTYPE,
	DB_SPEED,
	DB_DIRECTION,
	DB_ACCELERATION,
	DB_DECELERATION
};

// Animations tree entries
enum
{
	DB_ANIMATIONS,
	DB_ANIMNAME,
	DB_FRAME
};

// Values tree entries
#define GETDBPARAMTYPE(a)		((a&0xFF000000)>>24)
#define GETDBPARAMID(a)			((a&0x00FFFF00)>>8)
#define GETDBPARAMCOMMAND(a)	 (a&0x000000FF)
#define GETDBPARAM(a,b,c)		((a&0x000000FF)<<24)|((b&0xFFFF)<<8)|(c&0xFF)

struct EditDebugInfo
{
	TCHAR * Title;
	int		value;
	TCHAR * Text;
	int		lText;
};

///////////////////////////////////////////////////////////////////////
//
// RUNTIME BUFFER
//
///////////////////////////////////////////////////////////////////////

//Modes de demo
enum
{
	DEMONOTHING,
	DEMORECORD,
	DEMOPLAY,
};

//typedef void (*ACTIONENDROUTINE)();

//typedef void (*OBLROUTINE)(HeaderObject *);
struct objectsList {
	HeaderObject  *	oblOffset;
	void (* oblRoutine)(HeaderObject *);
};
// typedef	objectsList *			LPOBL;

#define		GAMEBUFFERS_SIZE	(12*1024)
#define		GAME_MAXOBJECTS		266
#define		OBJECT_MAX			128L
#define		OBJECT_SIZE			256L
#define		OBJECT_SHIFT		8
#define		MAX_INTERMEDIATERESULTS		256
#define		STEP_TEMPSTRINGS	64

struct runHeader2 {
	unsigned long	OldPlayer,		// Previous player entries
					NewPlayer,		// Modified player entries
					InputMask,		// Inhibated players entries
					InputPlayers;	// Valid players entries (mask!)
	unsigned char 	MouseKeys,		// Mousekey entries
					ActionLoop,		// Actions flag
				 	ActionOn,		// Flag: are we in actions?
					EnablePick;  	// Flag: Are we in pick for actions?

	int		  		EventCount;			// Number of the event 
	qualToOi *		EventQualPos;		// Position in event objects
	HeaderObject  *	EventPos;			// Position in event objects
	objInfoList *	EventPosOiList, 	// Position in oilist for TYPE exploration 
				*	EventPrev;			// Previous object address

	pev *			PushedEvents;		// 
	unsigned char *	PushedEventsTop,	// 
				  *	PushedEventsMax;	// 
	int	  			NewPushedEvents;	// 

	int		  		ActionCount;		// Action counter
	int		  		ActionLoopCount;	// Action loops counter
	void	(*ActionEndRoutine)();		// End of action routine
	unsigned short	CreationCount;		// Number of objects created since beginning of frame
	short			EventType;					
	POINT 			Mouse;				// Mouse coordinate
	POINT 			MouseClient;		// Mouse coordinates in the window
	short	  		CurrentClick,		// For click events II
					Free2;				// Ignore - Padding
	HeaderObject **	ShuffleBuffer,
				 ** ShufflePos;
	int		  		ShuffleNumber;			

	POINT			MouseSave;			// Mouse saving when pause
	int		  		PauseCompteur;
	unsigned long	PauseTimer;
	unsigned int	PauseVbl;
	FARPROC	   		LoopTraceProc,      // Debugging routine
					EventTraceProc;

};


// Flags pour rh3Scrolling
#define RH3SCROLLING_SCROLL					bit1
#define RH3SCROLLING_REDRAWLAYERS			bit2
#define RH3SCROLLING_REDRAWALL				bit3
#define RH3SCROLLING_REDRAWTOTALCOLMASK		bit4

#define	GAME_XBORDER		480
#define	GAME_YBORDER		300

struct runHeader3 {

	unsigned short	Graine,			// Random generator seed
					Free;			// Ignore - padding

	int		  		DisplayX,		// To scroll
					DisplayY;

	long			CurrentMenu;	// For menu II events

	int 			WindowSx,		// Window size
					WindowSy;

	short			CollisionCount;	// Collision counter 
	char			DoStop,			// Force the test of stop actions
					Scrolling;		// Flag: we need to scroll

	int		  		Panic,
			  		PanicBase,
			  		PanicPile,

//	short		  	XBorder_;		// Authorised border
//	short		  	YBorder_;
		 	  		XMinimum,   	// Object inactivation coordinates
					YMinimum,
					XMaximum,
			  		YMaximum,
					XMinimumKill,	// Object destruction coordinates
					YMinimumKill,
					XMaximumKill,
					YMaximumKill;
};

// Extensions 
// ~~~~~~~~~~~~~~~~~~~~
#define		KPX_MAXNUMBER					96
typedef struct tagKPXMSG {
	short 	( WINAPI * routine) (unsigned int, HWND, unsigned int, WPARAM, LPARAM);
	} kpxMsg;
#define		KPX_MAXFUNCTIONS				32
typedef struct tagKPXLIB {
	long 	( WINAPI * routine) (HeaderObject *, WPARAM, LPARAM);
	} kpxLib;
#define IsRunTimeFunctionPresent(num)	(num < KPX_MAXFUNCTIONS && ((RunHeader *)rdPtr->rHo.AdRunHeader)->rh4.rh4KpxFunctions[num].routine != NULL)
#define	CallRunTimeFunction(rdPtr,num,wParam,lParam)	(rdPtr->rHo.AdRunHeader->rh4.rh4KpxFunctions[num].routine((HeaderObject *)rdPtr, wParam, lParam) )
#define	CallRunTimeFunction2(hoPtr,num,wParam,lParam)	(hoPtr->AdRunHeader->rh4.rh4KpxFunctions[num].routine(hoPtr, wParam, lParam) )
#define	CallRunTimeFunction3(rh4_,num,wParam,lParam)	(rh4_.rh4KpxFunctions[num].routine(hoPtr, wParam, lParam) )

fancyenum(RFUNCTION) {
	//
	REHANDLE,				// Re-enable the call to Handle() every frame
	GENERATE_EVENT,			// Immediately create a triggered condition (do not call in functions that MMF hasn't began the call to, i.e. threads)
	PUSH_EVENT,				// Generates the event after the next event loop, safer for calling from threads and such.
	GET_STRING_SPACE_EX,	// Allocates memory from MMF for a char * or wchar_t *, letting you store a string.
	GET_PARAM_1,			// Retrieves the value of the first parameter of an a/c/e.
	GET_PARAM_2,			// Retrieves the value of 2nd+ parameter (first call with this is 2nd, next 3rd, etc).
	PUSH_EVENT_STOP,		// Removes event created by Push Event. Cannot do this with Generate Event as it happens immediately.
	PAUSE,
	CONTINUE,
	REDISPLAY,
	GET_FILE_INFOS,			// Retrieve information about the current app (see FILEINFOS namespace and Edif.Runtime.cpp)
	SUB_CLASS_WINDOW,
	REDRAW,					// Causes the object to redraw [a certain part or 100%?].
	DESTROY,
	GET_STRING_SPACE,		// Deprecated GET_STRING_SPACE_EX
	EXECUTE_PROGRAM,
	GET_OBJECT_ADDRESS,
	GET_PARAM,
	GET_EXP_PARAM,
	GET_PARAM_FLOAT,
	EDIT_INT,
	EDIT_TEXT,
	CALL_MOVEMENT,
	SET_POSITION,
	GET_CALL_TABLES
};}

#define CNC_GetParameter(rdPtr)							CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM, 0xFFFFFFFF, 0)
#define CNC_GetIntParameter(rdPtr)						CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM, 0, 0)
#define CNC_GetStringParameter(rdPtr)					CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM, 0xFFFFFFFF, 0)
#define CNC_GetFloatParameter(rdPtr)					CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM, 2, 0)
#define CNC_GetFloatValue(rdPtr, par)					CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM_FLOAT, par, 0)
#define CNC_GetFirstExpressionParameter(rdPtr, lParam, wParam)	CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM_1, wParam, lParam)
#define CNC_GetNextExpressionParameter(rdPtr, lParam, wParam)	CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM_2, wParam, lParam)

// ??
#define MAX_FRAMERATE						10

typedef void (* CALLANIMATIONS) (HeaderObject *, int);
#define callAnimations(hoPtr, anim) ( (hoPtr->hoAdRunHeader)->rh4.rh4Animations(hoPtr, anim) )

typedef unsigned int (* CALLDIRATSTART) (HeaderObject *, unsigned int);
#define callDirAtStart(hoPtr, initDir) ( (hoPtr->hoAdRunHeader)->rh4.rh4DirAtStart(hoPtr, initDir) )

typedef BOOL (* CALLMOVEIT) (HeaderObject *);
#define callMoveIt(hoPtr) ( (hoPtr->hoAdRunHeader)->rh4.rh4MoveIt(hoPtr) )

typedef BOOL (* CALLAPPROACHOBJECT) (HeaderObject *, int destX, int destY, int maxX, int maxY, int htFoot, int planCol, int& x, int &y);
#define callApproachObject(x, y, hoPtr, destX, destY, maxX, maxY, htFoot, planCol) ( (hoPtr->hoAdRunHeader)->rh4.rh4ApproachObject(hoPtr, destX, destY, maxX, maxY, htFoot, planCol, x, y) )

typedef void (* CALLCOLLISIONS) (HeaderObject *);
#define callCollisions(hoPtr) ( (hoPtr->hoAdRunHeader)->rh4.rh4Collisions(hoPtr) )

typedef BOOL (* CALLTESTPOSITION) (HeaderObject *, int x, int y, int htFoot, int planCol, BOOL flag);
#define callTestPosition(hoPtr, x, y, htFoot, planCol) ( (hoPtr->hoAdRunHeader)->rh4.rh4TestPosition(hoPtr, x, y, htFoot, planCol, 0) )

typedef unsigned char (* CALLGETJOYSTICK) (HeaderObject *, int);
#define callGetJoystick(hoPtr, player) ( (hoPtr->hoAdRunHeader)->rh4.rh4Joystick(hoPtr, player) )

typedef BOOL (* CALLCOLMASKTESTRECT) (HeaderObject *, int x, int y , int sx, int sy, int nLayer, int plan);
#define callColMaskTestRect(hoPtr, x, y, sx, sy, nLayer, plan) ( (hoPtr->hoAdRunHeader)->rh4.rh4ColMaskTestRect(hoPtr, x, y, sx, sy, nLayer, plan) )

typedef BOOL (* CALLCOLMASKTESTPOINT) (HeaderObject *, int x, int y, int nLayer, int plan);
#define callColMaskTestPoint(hoPtr, x, y, nLayer, plan) ( (hoPtr->hoAdRunHeader)->rh4.rh4ColMaskTestPoint(hoPtr, x, y, nLayer, plan) )

struct runHeader4 {

	kpj *			rh4KpxJumps;				// Jump table offset
	short			rh4KpxNumOfWindowProcs,		// Number of routines to call
					rh4Free;					// Padding - ignore
	kpxMsg			rh4KpxWindowProc[96];		// Message handle routines (max 96)
	kpxLib			rh4KpxFunctions[32];		// Available internal routines (max 32)

	// ?
	void (* Animations)(HeaderObject  *, int);
	// Get direction at start
	unsigned int (* DirAtStart)(HeaderObject  *, unsigned int);						
	// Move object
	BOOL (* MoveIt)(HeaderObject  *);
	// Approach object
	BOOL (* rh4ApproachObject)(HeaderObject *, int destX, int destY, int maxX, int maxY, int htFoot, int planCol, int& x, int &y);
	// ?
	void (* rh4Collisions)(HeaderObject  *);
	// ?
	void (* rh4TestPosition)(HeaderObject  *);
	// ?
	unsigned char (* rh4GetJoystick)(HeaderObject  *, int);
	// ?
	BOOL (* rh4ColMaskTestRect)(HeaderObject *, int x, int y , int sx, int sy, int nLayer, int plan);
	// ?
	BOOL (* rh4ColMaskTestPoint)(HeaderObject *, int x, int y, int nLayer, int plan);

	unsigned long		rh4SaveVersion;
	event *				rh4ActionStart;			// Save the current action
	int					rh4PauseKey;
	TCHAR *				rh4CurrentFastLoop;
	int					rh4EndOfPause;
	int		  			rh4EventCountOR;		// Number of the event for OR conditions
	short				rh4ConditionsFalse,
						rh4MouseWheelDelta;
	int					rh4OnMouseWheel;
	TCHAR *				rh4PSaveFilename;
	unsigned int		rh4MusicHandle;
	unsigned long		rh4MusicFlags,
						rh4MusicLoops;
	int					rh4LoadCount;
	short				rh4DemoMode,
						rh4SaveFrame;
	CDemoRecord *		rh4Demo;
	int					rh4SaveFrameCount;
	double				rh4MvtTimerCoef;
	CIPhoneJoystick *	rh4IPhoneJoystick;
	CIPhoneAd *			rh4IPhoneAd;
	char				rh4QuitString[32];		// Free, unknown usage
	
	unsigned long		rh4PickFlags0,			// 00-31
						rh4PickFlags1,			// 31-63
						rh4PickFlags2,			// 64-95
						rh4PickFlags3,			// 96-127
				  *		rh4TimerEventsBase;		// Timer events base

	short				rh4DroppedFlag,
						rh4NDroppedFiles;
	TCHAR *				rh4DroppedFiles;
	FastLoop *			rh4FastLoops;
	TCHAR *				rh4CreationErrorMessages;
	CValue				rh4ExpValue1,				// New V2
						rh4ExpValue2;

	long				rh4KpxReturn;				// WindowProc return 
	objectsList *		rh4ObjectCurCreate;
	short				rh4ObjectAddCreate;
	unsigned short		rh4Free10;					// For step through : fake key pressed
	HINSTANCE			rh4Instance;				// Application instance
	HWND				rh4HStopWindow;				// STOP window handle
	char				rh4DoUpdate,				// Flag for screen update on first loop
						rh4MenuEaten;				// Menu handled in an event?
	short				rh4Free2;
	int					rh4OnCloseCount;			// For OnClose event
	short				rh4CursorCount,				// Mouse counter
						rh4ScrMode;					// Current screen mode
	HPALETTE			rh4HPalette;				// Handle current palette
	int 				rh4VBLDelta;				// Number of VBL
	unsigned long		rh4LoopTheoric,				// Theorical VBL counter
						rh4EventCount;
	drawRoutine *		rh4FirstBackDrawRoutine,	// Backrgound draw routines list
				*		rh4LastBackDrawRoutine;		// Last routine used

	unsigned long		rh4ObjectList;				// Object list offset
	short				rh4LastQuickDisplay;		// Quick - display list
	unsigned char		rh4CheckDoneInstart,		// Correct start of frame with fade in
						rh4Free0;					// Ignore - padding
	mv *				rh4Mv;						// Yves' data
	HCURSOR				rh4OldCursor;				// Old cursor for Show / HideMouse in Vitalize! mode
	HeaderObject  *		rh4_2ndObject;	 			// Collision object address
	short 				rh4_2ndObjectNumber,        // Number for collisions
						rh4FirstQuickDisplay;		// Quick-display object list
	int					rh4WindowDeltaX,			// For scrolling
						rh4WindowDeltaY;               
	unsigned int		rh4TimeOut;					// For time-out
	int					rh4MouseXCenter,			// To correct CROSOFT bugs
						rh4MouseYCenter,			// To correct CROSOFT bugs
						rh4TabCounter;				// Objects with tabulation
	
	unsigned long		rh4AtomNum,					// For child window handling
						rh4AtomRd,
						rh4AtomProc;
	short				rh4SubProcCounter,			// To accelerate the windows
						rh4Free3;
	
	int					rh4PosPile;								// Expression evaluation pile position
	expression *		rh4ExpToken;							// Current position in expressions
	CValue *			rh4Results[MAX_INTERMEDIATERESULTS];	// Result pile
	long				rh4Operators[MAX_INTERMEDIATERESULTS];	// Operators pile
	
	TCHAR **			rh4PTempStrings;		// Debut zone 256 long
	int					rh4MaxTempStrings;
	long				rh4Free4[256-2];		// Free buffer

	int					rh4NCurTempString;					// Pointer on the current string
	unsigned long		rh4FrameRateArray[MAX_FRAMERATE];	// Framerate calculation buffer
	int					rh4FrameRatePos;					// Position in buffer
	unsigned long		rh4FrameRatePrevious;				// Previous time 
};

fancyenum(GAMEFLAGS) {
	VBLINDEP = bit2,
	LIMITED_SCROLL = bit3,
	FIRST_LOOP_FADE_IN = bit5,
	LOAD_ON_CALL = bit6,
	REAL_GAME = bit7,
	PLAY = bit8,
	//FADE_IN = bit8,
	//FADE_OUT = bit9,
	INITIALISING = bit10,
};}

struct RunHeader {
	void *				IdEditWin,			// Win *, but evaluates to void *
		 *				IdMainWin;
	void *				IdAppli;			// Appli *, but evaluates to void *

	HWND				HEditWin,
						HMainWin,
						HTopLevelWnd;

	CRunApp *			App;				// Application info
	CRunFrame *			Frame;				// Frame info

	unsigned int		JoystickPatch;		// To reroute the joystick

	unsigned char		Free10,				// Current movement needs to be stopped
						Free12, 			// Event evaluation flag
						NPlayers,			// Number of players
						MouseUsed;			// Players using the mouse

	unsigned short		GameFlags,			// Game flags
						Free;				// Alignment
	unsigned int		Player;				// Current players entry

	short 				Quit,
						QuitBis; 			// Secondary quit (scrollings)
	unsigned int		Free11,				// Value to return to the editor
						QuitParam;

	// Buffers
	int					NObjects,
						MaxObjects;
	
	unsigned int		Free2[4];			// !No description in original SDK

	int 				NumberOi;			// Number of OI in the list
	objInfoList *		OiList;				// ObjectInfo list

	unsigned int *		Events[7+1],		// Events pointers (NUMBER_OF_SYSTEM_TYPES+1)
				 *		EventLists,		 	// Pointers on pointers list
				 *		Free8,				// Timer pointers
				 *		EventAlways;		// Pointers on events to see at each loop
	eventGroup *		Programs;			// Program pointers
	short *				LimitLists;			// Movement limitation list
	qualToOi *			QualToOiList;		// Conversion qualifier->oilist

	unsigned int		SBuffers;			// Buffer size /1024	
	unsigned char * 	Buffer,				// Position in current buffer
				  * 	FBuffer,			// End of current buffer
				  * 	Buffer1,			// First buffer
				  * 	Buffer2;			// Second buffer

	int 				LevelSx,			// Window size
						LevelSy,
						WindowX,   			// Start of window in X/Y
						WindowY;

	unsigned int		VBLDeltaOld,		// Number of VBL
						VBLObjet,			// For the objects
						VBLOld;				// For the counter

	int					EventsSize;						
	unsigned short		MT_VBLStep,   		// Path movement variables
						MT_VBLCount;
	unsigned int		MT_MoveStep;

	int					LoopCount;			// Number of loops since start of level
	unsigned int		Timer,				// Timer in 1/50 since start of level
						TimerOld,			// For delta calculation
						TimerDelta;			// For delta calculation again

	eventGroup *		EventGroup;			// Current group
	long 				CurCode;			// Current event
	short				CurOi,
						Free4;				// Alignment
	long				CurParam[2];
	short 				CurObjectNumber,	// Object number
						FirstObjectNumber;	// Number, for collisions

	long				OiListPtr;			// OI list enumeration
	short 				ObListNext,			// Branch label

						DestroyPos;
	long				Free5,				
						Free6;

	runHeader2			rh2;				// Sub-structure #1
	runHeader3			rh3;				// Sub-structure #2
	runHeader4			rh4;				// Sub-structure #3

	unsigned long *		DestroyList;		// Destroy list address

	int					DebuggerCommand;	// Current debugger command
	char				Free13[DB_BUFFERSIZE];		// Buffer libre!
	void *				DbOldHO;
	unsigned short		DbOldId,
						Free7;					

	objectsList *		ObjectList;			// Object list address

};
//typedef	RunHeader 	* fprh;
//typedef	RunHeader 	* RunHeader *;
//typedef	RunHeader 	* LPRUNHEADER;


///////////////////////////////////////////////////////////////
//
// RUNTIME OBJECT DEFINITION
//
///////////////////////////////////////////////////////////////

#define HOX_INT

struct HeaderObject {
	short  				Number,			// Number of the object
		 				NextSelected;	// Selected object list. Do not move from &NextSelected == (this+2).

	int					Size;			// Structure size
    RunHeader *			AdRunHeader;	// Run-header address
	HeaderObject *		Address;			
	short				HFII,			// Number of LevObj
		  				Oi,				// Number of ObjInfo
						NumPrev,		// Same ObjInfo previous object
						NumNext,		// ... next
						Type;			// Type of the object
	unsigned short		CreationId;		// Number of creation
	objInfoList *		OiList;			// Pointer to OILIST information
	unsigned int *		Events,			// Pointer to specific events
						Free0;			// Ignore - reserved/unused
	unsigned char *		PrevNoRepeat,	// One-shot event handling
				  *		BaseNoRepeat;
	int 				Mark1,			// Number of loop marker for the events
						Mark2;
	TCHAR *				MT_NodeName;	// Name of the current node for path movements
	int					EventNumber;	// Number of the event called (for extensions)
	int					Free2;			// Ignore - reserved/unused
	Objects_Common *	Common;			// Common structure address

	union {
		struct {
			int CalculX,	// Low weight value
				X,          // X coordinate
				CalculY,	// Low weight value
				Y;			// Y coordinate
		};
		struct {
			__int64 CalculXLong,
					CalculYLong;
		};
	};
	int					ImgXSpot, ImgYSpot,		// Hot spot of the current image
						ImgWidth, ImgHeight;	// Width/Height of the current picture
	RECT				Rect;					// Display rectangle
	
	unsigned int		OEFlags;		// Objects flags
	short				Flags;			// Flags
	unsigned char		SelectedInOR,	// Selection lors d'un evenement OR
						Free;			// Ignore - used for struct member alignment
	int					OffsetValue;	// Values structure offset
	unsigned int		Layer;			// Layer

	short (* hoHandleRoutine)(HeaderObject *);	// General handle routine
	short (* hoModifRoutine)(HeaderObject *);  	// Modification routine when coordinates have been modified
	short (* hoDisplayRoutine)(HeaderObject *);	// Display routine

	short				LimitFlags,			// Collision limitation flags
						NextQuickDisplay;	// Quickdraw list
	saveRect			BackSave;			// Background
	
	EventParam *		CurrentParam;		// Address of the current parameter

	int					OffsetToWindows;	// Offset to windows
	unsigned int		Identifier;			// ASCII identifier of the object

};
// typedef	LPHO HeaderObject*;

#define	HOF_DESTROYED		bit1
#define	HOF_TRUEEVENT		bit2
#define	HOF_REALSPRITE		bit3
#define	HOF_FADEIN			bit4
#define	HOF_FADEOUT			bit5
#define	HOF_OWNERDRAW		bit6
#define	HOF_NOCOLLISION		bit14
#define	HOF_FLOAT			bit15
#define	HOF_STRING			bit16


// --------------------------------------
// Object's movement structure
// --------------------------------------
struct rMvt {

	int  	rmAcc;						// Current acceleration
	int  	rmDec;						// Current Decelaration 
	int		rmCollisionCount;			// Collision counter
	int		rmFree2;
	int  	rmStopSpeed;				// If stopped: speed to take again
	int  	rmFree0;					
	int  	rmFree1;					
	int  	rmAccValue;					// Acceleration calculation
	int  	rmDecValue;					// Deceleration calculation
	short	rmEventFlags;				// To accelerate events
	int  	rmFree3;	
	int  	rmFree4;
	BOOL	rmFree5;
	BOOL	rmFree6;

 	BOOL	rmFree7;
	BOOL	rmMoveFlag;					// Messages/movements

	BOOL	rmWrapping;					// For CHECK POSITION
	BOOL 	rmReverse;					// Ahead or reverse?

	BOOL  	rmBouncing;					// Bouncing?
	int		rmMvtNum;					// Number of the current movement

	union	
	{
		struct
		{
			unsigned char 	rmMvData[80];			// Space for data
		};
		struct	
		{
			int		MT_Speed;
			int	 	MT_Sinus;
			int	 	MT_Cosinus;
			unsigned int 	MT_Longueur;
			int	 	MT_XOrigin;
			int	 	MT_YOrigin;
			int	 	MT_XDest;
			int	 	MT_YDest;
			int	 	MT_MoveNumber;
			void * MT_MoveAddress;
			BOOL	MT_Direction;
			void *	MT_Movement;
			int		MT_Calculs;
			int		MT_XStart;
			int		MT_YStart;
			int		MT_Pause;
			TCHAR *	MT_GotoNode;
		};
		struct
		{
			int	 	MM_DXMouse;
			int	 	MM_DYMouse;
			int	 	MM_FXMouse;
			int	 	MM_FYMouse;
			int	 	MM_Stopped;
			int	 	MM_OldSpeed;
			unsigned char	MM_MvOpt;
		};	
		struct
		{
			int	 	MG_Bounce;
			long	MG_OkDirs;
			int	 	MG_BounceMu;
			int	 	MG_Speed;
			int 	MG_LastBounce;
			int	 	MG_DirMask;
			unsigned char	MG_MvOpt;
		};
		struct
		{
			int	 	MR_Bounce;
			int	 	MR_BounceMu;
			int	 	MR_Speed;
			int	 	MR_RotSpeed;
			int	 	MR_RotCpt;
			int	 	MR_RotPos;
			int	 	MR_RotMask;
			int	 	MR_OkReverse;
			int	 	MR_OldJoy;
			int 	MR_LastBounce;
			unsigned char	MR_MvOpt;
		};
		struct
		{
			long	MB_StartDir;
			int	 	MB_Angles;
			int	 	MB_Securite;
			int	 	MB_SecuCpt;
			int	 	MB_Bounce;
			int	 	MB_Speed;
			int	 	MB_MaskBounce;
			int	 	MB_LastBounce;
			int	 	MB_Blocked;
		};
		struct
		{
			int	 	MBul_Wait;
			HeaderObject *	MBul_ShootObject;
		};
		struct
		{
			int		MP_Type;
			int	 	MP_Bounce;
			int	 	MP_BounceMu;
			int	 	MP_XSpeed;
			int	 	MP_Gravity;
			int	 	MP_Jump;
			int	 	MP_YSpeed;
			int	 	MP_XMB;
			int	 	MP_YMB;
			int	 	MP_HTFOOT;
			int	 	MP_JumpControl;
			int	 	MP_JumpStopped;
			int	 	MP_PreviousDir;
			HeaderObject *	MP_ObjectUnder;
			int		MP_XObjectUnder;
			int		MP_YObjectUnder;
			BOOL	MP_NoJump;
		};
	};
};
// typedef rMvt*	LPRMV;
enum
{
	MPTYPE_WALK,
	MPTYPE_CLIMB,
	MPTYPE_JUMP,
	MPTYPE_FALL,
	MPTYPE_CROUCH,
	MPTYPE_UNCROUCH
};

// ----------------------------------------
// Object's animation structure
// ----------------------------------------
struct rAni {
	int	 			AnimForced,		// Flags if forced
		 			DirForced,
		 			SpeedForced;
	BOOL			Stopped;
	int	 			On;				// Current ation
	Animation *		Offset;
	int	 			Dir,			// Direction of current ation
		 			PreviousDir;	// Previous OK direction
	AnimDirection *	DirOffset;
	int	 			Speed,
		 			MinSpeed,		// Minimum speed of movement
		 			MaxSpeed,		// Maximum speed of movement
		 			DeltaSpeed,
		 			Counter,		// Animation speed counter
		 			Delta,			// Speed counter
		 			Repeat,			// Number of repeats
		 			RepeatLoop,		// Looping picture
		 			Frame,			// Current frame
		 			NumberOfFrame,	// Number of frames
	
		 			FrameForced;

};
//typedef rAni*	LPRA;

// ----------------------------------------
// Sprite display structure
// ----------------------------------------
struct Sprite {
	int	 			Flash,				// When flashing objects
		 			FlashCount,
		   			Layer,				// Layer
					ZOrder,				// Z-order value
		 			CreationFlags;		// Creation flags
	COLORREF		BackColor;			// background saving color
	unsigned int	Effect;				// Sprite effects
	LPARAM			EffectParam;
	unsigned short	Flags,				// Handling flags
					FadeCreationFlags;	// Saved during a fadein
};
//typedef Sprite *	LPRSP;
//typedef rSpr	Sprite;

fancyenum(RSFLAG) {
	HIDDEN = bit1,
	INACTIVE = bit2,
	SLEEPING = bit3,
	SCALE_RESAMPLE = bit4,
	ROTATE_ANTIA = bit5,
	VISIBLE = bit6,
	CREATED_EFFECT = bit7
};}


// ----------------------------------------
// Objects's internal variables (build # >= 243)
// ----------------------------------------
struct AltVals {
	CValue *		Values;
	long			Free1[26-1];	// 26 = number of alterable values
	long			ValueFlags;
	unsigned char	Free2[26];		// 26 = number of alterable values
	TCHAR *			Strings[10];	// 10 = number of alterable strings
};
//typedef AltVals *	LPRVAL;
	
/*
	Obsolete (build 242 or below) - Note: don't remove this (for the SDK)
  
typedef struct tagRV {
	long	rvValues[VALUES_NUMBEROF_ALTERABLE];
	long	rvValueFlags;
	unsigned char	rvValuesType[VALUES_NUMBEROF_ALTERABLE];
	char *	rvStrings[STRINGS_NUMBEROF_ALTERABLE];
	} AltVals;
typedef AltVals *	LPRVAL;
*/


// -----------------------------------------------
// Objects animation and movement structure
// -----------------------------------------------
#ifndef ANGLETYPE
	#ifdef HWABETA
		typedef float AngleVar;
	#else
		typedef int AngleVar;
	#endif
#endif
typedef void (* RCROUTINE)(HeaderObject *);
struct rCom {

	int		rcOffsetAnimation; 			// Offset to anims structures
	int		rcOffsetSprite;				// Offset to sprites structures
	RCROUTINE	rcRoutineMove;			// Offset to movement routine
	RCROUTINE	rcRoutineAnimation;		// Offset to animation routine

	int	   	rcPlayer;					// Player who controls

	int	   	rcNMovement;				// Number of the current movement
	CRunMvt *	rcRunMvt;				// Pointer to extension movement
	Spr  	rcSprite;					// Sprite ID if defined
	int	 	rcAnim;						// Wanted animation
	int	   	rcImage;					// Current frame
	float	rcScaleX;					
	float	rcScaleY;
	AngleVar	rcAngle;
	int	   	rcDir;						// Current direction
	int	   	rcSpeed;					// Current speed
	int	   	rcMinSpeed;					// Minimum speed
	int	   	rcMaxSpeed;					// Maximum speed
	BOOL	rcChanged;					// Flag: modified object
	BOOL	rcCheckCollides;			// For static objects

	int	 	rcOldX;            			// Previous coordinates
	int	 	rcOldY;
	int	 	rcOldImage;
	AngleVar	rcOldAngle;
	int	 	rcOldDir;
	int	 	rcOldX1;					// For zone detections
	int	 	rcOldY1;
	int	 	rcOldX2;
	int	 	rcOldY2;

	long	rcFadeIn;
	long	rcFadeOut;

};
	
	
// ------------------------------------------------------------
// ACTIVE OBJECTS DATAZONE
// ------------------------------------------------------------
struct RunObject {

	HeaderObject  	roHo;		  		// Common structure 

	rCom			roc;				// Anim/movement structure
	rMvt			rom;				// Movement structure 
	rAni			roa;				// Animation structure 
	Sprite			ros;				// Sprite handling structure
	AltVals			rov;				// Values structure

};
//typedef	RunObject *				RunObject *;
//typedef RunObject *				LPRUNOBJECT;

#define	GOESINPLAYFIELD			bit1
#define	GOESOUTPLAYFIELD			bit2
#define	WRAP						bit3



// ------------------------------------------------------
// EXTENSION EDITION HEADER STRUCTURE
// ------------------------------------------------------
struct extHeader_v1
{
	short extSize,
		  extMaxSize,
		  extOldFlags,		// For conversion purpose
		  extVersion;		// Version number
};
//typedef extHeader_v1*	extHeader *V1;

struct extHeader
{
	unsigned long extSize,
				  extMaxSize,
				  extVersion;			// Version number
	void *		  extID;				// object's identifier
	void *		  extPrivateData;		// private data
};
//typedef extHeader *		extHeader *;

// ------------------------------------------------------
// System objects (text, question, score, lives, counter)
// ------------------------------------------------------
struct rs {
	HeaderObject 	HeaderObject;				// For all the objects
	rCom			Common;				// Anims / movements / sprites structures
	rMvt			Movement;				// Mouvement structure
	Sprite			Sprite;				// Sprite handling
	
	short			Player;			// Number of the player if score or lives
	short			Flags;			// Type + flags

	union {
		unsigned char * SubAppli;			// Application (CCA object)
	};
	union
	{
		long		Mini;
		long		OldLevel;
	};
	union
	{
		long		Maxi;				// 
		long		Level;
	};
	CValue			Value;
	LONG			BoxCx;			// Dimensions box (for lives, counters, texts)
	LONG			BoxCy;			
	double			MiniDouble;		
	double			MaxiDouble;
	short			OldFrame;			// Counter only 
	unsigned char	Hidden;
	unsigned char	Free;
	TCHAR *			TextBuffer;		// Text buffer
	int				LBuffer;			// Length of the buffer in BYTES
	unsigned long	Font;				// Temporary font for texts
	union {
		COLORREF	TextColor;		// Text color
		COLORREF	Color1;			// Bar color
	};
	COLORREF		Color2;			// Gradient bar color
};
//typedef	rs *	LPRS;


// OILIST Structure : Data concerning the objects in the game
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define MAX_QUALIFIERS					8	// Maximum number of qualifiers per object


struct objInfoList {
	short			Oi,  			 // THE ObjectInfo number
					ListSelected,	 // First selection
					Type,			 // Type of the object
					Object;			 // First objects in the game
	unsigned int	oilEvents;		 // Events
	char			Wrap,			 // WRAP flags
					NextFlag;
	unsigned short	oilFree;		 // Not used
	int				NObjects,		 // Current number
					ActionCount,	 // Action loop counter
					ActionLoopCount; // Action loop counter
	
	// Current routine for the actions
	HeaderObject * (*CurrentRoutine)(objInfoList*, BOOL*);
	
	int				CurrentOi,		// Current object
					Next,			// Pointer on the next
					EventCount,		// When the event list is done
					NumOfSelected;	// Number of selected objects
	unsigned int	OEFlags;		// Object's flags
	short			LimitFlags,		// Movement limitation flags
					LimitList,      // Pointer to limitation list
					OIFlags,		// Objects preferences
					OCFlags2;		// Objects preferences II
	long			InkEffect,		// Ink effect
					EffectParam;	// Ink effect param
	short			HFII;			// First available frameitem
	COLORREF 		BackColor;		// Background erasing color
	short			Qualifiers[MAX_QUALIFIERS];		// Qualifiers for this object
	TCHAR			Name[24];	 	// Name	
	int				EventCountOR;	// Selection in a list of events with OR
	#ifdef HWABETA
		short *		lColList;		// Liste de collisions sprites
	#endif
};
//typedef	objInfoList	*	objInfoList *;

#define	OILIMITFLAGS_BORDERS		0x000F
#define	OILIMITFLAGS_BACKDROPS		bit5
#define	OILIMITFLAGS_ONCOLLIDE		bit8	// used by HWA
#define	OILIMITFLAGS_QUICKCOL		bit9
#define	OILIMITFLAGS_QUICKBACK		bit10
#define	OILIMITFLAGS_QUICKBORDER	bit11
#define	OILIMITFLAGS_QUICKSPR		bit12
#define	OILIMITFLAGS_QUICKEXT		bit13
#define	OILIMITFLAGS_ALL			0xFFFF

// Object creation structure
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct CreateObjectInfo {
	LevelObject *	cobLevObj;		// Leave first!
	unsigned short	cobLevObjSeg,
					cobFlags;
	int				cobX,
					cobY,
					cobDir,
					cobLayer,
					cobZOrder;
};
//typedef	CreateObjectInfo *	LPCOB;
//typedef	CreateObjectInfo *	fpcob;

// Flags for Create Objects
// -------------------------
#define	COF_NOMOVEMENT		bit1
#define	COF_HIDDEN			bit2
#define	COF_FIRSTTEXT		bit3

// Qualifier to oilist for machine langage
// ---------------------------------------

struct qualToOi {
	short		CurrentOi,
				Next;
	int			ActionPos;
	HeaderObject * (*CurrentRountine)(qualToOi*, BOOL* pRepeatFlag);
	int			ActionCount,
				ActionLoopCount;
	char		NextFlag,
				SelectedFlag;
	short		Oi,						// Array OINUM / OFFSET
				OiList;
};

// ------------------------------------------------------------------
// ------------------------------------------------------------------
// EXTENSION HANDLING
// ------------------------------------------------------------------
// ------------------------------------------------------------------

// Information structure about the extension
struct kpxRunInfos {
	void *			Conditions;			// 00 Ignore - requires STAND_ALONE - Offset to condition jump list
	void *			Actions;			// 04 Ignore - requires STAND_ALONE - Offset to action jump list
	void *			Expressions;		// 08 Ignore - requires STAND_ALONE - Offset to expression jump list
	short			NumOfConditions;	// 0C Number of conditions
	short			NumOfActions;		// 0E Number of actions
	short			NumOfExpressions;	// 10 Number of expressions
	unsigned short	EDITDATASize;		// 12 Size of the data zone when exploded
	unsigned int	EditFlags;			// 14 Object flags
	char			WindowProcPriority;	// 16 Priority of the routine 0-255
	char			Free;						
	short			EditPrefs;			// 18 Editing Preferences
	long			Identifier;			// 1A Identification string
	short			Version;			// 1E current version
										// 20
};

// Extension jump table
// --------------------

struct kpj {
	short				(__stdcall * CreateRunObject)			(HeaderObject *, extHeader *, CreateObjectInfo *);
	short				(__stdcall * DestroyRunObject)			(HeaderObject *, long);
	short				(__stdcall * HandleRunObject)			(HeaderObject *);
	short				(__stdcall * DisplayRunObject)			(HeaderObject *);
	cSurface *			(__stdcall * GetRunObjectSurface)		(HeaderObject *);
	short				(__stdcall * ReInitRunObject)			(HeaderObject *);
	short				(__stdcall * PauseRunObject)			(HeaderObject *);
	short				(__stdcall * ContinueRunObject)			(HeaderObject *);
	short				(__stdcall * PrepareToSave)				(HeaderObject *);
	short				(__stdcall * PrepareToSave2)			(HeaderObject *);
	short				(__stdcall * SaveBackground)			(HeaderObject *);
	short				(__stdcall * RestoreBackground)			(HeaderObject *);
	short				(__stdcall * ChangeRunData)				(HeaderObject *, HeaderObject *);
	short				(__stdcall * KillBackground)			(HeaderObject *);
	short				(__stdcall * GetZoneInfo)				(HeaderObject *);
	unsigned short *	(__stdcall * GetDebugTree)				(HeaderObject *);
	void 				(__stdcall * GetDebugItem)				(TCHAR *, HeaderObject *, int);
	void 				(__stdcall * EditDebugItem)				(HeaderObject *, int);
	void 				(__stdcall * GetRunObjectFont)			(HeaderObject *, LOGFONT *);
	void 				(__stdcall * SetRunObjectFont)			(HeaderObject *, LOGFONT *, RECT *);
	COLORREF 			(__stdcall * GetRunObjectTextColor)		(HeaderObject *);
	void				(__stdcall * SetRunObjectTextColor)		(HeaderObject *, COLORREF);
	short				(__stdcall * GetRunObjectWindow)		(HeaderObject *);
	sMask *				(__stdcall * GetRunObjectCollisionMask)	(HeaderObject *, LPARAM);
	BOOL				(__stdcall * SaveRunObject)				(HeaderObject *, HANDLE);
	BOOL				(__stdcall * LoadRunObject)				(HeaderObject *, HANDLE);
	void				(__stdcall * GetRunObjectMemoryUsage)	(HeaderObject *, int *, int *, int *);

	kpxRunInfos	infos;   
};

#define	KPJ_SHIFT				7
#define	KPJ_SIZE				0x80
#define KPJ_MAXNUMBER			128
#define	KPJ_STEP				8
#define	kpjJCreate				0x00
#define	kpjJDestroy				0x04
#define	kpjJRoutine				0x08
#define	kpjJDisplay				0x0C
#define	kpjJReInit				0x10
#define	kpjJPause				0x14
#define	kpjJEndPause			0x18
#define	kpjJPrepareToSave		0x1C
#define	kpjJRestart				0x20
#define	kpjJGetRunDataSize		0x24
#define	kpjJSaveBackground		0x28
#define	kpjJRestoreBackground	0x2C
#define	kpjJKillBackground		0x30

// Condition/action jumps
struct CallTables
{
	BOOL (** pConditions1)(event* pe, HeaderObject * pHo);
	BOOL (** pConditions2)(event* pe);
	void (** pActions)(event* pe);
	void (** pExpressions)();
};

#define	COMPARE_EQ				0
#define COMPARE_NE				1
#define	COMPARE_LE				2
#define	COMPARE_LT				3
#define	COMPARE_GE				4
#define	COMPARE_GT				5

#define OINUM_ERROR				(-1)

// Macro, converts 4 unsigned chars to one unsigned int
#define B2L(a,b,c,d)    ((unsigned int)(((unsigned int)((unsigned char)(d))<<24)|((unsigned int)((unsigned char)(c))<<16)|((unsigned int)((unsigned char)(b))<<8)|(unsigned int)((unsigned char)(a))))
	
// For GetFileInfos
fancyenum(FILEINFO) {
	DRIVE = 1,
	DIR,
	PATH,
	APP_NAME,
	TEMP_PATH
};}

// MACRO: next event
#define		EVTNEXT(p)		   		((LPEVT)((unsigned char * )p+p->evtSize))
// MACRO: returns first param
#define		EVTPARAMS(p) 			((LPEVP)(p->evtCode < 0 ? (unsigned char * )p+CND_SIZE : (unsigned char * )p+ACT_SIZE ))
// MACRO: returns the extension condition code
#define		EXTCONDITIONNUM(i)		(-((short)(i>>16))-1)           
#define		EXTACTIONNUM(i)			((short)(i>>16))

#define		KPXNAME_SIZE	60
#define		BADNAME_SIZE	(OINAME_SIZE+KPXNAME_SIZE+8)

// Extensions
// ----------


// Main loop exit codes
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define		LOOPEXIT_NEXTLEVEL		1
#define		LOOPEXIT_PREVLEVEL		2
#define		LOOPEXIT_GOTOLEVEL		3
#define		LOOPEXIT_NEWGAME		4
#define		LOOPEXIT_PAUSEGAME		5
#define		LOOPEXIT_SAVEAPPLICATION 6
#define		LOOPEXIT_LOADAPPLICATION 7
#define		LOOPEXIT_SAVEFRAME		8
#define		LOOPEXIT_LOADFRAME		9
#define		LOOPEXIT_ENDGAME		-2

#define		LOOPEXIT_QUIT			100
#define		LOOPEXIT_RESTART		101
#define		LOOPEXIT_TIMER			102
#define		LOOPEXIT_STEP			103
#define		LOOPEXIT_MONITOR		104
#define		LOOPEXIT_EVENTEDITOR	105
#define		LOOPEXIT_ESC			106
#define		LOOPEXIT_MENU			107
#define		LOOPEXIT_JUMPTO			108
#define		LOOPEXIT_RESET			109
#define		LOOPEXIT_PAUSE			110
#define		LOOPEXIT_PLAY			111

// Backdrop draw routines
// ----------------------------------------
typedef void (* DRAWROUTINE_PROC)(drawRoutine*);

struct drawRoutine {
	drawRoutine* next;
	DRAWROUTINE_PROC routine;
	long	param1;
	long	param2;
	long	param3;
	long	param4;
	long	param5;
	long	param6;
};

// To push an event
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define		MAX_PUSHEDEVENTS	32

struct pev {
	long	pevCode;
	void (* pevRoutine)(HeaderObject *, long);
	long	pevParam;
	HeaderObject *	pevObject;
	short	pevOi;
};
#define	PIXEL_SPEED					5

// QUALIFIERS
struct qualifierLoad
{
	OINUM		qOi;
	ITEMTYPE	qType;
	unsigned short		qList;
};
//typedef qualifierLoad *	LPQLOAD;

// start of alignment
// last realignment

#ifndef      _H2INC

// Object instance parent types
enum {
	PARENT_NONE,
	PARENT_FRAME,
	PARENT_FRAMEITEM,
	PARENT_QUALIFIER
};

// Proc definition
typedef void (__stdcall * UPDATEFILENAMEPROCA)(const char *, char *);
typedef void (__stdcall * UPDATEFILENAMEPROCW)(const wchar_t *, wchar_t *);
typedef void (__stdcall * UPDATEFILENAMEPROC)(const TCHAR *, TCHAR *);

// Old object name size, still used in events
#ifndef OINAME_SIZE
	#define	OINAME_SIZE			24
#endif	// OINAME_SIZE

// Obsolete
fancyenum(MODIF) {
	SIZE,
	PLANE,
	FLAGS,
	PICTURE,
	HFRAN,
	BOX,
	TEXT,
	PREFS
};}
#define	KPX_MODIF_BASE	1024

#endif	// _H2INC

// Standard objects
#define	KPX_BASE	32		// Extensions

// Menu header (v2)
struct MenuHdr {

	unsigned long	HdrSize,		// == sizeof(MenuHdr)
					MenuOffset,		// From start of MenuHdr
					MenuSize,
					AccelOffset,	// From start of MenuHdr
					AccelSize;		// Total data size = mhHdrSize + mhMenuSize + mhAccelSize 
};

//////////////////////////////////////////////////////////////////////////////
// Application mini Header
//
struct AppMiniHeader {

	union {
		char			Type[4];	// "PAME"
		unsigned long	FullType;
	};
	unsigned short		Version;	// Version number
	unsigned short		SubVersion;	// Sub-version number
	unsigned int		PrdVersion;	// MMF version
	unsigned int		PrdBuild;	// MMF build number

};

#ifdef _UNICODE
	#define	RUNTIME_DWTYPE				'UMAP'
	#define RUNTIME_DWTYPE_VTZ			'UZTV'
#else
	#define	RUNTIME_DWTYPE				'EMAP'
	#define RUNTIME_DWTYPE_VTZ			' ZTV'
#endif
#define	RUNTIME_VERSION_MMF1		0x0300
#define	RUNTIME_VERSION_MMF15		0x0301
#define	RUNTIME_VERSION_MMF2		0x0302
#define	RUNTIME_VERSION				RUNTIME_VERSION_MMF2

//////////////////////////////////////////////////////////////////////////////
// Application full Header
//

enum {
	CTRLTYPE_MOUSE,		// not used
	CTRLTYPE_JOY1,
	CTRLTYPE_JOY2,
	CTRLTYPE_JOY3,
	CTRLTYPE_JOY4,
	CTRLTYPE_KEYBOARD,
	MAX_CTRLTYPE
};

#define PLAYERNAME_SIZE		100
struct PlayerCtrls {
	unsigned short	PlayerCtrls_Type,		// Control type per player (0 = keyboard, 1-4 = joy1-4)
					PlayerCtrls_Keys[8];	// Control keys per player
};
//typedef PlayerCtrls* fpPlayerCtrls;

struct AppHeader {
	unsigned int	Size;					// Structure size
	unsigned short	Flags,					// Flags
					NewFlags,				// New flags
					Mode,					// Graphic mode
					OtherFlags;				// Other Flags
	short			XWinSize,				// Window x-size
					YWinSize;				// Window y-size
	unsigned int	ScoreInit,				// Initial score
					LivesInit;				// Initial number of lives
	PlayerCtrls		PlayerCtrls[4];			// Player controls
	unsigned int	BorderColour,			// Border colour
					NbFrames,				// Number of frames
					FrameRate;				// Number of frames per second
	unsigned char	MDIWindowMenu,			// Index of Window menu for MDI applications
					Free[3];				// Padding to a multiple of 4 bytes
};

// gaFlags
#define	GA_BORDERMAX				bit1
#define	GA_NOHEADING				bit2
#define	GA_PANIC					bit3
#define	GA_SPEEDINDEPENDANT			bit4
#define	GA_STRETCH					bit5
#define	GA_LOADALLIMAGESATSTART		bit6
#define	GA_LOADALLSOUNDSATSTART		bit7
#define	GA_MENUHIDDEN				bit8
#define	GA_MENUBAR					bit9
#define	GA_MAXIMISE					bit10
#define	GA_MIX						bit11
#define	GA_FULLSCREENATSTART		bit12
#define	GA_FULLSCREENSWITCH			bit13
#define	GA_PROTECTED				bit14
#define	GA_COPYRIGHT				bit15
#define	GA_ONEFILE					bit16
#define	GANF_SAMPLESOVERFRAMES		bit1
#define	GANF_RELOCFILES				bit2
#define	GANF_RUNFRAME				bit3
#define	GANF_SAMPLESEVENIFNOTFOCUS	bit4
#define	GANF_NOMINIMIZEBOX			bit5
#define	GANF_NOMAXIMIZEBOX			bit6
#define	GANF_NOTHICKFRAME			bit7
#define	GANF_DONOTCENTERFRAME		bit8
#define	GANF_SCREENSAVER_NOAUTOSTOP	bit9
#define	GANF_DISABLE_CLOSE			bit10
#define	GANF_HIDDENATSTART			bit11
#define	GANF_XPVISUALTHEMESUPPORT	bit12
#define	GANF_VSYNC					bit13
#define	GANF_RUNWHENMINIMIZED		bit14
#define	GANF_MDI					bit15
#define	GANF_RUNWHILERESIZING		bit16
#define	GAOF_DEBUGGERSHORTCUTS		bit1
#define	GAOF_DDRAW					bit2
#define	GAOF_DDRAWVRAM				bit3
#define	GAOF_OBSOLETE				bit4
#define	GAOF_AUTOIMGFLT				bit5
#define	GAOF_AUTOSNDFLT				bit6
#define	GAOF_ALLINONE				bit7
#define	GAOF_SHOWDEBUGGER			bit8
#define	GAOF_RESERVED_1				bit9
#define	GAOF_RESERVED_2				bit10
#define	GAOF_RESERVED_3				bit11
#define	GAOF_RESERVED_4				bit12
#define	GAOF_JAVASWING				bit13
#define	GAOF_JAVAAPPLET				bit14
#define	GAOF_D3D9					bit15
#define	GAOF_D3D8					bit16

// Optional header
struct AppHeader2 {

	unsigned long	Options,
					BuildType,
					BuildFlags;
	unsigned short	ScreenRatioTolerance,
					ScreenAngle;			// 0 (no rotation/portrait), 1 (90 clockwise/landscape left), 2 (90 anticlockwise/landscape right), 3 (automatic portrait), 4 (automatic landscape), 5 (fully automatic)
	unsigned long	Unused2;

};

fancyenum(AH2OPT) {
	KEEPSCREENRATIO = bit1,
	FRAMETRANSITION = bit2,		// (HWA only) a frame has a transition 
	RESAMPLESTRETCH = bit3,		// (HWA only) "resample when resizing" (works with "resize to fill window" option)
	GLOBALREFRESH = bit4,		// (Mobile) force global refresh
	MULTITASK = bit5,			// (iPhone) Multitask
	RTL = bit6,					// (Unicode) Right-to-left reading
	STATUSLINE = bit7,			// (iPhone/Android) Display status line
	RTLLAYOUT = bit8,			// (Unicode) Right-to-left layout
	ENABLEIAD = bit9,			// (iPhone) Enable iAd
	IADBOTTOM = bit10,			// (iPhone) Display ad at bottom
	AUTOEND = bit11,			// (Android)
	DISABLEBACKBUTTON = bit12,	// (Android) Disable Back button behavior
	ANTIALIASED = bit13,		// (iPhone) Smooth resizing on bigger screens
	CRASHREPORTING = bit14,		// (Android) Enable online crash reporting
};}

fancyenum(SCREENORIENTATION) {
	PORTRAIT,
	LANDSCAPE_LEFT,
	LANDSCAPE_RIGHT,
	AUTO,
	LANDSCAPE_AUTO,
	PORTRAIT_AUTO,
};}

#ifndef      _H2INC

// Build type values
fancyenum(BUILDTYPE) {
	STANDALONE,
	SCREENSAVER,
	INTERNETAPP,
	JAVA,
	JAVASTANDALONE,
	JAVAAPPLET,
	JAVAWEBSTART,
	JAVAMOBILE,
	JAVABLURAY,
	JAVAMAC,
	FLASH,
	JAVABLACKBERRY,
	ANDROID,
	IPHONE,
	IPHONEDEVEL,
	IPHONEFINAL,
	XNA_WINDOWS_APP,
	MAC,
	XNA_WINDOWS,
	XNA_XBOX,
	XNA_PHONE,
	XNA_XBOX_APP,
	XNA_PHONE_APP,
	STDMAX,			// end of standard build types
};}

// Build flag values
#define	BUILDFLAG_MAXCOMP			bit1
#define BUILDFLAG_COMPSND			bit2
#define BUILDFLAG_INCLUDEEXTFILES	bit3
#define BUILDFLAG_MANUALIMGFILTERS	bit4
#define BUILDFLAG_MANUALSNDFILTERS	bit5
#define	BUILDFLAG_NOAUTOEXTRACT		bit6
#define	BUILDFLAG_NOAPPLETCHECK		bit7
#define	BUILDFLAG_TEST				bit8
#define	BUILDFLAG_NOWARNINGS		bit9

#endif // _H2INC

//////////////////////////////////////////////////////////////////////////////
// Element of chunk Extensions
//
#ifndef      _H2INC
struct ExtDesc {
	unsigned short	extSize,
					extIndex;
	unsigned int	extMagicNumber;
};
// typedef ExtDesc *extHeader *DESC;

struct ExtDesc2 {
	unsigned short	extSize,
					extIndex;
	unsigned long	extMagicNumber,
					extVersionLS,
					extVersionMS;
};
//typedef ExtDesc2 *extHeader *DESC2;
#endif // _H2INC

//////////////////////////////////////////////////////////////////////////////
// Movement Extensions Chunk
//
struct MvtExtDesc {
	unsigned short	extTotalSize,
					extHdrSize;
	unsigned int	extBuild;
};
//typedef MvtExtDesc *LPMVTEXTDESC;

//////////////////////////////////////////////////////////////////////////////
// Frame Header
//
struct FrameHeader
{
	// Playfield
	LONG			Width,			// Playfield width in pixels
					Height;			// Playfield height in pixels
	COLORREF		Background;

	// Options
	unsigned int	Flags;


};

// leFlags
#define	LDISPLAYNAME		bit1
#define	LGRABDESKTOP		bit2
#define	LKEEPDISPLAY		bit3
// #define LFADEIN			bit4
// #define LFADEOUT			bit5
#define	LTOTALCOLMASK		bit6
#define	LPASSWORD			bit7
#define	LRESIZEATSTART		bit9
#define	LDONOTCENTER		bit10
#define	LFORCE_LOADONCALL	bit11
#define	LNOSURFACE			bit12
#define	LRESERVED_1			bit13
#define	LRESERVED_2			bit14
#define	LRECORDDEMO			bit15
#define	LTIMEDMVTS			bit16

//////////////////////////////////////////////////////////////////////////////
// Layers
//

#define FLOPT_XCOEF				bit1
#define FLOPT_YCOEF				bit2
#define FLOPT_NOSAVEBKD			bit3
#define FLOPT_WRAP_OBSOLETE		bit4
#define FLOPT_VISIBLE			bit5
#define FLOPT_WRAP_HORZ			bit6
#define FLOPT_WRAP_VERT			bit7
#define FLOPT_PREVIOUSEFFECT	bit8
#define FLOPT_REDRAW			0x010000
#define FLOPT_TOHIDE			0x020000
#define FLOPT_TOSHOW			0x040000

struct EditFrameLayer
{
	unsigned long	Options;		// Options
	float			xCoef, yCoef;	// X/Y Coefficents
	unsigned int	nBkdLOs,
					nFirstLOIndex;

};

struct EditFrameLayerEffect {

	unsigned long	InkFx,
					RGBA,
					ExtInkFxIdx,
					NumOfParams;
	LPARAM			paramData;		// offset

};

// Effects
#ifdef HWABETA

struct EffectHdr {
	unsigned long	EffectNameOffset,
					EffectDataOffset,
					EffectParamsOffset,
					Options;
};

#define EFFECTOPT_BKDTEXTUREMASK	0x000F

struct EffectParamsHdr {
	unsigned long	NumOfParams,
					ParamTypesOffset,
					ParamNamesOffset;
};

struct EffectRunData {

	unsigned int	EffectIndex,
					NumOfParams;
};

struct FrameEffect {

	unsigned int	InkEffect,
					InkEffectParam;
};

#endif // HWABETA

//////////////////////////////////////////////////////////////////////////////
// ObjInfo/FrameItem Header
//
struct ObjInfoHeader
{
	unsigned short	Handle,
					Type,
					Flags,			// Memory flags
					Reserved;		// Not used
	unsigned int	InkEffect,		// Ink effect
					InkEffectParam;	// Ink effect param
};

// oiFlags
fancyenum(OIFlags)
{
	LOAD_ON_CALL = bit1,
	DISCARDABLE = bit2,
	GLOBAL = bit3,
	RESERVED_1 = bit4,
	GLOBAL_EDITOR_NO_SYNC = bit5,
	GLOBAL_EDITOR_FORCE_SYNC = bit6,
};}

//////////////////////////////////////////////////////////////////////////////
// LevObj/FrameItemInstance
//

#ifndef      _H2INC
struct diskLO {
	unsigned short	LO_Handle;			// HLO
	unsigned short	OI_Handle;			// HOI
	LONG			X, Y;				// Coords
	unsigned short	ParentType;			// Parent type
	unsigned short	OI_ParentHandle;	// HOI Parent
	unsigned short	Layer;				// Layer
	unsigned short	Free;				// Not used
};
// typedef diskLO * fpDiskLO;
#endif  // _H2INC

//////////////////////////////////////////////////////////////////////////////
//
// ObjInfo/FrameItem ObjectsCommon
//

// Obstacle types
fancyenum(OBSTACLE) {
	NONE,
	SOLID,
	PLATFORM,
	LADDER,
	TRANSPARENT,		// for Add Backdrop
};}

////////////////////////////////
// Static object - ObjectsCommon
//
#ifndef      _H2INC
struct Static_OC {

	// Size
	unsigned int	Size;				// OC size?

	// Obstacle type & collision mode
	unsigned short	ObstacleType;		// Obstacle type
	unsigned short	ColMode;			// Collision mode (0 = fine, 1 = box)

	LONG	X, Y;						// Size

};
//typedef Static_OC * LPStatic_OC;
#endif  // _H2INC

///////////////////////////////////////////////////////////////
// Fill Type & shapes - Definitions
//

#ifndef      _H2INC
// Gradient
typedef struct GradientData {
	COLORREF		color1,
					color2;
	unsigned int	GradientFlags; // prev. vertical
} GradientData;

// Shapes
fancyenum(SHAPE) {
	NONE,			// Error'd
	LINE,
	RECTANGLE,
	ELLIPSE
};}

// Fill types
fancyenum(FILLTYPE) {
	NONE,
	SOLID,
	GRADIENT,
	MOTIF
};}

// Line flags
#define	LININVX	bit1
#define	LININVY	bit2

#endif // _H2INC

///////////////////////////////////////////////////////////////
// Fill Type - Part of FilledShape
//
#ifndef      _H2INC

class FillType_Data {
public:
	unsigned short			FillType;

	// Fill type
	union {
		// Lines in non-filled mode
		struct {
			unsigned int LineFlags;		// Only for lines in non filled mode
		};
		
		// Solid colour
		struct {
			COLORREF Color;			// Solid
		};
		
		// Gradient
		struct {
			COLORREF	 Color1,		
						 Color2;
			unsigned int GradientFlags;
		};

		// Image
		struct {
			unsigned short Image;			// Image
		};
	};
};

#endif // _H2INC

///////////////////////////////////////////////////////////////
// Filled Shape - Part of QuickBackdrop / Counter ObjectsCommon
//
#ifndef      _H2INC

class FilledShape_Data {
public:
	unsigned short	BorderSize;			// Border
	COLORREF		BorderColor;
	unsigned short	Shape;				// Shape
	FillType_Data	FillData;			// Fill Type infos
};
// typedef FilledShape_Data * LPFilledShape_Data;

#endif // _H2INC

/////////////////////////////////
// Quick backdrop - ObjectsCommon
//
#ifndef      _H2INC

typedef struct QuickBackdrop_OC {

	unsigned int		Size;

	unsigned short		ObstacleType;	// Obstacle type (0
	unsigned short		ColMode;		// Collision mode (0 = fine, 1 = box)

	LONG				X, Y;			// Size

	FilledShape_Data	FilledShape;	// Filled shape infos

} QuickBackdrop_OC;
typedef QuickBackdrop_OC * LPQuickBackdrop_OC;

#endif // _H2INC

/////////////////////////////////
// Backdrop - ObjectsCommon
//
#ifndef      _H2INC

struct Backdrop_OC {

	unsigned int	Size;

	unsigned short	ObstacleType;	// Obstacle type (0
	unsigned short	ColMode;		// Collision mode (0 = fine, 1 = box)

	LONG			X, Y;			// Size

	unsigned short	Image;			// Image

};
typedef Backdrop_OC * LPBackdrop_OC;

#endif // _H2INC

//////////////////////////////////////////////////////////////////////////////
//
// ObjInfo/FrameItem ObjectsCommon Data
//

//////////////////////////////////////////////////////////////////////////////
// ImageSet
//

class ImageSet_Data {
public:
	unsigned short		nbFrames;		// Number of frames
								// Followed by list of image handles (unsigned short[])
};
typedef ImageSet_Data * LPImageSet_Data;

////////////////////////////////////////
// Text - ocData
//
struct otText {

	unsigned int	otDWSize;
	LONG	otCx;
	LONG	otCy;
	unsigned int	otNumberOfText;			// Paragraph number (> 1 if question object)
	unsigned int	otStringOffset[1];		// String offsets

};
typedef otText	*	fpot;
#define sizeof_ot	(sizeof(otText)-sizeof(unsigned int))

typedef	struct	txString {
	unsigned short		tsFont;					// Font
	unsigned short		tsFlags;				// Flags
	COLORREF	tsColor;				// Color
	TCHAR		tsChar[1];
} txString;
typedef	txString	*	fpts;
#define	sizeof_ts	8					// (sizeof(txString)-1)

#define	TSF_LEFT		0x0000			// DT_LEFT
#define	TSF_HCENTER		bit1			// DT_CENTER
#define	TSF_RIGHT		bit2			// DT_RIGHT
#define	TSF_VCENTER		bit3			// DT_VCENTER
#define	TSF_HALIGN		0x000F			// DT_LEFT | DT_RIGHT | DT_CENTER | DT_VCENTER | DT_BOTTOM

#define	TSF_CORRECT		bit9
#define	TSF_RELIEF		bit10
#define TSF_RTL			bit11

////////////////////////////////////////
// Scores, lives, counters
//
struct CtAnim_Data {

	unsigned int		odDWSize;
	LONG		odCx;					// Size: only lives & counters
	LONG		odCy;
	unsigned short		odPlayer;				// Player: only score & lives
	unsigned short		odDisplayType;			// CTA_xxx
	unsigned short		odDisplayFlags;			// BARFLAG_INVERSE
	unsigned short		odFont;					// Font

	// Followed by display data:
//	union {
//		ImageSet_Data		odImages;
//		FilledShape_Data	odShape;
//	};

};
//typedef CtAnim_Data * LPCtAnim_Data;

// Display types
fancyenum(CTA) {
	HIDDEN,
	DIGITS,
	VBAR,
	HBAR,
	ANIM,
	TEXT,
};}

// Display flags
#define	CPTDISPFLAG_INTNDIGITS				0x000F		// 0 = normal display, other value = pad with zeros or truncate
#define	CPTDISPFLAG_FLOATNDIGITS			0x00F0		// add 1 to get the number of significant digits to display
#define	CPTDISPFLAG_FLOATNDIGITS_SHIFT		4
#define	CPTDISPFLAG_FLOATNDECIMALS			0xF000		// number of digits to display after the decimal point
#define	CPTDISPFLAG_FLOATNDECIMALS_SHIFT	12
#define	BARFLAG_INVERSE						bit9
#define	CPTDISPFLAG_FLOAT_FORMAT			bit10		// 1 to use the specified numbers of digits, 0 to use standard display (%g)
#define	CPTDISPFLAG_FLOAT_USENDECIMALS		bit11		// 1 to use the specified numbers of digits after the decimal point
#define	CPTDISPFLAG_FLOAT_PADD				bit12		// 1 to left padd with zeros

// Counters images (0-9 for regular numbers)
fancyenum(COUNTER_IMAGE) {
	SIGN_NEG = 10,
	SIGN_PLUS,
	POINT,
	EXP,
	MAX
};}
#define	V1_COUNTER_IMAGE_MAX	(COUNTER_IMAGE_SIGN_NEG+1)

////////////////////////////////////////
//
// Objet RTF - ocData
//
#ifndef      _H2INC

struct ocRTF {
	unsigned int	Size;
	unsigned int	Version;	// 0
	unsigned int	Options;	// Options
	COLORREF		BackColor;	// Background color	
	LONG			XSize;		// Size
	LONG			YSize;
};
//typedef ocRTF * LPOCRTF;

//#define	RTFOPT_TRANSPARENT	bit1		// Transparent
//#define	RTFOPT_VSLIDER		bit2		// Display vertical slider if necessary
//#define	RTFOPT_HSLIDER		bit3		// Display horizontal slider if necessary

#endif // _H2INC

////////////////////////////////////////
//
// Objet CCA - ocData
//
#ifndef      _H2INC

struct ocCCA {

	unsigned int	Size;
	LONG			XSize,			// Size (ignored)
					YSize;
	unsigned short	Version,		// 0
					NumStartFrame;
	unsigned int	Options,		// Options
					IconOffset,		// Icon offset
					Free;			// Ignore - reserved

};
//typedef ocCCA * LPOCCCA;

// Options
#define	CCAF_SHARE_GLOBALVALUES		bit1
#define	CCAF_SHARE_LIVES			bit2
#define	CCAF_SHARE_SCORES			bit3
#define	CCAF_SHARE_WINATTRIB		bit4
#define	CCAF_STRETCH				bit5
#define	CCAF_POPUP					bit6
#define CCAF_CAPTION				bit7
#define CCAF_TOOLCAPTION			bit8
#define CCAF_BORDER					bit9
#define CCAF_WINRESIZE				bit10
#define CCAF_SYSMENU				bit11
#define CCAF_DISABLECLOSE			bit12
#define CCAF_MODAL					bit13
#define CCAF_DIALOGFRAME			bit14
#define	CCAF_INTERNAL				bit15
#define	CCAF_HIDEONCLOSE			bit16
#define	CCAF_CUSTOMSIZE				0x00010000
#define	CCAF_INTERNALABOUTBOX		0x00020000
#define	CCAF_CLIPSIBLINGS			0x00040000
#define	CCAF_SHARE_PLAYERCTRLS		0x00080000
#define CCAF_MDICHILD				0x00100000
#define CCAF_DOCKED					0x00200000
#define	CCAF_DOCKING_AREA			0x00C00000
#define	CCAF_DOCKED_LEFT			0x00000000
#define	CCAF_DOCKED_TOP				0x00400000
#define	CCAF_DOCKED_RIGHT			0x00800000
#define	CCAF_DOCKED_BOTTOM			0x00C00000
#define	CCAF_REOPEN					0x01000000
#define CCAF_MDIRUNEVENIFNOTACTIVE	0x02000000
#define CCAF_DISPLAYASSPRITE		0x04000000

#endif // _H2INC

////////////////////////////////////////
//
// Transition
//

#ifndef      _H2INC

// Transition header
typedef struct TransitionHdr {
	unsigned int		trDllID;				// DLL id
	unsigned int		trID;					// Transition ID
	unsigned int		trDuration;				// Duration
	unsigned int		trFlags;				// From / to color / background
	unsigned int		trColor;				// Color
} TransitionHdr;

// Transition run-time data
class Transition_Data {
public:
	TransitionHdr	trHdr;
	unsigned int			trDllNameOffset;
	unsigned int			trParamsOffset;
	unsigned int			trParamsSize;
									// Followed by DLL name & transition parameters
};
typedef Transition_Data * LPTRANSITIONDATA;

#define	TRFLAG_COLOR	bit1
#define TRFLAG_UNICODE	bit2

#endif // _H2INC

// Text alignment flags
#define	TEXT_ALIGN_LEFT		bit1
#define	TEXT_ALIGN_HCENTER	bit2
#define	TEXT_ALIGN_RIGHT	bit3
#define	TEXT_ALIGN_TOP		bit4
#define	TEXT_ALIGN_VCENTER	bit5
#define	TEXT_ALIGN_BOTTOM	bit6

// Right-to-left ordering
#define	TEXT_RTL			bit9

// Text caps
#define	TEXT_FONT			0x00010000
#define	TEXT_COLOR			0x00020000
#define	TEXT_COLOR16		0x00040000

//////////////////////////////////////////////////////////////////////////////
//
//	Run-time Definitions
//

#ifdef	RUN_TIME

// Versions
#define	KNPV1_VERSION			0x126		// First K&P version
#define	CNCV1_VERSION			0x207		// Click & Create / The Games Factory
#define	KNP_VERSION				0x300

// Internet Versions
#define	FIRST_NETVERSION		bit1		// First vitalize version
#define	CCN_NETVERSION			bit2		// CCN applications
#define	MORECOMP_NETVERSION		0x003		// Short OIs, compression of levObjs, adpcm sounds, 
#define	CNC_NETVERSION			0x003		// Current Internet Version

// Collision mask margins
#define		COLMASK_XMARGIN			64
#define		COLMASK_YMARGIN			16

// Messages
#define		UM_KNP_GETAPPID		(WM_APP+7)	// No longer used
#define		UM_HVSCROLL			(WM_APP+8)	// No longer used

// App IDs
#define		APPID_KNC		0x3952			// No longer used
#define		APPID_KNCSA		0x9172			// No longer used

// Control IDs
#define		ID_EDIT_WIN		300				// ID of edit window

#endif	// RUN_TIME

// HLO
//#define	HLO			unsigned short
#define	HLO_ERROR	(-1)
//typedef	HLO *		LPHLO;

// Frame password size
#define	PASSWORD_SIZE			9

// Cross-ref tables
enum {
	IMG_TAB,
	FONT_TAB,
	SOUND_TAB,
	MUSIC_TAB,
	MAX_TABREF
};

#ifdef RUN_TIME

// Preferences
#define	PRMUSICON		bit15
#define	PRSOUNDON		bit16

// Frame handles
#define	HCELL			unsigned short
#define	HCELL_ERROR		((unsigned short)-1)

// Jump flags
#define	MPJC_NOJUMP		0
#define	MPJC_DIAGO		1
#define	MPJC_BUTTON1	2
#define	MPJC_BUTTON2	3

// How to play (no longer used)
enum	{
	HTP_NO=0,
	HTP_TXT,
	HTP_WRI,
	HTP_HLP
};
#define	NB_HTPTYPE	4

#endif	// RUN_TIME

// Versions
#define MMFVERSION_MASK		0xFFFF0000
#define MMFBUILD_MASK		0x00000FFF		// MMF build
#define MMFVERFLAG_MASK		0x0000F000
#define MMFVERFLAG_HOME		bit16		// TGF
#define MMFVERFLAG_PRO		bit15		// MMF Pro
#define MMFVERFLAG_DEMO		bit14		// Demo
#define MMFVERFLAG_PLUGIN	bit13		// Plugin
#define MMFVERSION_15		0x01050000		// MMF 1.5
#define MMFVERSION_20		0x02000000		// MMF 2.0
#define	MMF_CURRENTVERSION	MMFVERSION_20

// Build numbers
fancyenum(MMF_BUILD) {
	NONAME = 203,
	MMF_BUILD_MENUIMAGES,
	MMF_BUILD_SUBAPPICON,
	MMF_BUILD_WINMENUIDX,
	MMF_BUILD_NOTUSED,
	MMF_BUILD_ABOUTTEXT,
	FRAMEDPOS,
	TRSPCOLOR,
	BLD211,
	DEMO,
	PREBETA_2,
	PREBETA_3,
	PREBETA_4,	// First update of Jason's book
	ALTVALUES,
	EDTCHUNKS,
	COUNTERTEXT,
	BLD_SDK,		// Pre-SDK version,
	BLD_SDK2,		// Second SDK version,
	MVTSDK2,		// Second Mvt SDK version
	FIXCOLLISANDBEH,
	PROPFILTERS,
	MFXEXTS,
	LASTFRAMEOFFSET,
	EDUDEMOFEB06,
	GLBOBJBUG,
	IDTC,
	BLD229,
	BLD230,
	BLD231,
	FIXQUALIF,
	BLD233,
	BLD234_TGFPR,
	BLD235,
	BLD236,
	BLD237,
	BLD238,
	BLD239_CD,
	BLD240_DEMO,
	BLD241_SP1,
	BLD242_SP2,
	BLD243_SP3,
	BLD244_SP4,
	BLD245_SP5,
	BLD246_SP6,
	BLD247_SP7,
	BLD248_JAVA,
	BLD249_MOBILE_FLASH,
	BLD250,
	BLD251,
	BLD252,
	BLD253,
	BLD254,
};}
#define MMF_CURRENTBUILD 255

// MFA file format versions
#define MFA_BUILD_ALTSTR			1						// Alterable strings
#define MFA_BUILD_COUNTERTEXT		2						// Text mode in counters
#define MFA_BUILD_LASTFRAMEOFFSET	3						// Additional frame offset
#define MFA_BUILD_FIXQUALIF			4						// Fix in qualifiers + prd version
#ifdef _UNICODE
	#define MFA_BUILD_LANGID		5						// Language ID
	#define MFA_CURRENTBUILD		MFA_BUILD_LANGID
#else
	#define MFA_CURRENTBUILD		MFA_BUILD_FIXQUALIF
#endif

// Structures for picture editor
struct EditSurfaceParams {
	unsigned int	Size;			// sizeof(EditSurfaceParams)
	TCHAR *			WindowTitle;	// Picture Editor title (NULL = default title)
	cSurface *		Sf;				// Surface to edit
	unsigned int	Options;		// Options, see PictEdDefs.h
	unsigned int	FixedWidth;		// Default width or fixed width (if PICTEDOPT_FIXEDIMGSIZE is used)
	unsigned int	FixedHeight;	// Default height or fixed height (if PICTEDOPT_FIXEDIMGSIZE is used)
	POINT			hotSpot;		// Hot spot coordinates
	POINT			actionPoint;	// Action point coordinates
};
//typedef EditSurfaceParamsA* LPEDITSURFACEPARAMSA;
//typedef EditSurfaceParamsW* LPEDITSURFACEPARAMSW;

struct EditImageParams {

	unsigned int	Size;			// sizeof(EditImageParams)
	TCHAR *			WindowTitle;	// Picture Editor title (NULL = default title)
	unsigned int	Image,			// Image to edit - note: only the LOWORD is used in this version
					Options,		// Options, see PictEdDefs.h
					FixedWidth,		// Default width or fixed width (if PICTEDOPT_FIXEDIMGSIZE is used)
					FixedHeight;	// Default height or fixed height (if PICTEDOPT_FIXEDIMGSIZE is used)

};
//typedef EditImageParamsA* LPEDITIMAGEPARAMSA;
//typedef EditImageParamsW* LPEDITIMAGEPARAMSW;

// Structure for image list editor
struct EditAnimationParams {

	unsigned int	Size;			// sizeof(EditAnimationParams)
	TCHAR *			WindowTitle;	// Picture Editor title (NULL = default title)
	int				NumberOfImages,	// Number of images in the list
					MaxImages,		// Maximum number of images in the list
					StartIndex;		// Index of first image to edit in the editor
	unsigned int *	Images;			// Image list (one unsigned short per image)
	TCHAR **		ImageTitles;	// Image titles (can be NULL)
	unsigned int	Options,		// Options, see PictEdDefs.h
					FixedWidth,		// Default width or fixed width (if PICTEDOPT_FIXEDIMGSIZE is used)
					FixedHeight;	// Default height or fixed height (if PICTEDOPT_FIXEDIMGSIZE is used)

};


// Global variables structure
struct mv {

	// Common to editor and runtime
	HINSTANCE			HInst;				// Application HINSTANCE
	void *				IdAppli;			// Application object in DLL
	void *				IdMainWin;			// Main window object in DLL
	void *				IdEditWin;			// Child window object in DLL
	HWND				HMainWin,			// Main window handle
						HEditWin;			// Child window handle
	HPALETTE			HPal256;			// 256 color palette
	unsigned short		AppMode,			// Screen mode with flags
						ScrMode;			// Screen mode
	unsigned int		EditDXDocToClient,	// Edit time only: top-left coordinates
						EditDYDocToClient;
	CImageFilterMgr *	ImgFilterMgr;		// Image filter manager
	CSoundFilterMgr *	SndFilterMgr;		// Sound filter manager
	CSoundManager *		SndMgr;				// Sound manager

	union {
		CEditApp *		EditApp;			// Current application, edit time (not used)
		CRunApp *		RunApp;				// Current application, runtime
	};
	union {
		CEditFrame *	EditFrame;
		CRunFrame *		RunFrame;
	};

	// Runtime
	RunHeader *			RunHdr;				// Current RunHeader
	unsigned int		Prefs;				// Preferences (sound on/off)
	TCHAR *				subType;
	BOOL				FullScreen;			// Full screen mode
	TCHAR *				MainAppFileName;	// App filename
	int					AppListCount;
	int					AppListSize;
	CRunApp**			AppList;
	int					ExtListCount;
	int					ExtListSize;
	TCHAR **			ExtList;
	int					NbDllTrans;
	dllTrans*			DllTransList;
	unsigned int		JoyCaps[32];
	HHOOK				HMsgHook;
	int					ModalLoop;
	int					ModalSubAppCount;
	void *				Free[5];

	// Functions
	////////////

	// Editor: Open Help file
	void (CALLBACK * HelpA) (const char * pHelpFile, unsigned int nID, LPARAM lParam);

	// Editor: Get default font for object creation
	BOOL (CALLBACK * GetDefaultFontA) (LOGFONTA * plf, char * pStyle, int cbSize);

	// Editor: Edit images and animations
	BOOL (CALLBACK * EditSurfaceA) (void * edPtr, EditImageParams pParams, HWND hParent);
	BOOL (CALLBACK * EditImageA) (void * edPtr, EditImageParams pParams, HWND hParent);
	BOOL (CALLBACK * EditAnimationA) (void * edPtr, EditAnimationParams * pParams, HWND hParent);

	// Runtime: Extension User data
	void * (CALLBACK * GetExtUserData) (CRunApp* pApp, HINSTANCE hInst);
	void * (CALLBACK * SetExtUserData) (CRunApp* pApp, HINSTANCE hInst, void * pData);

	// Runtime: Register dialog box
	void (CALLBACK * RegisterDialogBox) (HWND hDlg);
	void (CALLBACK * UnregisterDialogBox) (HWND hDlg);

	// Runtime: Add surface as backdrop object
	void (CALLBACK * AddBackdrop) (cSurface * pSf, int x, int y, unsigned int dwInkEffect, unsigned int dwInkEffectParam, int nObstacleType, int nLayer);

	// Runtime: Binary files
	BOOL (CALLBACK * GetFileA)(const char * pPath, char * pFilePath, unsigned int dwFlags);
	void (CALLBACK * ReleaseFileA)(const char * pPath);
	HANDLE (CALLBACK * OpenHFileA)(const char * pPath, unsigned int * pDwSize, unsigned int dwFlags);
	void (CALLBACK * CloseHFile)(HANDLE hf);

	// Plugin: download file
	int (CALLBACK * LoadNetFileA) (char * pFilename);

	// Plugin: send command to Vitalize
	int (CALLBACK * NetCommandA) (int, void *, unsigned int, void *, unsigned int);

	// Editor & Runtime: Returns the version of MMF or of the runtime
	unsigned int (CALLBACK * GetVersion) ();

	// Editor & Runtime: callback function for properties or other functions
	LRESULT	(CALLBACK * CallFunction) (EDITDATA * edPtr, int nFnc, LPARAM lParam1, LPARAM lParam2, LPARAM lParam3);

	// Editor: Open Help file (UNICODE)
	void (CALLBACK * HelpW) (const wchar_t * pHelpFile, unsigned int nID, LPARAM lParam);

	// Editor: Get default font for object creation (UNICODE)
	BOOL (CALLBACK * GetDefaultFontW) (LOGFONTW * plf, wchar_t * pStyle, int cbSize);

	// Editor: Edit images and animations (UNICODE)
	BOOL (CALLBACK * EditSurfaceW) (EDITDATA * edPtr, EditImageParams * Params, HWND Parent);
	BOOL (CALLBACK * EditImageW) (EDITDATA * edPtr, EditImageParams * Params, HWND Parent);
	BOOL (CALLBACK * EditAnimationW) (EDITDATA * edPtr, EditAnimationParams * Params, HWND Parent);

	// Runtime: Binary files (UNICODE
	BOOL (CALLBACK * GetFileW)(const wchar_t * pPath, wchar_t * pFilePath, unsigned int dwFlags);
	void (CALLBACK * ReleaseFileW)(const wchar_t * pPath);
	HANDLE (CALLBACK * OpenHFileW)(const wchar_t * pPath, unsigned int * pDwSize, unsigned int dwFlags);

	// Plugin: download file
	int	(CALLBACK * LoadNetFileW) (wchar_t * pFilename);

	// Plugin: send command to Vitalize
	int	(CALLBACK * NetCommandW) (int, void *, unsigned int, void *, unsigned int);

	// Place-holder for next versions
	void * AdditionalFncs[6];

};
//typedef mv *LPMV;

#ifdef _UNICODE
	#define mvHelp	mvHelpW
	#define mvGetDefaultFont	mvGetDefaultFontW
	#define mvEditSurface	mvEditSurfaceW
	#define mvEditImage	mvEditImageW
	#define mvEditAnimation	mvEditAnimationW
	#define mvGetFile	mvGetFileW
	#define mvReleaseFile	mvReleaseFileW
	#define mvLoadNetFile	mvLoadNetFileW
	#define mvNetCommand	mvNetCommandW
	#define	mvGetFile		mvGetFileW
	#define	mvReleaseFile	mvReleaseFileW
	#define mvOpenHFile		mvOpenHFileW
#else
	#define mvHelp	mvHelpA
	#define mvGetDefaultFont	mvGetDefaultFontA
	#define mvEditSurface	mvEditSurfaceA
	#define mvEditImage	mvEditImageA
	#define mvEditAnimation	mvEditAnimationA
	#define mvGetFile	mvGetFileA
	#define mvReleaseFile	mvReleaseFileA
	#define mvLoadNetFile	mvLoadNetFileA
	#define mvNetCommand	mvNetCommandA
	#define	mvGetFile		mvGetFileA
	#define	mvReleaseFile	mvReleaseFileA
	#define mvOpenHFile		mvOpenHFileA
#endif

// Callback function identifiers for CallFunction
fancyenum(CallFunctionIDs) {
	// Editor only
	INSERTPROPS = 1,		// Insert properties into Property window
	REMOVEPROP,				// Remove property
	REMOVEPROPS,			// Remove properties
	REFRESHPROP,			// Refresh propery
	REALLOCEDITDATA,		// Reallocate edPtr
	GETPROPVALUE,			// Get object's property value
	GETAPPPROPVALUE,		// Get application's property value
	GETFRAMEPROPVALUE,		// Get frame's property value
	SETPROPVALUE,			// Set object's property value
	SETAPPPROPVALUE,		// Set application's property value
	SETFRAMEPROPVALUE,		// Set frame's property value
	GETPROPCHECK,			// Get object's property check state
	GETAPPPROPCHECK,		// Get application's property check state
	GETFRAMEPROPCHECK,		// Get frame's property check state
	SETPROPCHECK,			// Set object's property check state
	SETAPPPROPCHECK,		// Set application's property check state
	SETFRAMEPROPCHECK,		// Set frame's property check state
	INVALIDATEOBJECT,		// Refresh object in frame editor
	RECALCLAYOUT,			// Recalc runtime layout (docking)
	GETNITEMS,				// Get number of items - not yet implemented
	GETNEXTITEM,			// Get next item - not yet implemented
	GETNINSTANCES,			// Get number of item instances - not yet implemented
	GETNEXTINSTANCE,		// Get next item instance - not yet implemented

	// Editor & runtime
	MALLOC = 100,			// Allocate memory
	CALLOC,					// Allocate memory & set it to 0
	REALLOC,				// Re-allocate memory
	FREE,					// Free memory
	GETSOUNDMGR,			// Get sound manager
	CLOSESOUNDMGR,			// Close sound manager
	ENTERMODALLOOP,			// Reserved
	EXITMODALLOOP,			// Reserved
	CREATEEFFECT,			// Create effect (runtime only)
	DELETEEFFECT,			// Delete effect (runtime only)
	CREATEIMAGEFROMFILEA,	// Create image from file (runtime only)
	NEEDBACKGROUNDACCESS,	// HWA : tell the frame the frame surface can be read (runtime only)
	ISHWA,					// Returns TRUE if HWA version (editor and runtime)
	ISUNICODE,				// Returns TRUE if the editor or runtime is in Unicode mode
	ISUNICODEAPP,			// Returns TRUE if the application being loaded is a Unicode application
	GETAPPCODEPAGE,			// Returns the code page of the application
	CREATEIMAGEFROMFILEW,	// Create image from file (runtime only)
};}

// 3rd parameter of CREATEIMAGEFROMFILE
struct CreateImageFromFileInfo {
	int			nSize,
				xHS,
				yHS,
				xAP,
				yAP;
	COLORREF	trspColor;
};


// Callback function macros for mvCallFunction
__inline void mvInsertProps(mv * mV, EDITDATA * edPtr, PropData* pProperties, unsigned int nInsertPropID, BOOL bAfter) \
	{ if (!pProperties) return; mV->CallFunction(edPtr, CallFunctionIDs::INSERTPROPS, (LPARAM)pProperties, (LPARAM)nInsertPropID, (LPARAM)bAfter); }

__inline void mvRemoveProp(mv * mV, EDITDATA * edPtr, unsigned int nPropID) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::REMOVEPROP, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

__inline void mvRemoveProps(mv * mV, EDITDATA * edPtr, PropData* pProperties) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::REMOVEPROPS, (LPARAM)pProperties, (LPARAM)0, (LPARAM)0); }

__inline void mvRefreshProp(mv * mV, EDITDATA * edPtr, unsigned int nPropID, BOOL bReInit) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::REFRESHPROP, (LPARAM)nPropID, (LPARAM)bReInit, (LPARAM)0); }

__inline void * mvReAllocEditData(mv * mV, EDITDATA * edPtr, unsigned int dwNewSize) \
	{ return (void *)mV->CallFunction(edPtr, CallFunctionIDs::REALLOCEDITDATA, (LPARAM)edPtr, dwNewSize, 0); }

__inline Prop * mvGetPropValue(mv * mV, EDITDATA * edPtr, unsigned int nPropID) \
	{ return (Prop *)mV->CallFunction(edPtr, CallFunctionIDs::GETPROPVALUE, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

__inline Prop * mvGetAppPropValue(mv * mV, EDITDATA * edPtr, unsigned int nPropID) \
	{ return (Prop *)mV->CallFunction(edPtr, CallFunctionIDs::GETAPPPROPVALUE, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

__inline Prop * mvGetFramePropValue(mv * mV, EDITDATA * edPtr, unsigned int nPropID) \
	{ return (Prop *)mV->CallFunction(edPtr, CallFunctionIDs::GETFRAMEPROPVALUE, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

__inline void mvSetPropValue(mv * mV, EDITDATA * edPtr, unsigned int nPropID, Prop * pValue) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::SETPROPVALUE, (LPARAM)nPropID, (LPARAM)pValue, (LPARAM)0); }

__inline void mvSetAppPropValue(mv * mV, EDITDATA * edPtr, unsigned int nPropID, Prop * pValue) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::SETAPPPROPVALUE, (LPARAM)nPropID, (LPARAM)pValue, (LPARAM)0); }

__inline void mvSetFramePropValue(mv * mV, EDITDATA * edPtr, unsigned int nPropID, Prop * pValue) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::SETFRAMEPROPVALUE, (LPARAM)nPropID, (LPARAM)pValue, (LPARAM)0); }

__inline unsigned int mvGetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int nPropID) \
	{ return (unsigned int)mV->CallFunction(edPtr, CallFunctionIDs::GETPROPCHECK, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

__inline unsigned int mvGetAppPropCheck(mv * mV, EDITDATA * edPtr, unsigned int nPropID) \
	{ return (unsigned int)mV->CallFunction(edPtr, CallFunctionIDs::GETAPPPROPCHECK, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

__inline unsigned int mvGetFramePropCheck(mv * mV, EDITDATA * edPtr, unsigned int nPropID) \
	{ return (unsigned int)mV->CallFunction(edPtr, CallFunctionIDs::GETFRAMEPROPCHECK, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

__inline void mvSetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int nPropID, unsigned int nCheck) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::SETPROPCHECK, (LPARAM)nPropID, (LPARAM)nCheck, (LPARAM)0); }

__inline void mvSetAppPropCheck(mv * mV, EDITDATA * edPtr, unsigned int nPropID, unsigned int nCheck) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::SETAPPPROPCHECK, (LPARAM)nPropID, (LPARAM)nCheck, (LPARAM)0); }

__inline void mvSetFramePropCheck(mv * mV, EDITDATA * edPtr, unsigned int nPropID, unsigned int nCheck) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::SETFRAMEPROPCHECK, (LPARAM)nPropID, (LPARAM)nCheck, (LPARAM)0); }

__inline void mvInvalidateObject(mv * mV, EDITDATA * edPtr) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::INVALIDATEOBJECT, (LPARAM)0, (LPARAM)0, (LPARAM)0); }

__inline void * mvMalloc(mv * mV, int nSize) \
	{ return (void *)mV->CallFunction(NULL, CallFunctionIDs::MALLOC, (LPARAM)nSize, (LPARAM)0, (LPARAM)0); }

__inline void * mvCalloc(mv * mV, int nSize) \
	{ return (void *)mV->CallFunction(NULL, CallFunctionIDs::CALLOC, (LPARAM)nSize, (LPARAM)0, (LPARAM)0); }

__inline void * mvReAlloc(mv * mV, void * ptr, int nNewSize) \
	{ return (void *)mV->CallFunction(NULL, CallFunctionIDs::REALLOC, (LPARAM)ptr, (LPARAM)nNewSize, (LPARAM)0); }

__inline void mvFree(mv * mV, void * ptr) \
	{ mV->CallFunction(NULL, CallFunctionIDs::FREE, (LPARAM)ptr, (LPARAM)0, (LPARAM)0); }

__inline void mvRecalcLayout(mv * mV) \
	{ mV->CallFunction(NULL, CallFunctionIDs::RECALCLAYOUT, (LPARAM)0, (LPARAM)0, (LPARAM)0); }

__inline CSoundManager* mvGetSoundMgr(mv * mV) \
	{ mV->CallFunction(NULL, CallFunctionIDs::GETSOUNDMGR, (LPARAM)0, (LPARAM)0, (LPARAM)0); }

__inline void mvCloseSoundMgr(mv * mV) \
	{ mV->CallFunction(NULL, CallFunctionIDs::CLOSESOUNDMGR, (LPARAM)0, (LPARAM)0, (LPARAM)0); }

__inline int mvGetNItems(mv * mV, EDITDATA * edPtr, const char * extName) \
	{ return mV->CallFunction(edPtr, CallFunctionIDs::GETNITEMS, (LPARAM)extName, (LPARAM)0, (LPARAM)0); }

__inline void * mvGetFirstItem(mv * mV, EDITDATA * edPtr, const char * extName) \
	{ return (void *)mV->CallFunction(edPtr, CallFunctionIDs::GETNEXTITEM, (LPARAM)extName, (LPARAM)0, (LPARAM)0); }

__inline void * mvGetNextItem(mv * mV, EDITDATA * edPtr, void * edPtr1, const char * extName) \
	{ return (void *)mV->CallFunction(edPtr, CallFunctionIDs::GETNEXTITEM, (LPARAM)edPtr1, (LPARAM)extName, (LPARAM)0); }

#ifdef HWABETA

__inline BOOL mvCreateEffect(mv * mV, const char * pEffectName, LPINT pEffect, LPARAM* pEffectParam) \
	{ return (BOOL)mV->CallFunction(NULL, CallFunctionIDs::CREATEEFFECT, (LPARAM)pEffectName, (LPARAM)pEffect, (LPARAM)pEffectParam); }

__inline void mvDeleteEffect(mv * mV, int nEffect, LPARAM lEffectParam) \
	{ mV->CallFunction(NULL, CallFunctionIDs::DELETEEFFECT, (LPARAM)nEffect, (LPARAM)lEffectParam, (LPARAM)0); }

#endif // HWABETA

__inline BOOL mvCreateImageFromFileA(mv * mV, LPWORD pwImg, const char * pFilename, CreateImageFromFileInfo* pInfo) \
	{ return (BOOL)mV->CallFunction(NULL, CallFunctionIDs::CREATEIMAGEFROMFILEA, (LPARAM)pwImg, (LPARAM)pFilename, (LPARAM)pInfo); }

__inline BOOL mvCreateImageFromFileW(mv * mV, LPWORD pwImg, const wchar_t * pFilename, CreateImageFromFileInfo* pInfo) \
	{ return (BOOL)mV->CallFunction(NULL, CallFunctionIDs::CREATEIMAGEFROMFILEW, (LPARAM)pwImg, (LPARAM)pFilename, (LPARAM)pInfo); }

__inline void * mvNeebBackgroundAccess(mv * mV, CRunFrame* pFrame, BOOL bNeedAccess) \
	{ return (void *)mV->CallFunction(NULL, CallFunctionIDs::NEEDBACKGROUNDACCESS, (LPARAM)pFrame, (LPARAM)bNeedAccess, (LPARAM)0); }

__inline BOOL mvIsHWAVersion(mv * mV) \
	{ return mV->CallFunction(NULL, CallFunctionIDs::ISHWA, (LPARAM)0, (LPARAM)0, (LPARAM)0); }

__inline BOOL mvIsUnicodeVersion(mv * mV) \
	{ return mV->CallFunction(NULL, CallFunctionIDs::ISUNICODE, (LPARAM)0, (LPARAM)0, (LPARAM)0); }

__inline BOOL mvIsUnicodeApp(mv * mV, void * pApp) \
	{ return mV->CallFunction(NULL, CallFunctionIDs::ISUNICODEAPP, (LPARAM)pApp, (LPARAM)0, (LPARAM)0); }

__inline int mvGetAppCodePage(mv * mV, void * pApp) \
	{ return mV->CallFunction(NULL, CallFunctionIDs::GETAPPCODEPAGE, (LPARAM)pApp, (LPARAM)0, (LPARAM)0); }

#ifdef _UNICODE
	#define mvCreateImageFromFile	mvCreateImageFromFileW
#else
	#define mvCreateImageFromFile	mvCreateImageFromFileA
#endif

// Options for OpenHFile
#define	OHFF_LOCALFILE		bit1		// Vitalize mode only: don't try to download file from server

// EnumEltProc definition
typedef	int (CALLBACK* ENUMELTPROC)(unsigned short *, int, LPARAM, LPARAM);

// kpxGetInfos
#undef UNICODE
fancyenum(KGI) {
	VERSION,			// Version (required)
	NOTUSED,			// Not used
	PLUGIN,				// Version for plug-in (required)
	MULTIPLESUBTYPE,	// Allow sub-types
	NOTUSED2,			// Reserved
	ATX_CONTROL,		// Not used
	PRODUCT,			// Minimum product the extension is compatible with
	BUILD,				// Minimum build the extension is compatible with
	UNICODE,			// Returns TRUE if the extension is in Unicode
};}

// Extension function table
struct kpxFunc {
	HINSTANCE	kpxHInst;
	TCHAR *		kpxName,
		  *		kpxSubType;
	void *		kpxUserData;

	// See KGI:: above for int
	unsigned int (__stdcall * kpxGetInfos) (int);
	int  (__stdcall	* kpxLoadObject) (mv *, ObjectInfo *, unsigned char * , int);
	void (__stdcall	* kpxUnloadObject) (mv *, unsigned char * , int);
	void (__stdcall	* kpxUpdateFileNames) (mv *, TCHAR *, unsigned char * , void (__stdcall *)(LPCTSTR, TCHAR *));
	short(__stdcall	* kpxGetRunObjectInfos) (mv *, unsigned char * );
	short(__stdcall	* kpxWindowProc) (unsigned char * , HWND, unsigned int, WPARAM, LPARAM);
	int	 (__stdcall	* kpxEnumElts) (mv *, unsigned char * , ENUMELTPROC, ENUMELTPROC, LPARAM, LPARAM);
	int  (__stdcall	* kpxLoadData) (mv *, ObjectInfo *, unsigned char * , HFILE, unsigned int);		// Not used
	int  (__stdcall	* kpxUnloadData) (mv *, ObjectInfo *, unsigned char *);					// Not used
	void (__stdcall	* kpxStartApp) (mv *, CRunApp *);
	void (__stdcall	* kpxEndApp) (mv *, CRunApp *);
	void (__stdcall	* kpxStartFrame) (mv *, CRunApp *, int);
	void (__stdcall	* kpxEndFrame) (mv *, CRunApp *, int);
	HGLOBAL (__stdcall	* kpxUpdateEditStructure) (mv *, void *);

	// Vitalise function
	#ifdef VITALIZE
		BOOL				bValidated;
	#endif
	BOOL				bUnicode;
};
//typedef kpxFunc * fpKpxFunc;

// Movement Extension
struct MvxFnc {
	HINSTANCE	mvxHInst;
	TCHAR *		mvxFileTitle;

	CMvt * (CALLBACK * mvxCreateMvt) (unsigned int);

	#ifdef VITALIZE
		BOOL	bValidated;
	#endif
};

// Restore structure alignment
// See http://stupefydeveloper.blogspot.co.uk/2009/01/c-alignment-of-structure.html
#pragma pack (pop, mmf_master_header)


//////////////////////////////////////////////////////////////////////////////
//
// LO (frame object instance)
//
//
// Note: mainly used at runtime

struct LevelObject {

	unsigned short	Handle,			// HLO
					OiHandle;		// HOI
	int				X, Y;			// Coords
	unsigned short	ParentType,		// Parent type
					OiParentHandle,	// HOI Parent
					Layer,			// Layer
					Type;
	Spr *			Spr[4];			// Sprite handles for backdrop objects from layers > 1

};
// typedef LO *LPLO;
// typedef	LO *fpLevObj;


//////////////////////////////////////////////////////////////////////////////
//
// OI (frame object)
//

fancyenum(OILF)
{
	OCLOADED			= bit1,	//
	ELTLOADED			= bit2,	//
	TOLOAD				= bit3,	//
	TODELETE			= bit4,	//
	CURFRAME			= bit5,	//
	TORELOAD			= 0x20,	// Reload images when frame change
	IGNORELOADONCALL	= 0x40,	// Ignore load on call option
};}

////////////////////
// ObjInfo structure
////////////////////
//
// Note: mainly used at runtime
// only some members of the oiHdr member are valid at editing time
//
#ifdef OI
	#undef OI
#endif

struct ObjInfo {
	ObjInfoHeader		oiHdr;			// Header
	TCHAR *				oiName;			// Name
	Objects_Common *	oiOC;

	unsigned int		oiFileOffset,
						oiLoadFlags;
	unsigned short		oiLoadCount,
						oiCount;

#ifdef HWABETA
	unsigned char *		oiExtEffect;
	CEffectEx *			oiExtEffectEx;	// For backdrops, you must find a more efficient system (bank of effects for backdrops)
#endif // HWABETA

};

///////////////////////////////////////////////
//
// Added backdrop objects
//
struct bkd2 {

	unsigned short	loHnd,			// 0 
					oiHnd;			// 0 
	int				x,
					y;
	unsigned short	img,
					colMode,
					nLayer,
					obstacleType;
	Spr *			pSpr[4];
	unsigned int	inkEffect,
					inkEffectParam;

};
// typedef bkd2 *LPBKD2;

#define MAX_BKD2	(curFrame.m_maxObjects)

///////////////////////////////////////////////
//
// Layer at runtime
//
struct RunFrameLayer
{
	// Name
	TCHAR *		pName;			// Name

	// Offset
	int			x, y,			// Current offset
				dx, dy,			// Offset to apply to the next refresh

	// Added backdrops
				m_nBkd2Count;
	bkd2 *		m_pBkd2;

	// Ladders
	int			m_nLadderMax,
				m_nLadders;
	RECT *		m_pLadders;

	// Z-order max index for dynamic objects
	int			m_nZOrderMax;

	// Permanent data (EditFrameLayer)
	unsigned int	dwOptions;		// Options
	float			xCoef;
	float			yCoef;
	unsigned int	nBkdLOs,		// Number of backdrop objects
					nFirstLOIndex;	// Index of first backdrop object in LO table

	// EditFrameLayerEffect
	#ifdef HWABETA
		unsigned int	dwEffect;
		LPARAM			dwEffectParam;	// CEffectEx si extended
	#endif // HWABETA

	// Backup for restart
	unsigned int	backUp_dwOptions;
	float			backUp_xCoef,
					backUp_yCoef;
	unsigned int	backUp_nBkdLOs,
					backUp_nFirstLOIndex;

	#ifdef HWABETA
	//	unsigned int		backUp_dwEffect;		// A voir
	//	LPARAM		backUp_dwEffectParam;
	#endif // HWABETA

};

// Object transition data
struct objTransInfo {

	CTransition *	m_pTrans;				// Transition object
	cSurface *		m_SfSave,				// Background surface
			 *		m_SfDisplay,			// Working surface
			 *		m_Sf1,					// Source surface
			 *		m_Sf2;					// Destination surface
	BOOL			m_fadeOut,				// fade type
					m_fill1withbackground,	// source = background (opaque mode)
					m_fill2withbackground,	// dest = background (opaque mode)
					m_saved;				// m_SfSave has been updated
	int				m_xsave,
					m_ysave,
					m_cxsave,
					m_cysave;
	#ifdef HWABETA
		BOOL		m_bStepDrawBlit;		// Use StepDrawBlit instead of StepDraw
	#endif

};
// typedef objTransInfo * LPOBJTRANSINFO;

//////////////////////////////////////////////////////////////////////////////
//
// Frame at runtime
//

#define	MAX_TEMPSTRING	16
#define IPHONEOPT_JOYSTICK_FIRE1 bit1
#define IPHONEOPT_JOYSTICK_FIRE2 bit2
#define IPHONEOPT_JOYSTICK_LEFTHAND bit3
#define	IPHONEFOPT_MULTITOUCH			bit4
#define	IPHONEFOPT_SCREENLOCKING		bit5
#define	IPHONEFOPT_IPHONEFRAMEIAD		bit6
#define JOYSTICK_NONE 0x0000
#define JOYSTICK_TOUCH bit1
#define JOYSTICK_ACCELEROMETER bit2
#define JOYSTICK_EXT 0x0003


struct CRunFrame {
	// Header
	FrameHeader			hdr;

	// Name
	TCHAR *				name;

	// Palette
	LPLOGPALETTE		palette;

	// Background surface
	cSurface *			pSurface;

	// Coordinates of top-level pixel in edit window
	int					leX,
						leY,
						leLastScrlX,
						leLastScrlY,

	// Layers
						NumberOfLayers;
	RunFrameLayer *		LayerAccess;

	// Virtual rectangle
	RECT				VirtualRect;

	// Instances of frame objects
	int					FrameObjects_MaxIndex,
						FrameObjects_MaxHandle;
	unsigned short *	lo_handle_to_index;
	LevelObject *		los;

	// Variables for enumeration functions
	int					loFranIndex,
						oiFranIndex;

	// Remap table or palette handle for sub-application
	unsigned char * 	remapTable,

	// Transitions
				  * 	fadeIn,
				  *		fadeOut;
	cSurface *			pFadeInSurface1,
			 *			pFadeInSurface2;
	CTransition *		pTrans;

	// Exit code
	unsigned long		levelQuit;

	// Events
	int					rhOK;				// TRUE when the events are initialized
	RunHeader *			rhPtr;
	eventGroup *		eventPrograms;
	unsigned long		free[256-1];		// 256 = max event programs
	objInfoList *		oiList;
	void *				free0;
	unsigned short * 	qualToOiList,
				   * 	qualOilPtr,		// Do not liberate!
				   * 	qualOilPtr2;	// Do not liberate!
	short *				limitBuffer;
	unsigned long * 	listPointers,
				  *		eventPointers;
	qualifierLoad * 	qualifiers;
	short				nQualifiers;

	short				nConditions[7+OBJ::LAST]; // Number of system types + OBJ_LAST
	unsigned long		free2[256];			// 256 = max event programs
	unsigned short		wJoystick,
						wIPhoneOptions;
	unsigned char * 	swapBuffers;
	unsigned long		objectList;
	unsigned char * 	destroyList;
	int					free3,						// Ignore - reserved
						NumberOfPlayers,
						NumberOfPlayersReal,
						Level_loop_state,
						StartLeX,
						StartLeY;
	short				MaxObjects,
						MaxOI;
	objectsList *		ObjectsList_Access;
	int					ObjectsList_Count;
	BOOL				EventsBranched,
						Fade;
	unsigned long		FadeTimerDelta,
						FadeVblDelta,
						PasteMask;

	int					CurTempString;
	TCHAR *				TempString[MAX_TEMPSTRING];	// not used

	// Other
	cSurface *			SaveSurface;
	int					EditWinWidth,
						EditWinHeight;
	unsigned long		ColMaskBits;
	TCHAR *				demoFilePath;
	unsigned short		RandomSeed,
						Free;							// Ignore - padding
	unsigned long		MvtTimerBase;

	#ifdef HWABETA
		unsigned char * LayerEffects;

		// Frame effect
		FrameEffect *	Effect;						// Frame effect (chunk data, contains effect index & param used in blit)
		CEffectEx *		EffectEx;					// Current effect
		bool			FrameEffectChanged;			// Frame effect has been modified
		bool			AlwaysUseSecondarySurface;	// This frame always use a secondary surface

		// Secondary surface (render target used if background or frame effects)
		cSurface *		SecondarySurface;

		// List of sub-app surfaces to refresh at the end in D3D full screen mode
		CPList *			SurfacedSubApps;
	#endif

};
typedef CRunFrame *fpRunFrame;

////////////////////////
//
// Binary files
//
struct CBinaryFile {
	TCHAR			Path[MAX_PATH],	// path stored in ccn file
		  *			TempPath;		// path in temporary folder, if any
	unsigned long	FileSize,		// file size
					FileOffset;		// file offset in EXE/CCN file
	long			TempCount;		// usage count

	CBinaryFile() : 
		TempPath(NULL), FileSize(0), FileOffset(0), TempCount(0)
	{
		memset(Path, 0, MAX_PATH);
	}
	~CBinaryFile()
	{
		if (TempPath)
		{
			_tremove(TempPath);
			free(TempPath);
			TempPath = NULL;
			TempCount = 0;
		}
	}
};

//////////////////////////////////////////////////////////////////////////////
//
// Application
//

#define	ARF_MENUINIT				bit1
#define	ARF_MENUIMAGESLOADED		bit2		// menu images have been loaded into memory
#define	ARF_INGAMELOOP				bit3
#define ARF_PAUSEDBEFOREMODALLOOP	bit4

struct CRunApp {
	// Application info
	AppMiniHeader	miniHdr;			// Version
	AppHeader		hdr;				// General info
	TCHAR *			name,				// Name of the application
		  *			appFileName,		// filename (temporary file in editor mode)
		  *			editorFileName,		// filename of original .mfa file
		  *			copyright,			// copyright
		  *			aboutText;			// text to display in the About box

	// File infos
	TCHAR *			targetFileName;		// filename of original CCN/EXE file
	TCHAR *			tempPath;			// Temporary directory for external files
	HFILE			file;				// File handle
	unsigned long	startOffset;

	// Help file
	TCHAR *			doc;				// Help file pathname

	// Icon
	unsigned char *	icon16x16x8;		// = LPBITMAPINFOHEADER
	HICON			hIcon16x16x8;		// Small icon for the main window

	// Menu
	HMENU			hRunMenu;			// Menu
	unsigned char * accels;				// Accelerators
	TCHAR *			pMenuTexts;			// Menu texts (for ownerdraw menu)
	unsigned char * pMenuImages;		// Images index used in the menu
	MenuHdr *		pMenu;

	// Frame offsets
	int					frameMaxIndex;				// Max. number of frames
	int					frameMaxHandle;				// Max. frame handle
	unsigned short *	frame_handle_to_index;		// Handle -> index table
	unsigned long *		frameOffset;				// Frame offsets in the file

	// Frame passwords
	TCHAR **		framePasswords;				// Table of frame passwords (TCHAR * [])

	// Extensions
	int				nbKpx;						// Number of extensions
	kpxFunc *		kpxTab;						// Function table 1
	kpj (*kpxDataTable)[2];						// Function table 2 - pointer to kpj[2]

	// Movement Extensions
	int				nbMvx;						// Number of extensions
	MvxFnc *		mvxTable;					// DLL info

	// Elements
	TCHAR *			eltFileName[MAX_TABREF];	// Element banks
	HFILE			hfElt[MAX_TABREF];

	unsigned long	eltBaseOff;
	unsigned short	nbEltOff[MAX_TABREF];		// Sizes of file offset tables
	unsigned long * adTabEltOff[MAX_TABREF];	// File offsets of bank elements

	unsigned short	nbEltMemToDisk[MAX_TABREF],	// Size of elt cross-ref tables
					EltMemToDisk[MAX_TABREF],	// Element memory index -> element disk index
					EltDiskToMem[MAX_TABREF],	// Element disk index -> memory index

					tabNbCpt[MAX_TABREF];		// Sizes of usage count tables
	long *			tabAdCpt[MAX_TABREF];		// Usage count tables of bank elements

	// Binary files
	unsigned int	binaryFiles[8];

	// Temporary images
	unsigned int	NumberOfImagesTemp;			// List of temporary images (used by Replace Color action)
	unsigned char * ImagesTemp;					// Pointer to temporary images

	// Frame objects
	int					oiMaxIndex;
	int					oiMaxHandle;
	unsigned short *	oi_handle_to_index;
	ObjectInfo **		ois;
	int					oiFranIndex;			// for enumerating
	int					oiExtFranIndex;			// for enumerating

	// Sub-application
	CRunApp *		ParentApp;					// Parent application
	void *			SubAppObject;				// LPRS
	unsigned long	SubAppOptions;				// Sub-app options
	BOOL			SubAppIsVisible;			// Sub-app visibility
	HICON			SubAppIcon;					// Sub-app icon
	int				cx;							// Subapp: valid if stretched
	int				cy;

	// DLL infos
	void *			idAppli;					// App object in DLL
	int				nDepth;						// Screen depth
	cSurface*		protoLogScreen;				// Surface prototype

	// Edit window
	HWND			hEditWin;					// Edit Window handle
	void *			idEditWin;					// Edit Window identifier

	// Current frame
	CRunFrame*		Frame;						// Pointer to current frame

	// Run-time status
	int				bResizeTimer,
					refTime,
					appRunningState,
					startFrame,
					nextFrame,
					nCurrentFrame;
	BOOL			bWakeUp;
	unsigned short	oldFlags,
					appRunFlags;
	BOOL			bPlayFromMsgProc;

	// Debugger
	CDebugger *		pDebugger;

	// Full Screen
	int				depthFullScreen;
	WINDOWPLACEMENT sWndPlacement;				// Window position backup
	int				oldCxMax,					// Window size backup
					oldCyMax;
	cSurface *		pFullScreenSurface;

	// Global data
	BOOL			bSharePlayerCtrls,			// Sub-app: TRUE if shares player controls
					bShareLives,				// Sub-app: TRUE if shares lives
					bShareScores,				// Sub-app: TRUE if shares scores
					bShareGlobalValues,			// Sub-app: TRUE if shares global values
					bShareGlobalStrings;		// Sub-app: TRUE if shares global strings

	// Players
	PlayerCtrls	*	pPlayerCtrls;
	long *			pLives,
		 *			pScores;

	// Global values (warning: not valid if sub-app and global values are shared)
	unsigned char * pGlobalValuesInit;
	int				nGlobalValues;				// Number of global values
	CValue *		pGlobalValues;				// Global values
	unsigned char *	pFree,						// No longer used
				  *	pGlobalValueNames,

	// Global strings (warning: not valid if sub-app and global values are shared)
				  *	pGlobalStringInit;			// Default global string values
	int				nGlobalStrings;				// Number of global strings
	TCHAR **		pGlobalString;				// Pointers to global strings
	unsigned char * pGlobalStringNames;

	// Global objects
	unsigned char * AdGO;						// Global objects data

	// FRANCOIS
	short			NConditions[7+32-1];		// NUMBER_OF_SYSTEM_OBJECTS + KPX_BASE - 1->0 base change

	// External sound files
	TCHAR *			pExtMusicFile,
		  *			pExtSampleFile[32];			// External sample file per channel

	int				nInModalLoopCount;
	TCHAR *			pPlayerNames;
	unsigned long	dwColorCache;

	unsigned char * pVtz4Opt;					// not used
	unsigned long	dwFree;						// not used

	// Application load
	TCHAR *			pLoadFilename;
	unsigned int	saveVersion;
	BOOL			bLoading;

	// Bluray
	void *			pBROpt;

	// Build info
	AppHeader2 *	pHdr2;

	// Code page
	#ifdef _UNICODE
		unsigned long	dwCodePage;
		bool			bUnicodeAppFile;
	#endif

	// Effects
	#ifdef HWABETA
		unsigned char * HWA_Effects;					// Effects used in the application
		cSurface *		HWA_OldSecondarySurface;		// Secondary surface of the last frame, used in transitions
		bool			HWA_AlwaysUseSecondarySurface,	// At least one frame has a transition => always use a secondary surface in all the frames
						HWA_ShowWindowedMenu;			// To show menu after switch from full screen to windowed mode
		int				HWA_SubAppShowCount;			// To show the child window, otherwise it's not displayed...
	#endif // HWABETA

};
// typedef CRunApp*	fpRunApp;
//#endif // mmf_master_header
