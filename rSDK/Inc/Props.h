// Property definitions

#ifndef MMF2_Props_h
#define	MMF2_Props_h

///////////////////////////////////////////////////////
//
// Property data - used in property definition tables
//
typedef struct PropData
{
	int			dwID;				// Identifier
	UINT_PTR	sName;				// Name = ID of the property name in the resources, or LPCSTR
	UINT_PTR	sInfo;				// Info = ID of the property description in the resources, or LPCSTR
	UINT_PTR	lType;				// Property type, or pointer to CPropItem (custom properties)
	DWORD		dwOptions;			// Options (check box, bold, etc)
	LPARAM		lCreateParam;		// Parameter
} PropData;

#ifdef __cplusplus

///////////////////////////////////////////////////////
//
// Property values - used to set or get property values
//
class CPropValue;

//////////////
// Base class
//////////////
//
class CPropValue
{
protected:
	virtual ~CPropValue() {}
public:
	CPropValue() {}

	virtual void Delete() = 0;
	virtual CPropValue* CreateCopy() = 0;
	virtual BOOL IsEqual(CPropValue* value) = 0;
	virtual DWORD GetClassID() = 0;
};

//////////////
// Int
//////////////
//
class CPropIntValue : public CPropValue
{
protected:
	virtual ~CPropIntValue() {}
public:
	CPropIntValue() { m_nValue = 0; }
	CPropIntValue(int nValue) { m_nValue = nValue; }

	virtual void Delete() { delete this; }
	virtual CPropValue* CreateCopy() { return new CPropIntValue(m_nValue); }
	virtual BOOL IsEqual(CPropValue* value) { return (m_nValue == ((CPropIntValue*)value)->m_nValue); }
	virtual DWORD GetClassID() { return 'INT '; }

public:
	int		m_nValue;
};

//////////////
// DWORD
//////////////
//
class CPropDWordValue : public CPropValue
{
protected:
	virtual ~CPropDWordValue() {}
public:
	CPropDWordValue() { m_dwValue = 0L; }
	CPropDWordValue(DWORD dwValue) { m_dwValue = dwValue; }

	virtual void Delete() { delete this; }
	virtual CPropValue* CreateCopy() { return new CPropDWordValue(m_dwValue); }
	virtual BOOL IsEqual(CPropValue* value) { return (m_dwValue == ((CPropDWordValue*)value)->m_dwValue); }
	virtual DWORD GetClassID() { return 'DWRD'; }

public:
	DWORD	m_dwValue;
};

//////////////
// Float
//////////////
//
class CPropFloatValue : public CPropValue
{
protected:
	virtual ~CPropFloatValue() {}
public:
	CPropFloatValue() { m_fValue = 0.0f; }
	CPropFloatValue(float fValue) { m_fValue = fValue; }

	virtual void Delete() { delete this; }
	virtual CPropValue* CreateCopy() { return new CPropFloatValue(m_fValue); }
	virtual BOOL IsEqual(CPropValue* value) { return (m_fValue == ((CPropFloatValue*)value)->m_fValue); }
	virtual DWORD GetClassID() { return 'FLOT'; }

public:
	float	m_fValue;
};

//////////////
// Double
//////////////
//
class CPropDoubleValue : public CPropValue
{
protected:
	virtual ~CPropDoubleValue() {}
public:
	CPropDoubleValue() { m_dValue = 0.0; }
	CPropDoubleValue(double dValue) { m_dValue = dValue; }

	virtual void Delete() { delete this; }
	virtual CPropValue* CreateCopy() { return new CPropDoubleValue(m_dValue); }
	virtual BOOL IsEqual(CPropValue* value) { return (m_dValue == ((CPropDoubleValue*)value)->m_dValue); }
	virtual DWORD GetClassID() { return 'DBLE'; }

public:
	double	m_dValue;
};

//////////////
// Size
//////////////
//
class CPropSizeValue : public CPropValue
{
protected:
	virtual ~CPropSizeValue() {}
public:
	CPropSizeValue() { m_cx = m_cy = 0L; }
	CPropSizeValue(int cx, int cy) { m_cx = cx; m_cy = cy; }

	virtual void Delete() { delete this; }
	virtual CPropValue* CreateCopy() { return new CPropSizeValue(m_cx, m_cy); }
	virtual BOOL IsEqual(CPropValue* value) { return (m_cx == ((CPropSizeValue*)value)->m_cx && m_cy == ((CPropSizeValue*)value)->m_cy); }
	virtual DWORD GetClassID() { return 'SIZE'; }

public:
	int		m_cx;
	int		m_cy;
};

//////////////
// Int64
//////////////
//
class CPropInt64Value : public CPropValue
{
protected:
	virtual ~CPropInt64Value() {}
public:
	CPropInt64Value() { m_nValue = 0; }
	CPropInt64Value(__int64 nValue) { m_nValue = nValue; }

