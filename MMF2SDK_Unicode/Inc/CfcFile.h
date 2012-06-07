
#ifndef _BFile_h
#define	_BFile_h

#ifndef LPLPVOID
typedef LPVOID FAR *LPLPVOID;
#endif

#include "CfcError.h"
#include "StdDefs.h"

#ifndef OF_READ
#define OF_READ             0x00000000
#define OF_WRITE            0x00000001
#define OF_READWRITE        0x00000002
#define OF_SHARE_COMPAT     0x00000000
#define OF_SHARE_EXCLUSIVE  0x00000010
#define OF_SHARE_DENY_WRITE 0x00000020
#define OF_SHARE_DENY_READ  0x00000030
#define OF_SHARE_DENY_NONE  0x00000040
#endif

STDDLL_API HFILE WINAPI	File_OpenA(LPCSTR fname, int mode);
STDDLL_API HFILE WINAPI	File_CreateA(LPCSTR fname);
STDDLL_API int WINAPI	File_Read (HFILE hf, LPVOID buf, DWORD len);
STDDLL_API int WINAPI	File_ReadAndCount (HFILE hf, LPVOID buf, DWORD len);
STDDLL_API int WINAPI	File_ReadShortIntelData (HFILE hf, LPLPVOID pBuf);
STDDLL_API int WINAPI	File_ReadShortIntelString (HFILE hf, LPLPVOID pBuf);
STDDLL_API int WINAPI	File_ReadLongIntelData (HFILE hf, LPLPVOID pBuf);
STDDLL_API int WINAPI	File_Write (HFILE hf, LPVOID buf, DWORD len);
STDDLL_API DWORD WINAPI	File_GetPosition (HFILE hf);
STDDLL_API DWORD WINAPI	File_SeekBegin (HFILE hf, long pos);
STDDLL_API DWORD WINAPI	File_SeekCurrent (HFILE hf, long pos);
STDDLL_API DWORD WINAPI	File_SeekEnd (HFILE hf, long pos);
STDDLL_API long WINAPI	File_GetLength ( HFILE hf );
STDDLL_API void WINAPI	File_Close(HFILE hf);
STDDLL_API BOOL WINAPI	File_ExistA(LPCSTR pName);

STDDLL_API HFILE WINAPI	File_OpenW(LPCWSTR fname, int mode);
STDDLL_API HFILE WINAPI	File_CreateW(LPCWSTR fname);
STDDLL_API BOOL WINAPI	File_ExistW(LPCWSTR pName);

#ifdef _UNICODE
#define File_Open File_OpenW
#define File_Create File_CreateW
#define File_Exist File_ExistW
#else
#define File_Open File_OpenA
#define File_Create File_CreateA
#define File_Exist File_ExistA
#endif

#ifndef _MAC
#define	File_ReadIntelWord(h,p,l)	File_Read(h,p,l)
#define	File_ReadIntelDWord(h,p,l)	File_Read(h,p,l)
#else
int		File_ReadIntelWord(HFILE hf, LPVOID buf, DWORD len);
int		File_ReadIntelDWord(HFILE hf, LPVOID buf, DWORD len);
#endif

// Input file - abstract class
class FAR STDDLL_API CInputFile
{
	public:
		virtual			~CInputFile() {};

		virtual void	Delete();

		virtual	int		Read(LPBYTE dest, DWORD lsize) = 0;
		virtual	int		Read (LPBYTE dest, DWORD lsize, LPDWORD pRead) = 0;
		virtual	int		ReadByte(LPBYTE dest) = 0;
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

		virtual LPBYTE	GetBuffer(UINT nSize);
		virtual void	FreeBuffer(LPBYTE buf);

		virtual	LPSTR	GetFileNameA() = 0;
		virtual	LPWSTR	GetFileNameW() = 0;

		#ifdef _CFCFILE_UNICODE_DEFS
			#if defined(_UNICODE)
				#define GetFileName GetFileNameW
			#else
				#define GetFileName GetFileNameA
			#endif
		#endif
};
typedef CInputFile FAR * LPINPUTFILE;

// Bufferized input file
class FAR STDDLL_API CInputBufFile : public CInputFile
{
	public:
						CInputBufFile();
		virtual			~CInputBufFile();

		static CInputBufFile*	NewInstance();

				int		Create(HFILE hf);
				int		Create(HFILE hf, DWORD dwOffset, DWORD dwSize);
				int		Create(LPCSTR filename);
				int		Create(LPCSTR filename, DWORD dwOffset, DWORD dwSize);
				int		Create(LPCWSTR filename);
				int		Create(LPCWSTR filename, DWORD dwOffset, DWORD dwSize);

		virtual	int		Read(LPBYTE dest, DWORD lsize);
		virtual	int		Read (LPBYTE dest, DWORD lsize, LPDWORD pRead);
		virtual	int		ReadByte(LPBYTE dest);
//		virtual	int		ReadWord(LPWORD dest);
//		virtual	int		ReadDWord(LPDWORD dest);
		virtual long	GetPosition();
		virtual long	GetLength();
		virtual	long	Seek(long pos, int method);
	
		virtual	int		Attach(HANDLE hnd);
		virtual	HANDLE	Detach();

//		virtual LPBYTE	GetBuffer(UINT nSize);
//		virtual void	FreeBuffer(LPBYTE buf);

