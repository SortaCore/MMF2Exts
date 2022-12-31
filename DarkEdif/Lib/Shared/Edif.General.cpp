// ============================================================================
// Edif General:
// The following routines are used internally by Fusion, and should not need to
// be modified.
// ============================================================================

#include "Common.h"
#include "DarkEdif.h"

#ifdef _WIN32

// ============================================================================
// LIBRARY ENTRY & QUIT POINTS
// ============================================================================
HINSTANCE hInstLib = NULL;

// Entry point for DLL, when DLL is attached to by Fusion, or detached; or threads attach/detach
__declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE hDLL, std::uint32_t dwReason, LPVOID lpReserved)
{
	// DLL is attaching to the address space of the current process.
	if (dwReason == DLL_PROCESS_ATTACH && hInstLib == NULL)
	{
		hInstLib = hDLL; // Store HINSTANCE
		// no thread attach/detach for dynamic CRT, due to a small memory loss
		#ifdef _DLL
			DisableThreadLibraryCalls(hDLL);
		#endif
	}

	return TRUE;
}

// Called when the extension is loaded into memory.
// DarkEdif users shouldn't need to modify this function.
int FusionAPI Initialize(mv *mV, int quiet)
{
#pragma DllExportHint
	return Edif::Init(mV, quiet != FALSE);
}

// The counterpart of Initialize(). Called just before freeing the DLL.
// DarkEdif users shouldn't need to modify this function.
int FusionAPI Free(mv *mV)
{
#pragma DllExportHint
	// Edif is singleton, so no clean-up needed

	// But if the update checker thread is running, we don't want it to try to write to memory it can't access.
#if USE_DARKEDIF_UPDATE_CHECKER
	extern HANDLE updateThread;
	if (updateThread != NULL && WaitForSingleObject(updateThread, 3000) == WAIT_TIMEOUT)
		TerminateThread(updateThread, 2);
#endif
	return 0; // No error
}


// ============================================================================
// GENERAL INFO
// ============================================================================

// Routine called for each object when the object is read from the MFA file (edit time)
// or from the CCN or EXE file (run time).
// DarkEdif users shouldn't need to modify this function.
int FusionAPI LoadObject(mv * mV, const char * fileName, EDITDATA * edPtr, int reserved)
{
#pragma DllExportHint
	Edif::Init(mV, edPtr);
	return 0;
}

// The counterpart of LoadObject(): called just before the object is
// deleted from the frame.
// DarkEdif users shouldn't need to modify this function.
void FusionAPI UnloadObject(mv * mV, EDITDATA * edPtr, int reserved)
{
	#pragma DllExportHint
}


const TCHAR ** Dependencies = 0;

const TCHAR ** FusionAPI GetDependencies()
{
#pragma DllExportHint
	if (!Dependencies)
	{
		const json_value &DependenciesJSON = SDK->json["Dependencies"];

		Dependencies = new const TCHAR * [DependenciesJSON.u.object.length + 2];

		int Offset = 0;

		if (Edif::ExternalJSON)
		{
			TCHAR JSONFilename [MAX_PATH];

			GetModuleFileName (hInstLib, JSONFilename, sizeof (JSONFilename) / sizeof(*JSONFilename));

			TCHAR * Iterator = JSONFilename + _tcslen(JSONFilename) - 1;

			while(*Iterator != _T('.'))
				-- Iterator;

			_tcscpy(++ Iterator, _T("json"));

			Iterator = JSONFilename + _tcslen(JSONFilename) - 1;

			while(*Iterator != _T('\\') && *Iterator != _T('/'))
				-- Iterator;

			Dependencies [Offset ++] = ++ Iterator;
		}

		std::uint32_t i = 0;

		for(; i < DependenciesJSON.u.object.length; ++ i)
		{
			TCHAR* tstr = Edif::ConvertString(DependenciesJSON[i]);
			Dependencies[Offset + i] = tstr;
			Edif::FreeString(tstr);
		}

		Dependencies[Offset + i] = 0;
	}

	return Dependencies;
}

/// <summary> Called every time the extension is being created from nothing.
///			  Default property contents should be loaded from JSON. </summary>
std::int16_t FusionAPI GetRunObjectInfos(mv * mV, kpxRunInfos * infoPtr)
{
#pragma DllExportHint
	infoPtr->Conditions = &::SDK->ConditionJumps[0];
	infoPtr->Actions = &::SDK->ActionJumps[0];
	infoPtr->Expressions = &::SDK->ExpressionJumps[0];

	infoPtr->NumOfConditions = CurLang["Conditions"].u.object.length;
	infoPtr->NumOfActions = CurLang["Actions"].u.object.length;
	infoPtr->NumOfExpressions = CurLang["Expressions"].u.object.length;
#ifdef DARKSCRIPT_EXTENSION
	infoPtr->NumOfExpressions = Extension::GetNumExpressions();
#endif

	static unsigned short EDITDATASize = 0;
	if (EDITDATASize == 0)
	{
#ifdef NOPROPS
		EDITDATASize = sizeof(EDITDATA);
#else 
		EDITDATASize = DarkEdif::DLL::Internal_GetEDITDATASizeFromJSON();
#endif // NOPROPS
	}
	infoPtr->EDITDATASize = EDITDATASize;

	infoPtr->WindowProcPriority = Extension::WindowProcPriority;

	infoPtr->EditFlags = Extension::OEFLAGS;
	infoPtr->EditPrefs = Extension::OEPREFS;

	memcpy(&infoPtr->Identifier, ::SDK->json["Identifier"], 4);

	infoPtr->Version = Extension::Version;

	return TRUE;
}