	virtual void Delete() { delete this; }
	virtual CPropValue* CreateCopy() { return new CPropInt64Value(m_nValue); }
	virtual BOOL IsEqual(CPropValue* value) { return (m_nValue == ((CPropInt64Value*)value)->m_nValue); }
	virtual DWORD GetClassID() { return 'INT2'; }

public:
	__int64	m_nValue;
};

//////////////
// Pointer
//////////////
//
class CPropPtrValue : public CPropValue
{
protected:
	virtual ~CPropPtrValue() {}
public:
	CPropPtrValue() { m_ptr = NULL; }
	CPropPtrValue(LPVOID ptr) { m_ptr = ptr; }

	virtual void Delete() { delete this; }
	virtual CPropValue* CreateCopy() { return new CPropPtrValue(m_ptr); }
	virtual BOOL IsEqual(CPropValue* value) { return (m_ptr == ((CPropPtrValue*)value)->m_ptr); }
	virtual DWORD GetClassID() { return 'LPTR'; }

public:
	LPVOID	m_ptr;
};

//////////////
// Buffer
//////////////
//
class CPropDataValue : public CPropValue
{
protected:
	virtual ~CPropDataValue() {}
public:
	CPropDataValue() { m_dwDataSize = 0L; m_pData = 0; }
	CPropDataValue(DWORD dwDataSize, LPBYTE pData) {
		m_dwDataSize = dwDataSize;
		m_pData = NULL;
		if ( dwDataSize != 0 )
		{
			m_pData = (LPBYTE)malloc(dwDataSize);
			if ( m_pData != NULL )
			{
				if ( pData != NULL )
					memcpy(m_pData, pData, dwDataSize);
			}
			else
				m_dwDataSize = 0;
		}
	}
	CPropDataValue(LPCSTR pStr) {
		m_dwDataSize = 0;
		m_pData = NULL;
		if ( pStr == NULL )
			return;
		m_pData = (LPBYTE)_strdup(pStr);
		m_dwDataSize = strlen((LPCSTR)m_pData)+1;
	};
	CPropDataValue(LPCWSTR pStr) {
		m_dwDataSize = 0;
		m_pData = NULL;
		if ( pStr == NULL )
			return;
		m_pData = (LPBYTE)_wcsdup(pStr);
		m_dwDataSize = (wcslen((LPCWSTR)m_pData)+1)*2;
	};

	virtual void Delete() { free(m_pData); m_pData = NULL; m_dwDataSize = 0; delete this; }
	virtual CPropValue* CreateCopy() { return new CPropDataValue(m_dwDataSize, m_pData); }
	virtual BOOL IsEqual(CPropValue* value) {
		if ( m_dwDataSize != ((CPropDataValue*)value)->m_dwDataSize )
			return FALSE;
		if ( m_dwDataSize == 0 )
			return TRUE;
		if ( m_pData == NULL || ((CPropDataValue*)value)->m_pData == NULL )
			return FALSE;
		return (memcmp(m_pData, ((CPropDataValue*)value)->m_pData, m_dwDataSize) == 0);
	}
	virtual DWORD GetClassID() { return 'DATA'; }

public:
	DWORD	m_dwDataSize;
	LPBYTE	m_pData;
};

//////////////////
// Custom property
//////////////////
class CCustomProp
{
public:
	CCustomProp() {}

	virtual void		Initialize(HINSTANCE hInst, LPARAM lCreateParam, BOOL bReInit) {}
	virtual void		Delete() { delete this; }

	virtual void		Draw(HDC hDC, LPRECT pRc, HFONT hFont, COLORREF textColor) {}
	virtual CPropValue*	GetPropValue() { return NULL; }
	virtual void		SetPropValue(CPropValue* pValue) {}

	virtual void		Activate(HWND hParent, LPRECT pRc, HFONT hFont) {}
	virtual void		Deactivate() {}
	virtual void		Refresh() {}
	virtual void		Move(LPRECT pRc) {}
	virtual BOOL		UpdateData() { return FALSE; }

protected:
	virtual ~CCustomProp() {}
};

typedef	CCustomProp* (CALLBACK* CP_CREATEINSTANCE)();

typedef struct CustomPropCreateStruct {

	CP_CREATEINSTANCE	m_pCreateCustomPropFnc;
	LPARAM				m_lCreateParam;

} CustomPropCreateStruct;

// Notification Codes for custom properties
#define PWN_FIRST					(0U-2000U)

// Tells property window to validate the property item
// => the property window gets the property value from the property item 
//    and applies it to the other selected items
#define PWN_VALIDATECUSTOMITEM		(PWN_FIRST-1)

// Parameter structure for the PWN_VALIDATECUSTOMITEM notification message
typedef struct _NMPROPWND
{
    NMHDR			hdr;
	CCustomProp*	pCP;
} NMPROPWND, *PNMPROPWND, FAR *LPNMPROPWND;

