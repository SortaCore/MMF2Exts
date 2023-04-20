#include "Common.h"
#include "Edif.h"
#include "DarkEdif.h"

struct EdifGlobal
{
	TCHAR name[256];
	void * Value;

	EdifGlobal * Next;
};

#ifdef _WIN32
Edif::Runtime::Runtime(HeaderObject * _hoPtr) : hoPtr(_hoPtr),
ObjectSelection(hoPtr->AdRunHeader)
{
}

Edif::Runtime::~Runtime()
{
}

void Edif::Runtime::Rehandle()
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::REHANDLE, 0, 0);
}

void Edif::Runtime::GenerateEvent(int EventID)
{
	// Saving tokens allows events to be run from inside expressions
	// https://community.clickteam.com/threads/108993-Application-crashed-in-some-cases-when-calling-an-event-via-expression?p=769763&viewfull=1#post769763
	// As of CF2.5 build 293.9, this is done by runtime anyway, but if you want to
	// support Fusion 2.0, and you're generating events from expressions, you should include this
#ifdef DARKSCRIPT_EXTENSION
	auto pRh = hoPtr->AdRunHeader;
	expression*	saveExpToken = pRh->rh4.ExpToken;
#endif
	CallRunTimeFunction2(hoPtr, RFUNCTION::GENERATE_EVENT, EventID, 0);
#ifdef DARKSCRIPT_EXTENSION
	pRh->rh4.ExpToken = saveExpToken;
#endif
}

void Edif::Runtime::PushEvent(int EventID)
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::PUSH_EVENT, EventID, 0);
}

void * Edif::Runtime::Allocate(size_t size)
{
    return (void *) CallRunTimeFunction2(hoPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, size * sizeof(TCHAR));
}

TCHAR * Edif::Runtime::CopyString(const TCHAR * String)
{
	TCHAR * New = NULL;
    New = (TCHAR *) Allocate(_tcslen(String) + 1);
    _tcscpy(New, String);

    return New;
}

char * Edif::Runtime::CopyStringEx(const char * String)
{
	char * New = NULL;
	New = (char *)CallRunTimeFunction2(hoPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, (strlen(String) + 1) * sizeof(char));
	strcpy(New, String);

	return New;
}


wchar_t * Edif::Runtime::CopyStringEx(const wchar_t * String)
{
	wchar_t * New = NULL;
	New = (wchar_t *)CallRunTimeFunction2(hoPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, (wcslen(String) + 1) * sizeof(wchar_t));
	wcscpy(New, String);

	return New;
}


void Edif::Runtime::Pause()
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::PAUSE, 0, 0);
}

void Edif::Runtime::Resume()
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::CONTINUE, 0, 0);
}

void Edif::Runtime::Redisplay()
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::REDISPLAY, 0, 0);
}

void Edif::Runtime::GetApplicationDrive(TCHAR * Buffer)
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::DRIVE, (long) Buffer);
}

void Edif::Runtime::GetApplicationDirectory(TCHAR * Buffer)
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::DIR, (long) Buffer);
}

void Edif::Runtime::GetApplicationPath(TCHAR * Buffer)
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::PATH, (long) Buffer);
}

void Edif::Runtime::GetApplicationName(TCHAR * Buffer)
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::APP_NAME, (long) Buffer);
}

void Edif::Runtime::GetApplicationTempPath(TCHAR * Buffer)
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::TEMP_PATH, (long) Buffer);
}

void Edif::Runtime::Redraw()
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::REDRAW, 0, 0);
}

void Edif::Runtime::Destroy()
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::DESTROY, 0, 0);
}

void Edif::Runtime::ExecuteProgram(ParamProgram * Program)
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::EXECUTE_PROGRAM, 0, (long) Program);
}

long Edif::Runtime::EditInteger(EditDebugInfo * EDI)
{
    return CallRunTimeFunction2(hoPtr, RFUNCTION::EDIT_INT, 0, (long) EDI);
}

