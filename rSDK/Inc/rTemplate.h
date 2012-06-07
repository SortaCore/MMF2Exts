// rSDK stuff

// Include guard
#ifndef RTEMPLATE_INC
#define RTEMPLATE_INC

// C++ only
#ifndef __cplusplus
#pragma message("rSDK will only compile with a C++ compiler")
#endif

// VC++ only
#ifndef _MSC_VER
#pragma message("rSDK will only compile under Microsoft Visual C++")
#endif

// Dynamic array class if needed

#ifndef _VECTOR_

template<class T> class rVector {
protected:
	T * A;
	size_t C;
	size_t M;
public:
	
	// Constructor and destructor
	inline rVector() { M=3; C=0; A=(T *)malloc(sizeof(T)*M); }
	~rVector() { free(A); }

	// Copy constructor
	inline rVector(rVector<T> &O) {
		C=O.size();
		M=C+1;
		A=(T *)malloc(sizeof(T)*M);
		memcpy(A,O.array(),sizeof(T)*C);
	}

	// Assignment operator
	inline void operator = (rVector<T> &O) {
		C=O.size();
		M=C+1;
		free(A);
		A=(T *)malloc(sizeof(T)*M);
		memcpy(A,O.array(),sizeof(T)*C);
	}

	// Push an item onto the top of the vector
	inline void push_back(T O) {
		if (C>=M) {
			M=C*3+1;
			A=(T *)realloc(A,sizeof(T)*M);
		}
		A[C]=O;
		C++;
	}

	// Pop an item from the top of the vector
	inline void pop_back() {
		if (C<(M>>1)) {
			M>>=1;
			A=(T *)realloc(A,sizeof(T)*M);
		}
		C--;
	}

	// Get the size of the vector
	inline size_t size() const { return C; }

	// Get an item at any location in the vector
	inline T at(size_t I) const { return A[I]; }
	inline T operator [] (size_t I) const { return A[I]; }

	// Get the last item in the vector
	inline T back() const { return A[C-1]; }

	// Get a raw pointer to the array
	inline T * array() const { return A; }

};

#else
#define rVector vector
#endif

// Property macros
#define PROPS_IDS_START()			enum { PROPID_SETTINGS=PROPID_EXTITEM_CUSTOM_FIRST,	
#define PROPS_IDS_END()				};
#define PROPS_DATA_START()			PropData Properties[]={
#define PROPS_DATA_END()			PropData_End()};

// Debugger macros 
#define DEBUGGER_IDS_START()		enum {
#define DEBUGGER_IDS_END()			};
#define DEBUGGER_ITEMS_START()		WORD DebugTree[]={
#define DEBUGGER_ITEMS_END()		DB_END};

// ID macros
#define CONDITION_ID(id)			(26000+(id)*10)
#define ACTION_ID(id)				(25000+(id)*10)
#define EXPRESSION_ID(id)			(27000+(id)*10)

// Some of the menu macros (the rest are specific to actions/conditions/expressions)
#define SUB_START(string)			m.AddMenu(SubMenu,string);
#define SUB_END						m.AddMenu(MenuItem,"");
#define SEPARATOR					m.AddMenu(-3,"");

// Should be placed globally in some CPP file
#define EXT_INIT()					rVector<ExtFunction*> Conditions; \
									rVector<ExtFunction*> Actions; \
									rVector<ExtFunction*> Expressions; \
									short * conditionsInfos; \
									short * actionsInfos; \
									short * expressionsInfos; \
									LPCONDITION * ConditionJumps; \
									LPACTION * ActionJumps; \
									LPEXPRESSION * ExpressionJumps;

// Macros for the number of each type of function
#define CONDITION_COUNT				Conditions.size()
#define ACTION_COUNT				Actions.size()
#define EXPRESSION_COUNT			Expressions.size()

// Safe variable macro- use in macros that need to declare variables to avoid any conflict (what a hack)
#define SVAR(s)						_SAFE__##s##_