// Example:
//
// NMPROPWND nmp;
// UINT id = (UINT)IDC_CUSTOMITEM;	// identifier of the control that sends the notification message, if any
// nmp.hdr.code = PWN_VALIDATECUSTOMITEM;
// nmp.hdr.hwndFrom = m_hWnd;		// hWnd of the control that sends the notification message, if any
// nmp.hdr.idFrom = id;
// nmp.pCP = this;		// CCustomProp*
//
// // Edit property
// if ( OnEdit() )
// {
//		// Notify parent
//		::SendMessage(hParent, WM_NOTIFY, (WPARAM)id, (LPARAM)&nmp);
// }

// Internal symbol used by MMF, not important
#define IDC_CUSTOMITEM	1500

#endif // __cplusplus

////////////////////////////////////////////////
//
// Property types
//
enum {
	PROPTYPE_STATIC = 1,		// Simple static text
	PROPTYPE_FOLDER,			// Folder
	PROPTYPE_FOLDER_END,		// Folder End
	PROPTYPE_EDITBUTTON,		// Edit button, param1 = button text, or NULL if Edit
	PROPTYPE_EDIT_STRING,		// Edit box for strings, parameter = max length
	PROPTYPE_EDIT_NUMBER,		// Edit box for numbers, parameters = min value, max value, options (signed, float, spin)
	PROPTYPE_COMBOBOX,			// Combo box, parameters = list of strings, options (sorted, etc)
	PROPTYPE_SIZE,				// Size
	PROPTYPE_COLOR,				// Color
	PROPTYPE_LEFTCHECKBOX,		// Checkbox
	PROPTYPE_SLIDEREDIT,		// Edit + Slider
	PROPTYPE_SPINEDIT,			// Edit + Spin
	PROPTYPE_DIRCTRL,			// Direction Selector
	PROPTYPE_GROUP,				// Group
	PROPTYPE_LISTBTN,			// Internal, do not use
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
};

///////////////////
// Property options
///////////////////
//
#define	PROPOPT_NIL				0x00000000
#define	PROPOPT_CHECKBOX		0x00000001		// Left check box
#define PROPOPT_BOLD			0x00000002		// Name must be displayed in bold characters
#define PROPOPT_PARAMREQUIRED	0x00000004		// A non-null parameter must be provided
#define PROPOPT_REMOVABLE		0x00000008		// The property can be deleted by the user
#define PROPOPT_RENAMABLE		0x00000010		// The property can be renamed by the user
#define PROPOPT_MOVABLE			0x00000020		// The property can be moved by the user (list only)
#define	PROPOPT_LIST			0x00000040		// The property is a list
#define	PROPOPT_SINGLESEL		0x00000080		// This property is not displayed in multi-selection mode

// Internal, not used by extensions
#define PROPID_ROOT				0

