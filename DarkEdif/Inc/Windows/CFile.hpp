#pragma once

// General errors
enum class CFCERROR : int {
	NONE = 0,
	MIN = 0x40000000,
	NOT_ENOUGH_MEM = MIN,
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

#define _In_valid_hfile_ _In_ _Pre_satisfies_(hf != INVALID_HANDLE_VALUE)
#define _In_positive_i32_ _In_range_(1, INT_MAX)

// Opens an existing filepath; needs access mode and optional share mode.
// acesss mode: OF_READ, OF_WRITE, OF_READWRITE
// share mode: OF_SHARE_EXCLUSIVE for no read-write from other processes, OF_SHARE_DENY_NONE for other processes
// having read-write access, OF_SHARE_DENY_WRITE
// @param mode OF_XX enum
FusionAPIImport _Success_(return != HFILE_ERROR) _Must_inspect_result_ _NODISCARD MMF2_ORD(424)
	PreferFusionFunc_Unicode(1033, "File_OpenW")
	HFILE FusionAPI File_OpenA(_In_z_ const char* fname, _In_ int mode) EXDEF;
// Creates a new file for reading/writing. If file exists, opens and clears its contents. Use File_Open if not preferred.
FusionAPIImport _Success_(return != HFILE_ERROR) _Must_inspect_result_ _NODISCARD MMF2_ORD(420)
	PreferFusionFunc_Unicode(1031, "File_CreateW")
	HFILE FusionAPI File_CreateA(_In_z_ const char* fname) EXDEF;
// Reads the given number of bytes from the file handle. Returns 0 on success, CFCERROR enum on failure.
FusionAPIImport _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(425)
	int FusionAPI File_Read(_In_valid_hfile_ HFILE hf, _Out_writes_bytes_(len) void* buf, _In_ DWORD len) EXDEF;
// Reads given number of bytes, counting how many bytes were successfully read until EOF
// TODO: Confirm error handling
FusionAPIImport _Success_(return >= 0) _Must_inspect_result_ _NODISCARD MMF2_ORD(426)
	int FusionAPI File_ReadAndCount(_In_valid_hfile_ HFILE hf, _Out_writes_bytes_(len) void* buf, _In_ DWORD len) EXDEF;

// TODO: Confirm usage and update code docs and SAL
FusionAPIImport _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(428)
	int FusionAPI File_ReadShortIntelData(_In_valid_hfile_ HFILE hf, _Outptr_result_buffer_maybenull_(return) void** pBuf) EXDEF;

// TODO: Confirm usage and update code docs and SAL
FusionAPIImport _Success_(return >= 0) _Must_inspect_result_ MMF2_ORD(429)
	int FusionAPI File_ReadShortIntelString(_In_valid_hfile_ HFILE hf, _Outptr_result_maybenull_z_ void** pBuf) EXDEF;

// TODO: Confirm usage and update code docs and SAL
FusionAPIImport _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(427)
	int FusionAPI File_ReadLongIntelData(_In_valid_hfile_ HFILE hf,
		_Outptr_result_buffer_maybenull_(return) void** pBuf) EXDEF;

// Returns 0 on success, CFCERROR enum on failure.
FusionAPIImport _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(433)
	int FusionAPI File_Write(_In_valid_hfile_ HFILE hf, _In_reads_bytes_(len) void* buf, _In_ DWORD len) EXDEF;

// Gets offset from start of file
// TODO: What does this return when HFILE is invalid? 0? INT64_MAX?
FusionAPIImport _NODISCARD MMF2_ORD(423)
	DWORD FusionAPI File_GetPosition(_In_valid_hfile_ HFILE hf) EXDEF;
// Seeks to new position relative to file start, returning original position (from start?)
// Note: File_SeekBegin_Ex is preferable, but
// TODO: What does this return when HFILE is invalid? 0? INT64_MAX?
FusionAPIImport PreferFusionFunc_CF25_296_02(1145, "File_SeekBegin_Ex") MMF2_ORD(430)
	DWORD FusionAPI	File_SeekBegin(_In_valid_hfile_ HFILE hf, _In_range_(0, INT32_MAX) long pos) EXDEF;
// Seeks to new position relative to current position, returning original position (from start?)
// TODO: What does this return when HFILE is invalid? 0? INT64_MAX?
FusionAPIImport PreferFusionFunc_CF25_296_02(1146, "File_SeekCurrent_Ex") MMF2_ORD(431)
	DWORD FusionAPI	File_SeekCurrent(_In_valid_hfile_ HFILE hf, _In_ long pos) EXDEF;
// Performs a seek from the end of the file backwards by the specified amount.
// The position parameter should usually be <= 0.
// TODO: What does this return when HFILE is invalid? 0? INT64_MAX?
FusionAPIImport PreferFusionFunc_CF25_296_02(1147, "File_SeekEnd_Ex") MMF2_ORD(432)
	DWORD FusionAPI File_SeekEnd(_In_valid_hfile_ HFILE hf, _In_ long pos) EXDEF;

FusionAPIImport PreferFusionFunc_CF25_296_02(1143, "File_GetLength_Ex") _Success_(return >= 0) _Must_inspect_result_ _NODISCARD MMF2_ORD(422)
	// Returns the file length. Limited to 2GiB files; see File_GetLength_Ex, or SetFilePointerEx.
	long FusionAPI File_GetLength(_In_valid_hfile_ HFILE hf) EXDEF;

// Closes the given HFILE. Does not change its value. Recommended to use CloseHandle() instead.
// @remarks Due to possible write issues (e.g. out of space), you should avoid using this function
//			if writing the file, as you will not be able to handle flushing error scenarios, e.g. disk full.
//			A successful write does not mean it got to the disk; it means OS cached it for writing successfully.
FusionAPIImport [[deprecated("Use _lclose()")]] MMF2_ORD(419)
	void FusionAPI File_Close(_In_valid_hfile_ _Post_invalid_ HFILE hf) EXDEF;
FusionAPIImport _Must_inspect_result_ _NODISCARD MMF2_ORD(421)
	BOOL FusionAPI File_ExistA(_In_z_ const char* pName) EXDEF;

FusionAPIImport _Success_(return != HFILE_ERROR) _Must_inspect_result_ _NODISCARD MMF2_UNICODE_OR_CF25_REQUIRED(1033)
	HFILE FusionAPI File_OpenW(_In_z_ const UShortWCHAR* fname, _In_ int mode) EXDEF;
FusionAPIImport _Success_(return != HFILE_ERROR) _Must_inspect_result_ _NODISCARD  MMF2_UNICODE_OR_CF25_REQUIRED(1031)
	HFILE FusionAPI	File_CreateW(_In_z_ const UShortWCHAR* fname) EXDEF;
FusionAPIImport _Must_inspect_result_ _NODISCARD MMF2_UNICODE_OR_CF25_REQUIRED(1032)
	BOOL FusionAPI File_ExistW(_In_z_ const UShortWCHAR* pName) EXDEF;

// Phi note: limited some position parameters to signed int64 max, cos GetLengthEx is.
// I doubt we'll ever have >8 EiB files though.
// All these Ex functions are introduced in base CF2.5 from build 292.27, dated 1st April 2019.
FusionAPIImport _Ret_range_(0, INT64_MAX) _Must_inspect_result_ _NODISCARD CF25_292_27_REQUIRED(1143)
	std::int64_t FusionAPI File_GetLength_Ex(_In_valid_hfile_ HFILE hf) EXDEF;
FusionAPIImport _Ret_range_(0, INT64_MAX) _Must_inspect_result_ _NODISCARD CF25_292_27_REQUIRED(1144)
	std::uint64_t FusionAPI File_GetPosition_Ex(_In_valid_hfile_ HFILE hf) EXDEF;
FusionAPIImport CF25_292_27_REQUIRED(1145) std::uint64_t FusionAPI File_SeekBegin_Ex(_In_valid_hfile_ HFILE hf, _In_range_(0, INT64_MAX) std::int64_t pos) EXDEF;
FusionAPIImport CF25_292_27_REQUIRED(1146) std::uint64_t FusionAPI File_SeekCurrent_Ex(_In_valid_hfile_ HFILE hf, _In_ std::int64_t pos) EXDEF;
FusionAPIImport CF25_292_27_REQUIRED(1147) std::uint64_t FusionAPI File_SeekEnd_Ex(_In_valid_hfile_ HFILE hf, _In_ std::int64_t pos) EXDEF;

#ifdef _UNICODE
#define File_Open File_OpenW
#define File_Create File_CreateW
#define File_Exist File_ExistW
#else // !_UNICODE
#define File_Open File_OpenA
#define File_Create File_CreateA
#define File_Exist File_ExistA
#endif // !_UNICODE

#define	File_ReadIntelWord(h,p,l)	File_Read(h,p,l)
#define	File_ReadIntelDWord(h,p,l)	File_Read(h,p,l)

// Input file - abstract class
class FusionAPIImport CInputFile
{
public:
	virtual ~CInputFile() {};

