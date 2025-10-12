// ============================================================================
// Edif General:
// The following routines are used internally by Fusion, and should not need to
// be modified.
// ============================================================================

#include "Common.hpp"

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
	// Ignore warning about TerminateThread - better to kill thread than crash Fusion,
	// and at this point we've waited 3 full seconds
#pragma warning(push)
#pragma warning(disable: 6258)
	extern HANDLE updateThread;
	if (updateThread != NULL && WaitForSingleObject(updateThread, 3000) == WAIT_TIMEOUT)
		TerminateThread(updateThread, 2);
#pragma warning(pop)
#endif
	return 0; // No error
}


// ============================================================================
// GENERAL INFO
// ============================================================================
#ifndef NOPROPS
extern void LoadObjectWipe(EDITDATA* edPtr);
#endif

// Routine called for each object when the object is read from the MFA file (edit time)
// or from the CCN or EXE file (run time).
// DarkEdif users shouldn't need to modify this function.
int FusionAPI LoadObject(mv * mV, const char * fileName, EDITDATA * edPtr, int reserved)
{
#pragma DllExportHint
	Edif::Init(mV, edPtr);

	// DarkEdif smart props v2 checkbox wipe
#ifndef NOPROPS
	LoadObjectWipe(edPtr);
#endif
	return 0;
}

// The counterpart of LoadObject(): called just before the object is
// deleted from the frame.
// DarkEdif users shouldn't need to modify this function.
void FusionAPI UnloadObject(mv * mV, EDITDATA * edPtr, int reserved)
{
	#pragma DllExportHint
}


const TCHAR * Dependencies[32] = {};

const TCHAR ** FusionAPI GetDependencies()
{
#pragma DllExportHint
	// This is pointed to by Dependencies.
	static TCHAR singleton[1024];

	if (!Dependencies)
	{
		const json_value &DependenciesJSON = Edif::SDK->json["Dependencies"sv];
		TCHAR* singletonPtr = singleton;

		std::size_t Offset = 0;

		if (Edif::ExternalJSON)
		{
			TCHAR JSONFilename [MAX_PATH];

			GetModuleFileName (hInstLib, JSONFilename, sizeof (JSONFilename) / sizeof(*JSONFilename));

			TCHAR * Iterator = JSONFilename + _tcslen(JSONFilename) - 1;

			while (*Iterator != _T('.'))
				-- Iterator;

			_tcscpy(++ Iterator, _T("json"));

			Iterator = JSONFilename + _tcslen(JSONFilename) - 1;

			while (*Iterator != _T('\\') && *Iterator != _T('/'))
				-- Iterator;

			// We append it, then add a pointer to what we just appended
			// to our pointer list
			_tcscpy(singletonPtr, ++Iterator);
			Dependencies[Offset++] = singletonPtr;
			singletonPtr += _tcslen(singletonPtr) + 1;
		}

		for (unsigned int i = 0; i < DependenciesJSON.u.array.length; ++i)
		{
			std::tstring tstr = DarkEdif::UTF8ToTString(DependenciesJSON[i]);

			_tcscpy(singletonPtr, tstr.c_str());
			Dependencies[Offset++] = singletonPtr;
			singletonPtr += tstr.size() + 1;
		}

		Dependencies[Offset] = 0;
	}

	return Dependencies;
}
struct ForbiddenInternals2 {
	static inline void SetExtension(RunObject* ro, Extension* ext) {
		ro->SetExtension(ext);
	}
	static std::int16_t GetRunObjectInfos2(mv* mV, kpxRunInfos* infoPtr);
};

// Called every time the extension is being created from nothing.
// Default property contents should be loaded from JSON.
std::int16_t FusionAPI GetRunObjectInfos(mv* mV, kpxRunInfos* infoPtr)
{
#pragma DllExportHint
#if RuntimeBuild
	DarkEdif::MsgBox::Info(_T("!!"), _T("GetRunObjectInfos() called.\n"));
#endif
	return ForbiddenInternals2::GetRunObjectInfos2(mV, infoPtr);
}