long Edif::Runtime::EditText(EditDebugInfo * EDI)
{
    return CallRunTimeFunction2(hoPtr, RFUNCTION::EDIT_TEXT, 0, (long) EDI);
}

void Edif::Runtime::CallMovement(int ID, long Parameter)
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::CALL_MOVEMENT, ID, Parameter);
}

void Edif::Runtime::SetPosition(int X, int Y)
{
	CallRunTimeFunction2(hoPtr, RFUNCTION::SET_POSITION, X, Y);
}

CallTables * Edif::Runtime::GetCallTables()
{
    return (CallTables *)CallRunTimeFunction2(hoPtr, RFUNCTION::GET_CALL_TABLES, 0, 0);
}

#ifdef _WIN32
bool Edif::Runtime::IsHWACapableRuntime()
{
	// The old IsHWA() function. This returns true if it's a HWA-capable runtime, even if
	// the app is using Standard display mode. This function is necessary as HWA runtimes still allocate
	// extra RAM for the unused HWA features, moving the offsets and sizes of graphics-related classes
	//
	// Consider using GetAppDisplayMode() >= SurfaceDriver::Direct3D9 for "HWA with shaders" detection.
	// Note Direct3D8 is greater than Direct3D9.

	return hoPtr->AdRunHeader->rh4.rh4Mv->CallFunction(NULL, CallFunctionIDs::ISHWA, 0, 0, 0) == 1;
}

SurfaceDriver Edif::Runtime::GetAppDisplayMode()
{
	static SurfaceDriver sd = SurfaceDriver::Max;
	if (sd == SurfaceDriver::Max)
	{
		// Using GAOF_XX flags
		//constexpr int GAOF_DDRAW = 0x0002;
		//constexpr int GAOF_DDRAWVRAM = 0x0004;
		//constexpr int GAOF_D3D9 = 0x4000;
		//constexpr int GAOF_D3D8 = 0x8000;
		constexpr int GAOF_DDRAWEITHER = GAOF_DDRAW | GAOF_DDRAWVRAM;
		constexpr int GAOF_D3D11 = GAOF_D3D9 | GAOF_D3D8;

		// No need to find parent-most app; subapps have same OtherFlags
		const int i = hoPtr->AdRunHeader->App->hdr.OtherFlags;

		if ((i & GAOF_DDRAWEITHER) != 0)
			sd = SurfaceDriver::DirectDraw;
		else if ((i & GAOF_D3D11) == GAOF_D3D11)
			sd = SurfaceDriver::Direct3D11;
		else if ((i & GAOF_D3D8) != 0)
			sd = SurfaceDriver::Direct3D8;
		else if ((i & GAOF_D3D9) != 0)
			sd = SurfaceDriver::Direct3D9;
		else if ((i & (GAOF_D3D11 | GAOF_DDRAWEITHER)) == 0)
			sd = SurfaceDriver::Bitmap;
		else
			throw std::runtime_error("Unrecognised display mode");
	}
	return sd;
}
#endif // _WIN32

bool Edif::Runtime::IsUnicode()
{
#ifdef _WIN32
	return hoPtr->AdRunHeader->rh4.rh4Mv->CallFunction(NULL, CallFunctionIDs::ISUNICODE, 0, 0, 0) == 1;
#else
	// TODO: This is wrong
	return false;
#endif
}

event2 &Edif::Runtime::CurrentEvent()
{
    return *(event2 *) (((char *) param1) - CND_SIZE);
}

RunObject * Edif::Runtime::RunObjPtrFromFixed(int fixedvalue)
{
	objectsList * objList = hoPtr->AdRunHeader->ObjectList;
	int index = fixedvalue & 0x0000FFFF;

	if (index < 0 || index >= hoPtr->AdRunHeader->MaxObjects)
		return NULL;

	RunObject * theObject = (RunObject *)objList[index].oblOffset;

	if (theObject == NULL)
		return NULL;
	else if (FixedFromRunObjPtr(theObject) != fixedvalue)
		return NULL;

	return theObject;
}

