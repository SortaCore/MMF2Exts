## Java to C calls and lookups (JNI)
Details on Java to C function calls:
```java
native void mystuff(int b); // C++ native function in Java file
```
C version:
```cpp
// static Java func calling this C function
extern "C" JNIEXPORT void JNICALL mystuff(JNIEnv *, jclass javaClass, int b); 
// non-static Java func calling this C function
extern "C" JNIEXPORT void JNICALL mystuff(JNIEnv *, jobject javaInstancePtr, int b);
```

Register the functions in RegisterNatives, by adding them to the ExtraAndroidNatives.h file. A demonstration exists in Bluewing with `EndApp()` function.

Java has no concept of a non-class-based function, same as C#.
The first parameter for a C function called by Java is `JNIEnv *`. The second parameter is `jclass` for static Java functions, and
`jobject` for non-static Java functions. You can ignore parameters by not giving them a C name, as shown above.  

For Java string to C memory, use `threadEnv->GetStringUTFChars()`, and `ReleaseStringUTFChars()`.  
If you want to return text to Java, remember Java text is a `jobject`, not a C string `TCHAR *` and not a C++ string `std::string`. You can allocate it with  `CStrToJStr(cstr)`.

## To call C to Java (JNI)
Look up Java method ID (function ID) with `threadEnv->GetMethodID()`, or field ID (Java variables), then invoke on the jclass.  
See `javaExtPtr` member of Extension for accessing your Java wrapper's functions and variables.  
Method/Field IDs do not change at runtime, so you can cache them; *but* if you're using an inherited class that may be different methods per class, so assume the ID may be different too.

Strings are objects in Java. For C string to Java memory, see `CStrToJStr(cstr)`. Remember to free it after use, as the jstring is a jobject local ref.

References of objects and classes are by default JNI local refs, and will expire when the Java caller that created them finishes. To make it a global ref instead, the easiest way is to store it in the DarkEdif global template, e.g. `global<jobject>`, `global<jclass>`, and put that in Extension or where appropriate.  
While it's not needed to free local refs manually, as they'll be freed by expiry, it is recommended practice.

Failed Java functions that generate Java exceptions (e.g. a missing field ID) will put a java exception in storage, and usually return null/0.  
If the exception is still there when you do another JNI call, a CPU abort will happen, killing the app.

You can read exception with DarkEdif's `GetJavaExceptionStr()` for a nice read of the exception message.  
You can also use `JNIExceptionCheck()` to kill app with log lines and details if no exception is expected. Add one after any Java call for free, as they are no-op in release build config.  
Also note `threadEnv->HasExceptionOccurred()`, `threadEnv->ClearException()`.

## Debugging in VS
Since adb versions will kill each other, to keep VS debugging working when running adb logcat or other commands, you should run VS's adb. It will be running during debug, so a quick look with Process Explorer should give you the full path. Open that folder in Command Prompt and use adb normally.

To set up debug for your phone:
* Go into phone's system settings, advanced, find the Android OS build number, and tap on it multiple times to unlock Developer options.
* Go into developer options, enable USB debugging.
* Plug in your phone into the computer via USB. If VS is running ADB, or you do, the phone will immediately ask whether to trust the computer: hit OK.  
(It's worth noting that if your phone screen dies, a lot of the ways to recover files rely on ADB trusting your computer; consider leaving it permanently trusted and never turning off USB debug mode.)
* Once desktop is trusted, see if you can find your device next to the green Debug button in VS. If not, switch the CPU arch until it shows.
  (Restart VS if your phone does not show on any config, or if you have no pop-up on your phone requesting trust. Most phones are ARM64.)
* You may have to allow app installs from unknown sources, which will be a setting in your phone.

To set up debug for an APK:
* Build the APK.
* Open VS project properties. Go on Debugging section.
* Confirm your phone is plugged in and trusted if it does not show as Debug Target.
* Select the APK file under Package to Launch.
* Press OK; other settings should be fine unchanged.

For best debugging experience, build your phone's CPU variant in Debug.  
This will wrap it into MFX folder under Data\Runtime\Android.  
It is worth noting that **no** checks are done to make sure all arches exist in the Android zip, or that all arches are built in same config.

To update your test app when you change your code:
* Build your phone's CPU variant.
* Copy the MFX contents into your Fusion folder.
* Re-build your MFA.
* Copy your new APK into your phone.
* Install the new APK in your phone; it should consider it an upgrade.
There is a batch file for automating all these steps into a single button press. You'll have to customise it for your app.  
It expects Ctrl-B to be the keybind for building Fusion.

When releasing a full version, right-click your solution in VS and press Batch Build. Select all four CPU arches under Runtime configs; probably you want to select the Windows ones too.  
While debug is _possible_ in Release (Runtime) config, there's almost no information outside the log, so don't even try.


## CPU exceptions
1. SIGABRT will happen and point to no code if Java has an exception and you don't clear it before the next Java call. So if you call Java func A() and then B(), and A() has an exception, A() will return without C++ exception, but once you call B() you get instant SIGABRT.  
   Use `JNIExceptionCheck()`, which does nothing in release (Runtime) builds.
2. SIGABRT will also happen if you completely misuse Java, like FindClass("A B"), spaces aren't
   allowed in class names. This will happen instantly, not on next call.
3. SIGTRAP will happen if you have a function returning a value, but you have no return statements.
   The app will generally freeze up.
4. SIGBUS is another common error. It's similar to SIGSEGV, but means you used an impossible memory address,
   instead of an invalid one.  
   (Check return and parameter types of jni calls.)
5. If you're unable to use the debugger completely, check the API package you're using in project settings.  
	Setting it closer to your phone model may work better.
6. It is worth noting a breakpoint you set is SIGTRAP or SIGINT. In C, it's `raise(SIGINT)`.  
   It may not allow you to continue running app after a breakpoint, so rely on logging with LOGV and co.

### Enable more JNI checking
`adb shell setprop debug.checkjni 1`        (this is device-wide)  
Add `android:debuggable` to app manifest    (this is app-specific)

Read log of Fusion/ext:  
`adb logcat -s MMFRuntime:V MMFRuntimeNative:V libc:V dalvikvm:E AndroidRuntime:I System.err:V DEBUG:V Extension_Name:V`
* dalvikvm/System.err/DEBUG will report crash data on different models, useful during JNI as in some cases it will give you the correct JNI signature.
* libc is not of much use, but can give you some crash data.
* MMFRuntime is the usual Java Android runtime for Fusion.
* MMFRuntimeNative are events created by native (usually non-DarkEdif) extensions.