		virtual	LPSTR	GetFileNameA();
		virtual	LPWSTR	GetFileNameW();

	protected:
				int		Attach(HANDLE hnd, DWORD dwOffset, DWORD dwSize);
	private:
		HFILE	m_hf;
		DWORD	m_curpos;
		LPBYTE	m_buffer;
		LPBYTE	m_bufcurr;
		DWORD	m_remains;
		LPWSTR	m_fnameW;
		LPSTR	m_fnameA;

		DWORD	m_startOffset;
		DWORD	m_length;
};
typedef	CInputBufFile FAR * LPINPUTBUFFILE;

#define	BUFFILE_BUFFER_SIZE	16384

// Memory input file
class FAR STDDLL_API CInputMemFile : public CInputFile
{
	public:
						CInputMemFile();
		virtual			~CInputMemFile();

		static CInputMemFile*	NewInstance();

		int				Create(LPBYTE buffer, DWORD lsize);
		int				Create(DWORD lsize);
		LPBYTE			GetMemBuffer();

		virtual	int		Read(LPBYTE dest, DWORD lsize);
		virtual	int		Read (LPBYTE dest, DWORD lsize, LPDWORD pRead);
		virtual	int		ReadByte(LPBYTE dest);
//		virtual	int		ReadWord(LPWORD dest);
//		virtual	int		ReadDWord(LPDWORD dest);
		virtual long	GetPosition();
		virtual long	GetLength();
		virtual	long	Seek(long pos, int method);
	
		virtual	int		Attach(HANDLE hnd);
		virtual	HANDLE	Detach();

		virtual LPBYTE	GetBuffer(UINT nSize);
		virtual void	FreeBuffer(LPBYTE buf);

		virtual	LPSTR	GetFileNameA() { return NULL; }
		virtual	LPWSTR	GetFileNameW() { return NULL; }

	private:
		LPBYTE	m_buffer;
		LPBYTE	m_bufcurr;
		DWORD	m_curpos;
		DWORD	m_remains;
		BOOL	m_bAutoDelete;
};
typedef	CInputMemFile FAR * LPINPUTMEMFILE;

// Output file: base class
class STDDLL_API COutputFile
{
	public:
		COutputFile() {};
		virtual ~COutputFile() {};

//		virtual int		WriteByte(BYTE b) = 0;
//		virtual int		WriteWord(WORD b) = 0;

		int				WriteIntelWord(LPWORD pw);
		int				WriteIntelDWord(LPDWORD pdw);
		int				WriteIntelFloat(PFLOAT dest);
		int				WriteMacWord(LPWORD pw);
		int				WriteMacDWord(LPDWORD pdw);

		virtual int		Write(LPBYTE pb, UINT sz) = 0;
		virtual int		Flush() = 0;
		virtual DWORD	GetLength() = 0;

		virtual long	GetPosition() = 0;
		virtual long	Seek(long pos, int method) = 0;

		virtual	LPSTR	GetFileNameA() = 0;
		virtual	LPWSTR	GetFileNameW() = 0;
};
typedef COutputFile FAR * LPOUTPUTFILE;

// Memory output file
class STDDLL_API COutputMemFile : public COutputFile
{
	public:
		COutputMemFile();
		virtual ~COutputMemFile();

		int		Create(UINT nBlockSize = 512);
		int		Create(LPBYTE buffer, DWORD nBufferSize = 0x7FFFFFF);
		LPBYTE	GetBuffer();
		LPBYTE	DetachBuffer();
		static void FreeBuffer(LPBYTE pBuffer);	// car il faut libérer à l'interieur de la DLL

		virtual int		Write(LPBYTE pb, UINT sz);
		virtual int		Flush();
		virtual DWORD	GetLength();

		virtual long	GetPosition();
		virtual long	Seek(long pos, int method);

		virtual	LPSTR	GetFileNameA() { return NULL; }
		virtual	LPWSTR	GetFileNameW() { return NULL; }

	private:
		LPBYTE	m_buffer;
		LPBYTE	m_curptr;
		DWORD	m_totalsize;
		DWORD	m_cursize;
		DWORD	m_blocksize;
		BOOL	m_bReallocable;
};

// Bufferized output file
class STDDLL_API COutputBufFile : public COutputFile
{
	public:
		COutputBufFile();
		virtual ~COutputBufFile();

		int Create(HFILE hf, UINT nBufferSize = 4096);
		int Create(LPCSTR fname, UINT nBufferSize = 4096);
		int Create(LPCWSTR fname, UINT nBufferSize = 4096);

		virtual int		Write(LPBYTE pb, UINT sz);
		virtual int		Flush();
		virtual DWORD	GetLength();

		virtual long	GetPosition();
		virtual long	Seek(long pos, int method);

		virtual	LPSTR	GetFileNameA();
		virtual	LPWSTR	GetFileNameW();

	private:
		HFILE	m_hf;
		LPWSTR	m_fnameW;
		LPBYTE	m_buffer;
		LPBYTE	m_curptr;
		DWORD	m_cursize;
		DWORD	m_buffersize;
		BOOL	m_bBuffered;
		LPSTR	m_fnameA;
};

#endif	// _BFile_h

