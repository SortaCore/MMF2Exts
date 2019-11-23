
#ifndef PictEdDefs_h
#define PictEdDefs_h

#define PICTEDOPT_FIXEDIMGSIZE			0x0001		// User cannot change the image size
#define PICTEDOPT_HOTSPOT				0x0002		// Image has a hot spot
#define PICTEDOPT_ACTIONPOINT			0x0004		// Image has an action point
#define PICTEDOPT_16COLORS				0x0008		// 16 colors image
#define PICTEDOPT_FIXEDNFRAMES			0x0010		// User cannot add / remove frames (animation editor)
#define	PICTEDOPT_NUMBERS				0x0020		// Numbers (internal)
#define PICTEDOPT_NOTRANSPARENCY		0x0040		// The image has no transparent color
#define PICTEDOPT_NOALPHACHANNEL		0x0080		// The image cannot have an alpha channel
#define PICTEDOPT_APPICON				0x0100		// Internal
#define PICTEDOPT_ICONPALETTE			0x0200		// Internal
#define PICTEDOPT_CANBETRANSPARENT		0x0400		// The image can be empty (if this option is not specified, MMF refuses to close the picture editor if the image is empty)
#define PICTEDOPT_CANNOTMOVEFRAMES		0x0800		// Frames cannot be moved

#endif // PictEdDefs_h
