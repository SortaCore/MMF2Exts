// ============================================================================
//
// STRUCTS
// 
// ============================================================================
struct StructPassThru
{
	Extension * para_Extension;
	int para_totalFrames,
		para_numSamples,
		para_numBytes;
};

// Declare threads prematurely
DWORD WINAPI Record(StructPassThru *pDataArray);