std::int16_t ForbiddenInternals2::GetRunObjectInfos2(mv * mV, kpxRunInfos * infoPtr)
{
	infoPtr->Conditions = &Edif::SDK->ConditionJumps[0];
	infoPtr->Actions = &Edif::SDK->ActionJumps[0];
	infoPtr->Expressions = &Edif::SDK->ExpressionJumps[0];

	infoPtr->NumOfConditions = CurLang["Conditions"sv].u.object.length;
	infoPtr->NumOfActions = CurLang["Actions"sv].u.object.length;
	infoPtr->NumOfExpressions = CurLang["Expressions"sv].u.object.length;
#ifdef DARKSCRIPT_EXTENSION
	infoPtr->NumOfExpressions = Extension::GetNumExpressions();
#endif

	static unsigned short EDITDATASize = UINT16_MAX;
	if (EDITDATASize == UINT16_MAX)
	{
#ifdef NOPROPS
		EDITDATASize = sizeof(EDITDATA);
#elif EditorBuild
		EDITDATASize = DarkEdif::DLL::Internal_GetEDITDATASizeFromJSON();
#else
		EDITDATASize = sizeof(EDITDATA);
#endif // NOPROPS
	}
	infoPtr->EDITDATASize = EDITDATASize;

#if WNDPROC_OEFLAG_EXTENSION
	infoPtr->WindowProcPriority = Extension::WindowProcPriority;
#else
	infoPtr->WindowProcPriority = 100; // default
#endif

	infoPtr->EditFlags = Extension::OEFLAGS;
	infoPtr->EditPrefs = Extension::OEPREFS;

	const std::string_view ident = Edif::SDK->json["Identifier"sv];
	assert(ident.size() == 4);

	memcpy(&infoPtr->Identifier, ident.data(), 4);

	// Smart properties can change the version Fusion is told
	infoPtr->Version = DarkEdif::Properties::VersionFlags | Extension::Version;

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

	Extension* ext = new Extension((RunObject*)rdPtr, edPtr, cobPtr);
	ForbiddenInternals2::SetExtension((RunObject*)rdPtr, ext);
	ext->Runtime.ObjectSelection.pExtension = ext;
	return 0;
}

/* Don't touch any of these, they're global to all extensions! See Extension.cpp */

std::int16_t FusionAPI DestroyRunObject(RUNDATA * rdPtr, long fast)
{
	// fast is true if quickly clearing all frame objects at end of frame/app, otherwise false
#pragma DllExportHint
	Extension* ext = ((RunObject*)rdPtr)->GetExtension();
#ifdef _DEBUG
	// Something's wrong with the expected arrangement of RUNDATA, is pExtension not immediately following roa/rov and co?
	if (((RunObject*)rdPtr)->GetExtension() != ext)
		DarkEdif::MsgBox::Error(_T("DarkEdif - RUNDATA error"), _T("Expected RUNDATA::pExtension at %p, but was not present. Is RUNDATA in the wrong arrangement?"),
			((RunObject*)rdPtr)->GetExtension());
#endif
	delete ext;
	ForbiddenInternals2::SetExtension((RunObject*)rdPtr, nullptr);

	return 0;
}

REFLAG FusionAPI HandleRunObject(RUNDATA * rdPtr)
{
#pragma DllExportHint
	return ((RunObject*)rdPtr)->GetExtension()->Handle();
}


