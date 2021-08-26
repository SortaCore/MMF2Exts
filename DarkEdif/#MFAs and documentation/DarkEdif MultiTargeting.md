DarkEdif Multi-Target documentation
====
DarkEdif Multi-Target SDK was proof-of-concept tested in Dec 2018, then adopted in DarkEdif v10.

Anyone familiar with DarkEdif should have no trouble working with the Multi-Target.  
It has been carefully attempted to make multiplatform compiling as simple as possible.  
There are, though, several notes.

## Prerequisites
As a DarkEdif developer, you should start by reading the README in the [MMF2Exts root folder](https://github.com/SortaCore/MMF2Exts/blob/master/README.md), to find the base requirements of using the SDK.  
In your Visual Studio setup, ensure you have enabled the latest NDK versions, and enable Android C++ targeting, and iOS C++ targeting.

## Building Fusion for Android
To set up your Fusion for Android building, refer to this guide

### Fusion's built-in native support
Buried in Clickteam's docs, there's an [NDK example](https://github.com/ClickteamLLC/android), but barely complete at all.  
It uses libRuntimeNative.so as a go-between wrapper, as well as shared STL, libc++\_shared.so.  

### libRuntimeNative.so
This NDK wrapper or go-between for Java and NDK extensions cannot be used by DarkEdif, due to lack of extensibility.  
Namely, there can't be any JNI interaction, because the wrapper never exposes the JNI pointers to the base extension. You get the minimal functionality exposed by libRuntimeNative, and just have to make do.  
Normally, SO files use JNI_OnLoad() to start off their JNI process, but dlopen() that libRuntimeNative uses internally won't call that, so even the `JNIEnv *` passed to OnLoad isn't present.

### libc++\_shared.so
The shared STL, libc++\_shared.so in the APKs/AABs. In order to make use of a shared STL, you have to match the version they use, or you'll get dlopen() errors.  
Fusion Android runtime uses r21d as of 22nd June 2021. The [latest LTS](https://developer.android.com/ndk/downloads) is r21e, latest stable r22b.  
Meanwhile, Visual Studio 2019 is using r16b, but has support for all NDK versions, [in theory](https://developercommunity.visualstudio.com/t/Update-the-bundled-Android-NDK-to-Releas/782660#T-N1452635).

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
You can calculate the trade-off yourself, by refering to [this page](https://www.appbrain.com/stats/top-android-sdk-versions).

Fusion users themselves are better off using the latest API available, presently API 30, particularly if they plan on publishing on Play Store.
However, the API 18 is the minimum that the Fusion runtime itself will support, with Java exts raising the bar as needed.

### Tools to check the API and NDK version of Android SO libraries
You can test the runtime of an SO file using Command Prompt, `python parse_elf.py "path to libc++_shared.so`.
You can get parse_elf.py from [Android source code](https://android.googlesource.com/platform/ndk/+/master/parse_elfnote.py) - click the File button on top of that page.

As far as STL, you should use LLVM's STL, c++\_shared.so, which is [the only one still supported by Google](https://www.phoronix.com/scan.php?page=news_item&px=Android-NDK-r16). GNU STL is second-best, but doesn't play nice all the time.  
If you bring in a third-party library, it must target the same shared STL, or have one/both use static, but as Fusion runtime uses a shared one, it will probalby be best to rebuild the third-party libraries manually from source.


## Building Fusion for iOS
Problems with iOS building:  
First, if you delete a file from your project, it will stay on the Mac side.  
Solution: Delete from Mac side too.  
Second, if you delete a file from Mac side, it will not be re-copied over.  
Solution: Copy them over to Mac manually.  
Third, if you remove the entire project folder, that won't be enough; it still won't be re-copied over.

### IntelliSense is out of date
IntelliSense is using a lot of old headers that don't match the current iOS SDK. To fix this, you'll need to modify Microsoft's linked headers.
from Mac: `/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr/include`  
to PC: `C:\Program Files (x86)\Microsoft Visual Studio\2019\Preview\Common7\IDE\VC\vcpackages\IntelliSense\iOS\OSS\xcodefiles`  
Then amend what VS looks in for headers:  
`C:\Program Files (x86)\Microsoft Visual Studio\2019\Preview\MSBuild\Microsoft\VC\v160\Application Type\iOS\1.0\iOS.Common.props`  
Find `<ISenseIncludePath>`, remove the libcxx and musl entries, and instead link the xcodefiles folder.  
(You could use the iPhoneOS platform's headers instead, but since you'll be debugging on the simulators, that could cause confusion as your IntelliSense will be out of sync with Xcode's autocomplete.)

### Different CPU architectures
You can build for 7 iOS CPU types, of which 4 are targeted by defaults.  
Of these CPU targets, some are simulator, some are device-only, and some are used for both.

| iOS Arch 	| VS arch 	| Default? 	| Sim? 	| Device? 	| Depends on 	| Notes/devices used in                                                                                                                                                                                                                      	|
|----------	|---------	|----------	|------	|---------	|------------	|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------	|
| armv6    	| ARM¹    	| No       	| No   	| Yes     	| armv7      	| **Deprecated**²<br>ARMv6-_only_ devices:<br>iPod Touch 1st/2nd gen<br>iPod Nano 4th gen<br>iPhone 1, iPhone 3G                                                                                                                             	|
| armv7    	| ARM     	| No       	| No   	| Yes     	| _(none)_   	| iPhone 3GS+<br>iPad Mini 1st gen+<br>iPod Touch 3rd gen+                                                                                                                                                                                   	|
| armv7s   	| ARM¹    	| No       	| No   	| Yes     	| armv7      	| Introduced in iOS 7.0.<br>Superseded by arm64 quickly, so only in:<br>iPhone 5/5C<br>iPad 4th gen<br>iPod 5th gen                                                                                                                          	|
| arm64    	| ARM64   	| Yes      	| Yes  	| Yes     	| _(none)_   	| iPhone 5S+ (not iPhone 5 or 5C)<br>iPod Touch 6th gen+<br>iPad Mini 2nd gen+<br>iPad Air 1st gen+, iPad Pro 1st gen+, iPad 5th gen+<br>In most Macs pre-2021. Used for iOS simulation in those Macs.                                       	|
| arm64e   	| ARM64¹  	| No       	| Yes  	| Yes     	| arm64      	| 2018 evolution, since ARMv8.3A, in [A12+ chips](https://en.wikipedia.org/wiki/Apple_A12).<br>iPhone XS, XS Max, XR, 11, 11 Pro<br>iPad Mini 5th gen+<br>iPad Air 3rd gen+<br>iPad 8th gen+<br>Also in 2021+ Macs. Used for iOS simulation. 	|
| i386     	| x86     	| Yes      	| Yes  	| No      	| _(none)_   	| Simulator used on 32-bit Macs.<br>Only simulator choice for iOS 6.1 and below.<br>Mac doesn't _emulate_ iOS like a VM, it just simulates it.                                                                                               	|
| x86_64   	| x64     	| Yes      	| Yes  	| No      	| _(none)_   	| Simulator used on 64-bit pre-2021 Macs.<br>(Can also be used in 2021+ Macs.³)                                                                                                                                                              	|

¹ = You will have to include the "Depends on" iOS arch and build for the VS arch if targeting those.  
² = If running Xcode under [Rosetta translation](https://developer.apple.com/documentation/apple-silicon/about-the-rosetta-translation-environment), you can run x86_64 code on an ARM64 host machine, but that's not recommnded.  
³ = Xcode will no longer build for armv6, if using iOS SDK 5.0 or above, or XCode 4.5 and above. DarkEdif technically supports it, but we make no promises all the rest of Apple's tools will play nice.

#### Including extra architectures
Apple uses extra architectures, but the table above is hard-coded into PostBuildTool.
It is worth noting that Apple's devices are backwards-compatible. More on that later.

You can include armv7s and arm64e, at the cost of some extra size in the built EXT; you'll gain some speed boosts, in theory.  
(As for armv6, it's best to exclude it, as only a few of the oldest devices use it. You should avoid it)  

As far as the DarkEdif wrapper, there are no limitations on platforms you can use; all the code is platform-agnostic.  
However, the PostBuildTool is only programmed to link the seven architectures above, if passed in the target list.

#### Excluding architectures
Excluding one of the default 4 architectures above is possible. However, Visual Studio will still build that architecture if the project configuration is built, but the EXT file generated by PostBuildTool will not include them.

The only limitation DarkEdif has with excluding is that you have at least one device architecture, and one simulator architecture.

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

For a list of all Apple's supported devices and CPU architectures, see [this link](https://en.wikipedia.org/wiki/Apple_silicon#A_series).