std::uint32_t FusionAPI GetInfos(int info)
{
#pragma DllExportHint
	switch ((KGI)info)
	{
		case KGI::VERSION:
			return 0x300; // I'm a MMF2 extension!

		case KGI::PLUGIN:
			return 0x100; // Version 1 type o' plugin

		case KGI::PRODUCT:
			#ifdef PROEXT
				return 3; // MMF Developer
			#endif
			#ifdef TGFEXT
				return 1; // TGF2
			#endif
			return 2;		// MMF Standard

		case KGI::BUILD:
			return Extension::MinimumBuild;

	#ifdef _UNICODE
		case KGI::UNICODE_:
			return TRUE;	// I'm building in Unicode
	#endif

		default:
			return 0;
	}
}

std::int16_t FusionAPI CreateRunObject(RUNDATA * rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr)
{
#pragma DllExportHint
	/* Global to all extensions! Use the constructor of your Extension class (Extension.cpp) instead! */

	rdPtr->pExtension = new Extension(rdPtr, edPtr, cobPtr);
	rdPtr->pExtension->Runtime.ObjectSelection.pExtension = rdPtr->pExtension;

	return 0;
}

/* Don't touch any of these, they're global to all extensions! See Extension.cpp */

std::int16_t FusionAPI DestroyRunObject(RUNDATA * rdPtr, long fast)
{
#pragma DllExportHint
	delete rdPtr->pExtension;
	rdPtr->pExtension = NULL;

	return 0;
}

REFLAG FusionAPI HandleRunObject(RUNDATA * rdPtr)
{
#pragma DllExportHint
	return rdPtr->pExtension->Handle();
}

#ifdef VISUAL_EXTENSION

REFLAG FusionAPI DisplayRunObject(RUNDATA * rdPtr)
{
#pragma DllExportHint
	return rdPtr->pExtension->Display();
}

#endif

std::uint16_t FusionAPI GetRunObjectDataSize(RunHeader * rhPtr, EDITDATA * edPtr)
{
#pragma DllExportHint
	return (sizeof(RUNDATA));
}


std::int16_t FusionAPI PauseRunObject(RUNDATA * rdPtr)
{
#pragma DllExportHint
	// Note: PauseRunObject is required, or runtime will crash when pausing.
	return rdPtr->pExtension->FusionRuntimePaused();
}

std::int16_t FusionAPI ContinueRunObject(RUNDATA * rdPtr)
{
#pragma DllExportHint
	return rdPtr->pExtension->FusionRuntimeContinued();
}



#elif defined(__ANDROID__)

ProjectFunc jint getNumberOfConditions(JNIEnv *, jobject, jlong cptr)
{
	//raise(SIGTRAP);
	return CurLang["Conditions"].u.array.length;
}
typedef jobject ByteBufferDirect;
typedef jobject CCreateObjectInfo;

static RuntimeFunctions runFuncs;
ProjectFunc jlong createRunObject(JNIEnv * env, jobject javaExtPtr, ByteBufferDirect edPtr, CCreateObjectInfo coi, jint version)
{
	void * edPtrReal = mainThreadJNIEnv->GetDirectBufferAddress(edPtr);
	LOGI("Note: mainThreadJNIEnv is %p, env is %p; javaExtPtr is %p, edPtr %p, edPtrReal %p, coi %p.\n", mainThreadJNIEnv, env, javaExtPtr, edPtr, edPtrReal, coi);
	global<jobject> javaExtP(javaExtPtr, "createRunObject javaExtPtr");
	runFuncs.ext = javaExtP;

	Extension * ext = new Extension(runFuncs, (EDITDATA *)edPtrReal, javaExtPtr);
	runFuncs.ext = ext->javaExtPtr; // this is global so it's safer
	ext->Runtime.ObjectSelection.pExtension = ext;
	return (jlong)ext;
}

ProjectFunc void destroyRunObject(JNIEnv *, jobject, jlong ext, jboolean fast)
{
	JNIExceptionCheck();
	LOGV("Running " PROJECT_NAME " extension dtor in destroyRunObject...\n");
	delete ((Extension *)ext);
	JNIExceptionCheck();
	LOGV("Ran " PROJECT_NAME " extension dtor OK.\n");
}

ProjectFunc REFLAG handleRunObject(JNIEnv *, jobject, jlong ext)
{
	return ((Extension *)ext)->Handle();
}

ProjectFunc REFLAG displayRunObject(JNIEnv *, jobject, jlong ext)
{
	// WARNING: not sure if this will work. Function signature was not in native SDK.
	return ((Extension *)ext)->Display();
}

