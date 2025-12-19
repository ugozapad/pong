#include <assert.h>
#include <stdio.h>
#include "debug.h"
#include "FileSystem.h"
#include "render.h"

HSHADER CRender::RegisterShader( const char* pFilename )
{
	HRESULT hr;

	// Find loaded shader
	for ( int i = 0; i < m_dwNumShaders; i++ )
	{
		if ( strcmp( m_Shaders[ i ].m_Filename, pFilename ) == 0 )
			return i;
	}

	HFILE hFile = g_pFS->OpenFile( pFilename, "rb" );
	if ( hFile == -1 )
		return INVALID_SHADER_HANDLE;

	size_t uiFileLength = g_pFS->GetFileLength( hFile );

	DWORD* pData = (DWORD*)malloc( uiFileLength );
	g_pFS->ReadFile( hFile, pData, uiFileLength );
	g_pFS->CloseFile( hFile );

	HSHADER index = m_dwNumShaders++;

	const char* pFileExt = &pFilename[ strlen( pFilename ) - 3 ];
	if ( strcmp( pFileExt, "vso" ) == 0 )
	{
		hr = m_pd3dDevice->CreateVertexShader( pData, &m_Shaders[ index ].m_pVertexShader );
	}
	else if ( strcmp( pFileExt, "pso" ) == 0 )
	{
		m_Shaders[ index ].m_bIsPixelShader = true;

		hr = m_pd3dDevice->CreatePixelShader( pData, &m_Shaders[ index ].m_pPixelShader );
	}

	free( pData );

	strcpy( m_Shaders[ index ].m_Filename, pFilename );

	if ( FAILED( hr ) )
	{
		DError( "CRender::RegisterShader: Failed to create shader '%s'. Error: 0x%x", 
			pFilename, hr );
	}

	return index;
}

void CRender::UnregisterShader( HSHADER hShader )
{
	if ( hShader != INVALID_SHADER_HANDLE )
	{
		if ( m_Shaders[ hShader ].m_bIsPixelShader )
		{
			assert( m_Shaders[ hShader ].m_pPixelShader );

			m_Shaders[ hShader ].m_pPixelShader->Release();
			m_Shaders[ hShader ].m_pPixelShader = NULL;
		}
		else
		{
			assert( m_Shaders[ hShader ].m_pVertexShader );

			m_Shaders[ hShader ].m_pVertexShader->Release();
			m_Shaders[ hShader ].m_pVertexShader = NULL;
		}
		
		m_dwNumShaders--;
	}
}

void CRender::SetShader( HSHADER hShader )
{
	if ( hShader != INVALID_SHADER_HANDLE )
	{
		if ( m_Shaders[ hShader ].m_bIsPixelShader )
			 m_pd3dDevice->SetPixelShader( m_Shaders[ hShader ].m_pPixelShader );
		else
			 m_pd3dDevice->SetVertexShader( m_Shaders[ hShader ].m_pVertexShader );
	}
}

void CRender::ResetShader()
{
	m_pd3dDevice->SetVertexShader( NULL );
	m_pd3dDevice->SetPixelShader( NULL );
}

void CRender::SetVertexShaderConstantF( uint StartRegister, const float* pConstantData, uint Vector4fCount )
{
	m_pd3dDevice->SetVertexShaderConstantF( StartRegister, pConstantData, Vector4fCount );
}