// Grabs the local ext's RUNDATA size by OEFLAG calculation.
// @remarks While it's permitted to put extra things in RUNDATA, this function assumes pExtension is immediately following
//			the common structs like AltVals, and that the various structs are local SDK's sizes.
static constexpr std::uint16_t RDSize() {
	std::uint16_t rdSize = sizeof(HeaderObject);
	if constexpr ((Extension::OEFLAGS & (OEFLAGS::ANIMATIONS | OEFLAGS::MOVEMENTS | OEFLAGS::SPRITES)) != OEFLAGS::NONE)
		rdSize += sizeof(rCom);
	if constexpr ((Extension::OEFLAGS & OEFLAGS::ANIMATIONS) != OEFLAGS::NONE)
		rdSize += sizeof(rAni);
	if constexpr ((Extension::OEFLAGS & OEFLAGS::MOVEMENTS) != OEFLAGS::NONE)
		rdSize += sizeof(rMvt);
	if constexpr ((Extension::OEFLAGS & OEFLAGS::SPRITES) != OEFLAGS::NONE)
		rdSize += sizeof(RunSprite);
	if constexpr ((Extension::OEFLAGS & OEFLAGS::VALUES) != OEFLAGS::NONE)
		rdSize += std::max(sizeof(AltVals::CF25), sizeof(AltVals::MMF2));
	// OEFLAGS::WINDOW_PROC is ignored, as the int + HANDLE[] offset is pointed to by
	// a full pointer, so it can be put inside Extension
	rdSize += sizeof(Extension*); // pExtension
	return rdSize;
}

Extension* RunObject::GetExtension()
{
	char* ret = ((char *)this) + sizeof(HeaderObject);
	if ((rHo.hoOEFlags & (OEFLAGS::ANIMATIONS | OEFLAGS::MOVEMENTS | OEFLAGS::SPRITES)) != OEFLAGS::NONE)
		ret += sizeof(rCom);
	if ((rHo.hoOEFlags & OEFLAGS::ANIMATIONS) != OEFLAGS::NONE)
		ret += sizeof(rAni);
	if ((rHo.hoOEFlags & OEFLAGS::MOVEMENTS) != OEFLAGS::NONE)
		ret += sizeof(rMvt);
	if ((rHo.hoOEFlags & OEFLAGS::SPRITES) != OEFLAGS::NONE)
		ret += sizeof(RunSprite);
	if ((rHo.hoOEFlags & OEFLAGS::VALUES) != OEFLAGS::NONE)
		ret += std::max(sizeof(AltVals::CF25), sizeof(AltVals::MMF2));
	Extension* const ext = *(Extension**)ret;
	if (!ext)
	{
		LOGF(_T("OEFLAGS was altered between Fusion start and this MFA creation.\n")
			"The MFA uses an object of OEFLAGS %i, but the Extension expects OEFLAGS %i.\n"
			"This is an unrecoverable error. Restart Fusion editor.",
			rHo.hoOEFlags, Extension::OEFLAGS);
	}
	return ext;
}
void RunObject::SetExtension(Extension * const ext) {
	// RDSize includes Extension * ptr, so subtract it to get offset of it
	*(Extension**)(((char*)this) + RDSize() - sizeof(Extension*)) = ext;
}

std::uint16_t FusionAPI GetRunObjectDataSize(RunHeader * rhPtr, EDITDATA * edPtr)
{
#pragma DllExportHint
	return RDSize();
}

std::int16_t FusionAPI PauseRunObject(RUNDATA * rdPtr)
{
#pragma DllExportHint
	// Note: PauseRunObject is required, or runtime will crash when pausing.
	// TODO: Does the return from PauseRunObject actually change anything?
#if PAUSABLE_EXTENSION
	((RunObject*)rdPtr)->GetExtension()->FusionRuntimePaused();
#endif
	return 0;
}

std::int16_t FusionAPI ContinueRunObject(RUNDATA * rdPtr)
{
#pragma DllExportHint
#if PAUSABLE_EXTENSION
	((RunObject*)rdPtr)->GetExtension()->FusionRuntimeContinued();
#endif
	return 0;
}


#elif defined(__ANDROID__)

ProjectFunc jint getNumberOfConditions(JNIEnv *, jobject, jlong cptr)
{
	//raise(SIGTRAP);
	return CurLang["Conditions"sv].u.array.length;
}
typedef jobject ByteBufferDirect;