	// Deletes the object.
	// TODO: Is this opposite of Create() like cSurface, or opposite of NewInstance?
	virtual void MMF2_ORD(398) Delete() EXDEF;

	// Reads a buffer of unsigned int8. Returns 0 on success, CFCERROR enum on failure.
	// Written contents of buffer is undefined if EOF error occurs.
	// TODO: Confirm return type is CFCERROR or not
	virtual _Success_(return == 0) _Must_inspect_result_
		int Read(_Out_writes_bytes_(lsize) LPBYTE dest, _In_ DWORD lsize) = 0;

	// Reads a buffer of unsigned int8. Returns 0 on success, CFCERROR enum on failure.
	// EOF may result in incomplete buffer.
	// Passing null as pRead may be acceptable, or may not.
	virtual _Success_(return == 0) _Must_inspect_result_
		int Read(_Out_writes_bytes_to_(lsize, *pRead) LPBYTE dest, _In_ DWORD lsize, _Out_ LPDWORD pRead) = 0;

	// Reads a byte (uint8). Returns 0 on success, CFCERROR enum on failure.
	virtual _Success_(return == 0) _Must_inspect_result_
		int ReadByte(_Out_ LPBYTE dest) = 0;

	// Reads a little endian unsigned int16. Returns 0 on success, CFCERROR enum on failure.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(669)
		int ReadIntelWord(_Out_ LPWORD dest) EXDEF;
	// Reads a little endian unsigned int32. Returns 0 on success, CFCERROR enum on failure.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(665)
		int ReadIntelDWord(_Out_ LPDWORD dest) EXDEF;
	// Reads a big endian unsigned int16. Returns 0 on success, CFCERROR enum on failure.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(675)
		int ReadMacWord(_Out_ LPWORD dest) EXDEF;
	// Reads a big endian unsigned int32. Returns 0 on success, CFCERROR enum on failure.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(671)
		int ReadMacDWord(_Out_ LPDWORD dest) EXDEF;
	// Reads a little endian float32. Returns 0 on success, CFCERROR enum on failure.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(667)
		int ReadIntelFloat(_Out_ PFLOAT dest) EXDEF;
	// Reads a big endian float32. Returns 0 on success, CFCERROR enum on failure.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(673)
		int ReadMacFloat(_Out_ PFLOAT dest) EXDEF;
	// Reads array of little endian unsigned int16. Returns 0 on success, CFCERROR enum on failure.
	// Contents of array is undefined if EOF error occurs.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(670)
		int ReadIntelWordArray(_Out_writes_(count) LPWORD dest, _In_range_(0, INT_MAX) int count) EXDEF;
	// Reads an array of little endian unsigned int32. Returns 0 on success, CFCERROR enum on failure.
	// Contents of array is undefined if EOF error occurs.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(666)
		int ReadIntelDWordArray(_Out_writes_(count) LPDWORD dest, _In_range_(0, INT_MAX) int count) EXDEF;
	// Reads an array of little endian float32. Returns 0 on success, CFCERROR enum on failure.
	// Contents of array is undefined if EOF error occurs.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(668)
		int ReadIntelFloatArray(_Out_writes_(count) PFLOAT dest, _In_range_(0, INT_MAX) int count) EXDEF;
	// Reads an array of big endian unsigned int16. Returns 0 on success, CFCERROR enum on failure.
	// Contents of array is undefined if EOF error occurs.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(676)
		int ReadMacWordArray(_Out_writes_(count) LPWORD dest, _In_range_(0, INT_MAX) int count) EXDEF;
	// Reads an array of big endian unsigned int32. Returns 0 on success, CFCERROR enum on failure.
	// Contents of array is undefined if EOF error occurs.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(672)
		int ReadMacDWordArray(_Out_writes_(count) LPDWORD dest, _In_range_(0, INT_MAX) int count) EXDEF;
	// Reads an array of big endian float32. Returns 0 on success, CFCERROR enum on failure.
	// Contents of array is undefined if EOF error occurs.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(674)
		int ReadMacFloatArray(_Out_writes_(count) PFLOAT dest, _In_range_(0, INT_MAX) int count) EXDEF;