ProjectFunc short pauseRunObject(JNIEnv *, jobject, jlong ext)
{
	return ((Extension *)ext)->FusionRuntimePaused();
}

ProjectFunc short continueRunObject(JNIEnv *, jobject, jlong ext)
{
	return ((Extension *)ext)->FusionRuntimeContinued();
}

extern thread_local JNIEnv * threadEnv;
jclass GetExtClass(void * javaExtPtr) {
	assert(threadEnv && mainThreadJNIEnv == threadEnv);
	static global<jclass> clazz(mainThreadJNIEnv->GetObjectClass((jobject)javaExtPtr), "static global<> ext class, GetExtClass(), from javaExtPtr");
	return clazz;
};
jobject GetRH(void * javaExtPtr) {
	assert(threadEnv && mainThreadJNIEnv == threadEnv);
	static jfieldID getRH(mainThreadJNIEnv->GetFieldID(GetExtClass(javaExtPtr), "rh", "LRunLoop/CRun;"));
	return mainThreadJNIEnv->GetObjectField((jobject)javaExtPtr, getRH);
};

int act_getParamExpression(void * javaExtPtr, void * act) {
	static global<jclass> actClass(mainThreadJNIEnv->GetObjectClass((jobject)act), "static global<> actClass, from act_getParamExpression");
	static jmethodID getActExpr(mainThreadJNIEnv->GetMethodID(actClass, "getParamExpression", "(LRunLoop/CRun;I)I"));
	return mainThreadJNIEnv->CallIntMethod((jobject)act, getActExpr, GetRH(javaExtPtr), -1);
}
RuntimeFunctions::string act_getParamExpString(void * javaExtPtr, void * act) {
	static global<jclass> actClass(mainThreadJNIEnv->GetObjectClass((jobject)act), "static global<> actClass, from act_getParamExpString");
	static jmethodID getActExpr(mainThreadJNIEnv->GetMethodID(actClass, "getParamFilename2", "(LRunLoop/CRun;I)Ljava/lang/String;"));
	RuntimeFunctions::string str;
	str.ctx = (jstring)mainThreadJNIEnv->CallObjectMethod((jobject)act, getActExpr, GetRH(javaExtPtr), -1);
	str.ptr = mainThreadJNIEnv->GetStringUTFChars((jstring)str.ctx, NULL);
	return str;
}
float act_getParamExpFloat(void * javaExtPtr, void * act) {
	static global<jclass> actClass(mainThreadJNIEnv->GetObjectClass((jobject)act), "static global<>actClass, from act_getParamExpFloat");
	static jmethodID getActExpr(mainThreadJNIEnv->GetMethodID(actClass, "getParamExpFloat", "(LRunLoop/CRun;I)F"));
	return mainThreadJNIEnv->CallFloatMethod((jobject)act, getActExpr, GetRH(javaExtPtr), -1);
}

int cnd_getParamExpression(void * javaExtPtr, void * cnd) {
	static global<jclass> cndClass(mainThreadJNIEnv->GetObjectClass((jobject)cnd), "static global<>cndClass, from cnd_getParamExpression");
	static jmethodID getcndExpr(mainThreadJNIEnv->GetMethodID(cndClass, "getParamExpression", "(LRunLoop/CRun;I)I"));
	return mainThreadJNIEnv->CallIntMethod((jobject)cnd, getcndExpr, GetRH(javaExtPtr), -1);
}
RuntimeFunctions::string cnd_getParamExpString(void * javaExtPtr, void * cnd) {
	static global<jclass> cndClass(mainThreadJNIEnv->GetObjectClass((jobject)cnd), "static global<>cndClass, from cnd_getParamExpString");
	static jmethodID getcndExpr(mainThreadJNIEnv->GetMethodID(cndClass, "getParamFilename2", "(LRunLoop/CRun;I)Ljava/lang/String;"));
	RuntimeFunctions::string str;
	str.ctx = (jstring)mainThreadJNIEnv->CallObjectMethod((jobject)cnd, getcndExpr, GetRH(javaExtPtr), -1);
	str.ptr = mainThreadJNIEnv->GetStringUTFChars((jstring)str.ctx, NULL);
	return str;
}
float cnd_getParamExpFloat(void * javaExtPtr, void * cnd) {
	static global<jclass> cndClass(mainThreadJNIEnv->GetObjectClass((jobject)cnd), "static global<> cndClass, from cnd_getParamExpFloat");
	static jmethodID getcndExpr(mainThreadJNIEnv->GetMethodID(cndClass, "getParamExpFloat", "(LRunLoop/CRun;I)F"));
	float f = mainThreadJNIEnv->CallFloatMethod((jobject)cnd, getcndExpr, GetRH(javaExtPtr), -1);
	return f;
}

