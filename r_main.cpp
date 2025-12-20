#include "debug.h"
#include "filesystem.h"
#include "render.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// 2D vertex type
struct VERTEX_2D
{
	D3DVECTOR		position;	// The position
	D3DCOLOR		color;		// The color
	float			u, v;		// The texture coordinates
};

// Our custom FVF, which describes our 2d vertex structure
#define D3DFVF_VERTEX_2D (D3DFVF_XYZ|D3DFVF_TEX1|D3DFVF_DIFFUSE)

CRender* g_pRender                = 0;

// Texture Loader (replace of D3DX)
HRESULT XCreateTextureFromFile(LPDIRECT3DDEVICE9 pDevice,
	LPCTSTR pSrcFile,
	LPDIRECT3DTEXTURE9* ppTexture)
{
	HRESULT hr = S_OK;
	int iWidth;
	int iHeight;
	int iChannels;
	stbi_uc *pImage;
	stbi_uc *pImageData;
	UCHAR *pLockedImage;
	IDirect3DTexture9 *pTexture;
	D3DLOCKED_RECT lockedRect;

	// Load an image data.
	pImage = stbi_load( pSrcFile, &iWidth, &iHeight, &iChannels, STBI_rgb_alpha );
	if ( !pImage )
	{
		hr = HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );
		return hr;
	}

	pImageData = pImage;

	// Create a D3D Texture
	hr = pDevice->CreateTexture( iWidth, iHeight, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pTexture, NULL );
	if ( FAILED( hr ) )
	{
		stbi_image_free(pImage);
		return hr;
	}
	
	// Map a Texture Object
	hr = pTexture->LockRect( 0, &lockedRect, NULL, 0 );
	if ( FAILED( hr ) )
	{
		stbi_image_free(pImage);
		return hr;
	}

	// Fill Texture memory
	for ( int i = 0; i < iHeight; i++ )
	{
		pLockedImage = ( UCHAR *)lockedRect.pBits + i * lockedRect.Pitch;
		for ( int j = 0; j < iWidth; j++ )
		{
			*pLockedImage++ = pImageData[ 2 ];
			*pLockedImage++ = pImageData[ 1 ];
			*pLockedImage++ = pImageData[ 0 ];
			*pLockedImage++ = pImageData[ 3 ];
			pImageData += 4;
		}
	}
	
	// Unlock
	hr = pTexture->UnlockRect( 0 );
	if ( FAILED( hr ) )
		return hr;

	stbi_image_free( pImage );

	if ( ppTexture )
		 *ppTexture = pTexture;

	return hr;
}

//
// Purpose: Getting the flag for initializing the reference Direct3D device.
//			Check the command line parameter or convar.
// 
bool IsReferenceDevRequired()
{
	bool isRequired = !!strstr( GetCommandLineA(), "-refdevice" );
	return isRequired;
}

CRender::CRender()
{
	m_dwNumTextures		= 0;
	m_dwNumContainers	= 0;
	m_dwNumShaders		= 0;
	m_dwNumFonts		= 0;
	m_dwNumModels		= 0;

	m_bTrace			= false;

	memset( m_pTextures, 0, sizeof( m_pTextures ) );
	memset( m_pContainers, 0, sizeof( m_pContainers ) );
	memset( m_Shaders, 0, sizeof( m_Shaders ) );
	memset( m_pFonts, 0, sizeof( m_pFonts ) );
	memset( m_pModels, 0, sizeof( m_pModels ) );

	memset( &m_Viewport, 0, sizeof( m_Viewport ) );

	m_pd3d				= 0;
	m_pd3dDevice		= 0;

	m_matProjection.SetIdentity();
}

CRender::~CRender()
{
}