/////////////////////////////
// Property definition macros
/////////////////////////////
//
#define PropData_Folder(id,name,info) {id,name,info,PROPTYPE_FOLDER,PROPOPT_NIL,(LPARAM)NULL}
#define PropData_Folder_End() {-1,0,0,PROPTYPE_FOLDER_END,PROPOPT_NIL,(LPARAM)NULL}
#define PropData_Group(id,name,info) {id,name,info,PROPTYPE_GROUP,PROPOPT_NIL,(LPARAM)NULL}
#define PropData_StaticString(id,name,info) {id,name,info,PROPTYPE_STATIC,PROPOPT_NIL,(LPARAM)NULL}
#define PropData_StaticString_Opt(id,name,info,opt) {id,name,info,PROPTYPE_STATIC,opt,(LPARAM)NULL}
#define PropData_StaticString_List(id,name,info) {id,name,info,PROPTYPE_STATIC,PROPOPT_LIST,(LPARAM)NULL}
#define PropData_EditString(id,name,info) {id,name,info,PROPTYPE_EDIT_STRING,PROPOPT_NIL,(LPARAM)NULL}
#define PropData_EditString_Check(id,name,info) {id,name,info,PROPTYPE_EDIT_STRING,PROPOPT_CHECKBOX,(LPARAM)NULL}
#define PropData_EditString_Opt(id,name,info,opt) {id,name,info,PROPTYPE_EDIT_STRING,opt,(LPARAM)NULL}
#define PropData_EditNumber(id,name,info) {id,name,info,PROPTYPE_EDIT_NUMBER,PROPOPT_NIL,(LPARAM)NULL}
#define PropData_EditNumber_Check(id,name,info) {id,name,info,PROPTYPE_EDIT_NUMBER,PROPOPT_CHECKBOX,(LPARAM)NULL}
#define PropData_EditNumber_Opt(id,name,info,opt) {id,name,info,PROPTYPE_EDIT_NUMBER,opt,(LPARAM)NULL}
#define PropData_EditFloat(id,name,info) {id,name,info,PROPTYPE_EDIT_FLOAT,PROPOPT_NIL,(LPARAM)NULL}
#define PropData_EditFloat_Check(id,name,info) {id,name,info,PROPTYPE_EDIT_FLOAT,PROPOPT_CHECKBOX,(LPARAM)NULL}
#define PropData_EditFloat_Opt(id,name,info,opt) {id,name,info,PROPTYPE_EDIT_FLOAT,opt,(LPARAM)NULL}
#define PropData_EditMultiLine(id,name,info) {id,name,info,PROPTYPE_EDIT_MULTILINE,PROPOPT_NIL,(LPARAM)NULL}
#define PropData_EditMultiLine_Check(id,name,info) {id,name,info,PROPTYPE_EDIT_MULTILINE,PROPOPT_CHECKBOX,(LPARAM)NULL}
#define PropData_EditMultiLine_Opt(id,name,info,opt) {id,name,info,PROPTYPE_EDIT_MULTILINE,opt,(LPARAM)NULL}
#define PropData_SliderEdit(id,name,info,minmax) {id,name,info,PROPTYPE_SLIDEREDIT,PROPOPT_PARAMREQUIRED,(LPARAM)minmax}
#define PropData_SliderEdit_Check(id,name,info,minmax) {id,name,info,PROPTYPE_SLIDEREDIT,(PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),(LPARAM)minmax}
#define PropData_SliderEdit_Opt(id,name,info,opt,minmax) {id,name,info,PROPTYPE_SLIDEREDIT,(PROPOPT_PARAMREQUIRED|opt),(LPARAM)minmax}
#define PropData_SpinEdit(id,name,info,minmax) {id,name,info,PROPTYPE_SPINEDIT,PROPOPT_PARAMREQUIRED,(LPARAM)minmax}
#define PropData_SpinEdit_Check(id,name,info,minmax) {id,name,info,PROPTYPE_SPINEDIT,(PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),(LPARAM)minmax}
#define PropData_SpinEdit_Opt(id,name,info,opt,minmax) {id,name,info,PROPTYPE_SPINEDIT,(PROPOPT_PARAMREQUIRED|opt),(LPARAM)minmax}
#define PropData_Button(id,name,info,text) {id,name,info,PROPTYPE_EDITBUTTON,PROPOPT_NIL, (LPARAM)text}
#define PropData_Button_Check(id,name,info,text) {id,name,info,PROPTYPE_EDITBUTTON,PROPOPT_CHECKBOX,(LPARAM)text}
#define PropData_Button_Opt(id,name,info,opt,text) {id,name,info,PROPTYPE_EDITBUTTON,opt,(LPARAM)text}
#define PropData_EditButton(id,name,info) {id,name,info,PROPTYPE_EDITBUTTON,PROPOPT_NIL, (LPARAM)NULL}
#define PropData_EditButton_Check(id,name,info) {id,name,info,PROPTYPE_EDITBUTTON,PROPOPT_CHECKBOX,(LPARAM)NULL}
#define PropData_EditButton_Opt(id,name,info,opt) {id,name,info,PROPTYPE_EDITBUTTON,opt,(LPARAM)NULL}
#define PropData_Size(id,name,info,tab) {id,name,info,PROPTYPE_SIZE,PROPOPT_PARAMREQUIRED,(LPARAM)tab}
#define PropData_Size_Check(id,name,info,tab) {id,name,info,PROPTYPE_SIZE,(PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),(LPARAM)tab}
#define PropData_Size_Opt(id,name,info,opt,tab) {id,name,info,PROPTYPE_SIZE,(PROPOPT_PARAMREQUIRED|opt),(LPARAM)tab}
#define PropData_Color(id,name,info) {id,name,info,PROPTYPE_COLOR,PROPOPT_NIL,(LPARAM)NULL}
#define PropData_Color_Check(id,name,info) {id,name,info,PROPTYPE_COLOR,PROPOPT_CHECKBOX,(LPARAM)NULL}
#define PropData_Color_Opt(id,name,info,opt) {id,name,info,PROPTYPE_COLOR,opt,(LPARAM)NULL}
#define PropData_ComboBox(id,name,info,list) {id,name,info,PROPTYPE_COMBOBOX,PROPOPT_PARAMREQUIRED,(LPARAM)list}
#define PropData_ComboBox_Check(id,name,info,list) {id,name,info,PROPTYPE_COMBOBOX,(PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),(LPARAM)list}
#define PropData_ComboBox_Opt(id,name,info,opt,list) {id,name,info,PROPTYPE_COMBOBOX,(PROPOPT_PARAMREQUIRED|opt),(LPARAM)list}
#define PropData_CheckBox(id,name,info) {id,name,info,PROPTYPE_LEFTCHECKBOX,PROPOPT_CHECKBOX,(LPARAM)NULL}
#define PropData_CheckBox_Opt(id,name,info,opt) {id,name,info,PROPTYPE_LEFTCHECKBOX,(PROPOPT_CHECKBOX|opt),(LPARAM)NULL}
#define PropData_DirCtrl(id,name,info,param) {id,name,info,PROPTYPE_DIRCTRL,PROPOPT_PARAMREQUIRED,(LPARAM)param}
#define PropData_DirCtrl_Check(id,name,info,param) {id,name,info,PROPTYPE_DIRCTRL,(PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),(LPARAM)param}
#define PropData_DirCtrl_Opt(id,name,info,opt,param) {id,name,info,PROPTYPE_DIRCTRL,(PROPOPT_PARAMREQUIRED|opt),(LPARAM)param}
#define PropData_Filename(id,name,info,param) {id,name,info,PROPTYPE_FILENAME,PROPOPT_PARAMREQUIRED,(LPARAM)param}
#define PropData_Filename_Check(id,name,info,param) {id,name,info,PROPTYPE_FILENAME,(PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),(LPARAM)param}
#define PropData_Filename_Opt(id,name,info,opt,param) {id,name,info,PROPTYPE_FILENAME,(PROPOPT_PARAMREQUIRED|opt),(LPARAM)param}
#define PropData_PictureFilename(id,name,info,param) {id,name,info,PROPTYPE_PICTUREFILENAME,PROPOPT_PARAMREQUIRED,(LPARAM)param}
#define PropData_PictureFilename_Check(id,name,info,param) {id,name,info,PROPTYPE_PICTUREFILENAME,(PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),(LPARAM)param}
#define PropData_PictureFilename_Opt(id,name,info,opt,param) {id,name,info,PROPTYPE_PICTUREFILENAME,(PROPOPT_PARAMREQUIRED|opt),(LPARAM)param}
#define PropData_Font(id,name,info,param) {id,name,info,PROPTYPE_FONT,PROPOPT_PARAMREQUIRED,(LPARAM)param}
#define PropData_Font_Check(id,name,info,param) {id,name,info,PROPTYPE_FONT,(PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),(LPARAM)param}
#define PropData_Font_Opt(id,name,info,opt,param) {id,name,info,PROPTYPE_FONT,(PROPOPT_PARAMREQUIRED|opt),(LPARAM)param}
#define PropData_Custom(id,name,info,param) {id,name,info,PROPTYPE_CUSTOM,PROPOPT_PARAMREQUIRED,(LPARAM)param}
#define PropData_Custom_Check(id,name,info,param) {id,name,info,PROPTYPE_CUSTOM,(PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),(LPARAM)param}
#define PropData_Custom_Opt(id,name,info,opt,param) {id,name,info,PROPTYPE_CUSTOM,(PROPOPT_PARAMREQUIRED|opt),(LPARAM)param}
#define PropData_ComboBoxBtn(id,name,info,list) {id,name,info,PROPTYPE_COMBOBOXBTN,PROPOPT_PARAMREQUIRED,(LPARAM)list}
#define PropData_ComboBoxBtn_Check(id,name,info,list) {id,name,info,PROPTYPE_COMBOBOXBTN,(PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),(LPARAM)list}
#define PropData_ComboBoxBtn_Opt(id,name,info,opt,list) {id,name,info,PROPTYPE_COMBOBOXBTN,(PROPOPT_PARAMREQUIRED|opt),(LPARAM)list}
#define PropData_ImageList(id,name,info) {id,name,info,PROPTYPE_IMAGELIST,0,NULL}
#define PropData_ImageList_Check(id,name,info) {id,name,info,PROPTYPE_IMAGELIST,PROPOPT_CHECKBOX,NULL}
#define PropData_ImageList_Opt(id,name,info,opt) {id,name,info,PROPTYPE_IMAGELIST,opt,NULL}
#define PropData_IconComboBox(id,name,info,list) {id,name,info,PROPTYPE_ICONCOMBOBOX,PROPOPT_PARAMREQUIRED,(LPARAM)list}
#define PropData_IconComboBox_Check(id,name,info,list) {id,name,info,PROPTYPE_ICONCOMBOBOX,(PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),(LPARAM)list}
#define PropData_IconComboBox_Opt(id,name,info,opt,list) {id,name,info,PROPTYPE_ICONCOMBOBOX,(PROPOPT_PARAMREQUIRED|opt),(LPARAM)list}
#define PropData_URLButton(id,name,info,url) {id,name,info,PROPTYPE_URLBUTTON,PROPOPT_PARAMREQUIRED, (LPARAM)url}
#define PropData_URLButton_Check(id,name,info,url) {id,name,info,PROPTYPE_URLBUTTON,(PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),(LPARAM)url}
#define PropData_URLButton_Opt(id,name,info,opt,url) {id,name,info,PROPTYPE_URLBUTTON,(PROPOPT_PARAMREQUIRED|opt),(LPARAM)url}
#define PropData_End() {0}