	// Gets byte position inside file, limited to 2GiB; undefined return if file is larger than 2GiB.
	// Use GetPositionEx instead, or detach the file and use SetFilePointerEx().
	virtual _Success_(return >= 0) _Must_inspect_result_ _NODISCARD PreferFusionFunc_CF25_296_02(1151, "CInputFile::GetPositionEx")
		long GetPosition() = 0;
	// Gets number of bytes of the file, limited to 2GiB; undefined return if file is larger than 2GiB.
	// Use GetLengthEx instead, or detach the file and use GetFileSizeEx().
	virtual _Success_(return >= 0) _Must_inspect_result_ _NODISCARD PreferFusionFunc_CF25_296_02(1149, "CInputFile::GetLengthEx")
		long GetLength() = 0;

	// Relocates the read position by byte offset relative to start or end of file, or current position of read cursor, depending on method.
	// See SEEK_XX for methods. When seeking from end, SEEK_END, an offset of -5 is 5 bytes from the end of the file.
	// Returns original byte position from start of file, limited to 2GiB.
	// @remarks While you can pass a positive offset and write beyond end of file,
	//			this may create a sparse file and that's probably not wanted.
	// TODO: Confirm return value semantics when not using start of file seek
	virtual _Success_(return >= 0) PreferFusionFunc_CF25_296_02(1153, "CInputFile::SeekEx")
		long Seek(_In_ _When_(method==SEEK_SET, _In_range_(0, INT32_MAX))
			_When_(method==SEEK_END, _In_range_(INT32_MIN, 0))
			long offset, _In_range_(0,2) int method) = 0;

	// Stores a file handle inside the file. The file pointer is not invalidated.
	// TODO: Confirm NULL or INVALID_HANDLE_VALUE.
	// TODO: Confirm HFILE swap possible or not.
	virtual _Success_(return >= 0) _Must_inspect_result_
		int Attach(_In_valid_hfile_ HANDLE hf) = 0;
	// Removes the stored file handle from the class. The next detach call will return null.
	// TODO: Confirm NULL or INVALID_HANDLE_VALUE.
	virtual _Must_inspect_result_ _NODISCARD
		HANDLE Detach() = 0;

	// Reads a section of the file. The memory is const!
	virtual _Success_(return != nullptr) _Must_inspect_result_ _Ret_bytecount_(nSize) _NODISCARD
		LPBYTE GetBuffer(_In_ UINT nSize) EXDEF;

	// For freeing if you did Create() then DetachBuffer() only.
	// Otherwise, destroy the object, or detach the buffer then free.
	virtual void FreeBuffer(_Pre_notnull_ _Post_ptr_invalid_ LPBYTE buf) EXDEF;

