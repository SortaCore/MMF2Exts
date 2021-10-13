// General errors
enum class CFCERROR {
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
};



FusionAPIImport HFILE			FusionAPI	File_OpenA(const char * fname, int mode);
FusionAPIImport HFILE			FusionAPI	File_CreateA(const char * fname);
FusionAPIImport int				FusionAPI	File_Read(HFILE hf, void * buf, unsigned int len);
FusionAPIImport int				FusionAPI	File_ReadAndCount(HFILE hf, void * buf, unsigned int len);
FusionAPIImport int				FusionAPI	File_ReadShortIntelData(HFILE hf, void ** pBuf);
FusionAPIImport int				FusionAPI	File_ReadShortIntelString(HFILE hf, void ** pBuf);
FusionAPIImport int				FusionAPI	File_ReadLongIntelData(HFILE hf, void ** pBuf);
FusionAPIImport int				FusionAPI	File_Write(HFILE hf, void * buf, unsigned int len);
FusionAPIImport unsigned int	FusionAPI	File_GetPosition(HFILE hf);
FusionAPIImport unsigned int	FusionAPI	File_SeekBegin(HFILE hf, long pos);
FusionAPIImport unsigned int	FusionAPI	File_SeekCurrent(HFILE hf, long pos);
FusionAPIImport unsigned int	FusionAPI	File_SeekEnd(HFILE hf, long pos);
FusionAPIImport long			FusionAPI	File_GetLength(HFILE hf);
FusionAPIImport void			FusionAPI	File_Close(HFILE hf);
FusionAPIImport BOOL			FusionAPI	File_ExistA(const char * pName);

FusionAPIImport HFILE			FusionAPI	File_OpenW(const wchar_t * fname, int mode);
FusionAPIImport HFILE			FusionAPI	File_CreateW(const wchar_t * fname);
FusionAPIImport BOOL			FusionAPI	File_ExistW(const wchar_t * pName);

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
class FusionAPIImport CInputFile
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

	virtual unsigned char * GetBuffer(UINT nSize);
	virtual void	FreeBuffer(unsigned char * buf);

	virtual	char * GetFileNameA() = 0;
	virtual	wchar_t * GetFileNameW() = 0;

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
class FusionAPIImport CInputBufFile : public CInputFile
{
public:
	CInputBufFile();
	virtual					~CInputBufFile();

	static CInputBufFile * NewInstance();

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

	virtual	char * GetFileNameA();
	virtual	wchar_t * GetFileNameW();

protected:
	int						Attach(HANDLE hnd, unsigned int dwOffset, unsigned int dwSize);
private:
	HFILE					m_hf;
	unsigned int			m_curpos;
	unsigned char * m_buffer;
	unsigned char * m_bufcurr;
	unsigned int			m_remains;
	wchar_t * m_fnameW;
	char * m_fnameA;

	unsigned int			m_startOffset;
	unsigned int			m_length;
};
typedef	CInputBufFile * LPINPUTBUFFILE;

#define	BUFFILE_BUFFER_SIZE	16384

// Memory input file
class FusionAPIImport CInputMemFile : public CInputFile
{
public:
	CInputMemFile();
	virtual					~CInputMemFile();

	static CInputMemFile * NewInstance();

	int						Create(unsigned char * buffer, unsigned long lsize);
	int						Create(unsigned long lsize);
	unsigned char * GetMemBuffer();

	virtual	int				Read(unsigned char * dest, unsigned long lsize);
	virtual	int				Read(unsigned char * dest, unsigned long lsize, LPDWORD pRead);
	virtual	int				ReadByte(unsigned char * dest);
	//		virtual	int				ReadWord(LPWORD dest);
	//		virtual	int				ReadDWord(LPDWORD dest);
	virtual long			GetPosition();
	virtual long			GetLength();
	virtual	long			Seek(long pos, int method);

	virtual	int				Attach(HANDLE hnd);
	virtual	HANDLE			Detach();

	virtual unsigned char * GetBuffer(UINT nSize);
	virtual void			FreeBuffer(unsigned char * buf);

	virtual	char * GetFileNameA() { return NULL; }
	virtual	wchar_t * GetFileNameW() { return NULL; }

private:
	unsigned char * m_buffer;
	unsigned char * m_bufcurr;
	unsigned int			m_curpos;
	unsigned int			m_remains;
	BOOL					m_bAutoDelete;
};
//typedef	CInputMemFile * LPINPUTMEMFILE;

// Output file: base class
class FusionAPIImport COutputFile
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

	virtual	char * GetFileNameA() = 0;
	virtual	wchar_t * GetFileNameW() = 0;
};
//typedef COutputFile * LPOUTPUTFILE;

// Memory output file
class FusionAPIImport COutputMemFile : public COutputFile
{
public:
	COutputMemFile();
	virtual					~COutputMemFile();

	int						Create(UINT nBlockSize = 512);
	int						Create(unsigned char * buffer, unsigned int nBufferSize = 0x7FFFFFF);
	unsigned char * GetBuffer();
	unsigned char * DetachBuffer();
	static void				FreeBuffer(unsigned char * pBuffer);	// car il faut libérer à l'interieur de la DLL

	virtual int				Write(unsigned char * pb, UINT sz);
	virtual int				Flush();
	virtual unsigned int	GetLength();

	virtual long			GetPosition();
	virtual long			Seek(long pos, int method);

	virtual	char * GetFileNameA() { return NULL; }
	virtual	wchar_t * GetFileNameW() { return NULL; }

private:
	unsigned char * m_buffer;
	unsigned char * m_curptr;
	unsigned int			m_totalsize;
	unsigned int			m_cursize;
	unsigned int			m_blocksize;
	BOOL					m_bReallocable;
};

// Bufferized output file
class FusionAPIImport COutputBufFile : public COutputFile
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

	virtual	char * GetFileNameA();
	virtual	wchar_t * GetFileNameW();

private:
	HFILE					m_hf;
	wchar_t * m_fnameW;
	unsigned char * m_buffer;
	unsigned char * m_curptr;
	unsigned int			m_cursize;
	unsigned int			m_buffersize;
	BOOL					m_bBuffered;
	char * m_fnameA;
};
