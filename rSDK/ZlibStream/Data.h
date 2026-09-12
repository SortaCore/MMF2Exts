class rRundata;
typedef rRundata * LPRRDATA;

#ifdef _UNICODE
	#define EDITDATA	EDITDATAW
	#define LPEDATA		LPEDATAW
	typedef std::wstring tstring;
	typedef std::wstringstream tstringstream;
	#define t_gzopen(path, mode) gzopen_w(path, mode)
#else
	#define EDITDATA	EDITDATAA
	#define LPEDATA		LPEDATAA
	typedef std::string tstring;
	typedef std::stringstream tstringstream;
	#define t_gzopen(path, mode) gzopen(path, mode)
#endif

// --------------------------------
// RUNNING OBJECT DATA STRUCTURE
// --------------------------------
// If you want to store anything between actions/conditions/expressions
// you should store it here

typedef struct tagRDATA
{
	#include "MagicRDATA.h"

	//bool is volatile so it can be changed instantly by threads
	unsigned short inbuffersize;  //Default size of memory buffer
	double PercentageDifference;  //Percentage difference input->output
	tstring LastOutput;			  //Last finished file name
	tstring returnstring;		  //String to return with statistics or errors
	volatile bool threadsafe;	  //Ensure only one thread accesses rdPtr's variables

	tagRDATA() : inbuffersize(8192), PercentageDifference(0.0), LastOutput(_T("")), returnstring(_T("")), threadsafe(false) //Constructor
	{
		//vars initialized above in the initializer list
	}
	//functions
	~tagRDATA(){} //Destructor
} RUNDATA;
typedef	RUNDATA	* LPRDATA;

// --------------------------------
// EDITION OF OBJECT DATA STRUCTURE
// --------------------------------
// These values let you store data in your extension that will be saved in the MFA
// You should use these with properties

typedef struct tagEDATA_V1
{
	extHeader		eHeader;
//	short			swidth;
//	short			sheight;

} EDITDATA;
typedef EDITDATA * LPEDATA;