// Macros to return different types
#define ReturnFloat(flt)			rdPtr->rHo.hoFlags|=HOF_FLOAT; return rdPtr->rRd->Float2Long(flt)
#define ReturnString(str)			rdPtr->rHo.hoFlags|=HOF_STRING; return (long)str;

// For consistency
#define ReturnInteger(i)			return i;
#define ReturnBool(b)				return b;

// Safe return of a string- returns a copy
#define ReturnStringSafe(str)		int SVAR(1)=strlen(str)+1; \
									char * SVAR(2)=rdPtr->rRd->GetStringSpace(SVAR(1)); \
									memcpy(SVAR(2),str,SVAR(1)); \
									rdPtr->rHo.hoFlags|=HOF_STRING; \
									return long(SVAR(2))

// Macros to get parameters
#define ExParam(type)				rdPtr->rRd->GetExpressionParameter((short)param1,type)
#define Param(type)					rdPtr->rRd->GetParameter((short)param1,type)

// Copy something from edPtr to rdPtr (if it is copiable and has the same name on both sides)
#define CopyToRd(s)					rdPtr->s=edPtr->s

// Fixing some of the spelling mistakes
#define PARAM_COMPARISON			PARAM_COMPARAISON

// Fixing confusion
#undef	PARAM_STRING
#define	PARAM_STRING				PARAM_EXPSTRING
#define	PARAM_NUMBER				PARAM_EXPRESSION
#define EXPPARAM_NUMBER				EXPPARAM_LONG

// For the Param macro
#define TYPE_GENERIC				0x0003

// Registered ID constants
#define REGID_PRIVATE				-1
#define REGID_RUNTIME				-2

// Property strings are just cast to an int
#define PSTR(s)						((int)s)

// To generate an identifier
#define MAKEID(a,b,c,d)				((#@a<<24)|(#@b<<16)|(#@c<<8)|(#@d)) 

// The build of MMF
#define MMF_BUILD (mV->mvGetVersion()&MMFBUILD_MASK)

// Common include
#include "common.h"

// Typedefs for pointers to actions, conditions and expressions
typedef long (WINAPI DLLExport * LPCONDITION)(LPRDATA,long,long);
typedef short (WINAPI DLLExport * LPACTION)(LPRDATA,long,long);
typedef long (WINAPI DLLExport * LPEXPRESSION)(LPRDATA,long);

// Class to hold a list of parameters (thanks turbo)
class param_list {
public:

	// The constructor fills up the vectors with the given parameters
	param_list(size_t count,...) {

		// Variable argument list
		va_list list;

		// Start going through the arguments
		va_start(list,count);

		// This is twice the number of parameters given
		int count_dbl=count*2;

		// Loop through all the arguments
		for (int i=0;i<count_dbl;i++) {

			// If the argument is odd
			if (i%2==0)

				// It must be a type; push it onto the types vector
				Type.push_back(va_arg(list,short));

			// If the argument is even
			else

				// It must be a name; push it onto the names vector
				Name.push_back(va_arg(list,const char *));
		}

		// Stop going through the arguments
		va_end(list);
	}

	// This will hold the type of each argument
	rVector<short> Type;

	// This will hold the name of each argument
	rVector<const char *> Name;
};

// Temporary declaration of the ExtFunction class
class ExtFunction;

// Extern declarations of the condition, action and expression vectors
extern rVector<ExtFunction*> Conditions;
extern rVector<ExtFunction*> Actions;
extern rVector<ExtFunction*> Expressions;

// This class holds an action, condition or expression
class ExtFunction {
protected:

	// A vector to hold the types of the parameters
	rVector<short> ParamTypes;

	// A vector to hold the names of the parameters
	rVector<const char *> ParamStrings;

	// Pointers to conditions, actions and expressions- we have them all but don't use them all
	LPCONDITION mCondition;
	LPACTION mAction;
	LPEXPRESSION mExpression;

	// The name of the function
	const char * Name;

	// The flags
	short Flags;

public:

