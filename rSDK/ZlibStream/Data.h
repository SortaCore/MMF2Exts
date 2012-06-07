class rRundata;
typedef rRundata * LPRRDATA;

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
	string LastOutput;			  //Last finished file name
	string returnstring;		  //String to return with statistics or errors
	volatile bool threadsafe;	  //Ensure only one thread accesses rdPtr's variables

	tagRDATA() : inbuffersize(8192), PercentageDifference(0.0), LastOutput(""), returnstring(""), threadsafe(false) //Constructor
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
