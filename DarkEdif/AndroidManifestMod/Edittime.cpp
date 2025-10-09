// ============================================================================
// This file contains routines that are handled only during the Edittime,
// under the Frame and Event editors.
//
// Including creating, display, and setting up your object.
// ============================================================================

#include "Common.hpp"

// ============================================================================
// ROUTINES USED UNDER FRAME EDITOR
// ============================================================================

#if EditorBuild

// Called once object is created or modified, just after setup.
// Also called before showing the "Insert an object" dialog if your object
// has no icon resource
int FusionAPI MakeIconEx(mv * mV, cSurface * pIconSf, TCHAR * lpName, ObjInfo * oiPtr, EDITDATA * edPtr)
{
#pragma DllExportHint
	pIconSf->Delete();
	pIconSf->Clone(*Edif::SDK->Icon);

	pIconSf->SetTransparentColor(RGB(255, 0, 255));
	return 0;
}

// Called when you choose "Create new object". It should display the setup box
// and initialize everything in the datazone.
int FusionAPI CreateObject(mv * mV, LevelObject * loPtr, EDITDATA * edPtr)
{
#pragma DllExportHint
	if (!Edif::IS_COMPATIBLE(mV))
		return -1;

	Edif::Init(mV, edPtr);
	return DarkEdif::DLL::DLL_CreateObject(mV, loPtr, edPtr);
}

// Displays the object under the frame editor
void FusionAPI EditorDisplay(mv *mV, ObjectInfo * oiPtr, LevelObject * loPtr, EDITDATA * edPtr, RECT * rc)
{
#pragma DllExportHint
	cSurface * Surface = WinGetSurface((int) mV->IdEditWin);
	if (!Surface)
		return;

	// If you don't have this function run in Edittime.cpp, SDK Updater will be disabled for your ext
	// Don't comment or preprocessor-it out if you're removing it; delete the line entirely.
	DarkEdif::SDKUpdater::RunUpdateNotifs(mV, edPtr);

	Edif::SDK->Icon->Blit(*Surface, rc->left, rc->top, BMODE_TRANSP, BOP_COPY, 0);
}

// This routine tells MMF2 if the mouse pointer is over a transparent zone of the object.
// If not exported, the entire display is assumed to be opaque.
/*
BOOL FusionAPI IsTransparent(mv *mV, LevelObject * loPtr, EDITDATA * edPtr, int dx, int dy)
{
#pragma DllExportHint
	return FALSE;
}
*/

// Called when the object has been resized
/*
BOOL FusionAPI SetEditSize(mv * mv, EDITDATA * edPtr, int cx, int cy)
{
#pragma DllExportHint
	// Check compatibility
	if (!Edif::IS_COMPATIBLE(mV))
		return FALSE;

	edPtr->swidth = cx;
	edPtr->sheight = cy;
	return TRUE;
}
*/

// Returns the size of the rectangle of the object in the frame editor.
// If this function isn't define, a size of 32x32 is assumed.
/* void FusionAPI GetObjectRect(mv * mV, RECT * rc, LevelObject * loPtr, EDITDATA * edPtr)
{
#pragma DllExportHint
	if (!mV || !rc || !edPtr)
		return;

	rc->right = rc->left + SDK->Icon->GetWidth();	// edPtr->swidth;
	rc->bottom = rc->top + SDK->Icon->GetHeight();	// edPtr->sheight;
}*/

// Called when the user selects the Edit command in the object's popup menu
/*
BOOL FusionAPI EditObject(mv *mV, ObjInfo * oiPtr, LevelObject * loPtr, EDITDATA * edPtr)
{
#pragma DllExportHint
	// Check compatibility
	if (!Edif::IS_COMPATIBLE(mV))
		return FALSE;

	// do stuff
	return TRUE;
}
*/


// ============================================================================
// PROPERTIES
// ============================================================================