	// Gets the file name of this CInputFile. May be null if a memory-based file like CInputMemFile.
	virtual	_Ret_maybenull_z_ _NODISCARD PreferFusionFunc_Unicode(0, "CInputFile::GetFileNameW") FusionANSIWarning
		char * GetFileNameA() = 0;
	// Gets the file name of this CInputFile. May be null if a memory-based file like CInputMemFile.
	virtual	_Ret_maybenull_z_ _NODISCARD MMF2_UNICODE_OR_CF25_REQUIRED(0)
		UShortWCHAR * GetFileNameW() = 0;

	// Recommended you don't use these, due to conflict with e.g. IFileDialog::GetFileName
#ifdef _CFCFILE_UNICODE_DEFS
#if defined(_UNICODE)
#define GetFileName GetFileNameW
#else
#define GetFileName GetFileNameA
#endif
#endif

	// Gets byte position inside file. Requires CF2.5 build 296.27 or later.
	virtual _Success_(return >= 0) _Must_inspect_result_ _NODISCARD CF25_292_27_REQUIRED(1151)
		LONGLONG GetPositionEx()
	{
		// This code shouldn't run, it should call the derived class's implementation.
		LOGW(_T("Calling base " __FUNCTION__ " function when should be calling derived.\n"));
		return GetPosition();
	}

	// Gets number of bytes of the file. Requires CF2.5 build 296.26 or later.
	virtual _Success_(return >= 0) _Must_inspect_result_ _NODISCARD
		LONGLONG GetLengthEx()
	{
		// This code shouldn't run, it should call the derived class's implementation.
		LOGW(_T("Calling base " __FUNCTION__ " function when should be calling derived.\n"));
		return GetLength();
	}

	// Relocates the read position by byte offset relative to start or end of file, or current position of read cursor, depending on method.
	// Requires CF2.5 build 296.26 or later.
	// See SEEK_XX for methods. When seeking from end, SEEK_END, an offset of -5 is 5 bytes from the end of the file.
	// Returns original byte position from start of file.
	// @remarks While you can pass a positive offset and write beyond end of file,
	//			this may create a sparse file and that's probably not wanted.
	// TODO: Confirm return value semantics when not using start of file seek
	// Ordinal 5309.
	virtual _Success_(return >= 0)
		LONGLONG SeekEx(_In_ _When_(method == SEEK_SET, _In_range_(0, INT64_MAX))
			_When_(method == SEEK_END, _In_range_(INT64_MIN, 0))
			LONGLONG offset, _In_range_(0, 2) int method)
	{
		// This code shouldn't run, it should call the derived class's implementation.
		LOGW(_T("Calling base " __FUNCTION__ " function when should be calling derived.\n"));
		return GetLength();
	}

};
//typedef CInputFile * LPINPUTFILE;

// Bufferized input file
class FusionAPIImport CInputBufFile : public CInputFile
{
public:
	CInputBufFile() EXDEF;
	virtual ~CInputBufFile() EXDEF;

	static _Ret_maybenull_
		CInputBufFile * NewInstance() EXDEF;

	// Creates a buffer that is attached to the file.
	// Returns 0 on success, CFCERROR enum on failure.
	_Success_(return == 0) _Must_inspect_result_
		int Create(_In_valid_hfile_ HFILE hf) EXDEF;
	// Creates a buffer that is attached to the given byte range of a file, from [dwOffset, dwOffset+dwSize).
	// Returns 0 on success, CFCERROR enum on failure.
	_Success_(return == 0) _Must_inspect_result_
		int Create(_In_valid_hfile_ HFILE hf, _In_ DWORD dwOffset, _In_ DWORD dwSize) EXDEF;
	// Returns 0 on success, CFCERROR enum on failure.
	_Success_(return == 0) _Must_inspect_result_
		int Create(_In_z_ const char * filename) EXDEF;
	// Returns 0 on success, CFCERROR enum on failure.
	_Success_(return == 0) _Must_inspect_result_
		int Create(_In_z_ const char * filename, _In_ DWORD dwOffset, _In_ DWORD dwSize) EXDEF;
	// Returns 0 on success, CFCERROR enum on failure.
	_Success_(return == 0) _Must_inspect_result_
		int Create(_In_z_ const UShortWCHAR * filename) EXDEF;
	// Returns 0 on success, CFCERROR enum on failure.
	_Success_(return == 0) _Must_inspect_result_
		int Create(_In_z_ const UShortWCHAR * filename, _In_ DWORD dwOffset, _In_ DWORD dwSize) EXDEF;

	// Returns 0 on success, CFCERROR enum on failure.
	// Requires CF2.5 build 296.26 or later.
	_Success_(return == 0) _Must_inspect_result_ MMF2_ORD(1140)
		int CreateEx(_In_valid_hfile_ HFILE hf, _In_ ULONGLONG dwOffset, _In_ ULONGLONG dwSize) EXDEF;
	// Returns 0 on success, CFCERROR enum on failure.
	// Requires CF2.5 build 296.26 or later.
	_Success_(return == 0) _Must_inspect_result_ MMF2_ORD(1141)
		int CreateEx(_In_z_ const char * filename, ULONGLONG dwOffset, _In_ ULONGLONG dwSize) EXDEF;
	// Returns 0 on success, CFCERROR enum on failure.
	// Requires CF2.5 build 296.26 or later.
	_Success_(return == 0) _Must_inspect_result_ MMF2_ORD(1142)
		int CreateEx(_In_z_ const UShortWCHAR * filename, _In_ ULONGLONG dwOffset, _In_ ULONGLONG dwSize) EXDEF;


