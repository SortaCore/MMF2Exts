////////////////////
// ObjInfo structure
// Note: mainly used at runtime
// only some members of the oiHdr member are valid at editing time [which?]

#pragma once

struct ObjInfo {
	ObjInfoHeader		oiHdr;			// Header
	TCHAR *				oiName;			// name
	Objects_Common *	oiOC;

	unsigned int		oiFileOffset,
						oiLoadFlags;
	unsigned short		oiLoadCount,
						oiCount;

	// HWA only
	unsigned char *		oiExtEffect;
	struct CEffectEx *	oiExtEffectEx;	// For backdrops, you must find a more efficient system (bank of effects for backdrops)
};
