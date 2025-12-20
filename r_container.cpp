#include "debug.h"
#include "FileSystem.h"
#include "render.h"

// Model Container

CModelContainer::CModelContainer()
{
	m_Extension[0] = 0;
	m_Description[0] = 0;
}

CModelContainer::~CModelContainer()
{
}

void CModelContainer::Init()
{
}

void CModelContainer::Shutdown()
{
}

IModel* CModelContainer::CreateModel( const char* pFilename )
{
	return NULL;
}

void CModelContainer::ReleaseModel( IModel* pModel )
{
}

// Render

void CRender::RegisterModelContainer( CModelContainer* pContainer )
{
	assert( pContainer );
	m_pContainers[ m_dwNumContainers++ ] = pContainer;

	pContainer->Init();

	//DPrintf( "CRender::RegisterModelContainer: registered container \"%s\" \"%s\"\n",
	//	pContainer->m_Extension, pContainer->m_Description );
}

IModel* CRender::RegisterModel( const char *pFilename, const char* pExtension /*= NULL*/ )
{
	IModel* pModel = NULL;

	for ( int i = 0; i < m_dwNumModels; i++ )
	{
		if ( strcmp( pFilename, m_pModels[ i ]->m_Filename ) == 0 )
		{
			pModel = m_pModels[ i ]->m_pModel;
			return pModel;
		}
	}

	// Find model containter by extension.
	if ( pExtension )
	{
		for ( int j = 0; j < m_dwNumContainers; j++ )
		{
			if ( strcmp( m_pContainers[j]->m_Extension, pExtension ) == 0 )
			{
				pModel = m_pContainers[j]->CreateModel( pFilename );
				break;
			}
		}
	}

	if ( pModel == NULL )
		DError( "CRender::RegisterModel: Failed to register model '%s' with unknown extension %s\n", pFilename, pExtension );

	if ( m_bTrace )
		DPrintf( "CRender::RegisterModel: register model \"%s\"...\n", pFilename );

	// Allocate model info entry
	int iHandle = m_dwNumModels++;
	m_pModels[ iHandle ] = NEW CModelInfo;
	m_pModels[ iHandle ]->m_pModel = pModel;
	strcpy( m_pModels[ iHandle ]->m_Filename, pFilename );

	return pModel;
}

IModel* CRender::LoadModel( const char* pFilename )
{
	static char szBuffer[ 128 ];
	const char* pExtension = NULL;

	size_t uiLength = strlen( pFilename );
	pExtension = &pFilename[ uiLength - 3 ];

	// Try to make sure that passed filename are a texture
	if ( stricmp( pExtension, "png" ) == 0 || 
		 stricmp( pExtension, "jpg" ) == 0 ||
		 stricmp( pExtension, "tga" ) == 0 || 
		 stricmp( pExtension, "spr" ) == 0 )
		return RegisterModel( pFilename, "$spr" );

	// Load model config
	if ( stricmp( pExtension, "txt" ) == 0 )
	{
		HFILE hFile = g_pFS->OpenFile( pFilename, "r" );
		if ( hFile == INVALID_FILE_HANDLE )
		{
			DError( "CRender::LoadModel: Failed to open model text file: %s\n", pFilename );
			return NULL;
		}
		
		CFileHandle* pFileHandle = g_pFS->GetFileHandle( hFile );
		
		// Parse model config
		while ( true )
		{
			int result = fscanf( pFileHandle->m_pFile, "%s", szBuffer );
			if ( result == EOF )
				break;

			if ( stricmp( szBuffer, "$type" ) == 0 )
			{
				// Load the extension.
				fscanf( pFileHandle->m_pFile, "%s", szBuffer );
				TrimString( szBuffer );
				
				g_pFS->CloseFile( hFile );

				return RegisterModel( pFilename, szBuffer );
			}
			else
			{
				// Probably a commentary, skip it
				fgets( szBuffer, 128, pFileHandle->m_pFile );
			}
		}
	}

	return NULL;
}