// Inserts properties into the properties of the object.
BOOL FusionAPI GetProperties(mv * mV, EDITDATA * edPtr, BOOL bMasterItem)
{
#pragma DllExportHint
	return DarkEdif::DLL::DLL_GetProperties(mV, edPtr, bMasterItem);
}

// Called when the properties are removed from the property window.
void FusionAPI ReleaseProperties(mv * mV, EDITDATA * edPtr, BOOL bMasterItem)
{
#pragma DllExportHint
	return DarkEdif::DLL::DLL_ReleaseProperties(mV, edPtr, bMasterItem);
}

// Returns the value of properties that have a value.
// Note: see GetPropCheck for checkbox properties
void * FusionAPI GetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	return DarkEdif::DLL::DLL_GetPropValue(mV, edPtr, PropID);
}

// Returns the checked state of properties that have a check box.
BOOL FusionAPI GetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	return DarkEdif::DLL::DLL_GetPropCheck(mV, edPtr, PropID);
}

// Called by Fusion after a property has been modified.
void FusionAPI SetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID, void * Param)
{
#pragma DllExportHint
	DarkEdif::DLL::DLL_SetPropValue(mV, edPtr, PropID, Param);
}

// Called by Fusion when the user modifies a checkbox in the properties.
void FusionAPI SetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID, BOOL checked)
{
#pragma DllExportHint
	DarkEdif::DLL::DLL_SetPropCheck(mV, edPtr, PropID, checked);
}

// Called by Fusion when the user clicks the button of a Button, Image List or Font property.
BOOL FusionAPI EditProp(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	return DarkEdif::DLL::DLL_EditProp(mV, edPtr, PropID);
}

// Called by Fusion to request the enabled state of a property.
BOOL FusionAPI IsPropEnabled(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	// Example
	// -------
/*
	switch (nPropID) {

	case PROPID_CHECK:
		return (edPtr->nComboIndex != 0);
	}
*/
	return DarkEdif::DLL::DLL_IsPropEnabled(mV, edPtr, PropID);
}


// Called when a property is initialized and its creation parameter is NULL (in the PropData).
// Allows you, for example, to change the content of a combobox property according to specific settings in the EDITDATA structure.
/*LPARAM FusionAPI GetPropCreateParam(mv *mV, EDITDATA *edPtr, unsigned int PropID)
{
#pragma DllExportHint
	// Example
	// -------
	//	if ( PropID == PROPID_COMBO )
	//	{
	//		switch (edPtr->sType)
	//		{
	//		case TYPE1:
	//			return (LPARAM)ComboList1;
	//		case TYPE2:
	//			return (LPARAM)ComboList2;
	//		}
	//	}

	return DarkEdif::DLL::DLL_GetPropCreateParam(mV, edPtr, PropID);
}*/

// Called after a property has been initialized.
// Allows you, for example, to free memory allocated in GetPropCreateParam.
/*void FusionAPI ReleasePropCreateParam(mv *mV, EDITDATA *edPtr, unsigned int PropID, LPARAM lParam)
{
#pragma DllExportHint
	return DarkEdif::DLL::DLL_ReleasePropCreateParam(mV, edPtr, PropID, lParam);
}*/

// ============================================================================
// TEXT PROPERTIES
// ============================================================================

// Return the text capabilities of the object under the frame editor.
/*std::uint32_t FusionAPI GetTextCaps(mv * mV, EDITDATA * edPtr)
{
#pragma DllExportHint
	return 0;	// (TEXT_ALIGN_LEFT|TEXT_ALIGN_HCENTER|TEXT_ALIGN_RIGHT|TEXT_ALIGN_TOP|TEXT_ALIGN_VCENTER|TEXT_ALIGN_BOTTOM|TEXT_FONT|TEXT_COLOR);
}*/

