MMF2/CF2.5 Open Source Extension Repository
===
A single repository to contain and/or link to open source extensions for MMF2/CF2.5. All extensions are compatible with Fusion 2.0 and higher, including 2.5, and 2.5+.  
These extensions are collated for examples and for upgrading; these are subject to individual licenses.  
If you plan on distributing your own version publicly, it is highly recommended you get permission from original authors, where possible.  
Since these source codes are collated by Phi, not by the authors themselves, they may be old versions, but should be suitable for demonstration.

### Tools to use ###
It's recommended you use Visual Studio 2019, although other editions can be used with loss of platform compatiblity.

You should install either the Express edition, or get the free **Community** edition, which supports VS addons.
* VS 2017 Community - Android, Windows XP+ support, no iOS, no Mac - can be downloaded under [older VS downloads][Older VS editions]. Requires a free registration.
* VS 2019 Community - iOS, Mac, Android, Windows XP+ support - can be downloaded under [VS 2019 Community DD] (direct download from MS).
* VS 2022 Community - iOS, Mac, Android, Windows Vista+ support - can be downloaded here: [VS 2022 Community DD] (direct download from MS).
* VS 2026 - Android, Windows Vista+ support, iOS\*, Mac\* - can be downloaded here: [VS 2026 Express], [VS 2026 Community]  
  \*= _Microsoft will deprecate remote building for iOS/Mac in VS 2026. If this means removal, this will no longer be usable for DarkEdif._

### Contributing ###
It is encouraged to contribute to this repository by PRs. 🙂

If you wrote something that works only on one platform, or only on MMF2 or CF2.5, you can still contribute it!  
But code it to prevent compilation on other platforms you provide projects for, or to fail at runtime when using a wrong version.

### Windows targeting ###

#### XP+ targeting ####
The SDKs are compatible with the **C++ Windows XP targeting pack**, for Windows XP+ instead of Vista+ targeting. You can find it under Additional/Individual Components tab in the Visual Studio installer; search for "Windows XP" using the search bar on the top.

If the Visual Studio XP-targeting pack is installed, it will be auto-detected by all the Fusion SDKs and XP+ targeting will be enabled by default.  
If XP targeting pack is used, your extensions will be Windows XP and later compatible, and if not, it's Windows Vista and later.  
All Fusion SDKs will use [WINVER and _WIN32_WINNT][WINVER] preprocessor macros so Windows headers will hide Vista+ functions, making them unavailable to your project.

To confirm manually if any Windows API function is compatible, you can normally find out on MSDN under "Requirements > minimum client OS" at the bottom of the page;
for example [GetProcessIdOfThread()] is Vista+.

You'll have to install v14.27 of the C++ compiler tools - this is easy to search for in Visual Studio 2019 Components tab in Visual Studio Installer. It will appear as "MSVC v142 - VS 2019 C++ x64/x86 build tools (v14.27)". This is the latest toolset that Microsoft recommend for XP targeting.

#### Later OS versions
If you want to use later OS functions at cost of compatibility, you can change the targeting by using the FusionSDKConfig INI WindowsXPCompatibility option to false ([how to][XP compatibility false]), and reloading your VS solution.

You will want to:
1. Under project properties for Windows > General, there is Windows SDK Version property; change it to a later SDK to expose newer functions.  
   You can install more Windows SDKs via the Visual Studio Installer under Additional/Individual Components tab.
2. Under project properties, change C/C++ > Preprocessor > Preprocessor Definitions to manually define WINVER and \_WIN32\_WINNT to a number from [here][WINVER].  
   By default, if not targeting XP, the chosen Windows SDK's default Windows version will be used; e.g. Windows SDK 8.1 supports compiling for earlier than Windows 8.1, but targets WINVER/\_WIN32_WINNT to Windows 8.1+ by default.
3. Under project properties Linker > System > Minimum Required Version, change it to match the WINVER.  
   XP is WINVER 0x0501; the Linker version equivalent is "5.01". So, if you want to target Vista+ (0x0600), set minimum version to "6.00".

