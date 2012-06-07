// =======================================================================
// Internal errors
// =======================================================================
#ifndef _CfcError_h
#define	_CfcError_h

// General errors
#define	CFCERROR_MIN	0x40000000
#define	CFCERROR_MAX	0x40010000
enum {
	CFCERROR_NOERROR				= 0,

	// Not enough memory
	CFCERROR_NOT_ENOUGH_MEM			= CFCERROR_MIN,

	// Read error
	CFCERROR_READ_ERROR,

	// End of file reached
	CFCERROR_ENDOFFILE,

	// Write error
	CFCERROR_WRITE_ERROR,

	// Disk full
	CFCERROR_DISK_FULL,

	// Cannot open file
	CFCERROR_CANNOT_OPEN_FILE,

	// Cannot create file
	CFCERROR_CANNOT_CREATE_FILE,

	// Buffer too small (decompression)
	CFCERROR_BUFFER_TOO_SMALL,

	// Cannot set FileSize
	CFCERROR_CANNOT_SET_FILESIZE,

	// Internal error
	CFCERROR_UNKNOWN

};

// Surface errors
#define	SURFACEERROR_MIN	0x40010000
#define	SURFACEERROR_MAX	0x40020000
enum 
{
	SURFACEERROR_NOERR = 0,
	SURFACEERROR_NOT_SUPPORTED			= SURFACEERROR_MIN,
	SURFACEERROR_SURFACE_NOT_CREATED,
	SURFACEERROR_INTERNAL
};

#endif	// _CfcError_h
