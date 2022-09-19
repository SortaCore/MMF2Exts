#pragma once

// Visual Studio version check
#if defined(_MSC_VER) && _MSC_FULL_VER < 191627023
	// DarkEdif SDK makes use of C++17 features. Notably, std::string literals, std::mutex, etc.
	// It also makes use of some preprocessor features like __cpp_threadsafe_static_init which
	// aren't available on VS 2015 and below, or older versions of VS 2017 (below v16.9).
	// Windows XP standard library compatibility is supported up until VS 2019 v16.7,
	// with it supported in all versions of VS 2017, so do yourself a favour and upgrade to get
	// as many C++17 features as you can.
	#error Your version of Visual Studio is too old!
#endif
#if __cplusplus < 201703L
	// Android IntelliSense uses a different code inspecting engine, triggering this warning in edit time,
	// although at compile time it may indeed be using C++17 and thus have no issues.
	#error Not running in C++17 mode
#endif

// Define std::intXX_t types
#include <stdint.h>

// Include std::string, stringstream, and related functions
#include <sstream>
// Include std::size_t
#include <stddef.h>
// Include std::string_view
#include <string_view>
// vardiac args
#include <stdarg.h>

// Activate the ""s and ""sv string literals (C++17)
using namespace std::string_literals;
using namespace std::string_view_literals;

// Include TCHAR, allowing both ANSI and Unicode functions
#include "Strings.hpp"

// Enables bitwise operations on an enum class. Basically casts to numeric type, does the op, then casts back to enum class.
#define enum_class_is_a_bitmask(EnumClassType) \
	EnumClassType constexpr static operator|(EnumClassType lhs, EnumClassType rhs) { \
		return static_cast<EnumClassType>(static_cast<std::underlying_type<EnumClassType>::type>(lhs) | static_cast<std::underlying_type<EnumClassType>::type>(rhs)); \
	} \
	EnumClassType constexpr static operator&(EnumClassType lhs, EnumClassType rhs) { \
		return static_cast<EnumClassType>(static_cast<std::underlying_type<EnumClassType>::type>(lhs) & static_cast<std::underlying_type<EnumClassType>::type>(rhs)); \
	} \
	EnumClassType constexpr static operator|=(EnumClassType &lhs, EnumClassType rhs) { \
		lhs = static_cast<EnumClassType>(static_cast<std::underlying_type<EnumClassType>::type>(lhs) | static_cast<std::underlying_type<EnumClassType>::type>(rhs)); \
		return lhs; \
	} \
	EnumClassType constexpr static operator&=(EnumClassType &lhs, EnumClassType rhs) { \
		lhs = static_cast<EnumClassType>(static_cast<std::underlying_type<EnumClassType>::type>(lhs) & static_cast<std::underlying_type<EnumClassType>::type>(rhs)); \
		return lhs; \
	}

// allows the compiler to check printf format matches parameters
#ifdef _MSC_VER
	#define PrintFHintInside _In_z_ _Printf_format_string_
	#define PrintFHintAfter(formatParamIndex,dotsParamIndex) /* no op */
#elif defined(__clang__) && !defined (__INTELLISENSE__)
	#define PrintFHintInside /* no op */
	// Where formatParamIndex is 1-based index of the format param, and dots is the 1-based index of ...
	// Note class member functions should include the "this" pointer in the indexing.
	// You can use 0 for dotsParamIndex for vprintf-like format instead.
	#define PrintFHintAfter(formatParamIndex,dotsParamIndex) __printflike(formatParamIndex, dotsParamIndex)
#else
	#define PrintFHintInside /* no op */
	#define PrintFHintAfter(formatParamIndex,dotsParamIndex) /* no op */
#endif
// Generic class for reading actions, conditions and expression parameters
struct ACEParamReader {
	virtual float GetFloat(int i) = 0;
	virtual const TCHAR * GetString(int i) = 0;
	virtual int GetInteger(int i) = 0;
};

// Flags
enum class OEFLAGS : std::uint32_t {
	NONE = 0,
	DISPLAY_IN_FRONT = 0x0001,		// Active object/window control
	BACKGROUND = 0x0002,			// Background
	BACK_SAVE = 0x0004,				// No effect in HWA
	RUN_BEFORE_FADE_IN = 0x0008,
	MOVEMENTS = 0x0010,
	ANIMATIONS = 0x0020,
	TAB_STOP = 0x0040,
	// Needs to receive window process messages (i.e. app was minimized)
	// You NEED VISUAL_EXTENSION defined to use this!
	WINDOW_PROC = 0x0080,
	VALUES = 0x0100,				// Has alterable values/strings (will automatically create the associated a/c/e/p)
	SPRITES = 0x0200,
	INTERNAL_BACK_SAVE = 0x0400,	// No effect in HWA
	SCROLLING_INDEPENDENT = 0x0800,
	QUICK_DISPLAY = 0x1000,			// No effect in HWA
	NEVER_KILL = 0x2000,			// Never destroy object if too far from frame
	NEVER_SLEEP = 0x4000,
	MANUAL_SLEEP = 0x8000,
	TEXT = 0x10000,
	DONT_CREATE_AT_START = 0x20000,
};
enum_class_is_a_bitmask(OEFLAGS);

