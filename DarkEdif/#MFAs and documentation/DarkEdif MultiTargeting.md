DarkEdif Multi-Target documentation
====
DarkEdif Multi-Target SDK was proof-of-concept tested in Dec 2018, then adopted in DarkEdif v10.

Anyone familiar with DarkEdif should have no trouble working with the Multi-Target.  
It has been carefully attempted to make multiplatform compiling as simple as possible.  
There are, though, several notes.

## Prerequisites
As a DarkEdif developer, you should start by reading the README in the [MMF2Exts root folder][README],
to find the base requirements of using the DarkEdif SDK.  
In your Visual Studio setup, ensure you have enabled the latest NDK versions, and enable Android C++
targeting, iOS C++ targeting, and Windows XP toolset, as needed.

## Windows project won't build

### Windows SDK issues
If you receive a warning *"The WindowsSDKDir property is not defined. Some build tools may not be found."* then
you don't have a Windows SDK installed.

This isn't Fusion's Windows SDK; it's the Windows SDK saying what the OS makes available to C++ code.  
* If you're targeting XP+, you should have the Windows 7 SDK installed for you, as part of the XP-targeting component in Visual Studio Installer.
* If you're targeting Vista+, you can manually install the Windows 8.1 SDK from the VS Installer (VS 2017) or [Windows SDK archive] (VS 2019-2022).  
  (If you use 8.1 SDK, make sure in Visual Studio Installer, you add  "Windows Universal CRT SDK" component, or the 8.1 SDK will have missing headers).  
* If you only want Windows 10+ compatibility, use Visual Studio Installer's Components tab to install Windows 10 SDK.

You can install multiple Windows SDKs alongside each other.  
This repo's SDKs are designed to use the XP SDK if it can find it; otherwise it will defer to Visual Studio,
which will by default use the latest one you have.  
You can override the default in project properties > General > Windows SDK Version, but **don't** override the XP targeting this way.  
Instead, use the FusionSDKConfig.ini, `WindowsXPCompatibility = false` setting to disable Windows XP targeting temporarily, and restart VS.

Note that the backwards-compatibility is usually present, but not guaranteed. Write your code using the oldest SDK version you can use.  
Using later OS functions will make your extension not load on older OS versions; that includes both inside Fusion on older OSes, and in built Fusion apps running on older OSes.

If you want to access later Windows OS features, such as the taskbar progress bar, but you don't want your extension to not load on older OSes,
then consider using GetProcAddress to check if the function is there at runtime, so your extension can still load if it is not.  
There are guides on how to do it online, and Lacewing uses it in a [small way][CancelIOEx Lacewing example].


## Building with Windows XP support
There are extra handicaps when compiling for XP and above. However, they're mostly about setting up your developer environment properly and take only a few minutes of changes.

### VS toolset compatibility
While VS 2015 through VS 2022 come with XP-targeting C++ toolset, the C/C++ Run-Time (CRT) is not obligated
to support XP in how its functions are implemented. The CRT includes the C++ runtime libraries that contain
all the std::XX code and built-in C/C++ functions like `printf()`.

The Windows OS SDK is correctly annotated with `#if WINVER >= VISTA` to prevent you calling Windows API
functions that aren't available in your targeted OS version. The problem is, the CRT's library could use
 anything to implement its std::XX functions.

For example, std:\:shared_timed_mutex is a basic locking mechanism to prevent multiple threads fighting over
access to the same block of memory. It is available on Windows XP and above.  
However, in later versions of VS 2019 and in all versions of VS 2022, std::shared_timed_mutex was changed to
use an OS feature called [SRW locks] to provide the mutex feature.   
SRW locks are only available in **Vista** and above. So although std::shared_timed_mutex is available while
using the XP toolset, using it will produce an application (or in this case, Fusion extension) that won't load.

