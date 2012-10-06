
#pragma once


#include "json.h"

#include <vector>
#include <list>
#include <string>
using namespace std;

#include "MMFMasterHeader.h"

/*
#include "ccxhdr.h"
#include "CfcFile.h"
#include "ImageFlt.h"
#include "ImgFlt.h"

#include "Patch.h"
*/



class Extension;

#include "ObjectSelection.h"

#ifndef RUN_ONLY
	#if defined(MMFEXT)
		#define	IS_COMPATIBLE(v) (v->GetVersion != nullptr && (v->GetVersion() & MMFBUILD_MASK) >= Extension::MinimumBuild && (v->GetVersion() & MMFVERSION_MASK) >= MMFVERSION_20 && ((v->GetVersion() & MMFVERFLAG_MASK) & MMFVERFLAG_HOME) == 0)
	#elif defined(PROEXT)
		#define IS_COMPATIBLE(v) (v->GetVersion != nullptr && (v->GetVersion() & MMFBUILD_MASK) >= Extension::MinimumBuild && (v->GetVersion() & MMFVERSION_MASK) >= MMFVERSION_20 && ((v->GetVersion() & MMFVERFLAG_MASK) & MMFVERFLAG_PRO) != 0)
	#else
		#define	IS_COMPATIBLE(v) (v->GetVersion != nullptr && (v->GetVersion() & MMFBUILD_MASK) >= Extension::MinimumBuild && (v->GetVersion() & MMFVERSION_MASK) >= MMFVERSION_20)
	#endif
#else
	#define IS_COMPATIBLE(v) (false)
#endif

#define LinkAction(ID, Function) \
    SDK->ActionFunctions[ID] = Edif::MemberFunctionPointer(&Extension::Function);

#define LinkCondition(ID, Function) \
    SDK->ConditionFunctions[ID] = Edif::MemberFunctionPointer(&Extension::Function);

#define LinkExpression(ID, Function) \
    SDK->ExpressionFunctions[ID] = Edif::MemberFunctionPointer(&Extension::Function);

extern HINSTANCE hInstLib;

struct RUNDATA;
struct EDITDATA;

//typedef RUNDATA * RUNDATA *;
//typedef EDITDATA * EDITDATA *;
struct ACEInfo;
namespace Edif
{
	// New access properties
	namespace Properties
	{
		// Synced with Names
		static const enum IDs {
			PROPTYPE_STATIC = 1,		// Simple static text
			PROPTYPE_FOLDER,			// Folder
			PROPTYPE_FOLDER_END,		// Folder End
			PROPTYPE_EDITBUTTON,		// Edit button, param1 = button text, or nullptr if Edit
			PROPTYPE_EDIT_STRING,		// Edit box for strings, parameter = max length
			PROPTYPE_EDIT_NUMBER,		// Edit box for numbers, parameters = min value, max value
			PROPTYPE_COMBOBOX,			// Combo box, parameters = list of strings, options (sorted, etc)
			PROPTYPE_SIZE,				// Size
			PROPTYPE_COLOR,				// Color
			PROPTYPE_LEFTCHECKBOX,		// Checkbox
			PROPTYPE_SLIDEREDIT,		// Edit + Slider
			PROPTYPE_SPINEDIT,			// Edit + Spin
			PROPTYPE_DIRCTRL,			// Direction Selector
			PROPTYPE_GROUP,				// Group
			PROPTYPE_LISTBTN,			// !Internal, do not use
			PROPTYPE_FILENAME,			// Edit box + browse file button, parameter = FilenameCreateParam
			PROPTYPE_FONT,				// Font dialog box
			PROPTYPE_CUSTOM,			// Custom property
			PROPTYPE_PICTUREFILENAME,	// Edit box + browse image file button
			PROPTYPE_COMBOBOXBTN,		// Combo box, parameters = list of strings, options (sorted, etc)
			PROPTYPE_EDIT_FLOAT,		// Edit box for floating point numbers, parameters = min value, max value, options (signed, float, spin)
			PROPTYPE_EDIT_MULTILINE,	// Edit box for multiline texts, no parameter
			PROPTYPE_IMAGELIST,			// Image list
			PROPTYPE_ICONCOMBOBOX,		// Combo box with icons
			PROPTYPE_URLBUTTON,			// URL button
			PROPTYPE_DIRECTORYNAME,		// Directory pathname
			PROPTYPE_SPINEDITFLOAT,		// Edit + Spin, value = floating point number
			// For searches
			PROPTYPE_FIRST_ITEM = PROPTYPE_STATIC,
			PROPTYPE_LAST_ITEM = PROPTYPE_SPINEDITFLOAT,
		};