	// The first constructor overload, for a condition
	ExtFunction(LPCONDITION con,short flags,const char * name,param_list params) {

		// Set the condition pointer
		mCondition=con;

		// Store the param types/names and function name
		ParamTypes=params.Type;
		ParamStrings=params.Name;
		Name=name;

		// Store the function flags
		Flags=flags;

		// Push the function into the vector
		Conditions.push_back(this);
	}

	// Constructor overload for an action
	ExtFunction(LPACTION act,short flags,const char * name,param_list params) {

		// Set the action pointer
		mAction=act;
		
		// Store the param types/names and function name
		ParamTypes=params.Type;
		ParamStrings=params.Name;
		Name=name;

		// Store the function flags
		Flags=flags;

		// Push the function into the vector
		Actions.push_back(this);
	}

	// Constructor overload for an expression
	ExtFunction(LPEXPRESSION exp,short flags,const char * name,param_list params) {

		// Set the expression pointer
		mExpression=exp;

		// Store the param types/names and function name
		ParamTypes=params.Type;
		ParamStrings=params.Name;
		Name=name;

		// Store the function flags
		Flags=flags;

		// Push the function into the vector
		Expressions.push_back(this);
	}

	// Inline functions to access protected variables
	inline LPCONDITION		getCondition()			{ return mCondition; }
	inline LPACTION			getAction()				{ return mAction; }
	inline LPEXPRESSION		getExpression()			{ return mExpression; }
	inline const char *		getName()				{ return Name; }
	inline size_t			getParamCount()			{ return ParamTypes.size(); }
	inline short			getParamType(size_t i)	{ return ParamTypes[i]; }
	inline const char *		getParamName(size_t i)	{ return ParamStrings[i]; }
	inline short			getFlags()				{ return Flags; }
};

// Macro for a condition in main.cpp
#define CONDITION(num, name, flags, params) \
	long WINAPI DLLExport ConditionFunc##num(LPRDATA, long, long); \
	ExtFunction ConditionClass##num((LPCONDITION)ConditionFunc##num, flags, name, param_list##params); \
	long WINAPI DLLExport ConditionFunc##num(LPRDATA rdPtr, long param1, long param2) \

// Macro for an action in main.cpp
#define ACTION(num, name, flags, params) \
	short WINAPI DLLExport ActionFunc##num(LPRDATA, long, long); \
	void IActionFunc##num(LPRDATA, long, long); \
	ExtFunction ActionClass##num((LPACTION)ActionFunc##num, flags, name, param_list##params); \
	short WINAPI DLLExport ActionFunc##num(LPRDATA rdPtr, long param1, long param2) { \
	IActionFunc##num(rdPtr,param1,param2); return 0; \
	} inline void IActionFunc##num(LPRDATA rdPtr,long param1, long param2)

// Macro for an expression in main.cpp
#define EXPRESSION(num, name, flags, params) \
	long WINAPI DLLExport ExpressionFunc##num(LPRDATA rdPtr, long param1); \
	long IExpressionFunc##num(LPRDATA, long); \
	ExtFunction ExpressionClass##num((LPEXPRESSION)ExpressionFunc##num, flags, name, param_list##params); \
	long WINAPI DLLExport ExpressionFunc##num(LPRDATA rdPtr, long param1) { \
	rdPtr->rRd->P_GetExpressionParameter=G_GetFirstExpressionParameter; \
	return IExpressionFunc##num(rdPtr,param1); \
	} inline long IExpressionFunc##num(LPRDATA rdPtr,long param1)

// Include oop_ext.h (the code from TurboTemplate)
#include "oop_ext.h"

// Constants for the menu class
const int SubMenu=-1;
const int MenuItem=-2;
const int Separator=-3;

