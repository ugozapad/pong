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