struct COIInternals {
	static CreateObjectInfo&& MakeCreateObjectInfo(jobject o) {
		return std::move(CreateObjectInfo(o));
	}
};
ProjectFunc jlong createRunObject(JNIEnv * env, jobject javaExtPtr, ByteBufferDirect edPtr, jobject coi, jint version)
{
	void * edPtrReal = mainThreadJNIEnv->GetDirectBufferAddress(edPtr);
	LOGV("Note: mainThreadJNIEnv is %p, env is %p; javaExtPtr is %p, edPtr %p, edPtrReal %p, coi %p.\n", mainThreadJNIEnv, env, javaExtPtr, edPtr, edPtrReal, coi);
	global<jobject> javaExtP(javaExtPtr, "createRunObject javaExtPtr");

	CreateObjectInfo&& coiReal = COIInternals::MakeCreateObjectInfo(coi);
	Extension * ext = new Extension((EDITDATA *)edPtrReal, javaExtPtr, &coiReal);
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

extern const char* getClassName(jclass myCls, bool fullpath);
Extension* RunObject::GetExtension()
{
	if (!runExt.ref)
	{
		const jfieldID runExtField = threadEnv->GetFieldID(meClass, "ext", "LExtensions/CRunExtension;");
		JNIExceptionCheck();
		if (!runExtField)
		{
			threadEnv->ExceptionClear();
			LOGF(_T("RunObject::GetExtension() error: CExtension class was actually a \"%s\".\n"),
				getClassName(meClass, false));
			return nullptr;
		}
		runExt = global(threadEnv->GetObjectField(me, runExtField), "CRunExtension variant in RunObject");
		runExtClass = global(threadEnv->GetObjectClass(runExt), "CRunExtension variant class in RunObject");
	}

	// Non-DarkEdif won't have cptr
	const jfieldID cppExtField = threadEnv->GetFieldID(runExtClass, "cptr", "J");
	if (!cppExtField)
	{
		// not found: there's gonna be a FieldDoesNotExist Java exception
		threadEnv->ExceptionClear();
		LOGW(_T("RunObject::GetExtension() error: RunObject for extension class \"%s\" does not have an cptr Extension pointer.\n"),
			getClassName(runExtClass, false));
		return nullptr;
	}
	const jlong extPtr = threadEnv->GetLongField(runExt, cppExtField);
	JNIExceptionCheck();
	if (!extPtr)
	{
		LOGE(_T("RunObject::GetExtension() warning: cptr was null.\n"));
		return nullptr;
	}

	return (Extension*)extPtr;
}

#if PAUSABLE_EXTENSION
ProjectFunc void pauseRunObject(JNIEnv *, jobject, jlong ext)
{
	((Extension *)ext)->FusionRuntimePaused();
}

ProjectFunc void continueRunObject(JNIEnv *, jobject, jlong ext)
{
	((Extension *)ext)->FusionRuntimeContinued();
}
#endif // PAUSABLE_EXTENSION

extern thread_local JNIEnv * threadEnv;

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

	for (int k = 0; k < strU8Len; ++k)
	{
		// 4-byte UTF-8, welp.
		if (bytes[k] >= 0xF0 && bytes[k] <= 0xF5)
			goto reconvert;
	}
	LOGV("UTF-8 String \"%s\" should already be valid Modified UTF-8.\n", String);

	// No 4-byte characters, safe to convert directly
	jstr = threadEnv->NewStringUTF(String);
	if (threadEnv->ExceptionCheck())
		LOGE("Failed to convert string to Java UTF-8, got error %s.\n", GetJavaExceptionStr().c_str());
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

void DarkEdif::LOGFInternal(PrintFHintInside const char * x, ...)
{
	va_list va;
	va_start(va, x);
	LogV(DARKEDIF_LOG_ERROR, x, va);
	va_end(va);
#if _DEBUG
	fflush(stdout);
	fflush(stderr);
#endif
	__android_log_write(ANDROID_LOG_FATAL, PROJECT_TARGET_NAME_UNDERSCORES, "Killed by extension " PROJECT_NAME ".");
	if (threadEnv)
	{
#if _DEBUG
		raise(SIGINT);
#endif
		threadEnv->FatalError("Killed by extension " PROJECT_NAME ". Look at previous logcat entries from " PROJECT_TARGET_NAME_UNDERSCORES " for details.");
	}
	else
	{
		__android_log_write(ANDROID_LOG_FATAL, PROJECT_TARGET_NAME_UNDERSCORES, "Killed from unattached thread! Running exit.");
	}
	exit(EXIT_FAILURE);
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
	threadEnv->ExceptionDescribe();
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
	os << "Call stack, last function is bottommost:\n"sv;
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
		os << "  #"sv << std::setw(2) << idx << ": "sv << addr << "  "sv << (status == 0 ? outputMem : symbol) << '\n';
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
	// Don't try to catch 33, SIGLWP?, it's the signal used for dumping thrreads post-abort
};

static void my_handler(const int code, siginfo_t * const si, void * const sc)
{
	static size_t numCatches = 0;
	if (++numCatches > 3) {
		exit(0);
		return;
	}

#if DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_ERROR
	const char * signalName = "Unknown";
	for (std::size_t i = 0; i < std::size(signalCatches); ++i)
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

#if DARKEDIF_DISPLAY_TYPE > DARKEDIF_DISPLAY_ANIMATIONS
ProjectFunc void displayRunObject(JNIEnv*, jobject, jlong extPtr, jobject renderer);
ProjectFunc jobject getRunObjectCollisionMask(JNIEnv*, jobject, jlong extPtr, int flags);
ProjectFunc void getZoneInfos(JNIEnv*, jobject, jlong extPtr);
#endif

#if TEXT_OEFLAG_EXTENSION
ProjectFunc jobject getRunObjectFont(JNIEnv*, jobject, jlong ext);
ProjectFunc jint getRunObjectTextColor(JNIEnv*, jobject, jlong ext);
ProjectFunc void setRunObjectFont(JNIEnv*, jobject, jlong ext, jobject fontInfo, jobject rcPtr);
ProjectFunc void setRunObjectTextColor(JNIEnv*, jobject, jlong ext, int rgb);
#endif

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
	const std::string classNameCRun("Extensions/" "CRun" PROJECT_TARGET_NAME_UNDERSCORES);
	const std::string className("Extensions/" PROJECT_TARGET_NAME_UNDERSCORES);
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
	static JNINativeMethod methods[] = {
		method(getNumberOfConditions, "(J)I"),
		method(createRunObject, "(Ljava/nio/ByteBuffer;LRunLoop/CCreateObjectInfo;I)J"),
		method(destroyRunObject, "(JZ)V"),
		method(handleRunObject, "(J)S"),
		method(conditionJump, "(JILConditions/CCndExtension;)J"),
		method(actionJump, "(JILActions/CActExtension;)V"),
		method(expressionJump, "(JILExpressions/CNativeExpInstance;)V"),
#if PAUSABLE_EXTENSION
		method(pauseRunObject, "(J)V"),
		method(continueRunObject, "(J)V"),
#endif
#if DARKEDIF_DISPLAY_TYPE > DARKEDIF_DISPLAY_ANIMATIONS
		method(displayRunObject, "(JLOpenGL/GLRenderer;)V"),
		method(getRunObjectCollisionMask, "(JI)LSprites/CMask;"),
		// method(getRunObjectSurface, "(J)???"), // does not exist in Android :(
		method(getZoneInfos, "(J)V"),
#endif
#if TEXT_OEFLAG_EXTENSION
		method(getRunObjectFont, "(J)LServices/CFontInfo;"),
		method(getRunObjectTextColor, "(J)I"),
		method(setRunObjectFont, "(JLServices/CFontInfo;LServices/CRect;)V"),
		method(setRunObjectTextColor, "(JI)V"),
#endif
		EXTRAFUNCS
	};

	LOGV("Registering natives for %s...\n", PROJECT_NAME);
	if (mainThreadJNIEnv->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
		threadEnv = mainThreadJNIEnv;
		const std::string excStr = GetJavaExceptionStr();
		LOGF("Failed to register natives for ext %s; error %s.\n", PROJECT_NAME, excStr.c_str());
	}
	else
		LOGV("Registered natives for ext %s successfully.\n", PROJECT_NAME);
	mainThreadJNIEnv->DeleteLocalRef(clazz);

	threadEnv = mainThreadJNIEnv;

#ifdef _DEBUG
	if (!didSignals)
		prepareSignals();
#endif

	mv * mV = NULL;
	if (!Edif::SDK) {
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
		for (int i = 0; i < std::size(signalCatches); ++i)
			signal(signalCatches[i].signalNum, SIG_DFL);
	}
#endif
}

#else // iOS
#include "Extension.hpp"
#include "MMF2Lib/CTexture.h"
#include "MMF2Lib/CImage.h"
#include "MMF2Lib/CRenderer.h"
#include "MMF2Lib/CExtension.h"
#include "MMF2Lib/CRun.h"
#include "MMF2Lib/CRunApp.h"
#if MacBuild
#include "MMF2Lib/CRunViewController.h"
#endif

// Raw creation func
ProjectFunc void PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _init())
{
	mv * mV = NULL;
	if (!Edif::SDK)
	{
		LOGV("The SDK is being initialized.\n");
		Edif::Init(mV, false);
	}
}