// Flags modifiable by the program
enum class OEPREFS : short {
	NONE = 0,
	BACK_SAVE = 0x0001,		// No effect in HWA
	SCROLLING_INDEPENDENT = 0x0002,
	QUICK_DISPLAY = 0x0004,		// No effect in HWA
	SLEEP = 0x0008,
	LOAD_ON_CALL = 0x0010,
	GLOBAL = 0x0020,
	BACK_EFFECTS = 0x0040,
	KILL = 0x0080,
	INK_EFFECTS = 0x0100,
	TRANSITIONS = 0x0200,
	FINE_COLLISIONS = 0x0400,
	APPLET_PROBLEMS = 0x0800,
};
enum_class_is_a_bitmask(OEPREFS);

// These are based on the ANDROID_LOG_XXX enum.

#define DARKEDIF_LOG_VERBOSE 2
#define DARKEDIF_LOG_DEBUG 3
#define DARKEDIF_LOG_INFO 4
#define DARKEDIF_LOG_WARN 5
#define DARKEDIF_LOG_ERROR 6
#define DARKEDIF_LOG_FATAL 7
namespace DarkEdif {
	void Log(int logLevel, PrintFHintInside const TCHAR* msgFormat, ...) PrintFHintAfter(2,3);
}

#ifndef DARKEDIF_LOG_MIN_LEVEL
	#ifdef _DEBUG
		#define DARKEDIF_LOG_MIN_LEVEL DARKEDIF_LOG_VERBOSE
	#else
		#define DARKEDIF_LOG_MIN_LEVEL DARKEDIF_LOG_WARN
	#endif
#endif

#if (DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_VERBOSE)
	#define LOGV(x,...) DarkEdif::Log(DARKEDIF_LOG_VERBOSE, x, ##__VA_ARGS__)
#else
	#define LOGV(x,...) (void)0
#endif
#if (DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_DEBUG)
	#define LOGD(x,...) DarkEdif::Log(DARKEDIF_LOG_DEBUG, x, ##__VA_ARGS__)
#else
	#define LOGD(x,...) (void)0
#endif
#if (DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_INFO)
	#define LOGI(x,...) DarkEdif::Log(DARKEDIF_LOG_INFO, x, ##__VA_ARGS__)
#else
	#define LOGI(x,...) (void)0
#endif
#if (DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_WARN)
	#define LOGW(x,...) DarkEdif::Log(DARKEDIF_LOG_WARN, x, ##__VA_ARGS__)
#else
	#define LOGW(x,...) (void)0
#endif
#if (DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_ERROR)
	#define LOGE(x,...) DarkEdif::Log(DARKEDIF_LOG_ERROR, x, ##__VA_ARGS__)
#else
	#define LOGE(x,...) (void)0
#endif


// Tells the compiler not to generate any default constructor/destructor for this class
#define NO_DEFAULT_CTORS(className) \
	className() = delete; \
	~className() = delete; \
	className(className&) = delete; \
	className(className&&) = delete; \
	className(const className&) = delete; \
	className(const className&&) = delete;


struct extHeader
{
	std::uint32_t extSize,
				  extMaxSize,
				  extVersion;			// Version number
	std::uint32_t extID;				// object's identifier; null in Android/iOS!
	std::uint32_t extPrivateData;		// private data; was a pointer
};

enum class HeaderObjectFlags : std::uint16_t {
	None,
	Destroyed = 0x0001,
	TrueEvent = 0x0002,
	RealSprite = 0x0004,
	FadeIn = 0x0008,
	FadeOut = 0x0010,
	OwnerDraw = 0x0020,
	DeleteFadeSprite = 0x0040,

	NoCollision = 0x2000,
	Float = 0x4000,
	String = 0x8000
};
enum_class_is_a_bitmask(HeaderObjectFlags);

