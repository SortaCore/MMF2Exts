#pragma once
#include "MMFWindowsMasterHeader.hpp"
#include "SafeSurface.hpp"

// Surface.h old defines


// Surface errors
constexpr unsigned int SURFACEERROR_MIN = (unsigned int)SurfaceError::Min;
constexpr unsigned int SURFACEERROR_MAX = (unsigned int)SurfaceError::Max;
constexpr unsigned int SURFACEERROR_NOERR = (unsigned int)SurfaceError::NoError;
constexpr unsigned int SURFACEERROR_NOT_SUPPORTED = (unsigned int)SurfaceError::NotSupported;
constexpr unsigned int SURFACEERROR_SURFACE_NOT_CREATED = (unsigned int)SurfaceError::Min;
constexpr unsigned int SURFACEERROR_INTERNAL = (unsigned int)SurfaceError::Internal;


// Surface types
[[deprecated("Use SurfaceType enum")]]
constexpr int ST_MEMORY = (int)SurfaceType::Memory;
[[deprecated("Use SurfaceType enum")]]
constexpr int ST_MEMORYWITHDC = (int)SurfaceType::Memory_DeviceContext;
[[deprecated("Use SurfaceType enum")]]
constexpr int ST_DDRAW_SYSTEMMEMORY = (int)SurfaceType::DirectDraw_SystemMemory;
[[deprecated("Use SurfaceType enum")]]
constexpr int ST_HWA_SCREEN = (int)SurfaceType::HWA_Screen;
[[deprecated("Use SurfaceType enum")]]
constexpr int ST_HWA_RTTEXTURE = (int)SurfaceType::HWA_RenderTargetTexture;
[[deprecated("Use SurfaceType enum")]]
constexpr int ST_HWA_ROUTEXTURE = (int)SurfaceType::HWA_UnmanagedTexture;
[[deprecated("Use SurfaceType enum")]]
constexpr int ST_HWA_ROMTEXTURE = (int)SurfaceType::HWA_ManagedTexture;
[[deprecated("Use SurfaceType enum")]]
constexpr int ST_MAX = (int)SurfaceType::Max;

[[deprecated("Use SurfaceDriver enum")]]
constexpr int SD_DIB = (int)SurfaceDriver::DIB;
[[deprecated("Use SurfaceDriver enum")]]
constexpr int SD_WING = (int)SurfaceDriver::WinGDI;
[[deprecated("Use SurfaceDriver enum")]]
constexpr int SD_DDRAW = (int)SurfaceDriver::DirectDraw;
[[deprecated("Use SurfaceDriver enum")]]
constexpr int SD_BITMAP = (int)SurfaceDriver::Bitmap;
[[deprecated("Use SurfaceDriver enum")]]
constexpr int SD_3DFX = (int)SurfaceDriver::_3DFX;
[[deprecated("Use SurfaceDriver enum")]]
constexpr int SD_D3D9 = (int)SurfaceDriver::Direct3D9;
[[deprecated("Use SurfaceDriver enum")]]
constexpr int SD_D3D8 = (int)SurfaceDriver::Direct3D8;
[[deprecated("Use SurfaceDriver enum")]]
constexpr int SD_D3D11 = (int)SurfaceDriver::Direct3D11;
[[deprecated("Use SurfaceDriver enum")]]
constexpr int SD_MAX = (int)SurfaceDriver::Max;

constexpr int LI_NONE = (int)LoadImageFlags::None;
constexpr int LI_REMAP = (int)LoadImageFlags::Remap;
constexpr int LI_CHANGESURFACEDEPTH = (int)LoadImageFlags::ChangeSurfaceDepth;
constexpr int LI_DONOTNORMALIZEPALETTE = (int)LoadImageFlags::DoNotNormalizePalette;

#define ALPHATOSEMITRANSP(a) ((a==0) ? 128:(255-a)/2)
#define SEMITRANSPTOALPHA(s) ((s==128) ? 0:(255-s*2))

#define COLORREFATORGBA(c,a) ((c & 0x00FFFFFF) | (a << 24))

constexpr int SI_NONE = (int)SaveImageFlags::None;
constexpr int SI_ONLYHEADER = (int)SaveImageFlags::OnlyHeader;
constexpr int SI_SAVEALPHA = (int)SaveImageFlags::SaveAlpha;

constexpr unsigned int BLTF_ANTIA = (unsigned int)BlitOptions::AntiAliasing;
constexpr unsigned int BLTF_COPYALPHA = (unsigned int)BlitOptions::CopyAlpha;
#ifdef HWABETA
	constexpr unsigned int BLTF_SAFESRC = (unsigned int)BlitOptions::SafeSource;
	constexpr unsigned int BLTF_TILE = (unsigned int)BlitOptions::Tile;
#endif


// TODO: YQ: What is STRF? Stretch Function?
[[deprecated]]
constexpr unsigned int STRF_RESAMPLE		= (unsigned int)StretchFlags::Resample;
[[deprecated]]
constexpr unsigned int STRF_RESAMPLE_TRANSP	= (unsigned int)StretchFlags::ResampleTransparentColor;
[[deprecated]]
constexpr unsigned int STRF_COPYALPHA		= (unsigned int)StretchFlags::CopyAlpha;
#ifdef HWABETA
[[deprecated]]
constexpr unsigned int STRF_SAFESRC			= (unsigned int)StretchFlags::SafeSource;
[[deprecated]]
constexpr unsigned int STRF_TILE			= (unsigned int)StretchFlags::Tile;
#endif

[[deprecated]]
constexpr int SPA_NONE = (int)SetPaletteAction::None;
[[deprecated]]
constexpr int SPA_REMAPSURFACE = (int)SetPaletteAction::RemapSurface;	// remap current surface pixels to new palette
[[deprecated]]
constexpr int SPA_MAX = (int)SetPaletteAction::Max;

[[deprecated]]
typedef CollisionMask sMask;
[[deprecated]]
typedef CollisionMask * LPSMASK;

[[deprecated]]
constexpr int SCMF_FULL = (std::uint16_t)CreateCollisionMaskFlags::Full;
[[deprecated]]
constexpr int SCMF_PLATFORM = (std::uint16_t)CreateCollisionMaskFlags::Platform;