#ifndef RUN_ONLY

	// Class to hold a menu
	class Menu {
	private:
		HMENU RootMenu;
		rVector<HMENU> SubMenus;
		rVector<const char *> SubMenuNames;
	public:

		// The constructor creates a root menu and pushes it onto the submenu vector
		Menu() {
			RootMenu=CreateMenu();
			SubMenus.push_back(RootMenu);
		}
		
		void AddMenu(int id,const char * str,bool disabled=false) {
			if (id>0) {
				AppendMenu(SubMenus.back(),MF_BYPOSITION|MF_STRING,id,str);
			} else if (id==SubMenu) {
				HMENU hSubMenu=CreatePopupMenu();
				SubMenus.push_back(hSubMenu);
				SubMenuNames.push_back(str);
			} else if (id==MenuItem&&!disabled) {
				AppendMenu(SubMenus[SubMenus.size()-2],MF_BYPOSITION|MF_STRING|MF_POPUP,(UINT)SubMenus.back(), SubMenuNames.back());
				SubMenus.pop_back();	
				SubMenuNames.pop_back();
			} else if (id==MenuItem&&disabled) {
				AppendMenu(SubMenus[SubMenus.size()-2],MF_BYPOSITION|MF_STRING|MF_POPUP|MF_GRAYED,(UINT)SubMenus.back(),SubMenuNames.back());
				SubMenus.pop_back();	
				SubMenuNames.pop_back();
			} else if (id==Separator) {
				AppendMenu(SubMenus.back(),MF_BYPOSITION|MF_SEPARATOR,0,NULL);
			}
		}

		inline HMENU GetMenu() { return RootMenu; }
	};

	inline HMENU ConditionMenu(LPEDATA edPtr) {	
		#define	CONDITION_MENU
		#define	ITEM(id,string)	m.AddMenu(CONDITION_ID(id),string);
		#define	DISABLED(id,string)	m.AddMenu(CONDITION_ID(id),string,true);	
		Menu m;
		#include "menu.h"
		return m.GetMenu();
		#undef CONDITION_MENU
		#undef ITEM
		#undef DISABLED
	}

	inline HMENU ActionMenu(LPEDATA edPtr) {
		#define	ACTION_MENU
		#define	ITEM(id,string)	m.AddMenu(ACTION_ID(id),string);
		#define	DISABLED(id,string)	m.AddMenu(ACTION_ID(id),string,true);
		Menu m;
		#include "menu.h"
		return m.GetMenu();
		#undef ACTION_MENU
		#undef ITEM
		#undef DISABLED
	}

	inline HMENU ExpressionMenu(LPEDATA edPtr) {
		#define	EXPRESSION_MENU
		#define	ITEM(id,string)	m.AddMenu(EXPRESSION_ID(id),string);
		#define	DISABLED(id,string)	m.AddMenu(EXPRESSION_ID(id),string,true);
		Menu m;
		#include "menu.h"
		return m.GetMenu();
		#undef EXPRESSION_MENU
		#undef ITEM
		#undef DISABLED
	}

#endif

// A function designed to be called by the rRd thread functions
typedef void (__cdecl * rThread)(LPRDATA);

// Global functions to get expression parameters
inline long G_GetFirstExpressionParameter(LPRDATA rdPtr,short param1,long type);
inline long G_GetNextExpressionParameter(LPRDATA rdPtr,short param1,long type);

// A class to hold runtime functions
class rRundata {
protected:

	// A pointer to rdPtr
	LPRDATA rdPtr;

public:
	
	// Class constructor
	inline rRundata(LPRDATA _rdPtr) {
		rdPtr=_rdPtr;
		P_GetExpressionParameter=G_GetFirstExpressionParameter;
	}

	// A function pointer to one of the parameter functions
	long (* P_GetExpressionParameter)(LPRDATA rdPtr,short param1,long type);

	// A function to call the function pointer
	inline long GetExpressionParameter(short param1,long type) {
		return P_GetExpressionParameter(rdPtr,param1,type);
	}

	// Functions to get expression parameters
	inline long GetFirstExpressionParameter(short param1,long type) {
		return G_GetFirstExpressionParameter(rdPtr,param1,type);
	}

	inline long GetNextExpressionParameter(short param1,long type) {
		return G_GetNextExpressionParameter(rdPtr,param1,type);
	}