	// Reads the given number of bytes, returns 0 on success; to see read amount, pass third param
	virtual _Success_(return == 0 && return < (int)CFCERROR::MIN) _Must_inspect_result_
		int Read(_Out_writes_bytes_(lsize) LPBYTE dest, _In_ DWORD lsize) EXDEF;
	// Reads the given number of bytes, returns 0 on success. The amount read is stored in pRead.
	virtual _Success_(return == 0 || return == CFCERROR::END_OF_FILE) _Must_inspect_result_
		int Read(_Out_writes_bytes_to_(lsize, *pRead) LPBYTE dest, _In_ DWORD lsize, _Out_ LPDWORD pRead) EXDEF;
	// Reads a byte, returns 0 on success
	virtual _Success_(return == 0) _Must_inspect_result_
		int ReadByte(_Out_ LPBYTE dest) EXDEF;
	// virtual int ReadWord(LPWORD dest) EXDEF;
	// virtual int ReadDWord(LPDWORD dest) EXDEF;
	virtual _Success_(return >= 0) _Must_inspect_result_ _NODISCARD
		long GetPosition() EXDEF;
	virtual _Success_(return >= 0) _Must_inspect_result_ _NODISCARD
		long GetLength() EXDEF;

	// Seeks to the specified relative offset from the given exact position.
	// When seeking from end, SEEK_END, an offset of 5 is 5 bytes from the end of the file.
	// The return value is the position from start of file.
	// TODO: Confirm return value semantics when not using start of file seek
	virtual _Success_(return >= 0)
		long Seek(
			_In_ _When_(method==SEEK_SET, _In_range_(0, INT32_MAX))
			long offset, _In_range_(0,2) int method) EXDEF;

	// Stores the underlying file handle and adjusts the internal range to the full file
	// TODO: Investigate ownership mechanics

	virtual _Success_(return >= 0) _Must_inspect_result_
		int Attach(_In_valid_hfile_ HANDLE hf) EXDEF;
	// Returns the underlying file handle and removes it from ownership
	virtual	_Must_inspect_result_ _NODISCARD
		HANDLE Detach() EXDEF;

	// virtual unsigned char * GetBuffer(UINT nSize) EXDEF;
	// virtual void FreeBuffer(LPBYTE buf) EXDEF;

	// Returns the filename in ANSI - null if no underlying handle, e.g. a CInputMemFile
	virtual	_Ret_maybenull_z_ _NODISCARD MMF2_ORD(496)
		char * GetFileNameA() EXDEF;
	// Returns the filename in Unicode format - null if no underlying handle, e.g. a CInputMemFile
	virtual	_Ret_maybenull_z_ _NODISCARD MMF2_UNICODE_OR_CF25_REQUIRED(1039)
		UShortWCHAR * GetFileNameW() EXDEF;
	// Gets byte position inside file. Requires CF2.5 build 296.26 or later.
	virtual _Success_(return >= 0) _Must_inspect_result_ _NODISCARD CF25_292_27_REQUIRED(1150)
		LONGLONG GetPositionEx() EXDEF;
	// Gets number of bytes of the file. Requires CF2.5 build 296.26 or later.
	virtual _Success_(return >= 0) _Must_inspect_result_ _NODISCARD CF25_292_27_REQUIRED(1148)
		LONGLONG GetLengthEx() EXDEF;
	// Relocates the read position by byte offset relative to start or end of file, or current position of read cursor, depending on method.
	// Requires CF2.5 build 296.26 or later.
	// See SEEK_XX for methods. When seeking from end, SEEK_END, an offset of -5 is 5 bytes from the end of the file.
	// Returns original byte position from start of file.
	// @remarks While you can pass a positive offset and write beyond end of file,
	//			this may create a sparse file and that's probably not wanted.
	virtual _Success_(return >= 0) CF25_292_27_REQUIRED(1152)
		LONGLONG SeekEx(_In_ _When_(method == SEEK_SET, _In_range_(0, INT64_MAX))
			_When_(method == SEEK_END, _In_range_(INT64_MIN, 0))
			LONGLONG offset, _In_range_(0, 2) int method) EXDEF;
protected:
	// Attach to a byte range [dwOffset, dwOffset + dwSize) of the passed handle.
	_Success_(return == 0) _Must_inspect_result_ MMF2_ORD(336)
		int Attach(_In_valid_hfile_ HANDLE hnd, _In_ DWORD dwOffset, _In_ DWORD dwSize) EXDEF;
	// Attach to a byte range [dwOffset, dwOffset + dwSize) of the passed handle.
	// Requires CF2.5 build 296.26 or later.
	_Success_(return == 0) _Must_inspect_result_ CF25_292_27_REQUIRED(1139)
		int AttachEx(_In_valid_hfile_ HANDLE hnd, _In_ ULONGLONG dwOffset, _In_ ULONGLONG dwSize) EXDEF;
private:
	HFILE			m_hf;
	union {
		// MMF2, and CF2.5 before build 292.26
		DWORD			m_curpos;
		// CF2.5 after 292.26
		struct posAndSize {
			ULONGLONG m_curpos;
			ULONGLONG m_length;
		} * m_pPosSize;
	};
	LPBYTE			m_buffer;
	LPBYTE			m_bufcurr;
	DWORD			m_remains;
	union {
		// MMF2 base
		BOOL	m_bToClose;
		// MMF2 Unicode, CF2.5
		// @remarks Not confirmed this was switched over for MMF2 Unicode yet
		UShortWCHAR* m_fnameW;
	};
	char *			m_fnameA;

