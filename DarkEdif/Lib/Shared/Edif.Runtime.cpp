#include "Common.h"

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
	CallRunTimeFunction2(hoPtr, RFUNCTION::GENERATE_EVENT, EventID, 0);
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

bool Edif::Runtime::IsHWA()
{
#ifdef _WIN32
	return hoPtr->AdRunHeader->rh4.rh4Mv->CallFunction(NULL, CallFunctionIDs::ISHWA, 0, 0, 0) == 1;
#else
	// TODO: This is wrong
	return false;
#endif
}

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

#elif defined(__ANDROID__) // !_WIN32

Edif::Runtime::Runtime(RuntimeFunctions &runFuncs, jobject javaExtPtr2) : 
	runFuncs(runFuncs), javaExtPtr(javaExtPtr2, "Edif::Runtime::javaExtPtr from Edif::Runtime ctor"), ObjectSelection(NULL)
{
	std::string exc;
	javaExtPtrClass = global(mainThreadJNIEnv->GetObjectClass(javaExtPtr), "Extension::javaExtPtrClass from Extension ctor");
	if (javaExtPtrClass.invalid()) {
		exc = GetJavaExceptionStr();
		LOGE("Could not get javaExtPtrClass, got exception %s.", exc.c_str());
	}
	
	jfieldID javaHoField = mainThreadJNIEnv->GetFieldID(javaExtPtrClass, "ho", "LObjects/CExtension;");
	if (javaHoField == NULL) {
		exc = GetJavaExceptionStr();
		LOGE("Could not get javaHoField, got exception %s.", exc.c_str());
	}

	javaHoObject = global(mainThreadJNIEnv->GetObjectField(javaExtPtr, javaHoField), "Extension::javaHoObject from Extension ctor");
	if (javaHoObject.invalid()) {
		exc = GetJavaExceptionStr();
		LOGE("Could not get javaHoObject, got exception %s.", exc.c_str());
	}

	javaHoClass = global(mainThreadJNIEnv->GetObjectClass(javaHoObject), "Extension::javaHoClass from Extension ctor");
	if (javaHoClass.invalid()) {
		exc = GetJavaExceptionStr();
		LOGE("Could not find javaHoClass method, got exception %s.", exc.c_str());
	}
}

#define GenEdifFunction(x) \
	static jmethodID javaMethodID = threadEnv->GetMethodID(javaHoClass, x, "()V"); \
	if (javaMethodID == NULL) {\
		std::string exc = GetJavaExceptionStr(); \
		LOGE("Could not find %s method, got exception %s.", x, exc.c_str()); \
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
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.", __PRETTY_FUNCTION__);
	// TODO: Allocate missing.
	// return (void *)CallRunTimeFunction2(hoPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, size * sizeof(TCHAR));
	return NULL;
}

// Dummy functions. The conversion to Modified-UTF8 happens in JStrToCStr, inside expression return.
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
		return stackRet;
	}
	char * temp = (char *)realloc(lastHeapRet, len);
	if (temp == NULL)
	{
		free(lastHeapRet);
		lastHeapRet = NULL;
		LOGE("Ran out of memory allocating %zu bytes for string returning \"%.20s...\"!", len, String);
		strcpy(stackRet, "Out of memory! See logcat.");
		return stackRet;
	}
	strcpy(lastHeapRet, String);
	return (lastHeapRet = temp);
}
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
		LOGF("Thread ID %s already has JNI access.", ThreadIDToStr(thisThreadID).c_str());
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
		LOGF("Couldn't attach thread %s (ID %s) to JNI, AttachCurrentThread%s error %i.",
			threadName, ThreadIDToStr(thisThreadID).c_str(), asDaemon ? "AsDaemon" : "", error);
		return nullptr;
	}
	LOGV("Attached thread %s (ID %s) to JNI.", threadName, ThreadIDToStr(thisThreadID).c_str());
	JNIExceptionCheck();
	return threadEnv;
}
void Edif::Runtime::DetachJVMAccessForThisThread()
{
	const auto thisThreadID = std::this_thread::get_id();
	if (threadEnv == nullptr)
	{
		LOGF("Can't detach JVM access, thread ID %s already doesn't have JNI access", ThreadIDToStr(thisThreadID).c_str());
		return;
	}
	JNIExceptionCheck();
	const jint error = global_vm->DetachCurrentThread();
	if (error != JNI_OK)
	{
		LOGF("Couldn't detach thread ID %s from JNI, DetachCurrentThread error %i.", ThreadIDToStr(thisThreadID).c_str(), error);
		return;
	}

	threadEnv = nullptr;
}
inline JNIEnv * Edif::Runtime::GetJNIEnvForThisThread()
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
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	// CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::DRIVE, (long)Buffer);
}

