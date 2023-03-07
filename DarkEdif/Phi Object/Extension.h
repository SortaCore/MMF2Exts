#pragma once
#include "Edif.h"
#ifdef _WIN32
// Windows ACL permissions
#include <AclAPI.h>
#elif defined(__ANDROID__)
#include <android/sensor.h>
#else
#include <sys/uio.h>
#endif

class Extension
{
public:
	// Hide stuff requiring other headers
	//SaveExtInfo threadData; // Must be first variable in Extension class

#ifdef _WIN32
	RUNDATA* rdPtr;
	RunHeader* rhPtr;
#elif defined(__ANDROID__)
	RuntimeFunctions& runFuncs;
	global<jobject> javaExtPtr;
#else
	RuntimeFunctions& runFuncs;
	void* objCExtPtr;
#endif

    Edif::Runtime Runtime;

    static const int MinimumBuild = 251;
	// b8: Update for iOS fixes in SDK (SDK v17)
	// b7: Bugfix to alt string copying (SDK v16)
	// b6: Added alt string/value bulk copying (SDK v15)
	// b5: Smart property release (SDK v14)
	// b4: Release for FredT, added proximity sensor
	// b3: Release for Michael, added system object permission reading for Windows.
	// b2: Release to Flipswitchx with alt value by name feature.
	// b1: initial build with RAM, disk, and frame usage.
    static const int Version = 8;

	static const OEFLAGS OEFLAGS = OEFLAGS::NEVER_KILL | OEFLAGS::NEVER_SLEEP; // Use OEFLAGS namespace
    static const OEPREFS OEPREFS = OEPREFS::GLOBAL; // Use OEPREFS namespace

    static const int WindowProcPriority = 100;

#ifdef _WIN32
	Extension(RUNDATA* rdPtr, EDITDATA* edPtr, CreateObjectInfo* cobPtr);
#elif defined(__ANDROID__)
	Extension(RuntimeFunctions& runFuncs, EDITDATA* edPtr, jobject javaExtPtr);
#else
	Extension(RuntimeFunctions& runFuncs, EDITDATA* edPtr, void* objCExtPtr);
#endif
    ~Extension();


    /*  Add any data you want to store in your extension to this class
        (eg. what you'd normally store in rdPtr).

		For those using multi-threading, any variables that are modified
		by the threads should be in SaveExtInfo.
		See MultiThreading.h.

        Unlike rdPtr, you can store real C++ objects with constructors
        and destructors, without having to call them manually or store
        a pointer.
    */

	std::vector<std::tstring> errorList;
	Edif::recursive_mutex errorListLock;
	std::vector<std::tstring> frameNames;

	void GetFrameNames();

    /*  Add your actions, conditions and expressions as real class member
        functions here. The arguments (and return type for expressions) must
        match EXACTLY what you defined in the JSON.

        Remember to link the actions, conditions and expressions to their
        numeric IDs in the class constructor (Extension.cpp)
    */


	void MakeError(PrintFHintInside const char* ansiFormat, ...) PrintFHintAfter(1 + 1, 1 + 2);
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
	bool Sub_BuildTrusteeAndAccessPerms(__in std::tstring & sidOrAcc, __in std::string & argPermList,
		__out PTRUSTEE trustee, __out PSID * trusteePSID,
		__out DWORD * accessPermissions);
	bool Sub_GetTrueEffectiveRights(
		__in  PSID          pSid,
		__out PACCESS_MASK  pAccessRights);

#endif



	struct LastReadACL
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
	const TCHAR * aceLoopName;

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

		void CopyAltVals(HeaderObject* obj, int startIndex, int numVals, int destIndex);
		void CopyAltStrings(HeaderObject* obj, int startIndex, int numVals, int destIndex);

    /// Conditions

		const bool AlwaysTrue() const;
		bool IsEqual(int a, int b);
		const bool IsThisFrameASubApp();
		const bool DoesAccHaveEffectivePerm(const TCHAR * accOrSID, const TCHAR * perm);
		const bool OnNamedLoop(const TCHAR * loopName);

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
#ifdef __ANDROID__
		ASensorManager * androidSensorManager = NULL;
		const ASensor* androidProximitySensor = NULL;
		ASensorEventQueue* androidSensorEventQueue = NULL;
		static int StaticAndroidSensorCallback(int fd, int events, void* data);
		int AndroidSensorCallback(int fd, int events);
		std::atomic<float> proximityDistance;
#endif


    /* These are called if there's no function linked to an ID */

    void UnlinkedAction(int ID);
    long UnlinkedCondition(int ID);
    long UnlinkedExpression(int ID);



    /*  These replace the functions like HandleRunObject that used to be
        implemented in Runtime.cpp. They work exactly the same, but they're
        inside the extension class.
    */

    REFLAG Handle();
	REFLAG Display();

    short FusionRuntimePaused();
    short FusionRuntimeContinued();
};