	union {
		// MMF2, and CF2.5 before build 292.27
		struct OldCF {
			DWORD			m_startOffset;
			DWORD			m_length;
		} OldCF;
		// CF2.5 after 292.27
		ULONGLONG m_startOffset;
	};
};
//typedef	CInputBufFile * LPINPUTBUFFILE;

#define	BUFFILE_BUFFER_SIZE	16384

// Memory input file
class FusionAPIImport CInputMemFile : public CInputFile
{
public:
	MMF2_ORD(249) CInputMemFile() EXDEF;
	MMF2_ORD(248) CInputMemFile(const CInputMemFile&) EXDEF;
	virtual MMF2_ORD(276) ~CInputMemFile() EXDEF;

	static _Ret_maybenull_ MMF2_ORD(638)
		CInputMemFile *	NewInstance() EXDEF;

	// Points the internal buffer to the given pointer and size
	// Returns 0 on success, CFCERROR enum on failure.
	_Success_(return == 0) _Must_inspect_result_ MMF2_ORD(366)
		int Create(_Pre_notnull_ _Pre_readable_byte_size_(lsize) LPBYTE buffer, _In_ DWORD lsize) EXDEF;
	// Allocates a buffer pointing to the given pointer and size
	// Returns 0 on success, CFCERROR enum on failure.
	_Success_(return == 0) _Must_inspect_result_ MMF2_ORD(791)
		int Create(_In_ DWORD lsize) EXDEF;

	// Gets the underlying memory buffer, pointing to the start.
	_Ret_maybenull_ _Must_inspect_result_ _NODISCARD MMF2_ORD(473)
		LPBYTE GetMemBuffer() EXDEF;

	// Reads a given section from the buffer, advancing the internal cursor. Returns 0 on success.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(659)
		int Read(_Out_writes_bytes_(lsize) LPBYTE dest, _In_ DWORD lsize) EXDEF;
	// Reads a given section from the buffer, advancing the internal cursor. Returns 0 on success.
	// Writes the number of read bytes to parameter.
	virtual _Success_(return == 0 || return == CFCERROR::END_OF_FILE) _Must_inspect_result_ MMF2_ORD(660)
		int Read(_Out_writes_bytes_to_(lsize, *pRead) LPBYTE dest, _In_ DWORD lsize, _Out_ LPDWORD pRead) EXDEF;
	// Reads a single byte from the buffer to the parameter, advancing the internal cursor. Returns 0 on success.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(663)
		int ReadByte(_Out_ LPBYTE dest) EXDEF;
	// virtual int 			ReadWord(LPWORD dest) EXDEF;
	// virtual int 			ReadDWord(LPDWORD dest) EXDEF;

	// Reads the current position of the internal cursor relative to start of buffer, limited to 2GiB.
	// TODO: What does this return if no buffer?
	virtual _Success_(return >= 0) _Must_inspect_result_ MMF2_ORD(559)
		long GetPosition() EXDEF;

	// Reads the size of the current buffer, limited to 2GiB.
	// TODO: What does this return if no buffer? 0?
	virtual _Success_(return >= 0) _Must_inspect_result_ _NODISCARD MMF2_ORD(525)
		long GetLength() EXDEF;
	// Moves the internal cursor to a new position relative to the buffer.
	// Relocates the read position by byte offset relative to start or end of file, or current position of read cursor, depending on method.
	// See SEEK_XX for methods. When seeking from end, SEEK_END, an offset of -5 is 5 bytes from the end of the file.
	// Returns original byte position from start of file, limited to 2GiB.
	// TODO: Confirm return value semantics when not using start of file seek
	virtual _Success_(return >= 0) MMF2_ORD(714)
		long Seek(_In_ _When_(method == SEEK_SET, _In_range_(0, INT32_MAX))
			_When_(method == SEEK_END, _In_range_(INT32_MIN, 0))
			long offset,
			_In_range_(0, 2) int method) EXDEF;

	// Stores a file handle inside the file. The file pointer is not invalidated.
	// TODO: Confirm NULL or INVALID_HANDLE_VALUE.
	// TODO: Confirm HFILE swap possible or not.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(338)
		int Attach(_In_valid_hfile_ HANDLE hf) EXDEF;
	// Removes the stored file handle from the class. The next detach call will return null.
	// TODO: Confirm NULL or INVALID_HANDLE_VALUE.
	virtual	_Must_inspect_result_ _NODISCARD MMF2_ORD(404)
		HANDLE Detach() EXDEF;

	// Reads a section of the file. The memory is const!
	virtual _Success_(return != nullptr) _Must_inspect_result_ _Ret_bytecount_(nSize) _NODISCARD MMF2_ORD(473)
		LPBYTE GetBuffer(_In_ UINT nSize) EXDEF;
	// For freeing if you did Create() then DetachBuffer() only.
	// Otherwise, destroy the object, or detach the buffer then free.
	virtual MMF2_ORD(459) void FreeBuffer(_Pre_notnull_ _Post_ptr_invalid_ LPBYTE buf) EXDEF;

