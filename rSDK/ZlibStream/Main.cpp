// ============================================================================
//
// This file contains the actions, conditions and expressions your object uses
// 
// ============================================================================
#include "Common.h"

//Macros for Thread Safety
//If accessing rdPtr, wrap the accessing code in these
#define ThreadSafe_Start() \
	while (rdPtr -> threadsafe) \
	{ \
		Sleep(0); \
	} \
    rdPtr -> threadsafe=true
#define ThreadSafe_End() rdPtr -> threadsafe=false
//Macro for simplicity
#define Terminate() \
	ThreadSafe_Start(); \
	delete[] infilename, outfilename; \
	rdPtr -> LastOutput = outfilename; \
	rdPtr -> rRd -> CallEvent(1); \
	ThreadSafe_End(); \
    return 1

// Please note: Using GenerateEvent in place of PushEvent is not a good idea, being that
// access to rdPtr is blocked by ThreadSafe_Start. Example code:
//   ThreadSafe_Start();
//   rdPtr -> GenerateEvent(0);
//   ThreadSafe_End();
// If the user retrieves expressions with GenerateEvent, the ThreadSafe_End is not run
// until after they retrieve the variables. So of course the while loop in ThreadSafe_Start
// runs without stopping.
// PushEvent is slower, being called on next MMF loop, which means the expressions are accessed
// after: No "infinite-while-loop" problem.
// So here we have the caller type:
#define CallEvent PushEvent

//Define struct used to export data from actions to threads
struct Params
{
    char para_infilename [MAX_PATH];  //Input file name
    char para_outfilename [MAX_PATH]; //Output file name
	bool para_UseAppend;              //If true, append; otherwise (over)write output
    LPRDATA para_rdPtr;               //rdPtr (must be given or threads cannot access rdPtr)
};

//Calculate file size
unsigned long file_size(char *filename) {
   FILE *pFile = NULL;
   if (fopen_s(&pFile, filename, "rb"))  // Open file
	   return 0;						// If handle has a problem, return 0, eg no file
   fseek(pFile, 0, SEEK_END);          //If not, goto end of file
   unsigned long size = ftell(pFile);   //Get position (Must be stored so fclose can be run)
   fclose(pFile);                      //Close file
   return size;                         //Return
}

bool FileSizeCheck(char *infilename, LPRDATA rdPtr)
{
    if (file_size(infilename) < 1)
    {
        ThreadSafe_Start();
        rdPtr -> returnstring = "Input file size 0 or nonexistent.";
        ThreadSafe_End();
        return true;
    }
    else
        return false;
}

bool HandleCheck(FILE *file, gzFile gzfile, LPRDATA rdPtr)
{
    if (!file)
    {
        ThreadSafe_Start();
        rdPtr -> returnstring = "Input file malfunctioned.";
        gzclose(gzfile);
        ThreadSafe_End();
        return true;
    }
    else if (!gzfile)
    {
        ThreadSafe_Start();
        rdPtr -> returnstring = "Output file malfunctioned.";
        fclose(file);
        ThreadSafe_End();
        return true;
    }
    else
    {
        return false;
    }
}