To ensure your STL doesn't start falling over at random on XP, Microsoft [recommends][VS 2019 XP recommendation] using no later than Visual Studio 2019 toolset v14.27, which comes with Visual Studio 2019 Update 7 (VS v16.7).  
You can set up this old VS 2019 toolset in two ways:
1. Install the VS 2019 16.7 version directly; it should be linked in your MS account [here][VS 2019 Redist]. (if only later versions are available, open a [support ticket][VS support].)
2. Alternatively, install a later version of VS 2019, but install the v14.27 toolset under Individual Components tab of the Visual Studio Installer.  
This will produce binaries that work in XP and above, but note that IntelliSense may get confused about what \_MSC\_VER is defined as.  
It's worth noting VS 2019 Preview can be used with this method too, but VS 2022 *cannot* be used with either method.

### Tools to go with Visual Studio
Windows XP does not support running Visual Studio 2017 or above, and DarkEdif uses C++17 features that are only available in VS 2017 and above.  
Your best bet for debugging is using **WinDbg**, which can be found in the Windows 7 SDK as the "Debugging tools" component.  
Get it from [here][Windows 7 SDK Debugging Tools] or [here][Windows SDK archive].  
Install the variant that matches your XP computer; if your XP OS is 32-bit, get the 32-bit (x86) Windows 7 SDK installer.

WinDbg is fiddly to use, but you only need to know the commands:
- Menu item File > Open executable: opens the EXE file and starts running it while watching for crashes.
- `g`: Tells the program to continue until something breaks.  
  Note that WinDbg will automatically pause shortly after opening an executable, without a bug happening; this is expected behavior.
- `.dump /mFA "C:\Filename.dmp"`: produces a DMP dumpfile that Visual Studio 2017+ can open.  
  Send the DMP back to the VS computer, and use File > Open in the Fusion ext project; then press Attach in the dmp file's tab.

You may want to:
- Uninstall any redistributible before installing the Windows 7 SDK, particularly VS 2010, as you may get errors during the Windows 7 SDK install otherwise.
- Copy your symbol files from your VS computer to your XP one.  
  Symbol files (PDB files) contain the function names and addresses that are used by your extension.  
  Your symbol file cache location should be under your Visual Studio options.  
  You can set up the symbol cache in WinDbg by File > Symbol File Path (read [more][WinDbg symbol path guide]).


