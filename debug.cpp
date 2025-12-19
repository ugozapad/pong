#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "debug.h"

FILE* g_logFile = 0;

#define LOG_FLUSH

void Debug_Init()
{
	g_logFile = fopen( "game.log", "w" );
	assert( g_logFile );

	DPrintf("Log started\n");
}

void Debug_Shutdown()
{
	fclose( g_logFile );
}

void DPrintf( const char* msg, ... )
{
	static char buffer[ 1024 ];
	va_list		args;

	va_start( args, msg );
	int len = vsprintf( buffer, msg, args );
	va_end( args );

	if (g_logFile)
		fwrite( buffer, len, 1, g_logFile );

#ifdef LOG_FLUSH
	if (g_logFile)
		fflush(g_logFile); 
#endif
}

void DError( const char* msg, ... )
{
	static char buffer[ 1024 ];
	va_list		args;

	va_start( args, msg );
	vsprintf( buffer, msg, args );
	va_end( args );

	// printf error to log
	DPrintf( "%s\n", buffer );

	fflush( g_logFile );

	MessageBox( NULL, buffer, "Critical Error", MB_OK | MB_ICONERROR );

	ExitProcess( -1 );
}