// Last Objective-C class was freed
ProjectFunc void PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _dealloc())
{
	LOGV("The SDK is being freed.\n");
}

ProjectFunc int PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _getNumberOfConditions())
{
	return CurLang["Conditions"sv].u.array.length;
}
ProjectFunc void * PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _createRunObject(void * file, void* cobPtr, int version, void * objCExtPtr))
{
	EDITDATA * edPtr = (EDITDATA *)file;
	LOGV("Note: objCExtPtr is %p, edPtr %p.\n", objCExtPtr, edPtr);
	Extension * cppExt = new Extension((EDITDATA *)edPtr, objCExtPtr, (CreateObjectInfo*)cobPtr);
	cppExt->Runtime.ObjectSelection.pExtension = cppExt;
#if MacBuild==0
	DarkEdif::Internal_WindowHandle = ((CRunExtension*)objCExtPtr)->ho->hoAdRunHeader->rhApp->mainViewController;
#else
	DarkEdif::Internal_WindowHandle = ((CRunExtension*)objCExtPtr)->ho->hoAdRunHeader->rhApp->runViewController->window;
#endif
	return cppExt;
}
ProjectFunc short PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _handleRunObject)(void * cppExt)
{
	return (short) ((Extension *)cppExt)->Handle();
}
ProjectFunc void PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _destroyRunObject)(void * cppExt, bool bFast)
{
	delete ((Extension *)cppExt);
}
#if PAUSABLE_EXTENSION
ProjectFunc void PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _pauseRunObject)(void* cppExt)
{
	((Extension*)cppExt)->FusionRuntimePaused();
}
ProjectFunc void PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _continueRunObject)(void* cppExt)
{
	((Extension*)cppExt)->FusionRuntimeContinued();
}
#endif // PAUSABLE_EXTENSION

