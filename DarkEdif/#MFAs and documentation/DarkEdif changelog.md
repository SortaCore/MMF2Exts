DarkEdif SDK changelog
====
New versions of DarkEdif are published when the changes are impactful enough that developers are expected to update their SDK.  
Due to this, editor builds of DarkEdif extensions check for SDK updates using the same extension version check routine, covered more under [the wiki](https://github.com/SortaCore/MMF2Exts/wiki/DarkEdif-ext-dev-features#sdk-update-checker).

This does not mean a game-ending bug, but if there is a lot of code updates (including cosmetic), or a bugfix that will likely affect new or current users, then a new SDK version will be released.  
Minor changes won't necessitate an immediate version update, so to get the latest DarkEdif SDK, use the latest MMF2Exts commit.

If you're editing this file, note that there is use of a Markdown syntax, two spaces before line break to keep the line break without creating a new paragraph.  
Some editors will consider this to be trailing whitespace and remove it; make sure yours doesn't.  
Also note that commit SHAs are based on time and code differences, so it is impossible to know the commit SHA when writing a new version. The date of release should be in UTC timezone.

Changes until v20 release
----
*v20 not released yet*
- DarkEdif: Renamed `Edif::Runtime::GetOIFromObjectParam()` to `GetOIListIndexFromObjectParam()`, added `RunHeader::GetOIListIndexFromOi()`.
- PrepareDarkEdif: Now v1.0.0.4; focuses project name, and adds controls for target MFX differing to project name.
- PostBuildTool: Now v1.0.0.9; allows target MFX name to differ to project name, and checks for mismatched wrapper on Android + iOS
- PreBuildTool: Now v1.0.0.9; allows target MFX name to differ to project name, and checks for mismatched wrapper on Mac
- All platforms: Added support for target MFX filename to differ to project name, using FusionSDKConfig ini.
- PrepareDarkEdif: Now v1.0.0.3; fixes version in wrapper files put in project folder, now includes preceding "v".
- PostBuildTool: Now v1.0.0.8; iOS no longer mistakenly looks for custom hpp file twice instead of h and hpp file in project folder
- PreBuildTool: Now v1.0.0.8; aborts if you use non-numbers in a LinkA/C/E, which would break the call table generation for non-Windows
- Windows: Fixed alt string setting, and getting a blank alt string
- Edititme: Standardized string property handling, fixed blank string properties causing a warning popup
- All platforms: Fixed new bug in v19, New Direction and Time parameters mistakenly being interpreted as comparison conditions, causing conditions to sometimes be true when false was returned.
- iOS/Mac: Updated PreBuildTool and PostBuildTool to v1.0.0.7, fixed New Direction parameter being incorrectly read in iOS/Mac wrapper files.  
  This bug was copied from a mistake in Android/iOS/Mac Fusion runtime. The SDK had Android already fixed.
- Edittime: Fixed a new Visual Studio XP compatiblity bug, causing the object property readers to use uninitalized memory.  
  If it's safe to use a static struct constructor, `ThreadSafeStaticInitIsSafe` is defined (on all platforms).
- Edittime: Fixed crash when smart properties were upgraded and a new checkbox property was added, while an existing checkbox property was renamed.
- Edittime: Added once-per-MFA property upgrade popup, instead of once for every frame with ext.  
  Default smart property upgrade now results in popup if it has to reset any properties to JSON values, once per MFA only.


Changes until v19 release
----
*v19 released on 27th Feb 2024, commit [ce7afe2](https://github.com/SortaCore/MMF2Exts/commit/ce7afe2499b2fe049f401da4b5ac5f301418e0d8)*
- All platforms: Added a multiplatform, C++ style object selection framework, replacing Riggs framework with DarkEdif.
- All platforms: Added multiplatform-safe functions for the built-in variables used in object selection.  
Functions reflecting underlying variables exactly will be named get_XX() for variable XX, whereas those that may be
moved around in platforms (e.g. CRun variables moved to CEventProgram) are named GetXX().
- All platforms: Object parameters now pass OI number as `int` if condition, first object instance `RunObject *` if action.  
  You can prevent Fusion's automatic instance loop by calling `Runtime.CancelRepeatingObjectAction()`, or leave as is for every selected object instance `RunObject *` to be passed one by one to your action func.
- All platforms: Added RunObjectMultiPlatPtr which is `std::shared_ptr<RunObject>` on Android, and raw `RunObject *` pointer on others.  
Similarly, added CRunAppMultiPlat, CRunFrameMultiPlat; iOS/Mac implement them with that name, the others typedef to original names.
- Windows: Locked out the ext dev from accessing Fusion internal variables on Windows, as it's not portable.  
  If you want to mess with internals and there are no helper funcs, define `FUSION_INTERNAL_ACCESS` on project level.
- Windows: Locked out the ext dev from constructing/destructing Fusion internal structs.  
  If you need them, look for helper funcs first; otherwise use `(class *)malloc(sizeof(class))` or `std::unique_ptr<char[]>` equivalent.
- All platforms: Renamed most .h files to .hpp, to accurately reflect their internal C++ content, as opposed to C.  
  Lacewing Blue noteably uses a mix of C and C++ files, and without a CompileAs override, VS assumes the h files are C.
- iOS/Mac: Will now override code files' CompileAs to build with Objective-C for C files, and Objective-C++ for C++ files.
- iOS/Mac: Added library headers for internal variables; access via helper functions, or at your own risk.  
IntelliSense hates Objective-C and can't recognise Obj-C classes/functions, so you may cause false errors by using internals.
- iOS/Mac: Introduced a FusionInternals namespace to add helper functions while preventing conflicts between extensions.
- Edittime: Added a PrepareDarkEdifSDK tool (v1.0.0.2), so new extensions can be made in a couple minutes,  
  without making breaking inconsistencies (e.g. ext name and project folder name being different will break the BuildTools)
- Windows: Fixed GetRunningApplicationPath() when passed AppFolderOnly flag; would return drive letter by itself
- Android: Fixed GetJNIEnvForThisThread()'s strange declarations. `threadEnv` is preferred
- Windows: Added CF2.5+/Direct3D 11 specific cSurface code
- Edittime: Fixed new objects' custom EDITDATA variables being cleared in DarkEdif::DLL::DLL_CreateObject(), when their init should happen just before that
- Windows: Renamed Runtime.IsHWA() to better reflect it doesn't indicate HWA of app, but HWA of underlying runtime; added Runtime.GetAppDisplayMode().
- All platforms: Updated DarkEdifPreBuildTool to v1.0.0.6, has ParamTime, ParamObject and event line reading fixes
- All platforms: Updated DarkEdifPostBuildTool to v1.0.0.6, has ParamTime, ParamObject and event line reading fixes
- iOS/Mac: PreBuildTool no longer uses FusionSDKConfig global INI setting over project-specific INI setting.
- Mac: PreBuildTool will now copy whole Lib\Mac folder, instead of specifically MMF2Lib.framework.
- Edittime: JSON parser now ignores UTF-8 BOM if present; assumes UTF-8 if not. Other BOMs will cause parser abort.
- Edittime: Color and Edit direction properties now work
- Edittime: Fix a crash when making error messages for properties in JSONPropertyReader
- Android/iOS/Mac: Added more TCHAR function defines, strtol variants
- All platforms: Fixed EventGroup being invalid after a generated event
- All platforms: Fixed expression functions interfering with normal selection, causing infinite loops (ActionLoop) and crashes in some circumstances.  
Noteably, DarkScript would infinite loop when you created 2+ objects inside an event of a fastloop running 3+ times, then ran a DS function by action. The culprit was ActionLoop being reset by generated event's action, causing the same obj instance to be revisited each time control returned to the fastloop event.


Changes until v18 release
----
*v18 released on 31st Mar 2023, commit [81164af](https://github.com/SortaCore/MMF2Exts/commit/81164af0a252393e58ee0ec93d9a5fd20e1a5e62)*
- Mac: Added Mac exporter compatibility, iOS based, uses some compilation edits by the DarkEdif tools to target Mac instead.  
  To use it:
  1. Duplicate your iOS vcxproj file and filter file.
  2. Name the files XX.Mac instead of XX.iOS.
  3. Crack it open in a text editor. Change the internal GUID in a text editor. (VS has a menu item Tools \> Create GUID)
  4. Change the project name to XX.Mac inside the project.
  5. Delete the ARM and x86 configurations at the top. Each should take 4 lines each.
  6. Move the Import of FusionSDK_AfterMSSheets.props from the top of the file (approx line 33) to just before ImportGroup of ExtensionTargets, just before end of file.
  7. Modify the Import of AfterMSSheets to add `Condition="$(Platform)=='x64' AND '$(IsSecondRun)'!='1'"`. Refer to DarkEdif Template Mac vcxproj if you're not sure how.  
  8. Copy the ClInclude of the two Mac files - CRun$(ExtNameUnderscores).hpp, and CRun$(ExtNameUnderscores).mm, from the DarkEdif Template. You should copy 4 lines doing this.  
  9. Modify the vcxproj.filters and add the two CInclude/ClCompile files, so they are put nicely in the right folder.
  10. Any third-party libraries will need building for Mac (not Mac Catalyst, which is iOS-running-on-Mac). You can link with MacExtraLibraries in FusionSDKConfig.ini.
- Windows: Fixed issues that caused projects using C++20 not to work
- Windows: Fixed float parameters being corrupted when it's an action/condition taking 1-2 parameters  
  (bug introduced by SDK v15's A/C optimization)
- Android/iOS/Mac: (hopefully) fixed projects seen as building successfully when PostBuildTool failed  
  (this happens commonly wth iOS/Mac, as all iOS CPU archs must be built before PostBuildTool can combine them into a xcframework)
- iOS: Fixes from v17 was not actually implemented in the tool and this was elusive during tests
- iOS/Mac: PreBuildTool updated ~~to v1.0.0.4~~ (EXE accidentally still v1.0.0.3), now reads FusionSDKConfig.ini for modifying pbxproj, and generates/copies Mac code files
- Android/iOS/Mac: PostBuildTool updated to v1.0.0.5, now reads FusionSDKConfig.ini for expected OS archs/version instead of FusionSDK.props passing it; Mac compatiblity added


Changes until v17 release
----
*v17 released on 7th Mar 2023, commit [160c20b](https://github.com/SortaCore/MMF2Exts/commit/160c20bcf391b4ac26da45c042cf472177b3c419)*
- HTML5/UWP: Added HTML5/UWP template files for DarkEdif Template
- All platforms: Moved functions and global variables including ::SDK into Edif/DarkEdif namespace, as iOS was merging multiple exts' global functions/variables
- All platforms: Added support for New_Direction parameter (the 32-direction picker wheel).
- All platforms: PostBuildTool now v1.0.0.4; template files now support New_Direction parameter.
- All platforms: Fixed DarkEdif property ID error reporting in GetPropertyNum and GetPropertyStr
- All platforms: Changed default log level in Debug builds to info, not verbose. It gets old reading A/C/E parameter logs.
- Edittime: Fixed UCT Fix Tool links in messageboxes causing crash, due to presence of %20 in a sprintf function
- Edittime/Windows: Added invalid parameter handler if none present to prevent sprintf and related issues, in Edittime/Runtime configs
- Android/iOS: PostBuildTool libcrypto/libssl hack removed; now reads Additional Platform Files.txt from project directory, used to add to zip during Android/iOS building.
- Android: Fixed char not being signed by default on ARMv7 arch, due to a strange compiler default. This may affect iOS too.
- iOS: ~~PostBuildTool has altered Objective-C wrapper so multiple DarkEdif extensions can co-exist in one iOS MFA.~~  
  This was intended, and frustratingly, was the main reason DarkEdif SDK v17 was released, but the PostBuildTool's internal Objective-C files were not updated for that; this was done in PostBuildTool v1.0.0.5, i.e. SDK v18.
- iOS: Edif and Extension are now #defined to include extension name (with underscores), so they are not shared on iOS


Changes until v16 release
----
*v16 released on 31st Dec 2022, commit [9790d06](https://github.com/SortaCore/MMF2Exts/commit/9790d061263939e19bc015e5a6d254d216bd6160)*
- All platforms: Added DarkEdif::MakePathUnembeddedIfNeeded() to pull binary data files into temporary files
- Android/iOS: PostBuildTool now v1.0.0.3; template files modified for the embedded file feature
- Android/iOS: PostBuildTool now checks for updates to Android/iOS wrapper files in the C++ project folder
- Android: Fixed freeze in text expression when returning invalid UTF-8
- Edittime: Improved reporting for link A/C/E errors occurring in multiple languages
- iOS: Fixed library lookup so libraries can be linked to by C++ side; automatic adding of libraries to iOS build zips is not yet implemented
- iOS: PostBuildTool explicitly adds libcrypto and libssl to iOS built zips


Changes until v15 release
----
*v15 released on 25th Nov 2022, commit [0d3bbbd](https://github.com/SortaCore/MMF2Exts/commit/0d3bbbd95fde6e259dc545ea5f21d3db314341ae)*
- Windows: Optimized A/C calls when 2 parameters or less, E calls when 1 parameter or less
- Windows: Expanded ObjectSelection to use oiList parameters - hat tip to defisym
- All platforms: Added GetPropertyNum for reading numeric properties. While they were stored fine, they couldn't be read.
- All platforms: Added Time action/condition parameter type (turns into number)
- Edittime: Fixed a bug with reading JSON checkbox properties defaults, where every other checkbox property would be false.
- Edittime: Added a Icon.png bit depth check. When a minified PNG has less than 8-bit color depth, Fusion's PNG filter won't load it.
- Windows: Fixed object selection in Fusion 2.0 under HWA, Unicode and other scenarios
- All platforms: Renamed INTENSE_FUNCTIONS_EXTENSION to DARKSCRIPT_EXTENSION, and modified its effects
- All platforms: Removed SuppressZeroArraySizeWarning, as it doesn't work; warning 4200 is now set to be ignored globally
- Published FusionSDKConfig.ini template for users who want to customize their experience


Changes until v14 release
----
*v14 released on 7th July 2022, commit [90c1990](https://github.com/SortaCore/MMF2Exts/commit/90c199053b9b9e99769522cdf5ab0182a369aab1)*
- All platforms: Added smart property system; properties can now be altered between extension versions easily
- Edittime: Fixed update checker reporting to user when server sends no content. This is caused by server being down.  
	Server down is by default considered a "no update" scenario. This behaviour can be overriden by DarkEdif.ini setting.  
	When server sends garbage, it's reported to user though, as it's abnormal; whereas server being down, while uncommon, is normal.
- Windows: fixed incompatibility with Fusion 2.0 if extension had alt values (caused crash on alt string C and later being set)
- Updated DarkEdif pre-build tool to v1.0.0.2, fixed an infinite-loop bug caused by comments
- Windows: fixed potential error on Fusion 2.0 and CF2.5 < build 293.9 if an expression generated an event (e.g. creating errors)
- Windows: Fixed Direct3D 11 enum value in Surface


Changes until v13 release
----
*v13 released on 18th February 2022, commit [fee5f1a](https://github.com/SortaCore/MMF2Exts/commit/fee5f1a6c0dd9ec988926fb96b2184f1de42c6cb)*
- Edittime/Windows: several crash fixes to UC tagging feature (testing wasn't done, evidently)
- Edittime/Windows: removed debug popup related to RunMode
- Windows: Added errors when invalid Windows SDK 8.1/7.0 configuration is detected
- Windows: Fixed some Visual Studio property display issues (Label attribute in PropertyGroup)
- Android/iOS: Added current event number reading to mobile platforms - DarkEdif::GetCurrentFusionEventNum()
- Android/iOS: Fixes to generated template file inside PostBuildTool.
- Edittime: Fixed text property updating, would potentially corrupt or crash after 2+ edits
- All platforms: Added Editbox String Multiline property
- DarkEdif Template: Added PrepareAndroidBuild() example for accessing manifest mid-build


Changes until v12 release
----
*v12 released on 11th December 2021, commit [5f82ae9](https://github.com/SortaCore/MMF2Exts/commit/5f82ae9cb8c219ecc3df6e05e1c85a4eea391598)*
- Edittime/Windows: Windows XP CRT bug worked around, now fully supports XP
- All platforms: Improved VS 2017 compatibilty
- Edittime: DarkEdif update checker runs in background, no longer runs for Fusion startup screen, and will usually not report server-side errors
- Edittime: Release of DarkEdif update checker ext database tool (database is shared with all versions of SDK, including older ones)
- Edittime/Windows: Added UC tagging feature, to enable Runtime MFX files to be unique per Fusion developer, hopefully preventing antiviruses blocking the wheat with the chaff.
- All platforms: Moved ACE menu handling to Edif.Edittime.cpp, since it's Windows only.
- All platforms: DarkEdif::RunMode added - clearly tell when your MFX is being used, e.g. Frame Editor, Run Application, built EXE, Fusion splash screen, etc.
- All platforms: deprecated Edif::IsEdittime and Edif::IsFusionStartupRun. See previous item.
- All platforms: Added DarkEdif::EndsWith(), FileExists(), RemoveSuffixIfExists(), GetRunningApplicationPath(), GetMFXRelativeFolder()
- All platforms: Added a DarkEdif::MsgBox::Custom() function for manually passing icon/default button/etc; mostly has effect on Windows
- All platforms: Repaired MsgBox function to have extension name
- All platforms: Renamed jump A/C/E functions:
  - Windows: Edif::Action is now Edif:\:ActionJump, Edif::Condition and Edif::Expression, ditto for C/E
  - Android: darkedif\_action() is now darkedif\_actionJump(), ditto for C/E
  - iOS: extName\_action() is now extName\_actionJump(), ditto for C/E
  - DarkEdif PostBuildTool reflects these changes in its internal Objective-C++/Java templates
- iOS: properties now passed to extension, changed iOS function name \_free() to \_dealloc()
- Windows: DisableThreadLibraryCalls() optimization disabled for static CRT, following [MSDN docs](https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-disablethreadlibrarycalls)
- Windows: Removed General.cpp files; functions are now in Edif.General.cpp
- Windows: Edittime.cpp for all exts updated due to MsgBox functions
- All platforms: Renamed ambiguous Extension functions:
  - Extension::Save is now SaveFramePosition
  - Extension::Load is now LoadFramePosition
  - Extension::Pause is now FusionRuntimePaused
  - Extension::Continue is now FusionRuntimeContinued  
(It should probably be FusionRuntimeResumed, but I would rather keep Continue in the name so it's not hard to search for.)
- Edittime: Fixed edit button properties that crashed Fusion editor or had invalid text.
- All platforms: renamed macro which granted bitwise ops to enum classes from "fancyenumop" to "enum_class_is_a_bitmask"
- DarkEdif Template: Added warning inside WndProc function so DisplayRunObject is used.


Changes until v11 release
----
*v11 released on 26th August 2021, commit [36f8378](https://github.com/SortaCore/MMF2Exts/commit/36f8378d9ef6404420d480d53983c755491f53ac)*
- MultiTarget SDK integration into DarkEdif Template, providing Android and iOS compatibility - iOS still requires separate Mac with Xcode (latest Xcode on Big Sur is compatible)
- Added post-build tool for multiplatform version, allowing auto-packaging of built Android SO/iOS A files
- Updated pre-build tool to multiplatform version, fixing the constantly-rebuilding problem, and writing function calltable on build for the A/C/E functions, for ABIs that lack ASM equivalents
- Windows XP compatibility improved for external libraries by defining \_WIN32\_WINNT in project properties instead
- Worked around a Visual Studio 2019 Preview bug that caused Android compilations to fail (reported [here](https://developercommunity.visualstudio.com/t/shared-android-project-with-spaces-in-path-wont-co/1478150) )
- Made DarkEdif Template's icon file size smaller using TinyPNG [online service](https://tinypng.com/)
- Modified pre-build tool to report poorly formed DarkExt JSON files, where A/C/E parameter types are present, but names are missing
- Added DarkEdif MultiTargeting.md documentation file, tips about how to work with mobile C++ development and debug efficiently. Not completed yet. May be moved into the MMF2Exts wiki later.
- Added DARKEDIF_LOG_XXX compile macros, allowing multiplatform logging functions that can be removed at compile time. Modelled after the Android log levels.

Changes until v10 release
----
*v10 released on 11th June 2021, commit [357036f](https://github.com/SortaCore/MMF2Exts/commit/357036fa10c8d3293b447ca4cd27d0bc52bfaff6)*
- Fixed Edif::Condition for comparison condition parameters, uses long instead of int, and returns a text result correctly.
- Removed UNICODE undef. Some Windows APIs like AdvAPI.h use UNICODE define instead of \_UNICODE. Enums with a member UNICODE now use UNICODE\_ member instead.
- Removed excess newlines in template Conditions.cpp


Changes until v9 release
----
*v9 released on 10th May 2021, commit [9935197](https://github.com/SortaCore/MMF2Exts/commit/99351979ffba98ed4417c4007b581acbeab70c5a)*
- Adapted LinkConditionDebug for comparison condition parameters. They work a bit strangely, making conditions return the integer/text variable that Fusion runtime will compare.  
These were also updated for clearer variable names, and use of type comparisons instead of converting types to string and comparing after. Lost of const was also documented.  
- MMFMasterHeader: Fixed rCom::rcSprite mistakenly being Spr instead of Spr \*, found by Uppernate  
- Moved changelog into this file instead of inside DarkEdif.h.  
- Added runtime check so Run Application won't run the ext update checker
- Clarified ext icon blit/create error messages to include ext name
- DarkEdif Template JSON file: writing was formalized a bit


Changes until v8 release
----
*v8 released on 8th March 2021, commit [1ad988b](https://github.com/SortaCore/MMF2Exts/commit/1ad988b0715c46ce3b045958de5ea961dd6dd99a)*
- Optimised LinkACE debug functions. Cleaned up the JSON parser, including better error messages and fixing a 1-byte overflow.  
- Fixed static analysis and Windows XP clash, atomic header clash, pre-build tool not liking space-indented JSON.


Changes until v7 release
----
*v7 released on 1st December 2020, commit [28b0c08](https://github.com/SortaCore/MMF2Exts/commit/28b0c089dfeafc69bd00f1c0bc47801f085fb318)*
- Replaced the charset-converting code std::string input parameter with std::string_view.


Changes until v6 release
----
*v6 released on 14th Sept 2020, commit [7548374](https://github.com/SortaCore/MMF2Exts/commit/7548374fa9400b18196465a9e430e32240ed8912)*
- Removed SDK::EdittimeProperties in runtime builds; it's only needed to read the property value and type via JSON in runtime.


Changes until v5 release
----
*v5 released on 9th Sept 2020, commit [e86745c](https://github.com/SortaCore/MMF2Exts/commit/e86745c6cdc32af36e8bd6eeb011bb4b04788c43)*
- Added DarkEdif::GetEventNumber. Now sets XP targeting when XP compiler is used.  
- Pre-build tool now allows multiline-declared ACE functions. Fixed combo box property's initial value.  
- Fixed sub-expressions causing wrong expression return type (corrupting float expression responses).


Changes until v4 release
----
*v4 released on 4th Sept 2020, commit [9181958](https://github.com/SortaCore/MMF2Exts/commit/918195897fcdc229d535d229972b3ac734c73fb5)*
- Added a new type of updater reply for a nice message to ext dev, instead of one that includes all updater log.  
- Switched update thread spawn and wait to directly invoking the updater function.


Changes until v3 release
----
*v3 released on 1st Sept 2020, commit [3d4cc24](https://github.com/SortaCore/MMF2Exts/commit/3d4cc2470c6cf0c562608620cc31979b506986a4)*
- Made updater error messages visible to end users. The webserver will be smart about what errors to show, basing it on whether it's a pre-release extension, an SDK update but the user isn't the ext owner, etc.


Changes until v2 release
----
*v2 released on 31st Aug 2020, commit [31a7d45](https://github.com/SortaCore/MMF2Exts/commit/31a7d45216095646452f2722c794c033aaf71ea1)*
- Fixed the icon display in event editor when updater is in use.


v1 release
---
*v1 released on 30th Aug 2020, commit [08a9013](https://github.com/SortaCore/MMF2Exts/commit/08a901341a102af790f1b57b5b9ea6d0150892eb)*
- First SDK with updater, where a documented SDK version was more relevant.
