// debug.h: interface for the debug.
#ifndef DEBUG_H
#define DEBUG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Enable memory debugging
#ifdef _DEBUG
#define MEMORY_DEBUG
#endif // _DEBUG

#ifdef MEMORY_DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif // MEMORY_DEBUG

#include <assert.h>

void Debug_Init();
void Debug_Shutdown();

void DPrintf( const char* msg, ... );
void DError( const char* msg, ... );

#ifdef MEMORY_DEBUG
inline void* operator new(size_t size, const char* file, int line) {
    return _malloc_dbg(size, _NORMAL_BLOCK, file, line);
}

inline void operator delete(void* p, const char*, int) { 
    _free_dbg(p, _NORMAL_BLOCK); 
}

// Macro
#define NEW new(__FILE__, __LINE__)

#else

#define NEW new

#endif

#endif // !DEBUG_H
