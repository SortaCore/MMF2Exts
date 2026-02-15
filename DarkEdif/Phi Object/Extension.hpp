#pragma once
#include "Edif.hpp"
#ifdef _WIN32
// Windows ACL permissions
#include <AclAPI.h>
#elif defined(__ANDROID__)
#include <android/sensor.h>
#else
#include <sys/uio.h>
#endif

class Extension final
{
public:
	// ======================================
	// Required variables + functions
	// Variables here must not be moved or swapped around or it can cause future issues
	// ======================================
	RunHeader* rhPtr;
	RunObjectMultiPlatPtr rdPtr; // you should not need to access this
#ifdef __ANDROID__
	global<jobject> javaExtPtr;
#elif defined(__APPLE__)
	void* const objCExtPtr;
#endif

	Edif::Runtime Runtime;

	static const int MinimumBuild = 251;
	// b9: Update for iOS fixes, and new app root expression (SDK v20)
	// b8: Update for [intended] iOS fixes in SDK (SDK v17)
	// b7: Bugfix to alt string copying (SDK v16)
	// b6: Added alt string/value bulk copying (SDK v15)
	// b5: Smart property release (SDK v14)
	// b4: Release for FredT, added proximity sensor
	// b3: Release for Michael, added system object permission reading for Windows.
	// b2: Release to Flipswitchx with alt value by name feature.
	// b1: initial build with RAM, disk, and frame usage.
	static const int Version = 9;

	static constexpr OEFLAGS OEFLAGS = OEFLAGS::NONE;
	static constexpr OEPREFS OEPREFS = OEPREFS::NONE;

#ifdef _WIN32
	Extension(RunObject* const rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr);
#elif defined(__ANDROID__)
	Extension(const EDITDATA* const edPtr, const jobject javaExtPtr, const CreateObjectInfo* const cobPtr);
#else
	Extension(const EDITDATA* const edPtr, void* const objCExtPtr, const CreateObjectInfo* const cobPtr);
#endif
	~Extension();

	// ======================================
	// Extension data
	// ======================================

	std::vector<std::tstring> errorList;
	Edif::recursive_mutex errorListLock;
	std::vector<std::tstring> frameNames;

	void GetFrameNames();

	void MakeError(PrintFHintInside const TCHAR* tcharFormat, ...) PrintFHintAfter(1 + 1, 1 + 2);
	void StripSpaces(std::string & str);
	void StripUnderscores(std::string & str);
	void MakeStringLower(std::string & str);
#ifdef _WIN32
	void StripSpaces(std::wstring & str);
	void MakeStringLower(std::wstring & str);
#endif

#ifdef _WIN32
	static GENERIC_MAPPING objectTypeMappings[];

	static std::string GetLastErrorAsString();
	// Converts passed text parameters to a TRUSTEE and access perms
	bool Sub_BuildTrusteeAndAccessPerms(std::tstring & sidOrAcc, std::string & argPermList,
		PTRUSTEE trustee, PSID * trusteePSID,
		DWORD * accessPermissions);
	bool Sub_GetTrueEffectiveRights(PSID pSid, PACCESS_MASK  pAccessRights);
#endif // _WIN32

	struct LastReadACL final
	{
		// User parameter: ACL item path
		std::tstring itemPath;

#ifdef _WIN32
		// Deduced from user parameter: system object type, e.g. SE_OBJECT_TYPE::SE_REGISTRY_KEY
		SE_OBJECT_TYPE readObjectType = SE_OBJECT_TYPE::SE_UNKNOWN_OBJECT_TYPE;

		// Security descriptor; the main content of the deal
		// This is the only one that needs freeing, using LocalFree(), we make it automatic
		// via a custom deleter variant of unique_ptr.
		std::unique_ptr<SECURITY_DESCRIPTOR, void (*)(SECURITY_DESCRIPTOR*)> secDesc;

		// Bitfield of all the security details included in secDesc security descriptor
		SECURITY_INFORMATION securityInfo = 0;

		// Stored within secDesc, no need to free.
		PSID owner = NULL;
		// Primary group. Stored within secDesc, no need to free.
		PSID primaryGroup = NULL;

		// (DACL) An access control list that is controlled by the owner of an object and that specifies the access particular users
		// or groups can have to the object. Stored within secDesc, no need to free.
		PACL dacl = NULL;
		// (SACL) An ACL that controls the generation of audit messages for attempts to access a securable object.
		// The ability to get or set an object's SACL is controlled by a privilege typically held only by system administrators.
		// Stored within secDesc, no need to free.
		PACL sacl = NULL;

#endif
		// Set up custom deleters
		LastReadACL();
	} lastReadPerms;

	// User-supplied name of ACL entry loop
	const TCHAR * aceLoopName = nullptr;

#ifdef _WIN32
	// Current access control list entry being looped through.
	ACCESS_ALLOWED_ACE * currentLoopAce = NULL;
#endif