void CRender::Init( HWND hWnd )
{
	DPrintf( "starting render ...\n" );

	// Initialize Direct3D
	m_pd3d = Direct3DCreate9( D3D_SDK_VERSION );
	assert( m_pd3d );

	int numAdapters = (int)m_pd3d->GetAdapterCount();
	DPrintf( "num available adapters: %d\n", numAdapters );

	UINT adapterToUse = D3DADAPTER_DEFAULT;
	D3DDEVTYPE deviceType = D3DDEVTYPE_HAL;

	for ( int i = 0; i < numAdapters; i++ )
	{
		D3DADAPTER_IDENTIFIER9 adapterIdentifier;
		m_pd3d->GetAdapterIdentifier( i, 0, &adapterIdentifier );

		// print adapter info
		DPrintf( "adapter %d: %s\n", i, adapterIdentifier.Description );
	
		// Look for 'NVIDIA PerfHUD' adapter
		// If it is present, override default settings
		if ( strstr( adapterIdentifier.Description, "PerfHUD") != 0 )
		{
			adapterToUse = i;
			deviceType = D3DDEVTYPE_REF;

			DPrintf( "Found NVIDIA PerfHUD adapter\n" );

			break;
		}
	}

	D3DCAPS9 caps;
	m_pd3d->GetDeviceCaps( 0, D3DDEVTYPE_HAL, &caps );

	// If device doesn't support HW T&L (Hardware Transform and Lighting) or doesn't  
    // support 1.1 vertex shaders in HW then switch to SWVP.

	bool bTNLSupported = false;

    if( (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
         caps.VertexShaderVersion < D3DVS_VERSION(1,1) )
	{
		bTNLSupported = false;

		DPrintf( "T&L is not supported\n" );
	}
	else
	{
		bTNLSupported = true;

		DPrintf( "T&L is supported\n" );

		int vsVersionMajor = (((caps.VertexShaderVersion)>>8)&0xFF);
		int vsVersionMinor = (((caps.VertexShaderVersion)>>0)&0xFF);

		DPrintf( "Vertex shader version %d.%d\n", vsVersionMajor,
			vsVersionMinor );

		int psVersionMajor = (((caps.PixelShaderVersion)>>8)&0xFF);
		int psVersionMinor = (((caps.PixelShaderVersion)>>0)&0xFF);

		DPrintf( "Pixel shader version %d.%d\n", psVersionMajor,
			psVersionMinor );
	}

	// Create Direct3D device

	// get client rect of window
	RECT clientRect;
	GetClientRect( hWnd, &clientRect );

	// Fill present params structure
	D3DPRESENT_PARAMETERS d3dpp;
	memset( &d3dpp, 0, sizeof( d3dpp ) );
	d3dpp.Windowed               = TRUE;
	d3dpp.BackBufferCount        = 1;
	d3dpp.MultiSampleType        = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality     = 0;
	d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.hDeviceWindow          = hWnd;
	d3dpp.Flags                  = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	bool softwareProcessing = false;
	DPrintf( "Software Processing: %s\n", softwareProcessing ? "true" : "false" );

	if ( IsReferenceDevRequired() )
	{
		DPrintf( "... reference device required\n" );
		deviceType = D3DDEVTYPE_REF;
	}

	DWORD dwDeviceFlags = 0;
	if ( bTNLSupported  && !IsReferenceDevRequired() )
		 dwDeviceFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		 dwDeviceFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	HRESULT hr = m_pd3d->CreateDevice( adapterToUse, deviceType, hWnd,
									   dwDeviceFlags,
									   &d3dpp, &m_pd3dDevice );

	if ( FAILED( hr ) )
	{
		DError( "Failed to create Direct3D 9 device. HRESULT: 0x%x", hr );
	}

	RECT rect;
	GetClientRect( hWnd, &rect );

	D3DVIEWPORT9 vp;
	vp.X      = 0;
	vp.Y      = 0;
	vp.Width  = rect.right;
	vp.Height = rect.bottom;
	vp.MinZ   = 0.0f;
	vp.MaxZ   = 1.0f;
	m_pd3dDevice->SetViewport( &vp );
	
	// Turn off culling
	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	// Turn off D3D lighting
	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

	// Turn on the zbuffer
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

	// Turn off the zbuffer
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );

	// Creating 2D rendering system
	Init2DDraw();
}

