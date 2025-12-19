// FileSystem.h: interface for the CFileSystem class.
//
//////////////////////////////////////////////////////////////////////

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include "common.h"

#define MAX_OPENED_FILES 16
#define INVALID_FILE_HANDLE -1
#define LOG_FS

typedef int HFILE;

class CFileHandle
{
public:
	char m_szFilename[MAX_FNAME];
	FILE* m_pFile;

	CFileHandle();
};

class CFileSystem
{
public:
	CFileSystem(void);
	~CFileSystem(void);

	void Init();
	void Shutdown();

	HFILE OpenFile( const char* pFilename, const char* pMode );
	void CloseFile( HFILE hFile );

	// File API
	CFileHandle* GetFileHandle( HFILE hFile );

	size_t GetFileLength( HFILE hFile );
	void ReadFile( HFILE hFile, void* pData, size_t uiSize );
	void WriteFile( HFILE hFile, void* pData, size_t uiSize );

	bool IsExist( const char* pFilename );

private:
	CFileHandle m_Files[MAX_OPENED_FILES];
	int m_iOpenedFiles;

};

void TrimString( char* pStr );

extern CFileSystem* g_pFS;

#endif // !FILESYSTEM_H