int Edif::Runtime::FixedFromRunObjPtr(RunObject * object)
{
	if (object != NULL)
		return (object->roHo.CreationId << 16) + object->roHo.Number;
	return 0;
}

extern HINSTANCE hInstLib;

void Edif::Runtime::WriteGlobal(const TCHAR * name, void * Value)
{
    RunHeader * rhPtr = hoPtr->AdRunHeader;

	while (rhPtr->App->ParentApp)
		rhPtr = rhPtr->App->ParentApp->Frame->rhPtr;

    EdifGlobal * Global = (EdifGlobal *) rhPtr->rh4.rh4Mv->GetExtUserData(rhPtr->App, hInstLib);

    if (!Global)
    {
        Global = new EdifGlobal;

        _tcscpy(Global->name, name);
        Global->Value = Value;

        Global->Next = 0;

        rhPtr->rh4.rh4Mv->SetExtUserData(rhPtr->App, hInstLib, Global);

        return;
    }

    while (Global)
    {
        if (!_tcsicmp(Global->name, name))
        {
            Global->Value = Value;
            return;
        }

        if (!Global->Next)
            break;

        Global = Global->Next;
    }

    Global->Next = new EdifGlobal;
    Global = Global->Next;

    _tcscpy(Global->name, name);

    Global->Value = Value;
    Global->Next = 0;
}

void * Edif::Runtime::ReadGlobal(const TCHAR * name)
{
    RunHeader * rhPtr = hoPtr->AdRunHeader;

	while (rhPtr->App->ParentApp)
		rhPtr = rhPtr->App->ParentApp->Frame->rhPtr;

    EdifGlobal * Global = (EdifGlobal *) rhPtr->rh4.rh4Mv->GetExtUserData(rhPtr->App, hInstLib);

    while (Global)
    {
        if (!_tcsicmp(Global->name, name))
            return Global->Value;

        Global = Global->Next;
    }

    return NULL;
}

#else  // !_WIN32

// We use this in both Android and iOS. It uses stack memory for text up to 1kb,
// and heap memory for anything else, reusing the memory for every string expression.
//
// This would be considered unsafe in nested A/C/E scenarios,
// i.e. expression("3rd party object", expression1("this obj"), expression2("this obj"))
// but the Obj-C/Java wrapper has to copy out the UTF-8 text anyway, so we don't have
// to worry about expression1 becoming 2 or memory freed too early.

static char * lastHeapRet;
static char stackRet[1024];
static char zero[4];
TCHAR * Edif::Runtime::CopyString(const TCHAR * String) {
	if (!String[0])
		return zero;

	const size_t len = strlen(String) + 1;
	if (len < sizeof(stackRet))
	{
		strcpy(stackRet, String);
		LOGV("Returning text on stack: %p moved to stack %p \"%s\".\n", String, stackRet, stackRet);
		return stackRet;
	}
	char * temp = (char *)realloc(lastHeapRet, len);
	if (temp == NULL)
	{
		free(lastHeapRet);
		lastHeapRet = NULL;
		LOGE("Ran out of memory allocating %zu bytes for string returning \"%.20s...\"!\n", len, String);
		strcpy(stackRet, "Out of memory! See logcat.");
		return stackRet;
	}
	LOGV("Returning text on heap: %p moved to heap %p \"%s\".\n", String, temp, temp);
	strcpy(temp, String);
	return (lastHeapRet = temp);
}

#if defined(__ANDROID__)