int exp_getParamExpression(void * javaExtPtr, void * exp) {
	static global<jclass> expClass(mainThreadJNIEnv->GetObjectClass((jobject)exp), "static global<> expClass, from exp_getParamExpression");
	static jmethodID getexpExpr(mainThreadJNIEnv->GetMethodID(expClass, "getParamInt", "()I"));
	return mainThreadJNIEnv->CallIntMethod((jobject)exp, getexpExpr);
}
RuntimeFunctions::string exp_getParamExpString(void * javaExtPtr, void * exp) {
	static global<jclass> expClass(mainThreadJNIEnv->GetObjectClass((jobject)exp), "static global<> expClass, from exp_getParamExpString");
	static jmethodID getexpExpr(mainThreadJNIEnv->GetMethodID(expClass, "getParamString", "()Ljava/lang/String;"));
	RuntimeFunctions::string str;
	str.ctx = (jstring)mainThreadJNIEnv->CallObjectMethod((jobject)exp, getexpExpr);
	str.ptr = mainThreadJNIEnv->GetStringUTFChars((jstring)str.ctx, NULL);
	return str;
}
float exp_getParamExpFloat(void * javaExtPtr, void * exp) {
	static global<jclass> expClass(mainThreadJNIEnv->GetObjectClass((jobject)exp), "static global<> expClass, from exp_getParamExpFloat");
	static jmethodID setexpExpr(mainThreadJNIEnv->GetMethodID(expClass, "getParamFloat", "()F"));
	return mainThreadJNIEnv->CallFloatMethod((jobject)exp, setexpExpr);
}

void exp_setReturnInt(void * javaExtPtr, void * exp, int val) {
	static global<jclass> expClass(mainThreadJNIEnv->GetObjectClass((jobject)exp), "static global<> expClass, from exp_setReturnInt");
	static jmethodID setexpExpr(mainThreadJNIEnv->GetMethodID(expClass, "setReturnInt", "(I)V"));
	mainThreadJNIEnv->CallVoidMethod((jobject)exp, setexpExpr, val);
}

static std::uint8_t UTF8_CHAR_WIDTH[] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x1F
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x3F
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x5F
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x7F
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x9F
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xBF
	0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 0xDF
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 0xEF
	4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xFF
};

// This is set in Edif::Runtime
thread_local JNIEnv * threadEnv = nullptr;

jstring CStrToJStr(const char * String)
{
#ifdef _DEBUG
	if (threadEnv->ExceptionCheck())
		LOGF("Already a bug when returning a string!! Error %s, text to return %s.\n", GetJavaExceptionStr().c_str(), String);
	if (String == nullptr)
		LOGF("String pointer is null in CStrToJStr()!\n");
#endif

	// Java doesn't use regular UTF-8, but "modified" UTF-8, or CESU-8.
	// In Java, UTF-8 null and UTF8 4-byte characters aren't allowed. They will need re-encoding.
	// Thankfully, they're not common in English usage. So, we'll quickly check.

	// We can ignore the check for embedded null, as strlen() will stop at first null anyway, and Runtime.CopyString() is
	// only for expressions returning non-null strings.
	unsigned char * bytes = (unsigned char *)String;
	size_t strU8Len = strlen(String);
	jstring jstr = nullptr;

	for (int k = 0; k < strU8Len; k++)
	{
		// 4-byte UTF-8, welp.
		if (bytes[k] >= 0xF0 && bytes[k] <= 0xF5)
			goto reconvert;
	}
	LOGV("UTF-8 String \"%s\" should already be valid Modified UTF-8.\n", String);

	// No 4-byte characters, safe to convert directly
	jstr = threadEnv->NewStringUTF(String);
	if (threadEnv->ExceptionCheck())
		LOGE("Failed to convert string, got error %s.\n", GetJavaExceptionStr().c_str());
	return jstr;
reconvert:
	LOGV("Reconverting UTF-8 to Modified UTF-8 in Runtime.CopyStringEx().\n");
	std::string newString(strU8Len + (strU8Len >> 2), '\0');
	int inputByteIndex = 0, outputByteIndex = 0;
	while (inputByteIndex < strU8Len) {
		std::uint8_t b = bytes[inputByteIndex];

		/*
		// Null byte, but since we use strlen above, this won't happen
		if (b == 0) {
			assert(false && "Null byte inside expression return");
			// Null bytes in Java are encoded as 0xc0, 0x80
			newString[outputByteIndex++] = 0xC0;
			newString[outputByteIndex++] = 0x80;
			inputByteIndex++;
		}
		else */
		if (b < 128) {
			// Pass ASCII through quickly.
			newString[outputByteIndex++] = bytes[inputByteIndex++];
		}
		else {
			// Figure out how many bytes we need for this character.
			int w = UTF8_CHAR_WIDTH[bytes[inputByteIndex]];
			assert(w <= 4);
			assert(inputByteIndex + w <= strU8Len);
			if (w == 4) {
				// Assume valid UTF-8 was already confirmed; we have a 4-byte UTF-8 we need to convert to a UTF-32.
				// Convert using https://gist.github.com/ozdemirburak/89a7a1673cb65ce83469#file-converter-c-L190
				unsigned int charAsUTF32 = ((bytes[inputByteIndex] & 0x07) << 18) | ((bytes[inputByteIndex + 1] & 0x3f) << 12) | ((bytes[inputByteIndex + 2] & 0x3f) << 6) | (bytes[inputByteIndex + 3] & 0x3f);
				unsigned int charAsUTF32Modified = charAsUTF32 - 0x10000;
				std::uint16_t surrogates[] = { 0, 0 };
				surrogates[0] = ((std::uint16_t)(charAsUTF32Modified >> 10)) | 0xD800;
				surrogates[1] = ((std::uint16_t)(charAsUTF32Modified & 0x3FF)) | 0xDC00;

				auto enc_surrogate = [&outputByteIndex, &newString](std::uint16_t surrogate) {
					assert(0xD800 <= surrogate && surrogate <= 0xDFFF);
					// 1110xxxx 10xxxxxx 10xxxxxx
					newString[outputByteIndex++] = 0b11100000 | ((surrogate & 0b1111000000000000) >> 12);
					newString[outputByteIndex++] = 0b10000000 | ((surrogate & 0b0000111111000000) >> 6);
					newString[outputByteIndex++] = 0b10000000 | ((surrogate & 0b0000000000111111));
				};
				enc_surrogate(surrogates[0]);
				enc_surrogate(surrogates[1]);
			}
			else // Pass through short UTF-8 sequences unmodified.
			{
				// Corrupt string may cause infinite loop; replace invalid char instead
				if (w == 0)
				{
					newString[outputByteIndex] = '?';
					outputByteIndex += 1;
					inputByteIndex += 1;
					break;
				}

				// Basically just memcpy(newString.data() + outputByteIndex, &bytes[inputByteIndex], w);

				if (w == 1)
					*((std::uint8_t *)&newString[outputByteIndex]) = bytes[inputByteIndex];
				else if (w == 2)
					*((std::uint16_t *)&newString[outputByteIndex]) = *(std::uint16_t *)&bytes[inputByteIndex];
				else // w == 3
				{
					*((std::uint8_t *)&newString[outputByteIndex]) = bytes[inputByteIndex];
					*((std::uint16_t *)&newString[outputByteIndex + 1]) = *(std::uint16_t *)&bytes[inputByteIndex + 1];
				}
				outputByteIndex += w;
			}
			inputByteIndex += w;
		}
	}
	newString.resize(outputByteIndex);

	jstr = threadEnv->NewStringUTF(String);
	if (threadEnv->ExceptionCheck())
		LOGE("Failed to convert string, got error %s.\n", GetJavaExceptionStr().c_str());
	return jstr;
}

