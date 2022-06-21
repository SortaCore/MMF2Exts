DarkEdif SDK changelog
====
New versions of DarkEdif are published when the changes are impactful enough that developers are expected to update their SDK.  
Due to this, editor builds of DarkEdif extensions check for SDK updates using the same extension version check routine, covered more under [the wiki](https://github.com/SortaCore/MMF2Exts/wiki/DarkEdif-ext-dev-features#sdk-update-checker).

This does not mean a game-ending bug, but if there is a lot of code updates (including cosmetic), or a bugfix that will likely affect new or current users, then a new SDK version will be released.  
Minor changes won't necessitate an immediate version update, so to get the latest DarkEdif SDK, use the latest MMF2Exts commit.

If you're editing this file, note that the use of a Markdown syntax, two spaces before line break to keep the line break without creating a new paragraph, is used heavily.  
Also note that commit SHAs are based on time and code differences, so it is impossible to know the commit SHA when writing a new version. The day of releases should be in UTC timezone.


Changes until v14 release
----
*v14 not released yet*
- Windows: fixed incompatibility with Fusion 2.0 if extension had alt values (caused crash on alt string C and later being set)
- Updated DarkEdif pre-build tool to v1.0.0.2, fixed an infinite-loop bug caused by comments
- Windows: fixed potential error on Fusion 2.0 and CF2.5 < build 293.9 if an expression generated an event (e.g. creating errors)


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

Made updater error messages visible to end users. The webserver will be smart about what errors to show, basing it on whether it's a pre-release extension, an SDK update but the user isn't the ext owner, etc.


Changes until v2 release
----
*v2 released on 31st Aug 2020, commit [31a7d45](https://github.com/SortaCore/MMF2Exts/commit/31a7d45216095646452f2722c794c033aaf71ea1)*

- Fixed the icon display in event editor when updater is in use.


v1 release
---
*v1 released on 30th Aug 2020, commit [08a9013](https://github.com/SortaCore/MMF2Exts/commit/08a901341a102af790f1b57b5b9ea6d0150892eb)*

- First SDK with updater, where a documented SDK version was more relevant.