Edif::Runtime::Runtime(RuntimeFunctions &runFuncs, jobject javaExtPtr2) :
	runFuncs(runFuncs), javaExtPtr(javaExtPtr2, "Edif::Runtime::javaExtPtr from Edif::Runtime ctor"), ObjectSelection(NULL)
{
	std::string exc;
	javaExtPtrClass = global(mainThreadJNIEnv->GetObjectClass(javaExtPtr), "Extension::javaExtPtrClass from Extension ctor");
	if (javaExtPtrClass.invalid()) {
		exc = GetJavaExceptionStr();
		LOGE("Could not get javaExtPtrClass, got exception %s.\n", exc.c_str());
	}

	jfieldID javaHoField = mainThreadJNIEnv->GetFieldID(javaExtPtrClass, "ho", "LObjects/CExtension;");
	if (javaHoField == NULL) {
		exc = GetJavaExceptionStr();
		LOGE("Could not get javaHoField, got exception %s.\n", exc.c_str());
	}

	javaHoObject = global(mainThreadJNIEnv->GetObjectField(javaExtPtr, javaHoField), "Extension::javaHoObject from Extension ctor");
	if (javaHoObject.invalid()) {
		exc = GetJavaExceptionStr();
		LOGE("Could not get javaHoObject, got exception %s.\n", exc.c_str());
	}

	javaHoClass = global(mainThreadJNIEnv->GetObjectClass(javaHoObject), "Extension::javaHoClass from Extension ctor");
	if (javaHoClass.invalid()) {
		exc = GetJavaExceptionStr();
		LOGE("Could not find javaHoClass method, got exception %s.\n", exc.c_str());
	}
}

#define GenEdifFunction(x) \
	static jmethodID javaMethodID = threadEnv->GetMethodID(javaHoClass, x, "()V"); \
	if (javaMethodID == NULL) {\
		std::string exc = GetJavaExceptionStr(); \
		LOGE("Could not find %s method, got exception %s.\n", x, exc.c_str()); \
	} \
	else \
		threadEnv->CallVoidMethod(javaHoObject, javaMethodID);

Edif::Runtime::~Runtime()
{
}

extern thread_local JNIEnv * threadEnv;
void Edif::Runtime::Rehandle()
{
	static jmethodID javaMethodID = threadEnv->GetMethodID(javaHoClass.ref, "reHandle", "()V");
	threadEnv->CallVoidMethod(javaHoObject.ref, javaMethodID);
	// GenEdifFunction reHandle
}

void Edif::Runtime::GenerateEvent(int EventID)
{
	static jmethodID javaMethodID = threadEnv->GetMethodID(javaHoClass.ref, "generateEvent", "(II)V");
	threadEnv->CallVoidMethod(javaHoObject, javaMethodID, EventID, 0);
	//runFuncs.generateEvent(javaHoObject, EventID, 0);
}

void Edif::Runtime::PushEvent(int EventID)
{
	jmethodID javaMethodID = threadEnv->GetMethodID(javaHoClass, "pushEvent", "(II)V");
	threadEnv->CallVoidMethod(javaHoObject, javaMethodID, EventID, 0);
}

void * Edif::Runtime::Allocate(size_t size)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: Allocate missing.
	// return (void *)CallRunTimeFunction2(hoPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, size * sizeof(TCHAR));
	return NULL;
}

// Dummy functions. The conversion to Modified-UTF8 happens in JStrToCStr, inside Edif::Expression's return.
char * Edif::Runtime::CopyStringEx(const char * String) {
	return CopyString(String);
}
wchar_t * Edif::Runtime::CopyStringEx(const wchar_t * String) {
	throw std::runtime_error("Do not use wchar_t in Android!");
	return (wchar_t *)String;
}