	/// Actions

	std::int32_t physMemTotalMB = 0;
	std::int32_t physMemFreeMB = 0;
	// could be system max available, or process assigned max
	std::int32_t pageFileMemTotalMB = 0;
	std::int32_t pageFileMemFreeMB = 0;
	std::int32_t virtualMemTotalMB = 0;
	std::int32_t virtualMemFreeMB = 0;

	void UpdateRAMUsageInfo();
	void ReadSystemObjectPerms(const TCHAR * itemPathPtr, const TCHAR * itemTypePtr, int includeSystemACL);

	void AddBlankFramesToObject(int objectFV,
		int animNum, int dirNum,
		int numOfFrames, int insertIndexAt);
	//void AddBlankFramesToObject(int objectFV, int numOfFrames, int insertAtIndex);
	void AddImagesToObject(int objectFV,
		int animNum, int dirNum,
		const TCHAR * filenames, int insertIndexAt);
	void StoreDetails(int objectFV);
	void CheckForDiff();
	void Sub_AddImagesAtIndex(RunObject * runObj, std::vector<unsigned short> imgIDs,
		size_t animNum, size_t animDirNum, size_t insertAtIndex /* = MAXSIZE_T */);

	void IterateLastReadSystemObjectDACL(const TCHAR * loopName, const TCHAR * allowDenyBoth, int includeInheritedInt, int includeInheritOnlyInt);
	void AddNewDACLPermToSystemObject(const TCHAR * sidOrAccPtr, const TCHAR * allowDenyRevokePtr, const TCHAR * permListPtr, const TCHAR * inheritPtr);

	void CopyAltVals(RunObject * obj, int startIndex, int numVals, int destIndex);
	void CopyAltStrings(RunObject * obj, int startIndex, int numVals, int destIndex);

	/// Conditions

	const bool AlwaysTrue() const;
	bool IsEqual(int a, int b);
	const bool IsThisFrameASubApp();
	const bool DoesAccHaveEffectivePerm(const TCHAR * accOrSID, const TCHAR * perm);
	const bool OnNamedLoop(const TCHAR * loopName);
	bool InvalidateExplicitSelection();

	/// Expressions

	const TCHAR * Error();
	int Event_Index();
	int Frame_IndexFromName(const TCHAR * name);
	const TCHAR * Frame_NameFromIndex(int index);

	int Active_GetAnimFrameCount(int fixedValue, int animNum, int animDir);
	int Memory_PhysicalTotal();
	int Memory_PhysicalFree();
	int Memory_PageFileTotal();
	int Memory_PageFileFree();
	int Memory_VirtualTotal();
	int Memory_VirtualFree();
	std::uint32_t Disk_GetTotalCapacityOfDriveInMB(const TCHAR * path);
	std::uint32_t Disk_GetAvailableSpaceOfDriveInMB(const TCHAR * path);
	const TCHAR * GetAltValsFromObjName(const TCHAR* objectName, int altValueIndex, int numDecimalDigits);
	const TCHAR* GetAltStringsFromObjName(const TCHAR* objectName, int altStringIndex, const TCHAR* delim);
	const TCHAR* GetFlagsFromObjName(const TCHAR* objectName, int flagIndex);

	const TCHAR * GetAltValsFromFixedValue(int fixedValue, int altValueIndex, int numDecimalDigits);
	const TCHAR * GetAltStringsFromFixedValue(int fixedValue, int altStringIndex, const TCHAR * delim);
	const TCHAR * GetFlagsFromFixedValue(int fixedValue, int flagIndex);
	int GetCPUTemp();

	const TCHAR * GetLoopedACLEntry_AccountName();
	const TCHAR * GetLoopedACLEntry_SID();
	int GetLoopedACLEntry_AccessMask();


	float TestParamsFunc(int a, float b, const TCHAR* c, float d, int e, float f, const TCHAR* g, int h, float i,
		const TCHAR* j, int k, float l, const TCHAR* m, float n, int o, const TCHAR* p);

	float ProximitySensor();
	const TCHAR* GetAppRoot(int flags);
	const TCHAR* GetNetworkType();

#ifdef __ANDROID__
	ASensorManager * androidSensorManager = NULL;
	const ASensor* androidProximitySensor = NULL;
	ASensorEventQueue* androidSensorEventQueue = NULL;
	static int StaticAndroidSensorCallback(int fd, int events, void* data);
	int AndroidSensorCallback(int fd, int events);
	std::atomic<float> proximityDistance;

	jmethodID getActiveNetworkInfoMethod = NULL;
	global<jstring> contextConnectivityString;
	global<jobject> connectivityService;
#endif

	// Runs every tick of Fusion's runtime, can be toggled off and back on
	REFLAG Handle();

	// These are called if there's no function linked to an ID
	void UnlinkedAction(int ID);
	long UnlinkedCondition(int ID);
	long UnlinkedExpression(int ID);
};