//Compress a file
DWORD WINAPI compress_one_file(Params *pDataArray)
{    
    //Open struct and set variables
	char *infilename = new char [MAX_PATH];             //Get input file name
	strcpy_s(infilename, MAX_PATH, pDataArray -> para_infilename);
    char *outfilename = new char [MAX_PATH];             //Get output file name
	strcpy_s(outfilename, MAX_PATH, pDataArray -> para_outfilename);
    bool UseAppend = pDataArray -> para_UseAppend;      //G et whether to append or write
	LPRDATA rdPtr = pDataArray -> para_rdPtr;           //Get rdPtr
	delete pDataArray;                                  //Container is expendable 
	
	char * WriteType = "wb9";        //User is using write (overwriting)
	if (UseAppend) WriteType = "ab9"; //User is using append
					
	//Check input file size - no handles to close
    if (FileSizeCheck(infilename, rdPtr)) {Terminate();}

    //Open handles to both files
    FILE *infile = fopen(infilename, "rb");      //r = read, b = binary
    gzFile outfile = gzopen(outfilename, WriteType); //a = append (or w=write), b = binary, 9 = max compression
	
	//Check handles - HandleCheck() automatically closes handles if invalid
    if (HandleCheck(infile, outfile, rdPtr)) {Terminate();}

    //Declare variables
    unsigned long PreviousOutputSize = 0; //For calculation purposes
    signed int num_read = 0;
    unsigned long total_read = 0;
    ThreadSafe_Start();
    unsigned short tempinbuffersize = rdPtr -> inbuffersize;
    ThreadSafe_End();

	//If using Write, output size should not be included in the calculation later. Otherwise:
	if (UseAppend) PreviousOutputSize = file_size(outfilename); 

    //This makes sure that the buffer is the right size - if too large, set buffer smaller
    if ( file_size(infilename) < tempinbuffersize)
        tempinbuffersize = (unsigned short)file_size(infilename);
    
    //Then declare final variable, the buffer
    char *inbuffer = new char[tempinbuffersize];

    //Iteration through the files
    while ((num_read = fread(inbuffer, 1, tempinbuffersize, infile)) > 0)
    {
        total_read += num_read;
        gzwrite(outfile, inbuffer, num_read);
    }
               
    //Close thread
    fclose(infile);
    gzclose(outfile);
    delete[] inbuffer, infilename, outfilename;

	//Set variables afnter completion
    ThreadSafe_Start();
	unsigned long saveoutfilesize = file_size(outfilename);
	unsigned long saveinfilesize = file_size(infilename);
    rdPtr -> PercentageDifference = ((file_size(outfilename)-PreviousOutputSize)*(1.0/file_size(infilename)))*100.0;
    
	stringstream temp;
    temp <<"Buffer used: "
         << tempinbuffersize
         <<", total bytes read: "
         << file_size(infilename)
         <<", total bytes written: "
         << file_size(outfilename)-PreviousOutputSize
         <<", compression rate: "
         << rdPtr -> PercentageDifference
         <<"%.";
    rdPtr -> returnstring = temp.str();
    temp.flush();
    rdPtr -> LastOutput = outfilename;
    rdPtr -> rRd -> PushEvent(0); 
    ThreadSafe_End();
    return 0;
}

//Decompress a file
DWORD WINAPI decompress_one_file(Params *pDataArray)
{    
    //Open struct and set variables
	char *infilename = new char [MAX_PATH];             //Get input file name
	strcpy_s(infilename, MAX_PATH, pDataArray -> para_infilename);
    char *outfilename = new char [MAX_PATH];             //Get output file name
	strcpy_s(outfilename, MAX_PATH, pDataArray -> para_outfilename);
    bool UseAppend = pDataArray -> para_UseAppend;      //Get whether to append or write
	LPRDATA rdPtr = pDataArray -> para_rdPtr;           //Get rdPtr
	delete pDataArray;                                  //Container is expendable 
    
    char * WriteType = "wb";        //User is using write (overwriting)
	if (UseAppend) WriteType = "ab"; //User is using append

    if (FileSizeCheck(infilename, rdPtr)) {Terminate();}
    //Open handles to both files
    gzFile infile = gzopen(infilename, "rb"); //r = read, b = binary
    FILE *outfile = fopen(outfilename, WriteType); //a = append or w = write, b = binary
    
    if (HandleCheck(outfile, infile, rdPtr)) {Terminate();}
    //Declare variables
    unsigned long PreviousOutputSize = 0; //For calculation purposes
    int num_read = 0;
    unsigned long total_read = 0;
    ThreadSafe_Start();
    unsigned short tempinbuffersize = rdPtr -> inbuffersize;            
    ThreadSafe_End();
    
	//If using Write, output size should not be included in the calculation later. Otherwise:
	if (UseAppend) PreviousOutputSize = file_size(outfilename); 
    
	//This makes sure that the buffer is the right size - if too large, set buffer smaller
    if ( file_size(infilename) < tempinbuffersize)
        tempinbuffersize = (unsigned short)file_size(infilename);
    
    //Then declare final variable, the buffer
    char *inbuffer = new char[tempinbuffersize];
    
    //Iteration through the files
    while ((num_read = gzread(infile, inbuffer, tempinbuffersize)) > 0)
    {
        total_read += num_read;
        fwrite(inbuffer, 1, num_read, outfile);
    }

    //Close thread
    gzclose(infile);
    fclose(outfile);
    delete[] inbuffer, infilename, outfilename;

    //Set variables after completion
    ThreadSafe_Start();
    rdPtr -> PercentageDifference = ((file_size(outfilename)-PreviousOutputSize)*(1.0/file_size(infilename)))*100.0;
    stringstream temp;
    temp <<"Buffer used: "
         << tempinbuffersize
         <<", total bytes read: "
         << file_size(infilename)
         <<", total bytes written: "
         << file_size(outfilename)-PreviousOutputSize
         <<", decompression rate: "
         << rdPtr -> PercentageDifference
         <<"%.";
    rdPtr -> returnstring = temp.str();
    temp.flush();
    rdPtr -> LastOutput = outfilename;
    rdPtr -> rRd -> CallEvent(0);
    ThreadSafe_End();
    return 0;
}