	// Function to get action/condition parameters
	inline long GetParameter(short param1,long type) {
		switch(type) {
		default:
			return CNC_GetParameter(rdPtr);
		case TYPE_INT:
			return CNC_GetIntParameter(rdPtr);
		case TYPE_STRING:
			return CNC_GetStringParameter(rdPtr);
		case TYPE_FLOAT:
			long tmpf=CNC_GetFloatParameter(rdPtr);
			float param=*(float*)&tmpf;
			return (long)param;
		};
	}

	// To call a runtime function
	inline long CallFunction(short funcNum, short wParam, long lParam) {
		return callRunTimeFunction(rdPtr,funcNum,wParam,lParam);
	}

	// Forces the HandleRunObject routine to be called at next loop
	inline void Rehandle() {
		CallFunction(RFUNCTION_REHANDLE,0,0);
	}

	// Generates a triggered event
	inline void GenerateEvent(int EventID) {
		CallFunction(RFUNCTION_GENERATEEVENT,EventID,0);
	}

	// Generate an event at the end of the loop (safer)
	inline void PushEvent(int EventID) {
		CallFunction(RFUNCTION_PUSHEVENT,EventID,0);
	}

	// Allocate some space to store a string
	inline char * GetStringSpace(size_t size) {
		return (char *)callRunTimeFunction(rdPtr,RFUNCTION_GETSTRINGSPACE_EX,0,size);
	}

	// Pause the application before a lengthy task, without messing up timers etc.
	inline void Pause() {
		CallFunction(RFUNCTION_PAUSE,0,0);	
	}

	// Continue the application after pausing
	inline void Continue() {
		CallFunction(RFUNCTION_CONTINUE,0,0);	
	}

	// Force a complete redraw of the frame
	inline void Redisplay() {
		CallFunction(RFUNCTION_REDISPLAY,0,0);
	}

	// Get the drive of the application
	inline void GetApplicationDrive(char * buf) {
		CallFunction(RFUNCTION_GETFILEINFOS,FILEINFO_DRIVE,(long)buf);
	}

	// Get the directory of the application
	inline void GetApplicationDirectory(char * buf) {
		CallFunction(RFUNCTION_GETFILEINFOS,FILEINFO_DIR,(long)buf);
	}

	// Get the complete path of the application
	inline void GetApplicationPath(char * buf) {
		CallFunction(RFUNCTION_GETFILEINFOS,FILEINFO_PATH,(long)buf);
	}

	// Get the application name
	inline void GetApplicationName(char * buf) {
		CallFunction(RFUNCTION_GETFILEINFOS,FILEINFO_APPNAME,(long)buf);
	}

	// Force a call of the DisplayRunObject routine during the display update process
	inline void Redraw() {
		CallFunction(RFUNCTION_REDRAW,0,0);
	}

	// Destroy the object
	inline void Destroy() {
		CallFunction(RFUNCTION_DESTROY,0,0);
	}

	// Execute an external program
	inline void ExecuteProgram(prgParam * program) {
		CallFunction(RFUNCTION_EXECPROGRAM,0,(long)program);
	}

	// Open a dialog box to edit a integer value.
	inline long EditInt(EditDebugInfo * edi) {
		return CallFunction(RFUNCTION_EDITINT,0,(long)edi);
	}

	// Open a dialog box to edit a string
	inline long EditText(EditDebugInfo * edi) {
		return CallFunction(RFUNCTION_EDITTEXT,0,(long)edi);
	}

	// Call a movement's ActionEntry function
	inline void CallMovement(short id,long param) {
		CallFunction(RFUNCTION_CALLMOVEMENT,id,param);	
	}

	// Change the position of the object
	inline void SetPosition(int x,int y) {
		CallFunction(RFUNCTION_SETPOSITION,x,y);
	}

	// Simplify creating threads with rdPtr access
	inline HANDLE StartThread(rThread function) {
		return CreateThread(0,0,(LPTHREAD_START_ROUTINE)function,rdPtr,0,0);
	}