void CRender::Init2DDraw()
{
	// Initialize dynamic vertex buffer for quad primitive
	HRESULT hr = m_pd3dDevice->CreateVertexBuffer( 6 * sizeof(VERTEX_2D),
		0 /* Usage */, D3DFVF_VERTEX_2D, D3DPOOL_DEFAULT, 
		&m_pQuadVB, NULL );

	if ( FAILED( hr ) )
	{
		DError( "Failed to create 2d vertex buffer. HRESULT: 0x%x", hr );
	}

	VERTEX_2D* pVertices;
	if( FAILED( m_pQuadVB->Lock( 0, 0, (void**)&pVertices, 0 ) ) )
		DError( "Failed to lock vertex buffer." );

	m_pQuadVB->Unlock();

	// Initialize index buffer for quad primitive

	DWORD dwIBSize = 6 * sizeof(ushort);

	hr = m_pd3dDevice->CreateIndexBuffer( dwIBSize, D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pQuadIB, NULL );

	if ( FAILED( hr ) )
	{
		DError( "Failed to create 2d index buffer. HRESULT: 0x%x", hr );
	}

	// Fill data
	ushort* pData;
	hr = m_pQuadIB->Lock( 0, 0, (void**)&pData, 0 );
	
	// first triangle
	pData[ 0 ] = 0; pData[ 1 ] = 1; pData[ 2 ] = 2;

	// second triangle
	pData[ 3 ] = 3; pData[ 4 ] = 2; pData[ 5 ] = 1;

	hr = m_pQuadIB->Unlock();
}

void CRender::Shutdown()
{
	int i;


	// Clean-up a containers
	for ( i = 0; i < m_dwNumContainers; i++ )
	{
		m_pContainers[ i ]->Shutdown();
	}

	memset( m_pContainers, 0, sizeof( m_pContainers ) );

	// Clean-up a models
	if ( m_dwNumModels )
	{
		DPrintf( "CRender::Shutdown: unregistering %d models\n", m_dwNumModels );

		for ( i = 0; i < m_dwNumModels; i++ )
		{
			if ( m_pModels[ i ] )
			{
				delete m_pModels[ i ]->m_pModel;
				delete m_pModels[ i ];
				m_pModels[ i ] = NULL;
			}
		}

		memset( m_pModels, 0, sizeof( m_pModels ) );
		m_dwNumModels = 0;
	}

	// Clean-up a fonts
	if ( m_dwNumFonts )
	{
		DPrintf( "CRender::Shutdown: unregistering %d fonts\n", m_dwNumFonts );

		for ( i = 0; i < m_dwNumFonts; i++ )
		{
			if ( m_pFonts[ i ] )
			{
				delete m_pFonts[ i ];
				m_pFonts[ i ] = NULL;
			}
		}

		memset( m_pFonts, 0, sizeof( m_pFonts ) );
		m_dwNumFonts = 0;
	}

	// Clean-up a shaders
	if ( m_dwNumShaders )
	{
		DPrintf( "CRender::Shutdown: unregistering %d shaders\n", m_dwNumShaders );

		for ( i = 0; i < m_dwNumShaders; i++ )
		{
			if ( m_Shaders[ i ].m_pVertexShader )
			{
				m_Shaders[ i ].m_pVertexShader->Release();
				m_Shaders[ i ].m_pVertexShader = NULL;
			}
			else if ( m_Shaders[ i ].m_pPixelShader )
			{
				m_Shaders[ i ].m_pPixelShader->Release();
				m_Shaders[ i ].m_pPixelShader = NULL;
			}
		}
	
		memset( m_Shaders, 0, sizeof( m_Shaders ) );
		m_dwNumShaders = 0;
	}

	// Clean-up a textures
	if ( m_dwNumTextures )
	{
		DPrintf( "CRender::Shutdown: unregistering %d cached textures\n", m_dwNumTextures );

		for ( i = 0; i < m_dwNumTextures; i++ )
		{
			if ( m_pTextures[ i ] )
			{
				assert( m_pTextures[ i ]->m_pTexture );

				m_pTextures[ i ]->m_pTexture->Release();
				m_pTextures[ i ]->m_pTexture = NULL;

				delete m_pTextures[ i ];
				m_pTextures[ i ] = NULL;
			}
		}

		memset( m_pTextures, 0, sizeof( m_pTextures ) );
		m_dwNumTextures = 0;
	}

	if ( m_pQuadIB != NULL )
		 m_pQuadIB->Release();

	if ( m_pQuadVB != NULL )
		 m_pQuadVB->Release();

	if( m_pd3dDevice != NULL )
		m_pd3dDevice->Release();

	if( m_pd3d != NULL )
		m_pd3d->Release();

	// Reset pointers
	m_pQuadIB = NULL;
	m_pQuadVB = NULL;
	m_pd3dDevice = NULL;
	m_pd3d = NULL;
}

void CRender::BeginScene()
{
	HRESULT hr = m_pd3dDevice->TestCooperativeLevel();
	if ( hr == D3DERR_DEVICELOST )
	{
		DError( "CRender::BeginScene: m_pd3dDevice->TestCooperativeLevel() D3DERR_DEVICELOST" );
	}

	// Clear the backbuffer and the zbuffer
	m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 
                         D3DCOLOR_XRGB(0,0,/*255*/0), 1.0f, 0 );

	m_pd3dDevice->BeginScene();
}