// Action and condition parameter type
enum class Params : std::uint16_t {
	Object = 1,						// ParamObject
	Time,							// ParamTime
	Border,							// ParamBorder
	Direction,						// ParamDir
	Integer,						// ParamInt
	Sample,							// ParamSound
	Music,							// ParamSound also
	Position,						// ParamPosition
	Create,							// ParamCreate
	Animation,						// ParamAnimation
	NoP,							// No struct (no data)
	Player,							// ParamPlayer
	Every,							// ParamEvery
	Key,							// ParamKey
	Speed,							// ParamSpeed
	NewPosition,					// ParamNewPosition
	Joystick_Direction, 			// ParamNewPosition also
	Shoot,							// ParamShoot
	Playfield_Zone,					// ParamZone
	System_Create,					// ParamCreate
	Expression = 22,				// ParamExpression
	// ParamComparison, also PARAM_COMPARAISON [sic] or PARAM_COMPARISON.
	// Used in conditions only. Expects condition to return an int, and user is asked to give a value expression and math op to compare with.
	// The parameter is int in C++, but can be ignored, as Fusion handles the comparison.
	// Return the int to compare with the condition function.
	Comparison,
	Colour,							// ParamColour
	Buffer,							// ParamBuffer
	Frame,							// ParamFrame - Storyboard frame number
	Sample_Loop,					// ParamSoundLoop
	Music_Loop,						// ParamSoundLoop also
	New_Direction,					// ParamNewDir
	Text_Number,					// ParamTextNum
	Click,							// ParamClick
	Program = 33,					// ParamProgram
	Old_GlobalVariable, 			// DEPRECATED - DO NOT USE
	Condition_Sample,				// ParamCondSound
	Condition_Music,				// ParamCondSound also
	Editor_Comment,					// ParamEditorComment - Event editor comment
	Group,							// ParamGroup
	Group_Pointer,					// ParamGroupPointer
	Filename,						// ParamFilename
	String,							// ParamString
	// ParamCmpTime; PARAM_CMPTIME. See Comparison above.
	// Condition should return int type (time in milliseconds to compare to), and expect C++ int parameter.
	Compare_Time,
	Paste_Sprite,					// ParamPasteSprite
	Virtual_Key_Code,				// ParamVKCode
	String_Expression,				// ParamStringExp
	// ParamCmpString; PARAM_CMPSTRING. See Comparison above.
	// Condition should return text type (const TCHAR *), and expect C++ text type parameter.
	String_Comparison,
	Ink_Effect,						// ParamInkEffect
	Menu,							// ParamMenu
	Global_Variable,				// ParamVariable
	Alterable_Value,				// ParamVariable also
	Flag,							// ParamVariable also also!
	Global_Variable_Expression, 	// ParamExpression
	Alterable_Value_Expression,		// ParamExpression also
	Flag_Expression,				// ParamExpression also also!
	Extension,						// ParamExtension
	_8Dirs,							// Param8Dirs
	Movement,						// ParamMvt
	Global_String,					// ParamVariable
	Global_String_Expression,		// ParamExpression also also also!!
	Program_2,						// ParamProgram2
	Alterable_String,				// ParamVariable
	Alterable_String_Expression,	// ParamExpression
	Filename_2,						// ParamFilename2 - allows filters of extensions or something.
	Effect,							// ParamEffect - HWA effect?
	Custom_Base = 1000,				// Base number for custom returns
};

enum class ExpParams : short {
	Long = 1,
	Integer = Long,
	Float = Integer,
	UnsignedInteger = Integer,
	GlobalVariable,
	String,
	AlterableValue,
	Flag,
};

enum class ExpReturns {
	Long = 0,
	Integer = Long,
	Float,
	String,
};

// Expression return type. Originally EXPFLAG_*, expression flags.
enum class ExpReturnType : short {
	Integer = 0x0000,
	String = 0x0001,
	// enum item originally named DOUBLE, but this is misleading; a float is returned.
	Float = 0x0002,
	UnsignedInteger = Integer
};



// Definition of conditions / actions flags
enum class EVFLAGS : std::uint16_t {
	NONE = 0,
	REPEAT = 0x0001,
	DONE = 0x0002,
	DEFAULT = 0x0004,
	DONEBEFOREFADEIN = 0x0008,
	NOTDONEINSTART = 0x0010,
	ALWAYS = 0x0020,
	BAD = 0x0040,
	BADOBJECT = 0x0080,
	DEFAULTMASK = (ALWAYS + REPEAT + DEFAULT + DONEBEFOREFADEIN + NOTDONEINSTART),
	// Originally EVFLAGS_NOTABLE
	NOTABLE = 0x0200
};
enum_class_is_a_bitmask(EVFLAGS);

// Running flags
enum class REFLAG : short {
	// OK; if used in Handle(), then indicates to call Handle next tick
	NONE = 0,
	// Don't call Handle next tick
	ONE_SHOT = 0x1,
	// Call Display after this
	DISPLAY = 0x2,
	// WndProc responses
	MSG_HANDLED = 0x4,
	MSG_CATCHED = 0x8,
	MSG_DEF_PROC = 0x10,
	// ?
	MSGRETURNVALUE = 0x40,
};

// LOGF is implemented in Windows and iOS, with Android the built-in LOGF() is used.
void LOGF(const TCHAR * x, ...);

// Useful functions
#include <thread>
#include <atomic>
#include <assert.h>