// Introduced in DarkEdif SDK v20
extern "C" long DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, getCPtr)(CExtension* ext);
Extension* RunObject::GetExtension()
{
	return (Extension *)DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, getCPtr)((CExtension *)this);
}

#endif // Apple


#if DARKEDIF_DISPLAY_TYPE > DARKEDIF_DISPLAY_ANIMATIONS

struct ForbiddenInternals
{
	static void * GetInternalSurface(DarkEdif::Surface* s)
	{
#ifdef _WIN32
		s->Internal_WinZoneHack();
		return s->surf;
#elif defined(__ANDROID__)
		// Android shouldn't need internal surface? but it's s->textSurface
		return nullptr;
#elif defined(__APPLE__)
		return s->bmp;
#endif
	}
	static void * GetSurfCollisionMaskNativePtr(DarkEdif::Surface* s, const std::uint32_t flags) {
		return s->GetCollisionMask(flags)->GetNativePointer();
	}
	static void* GetCollisionMaskNativePtr(DarkEdif::CollisionMask* colMask) {
		return colMask->GetNativePointer();
	}
	static void Internal_BlitToFrame(DarkEdif::Surface* s) {
		return s->BlitToFrameWithExtEffects();
	}
};

#ifdef _WIN32
short FusionAPI DisplayRunObject(RUNDATA* rdPtr)
{
#pragma DllExportHint
	Extension* const ext = ((RunObject*)rdPtr)->GetExtension();
#elif defined(__ANDROID__)
ProjectFunc void displayRunObject(JNIEnv*, jobject, jlong extP, jobject renderer)
{
	Extension* const ext = (Extension*)extP;
#else
ProjectFunc void PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _displayRunObject)(void* cppExtPtr, void* crenderer)
{
	Extension* const ext = (Extension*)cppExtPtr;
#endif

	// Tick display if ext dev has manual drawing
#if DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_MANUAL
	ext->Display();
#endif

	// If object has no ANIMATIONS, MOVEMENTS, SPRITES flags,
	// GetRunObjectSurface will be called, the returned surface drawn,
	// but not to (hoX, hoY), but to (0, 0) on frame instead. To avoid that,
	// we draw it ourselves in that scenario.
	//if constexpr ((Extension::OEFLAGS & OEFLAGS::SPRITES) == OEFLAGS::NONE)

#if DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_SIMPLE
#ifdef __ANDROID__
	// Android always manually draws
	ext->surf->BlitToFrameWithExtEffects();
#elif defined(__APPLE__)
	// We shouldn't be calling this, as CExtension.m should delegate to draw, not drawSprite,
	// meaning it calls GetRunObjectSurface first and only delegates to DisplayRunObject if it returns null
	static bool didAppleWarning = false;
	if (!didAppleWarning)
	{
		LOGE(_T("CExtension draw fix not implemented! Are you on latest iOS/Mac exporter?\n"));
		didAppleWarning = true;
	}
	ext->surf->BlitToFrameWithExtEffects();
#endif
#endif // display type

#ifdef _WIN32
	// Although Windows returns a short, Yves confirmed the return value is ignored
	return 0;
#endif
}

// Android does not implement GetRunObjectSurface
#ifndef __ANDROID__  

#ifdef _WIN32
cSurface* FusionAPI GetRunObjectSurface(RUNDATA * rdPtr)
{
#pragma DllExportHint
	Extension* const ext = ((RunObject*)rdPtr)->GetExtension();
#else // apple
ProjectFunc void * PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _getRunObjectSurface)(void* cppExtPtr)
{
	Extension* const ext = (Extension*)cppExtPtr;
#endif

	// Without BACK_SAVE, the extension smears the display
	if constexpr ((Extension::OEFLAGS & (OEFLAGS::SPRITES | OEFLAGS::BACK_SAVE)) == OEFLAGS::SPRITES)
	{
		static bool warningDone = false;
		if (!warningDone)
		{
			warningDone = true;
			LOGW(_T("Using OEFLAGS::SPRITES without BACK_SAVE will cause display smearing.\n"));
		}
	}

	void * intSurf = nullptr;
#if DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_SIMPLE
	if (!ext->surf)
		LOGF(_T("GetRunObjectSurface found Extension null surface is null.\n"));
	intSurf = ForbiddenInternals::GetInternalSurface(&*ext->surf);
#else // manual
	LOGV(_T("GetRunObjectSurface running.\n"));
	
	auto surf = ext->GetDisplaySurface();
	if (surf)
		intSurf = ForbiddenInternals::GetInternalSurface(surf);
	else // null surface delegates to DisplayRunObject
		LOGV(_T("GetRunObjectSurface got a null surface, returning null."));
#endif
#ifdef _WIN32
	return (cSurface *)intSurf;
#else
	return intSurf;
#endif

}
#endif // not Android

#ifdef _WIN32
#pragma warning (suppress: 4996)
sMask* FusionAPI GetRunObjectCollisionMask(RUNDATA * rdPtr, LPARAM flags)
{
#pragma DllExportHint
	Extension* const ext = ((RunObject*)rdPtr)->GetExtension();
#elif defined(__ANDROID__)
ProjectFunc jobject getRunObjectCollisionMask(JNIEnv*, jobject, jlong extP, int flags)
{
	Extension* const ext = (Extension *)extP;
#else // apple
ProjectFunc void * PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _getRunObjectCollisionMask)(void* cppExtPtr, int flags)
{
	Extension* const ext = (Extension*)cppExtPtr;
#endif
	if ((flags & 1) != flags)
		LOGF(_T("Unexpected flags passed to GetRunObjectCollisionMask: Expected 0 or 1, got %d.\n"), flags);
	void * nativeMask = nullptr;

	// No fine collisions? Don't even need this mask then
	if constexpr ((Extension::OEPREFS & OEPREFS::FINE_COLLISIONS) != OEPREFS::NONE)
	{
#if DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_SIMPLE
		DarkEdif::Surface* const surf = &*ext->surf;
		if (surf)
		{
			LOGV(_T("GetRunObjectCollisionMask got a non-null surface.\n"));
			nativeMask = ForbiddenInternals::GetSurfCollisionMaskNativePtr(surf, (std::uint32_t)flags);
		}
		else
			LOGV(_T("GetRunObjectSurface got a null surface, returning null.\n"));
#else // manual display
		DarkEdif::CollisionMask* const colMask = ext->GetCollisionMask((std::uint32_t)flags);
		if (colMask)
		{
			LOGV(_T("GetRunObjectCollisionMask got a non-null mask.\n"));
			nativeMask = ForbiddenInternals::GetCollisionMaskNativePtr(colMask);
		}
		else
			LOGV(_T("GetRunObjectSurface got a null mask, returning null.\n"));
#endif
	}

#ifdef _WIN32
#pragma warning (suppress: 4996)
	return (sMask*)nativeMask;
#elif defined(__ANDROID__)
	return (jobject)nativeMask;
#else // Apple
	return nativeMask;
#endif
}

#ifdef _WIN32
void FusionAPI GetZoneInfos(RUNDATA * rdPtr)
{
#pragma DllExportHint
#if DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_MANUAL
	return ((RunObject*)rdPtr)->GetExtension()->GetZoneInfos();
#endif // manual display
}
#elif defined(__ANDROID__)
ProjectFunc void getZoneInfos(JNIEnv*, jobject, jlong ext)
{
#if DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_MANUAL
	((Extension*)ext)->GetZoneInfos();
#endif // manual display
}
#else // Apple
ProjectFunc void PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _getZoneInfos)(void* ext)
{
#if DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_MANUAL
	((Extension*)ext)->GetZoneInfos();
#endif // manual display
}
#endif // Apple

// else not displaying ext
#elif defined(__WIN32)
short FusionAPI DisplayRunObject(RUNDATA* rdPtr)
{
	#pragma DllExportHint
	// TODO: My brain says DisplayRunObject is necessary regardless of whether ext
	// actually displays. Might crash when pausing and resuming ext, first created, etc.
	// Need to test that.
	return 0;
}
#endif // displaying ext