## My Windows ext doesn't load
If your extension compiles and links without error (Visual Studio's Output window ends with "build succeeded"), but your project does not load, in either of these ways:
1. Fusion 2.0/2.5 will not show the extension in the Create New Object window
2. Built Fusion EXEs that use your extension, when run, are creating a messagebox:  
  "Cannot load ExtName.mfx. This object might need an external program or library not yet installed."
This error means that external files are missing.

This error message box also occurs when:
- you use a Vista+ function in Windows XP, or a 7+ function in Windows Vista, etc.
- your extension is set to build with `FavorSizeOrSpeed=size_redist` Fusion SDK config INI option,
  or has VS property setting C/C++ > Code Generation > Runtime Library to a DLL version (Multithreaded Debug DLL or Multithreaded DLL).  
  `_DLL` will be defined if you've done either.  
  DLL runtime libraries store CRT functions outside of your extension's MFX, depending on the user having your VS's C++ redistributible installed.
  For VS 2015-2022, you can use any VS redistributible up to the VS 2022 redistributible; but note that Windows XP is only compatible
  up to VS 2019 redistributible v16.7.
- you use external DLLs that you link to with LIB files (dynamic library), which are not found by the running application.

Your Fusion extension will look for the external DLLs (redistributible DLL files, Windows DLL files, or third-party DLLs you've
directly linked to), in system path, and:
- During Fusion editor usage: Fusion install folder (where mmf2u.exe is)
- During Run Application: Data\\Runtime or Data\\Runtime\\Unicode folder
- For built EXEs: The built EXE application folder; and Modules sub-directory, if it's an Unpacked EXE.

If you're struggling to find the function that your ext can't find in the OS, try opening Microsoft's [Dependency Walker] on your MFX file.  
1. Once opened, it should show red-icon DLL files on the left.
2. Click on a red one, and functions with a red C icon on the top right are functions that your ext cannot find.  
3. If your ext seems fine, try opening the Walker on your Fusion EXE, and press Start Profiling.  
Regretfully, Dependency Walker only works on Windows 8 and lower; you may prefer third-party apps like [Dependencies][Dependencies tool], but they may not have the profiling option.


### CRT bug workround to do with static initialization
By default, XP-targeted Fusion SDK projects will use `/Zc:threadSafeInit-`.  
This means that static thread-local variables will not be initialized in a thread-safe way; but this will only matter
if these static variables are both function-level in scope, use constructors, and need to be thread-safe.

By default, VS uses Windows **Vista** and above functionality to initialize static local variables in a thread-safe way,
unless `/Zc:threadSafeInit-` is specified to disable this behaviour.  
When this behaviour is disabled, or you're using Vista+, or otherwise won't have a problem, `ThreadSafeStaticInitIsSafe` is defined.

Since it uses Vista+ functions, it causes buggy behaviour on Windows XP with any non-trivial initialization of static
local variables; that is, where a constructor function has to be called to set up the static, not a flat struct initialization.

static int, const char \*, char \[], etc  std:\:pair, std:\:tuple etc will construct fine; it's just non-trivial initialization like struct constructors:
```cpp
static std::string foo = "x"s;
static std::string_view yes = "no"sv;  
static structname bar = structname(XX);
```
...these will not construct properly.

Windows XP will allocate the memory for the variable, but not run the initializer, only default-constructing or zero-filling the memory.

Static variables on a file-level scope don't use this, and so will construct with no issues.

Consider using wraparound code to disable static behaviour.
```cpp
#ifdef ThreadSafeStaticInitIsSafe
	// Function-level static struct initialization is okay
	static std::string foo = "x"s;
#else 
	// Broken function-level scope static initialization; construct a non-static instead
	std::string foo = "x"s;
#endif
```

For an example of this workaround, see the Edif::Init() function.


## Building Fusion for Android
To set up your Fusion for Android building, refer to this guide

### Fusion's built-in native support
Buried in Clickteam's docs, there's an [NDK example][Clickteam NDK example], but it's barely complete at all.  
It uses libRuntimeNative.so as a go-between wrapper, as well as shared STL, libc++\_shared.so.  

### libRuntimeNative.so
This NDK wrapper or go-between for Java and NDK extensions cannot be used by DarkEdif, due to lack of extensibility.  
Namely, there can't be any JNI interaction, because the wrapper never exposes the JNI pointers to the base extension. You get the minimal functionality exposed by libRuntimeNative, and just have to make do.  
Normally, SO files use JNI_OnLoad() to start off their JNI process, but dlopen() that libRuntimeNative uses internally won't call that, so even the `JNIEnv *` passed to OnLoad isn't present.

### libc++\_shared.so
The shared STL, libc++\_shared.so in the APKs/AABs. In order to make use of a shared STL, you have to match the version they use, or you'll get dlopen() errors.  
Fusion Android runtime uses r21d as of 22nd June 2021. The [latest LTS][latest NDK LTS] is r21e, latest stable r22b.  
Meanwhile, Visual Studio 2019 is using r16b, but has support for all NDK versions, [in theory][Visual Studio NDK compatibility discussion].

Note that attempts to use Fusion's built-in shared STL even with correct versions, result in SIGSEGV on app start, so presently DarkEdif uses a static runtime even in the Runtime configuration.  

### Android API levels
Finally, the Fusion built-in SO files target Android API:
- 16 (Android 4.1) for whatever isn't 64-bit
- 21 (Android 5.0) for 64-bit; the first 64-bit Android API  
  
This API choice isn't a big problem, more a limitation, as Android OS is backward-compatible to some extent even with the NDK libraries, but you will lose a lot of APIs, including several basic ones.

Consider using API 19, which gives you 4.4.x+, most API 16 devices will be upgradable to OS 4.4.4. Even if not:
- OS v4.0 is on 0.1% of Android devices
- OS v4.1-4.3 is on 0.5% of Android devices
- OS v4.4 is on 1.5% of Android devices  
You can calculate the trade-off yourself, by refering to [this page][AppBrain Android OS usage].

Fusion users themselves are better off using the latest API available, presently API 30, particularly if they plan on publishing on Play Store.
However, the API 18 is the minimum that the Fusion runtime itself will support, with Java exts raising the bar as needed.

### Tools to check the API and NDK version of Android SO libraries
You can test the runtime of an SO file using Command Prompt, `python parse_elf.py "path to libc++_shared.so`.
You can get parse_elf.py from [Android source code][Parse ELF py file] - click the File button on top of that page.

As far as STL, you should use LLVM's STL, c++\_shared.so, which is the only one [still supported by Google][Google's STL support]. GNU STL is second-best, but doesn't play nice all the time.  
If you bring in a third-party library, it must target the same shared STL, or have one/both use static, but as Fusion runtime uses a shared one, it will probalby be best to rebuild the third-party libraries manually from source.