/////////////////////////////////
// Property definition parameters
/////////////////////////////////

// Parameter for slider & spin property items
typedef struct {
	int nMinValue;		// Minimum value (note: cannot be less than -32768 for SpinEdit property)
	int nMaxValue;		// Maximum value (note: cannot be greater than 32767 for SpinEdit property)
} MinMaxParam;
typedef MinMaxParam* LPMINMAXPARAM;

// Direction Control Styles
enum 
{
	DCS_NOBORDER	= 0,		// No border
	DCS_FLAT		= 1,		// Flat
	DCS_3D			= 2,		// 3D
	DCS_SLIDER		= 4,		// Slider to change the number of directions
	DCS_EMPTY		= 8,		// ??? should affect the appearance of direction spots...
	DCS_SETALL_BTNS	= 16,		// "Select All" and "Reset" buttons
};

// Initialization structure for direction property
typedef struct {
	BOOL	bMultiSel;	// TRUE for multi-selection
	int		numDirs;	// Number of directions: 4,8,16,32
	DWORD	style;		// Style DCS_NOBORDER, DCS_FLAT, DCS_3D, [DCS_SLIDER,] DCS_EMPTY, DCS_SETALL_BTNS
} DirCtrlCreateParam;

// Direction Property Value 
typedef struct {
	int		selDir;		// Direction index, single selection mode
	DWORD	selDir32;	// 32-bit direction mask, multi-selection mode
	int		numDirs;	// Number of directions (4, 8, 16 or 32)
	int		reserved;	// Not used
} PropDirValue;

