// ============================================================================
//
// This file contains the actions, conditions and expressions your object uses
// 
// ============================================================================

#include "common.h"
#include "unzip.h"
// ============================================================================
//
// GLOBAL VARIABLES
//
// ============================================================================
string returnstring = "";
TCHAR * LastFileExtracted = "";
bool NoCurrentCompression = true, PasswordBeingUsed = false;
int NumberOfFilesExtracted = 0;
HANDLE MainThread;
//Define struct used to export data from actions to threads
typedef struct Parameters {
    char *para_infilename;		//Input file name
    char *para_outfilename;		//Output file name
	char *para_password;		//Password
	LPRDATA para_rdPtr;			//rdPtr (must be given or threads cannot access rdPtr)
} MYDATA, *PMYDATA;

// ============================================================================
//
// THREADS
//
// ============================================================================

DWORD WINAPI decompress(LPVOID lpParam)
{	
	//Open struct and set variables
	PMYDATA pDataArray = (PMYDATA)lpParam;				//Open the struct
	char *infilename = pDataArray -> para_infilename;	//Get input file name
	char *outfilename = pDataArray -> para_outfilename;	//Get output file name
	char *password = pDataArray -> para_password;		//Get password
	LPRDATA rdPtr = pDataArray -> para_rdPtr;			//Get rdPtr
	
	//Set non-struct variables & begin extraction
	HZIP hz = OpenZip(infilename,0); //0 = no password
	SetUnzipBaseDir(hz,outfilename); //Set the extract-to folder
	ZIPENTRY baseze;
	ZRESULT zr = GetZipItem(hz,-1,&baseze); //ZRESULT zr contains error number.

	//This part may get complicated.
	//No error with no password, so just extract
	for (int zi=0; zi<baseze.index; zi++)
	{
		ZIPENTRY ze;
		GetZipItem(hz,zi,&ze);			// fetch individual details
		zr=UnzipItem(hz, zi, ze.name);	// e.g. the item's name.
	}
	CloseZip(hz);
	if (zr==ZR_OK)
	{
		rdPtr->rRd->PushEvent(0);
		NoCurrentCompression=true;
		PasswordBeingUsed=false;
	}
	else
	{ //Error occured when extracting with no password
		if (zr==ZR_PASSWORD&&password!="0")
		{ //ZR_PASSWORD -> invalid password. So try with password, if one is provided.
			zr = ZR_OK;
			CloseZip(hz);
			hz = OpenZip(infilename,password);
			GetZipItem(hz,-1,&baseze);
			NumberOfFilesExtracted=baseze.index;
			MessageBox(NULL,"zr==ZR_PASSWORD&&password!=\"0\"", "Debug", NULL);
			PasswordBeingUsed=true;
			
			//No error when extracting with given password, so just extract.
			for (int zi=0; zi<baseze.index; zi++)
			{	//I Can Haz Stallage
				ZIPENTRY ze;
				GetZipItem(hz,zi,&ze);			// fetch individual details
				zr=UnzipItem(hz, zi, ze.name);	// e.g. the item's name.
			}
			CloseZip(hz);
			NoCurrentCompression=true;
			if (zr==ZR_OK)
			{
				rdPtr->rRd->PushEvent(0);
				MessageBox(NULL,"zr==ZR_PASSWORD&&password!=\"0\" > zr==ZR_OK", "Debug", NULL);
			}
			else
			{
				if (zr==ZR_PASSWORD)
				{ //The password provided was wrong, so report it.
					returnstring="Password incorrect or required.";
					MessageBox(NULL,"zr==ZR_PASSWORD&&password!=\"0\" > zr==ZR_PASSWORD", "Debug", NULL);
					rdPtr->rRd->PushEvent(1);
				}
				else
				{ //Password was correct but error occured anyway
					MessageBox(NULL,"zr==ZR_PASSWORD&&password!=\"0\" > zr!=ZR_PASSWORD", "Debug", NULL);
					stringstream temp; 
					temp<<"An error occured, but the provided password was correct. Error number "<<zr<<".";
					returnstring=temp.str();
					NoCurrentCompression=true;
					rdPtr->rRd->PushEvent(1);
				}
			}
		}
		else
		{ //Not a password error when we tried to open without a password...
			stringstream temp; 
			temp<<"An error occured - not a password error. Error number "<<zr<<".";
			returnstring=temp.str();
			NoCurrentCompression=true;
			rdPtr->rRd->PushEvent(1);
		}
	}
	//FormatZipMessage(zr,LastFileExtracted,sizeof(LastFileExtracted));
	return 0;
}


// ============================================================================
//
// CONDITIONS
// 
// ============================================================================

CONDITION(
	/* ID */			0,
	/* Name */			"%o: On completion",
	/* Flags */			0,
	/* Params */		(0)
) {
	return true;
}

CONDITION(
	/* ID */			1,
	/* Name */			"%o: On error",
	/* Flags */			0,
	/* Params */		(0)
) {
	return true;
}

// ============================================================================
//
// ACTIONS
// 
// ============================================================================

ACTION(
	/* ID */		0,
	/* Name */		"Unzip archive (%0) --> (%1), password (%2)",
	/* Flags */		0,
	/* Params */	(3, PARAM_FILENAME, "Input file:",
						PARAM_STRING, "Output file:",
						PARAM_STRING, "Password: (use \"0\" for none)")
) {
	if (NoCurrentCompression)
	{
		//Retrieve variables from parameters
		char * p1=(char *)Param(TYPE_STRING);
		char * p2=(char *)Param(TYPE_STRING);
		char * p3=(char *)Param(TYPE_STRING);
		
		//Reset old variables
		returnstring="";
		PasswordBeingUsed=false;
		LastFileExtracted="";
		
		//Declare struct and set variables in struct
		PMYDATA Parameters = new MYDATA;	//Declaration
		Parameters->para_infilename=p1;		//Pass the input archive name
		Parameters->para_outfilename=p2;	//Pass the output folder name
		Parameters->para_password=p3;		//Pass the password
		Parameters->para_rdPtr=rdPtr;		//Pass rdPtr

		//Create thread
		MainThread=CreateThread(NULL, 0, decompress, Parameters, 0, NULL);
	}
	else
	{	//Only one decompression each time.
		returnstring = "Extraction already in progress.";
		rdPtr->rRd->PushEvent(1);
	}
}

// ============================================================================
//
// EXPRESSIONS
// 
// ============================================================================

EXPRESSION(
	/* ID */			0,
	/* Name */			"LastFileExtracted$(",
	/* Flags */			EXPFLAG_STRING,
	/* Params */		(0)
) {
	ReturnString(LastFileExtracted);
}

EXPRESSION(
	/* ID */			1,
	/* Name */			"PasswordUsed(",
	/* Flags */			0,
	/* Params */		(0)
) {
	return PasswordBeingUsed;
}

EXPRESSION(
	/* ID */			2,
	/* Name */			"Error$(",
	/* Flags */			EXPFLAG_STRING,
	/* Params */		(0)
) {
	ReturnString(returnstring.c_str());
}

EXPRESSION(
	/* ID */			3,
	/* Name */			"FileCount(",
	/* Flags */			0,
	/* Params */		(0)
) {
	return NumberOfFilesExtracted;
}