	// Grab the cSurface of another object
	void GrabSurface(LPRO object,cSurface &destination) {
		fprh rhPtr=rdPtr->rHo.hoAdRunHeader;
		cSurface imageSurface;
		LockImageSurface(rhPtr->rhIdAppli,object->roc.rcImage,imageSurface);
		destination=imageSurface;
		UnlockImageSurface(imageSurface);
	}

	// Convert a float to a long for returning to MMF
	long Float2Long(double value) {
		float IntoFloat=(float)value;
		long myAnswer=0;
		memcpy((void *)&myAnswer,(void *)&IntoFloat,sizeof(float));
		return myAnswer;
	}

	// All the (useful) TigsExt.hpp functions which people might miss
	// are now included in rRundata (or at least similar stuff)

	// Check if a number is within the bounds of min and max
	inline bool CheckBounds(int number,int min,int max) {
		return !(number<min||number>max);
	}

	// Float version
	inline bool CheckBounds(float number,float min,float max) {
		return !(number<min||number>max);
	}

	// Force a number to be within the bounds of min and max
	inline int ForceBounds(int number,int min,int max) {
		number=number>max?max:number;
		number=number<min?min:number;
		return number;
	}

	// Float version
	inline float ForceBounds(float number,float min,float max) {
		number=number>max?max:number;
		number=number<min?min:number;
		return number;
	}

	// Convert an LPRO to a fixed value
	inline int LPRO2Fixed(LPRO o) {
		return ((o->roHo.hoCreationId<<16)+o->roHo.hoNumber);
	}

	// Convert a fixed value to an LPRO
	inline LPRO Fixed2LPRO(int fixed) {
		LPOBL objList=rdPtr->rHo.hoAdRunHeader->rhObjectList;
		return LPRO(objList[0x0000FFFF&fixed].oblOffset);
	}

	// Call a function in an LPRO
	inline void LPRO_CallFunction(LPRO o,short funcNum, short wParam=0, long lParam=0) {
		LPRH(o->roHo.hoAdRunHeader)->rh4.rh4KpxFunctions[funcNum].routine((LPHO)o,wParam,lParam);
	}

	// Update an LPRO
	inline void LPRO_Update(LPRO o) {
		o->roc.rcChanged=1;
	}

	// Check if an LPRO is destroyed
	inline bool LPRO_IsDestroyed(LPRO o) {
		return (o->roHo.hoFlags&HOF_DESTROYED?1:0);
	}

	// Move an LPRO
	inline void LPRO_Move(LPRO o,int x,int y) {
		LPRO_CallFunction(o,RFUNCTION_SETPOSITION,x,y);
	}

	// Rehandle an LPRO
	inline void LPRO_Rehandle(LPRO o) {
		LPRO_CallFunction(o,RFUNCTION_REHANDLE);
	}

	// Redraw an LPRO
	inline void LPRO_Redraw(LPRO o) {
		LPRO_CallFunction(o,RFUNCTION_REDRAW);
	}

	// Generate an event within an LPRO
	inline void LPRO_GenerateEvent(LPRO o,int EventNumber) {
		LPRO_CallFunction(o,RFUNCTION_GENERATEEVENT,EventNumber);
	}

	// Push an event within an LPRO
	inline void LPRO_PushEvent(LPRO o,int EventNumber) {
		LPRO_CallFunction(o,RFUNCTION_PUSHEVENT,EventNumber);
	}

	// Destroy an LPRO
	inline void LPRO_Destroy(LPRO o) {
		LPRO_CallFunction(o,RFUNCTION_DESTROY);
	}
};


// The actual functions for getting parameters
inline long G_GetFirstExpressionParameter(LPRDATA rdPtr,short param1,long type) {
	rdPtr->rRd->P_GetExpressionParameter=G_GetNextExpressionParameter;
	return CNC_GetFirstExpressionParameter(rdPtr,param1,type);
}