// ============================================================================
//
// CONDITIONS
// 
// ============================================================================

CONDITION(
    /* ID */        0,
    /* Name */      "%o: On success",
    /* Flags */     0,
    /* Params */    (0)
) {
    return true;
}

CONDITION(
    /* ID */        1,
    /* Name */      "%o: On error",
    /* Flags */     0,
    /* Params */    (0)
) {
    return true;
}

// ============================================================================
//
// ACTIONS
// 
// ============================================================================

ACTION( //Old compress function
    /* ID */        0,
    /* Name */      "<<REPLACE THIS ACTION>> Compress a file (%0) --> (%1)",
    /* Flags */     0,
    /* Params */    (2, PARAM_STRING, "Input file:", PARAM_STRING, "Output file:")
) {
	//Old version of compress action: Notify user
	MessageBoxA(NULL, "Sorry to interrupt your programming, but you have\nan old \"Compress file\" action in ZlibStream which needs replacing.\nClick the ZlibStream icon in your event editor to see all the events\nZlibStream is in.", "Darkwire Software - Old Event Notice", MB_OK);
}

ACTION( //Old decompress function
    /* ID */        1,
    /* Name */      "<<REPLACE THIS ACTION>> Decompress a file (%0) --> (%1)",
    /* Flags */     0,
    /* Params */    (2, PARAM_STRING, "Input file:", PARAM_STRING, "Output file:")
) {
    //Old version of decompress action: Notify user
	MessageBoxA(NULL, "Sorry to interrupt your programming, but you have\nan old \"Decompress file\" action in ZlibStream which needs replacing.\nClick the ZlibStream icon in your event editor to see all the events\nZlibStream is in.", "Darkwire Software - Old Event Notice", MB_OK);
}

ACTION(
    /* ID */        2,
    /* Name */      "Set buffer size to %0 bytes",
    /* Flags */     0,
    /* Params */    (1, PARAM_NUMBER, "Buffer size (1 to 16384 bytes):")
) {
    int p1 = Param(TYPE_INT);
    ThreadSafe_Start();
    if (p1 > 0 && p1 < 16385)
        rdPtr -> inbuffersize = p1;
    else
    {
        rdPtr -> returnstring = "Buffer is an invalid size. Must be between 0 and 16385 (exclusive).";
        rdPtr -> rRd -> CallEvent(1);
    }
    ThreadSafe_End();
}

