// FileSystem.cpp: implementation of the CFileSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "debug.h"
#include "FileSystem.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <io.h>

//////////////////////////////////////////////////////////////////////
// Global variables
//////////////////////////////////////////////////////////////////////
CFileSystem* g_pFS = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileHandle::CFileHandle()
{
	m_szFilename[0] = 0;
	m_pFile = NULL;
}

CFileSystem::CFileSystem()
{
	m_iOpenedFiles = 0;

	memset( m_Files, 0, sizeof( m_Files ) );
}

CFileSystem::~CFileSystem()
{
}

HFILE CFileSystem::OpenFile( const char* pFilename, const char* pMode )
{
	int iHandle;

	// find already opened file
	for ( int i = 0; i < MAX_OPENED_FILES; i++ )
	{
		if ( m_Files[ i ].m_pFile && strcmp( m_Files[ i ].m_szFilename, pFilename ) == 0 )
			return i;
	}
	
	if ( !IsExist( pFilename ) && pMode[0] == 'r' )
	{
		DPrintf( "CFileSystem::OpenFile: '%s' is not exist\n", pFilename );
		return -1;
	}

	// Find free handle
	for ( int i = 0; i < MAX_OPENED_FILES; i++ )
	{
		if ( m_Files[ i ].m_pFile == NULL )
		{
			iHandle = i;
			break;
		}
	}

	m_Files[ iHandle ].m_pFile = fopen( pFilename, pMode );
	assert( m_Files[ iHandle ].m_pFile );

	strcpy( m_Files[ iHandle ].m_szFilename, pFilename );

	m_iOpenedFiles++;

	return iHandle;
}

void CFileSystem::CloseFile( HFILE hFile )
{
	assert( hFile <= MAX_OPENED_FILES );
	
	fclose( m_Files[ hFile ].m_pFile );
	m_Files[ hFile ].m_pFile = NULL;
	
	m_iOpenedFiles--;
}

CFileHandle* CFileSystem::GetFileHandle( HFILE hFile )
{
	assert( hFile <= MAX_OPENED_FILES );
	return &m_Files[ hFile ];
}

size_t CFileSystem::GetFileLength( HFILE hFile )
{
	assert( hFile <= MAX_OPENED_FILES );
	assert( m_Files[ hFile ].m_pFile );

	size_t size;

	fseek( m_Files[ hFile ].m_pFile, 0, SEEK_END );
	size = (size_t)ftell( m_Files[ hFile ].m_pFile );
	fseek( m_Files[ hFile ].m_pFile, 0, SEEK_SET );

	return size;
}

void CFileSystem::ReadFile( HFILE hFile, void* pData, size_t uiSize )
{
	assert( hFile <= MAX_OPENED_FILES );
	assert( m_Files[ hFile ].m_pFile );

	fread( pData, uiSize, 1, m_Files[ hFile ].m_pFile );
}

void CFileSystem::WriteFile( HFILE hFile, void* pData, size_t uiSize )
{
	assert( hFile <= MAX_OPENED_FILES );
	assert( m_Files[ hFile ].m_pFile );

	fwrite( pData, uiSize, 1, m_Files[ hFile ].m_pFile );
}

bool CFileSystem::IsExist( const char* pFilename )
{
#ifdef LOG_FS
	bool bExist = _access( pFilename, /*F_OK*/ 0 ) == 0;
	if ( !bExist )
		DPrintf( "FS: %s not found\n", pFilename );

	return bExist;
#else
	return _access( pFilename, /*F_OK*/ 0 ) == 0;
#endif // LOG_FS
}

void TrimString( char* pStr )
{
	int length = strlen( pStr );

	if ( pStr[ 0 ] == '\"' )
	{
		for ( int i = 0; i < length + 1; i++ )
		{
			pStr[ i ] = pStr[ i + 1 ];
		}
	}

	length = strlen( pStr );

	if ( pStr[ length - 1 ] == '\"' )
		 pStr[ length - 1 ] = 0;
}