		// Synced with PropertyIDs
		static const char * Names [] = {
			"!Start item",		// IDs starts with 1
			"Text",
			"Folder",
			"FolderEnd",
			"Edit button",
			"Editbox String",
			"Editbox Number",
			"Combo Box",
			"Size",
			"Color",
			"Checkbox",
			"Edit slider",
			"Edit spin",
			"Edit direction",
			"Group",
			"!List button",			// Internal, do not use
			"Editbox File",
			"Font",
			"Custom",
			"Editbox Image File",
			"Combo Box Button",
			"Editbox Float",
			"Editbox String Multiline",
			"Image List",
			"Combo Box Icons",
			"URL button",
			"Editbox Folder",
			"Edit spin float",
		};
	}
    class SDK
    {
    public:

		json_value &json;

        SDK (mv * mV, json_value &);
        ~SDK ();

        vector<ACEInfo *>	ActionInfos;
		vector<ACEInfo *>	ConditionInfos;
		vector<ACEInfo *>	ExpressionInfos;

        void ** ActionJumps;
        void ** ConditionJumps;
        void ** ExpressionJumps;

        vector<void *> ActionFunctions;
        vector<void *> ConditionFunctions;
        vector<void *> ExpressionFunctions;
		PropData * EdittimeProperties;

        unsigned char * FunctionMemory;

		mv* mV;
        cSurface * Icon;
    };
    
    class Runtime
    {
    protected:

        RUNDATA * rdPtr;

    public:

        long param1, param2;

        Runtime(RUNDATA * _rdPtr);
        ~Runtime();

        void Rehandle();

        void GenerateEvent(int EventID);
        void PushEvent(int EventID);

        void * Allocate(size_t);
        TCHAR * CopyString(const TCHAR *);

        void Pause();
        void Resume();

        void Redisplay();
        void Redraw();
		RunObject * RunObjPtrFromFixed(int fixedValue);
		int FixedFromRunObjPtr(RunObject * object);

		void SetPosition(int X, int Y);
		CallTables * GetCallTables();
        void CallMovement(int ID, long Parameter);

        void Destroy();

        void GetApplicationDrive(TCHAR * Buffer);
        void GetApplicationDirectory(TCHAR * Buffer);
        void GetApplicationPath(TCHAR * Buffer);
        void GetApplicationName(TCHAR * Buffer);
        void GetApplicationTempPath(TCHAR * Buffer);

        void ExecuteProgram(ParamProgram * Program);

        long EditInteger(EditDebugInfo *);
        long EditText(EditDebugInfo *);

        bool IsHWA();
        bool IsUnicode();

        event &CurrentEvent();

        Riggs::ObjectSelection ObjectSelection;

        void WriteGlobal(const TCHAR * Name, void * Value);
        void * ReadGlobal(const TCHAR * Name);

        #ifdef EdifUseJS
        
            JSContext * GetJSContext();

        #endif
            
    };

    extern bool ExternalJSON;

    void GetSiblingPath (TCHAR * Buffer, const TCHAR * Extension);
    
    const int DependencyNotFound     = 0;
    const int DependencyWasFile      = 1;
    const int DependencyWasResource  = 2;

    int GetDependency (char *& Buffer, size_t &Size, const TCHAR * FileExtension, int Resource);

	TCHAR * ConvertString(const char* urf8String);
	TCHAR * ConvertAndCopyString(TCHAR* tstr, const char* urf8String, int maxLength);
	inline void FreeString(TCHAR* s)
	{
		free(s);
	}

    extern TCHAR LanguageCode[3];
    extern bool IsEdittime;

    extern HMENU ActionMenu, ConditionMenu, ExpressionMenu;

    HMENU LoadMenuJSON (int BaseID, const json_value &Source, HMENU Parent = 0);

    int Init(mv * mV);
    void Init(mv * mV, EDITDATA * edPtr);

    void Free(mv * mV);
    void Free(EDITDATA * edPtr);

    long __stdcall Condition (RUNDATA * rdPtr, long param1, long param2);
    short __stdcall Action (RUNDATA * rdPtr, long param1, long param2);
    long __stdcall Expression (RUNDATA * rdPtr, long param);
   
    inline int ActionID(int ID)
    {
        return 25000 + ID;
    }

    inline int ConditionID(int ID)
    {
        return 26000 + ID;
    }

    inline int ExpressionID(int ID)
    {
        return 27000 + ID;
    }

    template<class T> inline void * MemberFunctionPointer(T Function)
    {
        T _Function = Function;
        return *(void **) &_Function;
    }
}
extern Edif::SDK * SDK;