## Building Fusion for iOS
Problems with iOS building:

Problem 1: if you delete a file from your project, it will stay on the Mac side.  
Solution: Delete from Mac side too.  

Problem 2: if you delete a file from Mac side, it will not be re-copied over.  
Solution: Copy them over to Mac manually. If you remove the entire project folder, that won't be enough; it still won't be re-copied over.

Problem 3: if you don't include a file in your VS project or shared project, it won't be copied over, regardless of whether it's a code, header or resource file.
Solution: Add them to your project.

### iOS development setup
An iMac late 2012 or above is necessary; you need XCode 10.2+, which is OS Mojave+.  
Mac OS Big Sur and Montgomery are both compatible, as is the latest Xcode.

If you want to support 32-bit devices, including ARMv7 and x86 simulator, you will need **XCode 13.4.1 or earlier** [from here][Xcode versions].  
Xcode below 14 can still targets iOS 10.0+, and iOS 10.0 is the last to support 32-bit CPUs.  
Note that if you have later, 32-bit incompatibility will not remove the iOS VS project configuration, and you must customise FusionSDKConfig.ini so the PostBuildTool does not expect those CPU arches.

The earliest iOS you can simulate in latest XCode - noted in July 2021 - is iOS 11.4, which runs on iPhone 5S, but not 5 or 5C.
You can install it under XCode config > Components > Simulators.

Install Xcode 13.4.1 on Mac. Set up SSH on Mac.
PuTTY setup. Validate by running PuTTY. Recommended to use encryption keys. Link guides.