inline long G_GetNextExpressionParameter(LPRDATA rdPtr,short param1,long type) {
	return CNC_GetNextExpressionParameter(rdPtr,param1,type);
}

// A long pointer to rRundata
typedef rRundata * LPRRDATA;

// Makes the flags based on what was defined in information.h
inline void MagicFlags(DWORD &d) {

	d=0;

	#ifdef M_OEFLAG_DISPLAYINFRONT
		d|=OEFLAG_DISPLAYINFRONT;
	#endif

	#ifdef M_OEFLAG_BACKGROUND
		d|=OEFLAG_BACKGROUND;
	#endif

	#ifdef M_OEFLAG_BACKSAVE
		d|=OEFLAG_BACKSAVE;
	#endif

	#ifdef M_OEFLAG_RUNBEFOREFADEIN
		d|=OEFLAG_RUNBEFOREFADEIN;
	#endif

	#ifdef M_OEFLAG_MOVEMENTS
		d|=OEFLAG_MOVEMENTS;
	#endif

	#ifdef M_OEFLAG_ANIMATIONS
		d|=OEFLAG_ANIMATIONS;
	#endif

	#ifdef M_OEFLAG_TABSTOP
		d|=OEFLAG_TABSTOP;
	#endif

	#ifdef M_OEFLAG_WINDOWPROC
		d|=OEFLAG_WINDOWPROC;
	#endif

	#ifdef M_OEFLAG_VALUES
		d|=OEFLAG_VALUES;
	#endif

	#ifdef M_OEFLAG_SPRITES
		d|=OEFLAG_SPRITES;
	#endif

	#ifdef M_OEFLAG_INTERNALBACKSAVE
		d|=OEFLAG_INTERNALBACKSAVE;
	#endif

	#ifdef M_OEFLAG_SCROLLINGINDEPENDANT
		d|=OEFLAG_SCROLLINGINDEPENDANT;
	#endif

	#ifdef M_OEFLAG_QUICKDISPLAY
		d|=OEFLAG_QUICKDISPLAY;
	#endif

	#ifdef M_OEFLAG_NEVERKILL
		d|=OEFLAG_NEVERKILL;
	#endif

	#ifdef M_OEFLAG_NEVERSLEEP
		d|=OEFLAG_NEVERSLEEP;
	#endif

	#ifdef M_OEFLAG_MANUALSLEEP
		d|=OEFLAG_MANUALSLEEP;
	#endif

	#ifdef M_OEFLAG_TEXT
		d|=OEFLAG_TEXT;
	#endif
}

// Makes the prefs based on what was defined in information.h
inline void MagicPrefs(short &d) {

	d=0;

	#ifdef M_OEPREFS_BACKSAVE
		d|=OEPREFS_BACKSAVE;
	#endif

	#ifdef M_OEPREFS_SCROLLINGINDEPENDANT
		d|=OEPREFS_SCROLLINGINDEPENDANT;
	#endif

	#ifdef M_OEPREFS_QUICKDISPLAY
		d|=OEPREFS_QUICKDISPLAY;
	#endif

	#ifdef M_OEPREFS_SLEEP
		d|=OEPREFS_SLEEP;
	#endif

	#ifdef M_OEPREFS_LOADONCALL
		d|=OEPREFS_LOADONCALL;
	#endif

	#ifdef M_OEPREFS_GLOBAL
		d|=OEPREFS_GLOBAL;
	#endif

	#ifdef M_OEPREFS_BACKEFFECTS
		d|=OEPREFS_BACKEFFECTS;
	#endif

	#ifdef M_OEPREFS_KILL
		d|=OEPREFS_KILL;
	#endif

	#ifdef M_OEPREFS_INKEFFECTS
		d|=OEPREFS_INKEFFECTS;
	#endif

	#ifdef M_OEPREFS_TRANSITIONS
		d|=OEPREFS_TRANSITIONS;
	#endif

	#ifdef M_OEPREFS_FINECOLLISIONS
		d|=OEPREFS_FINECOLLISIONS;
	#endif
}

#endif // !RTEMPLATE_INC