// Converts std::thread::id to a std::string
std::string ThreadIDToStr(std::thread::id id)
{
	// Most compatible way
	std::ostringstream str;
	if (id != std::this_thread::get_id())
	{
		LOGE("Not the right ID.\n");
		str << std::hex << id;
	}
	else
		str << gettid();
	return str.str();
}

void exp_setReturnString(void * javaExtPtr, void * exp, const char * val) {
	static global<jclass> expClass(mainThreadJNIEnv->GetObjectClass((jobject)exp), "static global<> expClass, from exp_setReturnString");
	static jmethodID setexpExpr(mainThreadJNIEnv->GetMethodID(expClass, "setReturnString", "(Ljava/lang/String;)V"));

	// Convert into Java memory
	jstring jStr = CStrToJStr(val);
	mainThreadJNIEnv->CallVoidMethod((jobject)exp, setexpExpr, jStr);
	JNIExceptionCheck();
	mainThreadJNIEnv->DeleteLocalRef(jStr); // not strictly needed
	JNIExceptionCheck();
}
void exp_setReturnFloat(void * javaExtPtr, void * exp, float val) {
	static global<jclass> expClass(mainThreadJNIEnv->GetObjectClass((jobject)exp), "static global<> expClass, from exp_setReturnFloat");
	static jmethodID getexpExpr(mainThreadJNIEnv->GetMethodID(expClass, "setReturnFloat", "(F)V"));
	mainThreadJNIEnv->CallVoidMethod((jobject)exp, getexpExpr, val);
}

void freeString(void * ext, RuntimeFunctions::string str)
{
	threadEnv->ReleaseStringUTFChars((jstring)str.ctx, str.ptr);
	JNIExceptionCheck();
	str = { NULL, NULL };
}
void generateEvent(void * javaExtPtr, int code, int param) {
	LOGW("GenerateEvent ID %i attempting...\n", code);
	static global<jclass> expClass(threadEnv->GetObjectClass((jobject)javaExtPtr), "static global<> expClass, from generateEvent");
	static jfieldID getHo(threadEnv->GetFieldID(expClass, "ho", "LObjects/CExtension;")); // ?
	jobject ho = threadEnv->GetObjectField((jobject)javaExtPtr, getHo);
	static global<jclass> hoClass(threadEnv->GetObjectClass(ho), "static global<> ho, from generateEvent");
	static jmethodID genEvent(threadEnv->GetMethodID(hoClass, "generateEvent", "(II)V"));
	threadEnv->CallVoidMethod(ho, genEvent, code, param);
};
void pushEvent(void * javaExtPtr, int code, int param) {
	static global<jclass> expClass(threadEnv->GetObjectClass((jobject)javaExtPtr), "static global<> expClass, from pushEvent");
	static jfieldID getHo(threadEnv->GetFieldID(expClass, "ho", "LObjects/CExtension;")); // ?
	jobject ho = threadEnv->GetObjectField((jobject)javaExtPtr, getHo);
	static global<jclass> hoClass(threadEnv->GetObjectClass(ho), "static global<> ho, from pushEvent");
	static jmethodID pushEvent(threadEnv->GetMethodID(hoClass, "pushEvent", "(II)V"));
	threadEnv->CallVoidMethod(ho, pushEvent, code, param);
};