void CRender::EndScene()
{
	// End the scene
	m_pd3dDevice->EndScene();

	// Present the backbuffer contents to the display
	m_pd3dDevice->Present( NULL, NULL, NULL, NULL );

	// TODO: SLOW
	//m_pd3dDevice->SetTexture( 0, NULL );
}

void CRender::Draw2DRect( HTEXTURE hTexture, float fX, float fY, float fWidth, float fHeight, unsigned int uiColor )
{
	Draw2DRectUV( hTexture, fX, fY, fWidth, fHeight, 0.0f, 1.0f, 0.0f, 1.0f, uiColor );
}

void CRender::Draw2DRectUV( HTEXTURE hTexture, float fX, float fY, float fWidth, float fHeight, float fU0, float fU1, float fV0, float fV1, unsigned int uiColor )
{
	// Lock vertex buffer
	VERTEX_2D* pVertices;
	if ( FAILED( m_pQuadVB->Lock(0, 0, (void**)&pVertices, 0 ) ) )
		DError( "Failed to lock vertex buffer." );

	pVertices[0].position.x = fX;
	pVertices[0].position.y = fY + fHeight;
	pVertices[0].position.z = 0.0f;
	pVertices[0].u = fU0;
	pVertices[0].v = fV0;
	pVertices[0].color = uiColor;

	pVertices[1].position.x = fX;
	pVertices[1].position.y = fY;
	pVertices[1].position.z = 0.0f;
	pVertices[1].u = fU0;
	pVertices[1].v = fV1;
	pVertices[1].color = uiColor;

	pVertices[2].position.x = fX + fWidth;
	pVertices[2].position.y = fY + fHeight;
	pVertices[2].position.z = 0.0f;
	pVertices[2].u = fU1;
	pVertices[2].v = fV0;
	pVertices[2].color = uiColor;

	pVertices[3].position.x = fX + fWidth;
	pVertices[3].position.y = fY;
	pVertices[3].position.z = 0.0f;
	pVertices[3].u = fU1;
	pVertices[3].v = fV1;
	pVertices[3].color = uiColor;

	// Unlock
	m_pQuadVB->Unlock();

	// Activate user sampler
	SetTexture( hTexture, 0 );

	// Set the 2D projection matrix
	Push2DMatrix( &m_Viewport );

	// Setup vertex buffer
	m_pd3dDevice->SetStreamSource( 0, m_pQuadVB, 0, sizeof( VERTEX_2D ) );

	// Setup index buffer
	m_pd3dDevice->SetIndices( m_pQuadIB );

	// Setup FVF (Flexible Vertex Format)
	m_pd3dDevice->SetFVF( D3DFVF_VERTEX_2D );

	// Draw this!
	m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2 );
}

void CRender::Draw2DLine( float fX1, float fY1, float fX2, float fY2, unsigned int uiColor )
{
	// Lock vertex buffer
	VERTEX_2D* pVertices;
	if ( FAILED( m_pQuadVB->Lock(0, 0, (void**)&pVertices, 0 ) ) )
		DError( "Failed to lock vertex buffer." );

	pVertices[0].position.x = fX1;
	pVertices[0].position.y = fY1;
	pVertices[0].position.z = 0.0f;
	pVertices[0].u = 0.0f;
	pVertices[0].v = 0.0f;
	pVertices[0].color = uiColor;

	pVertices[1].position.x = fX2;
	pVertices[1].position.y = fY2;
	pVertices[1].position.z = 0.0f;
	pVertices[1].u = 0.0f;
	pVertices[1].v = 0.0f;
	pVertices[1].color = uiColor;

	// Unlock
	m_pQuadVB->Unlock();

	// Disable texturing
	m_pd3dDevice->SetTexture( 0, NULL );
	m_pd3dDevice->SetTexture( 1, NULL );
	m_pd3dDevice->SetTexture( 2, NULL );
	m_pd3dDevice->SetTexture( 3, NULL );
	m_pd3dDevice->SetTexture( 4, NULL );

	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );

	// Set the 2D projection matrix
	Push2DMatrix( &m_Viewport );

	// Setup vertex buffer
	m_pd3dDevice->SetStreamSource( 0, m_pQuadVB, 0, sizeof( VERTEX_2D ) );

	// Setup index buffer
	m_pd3dDevice->SetIndices( NULL );

	// Setup FVF (Flexible Vertex Format)
	m_pd3dDevice->SetFVF( D3DFVF_VERTEX_2D );

	// Draw this!
	m_pd3dDevice->DrawPrimitive( D3DPT_LINELIST, 0, 1 );
}