JNIEnv * Edif::Runtime::AttachJVMAccessForThisThread(const char * threadName, bool asDaemon)
{
	const auto thisThreadID = std::this_thread::get_id();
	if (threadEnv != nullptr)
	{
		LOGF("Thread ID %s already has JNI access.\n", ThreadIDToStr(thisThreadID).c_str());
		return nullptr;
	}

	pthread_setname_np(pthread_self(), threadName);

	JavaVMAttachArgs args = {
		.name = threadName,
		.group = NULL,
		.version = JNI_VERSION_1_6
	};

	// Daemon means the JVM won't keep the app running if this thread is still alive.
	// Do you want main thread exiting to choose whether the app is running or not?
	jint error;
	if (asDaemon)
		error = global_vm->AttachCurrentThreadAsDaemon(&threadEnv, &args);
	else
		error = global_vm->AttachCurrentThread(&threadEnv, &args);
	if (error != JNI_OK)
	{
		LOGF("Couldn't attach thread %s (ID %s) to JNI, AttachCurrentThread%s error %i.\n",
			threadName, ThreadIDToStr(thisThreadID).c_str(), asDaemon ? "AsDaemon" : "", error);
		return nullptr;
	}
	LOGV("Attached thread %s (ID %s) to JNI.\n", threadName, ThreadIDToStr(thisThreadID).c_str());
	JNIExceptionCheck();
	return threadEnv;
}
void Edif::Runtime::DetachJVMAccessForThisThread()
{
	const auto thisThreadID = std::this_thread::get_id();
	if (threadEnv == nullptr)
	{
		LOGF("Can't detach JVM access, thread ID %s already doesn't have JNI access.\n", ThreadIDToStr(thisThreadID).c_str());
		return;
	}
	JNIExceptionCheck();
	const jint error = global_vm->DetachCurrentThread();
	if (error != JNI_OK)
	{
		LOGF("Couldn't detach thread ID %s from JNI, DetachCurrentThread error %i.\n", ThreadIDToStr(thisThreadID).c_str(), error);
		return;
	}
	LOGV("Detached thread ID %s from JNI OK.\n", ThreadIDToStr(thisThreadID).c_str());

	threadEnv = nullptr;
}
JNIEnv * Edif::Runtime::GetJNIEnvForThisThread()
{
	return threadEnv;
}


void Edif::Runtime::Pause()
{
	GenEdifFunction("pause");
}

void Edif::Runtime::Resume()
{
	GenEdifFunction("resume");
}

void Edif::Runtime::Redisplay()
{
	GenEdifFunction("redisplay");
}

void Edif::Runtime::GetApplicationDrive(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	// CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::DRIVE, (long)Buffer);
}

void Edif::Runtime::GetApplicationDirectory(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	//CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::DIR, (long)Buffer);
}

void Edif::Runtime::GetApplicationPath(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	// CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::PATH, (long)Buffer);
}

void Edif::Runtime::GetApplicationName(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	//CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::APP_NAME, (long)Buffer);
}

void Edif::Runtime::GetApplicationTempPath(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
}

void Edif::Runtime::Redraw()
{
	GenEdifFunction("redraw");
}

void Edif::Runtime::Destroy()
{
	GenEdifFunction("destroy");
}

void Edif::Runtime::CallMovement(int ID, long Parameter)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// See ~line 592, CExtension.java
	// GenEdifFunction("redisplay");
	//CallRunTimeFunction2(hoPtr, RFUNCTION::CALL_MOVEMENT, ID, Parameter);
}

void Edif::Runtime::SetPosition(int X, int Y)
{
	jmethodID javaMethodID = threadEnv->GetMethodID(javaExtPtrClass, "setPosition", "(II)V");
	threadEnv->CallVoidMethod(javaExtPtr, javaMethodID, X, Y);
}

static EdifGlobal * staticEdifGlobal; // LB says static/global values are functionally equivalent to getUserExtData, so... yay.

void Edif::Runtime::WriteGlobal(const TCHAR * name, void * Value)
{
	EdifGlobal * Global = (EdifGlobal *)staticEdifGlobal;

	if (!Global)
	{
		Global = new EdifGlobal;

		_tcscpy(Global->name, name);
		Global->Value = Value;
		Global->Next = NULL;

		staticEdifGlobal = Global;

		return;
	}

	while (Global)
	{
		if (!_tcsicmp(Global->name, name))
		{
			Global->Value = Value;
			return;
		}

		if (!Global->Next)
			break;

		Global = Global->Next;
	}

	Global->Next = new EdifGlobal;
	Global = Global->Next;

	_tcscpy(Global->name, name);

	Global->Value = Value;
	Global->Next = 0;
}

