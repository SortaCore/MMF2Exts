MMF2/CF2.5 Open Source Extension Repository
===
A single repository to contain and/or link to open source extensions for MMF2/CF2.5. All extensions are compatible with both Fusion 2.0 and implicitly 2.5.  
These extensions are collated for examples and for upgrading; these are subject to individual licenses.  
If you plan on distributing your own version publicly, it is highly recommended you get permission from original authors, where possible.  
Since these source codes are collated by Phi, not by the authors themselves, they may be old versions, but should be suitable for demonstration.

### Tools to use ###
It's recommended you use Visual Studio 2019, although Visual Studio 2017 should also be compatible.

You should install an [Express edition](https://visualstudio.microsoft.com/vs/express/), or register for free with Microsoft to get
[Community edition](https://visualstudio.microsoft.com/vs/community/), which supports VS addons.

The SDKs are compatible with the C++ Windows XP targeting pack, for XP+ instead of Vista+ targeting. You can find it under Additional Components tab in the Visual Studio installer.



### XP targeting ###
If the XP-targeting compiler is installed it will be auto-detected by all the SDKs and used instead.  
If XP targeting pack is used, your extensions will be Windows XP and later compatible, and if not, it's Windows Vista and later.  
DarkEdif will use [WINVER and _WIN32_WINNT](https://docs.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt) preprocessor
macros so Windows headers will disable Vista+ functions, making them unavailable to your project.

To confirm if the Windows function is compatible, you can normally find out on MSDN under "minimum client OS",
for example [GetProcessIdOfThread()](https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocessidofthread) is Vista+.  
If you want to use later OS functions at cost of compatibility, you can change the targeting by switching the Platform Toolset to non-XP and
redefining WINVER and \_WIN32\_WINNT to a number from [here](https://docs.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt).

### Android targeting ###
To target Android, in the Visual Studio Installer, under Individual Components tab, you need to enable "C++ Android development tools". That should be all you need.

For more details on Android, such as pitfalls, features and setting up debugging, make sure you read the Android section of the [MultiTarget guide](DarkEdif/%23MFAs%20and%20documentation/DarkEdif%20MultiTargeting.md) under DarkEdif documentation.

You don't need the Fusion Android exporter to build Android extensions, but you'll be unable to test your extensions without it.

### iOS targeting ####
To target iOS, in the Visual Studio Installer, under Individual Components tab, you need to enable "C++ iOS development tools". That should be all you need.

For more details on iOS, such as pitfalls, features and setting up debugging, make sure you read the iOS section of the [MultiTarget guide](DarkEdif/%23MFAs%20and%20documentation/DarkEdif%20MultiTargeting.md) under DarkEdif documentation.

iOS development requires you to have a Mac, with vcremote installed. Follow Microsoft's guide on linking your Visual Studio to a Mac running XCode [here](https://docs.microsoft.com/en-us/cpp/cross-platform/install-and-configure-tools-to-build-using-ios?view=msvc-160).  
An iMac from late 2012 or above is necessary; you need XCode 10.2 for compatibilty with Visual Studio, which is Mac OS Mojave and later.
Since the latest Mac OS "Big Sur" runs on 2014 and later Macs, you may want to get a 2014+ instead.

You don't need the Fusion iOS exporter to build iOS extensions, but you'll be unable to test your extensions without it.

DarkEdif will automatically package your built Android files into a encoded EXT file in the MFX\\Data\\Runtime\\iPhone folder.  
This file is valid, but Fusion does not look for third-party extensions, so you'll have to manually add both the framework and the code files in XCode, which thankfully takes less than a minute.

### Using a single project ###
For exporting a single project, you will need to download the project folder, the Lib, and the Inc folder.
For example, DarkEdif Template can be extracted by downloading DarkEdif\DarkEdif Template, DarkEdif\Inc, and DarkEdif\Lib.
All projects in this repository use a shared Visual Studio props file, found in Lib, which will set all general vcxproj settings, automatically checking what SDK is in use. This has several effects:

1. You won't find most project settings entered in the vcxproj; you should instead open the Edit dialog for a property and view the "evaluated version".
2. If you are converting from one SDK to another, most of the work will be done for you.
3. Adding a new project configuration (example, you have Edittime, and you add Edittime Unicode), the props file will read the project configuration name and apply the settings for Edittime and Unicode; making it non-debug, adding the `_UNICODE` defines, etc.

## SDK variants
### MMF2SDK
MMF2SDK is the original MMF2SDK provided by Clickteam, programmed in C, with use of some preprocessor macros. It's a lot of manual work, and IntelliSense doesn't like it much (IntelliSense isn't great with preprocessor macros that start/end functions/sections).

### MMF2SDK_Unicode
MMF2SDK_Unicode is the MMF2SDK provided by Clickteam, with both ANSI (non-Unicode) and Unicode builds possible. Ease of use is about equal with MMF2SDK, as long as you understand [how to convert an ANSI project to ANSI & Unicode](#markdown-anchor-how-to-convert-ansi-functions-to-ansi--unicode).

### rSDK
rSDK is a change up with even heavier use of preprocessor macros, written by James with contribution from other users. [CT forum thread](https://community.clickteam.com/threads/42183-rSDK).  It was introduced with extra macros that did away quite a bit of work of MMF2SDK, but still a lot of manual work and IntelliSense doesn't like it either.

### Edif
Edif is a newer, C++ SDK that handles most of the messy parts of Fusion, and does away with the heavy use of preprocessor macros inherent to older SDKs. This was written by James as well, some time later. [CT forum thread](https://community.clickteam.com/threads/61692-Edif-Extension-Development-Is-Fun).

### DarkEdif
DarkEdif is a continuation of Edif with changes mostly coded by Phi, with some code contributions from LB and other developers. [CT forum thread](https://community.clickteam.com/threads/71793-DarkEDIF-Taking-suggestions?p=608099&viewfull=1#post608099).

It includes all the features of Edif and some extra, including:

* Multi-language JSON file
* Properties defined in the JSON file
* In debug build, a runtime check that A/C/E parameters in C++ and JSON are the same
  (only active in Debug/Debug Unicode builds; disable via defining `FAST_ACE_LINK` in project properties)
* A JSON minifier for runtime builds
* Error messages during Edif crash scenarios (e.g. missing actions)
* Extension description defined at runtime via `JSON_COMMENT_MACRO` (demo: Bluewing Client/Server, Common.h)
* Fusion debugger access via `DarkEdif::FusionDebugger` (demo: Bluewing Client/Server, Extension.cpp, Extension.h)
* An opt-in SDK and extension update checker tool

DarkEdif uses a pre-build standalone tool (and in non-Windows platforms, also a post-build standalone tool) programmed in C#. This tool is currently not open-source.

More details on DarkEdif are available in the Wiki, see:
* [DarkEdif ext dev features](https://github.com/SortaCore/MMF2Exts/wiki/DarkEdif-ext-dev-features) for a list of features available to a DarkEdif extension developer
* [DarkEdif Fusion user features](https://github.com/SortaCore/MMF2Exts/wiki/DarkEdif-Fusion-user-features) for a list of features available to Fusion users

## How to convert ANSI functions to ANSI & Unicode
Make sure you're aware of what any text-related function you call expects. Does it ask for number of elements in array, or number of bytes in array?
Note TCHAR is a preprocessor definition provided by Microsoft, which will be replaced with char in ANSI builds, and wchar_t in Unicode builds.

If you don't want to provide Fusion 2.0 ANSI compatiblity, you can remove the non-Unicode project configurations from your project. Both CF2.5 and Fusion 2.0 Unicode can still use ANSI-only extensions, but both use Unicode by default if both variants of an extension are available.

* `char` -> `TCHAR`
* `strcpy()` -> `_tcscpy()`; any `strXX()` becomes `_tcsXX()`, for things like `sprintf()` you may have to google to make sure you're using the right one. If in doubt, MSDN normally provides the TCHAR equivalent.
* `sizeof(char[] variable)` -> `sizeof(TCHAR[] variable) / sizeof(TCHAR)` (for functions expecting array sizes in elements)
* `sizeof(char[] variable)` -> `sizeof(TCHAR[] variable) * sizeof(TCHAR)` (for functions expecting array sizes in bytes)
* `std::string` -> `std::tstring`, likewise for `std::stringstream` and `std::string_view`; a feature of DarkEdif, evil as it's not part of std, but a readability feature.