void LOGF(const TCHAR * x, ...)
{
	va_list va;
	va_start(va, x);
	__android_log_vprint(ANDROID_LOG_ERROR, PROJECT_NAME_UNDERSCORES, x, va);
	va_end(va);
	__android_log_write(ANDROID_LOG_FATAL, PROJECT_NAME_UNDERSCORES, "Killed by extension " PROJECT_NAME ".");
	if (threadEnv)
		threadEnv->FatalError("Killed by extension " PROJECT_NAME ". Look at previous logcat entries from " PROJECT_NAME_UNDERSCORES " for details.");
	else
	{
		__android_log_write(ANDROID_LOG_FATAL, PROJECT_NAME_UNDERSCORES, "Killed from unattached thread! Running exit.");
		exit(EXIT_FAILURE);
	}
}
// Call via JNIExceptionCheck(). If a Java exception is pending, instantly dies.
void Indirect_JNIExceptionCheck(const char * file, const char * func, int line)
{
	if (!threadEnv)
	{
		LOGF("JNIExceptionCheck() called before threadEnv was initialized.\n");
		return;
	}
	if (!threadEnv->ExceptionCheck())
		return;
	jthrowable exc = threadEnv->ExceptionOccurred();
	threadEnv->ExceptionClear(); // else GetObjectClass fails, which is dumb enough.
	jclass exccls = threadEnv->GetObjectClass(exc);
	jmethodID getMsgMeth = threadEnv->GetMethodID(exccls, "toString", "()Ljava/lang/String;");

	jstring excStr = (jstring)threadEnv->CallObjectMethod(exc, getMsgMeth);
	const char * c = threadEnv->GetStringUTFChars(excStr, NULL);
	LOGF("JNIExceptionCheck() in file \"%s\", func \"%s\", line %d, found a JNI exception: %s.\n",
		file, func, line, c);

	raise(SIGINT);
	threadEnv->ReleaseStringUTFChars(excStr, c);
	return;
}
std::string GetJavaExceptionStr()
{
	if (!threadEnv->ExceptionCheck())
		return std::string("No exception!");
	jthrowable exc = threadEnv->ExceptionOccurred();
	threadEnv->ExceptionClear(); // else GetObjectClass fails, which is dumb enough.
	jclass exccls = threadEnv->GetObjectClass(exc);
	jmethodID getMsgMeth = threadEnv->GetMethodID(exccls, "toString", "()Ljava/lang/String;");

	jstring excStr = (jstring)threadEnv->CallObjectMethod(exc, getMsgMeth);
	const char * c = threadEnv->GetStringUTFChars(excStr, NULL);

	std::string ret(c);

	threadEnv->ReleaseStringUTFChars(excStr, c);
	return ret;
}


#ifdef _DEBUG

#include <iostream>
#include <iomanip>
#include <unwind.h>
#include <dlfcn.h>

namespace {

	struct BacktraceState
	{
		void ** current;
		void ** end;
	};

	static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context * context, void * arg)
	{
		BacktraceState * state = static_cast<BacktraceState *>(arg);
		uintptr_t pc = _Unwind_GetIP(context);
		if (pc) {
			if (state->current == state->end) {
				return _URC_END_OF_STACK;
			}
			else {
				*state->current++ = reinterpret_cast<void *>(pc);
			}
		}
		return _URC_NO_REASON;
	}

}

// Taken from https://stackoverflow.com/a/28858941
size_t captureBacktrace(void ** buffer, size_t max)
{
	BacktraceState state = { buffer, buffer + max };
	_Unwind_Backtrace(unwindCallback, &state);

	return state.current - buffer;
}
#include <cxxabi.h>
void dumpBacktrace(std::ostream & os, void ** buffer, size_t count)
{
	os << "Call stack, last function is bottommost:\n";
	size_t outputMemSize = 512;
	char * outputMem = (char *)malloc(outputMemSize);

	for (int idx = count - 1; idx >= 0; idx--) {
		//	for (size_t idx = 0; idx < count; ++idx) {
		const void * addr = buffer[idx];
		const char * symbol = "";

		Dl_info info;
		if (dladdr(addr, &info) && info.dli_sname) {
			symbol = info.dli_sname;
		}
		memset(outputMem, 0, outputMemSize);
		int status = 0;
		abi::__cxa_demangle(symbol, outputMem, &outputMemSize, &status);
		os << "  #" << std::setw(2) << idx << ": " << addr << "  " << (status == 0 ? outputMem : symbol) << "\n";
	}
	free(outputMem);
}
//int signalCatches[] = { SIGABRT, SIGSEGV, SIGBUS, SIGPIPE };
struct Signal {
	int signalNum;
	const char * signalName;
	Signal(int s, const char * n) : signalNum(s), signalName(n) {}
};
static Signal signalCatches[] = {
	//{SIGSEGV, "SIGSEGV" },
	{SIGBUS, "SIGBUS"},
	{SIGPIPE, "SIGPIPE"}
};