/*
// This code may be used by globals, if we ever re-integrate JNI method with getStorage and whatnot
static char * GetObjectNameWithPackage(const char * name)
{
	static bool gotten = false;
	static std::string packageName = "";
	if (!gotten)
	{
		jclass javaExtPtrClass = threadEnv->GetObjectClass(javaExtPtr);
		jmethodID getClassNameMethod = threadEnv->GetMethodID(javaExtPtrClass, "getName", "()Ljava/lang/String;");
		jstring className = (jstring)threadEnv->CallObjectMethod(javaExtPtrClass, getClassNameMethod);
		const char * classNameCPtr = threadEnv->GetStringUTFChars(className, NULL);
		packageName = classNameCPtr;
		threadEnv->ReleaseStringUTFChars(className, classNameCPtr);
		gotten = true;
	}

	static char newName[256];
	sprintf(newName, "L%s/%s;", packageName.c_str(), name);
	return newName;
}*/

void * Edif::Runtime::ReadGlobal(const TCHAR * name)
{
	/*
	// Access CRun from CRunExtension::rh
	// Access CRunApp from CRun::rhApp
	// Access CRunApp from CRunApp::parentApp
	// Access CRun again from CRunApp::run

	jclass javaExtPtrClass = threadEnv->GetObjectClass(javaExtPtr);
	jclass CRunClass = threadEnv->FindClass(GetObjectNameWithPackage("CRun"));
	jclass CRunAppClass = threadEnv->FindClass(GetObjectNameWithPackage("CRunApp"));

	jfieldID CRunExtension_rh_fieldID = threadEnv->GetFieldID(javaExtPtrClass, "rh", GetObjectNameWithPackage("CRun"));
	jfieldID CRun_rhApp_fieldID = threadEnv->GetFieldID(CRunClass, "rhApp", GetObjectNameWithPackage("CRunApp"));
	jfieldID CRunApp_parentApp_fieldID = threadEnv->GetFieldID(CRunAppClass, "parentApp", GetObjectNameWithPackage("CRunApp"));
	jfieldID CRunApp_run_fieldID = threadEnv->GetFieldID(CRunAppClass, "run", GetObjectNameWithPackage("CRunApp"));

	jobject cRun = threadEnv->GetObjectField(javaExtPtr, CRunExtension_rh_fieldID);
	jobject cRunApp = threadEnv->GetObjectField(javaExtPtr, CRun_rhApp_fieldID);

	for (jobject cRunAppTemp = cRunApp; cRunAppTemp != NULL; )
	{
		cRunApp = cRunAppTemp;
		cRun = threadEnv->GetObjectField(cRunApp, CRunApp_run_fieldID);

		cRunAppTemp = threadEnv->GetObjectField(cRunApp, CRunApp_parentApp_fieldID);
	}
	char methodParams[256];

	sprintf(methodParams, "(%sI)V", GetObjectNameWithPackage("EdifGlobal"));
	jmethodID CRun_addStorage_methodID = threadEnv->GetMethodID(CRunClass, "addStorage", methodParams);

	sprintf(methodParams, "(I)%s", GetObjectNameWithPackage("EdifGlobal"));
	jmethodID CRun_getStorage_methodID = threadEnv->GetMethodID(CRunClass, "getStorage", methodParams);

	jmethodID CRun_delStorage_methodID = threadEnv->GetMethodID(CRunClass, "delStorage", "(I)V");

	int id = 'EDIF';

	jobject edifData = threadEnv->CallObjectMethod(cRun, CRun_getStorage_methodID, id);
	*/
	EdifGlobal * Global = staticEdifGlobal;

	while (Global)
	{
		if (!_tcsicmp(Global->name, name))
			return Global->Value;

		Global = Global->Next;
	}

	return NULL;
}

#else // iOS
Edif::Runtime::Runtime(RuntimeFunctions & runFuncs, void * objCExtPtr) :
	runFuncs(runFuncs), objCExtPtr(objCExtPtr), ObjectSelection(NULL)
{

}