// Initialization structure for file selector property
typedef struct {
	LPCSTR	extFilter;	// Filter string for GetOpenFilename dialog (for example "All Files (*.*)|*.*|")
	DWORD	options;	// Options for GetOpenFilename dialog (OFN_FILEMUSTEXIST, OFN_PATHMUSTEXIST, OFN_HIDEREADONLY, etc.)
} FilenameCreateParam;	

////////////////////////////////////////////////
//
// MMF property identifier
//

//////////
// Tab IDs
//////////
//
enum {
	PROPID_TAB_GENERAL=1,
	PROPID_TAB_DISPLAY,
	PROPID_TAB_WINDOW,
	PROPID_TAB_SIZEPOS,
	PROPID_TAB_TEXTOPT,
	PROPID_TAB_MOVEMENT,
	PROPID_TAB_RUNOPT,
	PROPID_TAB_VALUES,
	PROPID_TAB_EVENTS,
	PROPID_TAB_ABOUT,
	PROPID_TAB_CUSTOM1 = 15,
	PROPID_TAB_CUSTOM2,
	PROPID_TAB_CUSTOM3,
	PROPID_TAB_CUSTOM4,
	PROPID_TAB_CUSTOM5,
	PROPTAB_MAX				// 20
};

///////////////
// Property IDs
//////////
//
// If you need them, you can retrieve, set or refresh 
// standard properties with the mvSetPropValue, etc. macros
// and the following identifiers.
//

// Objects
enum {
	PROPID_ITEM_FIRST = 20,
	PROPID_ITEM_NAME,
	PROPID_ITEM_ICON,
	PROPID_ITEM_HELP,
	PROPID_ITEM_LAST,

	PROPID_FII_FIRST = 180,
	PROPID_FII_POS,					// Those properties are not available if the frame editor is not open
	PROPID_FII_X,
	PROPID_FII_Y,
	PROPID_FII_SIZE,
	PROPID_FII_CX,
	PROPID_FII_CY,
	PROPID_FII_ROT,
	PROPID_FII_ANGLE,
	PROPID_FII_LAST,

	PROPID_FITEM_FIRST=200,
	PROPID_FITEM_INKFXFOLDER,
	PROPID_FITEM_TRANSPARENT,
	PROPID_FITEM_INKFX,
	PROPID_FITEM_INKFXPARAM,
	PROPID_FITEM_ANTIA,
	PROPID_FITEM_MEMORYFOLDER,
	PROPID_FITEM_GLOBAL,
	PROPID_FITEM_LOADONCALL,
	PROPID_FITEM_OPTIONS,
	PROPID_FITEM_LAST=249,

	PROPID_DYNITEM_FIRST=250,

	PROPID_DYNITEM_TRANSITIONS=250,
	PROPID_DYNITEM_FADEIN,
	PROPID_DYNITEM_FADEOUT,
	PROPID_DYNITEM_OBSTACLE,
	PROPID_DYNITEM_OBSTACLETYPE,
	PROPID_DYNITEM_AUTOROTATEGROUP,
	PROPID_DYNITEM_AUTOROTATE,