void Edif::Runtime::GetApplicationDirectory(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	//CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::DIR, (long)Buffer);
}

void Edif::Runtime::GetApplicationPath(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	// CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::PATH, (long)Buffer);
}

void Edif::Runtime::GetApplicationName(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	//CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::APP_NAME, (long)Buffer);
}

void Edif::Runtime::GetApplicationTempPath(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.", __PRETTY_FUNCTION__);
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
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.", __PRETTY_FUNCTION__);
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
	runFuncs(runFuncs), ObjectSelection(NULL)
{

}


Edif::Runtime::~Runtime()
{
}

extern "C"
{
	void DarkEdif_generateEvent(void * ext, int code, int param);
	void DarkEdif_reHandle(void * ext);
}

void Edif::Runtime::Rehandle()
{
	DarkEdif_reHandle(this->objCExtPtr);
}

void Edif::Runtime::GenerateEvent(int EventID)
{
	DarkEdif_generateEvent(this->objCExtPtr, EventID, 0);
}

void Edif::Runtime::PushEvent(int EventID)
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.", __PRETTY_FUNCTION__);
}

void * Edif::Runtime::Allocate(size_t size)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.", __PRETTY_FUNCTION__);
	// TODO: Allocate missing.
	// return (void *)CallRunTimeFunction2(hoPtr, RFUNCTION::GET_STRING_SPACE_EX, 0, size * sizeof(TCHAR));
	return NULL;
}

// Dummy functions. The conversion to Modified-UTF8 happens in JStrToCStr, inside expression return.
TCHAR * Edif::Runtime::CopyString(const TCHAR * String) {
	return (TCHAR *)String;
}
char * Edif::Runtime::CopyStringEx(const char * String) {
	return (char *)String;
}
wchar_t * Edif::Runtime::CopyStringEx(const wchar_t * String) {
	return (wchar_t *)String;
}

void Edif::Runtime::Pause()
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.", __PRETTY_FUNCTION__);
}

void Edif::Runtime::Resume()
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.", __PRETTY_FUNCTION__);
}

void Edif::Runtime::Redisplay()
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.", __PRETTY_FUNCTION__);
}

void Edif::Runtime::GetApplicationDrive(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	// CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::DRIVE, (long)Buffer);
}

void Edif::Runtime::GetApplicationDirectory(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	//CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::DIR, (long)Buffer);
}

void Edif::Runtime::GetApplicationPath(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	// CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::PATH, (long)Buffer);
}

void Edif::Runtime::GetApplicationName(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
	//CallRunTimeFunction2(hoPtr, RFUNCTION::GET_FILE_INFOS, (WPARAM)FILEINFO::APP_NAME, (long)Buffer);
}

void Edif::Runtime::GetApplicationTempPath(TCHAR * Buffer)
{
	LOGF("Function %s not implemented in DarkEdif Android, and cannot be called.", __PRETTY_FUNCTION__);
	// TODO: This is wrong, but not crucial.
}

void Edif::Runtime::Redraw()
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.", __PRETTY_FUNCTION__);
}

void Edif::Runtime::Destroy()
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.", __PRETTY_FUNCTION__);
}

void Edif::Runtime::CallMovement(int ID, long Parameter)
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.", __PRETTY_FUNCTION__);
	// See ~line 592, CExtension.java
	// GenEdifFunction("redisplay");
	//CallRunTimeFunction2(hoPtr, RFUNCTION::CALL_MOVEMENT, ID, Parameter);
}

void Edif::Runtime::SetPosition(int X, int Y)
{
	LOGF("Function %s not implemented in DarkEdif iOS, and cannot be called.", __PRETTY_FUNCTION__);
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