// Return the font used the object.
// Note: the pStyle and cbSize parameters are obsolete and passed for compatibility reasons only.
/*BOOL FusionAPI GetTextFont(mv * mV, EDITDATA * edPtr, LOGFONT * Font, TCHAR * pStyle, unsigned int cbSize)
{
#pragma DllExportHint
	// Example: copy LOGFONT structure from EDITDATA
	// memcpy(plf, &edPtr->m_lf, sizeof(LOGFONT));

	return TRUE;
}*/

// Change the font used the object.
// Note: the pStyle parameter is obsolete and passed for compatibility reasons only.
//
/*BOOL FusionAPI SetTextFont(mv * mV, EDITDATA * edPtr, LOGFONT * Font, [[deprecated]] const char * pStyle)
{
#pragma DllExportHint
	// Example: copy LOGFONT structure to EDITDATA
	// memcpy(&edPtr->m_lf, plf, sizeof(LOGFONT));

	return TRUE;
}*/

// Get the text color of the object.
/*COLORREF FusionAPI GetTextClr(mv * mV, EDITDATA * edPtr)
{
#pragma DllExportHint
	return 0; // try RGB()
}*/

// Called by Fusion to set the text color of the object.
/*void FusionAPI SetTextClr(mv *mV, EDITDATA * edPtr, COLORREF color)
{
#pragma DllExportHint
	// Example
	// edPtr->fontColor = color;
}*/

// Get the text alignment of the object.
/*std::uint32_t FusionAPI GetTextAlignment(mv *mV, EDITDATA * edPtr)
{
#pragma DllExportHint
	return 0;
}*/

// Set the text alignment of the object.
/*void FusionAPI SetTextAlignment(mv *mV, EDITDATA * edPtr, unsigned int AlignFlags)
{
#pragma DllExportHint
}*/


// ============================================================================
// ROUTINES USED WHEN BUILDING
// ============================================================================

// Uses RAII to close the file when the struct is destructed
struct AutoFileCloser
{
	FILE* fil;
	AutoFileCloser(FILE* fil) : fil(fil) {}
	AutoFileCloser(AutoFileCloser&) = delete;
	AutoFileCloser(AutoFileCloser&&) = delete;
	AutoFileCloser() = delete;
	~AutoFileCloser()
	{
		if (fil)
			fclose(fil);
	}
};

// Turns number into ordinal; e.g. 1 to "1st", 2 to "2nd", etc.
std::tstring NumberToOrdinal(std::size_t number) {
	const TCHAR * suffix = _T("th");
	if (number % 100 < 11 || number % 100 > 13) {
		if (number % 10 == 1)
			suffix = _T("st");
		else if (number % 10 == 2)
			suffix = _T("nd");
		else if (number % 10 == 3)
			suffix = _T("rd");
	}
	return std::to_tstring(number) + suffix;
}

TCHAR errtextbuffer[256];

// Reads the current errno as text, and merges error number and text.
const TCHAR* errnotext()
{
	TCHAR bob[256];
	int err = errno;
	if (_tcserror_s(bob, std::size(bob), err))
		_tcscpy_s(bob, std::size(bob), _T("(text unknown)"));
	_stprintf_s(errtextbuffer, std::size(errtextbuffer), _T("%d: %s"), err, bob);
	return errtextbuffer;
}

// Reads the current GetLastError() as text, and merges error number and text.
const TCHAR* GetLastErrorText()
{
	DWORD lastErr = GetLastError();

	if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, lastErr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errtextbuffer,
		std::size(errtextbuffer), 0) <= 0)
	{
		_stprintf_s(errtextbuffer, std::size(errtextbuffer), _T("%u: (unknown error)"), lastErr);
	}
	else
	{
		// error messages end with crlf sometimes...
		std::size_t errtextbufferlen = _tcslen(errtextbuffer);
		if (errtextbuffer[errtextbufferlen - 1] == _T('\n'))
			errtextbuffer[errtextbufferlen - 2] = _T('\0');
	}

	return errtextbuffer;
}

