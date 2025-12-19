#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <wingdi.h>
#include "debug.h"
#include "common.h"

// Global variables
DWORD		g_dwStartTime = 0;
DWORD		g_dwEndTime = 0;
float		g_fDeltaTime = 0.0f;

//-----------------------------------------------------------------------------
// Name: Sys_InitRandom()
// Desc:
//-----------------------------------------------------------------------------
void Sys_InitRandom()
{
	srand( time( NULL ) );
}

//-----------------------------------------------------------------------------
// Name: Sys_RandomRange( float fLower, float fUpper )
// Desc:
//-----------------------------------------------------------------------------
float Sys_RandomRange( float fLower, float fUpper )
{
	float fRange = fUpper - fLower;
	return fLower + ( ( float ) rand() / ( float ) RAND_MAX ) * fRange;
}

//-----------------------------------------------------------------------------
// Name: Sys_InitTimer()
// Desc: Initialize the timer.
//-----------------------------------------------------------------------------
void Sys_InitTimer()
{
	g_dwStartTime = timeGetTime();
	Sleep(1);
	g_dwEndTime = timeGetTime();
	Sleep(1);
}

//-----------------------------------------------------------------------------
// Name: Sys_GetDelta()
// Desc:
//-----------------------------------------------------------------------------
float Sys_GetDelta()
{
	return g_fDeltaTime;
}

//-----------------------------------------------------------------------------
// Name: Sys_UpdateTimer()
// Desc: Update system timer.
//-----------------------------------------------------------------------------
void Sys_UpdateTimer()
{
	// Calculate time
	g_dwStartTime = timeGetTime();
	g_fDeltaTime = g_dwEndTime > 0 ? ( float ) (( double ) (g_dwStartTime - g_dwEndTime) / 1000.0f) : ( float ) (1.0f / 120.0f);
	g_dwEndTime = g_dwStartTime;
}

// Event system

#define MAX_EVENTS 12

struct Sys_Event {
	EventType_t type;
	int userNum;
	const char* userString;
};

static Sys_Event g_events[MAX_EVENTS];
static int g_numEvents = 0;

//-----------------------------------------------------------------------------
// Name: Sys_SendEvent( EventType_t type, int userNum, const char* userString )
// Desc: Send event to the engine bus.
//-----------------------------------------------------------------------------
void Sys_SendEvent( EventType_t type, int userNum, const char* userString )
{
	// Check for out of bounds
	if ( g_numEvents >= MAX_EVENTS )
	{
		DPrintf( "Sys_SendEvent: buffer overflow, event skipped...\n" );
		return;
	}

	// Check for our of range
	if ( type >= EVENT_COUNT )
	{
		DError( "Sys_SendEvent: event type is greater than EVENT_COUNT\n" );
	}

	// Allocate event
	Sys_Event* pEvent = &g_events[ g_numEvents++ ];
	pEvent->type = type;
	pEvent->userNum = userNum;
	pEvent->userString = userString;
}

//-----------------------------------------------------------------------------
// Name: Sys_ProcessEvents( eventOverviewFunc_t eventOverviewFunc )
// Desc: Event update.
//-----------------------------------------------------------------------------
void Sys_ProcessEvents( eventOverviewFunc_t eventOverviewFunc )
{
	assert( eventOverviewFunc );

	// Process events
	for ( int i = 0; i < g_numEvents; i++ )
	{
		Sys_Event* pEvent = &g_events[ i ];
		eventOverviewFunc( pEvent->type, pEvent->userNum, pEvent->userString );
	}

	// Clear everything
	if ( g_numEvents )
	{
		memset( g_events, 0, sizeof( g_events ) );
		g_numEvents = 0;
	}
}