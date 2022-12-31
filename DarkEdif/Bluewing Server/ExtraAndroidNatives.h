// This file shouldn't be included directly. It's added by Edif.General.cpp, if it exists in project folder.
// You can define your functions this way. Separate EXTRAFUNCS by commas, as if they were in the RegisterNatives list.

// Make sure in the Java file, your Example() function is called "darkedif_Example()", case sensitive.
// Static Java functions take C++ parameters JNIEnv *, jclass, non-static take JNIEnv *, jobject.
// Other parameters are appended onto both Java and C++ definitions. For examples, see the auto-generated Java files.

// Extern declaration
ProjectFunc void EndApp(JNIEnv *, jclass);

// Register native
#define EXTRAFUNCS method(EndApp, "()V")
