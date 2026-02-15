#include "Common.hpp"

#ifdef __ANDROID__
#include <android/sensor.h>
#endif
///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

#ifdef _WIN32
Extension::Extension(RunObject* const _rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr) :
	rdPtr(_rdPtr), rhPtr(_rdPtr->get_rHo()->get_AdRunHeader()), Runtime(this)
#elif defined(__ANDROID__)
Extension::Extension(const EDITDATA* const edPtr, const jobject javaExtPtr, const CreateObjectInfo* const cobPtr) :
	javaExtPtr(javaExtPtr, "Extension::javaExtPtr from Extension ctor"),
	Runtime(this, this->javaExtPtr)
#else
Extension::Extension(const EDITDATA* const edPtr, void* const objCExtPtr, const CreateObjectInfo* const cobPtr) :
	objCExtPtr(objCExtPtr), Runtime(this, objCExtPtr)
#endif
{
	/*
		Link all your action/condition/expression functions to their IDs to match the
		IDs in the JSON here
	*/

	LinkAction(0, AddBlankFramesToObject);
	LinkAction(1, AddImagesToObject);
	LinkAction(2, StoreDetails);
	LinkAction(3, CheckForDiff);
	LinkAction(4, UpdateRAMUsageInfo);
	LinkAction(5, ReadSystemObjectPerms);
	LinkAction(6, IterateLastReadSystemObjectDACL);
	LinkAction(7, AddNewDACLPermToSystemObject);
	LinkAction(8, CopyAltVals);
	LinkAction(9, CopyAltStrings);

	LinkCondition(0, AlwaysTrue /* On error */);
	LinkCondition(1, IsThisFrameASubApp);
	LinkCondition(2, DoesAccHaveEffectivePerm);
	LinkCondition(3, OnNamedLoop /* On DACL entry loop */);
	LinkCondition(4, InvalidateExplicitSelection);
	//LinkCondition(1, IsEqual);

	LinkExpression(0, Error);
	LinkExpression(1, Event_Index);
	LinkExpression(2, Frame_IndexFromName);
	LinkExpression(3, Frame_NameFromIndex);
	LinkExpression(4, Active_GetAnimFrameCount);
	LinkExpression(5, Memory_PhysicalTotal);
	LinkExpression(6, Memory_PhysicalFree);
	LinkExpression(7, Memory_PageFileTotal);
	LinkExpression(8, Memory_PageFileFree);
	LinkExpression(9, Memory_VirtualTotal);
	LinkExpression(10, Memory_VirtualFree);
	LinkExpression(11, Disk_GetTotalCapacityOfDriveInMB);
	LinkExpression(12, Disk_GetAvailableSpaceOfDriveInMB);

	LinkExpression(13, TestParamsFunc);

	LinkExpression(14, GetAltValsFromObjName);
	LinkExpression(15, GetAltStringsFromObjName);
	LinkExpression(16, GetFlagsFromObjName);
	LinkExpression(17, GetAltValsFromFixedValue);
	LinkExpression(18, GetAltStringsFromFixedValue);
	LinkExpression(19, GetFlagsFromFixedValue);
	LinkExpression(20, GetCPUTemp);
	LinkExpression(21, GetLoopedACLEntry_AccountName);
	LinkExpression(22, GetLoopedACLEntry_SID);
	LinkExpression(23, GetLoopedACLEntry_AccessMask);
	LinkExpression(24, ProximitySensor);
	LinkExpression(25, GetAppRoot);
	LinkExpression(26, GetNetworkType);


	/*
		This is where you'd do anything you'd do in CreateRunObject in the original SDK

		It's the only place you'll get access to edPtr at runtime, so you should transfer
		anything from edPtr to the extension class here.

	*/

#ifdef __ANDROID__
	do {
		androidSensorManager = ASensorManager_getInstance();
		if (androidSensorManager == NULL) {
			MakeError("Proximity Sensor startup error, couldn't get sensor manager\n");
			break;
		}
		androidProximitySensor = ASensorManager_getDefaultSensor(androidSensorManager, ASENSOR_TYPE_PROXIMITY);
		if (androidProximitySensor == NULL) {
			MakeError("Proximity Sensor startup error, couldn't get default proximity sensor\n");
			break;
		}

		androidSensorEventQueue = ASensorManager_createEventQueue(androidSensorManager, ALooper_prepare(0), 1, StaticAndroidSensorCallback, this);
		if (androidSensorEventQueue == NULL) {
			MakeError("Proximity Sensor startup error, cannot create event queue\n");
			break;
		}

		/*turn on all available sensors*/
		if (ASensorEventQueue_enableSensor(androidSensorEventQueue, androidProximitySensor) != 0) {
			MakeError("Proximity Sensor startup error, cannot enable proximity sensor\n");
			break;
		}
	} while (false);
#endif
//	GetFrameNames();
}