ACTION(
    /* ID */        3,
    /* Name */      "Compress a file (%0) --> (%1), append = %2",
    /* Flags */     0,
    /* Params */    (3, PARAM_STRING, "Input file:", PARAM_STRING, "Output file:", PARAM_NUMBER, "Append if output file already exists? (0 for overwriting, 1 for adding to end)")
) {
    //Retrieve variables from parameters
    char *p1 = (char *) Param(TYPE_STRING);
    char *p2 = (char *) Param(TYPE_STRING);
    int p3 = Param(TYPE_INT);
	
	//Set bool for simplicity
	bool temp = false;
	if (p3 != 0)
		temp = true;

    //Declare struct and set variables in struct
    Params* Parameters = new Params;                          //Declaration
	strcpy_s(Parameters -> para_infilename, MAX_PATH, p1);  //Pass the input file name
    strcpy_s(Parameters -> para_outfilename, MAX_PATH, p2); //Pass the output file name
	Parameters -> para_UseAppend = temp;                      //Pass the append command
    Parameters -> para_rdPtr = rdPtr;                         //Pass rdPtr
	
	//Create thread
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&compress_one_file, Parameters, 0, NULL);
}

ACTION(
    /* ID */        4,
    /* Name */      "Decompress a file (%0) --> (%1), append = %2",
    /* Flags */     0,
    /* Params */    (3, PARAM_STRING, "Input file:", PARAM_STRING, "Output file:", PARAM_NUMBER, "Append if output file already exists? (0 for overwriting, 1 for adding to end)")
) {
    //Retrieve variables from parameters
    char *p1 = (char *) Param(TYPE_STRING);
    char *p2 = (char *) Param(TYPE_STRING);
    int p3 = Param(TYPE_INT);
	
	//Set bool for simplicity
	bool temp = false;
	if (p3 != 0)
		temp = true;

    //Declare struct and set variables in struct
    Params* Parameters = new Params;     //Declaration
	strcpy_s(Parameters -> para_infilename, MAX_PATH, p1);  //Pass the input file name
    strcpy_s(Parameters -> para_outfilename, MAX_PATH, p2); //Pass the output file name
    Parameters -> para_UseAppend = temp;                      //Pass the append command
	Parameters -> para_rdPtr = rdPtr;                         //Pass rdPtr
	
    //Create thread
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&decompress_one_file, Parameters, 0, NULL);
}

// ============================================================================
//
// EXPRESSIONS
// 
// ============================================================================

EXPRESSION(
    /* ID */        0,
    /* Name */      "ReturnedCode(",
    /* Flags */     0,
    /* Params */    (0)
) {
    //Deprecated variable: Notify user
	MessageBoxA(NULL, "Sorry to interrupt your programming, but you have\nan old \"ReturnedCode()\" expression from ZlibStream which is removed.\nClick the ZlibStream icon in your event editor to see all the events\nZlibStream is in.", "Darkwire Software - Old Expression Notice", MB_OK);
	return false;
}

EXPRESSION(
    /* ID */        1,
    /* Name */      "ReturnedString$(",
	/* Flags */     EXPFLAG_STRING,
    /* Params */    (0)
) {
    ThreadSafe_Start();
    string temp = rdPtr -> returnstring;
    ThreadSafe_End();
    ReturnStringSafe(temp.c_str());
}

EXPRESSION(
    /* ID */        2,
    /* Name */       "BufferSize(",
    /* Flags */     0,
    /* Params */    (0)
) {
    ThreadSafe_Start();
    unsigned short temp = rdPtr -> inbuffersize;
    ThreadSafe_End();
    return temp;
}

EXPRESSION(
    /* ID */        3,
    /* Name */      "Percentage(",
    /* Flags */     EXPFLAG_DOUBLE,
    /* Params */    (0)
) {
	ThreadSafe_Start();
	double temp = rdPtr -> PercentageDifference;
    ThreadSafe_End();
	//This casts to 2 decimal places
	char sprintfdest[20];
    sprintf(sprintfdest,"%.2f", temp);
	float temp2 = (float)atof(sprintfdest);
	//Voila
    ReturnFloat(temp2);
}


EXPRESSION(
    /* ID */        4,
    /* Name */      "LastOutputFile$(",
    /* Flags */     EXPFLAG_STRING,
    /* Params */    (0)
) {
    ThreadSafe_Start();
    string temp = rdPtr -> LastOutput;
    ThreadSafe_End();
    ReturnStringSafe(temp.c_str());
}