void replaceAll(std::string &replaceIn, std::string_view from, std::string_view to)
{
	std::size_t start_pos = 0;
	while ((start_pos = replaceIn.find(from, start_pos)) != std::string::npos) {
		replaceIn.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}

#include <io.h> // for _chsize_s, file truncating
#include <fstream>
#include <regex>

// This routine is called by Fusion when an Android build is prepared before building.
// It enables you to modify the Android manifest file to add your own content, or otherwise check the Android build.
// It is called in the Extensions[\Unicode] MFX, for any extension in the MFA that defines PrepareAndroidBuild,
// including exts that have no corresponding Data\Runtime\Android file and would create a not-compatible build warning.
void FusionAPI PrepareAndroidBuild(mv* mV, EDITDATA* edPtr, LPCTSTR androidDirectoryPathname)
{
#pragma DllExportHint

	// First, we'll check if edPtr has data to use
	std::string userManifestModifications = DarkEdif::TStringToUTF8(edPtr->Props.GetPropertyStr("Data to replace"sv));
	if (userManifestModifications.empty())
		return; // nothing to do

	if (userManifestModifications.find("# TO #\r\n"sv) == std::string_view::npos)
		return DarkEdif::MsgBox::Error(_T("Error modifying manifest"), _T("Property should be blank, or have valid data in it. Current data:\n%s"), DarkEdif::UTF8ToTString(userManifestModifications).c_str());

	std::ofstream log;
	std::tstring debugPath;
	// First, we'll check if edPtr has data to use
	if (edPtr->Props.IsPropChecked("Debug paths"sv))
	{
		debugPath = edPtr->Props.GetPropertyStr("Debug paths"sv);
		if (debugPath.empty())
			return DarkEdif::MsgBox::Error(_T("Error copying manifest"), _T("Specified to copy out the manifests, but no folder path given."));

		if (debugPath.back() != _T('\\'))
			debugPath += _T('\\');
		log.open((DarkEdif::TStringToUTF8(debugPath) + "AndroidManifestMod.log"s).c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
	}

	// Erase the manifest file so the build will fail
	std::tstring manifestPath = androidDirectoryPathname;
	manifestPath += _T("app\\src\\main\\AndroidManifest.xml"sv);

	if (!debugPath.empty())
	{
		std::tstring modifiedManifestPath = debugPath + _T("AndroidManifest_Original.xml");
		if (CopyFile(manifestPath.c_str(), modifiedManifestPath.c_str(), FALSE) == FALSE)
		{
			return DarkEdif::MsgBox::Error(_T("Error copying manifest"),
				_T("Copying the original manifest to path \"%s\" failed; error %s."),
				modifiedManifestPath.c_str(), GetLastErrorText()
			);
		}
	}

	FILE* file;
	if (_tfopen_s(&file, manifestPath.c_str(), _T("rb+")))
		return DarkEdif::MsgBox::Error(_T("Error modifying manifest"), _T("Couldn't open manifest file at path \"%s\", error %s."), manifestPath.c_str(), errnotext());
	AutoFileCloser fileCloser(file);

	if (fseek(file, 0, SEEK_END) != 0)
		return DarkEdif::MsgBox::Error(_T("Error modifying manifest"), _T("Couldn't seek end in manifest, error %s."), errnotext());
	long fileSize = ftell(file);
	if (fileSize <= 0)
		return DarkEdif::MsgBox::Error(_T("Error modifying manifest"), _T("Couldn't read end position in manifest, error %s."), errnotext());

	// Now we have size, roll back the read/write pointer to start
	if (fseek(file, 0, SEEK_SET) != 0)
		return DarkEdif::MsgBox::Error(_T("Error modifying manifest"), _T("Couldn't seek start in manifest, error %s."), errnotext());

	// Manifest is always in UTF-8, no BOM, using CRLF line endings.
	std::string manifestContent(fileSize, '\xFF');

	size_t actuallyRead = fread(manifestContent.data(), 1, fileSize, file);
	if (actuallyRead != fileSize)
		return DarkEdif::MsgBox::Error(_T("Error modifying manifest"), _T("Only read %zu of %ld bytes, error %s."), actuallyRead, fileSize, errnotext());

	// First, confirm that the manifest wasn't already modified, we'll tag it later ourselves when we modify it
	if (manifestContent.find("AndroidManifestMod"sv) != std::string_view::npos)
		return DarkEdif::MsgBox::Error(_T("Error modifying manifest"), _T("Manifest was altered already, but is attempting to be altered again.\nDo you have multiple AndroidManifestMod extensions?"));

	// Now we've read, roll back so we can write
	rewind(file);

	// data is in format:
	// ## FROM ##[\r\n]
	// Data[\r\n]
	// [\r\n]
	// ## TO ##[\r\n]
	// Data[\r\n]
	// [\r\n]
	// ## FROM ##[\r\n]...

	// For easier searching later, we'll add two CRLF newlines to beginning of user data
	// that way, the first FROM also has two preceding newlines
	userManifestModifications.insert(0, "\r\n\r\n"s);

	const std::string_view fromSV = "\r\n\r\n# FROM #\r\n"sv;
	const std::string_view toSV = "\r\n\r\n# TO #\r\n"sv;

	std::size_t fromStart = userManifestModifications.find(fromSV);
	if (fromStart == std::string::npos)
	{
		return DarkEdif::MsgBox::Error(_T("Error modifying manifest"),
			_T("Couldn't find any \"# FROM #\" in user list of things to change."));
	}

	for (std::size_t i = 1, fromSectionStart, fromSectionEnd;
		fromStart != std::string::npos;
		fromStart = userManifestModifications.find(fromSV, fromStart), ++i)
	{
		fromSectionStart = fromStart + fromSV.size();

		std::size_t nextTo = userManifestModifications.find(toSV, fromSectionStart);
		if (nextTo == std::string::npos)
		{
			return DarkEdif::MsgBox::Error(_T("Error modifying manifest"),
				_T("Property format invalid: The %s FROM...\n%.40s\n... has no matching TO."),
				NumberToOrdinal(i).c_str(),
				DarkEdif::UTF8ToTString(userManifestModifications.substr(fromSectionStart)).c_str());
		}
		fromSectionEnd = nextTo;

		// Next FROM, if it exists, is the end of this TO
		std::size_t toSectionStart = nextTo + toSV.size(), toSectionEnd;
		std::size_t nextFrom = userManifestModifications.find(fromSV, toSectionStart);
		if (nextFrom != std::string::npos)
			toSectionEnd = nextFrom - 4; // minus the TO section's preceding crlf, crlf
		else // if there's no FROM after this TO, assume that this TO ends at the end of the string
		{
			// we found a TO after a TO? bad format
			if (userManifestModifications.find(toSV, toSectionStart + 1) != std::string::npos)
			{
				return DarkEdif::MsgBox::Error(_T("Error modifying manifest"),
					_T("Property format invalid: The %s TO...\n%.40s\n... has another TO following, instead of a FROM."),
					NumberToOrdinal(i).c_str(),
					DarkEdif::UTF8ToTString(userManifestModifications.substr(toSectionStart)).c_str());
			}

			toSectionEnd = userManifestModifications.size();
			while (userManifestModifications[toSectionEnd - 1] == '\n')
				toSectionEnd -= 2; // minus ending crlf(s)
		}


		try
		{
			std::string searchForStr = userManifestModifications.substr(fromSectionStart, fromSectionEnd - fromSectionStart);
			replaceAll(searchForStr, "\\r"sv, "\r"sv);
			replaceAll(searchForStr, "\\n"sv, "\n"sv);
			replaceAll(searchForStr, "\\t"sv, "\t"sv);
			replaceAll(searchForStr, "\\\\"sv, "\\"sv);

			std::regex searchFor(searchForStr);
			std::string replaceWith(userManifestModifications.substr(toSectionStart, toSectionEnd - toSectionStart));
			replaceAll(replaceWith, "\\r"sv, "\r"sv);
			replaceAll(replaceWith, "\\n"sv, "\n"sv);
			replaceAll(replaceWith, "\\t"sv, "\t"sv);
			replaceAll(replaceWith, "\\\\"sv, "\\"sv);

			log << "REGEX ["sv << searchForStr << "]\n"sv
				<< "REPLACE WITH ["sv << replaceWith << "]\n"sv
				<< "FOUND = "sv;
			if (!std::regex_search(manifestContent, searchFor))
			{
				log << "FALSE\n\n"sv;
				log << "fromStart = " << fromStart << ", fromSectionStart = " << fromSectionStart
					<< ", fromSectionEnd = " << fromSectionEnd << ".\n";
				log << "toSectionStart = " << toSectionStart << ", toSectionEnd = " << toSectionEnd
					<< ".\nnextFrom = " << nextFrom << ".\n";
				return DarkEdif::MsgBox::Error(_T("Error modifying manifest"),
					_T("Property format was not found in manifest! The %s FROM...\n%.40s\n... was not found in the manifest."),
					NumberToOrdinal(i).c_str(),
					DarkEdif::UTF8ToTString(userManifestModifications.substr(fromSectionEnd)).c_str());
			}
			log << "TRUE\n\n"sv;

			std::string result = std::regex_replace(manifestContent, searchFor, replaceWith);
			manifestContent = result;
		}
		catch (std::regex_error e)
		{
			return DarkEdif::MsgBox::Error(_T("Error modifying manifest"),
				_T("Property format invalid: Regex error \"%s\".\nThe %s FROM:%.50s\nor the TO:%.50s\nis invalid."),
				DarkEdif::UTF8ToTString(e.what()).c_str(),
				NumberToOrdinal(i).c_str(),
				DarkEdif::UTF8ToTString(userManifestModifications.substr(fromSectionStart, fromSectionEnd)).c_str(),
				DarkEdif::UTF8ToTString(userManifestModifications.substr(toSectionStart, toSectionEnd)).c_str());
		}
		fromStart = toSectionEnd;
	}

	// store a little note so ManifestMod doesn't double-execute
	std::size_t appAt = manifestContent.find("<application "sv);
	manifestContent.insert(appAt, "<!-- Modified by AndroidManifestMod -->\r\n\t"sv);

	std::size_t actuallyWritten = fwrite(manifestContent.data(), 1, manifestContent.size(), file);
	if (actuallyWritten != manifestContent.size())
	{
		if (errno == 0)
			errno = ferror(file);
		return DarkEdif::MsgBox::Error(_T("Error modifying manifest"),
			_T("Couldn't write new content to file. Wrote %zu of %zu bytes; error %s."),
			actuallyWritten, manifestContent.size(), errnotext());
	}
	// If new file size is somehow smaller than original file size, truncate it
	if ((std::size_t)fileSize > manifestContent.size() &&
		_chsize_s(_fileno(file), fileSize) != 0)
	{
		return DarkEdif::MsgBox::Error(_T("Error modifying manifest"),
			_T("Couldn't truncate file. Error %s."), errnotext());
	}
	if (fclose(file) != 0)
	{
		fileCloser.fil = NULL;
		return DarkEdif::MsgBox::Error(_T("Error modifying manifest"),
			_T("Writing the trailing file contents to disk got error %s."), errnotext());
	}
	fileCloser.fil = NULL;

	if (!debugPath.empty())
	{
		std::tstring modifiedManifestPath = debugPath + _T("AndroidManifest_Modified.xml");
		if (CopyFile(manifestPath.c_str(), modifiedManifestPath.c_str(), FALSE) == FALSE)
		{
			return DarkEdif::MsgBox::Error(_T("Error copying manifest"),
				_T("Copying the modified manifest to path \"%s\" failed; error %s."),
				modifiedManifestPath.c_str(), GetLastErrorText()
			);
		}
	}
}

#endif // EditorBuild