static void my_handler(const int code, siginfo_t * const si, void * const sc)
{
	static size_t numCatches = 0;
	if (++numCatches > 3) {
		exit(0);
		return;
	}

#if DARKEDIF_MIN_LOG_LEVEL <= DARKEDIF_LOG_ERROR
	const char * signalName = "Unknown";
	for (size_t i = 0; i < std::size(signalCatches); i++)
	{
		if (signalCatches[i].signalNum == code)
		{
			signalName = signalCatches[i].signalName;
			break;
		}
	}
#endif

	const size_t max = 30;
	void * buffer[max];
	std::ostringstream oss;

	dumpBacktrace(oss, buffer, captureBacktrace(buffer, max));

	LOGI("%s\n", oss.str().c_str());
	LOGE("signal code raised: %d, %s.\n", code, signalName);
	// Breakpoint
#ifdef _DEBUG
	raise(SIGTRAP);
#endif
}
static bool didSignals = false;

static void prepareSignals()
{
	didSignals = true;

	for (int i = 0; i < std::size(signalCatches); i++) {

		struct sigaction sa;
		struct sigaction sa_old;
		memset(&sa, 0, sizeof(sa));
		sigemptyset(&sa.sa_mask);
		sa.sa_sigaction = my_handler;
		sa.sa_flags = SA_SIGINFO;
		if (sigaction(signalCatches[i].signalNum, &sa, &sa_old) != 0) {
			LOGW("Failed to set up %s sigaction.\n", signalCatches[i].signalName);
		}
#if 0 && __ANDROID_API__ >= 28
		struct sigaction64 sa64;
		struct sigaction64 sa64_old;
		memset(&sa64, 0, sizeof(sa64));
		sigemptyset(&sa64.sa_mask);
		sa.sa_sigaction = my_handler;
		sa.sa_flags = SA_SIGINFO;
		if (sigaction64(signalCatches[i].signalNum, &sa64, &sa64_old) != 0) {
			LOGW("Failed to set up %s sigaction (64 bit).\n", signalCatches[i].signalName);
		}
#endif
	}

	LOGI("Set up %zu sigactions.\n", std::size(signalCatches));
}
#endif // _DEBUG

// Included from root dir
#if __has_include("ExtraAndroidNatives.h")
#include <ExtraAndroidNatives.h>
#endif
#ifndef EXTRAFUNCS
#define EXTRAFUNCS /* none*/
#endif

ProjectFunc jlong conditionJump(JNIEnv*, jobject, jlong extPtr, jint cndID, CCndExtension cnd);
ProjectFunc void actionJump(JNIEnv*, jobject, jlong extPtr, jint actID, CActExtension act);
ProjectFunc void expressionJump(JNIEnv*, jobject, jlong extPtr, jint expID, CNativeExpInstance exp);