Edif::Runtime::~Runtime()
{
}

extern "C" void DarkEdifObjCFunc(PROJECT_NAME_RAW, generateEvent)(void * ext, int code, int param);
extern "C" void DarkEdifObjCFunc(PROJECT_NAME_RAW, reHandle)(void * ext);

void Edif::Runtime::Rehandle()
{
	DarkEdifObjCFunc(PROJECT_NAME_RAW, reHandle)(this->objCExtPtr);
}

void Edif::Runtime::GenerateEvent(int EventID)
{
	DarkEdifObjCFunc(PROJECT_NAME_RAW, generateEvent)(this->objCExtPtr, EventID, 0);
}

void Edif::Runtime::PushEvent(int EventID)
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.\n", __PRETTY_FUNCTION__);
}

void * Edif::Runtime::Allocate(size_t size)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: Allocate missing.
	// return (void *)CallRunTimeFunction2(hoPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, size * sizeof(TCHAR));
	return NULL;
}

// Dummy functions.
char * Edif::Runtime::CopyStringEx(const char * String) {
	return CopyString(String);
}
wchar_t * Edif::Runtime::CopyStringEx(const wchar_t * String) {
	throw std::runtime_error("Do not use wchar_t in iOS!");
	return (wchar_t *)String;
}

void Edif::Runtime::Pause()
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.\n", __PRETTY_FUNCTION__);
}

void Edif::Runtime::Resume()
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.\n", __PRETTY_FUNCTION__);
}

void Edif::Runtime::Redisplay()
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.\n", __PRETTY_FUNCTION__);
}

void Edif::Runtime::GetApplicationDrive(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	// CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::DRIVE, (long)Buffer);
}

void Edif::Runtime::GetApplicationDirectory(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	//CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::DIR, (long)Buffer);
}

void Edif::Runtime::GetApplicationPath(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	// CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::PATH, (long)Buffer);
}

void Edif::Runtime::GetApplicationName(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	//CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::APP_NAME, (long)Buffer);
}

void Edif::Runtime::GetApplicationTempPath(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.\n", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
}

void Edif::Runtime::Redraw()
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.\n", __PRETTY_FUNCTION__);
}

void Edif::Runtime::Destroy()
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.\n", __PRETTY_FUNCTION__);
}

void Edif::Runtime::CallMovement(int ID, long Parameter)
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.\n", __PRETTY_FUNCTION__);
	// See ~line 592, CExtension.java
	// GenEdifFunction("redisplay");
	//CallRunTimeFunction2(hoPtr, RFUNCTION::CALL_MOVEMENT, ID, Parameter);
}

void Edif::Runtime::SetPosition(int X, int Y)
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.\n", __PRETTY_FUNCTION__);
}

static EdifGlobal * staticEdifGlobal; // LB says static/global values are functionally equivalent to getUserExtData, so... yay.

void Edif::Runtime::WriteGlobal(const TCHAR * name, void * Value)
{
	EdifGlobal * Global = (EdifGlobal *)staticEdifGlobal;

	if (!Global)
	{
		Global = new EdifGlobal;

		_tcscpy(Global->name, name);
		Global->Value = Value;
		Global->Next = NULL;

		staticEdifGlobal = Global;

		return;
	}

	while (Global)
	{
		if (!_tcsicmp(Global->name, name))
		{
			Global->Value = Value;
			return;
		}

		if (!Global->Next)
			break;

		Global = Global->Next;
	}

	Global->Next = new EdifGlobal;
	Global = Global->Next;

	_tcscpy(Global->name, name);

	Global->Value = Value;
	Global->Next = 0;
}

void * Edif::Runtime::ReadGlobal(const TCHAR * name)
{
	EdifGlobal * Global = staticEdifGlobal;

	while (Global)
	{
		if (!_tcsicmp(Global->name, name))
			return Global->Value;

		Global = Global->Next;
	}

	return NULL;
}

#endif

#endif // !_WIN32