	// Returns null ptr, always
	virtual	_Ret_maybenull_z_ MMF2_ORD(497) PreferFusionFunc_Unicode(1040, "CInputMemFile::GetFileNameW")
		char * GetFileNameA() EXDEF;
	// Returns null ptr, always
	virtual	_Ret_maybenull_z_ MMF2_UNICODE_OR_CF25_REQUIRED(1040)
		UShortWCHAR * GetFileNameW() EXDEF;

private:
	LPBYTE	m_buffer;
	LPBYTE	m_bufcurr;
	DWORD	m_curpos;
	DWORD	m_remains;
	BOOL	m_bAutoDelete;
};
//typedef	CInputMemFile * LPINPUTMEMFILE;

// Output file: base class
class FusionAPIImport COutputFile
{
public:
	MMF2_ORD(251) COutputFile() EXDEF;
	MMF2_ORD(250) COutputFile(const COutputFile &) EXDEF;
	MMF2_ORD(300) COutputFile& operator = (const COutputFile&) EXDEF;
	virtual MMF2_ORD(278) ~COutputFile() EXDEF;

	// virtual int			WriteByte(BYTE b) = 0;
	// virtual int			WriteWord(WORD b) = 0;

	// Returns 0 on success, CFCERROR enum on failure.
	_Success_(return == 0) _Must_inspect_result_ MMF2_ORD(777)
		int WriteIntelWord(_In_ LPWORD pw) EXDEF;
	// Returns 0 on success, CFCERROR enum on failure.
	_Success_(return == 0) _Must_inspect_result_ MMF2_ORD(775)
		int WriteIntelDWord(_In_ LPDWORD pdw) EXDEF;
	// Returns 0 on success, CFCERROR enum on failure.
	_Success_(return == 0) _Must_inspect_result_ MMF2_ORD(776)
		int WriteIntelFloat(_In_ PFLOAT dest) EXDEF;
	// Returns 0 on success, CFCERROR enum on failure.
	_Success_(return == 0) _Must_inspect_result_ MMF2_ORD(779)
		int WriteMacWord(_In_ LPWORD pw) EXDEF;
	// Returns 0 on success, CFCERROR enum on failure.
	_Success_(return == 0) _Must_inspect_result_ MMF2_ORD(778)
		int WriteMacDWord(_In_ LPDWORD pdw) EXDEF;

	// Returns 0 on success, CFCERROR enum on failure.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(778)
		int Write(_In_reads_bytes_(sz) LPBYTE pb, _In_ UINT sz) = 0;
	// Returns 0 on success, CFCERROR enum on failure.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(778)
		int Flush() = 0;
	// Gets number of bytes written so far. Unsigned; no failure value documented.
	virtual _Must_inspect_result_ MMF2_ORD(778)
		DWORD GetLength() = 0;

	virtual _Success_(return >= 0) _Must_inspect_result_ _NODISCARD
		long GetPosition() = 0;
	virtual
		long Seek(long pos, int method) = 0;

	// Returns the filename in ANSI - null if no underlying handle, e.g. a COutputMemFile
	virtual	_Ret_maybenull_z_ MMF2_ORD(778) PreferFusionFunc_Unicode(0, "COutputFile::GetFileNameW")
		char * GetFileNameA() = 0;
	// Returns the filename in Unicode - null if no underlying handle, e.g. a COutputMemFile
	virtual	_Ret_maybenull_z_ MMF2_UNICODE_OR_CF25_REQUIRED(0)
		UShortWCHAR * GetFileNameW() = 0;
};
//typedef COutputFile * LPOUTPUTFILE;

// Memory output file
class FusionAPIImport COutputMemFile : public COutputFile
{
public:
	// Creates a RAM-backed faux file.
	MMF2_ORD(253) COutputMemFile() EXDEF;
	MMF2_ORD(252) COutputMemFile(const COutputMemFile &) EXDEF;
	// @remarks Is this noexcept?
	virtual MMF2_ORD(454) ~COutputMemFile() EXDEF;

	// Creates a buffer using Fusion runtime memory. Must free this with FreeBuffer().
	// Returns 0 on success, CFCERROR enum on failure.
	_Success_(return == 0) _Must_inspect_result_ MMF2_ORD(369)
		int Create(_In_ UINT nBlockSize = 512) EXDEF;

	// Creates a buffer using Fusion runtime memory. Must free with FreeBuffer().
	// Returns 0 on success, CFCERROR enum on failure.
	// @remarks Phi note: I removed the default value. Not sure why there's
	//			a default size for an input buffer.
	//			TODO: What sort of free does the COutputMemFile() do in dtor?
	//			DLL memory or runtime free?
	//			If the latter, we'll probably have to advise to not use this Create variant,
	//			as it makes state inconsistent
	_Success_(return == 0) _Must_inspect_result_ MMF2_ORD(370)
		int Create(_Pre_notnull_ LPBYTE buffer,
			_In_range_(0, INT32_MAX) DWORD nBufferSize) EXDEF;

	// Gets a copy of the underlying buffer pointer.
	_Ret_maybenull_ _Must_inspect_result_ _NODISCARD MMF2_ORD(474)
		LPBYTE GetBuffer() EXDEF;

	// Gets the underlying buffer, releasing it from the class.
	// The caller must free it using the correct function.
	_Ret_maybenull_ _Must_inspect_result_ _NODISCARD MMF2_ORD(406)
		LPBYTE DetachBuffer() EXDEF;

