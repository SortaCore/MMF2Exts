// ==========================================================================================
// FUSION PICTURE EDITOR
// Used for the Fusion editor's image or animation editor, when invoked by the extension.
// Only usable in Windows edittime.
// ==========================================================================================
#pragma once

// For the enum bitmasking
#include "WindowsDefines.hpp"
// For the picture editor window title
#include <tchar.h>
// For the image content
#include "Surface.h"

// Picture editor open options; used for Fusion editor's animation and image editor
enum class _Enum_is_bitflag_ PictureEditorOptions : unsigned int {
	// User cannot change the image size
	FixedImageSize		= 0x0001,
	// Image has a hot spot
	HotSpot				= 0x0002,
	// Image has an action point
	ActionPoint			= 0x0004,
	// 16 colors image
	_16Colors			= 0x0008,
	// User cannot add / remove frames (animation editor only)
	FixedNFrames		= 0x0010,
	// Numbers (internal)
	Numbers				= 0x0020,
	// The image has no transparent color
	NoTransparency		= 0x0040,
	// The image cannot have an alpha channel
	NoAlphaChannel		= 0x0080,
	// Internal usage
	AppIcon				= 0x0100,
	// Internal usage
	IconPalette			= 0x0200,
	// The image can be empty (if this option is not specified, MMF refuses to close the picture editor if the image is empty)
	CanBeTransparent	= 0x0400,
	// Frames cannot be moved
	CannotMoveFrames	= 0x0800,	
};
enum_class_is_a_bitmask(PictureEditorOptions);

// ==========================================================================================
// Structures for picture editor
// There are technically two variants of all of these; an ANSI variant, and a Unicode variant.
// However, since we should only use one of them, we'll define the TCHAR variant and leave it
// be.
// ==========================================================================================

// Structures for picture editor
template<typename T>
struct EditSurfaceParamsT {
	unsigned int	size;			// sizeof(EditSurfaceParams)
	T *				WindowTitle;	// Picture Editor title (NULL = default title)
	cSurface *		Sf;				// Surface to edit
	unsigned int	Options;		// Options, see PictEdDefs.h
	unsigned int	FixedWidth;		// Default width or fixed width (if PICTEDOPT::FIXEDIMGSIZE is used)
	unsigned int	FixedHeight;	// Default height or fixed height (if PICTEDOPT::FIXEDIMGSIZE is used)
	POINT			hotSpot;		// Hot spot coordinates
	POINT			actionPoint;	// Action point coordinates
};
using EditSurfaceParamsA = EditSurfaceParamsT<char>;
using EditSurfaceParamsW = EditSurfaceParamsT<wchar_t>;
using EditSurfaceParams = EditSurfaceParamsT<TCHAR>;

template<typename T = TCHAR>
struct EditImageParamsT {
	unsigned int			size;			// sizeof(EditImageParams)
	T *						WindowTitle;	// Picture Editor title (NULL = default title)
	unsigned int			Image;			// Image to edit - note: only the LOWORD is used in this version
	PictureEditorOptions	Options;		// Options
	unsigned int			FixedWidth,		// Default width or fixed width (if PICTEDOPT::FIXEDIMGSIZE is used)
							FixedHeight;	// Default height or fixed height (if PICTEDOPT::FIXEDIMGSIZE is used)
};
using EditImageParamsA = EditImageParamsT<char>;
using EditImageParamsW = EditImageParamsT<wchar_t>;
using EditImageParams = EditImageParamsT<TCHAR>;

// Structure for image list editor
template<typename T = TCHAR>
struct EditAnimationParamsT {
	unsigned int			size;			// sizeof(EditAnimationParams)
	T *						WindowTitle;	// Picture Editor title (NULL = default title)
	int						NumberOfImages,	// Number of images in the list
							MaxImages,		// Maximum number of images in the list
							StartIndex;		// Index of first image to edit in the editor
	unsigned int *			Images;			// Image list (one unsigned short per image)
	T **					ImageTitles;	// Image titles (can be NULL)
	PictureEditorOptions	Options;		// Options
	unsigned int			FixedWidth,		// Default width or fixed width (if PICTEDOPT::FIXEDIMGSIZE is used)
							FixedHeight;	// Default height or fixed height (if PICTEDOPT::FIXEDIMGSIZE is used)
};
using EditAnimationParamsA = EditAnimationParamsT<char>;
using EditAnimationParamsW = EditAnimationParamsT<wchar_t>;
using EditAnimationParams = EditAnimationParamsT<TCHAR>;
