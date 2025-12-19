#ifndef COMMON_H
#define COMMON_H

#ifndef MAX_FNAME
#define MAX_FNAME			260
#endif

#ifndef M_PI
#define M_PI				3.14159265358979323846
#endif

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

struct SVIEWPORT
{
	int Width;
	int Height;
};

#include <assert.h>

#include "Vector.h"

// Global functions
float Sys_GetDelta();
float Sys_RandomRange( float fLower, float fUpper );

// System functions

// Random
void Sys_InitRandom();

// Timer
void Sys_InitTimer();
float Sys_GetDelta();
void Sys_UpdateTimer();

// Event system

enum EventType_t
{
	EVENT_NONE,
	EVENT_INIT_GAME,
	EVENT_SHUTDOWN_GAME,
	EVENT_ON_GAME_INIT, // Game part
	EVENT_COUNT,
	EVENT_FORCE_DWORD = 0xffffffff
};

// User-defined event overview function
typedef void ( *eventOverviewFunc_t )( EventType_t type, int userNum, const char* userString );

// Send the event
void Sys_SendEvent( EventType_t type, int userNum, const char* userString );

// Event update
void Sys_ProcessEvents( eventOverviewFunc_t eventOverviewFunc );

#endif