### IntelliSense is out of date
If you're inundanted with a ton of IntelliSense errors, then read this section. Visual Studio's IntelliSense is using a lot of old iOS headers that don't match
Xcode's SDK. To fix this, you'll need to modify Microsoft's linked headers. Copy:  
from Mac: `/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr/include`  
to PC: `C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\VC\vcpackages\IntelliSense\iOS\OSS\xcodefiles`  
(If you're not using VS 2019 Community, then your PC path may vary.)

Next step is to amend what folders VS IntelliSense looks in for headers, so open:  
`C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Microsoft\VC\v160\Application Type\iOS\1.0\iOS.Common.props`  
Find `<ISenseIncludePath>`, remove the libcxx and musl entries, and instead link the xcodefiles folder; and also link the xcodefiles\c++\v1 folder.  
You may have to open an editor in Administrator mode, or grant the Users role modify permissions, as the props file will be read-only to Users.  

As a side note, you could use the iPhoneOS platform's headers instead of the simulator's headers, but since you'll be debugging on the simulators,
that may cause yourself some confusion as your IntelliSense will be out of sync with Xcode's IntelliSense equivalent.

### Different CPU architectures
You can build for 7 iOS CPU types, of which 4 are targeted by defaults.  
Of these CPU targets, some are simulator, some are device-only, and some are used for both.

| iOS Arch 	| VS arch 	| Default? 	| Sim? 	| Device? 	| Depends on 	| Notes/devices used in                                                                                                                                                                                 	|
|----------	|---------	|----------	|------	|---------	|------------	|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------	|
| armv6    	| ARM¹    	| No       	| No   	| Yes     	| armv7      	| **Deprecated**²<br>ARMv6-_only_ devices:<br>iPod Touch 1st/2nd gen<br>iPod Nano 4th gen<br>iPhone 1, iPhone 3G                                                                                        	|
| armv7    	| ARM     	| No       	| No   	| Yes     	| _(none)_   	| iPhone 3GS+<br>iPad Mini 1st gen+<br>iPod Touch 3rd gen+                                                                                                                                              	|
| armv7s   	| ARM¹    	| No       	| No   	| Yes     	| armv7      	| Introduced in iOS 7.0.<br>Superseded by arm64 quickly, so only in:<br>iPhone 5/5C<br>iPad 4th gen<br>iPod 5th gen                                                                                     	|
| arm64    	| ARM64   	| Yes      	| Yes  	| Yes     	| _(none)_   	| iPhone 5S+ (not iPhone 5 or 5C)<br>iPod Touch 6th gen+<br>iPad Mini 2nd gen+<br>iPad Air 1st gen+, iPad Pro 1st gen+, iPad 5th gen+<br>In most Macs pre-2021. Used for iOS simulation in those Macs.  	|
| arm64e   	| ARM64¹  	| No       	| Yes  	| Yes     	| arm64      	| 2018 evolution, since ARMv8.3A, in [A12+ chips].<br>iPhone XS, XS Max, XR, 11, 11 Pro<br>iPad Mini 5th gen+<br>iPad Air 3rd gen+<br>iPad 8th gen+<br>Also in 2021+ Macs. Used for iOS simulation. 		|
| i386     	| x86     	| Yes      	| Yes  	| No      	| _(none)_   	| Simulator used on 32-bit Macs.<br>Only simulator choice for iOS 6.1 and below.<br>Mac doesn't _emulate_ iOS like a VM, it just simulates it.                                                          	|
| x86_64   	| x64     	| Yes      	| Yes  	| No      	| _(none)_   	| Simulator used on 64-bit pre-2021 Macs.<br>(Can also be used in 2021+ Macs.³)                                                                                                                         	|

¹ = You will have to include the "Depends on" iOS arch and build for the VS arch if targeting those.  
² = If running Xcode under [Rosetta translation], you can run x86_64 code on an ARM64 host machine, but that's not recommnded.  
³ = Xcode will no longer build for armv6, if using iOS SDK 5.0 or above, or Xcode 4.5 and above. DarkEdif technically supports it, but we make no promises all the rest of Apple's tools will play nice.

#### Including extra architectures
Apple uses extra architectures, but the table above is hard-coded into PostBuildTool.
It is worth noting that Apple's devices are backwards-compatible. More on that later.

You can include armv7s and arm64e, at the cost of some extra size in the built EXT; you'll gain some speed boosts, in theory.  
(As for armv6, it's best to exclude it, as only a few of the oldest devices use it.)  

As far as the DarkEdif wrapper, there are no limitations on platforms you can use; all the code is platform-agnostic.  
However, the PostBuildTool is only programmed to link the seven architectures above, if passed in the target list.

#### Excluding architectures
Excluding one of the default 4 architectures above is possible. However, Visual Studio will still build that architecture if the project configuration is built, but the EXT file generated by PostBuildTool will not include them.

The only limitation DarkEdif has with excluding is that you have at least one device architecture, and one simulator architecture.

### My extension's files are missing from built Fusion iOS projects!
DarkEdif will automatically package your built iOS files into a encoded EXT file in the MFX\\Data\\Runtime\\iPhone folder.  
This file is valid, or Fusion wouldn't build at all, but the Fusion does not automatically add third-party extensions. So while it will include your ext's files in the built iOS project zips, the Xcode project won't have them referenced.  
You'll have to manually add both the framework and the code files to the Xcode project, which thankfully takes less than a minute.
1. Open your Xcode project in Xcode.
2. Right-click the Extension folder in your Xcode project, and press Add Files.
3. Select the extension's files to add them; they should be in your project\\Extensions folder.  
   They will normally be named CRun**ExtName**.xxx - Xcode should also highlight the files that your project hasn't linked already.
4. After adding those, right-click your Frameworks folder and pick Add Files again
5. This time, under your project folder\\Frameworks, add the xcframework package. Don't add the contents; add the xcframework itself.

#### Notes

##### Making small applications?
Apple will strip out unnecessary architectures from downloaded apps, when you upload to their App Store, so you needn't be frugal about them. Include extra architectures if you think it will speed up your app enough to be worth it.

##### Order of build
The architectures will be built for simulator and/or device, whatever is available for that arch.
Simulators builds will be started as a separate build after the arch's regular device build, if the arch includes both.

The variant builds will be built after the ones they depend on; for example, arm64e will be built after both device and simulator build of arm64.

#####  Backwards compatibilty
Apple has backwards-compatible devices. armv6 code will run on armv7 and arm64 devices, and armv7 code will run on arm64 devices, albeit with some slowdown.  
Similarly, armv7 code will run on armv7s devices, and armv7/arm64 code will run on arm64e devices.

##### Unchanged output file on failure
If a build fails for one architecture, the EXT (and decoded ZIP) are left as they are, with the old version still in the XCFramework.
This is due to the huge amount of tests and edge scenarios that would need covering to make sure there was always a functional, up-to-date EXT file that reflected the C++ code.
As a simple example, if you only build armv7, and it fails, should PostBuildTool discard the EXT and ZIP file, even though it might have your own modifications to the CPP/H/OBJ-C files?

It is recommended you always build for all your targeted architectures when you build at all, to prevent an inconsistent result across devices. Best to get in the habit earlier rather than later.  
You can build for all architectures by right-clicking the solution file, selecting Batch Build and selecting the desired project configurations.

For a list of all Apple's supported devices and CPU architectures, see [this link][Apple CPU A series].


[README]: https://github.com/SortaCore/MMF2Exts/blob/master/README.md "MMF2Exts README"
[SRW locks]: https://docs.microsoft.com/en-us/windows/win32/sync/slim-reader-writer--srw--locks "Slim Reader-Writer Locks"
[VS 2019 Redist]: https://my.visualstudio.com/Downloads?q=Visual%20C%20Redistributable%20Visual%20Studio%202019%20version%20%2216.7%22 "VS Dev Essentials VS2019 v16.7 C++ redistributables"
[VS support]: https://my.visualstudio.com/GetHelp?mkt=en-us "VS Dev Essentials support"
[VS 2019 XP recommendation]: https://docs.microsoft.com/en-us/cpp/porting/binary-compat-2015-2017?view=msvc-160#:~:text=support%20Windows%20XP-,is%20version%2016.7
[Windows 7 SDK Debugging Tools]: https://www.microsoft.com/en-gb/download/details.aspx?id=8279
[Windows SDK archive]: https://developer.microsoft.com/en-us/windows/downloads/sdk-archive/
[WinDbg symbol path guide]: https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/setting-symbol-and-source-paths-in-windbg#symbol-path
[Dependency Walker]: https://www.dependencywalker.com/
[Dependencies tool]: https://github.com/lucasg/Dependencies/releases
[Clickteam NDK example]: https://github.com/ClickteamLLC/android
[Latest NDK LTS]: https://developer.android.com/ndk/downloads
[Visual Studio NDK compatibility discussion]: https://developercommunity.visualstudio.com/t/Update-the-bundled-Android-NDK-to-Releas/782660#T-N1452635
[AppBrain Android OS usage]: https://www.appbrain.com/stats/top-android-sdk-versions
[Parse ELF py file]: https://android.googlesource.com/platform/ndk/+/master/parse_elfnote.py
[Google's STL support]: https://www.phoronix.com/scan.php?page=news_item&px=Android-NDK-r16
[Xcode versions]: https://xcodereleases.com
[A12+ chips]: https://en.wikipedia.org/wiki/Apple_A12
[Rosetta translation]: https://developer.apple.com/documentation/apple-silicon/about-the-rosetta-translation-environment
[Apple CPU A series]: https://en.wikipedia.org/wiki/Apple_silicon#A_series
[CancelIOEx Lacewing example]: https://github.com/SortaCore/MMF2Exts/blob/master/DarkEdif/Lib/Shared/Lacewing/src/windows/fdstream.c#:~:text=GetProcAddress
