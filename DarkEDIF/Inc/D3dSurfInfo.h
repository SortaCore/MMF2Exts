
#ifndef D3dSurfInfo_h
#define D3dSurfInfo_h

typedef struct D3DSURFINFO {

	DWORD					m_lSize;
	int						m_nD3DVersion;		// 8 or 9
	LPVOID					m_pD3D;
	LPVOID					m_pD3DDevice;
	LPVOID					m_pD3DTexture;
	int						m_dwPixelShaderVersion;
	int						m_dwVertexShaderVersion;		// not used
	int						m_dwMaxTextureWidth;
	int						m_dwMaxTextureHeight;

} D3DSURFINFO;

#endif // D3dSurfInfo_h
