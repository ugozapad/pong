#include "debug.h"
#include "render.h"

void CFont::Load( const char* pFilename )
{
	// Load the texture
	m_hFontTexture = g_pRender->RegisterTexture( pFilename );

	// Copy filename
	strcpy( m_Filename, pFilename );

	// Get the dimension of texture
	g_pRender->GetTextureSize( m_hFontTexture, 0, &m_dwWidth, &m_dwHeight );
}

HCFONT CRender::RegisterFont( const char* pFilename )
{
	int index = 0;

	for ( int i = 0; i < MAX_FONTS; i++ )
	{
		if ( m_pFonts[ i ] && strcmp( m_pFonts[ i ]->m_Filename, pFilename ) == 0 )
		{
			return i;
		}
	}

	// find free slot
	for ( int j = 0; j < MAX_FONTS; j++ )
	{
		if ( !m_pFonts[ j ] )
		{
			index = j;
			break;
		}
	}

	// Allocate and load font 
	m_pFonts[ index ] = NEW CFont();
	m_pFonts[ index ]->Load( pFilename );

	if ( m_bTrace )
		DPrintf( "CRender::RegisterFont: registering \"%s\"...\n", pFilename );

	m_dwNumFonts++;

	return index;
}

void CRender::UnregisterFont( HCFONT hFont )
{
	if ( hFont != INVALID_RENDER_HANDLE )
	{
		delete m_pFonts[ hFont ];
		m_pFonts[ hFont ] = NULL;
		
		m_dwNumFonts--;
	}
}

void CRender::Draw2DText( HCFONT hFont, const char* pString, float fX, float fY, unsigned int uiColor )
{
	CFont* pFont = m_pFonts[ hFont ];

	// Enable additive blending
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	// DX9 half-pixel offset
	float fTexelU = 1.0f / pFont->m_dwWidth;
	float fTexelV = 1.0f / pFont->m_dwHeight;

	size_t uiLength = strlen( pString );

	for ( int i = 0; i < uiLength; i++ )
	{
		// First character in font texture is 32 ASCII code
		int c = pString[ i ] - 32;

		int iRow = c >> 4;
		int iCol = c & 15;

		// Font grid is fixed by 32 pixels per character
		float fU0 = (iCol * 32.0f + 0.5f) * fTexelU;
		float fU1 = ((iCol + 1) * 32.0f - 0.5f) * fTexelU;
		float fV0 = (iRow * 32.0f + 0.5f) * fTexelV;
		float fV1 = ((iRow + 1) * 32.0f - 0.5f) * fTexelV;

		Draw2DRectUV( pFont->m_hFontTexture, fX, fY, 32.0f, 32.0f,
			fU0, fU1, fV0, fV1, uiColor );

		// Max width of character is 12 pixels
		fX += 8.5f;
	}

	// Reset state
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
}