	PROPID_MVT_FIRST=300,
	PROPID_DYNITEM_MVTFOLDER,
	PROPID_DYNITEM_MVTLIST,
	PROPID_DYNITEM_MVTTYPE,
	PROPID_DYNITEM_INITDIR,
	PROPID_DYNITEM_TRYMVT,
	PROPID_DYNITEM_SPEEDFOLDER,
	PROPID_DYNITEM_SPEED,
	PROPID_DYNITEM_DECEL,
	PROPID_DYNITEM_ACCEL,
	PROPID_DYNITEM_MOVEATSTART,
	PROPID_DYNITEM_BOUNCEFOLDER,
	PROPID_DYNITEM_NANGLES,
	PROPID_DYNITEM_RANDOMIZER,
	PROPID_DYNITEM_SECURITY,
	PROPID_DYNITEM_EDITMVT,
	PROPID_DYNITEM_PLAYER,
	PROPID_DYNITEM_DIRS,
	PROPID_DYNITEM_ENABLEREVERSE,
	PROPID_DYNITEM_ROTATION,
	PROPID_DYNITEM_ROTSPEED,
	PROPID_DYNITEM_JUMP,
	PROPID_DYNITEM_GRAVITY,
	PROPID_DYNITEM_JUMPSTRENGTH,
	PROPID_DYNITEM_JUMPCONTROL,
	PROPID_MVT_LAST,

	PROPID_DYNITEM_ALTVALUES_GROUP=350,
	PROPID_DYNITEM_ALTSTRINGS_GROUP,
	PROPID_DYNITEM_QUALIF,
	PROPID_DYNITEM_DISPLAY,
	PROPID_DYNITEM_VISIBLEATSTART,
	PROPID_DYNITEM_DISPLAYASBKD,
	PROPID_DYNITEM_BKDFOLDER,
	PROPID_DYNITEM_SAVEBKD,
	PROPID_DYNITEM_WIPEWITHCOLOR,
	PROPID_DYNITEM_CREATEBEFOREFADEIN,
	PROPID_DYNITEM_SCROLLING,
	PROPID_DYNITEM_FOLLOWFRAME,
	PROPID_DYNITEM_DESTROYIFTOOFAR,
	PROPID_DYNITEM_INACTIVATEIFTOOFAR,
	PROPID_DYNITEM_COLLISIONS,
	PROPID_DYNITEM_FINECOLL,
	PROPID_DYNITEM_BEHAVIOR_HDR,
	PROPID_DYNITEM_BEHAVIOR_END,
	PROPID_DYNITEM_CREATE,
	PROPID_DYNITEM_CREATEATSTART,

	PROPID_TEXT_FIRST=380,
	PROPID_TEXT_FONT,
	PROPID_TEXT_FACENAME,
	PROPID_TEXT_SIZE,
	PROPID_TEXT_BOLD,
	PROPID_TEXT_ITALIC,
	PROPID_TEXT_UNDERLINE,
	PROPID_TEXT_STRIKEOUT,
	PROPID_TEXT_COLOR,
	PROPID_TEXT_ALIGNGROUP,
	PROPID_TEXT_HALIGN,
	PROPID_TEXT_VALIGN,
	PROPID_TEXT_LAST,

	PROPID_DYNITEM_LAST,

	PROPID_EXTITEM_FIRST=400,
	PROPID_EXTITEM_EDIT,
	PROPID_EXTITEM_ABOUT,
	PROPID_EXTITEM_NAME,
	PROPID_EXTITEM_AUTHOR,
	PROPID_EXTITEM_COPYRIGHT,
	PROPID_EXTITEM_COMMENT,
	PROPID_EXTITEM_WEBSITE,
	PROPID_EXTITEM_FILENAME,
	PROPID_EXTITEM_LAST,

	PROPID_DYNITEM_ALTVALUE_FIRST=0x50000,
	PROPID_DYNITEM_ALTVALUE_LAST =0x50019,

	PROPID_DYNITEM_ALTSTRING_FIRST=0x50800,
	PROPID_DYNITEM_ALTSTRING_LAST =0x50809,

	PROPID_DYNITEM_BEHAVIOR_FIRST=0x51000,
	PROPID_DYNITEM_BEHAVIOR_LAST =0x51FFF,

	PROPID_MVTEXT_FIRST=0x52000,
	PROPID_MVTEXT_LAST =0x52FFF,

	PROPID_FADE_PARAM_FIRST=0x53000,
	PROPID_FADE_PARAM_LAST =0x53FFF,

	// Extensions must use property identifiers between PROPID_EXTITEM_CUSTOM_FIRST and PROPID_EXTITEM_CUSTOM_LAST
	PROPID_EXTITEM_CUSTOM_FIRST=0x80000,
	PROPID_EXTITEM_CUSTOM_LAST =0xFFFFF,
};

