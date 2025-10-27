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
#if __cplusplus < 201703L && !defined(__INTELLISENSE__)
	// Android IntelliSense uses a different code inspecting engine, triggering this warning in edit time,
	// although at compile time it may indeed be using C++17 and thus have no issues.
	#error Not running in C++17 mode
#endif

// Normally FusionSDK.props turns __cpp_threadsafe_static_init off with /Zc:threadSafeInit- and defines ThreadSafeStaticInitIsSafe
#ifndef ThreadSafeStaticInitIsSafe
	// The threadsafe init is disabled, or we're using VS 2017, so it's safe.
	// Or we're targeting Vista+ (WINVER >= 0x0600), threadsafe init works there, so safe too.
	#if !defined(__cpp_threadsafe_static_init) || _MSC_VER < 1920 || WINVER > 0x0503
		#define ThreadSafeStaticInitIsSafe 1
	#endif
// We're still targeting XP, and it's VS 2019, why don't we have the define?
// Assume Vista+ otherwise, because we don't want to import windows.h here
#elif defined(__cpp_threadsafe_static_init) && (defined(WINVER) && WINVER <= 0x0503) && _MSC_VER > 1920 && _MSC_FULL_VER < 193030705
	#error Static constructors cannot be used on XP; this should be disabled with /Zc:threadSafeInit-
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
	} \
	EnumClassType constexpr static operator~(EnumClassType &en) { \
		en = static_cast<EnumClassType>(~static_cast<std::underlying_type<EnumClassType>::type>(en)); \
		return en; \
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

// Editor and runtime flags telling Fusion runtime your ext capacity,
// and what its RUNDATA should implement. Note that in DarkEdif, all RUNDATA
// layout is handled for you, with most functions passed to Extension.
enum class OEFLAGS : std::uint32_t {
	NONE = 0,

	// If you set this, DarkEdif will know your ext must run its display functions
	// However, consider adding SPRITES, MOVEMENTS, BACKGROUND etc instead.
	MANUAL_DISPLAY_EXT = 0,

	// If you set this, DarkEdif will attempt to manage your ext as a single-surface,
	// auto-drawing-on-edit object. If you want something more complicated,
	// use the manual display mode.
	// This expects the use of SPRITES and BACK_SAVE flags.
	// You cannot use this with ANIMATIONS.
	SIMPLE_DISPLAY_EXT = 0,

	// Indicates editor should display this object in front of others.
	// Recommended if you are displaying a sub-window and it will be drawn that way at runtime.
	// @remarks Used for child-window objects like buttons, editbox, list,
	//			as Windows OS draws them as a separate borderless "window",
	//			so Actives cannot be drawn overlapping the button/editbox/etc.
	DISPLAY_IN_FRONT = 0x0001,

	// Background object, very infrequent or no display changes.
	// When this is changed, the entire window is redrawn.
	BACKGROUND = 0x0002,

	// Makes the runtime save and restore the background of the object.
	// Only has effect in Windows Software display modes.
	// Expects OEPREFS::BACK_SAVE
	BACK_SAVE = 0x0004,

	// Runs CreateRunObject before frame transition frame-in.
	// Extensions should not specify this; it is set by the runtime for objects with the property checked.
	RUN_BEFORE_FADE_IN = 0x0008,

	// Adds movement A/C/E/P to the object. See OEFLAGS::SPRITES.
	MOVEMENTS = 0x0010,

	// Adds animation A/C/E/P and customising to the object. Consider OEPREFS::INK_EFFECTS.
	// @remarks If specified with OEFLAGS::SPRITES, Fusion will draw the object for you.
	ANIMATIONS = 0x0020,

	// If set, indicates the Tab button can select this object.
	// @remarks Tab and Ctrl-Tab is used for child-window controls like button, list, editbox, on all OSes.
	TAB_STOP = 0x0040,

	// Needs to receive window process messages (i.e. app was minimized).
	// If you are displaying a window, see OEFLAGS::DISPLAY_IN_FRONT.
	// @remarks Will #define WNDPROC_OEFLAG_EXTENSION
	WINDOW_PROC = 0x0080,

	// Enables A/C/E/P for alterable values, strings, internal flags.
	// @remarks No further effort is needed for you to implement these in DarkEdif; using this flag is enough.
	VALUES = 0x0100,

	// Enables A/C/E/P similar to an Active object.
	SPRITES = 0x0200,

	// Used with BACK_SAVE. Expects the ext to save the background at runtime request,
	// runtime will call SaveBackground(), RestoreBackground(), KillBackground().
	// Only has effect in Windows Software display mode.
	// @remarks DarkEdif does not expose these functions, due to low expected usage.
	INTERNAL_BACK_SAVE = 0x0400,

	// Indicates "follow the playfield" option is checked. Object will be moved by runtime
	// when frame is scrolled.
	// See OEPREFS::SCROLLING_INDEPENDENT to show the scroll independent property.
	SCROLLING_INDEPENDENT = 0x0800,

	// Causes the ext's display to not be erased and redrawn if an active/sprite moves in front
	// of this object.
	// Only usable with SPRITES. Has no effect in Direct3D 8+, only Windows Software display.
	// @remarks Such a sprite is drawn just on the top of the backdrops, rather than with other sprites.
	QUICK_DISPLAY = 0x1000,

	// Never destroy object if too far from frame
	NEVER_KILL = 0x2000,

	// If set, object is kept activated when far from frame. See OEFLAGS::MANUAL_SLEEP.
	NEVER_SLEEP = 0x4000,

	// Indicates the object may not be inactivated if far from frame.
	// If at runtime, OEFLAGS::NEVER_SLEEP is set, then it stays active.
	// @remarks OEFLAGS::NEVER_SLEEP can be set/cleared at runtime to set/clear inactivation-ability,
	//			hence the "manual sleep".
	MANUAL_SLEEP = 0x8000,

	// Adds text property tab and actions/conditions/expressions to the object.
	// What properties appear in the tab is defined by Extension::TextCapacity.
	// Also see DarkEdif::EditDataFont and DarkEdif::FontMultiPlat.
	// @remarks Very few objects use this; some include HiScore, String, Lives, etc, as they draw text.
	TEXT = 0x10000,

	// Unchecks Create At Start by default for newly created objects in frame editor.
	DONT_CREATE_AT_START = 0x20000,

	// Unused editor-set flag added in 2.5; reserved, do not use
	FAKE_SPRITE = 0x40000,

	// Unused editor-set flag added in 2.5; reserved, do not use
	FAKE_COLLISIONS = 0x80000,

	// Internal runtime flag used to fix a bug when changing animation
	DONT_RESET_ANIM_COUNTER = 0x100000,
};
enum_class_is_a_bitmask(OEFLAGS);

// Default preferences and allowed options for your extension when newly created in editor.
enum class OEPREFS : short {
	NONE = 0,

	// Enables saving background, has effect in Windows Software display mode only
	// Expects OEFLAGS::BACK_SAVE
	BACK_SAVE = 0x0001,

	// Shows the "follow the playfield" option, auto-moving the object when frame is scrolled.
	// When set, OEFLAGS contains OEFLAGS::SCROLLING_INDEPENDENT.
	// @remarks Was French-spelt OEPREFS_SCROLLINGINDEPENDANT.
	SCROLLING_INDEPENDENT = 0x0002,

	// Windows Software display mode only, draws between background and proper sprites
	QUICK_DISPLAY = 0x0004,

	// Enables "inactivate if far from frame" option.
	SLEEP = 0x0008,

	// Has no effect, functionality was replaced with properties in Fusion 2.0
	// But note OIF_GLOBAL and similar tech.
	LOAD_ON_CALL = 0x0010,

	// Has no effect, functionality was replaced with properties in Fusion 2.0
	GLOBAL = 0x0020,

	// Enables save background and wipe background display properties.
	// Requires OEPREFS::BACK_SAVE.
	// This property only applies in Windows Software display.
	// @remarks Yves suggested erasing background was intended for paint
	//			effects, or for static objects while display is scrolling.
	BACK_EFFECTS = 0x0040,

	// Enables "destroy if far from playfield" property. See OEFLAGS::NEVER_KILL.
	KILL = 0x0080,

	// Enables HWA effects property pane. Used for both old Fusion 2.0 ink effects,
	// and newer Direct3D effects.
	INK_EFFECTS = 0x0100,

	// Enables the (dated) display transitions when appearing/disappearing.
	// Shows the Transitions > Fade In and Fade Out properties in Display tab.
	// @remarks Handled by returning the display surface in GetRunObjectSurface().
	TRANSITIONS = 0x0200,

	// If set, fine collision is active. You must maintain a CollisionMask that reflects the
	// collision area of the currently displaying image of your object, and return it in Extension::GetCollisionMask().
	// @remarks If not used, box collision is used; i.e. the entire rectangle of
	//			(rHo->X, rHo->Y) to (rHo->X + rHo->imgWidth, rHo->Y + rHo->imgHeight)
	//			is considered the collision area of your object.
	FINE_COLLISIONS = 0x0400,

	// Flag used for some extensions in the deprecated Fusion 2.0 Java builds
	// e.g. Java Web Applet, Java Web Start
	APPLET_PROBLEMS = 0x0800,

};
enum_class_is_a_bitmask(OEPREFS);

// Bitfield indicating what text support an OEFLAGS::TEXT extension has.
// @remarks See GetTextCaps() and Extension::TextCapacity.
//			Based on TEXT_FONT, TEXT_LEFT_ALIGN, etc.
enum class TextCapacity : std::uint32_t {
	None = 0,
	// If set, user can pick these alignments - see HAlign, VAlign, and RightToLeft
	Left = 0x1,
	HCenter = 0x2,
	Right = 0x4,
	Top = 0x8,
	VCenter = 0x10,
	Bottom = 0x20,
	// If set, text can be drawn in right-to-left (RTL) ordering,
	// used in some Asian and Arabic languages
	RightToLeft = 0x100,
	// If set, font typeface can be edited by user
	Font = 0x10000,
	// If set, text color can be picked
	Color = 0x20000,
	// Color16 = 0x40000, // limited color palette? (never used, consider this obsolete)

	// Horizontal alignment of all types
	HorizontalAlign = Left | HCenter | Right,
	// Vertical alignment of all types
	VerticalAlign = Top | VCenter | Bottom,
};
enum_class_is_a_bitmask(TextCapacity);

// These are based on the ANDROID_LOG_XXX enum.

#define DARKEDIF_LOG_VERBOSE 2
#define DARKEDIF_LOG_DEBUG 3
#define DARKEDIF_LOG_INFO 4
#define DARKEDIF_LOG_WARN 5
#define DARKEDIF_LOG_ERROR 6
#define DARKEDIF_LOG_FATAL 7
namespace DarkEdif {
	void Log(int logLevel, PrintFHintInside const TCHAR* msgFormat, ...) PrintFHintAfter(2,3);
	[[noreturn]] void LOGF(PrintFHintInside const TCHAR* msgFormat, ...) PrintFHintAfter(1, 2);
}

#ifndef DARKEDIF_LOG_MIN_LEVEL
	#ifdef _DEBUG
		#define DARKEDIF_LOG_MIN_LEVEL DARKEDIF_LOG_DEBUG
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

// objInfoList::name size, define copied from original SDK
#ifndef OINAME_SIZE
	#define	OINAME_SIZE			24
#endif	// OINAME_SIZE

// Tells the compiler not to generate any default constructor/destructor for this class
#define NO_DEFAULT_CTORS_OR_DTORS(className) \
	className() = delete; \
	~className() = delete; \
	className(className&) = delete; \
	className(className&&) = delete; \
	className(const className&) = delete; \
	className(const className&&) = delete;
#define NO_DEFAULT_CTORS(className) \
	className() = delete; \
	className(className&) = delete; \
	className(className&&) = delete; \
	className(const className&) = delete; \
	className(const className&&) = delete;

struct extHeader final
{
	NO_DEFAULT_CTORS_OR_DTORS(extHeader);
	std::uint32_t extSize,
				  extMaxSize,
				  extVersion;			// Version number
	std::uint32_t extID;				// object's identifier; pointer to ext, null in Android/iOS!
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
	// Indicates an object was launched from this one
	Shooter = 0x0040,

	NoCollision = 0x2000,
	// The current expression of this object is returning float (as opposed to int default)
	Float = 0x4000,
	// The current expression of this object is returning string (as opposed to int default)
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
	Color,							// ParamColor
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

#ifdef __APPLE__
inline namespace FusionInternals {
	struct RunObject;
}
//using namespace FusionInternals;
#else
struct RunObject;
#endif

// Generic class for reading actions, conditions and expression parameters
struct ACEParamReader {
	virtual float GetFloat(int i) = 0;
	virtual const TCHAR* GetString(int i) = 0;
	virtual int GetInteger(int i, Params type) = 0;
	// Returns RunObject * for Action, qualifier or singular OI for Condition
	virtual long GetObject(int i) = 0;
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
#ifdef _WIN32
	// Don't call Handle next tick
	ONE_SHOT = 0x1,
	// Call Display after this
	DISPLAY = 0x2,
#else // This was switched for non-Windows... why?!
	// Call Display after this
	DISPLAY = 0x1,
	// Don't call Handle next tick
	ONE_SHOT = 0x2,
#endif
	// WndProc responses
	MSG_HANDLED = 0x4,
	MSG_CATCHED = 0x8,
	MSG_DEF_PROC = 0x10,
	// ?
	MSGRETURNVALUE = 0x40,
};
enum_class_is_a_bitmask(REFLAG);

// If set in current event's flags (event2 struct), makes an action function run once for each selected instance
// before moving to next action in event. If set off, the action is only run once, and it is assumed you loop
// the selected instances yourself.
#define	ACTFLAGS_REPEAT	0x1

// Maximum number of qualifiers an object can have
#define MAX_QUALIFIERS 8

#define LOGF(x, ...) DarkEdif::LOGFInternal(x, ##__VA_ARGS__)

// Useful functions
#include <thread>
#include <atomic>
#include <assert.h>

// Series of classes/structs that will be referred to later
namespace DarkEdif { class Surface; }
namespace Edif { class Runtime; }

// Apple exts are static libs and their defines will clash
#ifdef __APPLE__
inline namespace FusionInternals {
#endif

struct EventGroupMP;


#ifdef __APPLE__
}
#endif

// Prevents using Fusion internals directly, to allow multiplatform consistency
// DarkEdif CPP files will define this, your code shouldn't, unless it's Windows only ext and you want internals
#define DarkEdifInternalAccessPublic public
#define DarkEdifInternalAccessPriv private

#ifndef FUSION_INTERNAL_ACCESS
#define DarkEdifInternalAccessProtected protected
#else
// no op
#define DarkEdifInternalAccessProtected public
#endif

// This ext does not display
#define DARKEDIF_DISPLAY_NONE		0
// This ext uses Fusion's built-in animation system
#define DARKEDIF_DISPLAY_ANIMATIONS	1
// This ext uses DarkEdif simple display system - one surface, automagic
#define DARKEDIF_DISPLAY_SIMPLE		2
// This ext uses DarkEdif manual display system
#define DARKEDIF_DISPLAY_MANUAL		3

// Indicates visibility, object deactivation from being outside frame, scale quality, etc
enum class RunSpriteFlag : std::uint16_t {
	// No flag
	None = 0x0,
	// Object is set to invisible, OR flashing is hiding the object, OR layer is set to invisible - takes effect to override the Visible flag
	Hidden = 0x1,
	// Obsolete flag; runtime sets this, but does not read it. See Sleeping, and CSprite.F_INACTIV
	Inactive = 0x2,
	// Object is outside the RH3MinimumKill margin, inactive if far from frame is on, and object instance has been deactivated.
	// Player and QuickDisplay objects do not deactivate.
	// This implies that object is not destroyed when far from frame. Note CRunApp.AH2OPT_DESTROYIFNOINACTIVATE.
	// Sleeping objects do not move, do not flash, and have their sprite entry (roc.rcSprite) temporarily deleted.
	Sleeping = 0x4,
	// Use or has used resample-tech to resize this object display to new scale
	Scale_Resample = 0x8,
	// Use or has used resample-tech to rotate this object display to new angle
	Rotate_Antialias = 0x10,
	// Object is visible, flashing accounted for, layer visibility not accounted for - overridden by Hidden flag
	Visible = 0x20,
	// Runtime use only: Object's effect has been constructed in runtime side
	Created_Effect = 0x40
}; // RSFLAG_XX enum
enum_class_is_a_bitmask(RunSpriteFlag);

// @remarks This is defined here as RunSprite uses it in all platforms
enum class BlitOperation {
	// No merge or altering operation, just flat overwriting destination surface
	Copy = 0,
	// Blend the destination RGB with source surface RGB
	// blend_coef is in range 0 to 128, 128 being a full merge
	// dest = ((dest * blend_coef) + (src * (128-blend_coef)))/128
	Blend,
	// Invert the source RGB before overwriting destination RGB
	// src XOR 0xFFFFFF
	Invert,
	// Combine the source RGB with dest RGB using XOR
	// src XOR dest
	XOR,
	// Combine the source RGB with dest RGB using AND
	// src AND dest
	AND,
	// Combine the source RGB with dest RGB using OR
	// src OR dest
	OR,
	// Blend the destination RGB with source surface RGB
	// dest = ((dest * blend_coef) + ((src==transp) ? replace : src * (128-blend_coef) ))/128
	BlendReplaceTransp,
	// Removed in Fusion 2.5, possibly 2.0
	// Blits using the WinGDI raster operation (ROP) specified in the lParam
	// https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-bitblt#:~:text=A%20raster-operation%20code
	DrawWithROP,
	// Applies INVERT then AND
	AndNot,
	// Combines the source RGB with the dest RGB by adding src and dest together by color channels
	Addition,
	// Gets the mean average of R,G,B and uses it for dest R,G,B
	Mono,
	// Combines the source RGB with the dest RGB by subtracting src from dest by color channels
	Subtract,
	// ?
	BlendDontReplaceColor,
	// Indicates use of a HWA effect (Direct3D)
	EffectEx,
#ifdef __ANDROID__
	// Android only: used to indicate a TextSurface rendering without any HWA effect
	// @remarks See CTextSurface.java. BOP_TEXT is not defined in Windows or iOS SDK.
	Text,
#endif
	// Max possible outside of a bitmask
	// In Android, this is 15 due to presence of Text, other platforms have it as 14.
	// @remarks KcBoxA/B on iOS have it as 13, mistakenly
	Max,
	// The bitmask for possible blit ops
	Mask = 0x0FFF,
	// The bitmask for blit ops + RGBAFilter
	EffectMask = 0xFFFF,
	// Used in ros->rsEffect to indicate alpha and/or color coefficient alters the image.
	// If not set in ros->rsEffect, indicates blend coefficient is 0, RGB coeff is RGB(255,255,255)
	RGBAFilter = 0x1000,
	// Indicates transparency is enabled in ink effect property. Should be excluded with Mask.
	Transparent = 0x10000000L,
	// Indicates anti-alias is checked in ink effect property. Should be excluded with Mask.
	AntiAlias = 0x20000000L,
	// None: same as Copy, included for bitmask ease
	None = Copy,
};
enum_class_is_a_bitmask(BlitOperation);