You will also need a Windows SDK:
* Windows 7.0 SDK: If you've got the *C++ Windows XP-targeting pack* Visual Studio Installer component installed, the Windows 7.0 SDK will be installed as part of it.  
* Windows 8.1 SDK:  
  Install *Universal CRT SDK* component in Visual Studio Installer's Components tab. Then:
  * VS 2017: Add 8.1 SDK from Visual Studio Installer's Components tab.  
  * VS 2019, 2022: Install 8.1 SDK manually from the [Windows SDK archive]. You only need the first option in the installer.
* Windows 10 SDK: VS 2017-2022, install from Visual Studio Installer's Components tab.
* Windows 11 SDK: VS 2022-2026, install from Visual Studio Installer's Components tab.


### Android targeting ###
To target Android, in the Visual Studio Installer, under Individual Components tab, you need to enable "C++ Android development tools". That should be all you need.

You'll need Visual Studio 2019 or 2022. VS 2017 can't use C++17 standard on Android.

For more details on Android, such as pitfalls, features and setting up debugging, make sure you read the Android section of the [MultiTarget guide] under DarkEdif documentation.

You don't need the Fusion Android exporter to build Android extensions, but you'll be unable to test your extensions without it, and as any developer can tell you, there's no way you'll write code that works as expected first try.

### iOS/Mac targeting ####
To target iOS and/or Mac, in the Visual Studio Installer, under Individual Components tab, you need to enable "C++ iOS development tools".  
You will also need to download the free [PuTTY tools][Link to PuTTY], and set up how to use it in FusionSDKConfig.ini; the FusionSDKConfig.Template.ini file should describe how to do it.

iOS development requires you to have a Mac, with vcremote installed. Follow Microsoft's guide on linking your Visual Studio to a Mac running Xcode [here][Link VS to Xcode].  
An iMac from late 2012 or above is necessary; you need Xcode 10.2+ for compatibilty with Visual Studio, which is Mac OS Mojave and later.
Since the latest Mac OS "Big Sur" runs on 2014 and later Macs, you may want to get a 2014+ instead.

You don't need the Fusion iOS/Mac exporter to build iOS/Mac extensions, but you'll be unable to test your extensions without it, and as any developer can tell you, there's no way you'll write code that works as expected first try.

DarkEdif will automatically package your built iOS files into a encoded EXT file in the MFX\\Data\\Runtime\\iPhone folder.  
These produced files are valid, but Fusion does not look for third-party extensions, so you'll have to manually add both the framework and the code files in Xcode, which thankfully takes less than a minute.  
Likewise, DarkEdif will automatically package your built Mac files into a DAT file in the MFX\\Data\\Runtime\\Mac folder.

For more details on iOS/Mac, such as pitfalls, features and setting up debugging, make sure you read the iOS section of the [MultiTarget guide] under DarkEdif documentation.

### Using a single project ###
For exporting a single project, you will need to download the project folder, the Lib, and the Inc folder.
For example, DarkEdif Template can be extracted by downloading DarkEdif\DarkEdif Template, DarkEdif\Inc, and DarkEdif\Lib.

If you are using DarkEdif, you can use the PrepareDarkEdif tool to generate a fresh project based on the template. The tool will remove things you don't want, like translations, ANSI compatibility, other platforms, etc.

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
rSDK is a change up with even heavier use of preprocessor macros, written by James with contribution from other users. [rSDK Clickteam forum thread].  
It was introduced with extra macros that did away quite a bit of work of MMF2SDK, but still a lot of manual work and IntelliSense doesn't like it either, so you will have very inconsistent auto-complete in VS.

### Edif
Edif is a newer, C++ SDK that handles most of the messy parts of Fusion, and does away with the heavy use of preprocessor macros inherent to older SDKs. This was written by James as well, some time later. [Edif Clickteam forum thread].

### DarkEdif
DarkEdif is a continuation of Edif with changes mostly coded by Phi, with some code contributions from LB and
other developers. [DarkEdif Clickteam forum thread].

It includes all the features of Edif and some extra, including:

* Multi-language JSON file
* Object properties defined in the JSON file
* Smart properties - you can rearrange, delete, rename object properties between extension versions, and their data will updated automatically
* In debug build, a runtime check that A/C/E parameters in C++ and JSON are the same
  (only active in Debug/Debug Unicode builds; disable via defining `FAST_ACE_LINK` in project properties)