HTEXTURE CRender::RegisterTexture( const char* filename )
{
	IDirect3DTexture9* pTexture = NULL;
	int index = 0;

	for ( int i = 0; i < MAX_TEXTURES; i++ )
	{
		if ( m_pTextures[ i ] && strcmp( m_pTextures[ i ]->m_Filename, filename ) == 0 )
		{
			return i;
		}
	}

	// find free slot
	for ( int j = 0; j < MAX_TEXTURES; j++ )
	{
		if ( !m_pTextures[ j ] )
		{
			index = j;
			break;
		}
	}

	// Load texture 
	HRESULT hr = XCreateTextureFromFile( m_pd3dDevice, filename,
		&pTexture );

	if ( FAILED( hr ) )
	{
		DPrintf( "Texture %s loading error: 0x%x\n", filename, hr );
		return INVALID_RENDER_HANDLE;
	}

	// Allocate texture
	m_pTextures[ index ] = NEW CTexture();
	m_pTextures[ index ]->m_pTexture = pTexture;
	strcpy( m_pTextures[ index ]->m_Filename, filename );

	if ( m_bTrace )
		DPrintf( "CRender::RegisterTexture: registering \"%s\"...\n", filename );

	int dwWidth = 0, dwHeight = 0;
	GetTextureSize( index, 0, &dwWidth, &dwHeight );

	m_dwNumTextures++;

	return index;
}

void CRender::UnregisterTexture( HTEXTURE hTexture )
{
	if ( hTexture != INVALID_RENDER_HANDLE )
	{
		assert( m_pTextures[ hTexture ] );
		assert( m_pTextures[ hTexture ]->m_pTexture );

		m_pTextures[ hTexture ]->m_pTexture->Release();
		
		delete m_pTextures[ hTexture ];
		m_pTextures[ hTexture ] = 0;

		m_dwNumTextures--;
	}
}

void CRender::SetTexture( HTEXTURE hTexture, int stage )
{
	if ( hTexture != INVALID_RENDER_HANDLE )
	{
		assert( m_pTextures[ hTexture ] );
		assert( m_pTextures[ hTexture ]->m_pTexture );
		
		m_pd3dDevice->SetTexture( stage, m_pTextures[ hTexture ]->m_pTexture );
	}
	else
	{
		m_pd3dDevice->SetTexture( stage, NULL );
	}
}

void CRender::GetTextureSize( HTEXTURE hTexture, int iLevel, int* pWidth, int* pHeight )
{
	assert( pWidth );
	assert( pHeight );

	if ( hTexture != INVALID_RENDER_HANDLE )
	{
		assert( m_pTextures[ hTexture ] );
		assert( m_pTextures[ hTexture ]->m_pTexture );
		
		D3DSURFACE_DESC desc;
		memset( &desc, 0, sizeof( desc ) );

		m_pTextures[ hTexture ]->m_pTexture->GetLevelDesc( iLevel, &desc );

		*pWidth = desc.Width;
		*pHeight = desc.Height;
	}
}

IDirect3DDevice9* CRender::Dev()
{
	return m_pd3dDevice;
}

void CRender::SetViewport( SVIEWPORT* pViewport )
{
	if ( !pViewport )
		return;

	m_Viewport = *pViewport;

	// Setup dx viewport
	D3DVIEWPORT9 vp;
	memset( &vp, 0, sizeof( vp ) );
	vp.X = 0;
	vp.Y = 0;
	vp.Width = pViewport->Width;
	vp.Height = pViewport->Height;
	m_pd3dDevice->SetViewport( &vp );
}

void CRender::Push2DMatrix( SVIEWPORT* pViewport )
{
	m_matProjection.OrthoOffCenterLH( 0.0f, (float)pViewport->Width, 0.0f, (float)pViewport->Height, -1.0f, 1.0f );
	m_pd3dDevice->SetTransform( D3DTS_PROJECTION, ( D3DMATRIX *)&m_matProjection);
}