#ifdef __ANDROID__
int Extension::StaticAndroidSensorCallback(int fd, int events, void* data)
{
	return ((Extension*)data)->AndroidSensorCallback(fd, events);
}
int Extension::AndroidSensorCallback(int fd, int events)
{
	/**
	 * For callback-based event loops, this is the prototype of the function
	 * that is called when a file descriptor event occurs.
	 * It is given the file descriptor it is associated with,
	 * a bitmask of the poll events that were triggered (typically ALOOPER_EVENT_INPUT),
	 * and the data pointer that was originally supplied.
	 *
	 * Implementations should return 1 to continue receiving callbacks, or 0
	 * to have this file descriptor and callback unregistered from the looper.
	 */
//	ASensorEventQueue_getEvents(androidSensorEventQueue, event
	ASensorEvent event = {};
	int res = ASensorEventQueue_getEvents(androidSensorEventQueue, &event, 1);
	if (res != 1) {
		MakeError("error, no pending events\n");
		return 1;
	}
	if (event.type == ASENSOR_TYPE_PROXIMITY) {
		proximityDistance = event.distance;
	}

	return 1;
}
#endif

Extension::~Extension()
{
#ifdef __ANDROID__
	if (androidSensorEventQueue != nullptr)
	{
		if (androidProximitySensor &&
			ASensorEventQueue_disableSensor(androidSensorEventQueue, androidProximitySensor) != 0) {
			MakeError("error, cannot disable sensor %s\n", ASensor_getName(androidProximitySensor));
		}
		ASensorManager_destroyEventQueue(androidSensorManager, androidSensorEventQueue);
	}
#endif
}

Extension::LastReadACL::LastReadACL()
#ifdef _WIN32
	: secDesc((SECURITY_DESCRIPTOR *)LocalAlloc(0,1),
		[](SECURITY_DESCRIPTOR * p) { LocalFree(p); return; })
#endif
{

}

void Extension::MakeError(PrintFHintInside const TCHAR* tcharFormat, ...)
{
	va_list v;
	TCHAR buffer[4096], prefix[64];

	va_start(v, tcharFormat);
	try
	{
		// Failed to output; report it
		if (_vstprintf_s(buffer, sizeof(buffer) / sizeof(*buffer), tcharFormat, v) == -1)
		{
			// Failed to report failure; try direct UI error message - we could detect failure then too but no point
			if (_stprintf_s(buffer, sizeof(buffer) / sizeof(*buffer), _T("Failed to use format \"%s\"; error %i: %s."), tcharFormat, errno, _tcserror(errno)) == -1)
				DarkEdif::MsgBox::Error(_T("Failed to report error"), _T("Double error report failure..."));
			DarkEdif::MsgBox::Error(_T("Error"), _T("%s"), buffer);
		}
		else // Error report OK; output it
		{
			// Try to output error number if possible
			prefix[0] = _T('\0');

			auto loc = DarkEdif::GetCurrentFusionEventNum(this);
			if (loc != -1)
			{
				if (_stprintf_s(prefix, sizeof(prefix) / sizeof(*prefix), _T("Event %i: "), loc) == -1)
					_stprintf_s(buffer, sizeof(buffer) / sizeof(*buffer), _T("Reading event index failed, error %i: %s."), errno, _tcserror(errno));
			}
			std::tstring error = prefix;
			error += buffer;

			errorListLock.edif_lock();
			errorList.push_back(error);
			errorListLock.edif_unlock();
			Runtime.Rehandle();
		}
	}
	catch (...)
	{
		// Failed to report failure; try direct UI error message - we could detect failure then too but no point
		if (_stprintf_s(buffer, sizeof(buffer) / sizeof(*buffer), _T("Failed to use format \"%s\"; error %i: %s."), tcharFormat, errno, _tcserror(errno)) == -1)
			DarkEdif::MsgBox::Error(_T("Failed to report error"), _T("Double error report failure..."));
		DarkEdif::MsgBox::Error(_T("Error"), _T("%s"), buffer);
	}

	va_end(v);
}