* A JSON minifier for runtime builds
* Error messages during Edif crash scenarios (e.g. missing actions)
* Extension description defined at runtime via `JSON_COMMENT_MACRO` (demo: Bluewing Client/Server, Common.h)
* Fusion debugger access via `DarkEdif::FusionDebugger` (demo: Bluewing Client/Server, Extension.cpp, Extension.h)
* An opt-in SDK and extension update checker tool

DarkEdif uses a pre-build standalone tool (and in non-Windows platforms, also a post-build standalone tool) programmed in C#. This tool is currently not open-source.

More details on DarkEdif are available in the Wiki, see:
* [DarkEdif ext dev features] for a list of features available to a DarkEdif extension developer
* [DarkEdif Fusion user features] for a list of features available to Fusion users

## How to convert ANSI functions to ANSI & Unicode
Make sure you're aware of what any text-related function you call expects. Does it ask for number of elements in an array, or number of bytes in an array?
Note TCHAR is a preprocessor definition provided by Microsoft, which will be replaced with char in ANSI builds, and wchar_t in Unicode builds.

If you don't want to provide Fusion 2.0 ANSI compatiblity, you can remove the non-Unicode project configurations from your project. Both CF2.5 and Fusion 2.0 Unicode can still use ANSI-only extensions, but both use Unicode by default if both variants of an extension are available.

* `char` -> `TCHAR`
* `strcpy()` -> `_tcscpy()`; any `strXX()` becomes `_tcsXX()`, for things like `sprintf()` you may have to google to make sure you're using the right one. If in doubt, MSDN normally provides the TCHAR equivalent.
* `sizeof(char[] variable)` -> `sizeof(TCHAR[] variable) / sizeof(TCHAR)` (for functions expecting array sizes in elements)
* `sizeof(char[] variable)` -> `sizeof(TCHAR[] variable) * sizeof(TCHAR)` (for functions expecting array sizes in bytes)
* `std::string` -> `std::tstring`, likewise for `std::stringstream` and `std::string_view`; a feature of DarkEdif, evil as it's not part of std, but a readability feature.


[XP compatibility false]: https://github.com/SortaCore/MMF2Exts/wiki/DarkEdif-ext-dev-features#disable-windows-xp-compatibility
[Older VS editions]: https://visualstudio.microsoft.com/vs/older-downloads/
[VS 2017 Community DD]: https://aka.ms/vs/15/release/vs_community.exe
[VS 2019 Community DD]: https://aka.ms/vs/16/release/vs_community.exe
[VS 2022 Community DD]: https://aka.ms/vs/17/release/vs_community.exe
[VS 2026 Express]: https://visualstudio.microsoft.com/vs/express/
[VS 2026 Community]: https://visualstudio.microsoft.com/vs/community/
[DarkEdif ext dev features]: https://github.com/SortaCore/MMF2Exts/wiki/DarkEdif-ext-dev-features
[DarkEdif Fusion user features]: https://github.com/SortaCore/MMF2Exts/wiki/DarkEdif-Fusion-user-features
[DarkEdif Clickteam forum thread]: https://community.clickteam.com/threads/71793-DarkEDIF-Taking-suggestions?p=608099&viewfull=1#post608099 "DarkEdif forum thread"
[GetProcessIdOfThread()]: https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocessidofthread#requirements
[WINVER]: https://docs.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt
[MultiTarget guide]: DarkEdif/%23MFAs%20and%20documentation/DarkEdif%20MultiTargeting.md
[Link VS to Xcode]: https://docs.microsoft.com/en-us/cpp/cross-platform/install-and-configure-tools-to-build-using-ios?view=msvc-160
[rSDK Clickteam forum thread]: https://community.clickteam.com/threads/42183-rSDK
[Edif Clickteam forum thread]: https://community.clickteam.com/threads/61692-Edif-Extension-Development-Is-Fun
[Windows SDK archive]: https://developer.microsoft.com/en-us/windows/downloads/sdk-archive/
[Link to PuTTY]: https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html