// Application
enum {
	PROPID_APP_FIRST = 50,
	PROPID_APP_ABOUT,
	PROPID_APP_AUTHOR,
	PROPID_APP_DESCRIPTION,
	PROPID_APP_COPYRIGHT,
	PROPID_APP_COMPANY,
	PROPID_APP_FILEVERSION,
	PROPID_APP_ICON,
	PROPID_APP_WINDOWSIZE,
	PROPID_APP_WINDOWBACKGROUNDCOLOR,
	PROPID_APP_WINDOWSTYLE,
	PROPID_APP_HEADING,
	PROPID_APP_HEADINGMAX,
	PROPID_APP_MENU,
	PROPID_APP_MENUBAR,
	PROPID_APP_MENUBOOTUP,
	PROPID_APP_NOMINIMIZEBOX,
	PROPID_APP_NOMAXIMIZEBOX,
	PROPID_APP_NOTHICKFRAME,
	PROPID_APP_MAXIMIZEDONBOOTUP,
	PROPID_APP_HIDDENATSTART,
	PROPID_APP_DISABLECLOSE,
	PROPID_APP_OPTIONS,
	PROPID_APP_FULLSCREEN,
	PROPID_APP_FULLSWITCH,
	PROPID_APP_RESIZETOWINDOWSIZE,
	PROPID_APP_DONOTCENTERFRAME,
	PROPID_APP_MACHINEINDEP,
	PROPID_APP_SCREENSAVER_NOAUTOSTOP,
	PROPID_APP_SOUNDOPT,
	PROPID_APP_MULTISAMPLE,
	PROPID_APP_SOUNDSOVERFRAMES,
	PROPID_APP_SAMPLESEVENIFNOTFOCUS,
	PROPID_APP_DISPOPT,
	PROPID_APP_DISPLAYMODE,
	PROPID_APP_ENABLEVISUALTHEMES,
	PROPID_APP_PLAYERS,
	PROPID_APP_INITSCORE,
	PROPID_APP_INITLIVES,
	PROPID_APP_FILENAME,
	PROPID_APP_HELP,
	PROPID_APP_HELPFILE,
	PROPID_APP_PLAYERCTRLS,
	PROPID_APP_VTZ,
	PROPID_APP_VTZPREVIEWFILE,
	PROPID_APP_GLOBALVALUE_GROUP,
	PROPID_APP_GLOBALSTRING_GROUP,
	PROPID_APP_GLOBALEVENTS,
	PROPID_APP_BUILD,
	PROPID_APP_BUILDTYPE,
	PROPID_APP_INSTALLSETTINGS,
	PROPID_APP_BUILDFILE,
	PROPID_APP_BUILDOPT,
	PROPID_APP_COMPLEVEL,
	PROPID_APP_COMPSND,
	PROPID_APP_ALLINONE,
	PROPID_APP_FRAMERATE,
	PROPID_APP_VSYNC,
	PROPID_APP_RUNWHENMINIMIZED,
	PROPID_APP_RUNOPTGROUP,
	PROPID_APP_RUNCMDLINE,
	PROPID_APP_MDI,
	PROPID_APP_WINMENUIDX,
	PROPID_APP_ABOUTTEXT,
	PROPID_APP_RUNWHILERESIZING,
	PROPID_APP_DEBUGGERSHORTCUTS,
	PROPID_APP_GRAPHICS,
	PROPID_APP_GRAPHICMODE,
	PROPID_APP_IMAGEFILTERS,
	PROPID_APP_SOUNDFILTERS,
	PROPID_APP_SHOWDEBUGGER,
	PROPID_APP_COMPRUNTIME,
	PROPID_APP_DONOTSHAREDATA,
	PROPID_APP_VTZU,
	PROPID_APP_VTZS,
	PROPID_APP_VISTAOPT,
	PROPID_APP_EXECLEVEL,
	PROPID_APP_LAST,

	PROPID_APP_GLOBALVALUE_FIRST = 100000,
	PROPID_APP_GLOBALVALUE_LAST =  199998,

	PROPID_APP_GLOBALSTRING_FIRST = 200000,
	PROPID_APP_GLOBALSTRING_LAST =  299998,
};

// Frame
enum {
	PROPID_FRAME_FIRST = 100,
	PROPID_FRAME_FRAME,
	PROPID_FRAME_SIZE,
	PROPID_FRAME_BACKGROUNDCOLOR,
	PROPID_FRAME_PALETTE,
	PROPID_FRAME_RUNOPT,
	PROPID_FRAME_GRABDESKTOP,
	PROPID_FRAME_KEEPDISPLAY,
	PROPID_FRAME_TOTALCOLMASK,
	PROPID_FRAME_FRAMETITLEINCAPTION,
	PROPID_FRAME_RESIZETOSCREENSIZE,
	PROPID_FRAME_FORCELOADONCALL,
	PROPID_FRAME_NOBJECTS,
	PROPID_FRAME_PASSWORD,
	PROPID_FRAME_TRANSITIONS,
	PROPID_FRAME_FADEIN,
	PROPID_FRAME_FADEOUT,
	PROPID_FRAME_NOSURFACE,
	PROPID_FRAME_SCRSETUP,
	PROPID_FRAME_VIRTUALRIGHT,
	PROPID_FRAME_VIRTUALBOTTOM,
	PROPID_FRAME_DEMO,
	PROPID_FRAME_DEMOFILENAME,
	PROPID_FRAME_RECORD,
	PROPID_FRAME_RANDOMSEED,
	PROPID_FRAME_LAST,
};

#endif // MMF2_Props_h