ProjectFunc jint JNICALL JNI_OnLoad(JavaVM * vm, void * reserved) {
	// https://developer.android.com/training/articles/perf-jni.html#native_libraries

	jint error = vm->GetEnv(reinterpret_cast<void **>(&mainThreadJNIEnv), JNI_VERSION_1_6);
	if (error != JNI_OK) {
		LOGF("GetEnv failed with error %d.\n", error);
		return -1;
	}
	global_vm = vm;
	LOGV("GetEnv OK, returned %p.\n", mainThreadJNIEnv);

	JavaVMAttachArgs args;
	args.version = JNI_VERSION_1_6; // choose your JNI version
	args.name = PROJECT_NAME ", JNI_Load"; // you might want to give the java thread a name
	args.group = NULL; // you might want to assign the java thread to a ThreadGroup
	if ((error = vm->AttachCurrentThread(&mainThreadJNIEnv, NULL)) != JNI_OK) {
		LOGF("AttachCurrentThread failed with error %d.\n", error);
		return -1;
	}

	// Get jclass with mainThreadJNIEnv->FindClass.
	// Register methods with mainThreadJNIEnv->RegisterNatives.
	std::string classNameCRun("Extensions/" "CRun" PROJECT_NAME_UNDERSCORES);
	std::string className("Extensions/" PROJECT_NAME_UNDERSCORES);
	LOGV("Looking for class %s... [1/2]\n", classNameCRun.c_str());
	jclass clazz = mainThreadJNIEnv->FindClass(classNameCRun.c_str());
	if (clazz == NULL) {
		LOGV("Couldn't find %s, now looking for %s... [2/2]\n", classNameCRun.c_str(), className.c_str());
		if (mainThreadJNIEnv->ExceptionCheck()) {
			mainThreadJNIEnv->ExceptionClear();
			LOGV("EXCEPTION [1] %d\n", 0);
		}
		clazz = mainThreadJNIEnv->FindClass(className.c_str());
		if (clazz == NULL)
		{
			if (mainThreadJNIEnv->ExceptionCheck()) {
				mainThreadJNIEnv->ExceptionClear();
				LOGV("EXCEPTION [2] %d\n", 0);
			}
			LOGF("Couldn't find class %s. Aborting load of extension.\n", className.c_str());
			return JNI_VERSION_1_6;
		}
		LOGV("Found %s. [2/2]\n", className.c_str());
	}
	else
		LOGV("Found %s. [1/2]\n", classNameCRun.c_str());

#define method(a,b) { "darkedif_" #a, b, (void *)&a }
	//public native long DarkEdif_createRunObject(ByteBuffer edPtr, CCreateObjectInfo cob, int version);
	static JNINativeMethod methods[] = {
		method(getNumberOfConditions, "(J)I"),
		method(createRunObject, "(Ljava/nio/ByteBuffer;LRunLoop/CCreateObjectInfo;I)J"),
		method(destroyRunObject, "(JZ)V"),
		method(handleRunObject, "(J)S"),
		method(displayRunObject, "(J)S"),
		method(pauseRunObject, "(J)S"),
		method(continueRunObject, "(J)S"),
		method(conditionJump, "(JILConditions/CCndExtension;)J"),
		method(actionJump, "(JILActions/CActExtension;)V"),
		method(expressionJump, "(JILExpressions/CNativeExpInstance;)V"),
		EXTRAFUNCS
	};

	LOGV("Registering natives for %s...\n", PROJECT_NAME);
	if (mainThreadJNIEnv->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
		threadEnv = mainThreadJNIEnv;
		std::string excStr = GetJavaExceptionStr();
		LOGF("Failed to register natives for ext %s; error %s.\n", PROJECT_NAME, excStr.c_str());
	}
	else
		LOGV("Registered natives for ext %s successfully.\n", PROJECT_NAME);
	mainThreadJNIEnv->DeleteLocalRef(clazz);
	runFuncs.ext = NULL;

	// could be Actions.RunLoop.CRun
	runFuncs.act_getParamExpression = act_getParamExpression;
	runFuncs.act_getParamExpString = act_getParamExpString;
	runFuncs.act_getParamExpFloat = act_getParamExpFloat;

	runFuncs.cnd_getParamExpression = cnd_getParamExpression;
	runFuncs.cnd_getParamExpString = cnd_getParamExpString;
	runFuncs.cnd_getParamExpFloat = cnd_getParamExpFloat;

	runFuncs.exp_getParamInt = exp_getParamExpression;
	runFuncs.exp_getParamString = exp_getParamExpString;
	runFuncs.exp_getParamFloat = exp_getParamExpFloat;

	runFuncs.exp_setReturnInt = exp_setReturnInt;
	runFuncs.exp_setReturnString = exp_setReturnString;
	runFuncs.exp_setReturnFloat = exp_setReturnFloat;

	runFuncs.freeString = freeString;

	runFuncs.generateEvent = generateEvent;
	threadEnv = mainThreadJNIEnv;

#ifdef _DEBUG
	if (!didSignals)
		prepareSignals();
#endif

	mv * mV = NULL;
	if (!::SDK) {
		LOGV("The SDK is being initialized.\n");
		Edif::Init(mV, false);
	}

	return JNI_VERSION_1_6;
}
ProjectFunc void JNICALL JNI_OnUnload(JavaVM * vm, void * reserved)
{
	LOGV("JNI_Unload.\n");

#ifdef _DEBUG
	// Reset signals
	if (didSignals)
	{
		for (int i = 0; i < std::size(signalCatches); i++)
			signal(signalCatches[i].signalNum, SIG_DFL);
	}
#endif
}

#else // iOS
#include "Extension.h"
class CValue;

// Raw creation func
ProjectFunc void PROJ_FUNC_GEN(PROJECT_NAME_RAW, _init())
{
	mv * mV = NULL;
	if (!::SDK) {
		LOGV("The SDK is being initialized.\n");
		Edif::Init(mV, false);
	}
}

// Last Objective-C class was freed
ProjectFunc void PROJ_FUNC_GEN(PROJECT_NAME_RAW, _dealloc())
{
	LOGV("The SDK is being freed.\n");
}

ProjectFunc int PROJ_FUNC_GEN(PROJECT_NAME_RAW, _getNumberOfConditions())
{
	return CurLang["Conditions"].u.array.length;
}
ProjectFunc void * PROJ_FUNC_GEN(PROJECT_NAME_RAW, _createRunObject(void * file, int cob, int version, void * objCExtPtr))
{
	EDITDATA * edPtr = (EDITDATA *)file;
	LOGV("Note: objCExtPtr is %p, edPtr %p.\n", objCExtPtr, edPtr);
	RuntimeFunctions * runFuncs = new RuntimeFunctions();
	Extension * cppExt = new Extension(*runFuncs, (EDITDATA *)edPtr, objCExtPtr);
	cppExt->Runtime.ObjectSelection.pExtension = cppExt;
	return cppExt;
}
ProjectFunc short PROJ_FUNC_GEN(PROJECT_NAME_RAW, _handleRunObject)(void * cppExt)
{
	return (short) ((Extension *)cppExt)->Handle();
}
ProjectFunc void PROJ_FUNC_GEN(PROJECT_NAME_RAW, _destroyRunObject)(void * cppExt, bool bFast)
{
	delete ((Extension *)cppExt);
}

#endif