	// For freeing if you did Create() then DetachBuffer() only.
	// Otherwise, destroy the object, or detach the buffer then free.
	static void MMF2_ORD(460) FreeBuffer(_Pre_notnull_ _Post_ptr_invalid_ LPBYTE pBuffer) EXDEF;

	// Writes passed data to the memory buffer.
	// Returns 0 on success, CFCERROR enum on failure.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(774)
		int Write(_In_reads_bytes_(sz) LPBYTE pb, _In_ UINT sz) EXDEF;

	// Flushes any pending writes to the file.
	// Returns 0 on success, CFCERROR enum on failure.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(454)
		int Flush() EXDEF;
	// Gets number of bytes written so far. Unsigned; no failure value documented.
	virtual _Must_inspect_result_ _NODISCARD MMF2_ORD(517)
		DWORD GetLength() EXDEF;

	virtual _Success_(return == 0) _Must_inspect_result_ _NODISCARD MMF2_ORD(561)
		long GetPosition() EXDEF;
	virtual MMF2_ORD(716)
		long Seek(long pos, int method) EXDEF;

	// Returns null, as COutputMemFile stores in memory only.
	virtual	_Ret_null_ MMF2_ORD(499)
		char * GetFileNameA() { return nullptr; }
	// Returns null, as COutputMemFile stores in memory only.
	virtual	_Ret_null_ MMF2_UNICODE_OR_CF25_REQUIRED(1042)
		UShortWCHAR * GetFileNameW() { return nullptr; }

private:
	LPBYTE	m_buffer;
	LPBYTE	m_curptr;
	DWORD	m_totalsize;
	DWORD	m_cursize;
	DWORD	m_blocksize;
	BOOL	m_bReallocable;
};

// Bufferized output file
class FusionAPIImport COutputBufFile : public COutputFile
{
public:
	MMF2_ORD(784) COutputBufFile() EXDEF;
	MMF2_ORD(783) COutputBufFile(const COutputBufFile &) EXDEF;
	MMF2_ORD(277) virtual ~COutputBufFile() EXDEF;

	// Returns 0 on success, CFCERROR enum on failure.
	_Success_(return == 0) _Must_inspect_result_ MMF2_ORD(367)
		int Create(_In_ HFILE hf, _In_ UINT nBufferSize = 0xFFFF) EXDEF;
	// Opens write access to the given file, overwriting?
	// Returns 0 on success, CFCERROR enum on failure.
	// Use _FUSIONT("X") for filename if wanting Unicode compatibility.
	_Success_(return == 0) _Must_inspect_result_ MMF2_ORD(368)
		int Create(_In_z_ const char * fname, _In_ UINT nBufferSize = 0xFFFF) EXDEF;
	// Returns 0 on success, CFCERROR enum on failure.
	// Use _FUSIONT("X") for filename if wanting ANSI compatibility.
	_Success_(return == 0) _Must_inspect_result_ MMF2_ORD(1026)
		int Create(_In_z_ const UShortWCHAR * fname, _In_ UINT nBufferSize = 0xFFFF) EXDEF;

	// Writes passed data to file. It may not be flushed to disk immediately.
	// Returns 0 on success, CFCERROR enum on failure.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(773)
		int Write(_In_reads_bytes_(sz) _Const_ LPBYTE pb, _In_ UINT sz) EXDEF;
	// Flushes any pending writes to the file.
	// Returns 0 on success, CFCERROR enum on failure.
	virtual _Success_(return == 0) _Must_inspect_result_ MMF2_ORD(453)
		int Flush() EXDEF;

	// Gets file length, 0+.
	// @remarks TODO: Is this file length, or written file length?
	//			On invalid HFILE, what happens?
	virtual _Must_inspect_result_ MMF2_ORD(526)
		DWORD GetLength() EXDEF;

	// Gets file position, 0+ for valid file.
	// @remarks TODO: Is this file length, or written file length?
	virtual _Success_(return >= 0) _Must_inspect_result_ MMF2_ORD(560)
		long GetPosition() EXDEF;
	// Seeks to the specified relative offset from the given exact position.
	// When seeking from end, SEEK_END, an offset of 5 is 5 bytes from the end of the file.
	// The return value is the position from start of file.
	// TODO: Confirm return value semantics when not using start of file seek
	virtual  _Success_(return >= 0) _Must_inspect_result_ MMF2_ORD(715)
		long Seek(
			_When_(method==SEEK_SET, _In_range_(0, INT32_MAX))
			_In_ long offset, _In_range_(0,2) int method) EXDEF;

	// Gets filename in ANSI, or null if class is not holding a file (no successful Create call).
	virtual	_Ret_maybenull_z_ MMF2_ORD(498)
		char * GetFileNameA() EXDEF;
	// Gets filename in Unicode characters, or null if class is not holding a file (no successful Create call).
	virtual	_Ret_maybenull_z_ MMF2_ORD(1041)
		UShortWCHAR * GetFileNameW() EXDEF;

private:
	HFILE			m_hf;
	union {
		// MMF2 base
		BOOL		 m_bAutoClose;
		// MMF2 Unicode, CF2.5
		UShortWCHAR* m_fnameW;
	};
	LPBYTE			m_buffer;
	LPBYTE			m_curptr;
	DWORD			m_cursize;
	DWORD			m_buffersize;
	BOOL			m_bBuffered;
	char *			m_fnameA;
};