void Extension::GetFrameNames()
{
#ifdef _WIN32
	CRunAppMultiPlat& app = *rdPtr->get_rHo()->get_AdRunHeader()->get_App();
	//std::vector<TCHAR*> names;
	//int numFrames = rdPtr->rHo.AdRunHeader->App->frameMaxIndex;
	std::size_t numFrames = rdPtr->get_rHo()->get_AdRunHeader()->get_App()->GetNumFusionFrames();

	//TCHAR wow[1024] = { 0 };
	//GetFinalPathNameByHandle((HANDLE)app.file, wow, sizeof(wow), 0);
	//std::tstring filename = GetFileNameFromHandle((HANDLE)app.file);
	HANDLE cnnHandle = (HANDLE)app.file;
	unsigned long lS = GetFileSize(cnnHandle, NULL);


	// Store current position JIC Fusion gets confused
	// TODO: According to Ruthoranium, this SetFilePointer is what screws up the reading later, somehow
	// Perhaps it's a POSIX handle - no, Yves confirmed it was CreateFile
	// File_SeekCurrent?
	unsigned long lA = SetFilePointer(cnnHandle, 0L, NULL, FILE_CURRENT);
#define DieWithError(cond, closeIt, doThis) { \
		if (cond) \
		{ \
			MakeError(_T("") "FrameName collector: Line %i: Failed to "##doThis##", %u.", __LINE__, GetLastError()); \
			if constexpr(closeIt) \
				goto safeExit; \
			else \
				return; \
		} \
	}
	DieWithError(lA == INVALID_SET_FILE_POINTER, false, "get frame pointer");


	for (std::size_t i = 0; i < numFrames; ++i)
	{
		unsigned long frameDataStartPos = app.frameOffset[i];
		unsigned long l = SetFilePointer(cnnHandle, frameDataStartPos, NULL, FILE_BEGIN);
		unsigned long chunkAvail = 0UL;
		DieWithError(l == INVALID_SET_FILE_POINTER, true, "jump to Fusion frame start");

		struct Chunk {
			short ID;
			short Flags;
			int Size;
		} chunk;
		static const short CHUNK_LAST = 0x7F7F, CHUNK_FRAMENAME = 0x3335;

		do {
			DieWithError(FALSE == PeekNamedPipe(cnnHandle, NULL, 0, NULL, &chunkAvail, NULL) || chunkAvail < sizeof(chunk) || FALSE == ReadFile(cnnHandle, &chunk, sizeof(chunk), NULL, NULL), false, "reading chunk header");

			if (chunk.Size == 0)
				continue;

			if (chunk.ID == CHUNK_FRAMENAME)
			{
				TCHAR* c = (TCHAR*)_malloca(chunk.Size);
				DieWithError(c == nullptr, false, "Allocating memory");
				DieWithError(FALSE == ReadFile(cnnHandle, c, chunk.Size, NULL, NULL), false, "reading frame name");
				// TODO: This can be compressed with zlib! grab zlib and slap it into normalcy
				// int32 decompSize, compSize, then decrypt
				DieWithError((chunk.Flags & 1) != 0, false, "frank name chunk is compressed");
				DieWithError((chunk.Flags & 2) != 0, false, "frank name chunk is encrypted");
				frameNames.push_back(c);
				_freea(c);
				break;
			}
		} while (chunk.ID != CHUNK_LAST);
	}

safeExit:
	// Move pointer back
	SetFilePointer(cnnHandle, lA, NULL, FILE_BEGIN);
#undef DieWithError
#elif defined (__ANDROID__)
	MakeError("Frame names on Android not implemented yet.");
#endif
}

REFLAG Extension::Handle()
{
	if (!errorListLock.edif_try_lock())
		return REFLAG::NONE; // try next time

	while (!errorList.empty())
	{
		Runtime.GenerateEvent(0);
		errorList.erase(errorList.cbegin());
	}
	errorListLock.edif_unlock();

	// Will not be called next loop if runNextLoop is false
	return REFLAG::ONE_SHOT;
}

// These are called if there's no function linked to an ID

void Extension::UnlinkedAction(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedAction() called"), _T("Running a fallback for action ID %d. Make sure you ran LinkAction()."), ID);
}

long Extension::UnlinkedCondition(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedCondition() called"), _T("Running a fallback for condition ID %d. Make sure you ran LinkCondition()."), ID);
	return 0;
}

long Extension::UnlinkedExpression(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedExpression() called"), _T("Running a fallback for expression ID %d. Make sure you ran LinkExpression()."), ID);
	// Unlinked A/C/E is fatal error , but try not to return null string and definitely crash it
	if ((size_t)ID < Edif::SDK->ExpressionInfos.size() && Edif::SDK->ExpressionInfos[ID]->Flags.ef == ExpReturnType::String)
		return (long)Runtime.CopyString(_T(""));
	return 0;
}
