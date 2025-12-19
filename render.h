#ifndef RENDER_H
#define RENDER_H

#include <d3d9.h>

#include "common.h"
#include "Vector.h"
#include "Matrix.h"

#define INVALID_RENDER_HANDLE -1 // Shared between texture, shader and model

#define MAX_TEXTURES 256
#define MAX_CONTAINERS 16
#define MAX_SHADERS 16
#define MAX_FONTS 16
#define MAX_MODELS 64

typedef int HTEXTURE;
typedef int HSHADER;
typedef int HCFONT;

// Texture class
class CTexture
{
public:
	char m_Filename[64];
	IDirect3DTexture9* m_pTexture;
};

// Shader class
class CShader
{
public:
	char m_Filename[64];

	union
	{
		IDirect3DVertexShader9* m_pVertexShader;
		IDirect3DPixelShader9* m_pPixelShader;
	};

	bool m_bIsPixelShader;
};

// Model interface
class IModel
{
public:
	~IModel() {}

	virtual void Load( const char* pFilename ) = 0;
	virtual void Render( const CMatrix& matWorld ) = 0;
};

// Model container
class CModelContainer
{
public:
	char m_Extension[64];
	char m_Description[64];

public:
	CModelContainer();
	virtual ~CModelContainer();

	virtual void Init();
	virtual void Shutdown();

	virtual IModel* CreateModel( const char* pFilename );
	virtual void ReleaseModel( IModel* pModel );
};

// Model info class
class CModelInfo
{
public:
	char m_Filename[64];
	IModel* m_pModel;
};

// Font class
class CFont
{
public:
	char m_Filename[64];
	HTEXTURE m_hFontTexture;
	int m_dwWidth, m_dwHeight;

public:
	void Load( const char* pFilename );

};

// Render class
class CRender
{
public:
	CRender();
	~CRender();

	void Init( HWND hWnd );
	void Shutdown();

	void Init2DDraw();

	// Rendering

	void BeginScene();
	void EndScene();

	void Draw2DRect( HTEXTURE hTexture, float fX, float fY, float fWidth, float fHeight, unsigned int uiColor );
	void Draw2DRectUV( HTEXTURE hTexture, float fX, float fY, float fWidth, float fHeight, float fU0, float fU1, float fV0, float fV1, unsigned int uiColor );
	void Draw2DText( HCFONT hFont, const char* pString, float fX, float fY, unsigned int uiColor );
	void Draw2DLine( float fX1, float fY1, float fX2, float fY2, unsigned int uiColor );

	// Texture
	HTEXTURE RegisterTexture( const char* filename );
	void UnregisterTexture( HTEXTURE hTexture );
	void SetTexture( HTEXTURE hTexture, int stage );
	void GetTextureSize( HTEXTURE hTexture, int iLevel, int* pWidth, int* pHeight );

	// Model
	void RegisterModelContainer( CModelContainer* pContainer );
	IModel* RegisterModel( const char *pFilename, const char* pExtension = NULL );

	// Helper function for don't bothering with extensions
	IModel* LoadModel( const char* pFilename );

	// Shader
	HSHADER RegisterShader( const char* pFilename );
	void UnregisterShader( HSHADER hShader );
	void SetShader( HSHADER hShader );
	void ResetShader();
	void SetVertexShaderConstantF( uint StartRegister, const float* pConstantData, uint Vector4fCount );

	// Font
	HCFONT RegisterFont( const char* pFilename );
	void UnregisterFont( HCFONT hFont );

	// Direct3D
	IDirect3DDevice9* Dev();

	// Viewport
	void SetViewport( SVIEWPORT* pViewport );

private:
	void Push2DMatrix( SVIEWPORT* pViewport );

private:
	CTexture*			m_pTextures[MAX_TEXTURES];
	CModelContainer*	m_pContainers[MAX_CONTAINERS];
	CShader				m_Shaders[MAX_SHADERS];
	CFont*				m_pFonts[MAX_FONTS];
	CModelInfo*			m_pModels[MAX_MODELS];

	// Matrices
	CMatrix		m_matProjection;

	// Direct3D
	IDirect3D9*			m_pd3d;
	IDirect3DDevice9*	m_pd3dDevice;

	IDirect3DVertexBuffer9*			m_pQuadVB;
	IDirect3DIndexBuffer9*			m_pQuadIB;

	SVIEWPORT			m_Viewport;
	HWND				m_hWnd;
	DWORD				m_dwNumTextures;
	DWORD				m_dwNumContainers;
	DWORD				m_dwNumShaders;
	DWORD				m_dwNumFonts;
	DWORD				m_dwNumModels;

	bool				m_bTrace;
};

extern CRender* g_pRender;

class CVertexBuffer
{
private:
	D3DVERTEXBUFFER_DESC m_Desc;
	IDirect3DVertexBuffer9*	m_pVB;

public:
	CVertexBuffer();
	~CVertexBuffer();

	void Create( const D3DVERTEXBUFFER_DESC& desc );
	void Destroy();

	void* Lock( int sizeToLock );
	void Unlock();

	IDirect3DVertexBuffer9* GetBuffer();
};

#endif