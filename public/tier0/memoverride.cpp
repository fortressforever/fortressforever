//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Insert this file into all projects using the memory system
// It will cause that project to use the shader memory allocator
//
// $NoKeywords: $
//=============================================================================//


#if !defined(STEAM) && !defined(NO_MALLOC_OVERRIDE)

#undef PROTECTED_THINGS_ENABLE   // allow use of _vsnprintf

#ifdef _WIN32
#define WIN_32_LEAN_AND_MEAN
#include <windows.h>
#endif



#include "tier0/dbg.h"
#include "tier0/memalloc.h"
#include <string.h>
#include <stdio.h>
#include "memdbgoff.h"

#ifdef _WIN32
// ARG: crtdbg is necessary for certain definitions below,
// but it also redefines malloc as a macro in release.
// To disable this, we gotta define _DEBUG before including it.. BLEAH!
#define _DEBUG 1
#include "crtdbg.h"
#ifdef NDEBUG
#undef _DEBUG
#endif

// Turn this back off in release mode.
#ifdef NDEBUG
#undef _DEBUG
#endif

#elif _LINUX
#define __cdecl
#endif

#ifdef _WIN32
const char *MakeModuleFileName()
{
	if ( g_pMemAlloc->IsDebugHeap() )
	{
		char *pszModuleName = (char *)HeapAlloc( GetProcessHeap(), 0, MAX_PATH ); // small leak, debug only

		MEMORY_BASIC_INFORMATION mbi;
		static int dummy;
		VirtualQuery( &dummy, &mbi, sizeof(mbi) );

		GetModuleFileName( reinterpret_cast<HMODULE>(mbi.AllocationBase), pszModuleName, MAX_PATH );
		return pszModuleName;
	}
	return NULL;
}

static void *AllocUnattributed( size_t nSize )
{
	static const char *pszOwner = MakeModuleFileName();

	if ( !pszOwner )
		return g_pMemAlloc->Alloc(nSize);
	else
		return g_pMemAlloc->Alloc(nSize, pszOwner, 0);
}

static void *ReallocUnattributed( void *pMem, size_t nSize )
{
	static const char *pszOwner = MakeModuleFileName();

	if ( !pszOwner )
		return g_pMemAlloc->Realloc(pMem, nSize);
	else
		return g_pMemAlloc->Realloc(pMem, nSize, pszOwner, 0);
}

#else
#define MakeModuleFileName() NULL
inline void *AllocUnattributed( size_t nSize )
{
	return g_pMemAlloc->Alloc(nSize);
}

inline void *ReallocUnattributed( void *pMem, size_t nSize )
{
	return g_pMemAlloc->Realloc(pMem, nSize);
}
#endif

//-----------------------------------------------------------------------------
// Standard functions in the CRT that we're going to override to call our allocator
//-----------------------------------------------------------------------------
#ifdef _WIN32
// this magic only works under win32
// under linux this malloc() overrides the libc malloc() and so we
// end up in a recursion (as g_pMemAlloc->Alloc() calls malloc)
extern "C"
{
	
void *malloc( size_t nSize )
{
	return AllocUnattributed( nSize );
}

void free( void *pMem )
{
	g_pMemAlloc->Free(pMem);
}

void *realloc( void *pMem, size_t nSize )
{
	return ReallocUnattributed( pMem, nSize );
}

void *calloc( size_t nCount, size_t nElementSize )
{
	void *pMem = AllocUnattributed( nElementSize * nCount );
	memset(pMem, 0, nElementSize * nCount);
	return pMem;
}

} // end extern "C"
#endif

//-----------------------------------------------------------------------------
// Non-standard MSVC functions that we're going to override to call our allocator
//-----------------------------------------------------------------------------
extern "C"
{
	
void *_malloc_base( size_t nSize )
{
	return AllocUnattributed( nSize );
}

void _free_base( void *pMem )
{
	g_pMemAlloc->Free(pMem);
}

void *_realloc_base( void *pMem, size_t nSize )
{
	return ReallocUnattributed( pMem, nSize );
}

int _heapchk()
{
	return g_pMemAlloc->heapchk();
}

int _heapmin()
{
	return 1;
}

size_t _msize( void *pMem )
{
	return g_pMemAlloc->GetSize(pMem);
}

size_t msize( void *pMem )
{
	return g_pMemAlloc->GetSize(pMem);
}

void *__cdecl _heap_alloc( size_t nSize )
{
	return AllocUnattributed( nSize );
}

void *__cdecl _nh_malloc( size_t nSize, int )
{
	return AllocUnattributed( nSize );
}

void *__cdecl _expand( void *pMem, size_t nSize )
{
	return g_pMemAlloc->Expand(pMem, nSize);
}

unsigned int _amblksiz = 16; //BYTES_PER_PARA;

size_t __cdecl _get_sbh_threshold( void )
{
	return 0;
}

int __cdecl _set_sbh_threshold( size_t )
{
	return 0;
}

int __cdecl _heapadd( void *, size_t )
{
	return 0;
}

int __cdecl _heapset( unsigned int )
{
	return 0;
}

size_t __cdecl _heapused( size_t *, size_t * )
{
	return 0;
}

#ifdef _WIN32
int __cdecl _heapwalk( _HEAPINFO * )
{
	return 0;
}
#endif

} // end extern "C"


//-----------------------------------------------------------------------------
// Debugging functions that we're going to override to call our allocator
// NOTE: These have to be here for release + debug builds in case we
// link to a debug static lib!!!
//-----------------------------------------------------------------------------

extern "C"
{
	
void *malloc_db( size_t nSize, const char *pFileName, int nLine )
{
	return g_pMemAlloc->Alloc(nSize, pFileName, nLine);
}

void free_db( void *pMem, const char *pFileName, int nLine )
{
	g_pMemAlloc->Free(pMem, pFileName, nLine);
}

void *realloc_db( void *pMem, size_t nSize, const char *pFileName, int nLine )
{
	return g_pMemAlloc->Realloc(pMem, nSize, pFileName, nLine);
}
	
} // end extern "C"



//-----------------------------------------------------------------------------
// Prevents us from using an inappropriate new or delete method,
// ensures they are here even when linking against debug or release static libs
//-----------------------------------------------------------------------------
void *operator new( unsigned int nSize )
{
	return AllocUnattributed( nSize );
}

void *operator new( unsigned int nSize, int nBlockUse, const char *pFileName, int nLine )
{
	return g_pMemAlloc->Alloc(nSize, pFileName, nLine);
}

void operator delete( void *pMem )
{
	g_pMemAlloc->Free( pMem );
}

void *operator new[] ( unsigned int nSize )
{
	return AllocUnattributed( nSize );
}

void *operator new[] ( unsigned int nSize, int nBlockUse, const char *pFileName, int nLine )
{
	return g_pMemAlloc->Alloc(nSize, pFileName, nLine);
}

void operator delete[] ( void *pMem )
{
	g_pMemAlloc->Free( pMem );
}


//-----------------------------------------------------------------------------
// These methods are standard MSVC heap initialization + shutdown methods
//-----------------------------------------------------------------------------
extern "C"
{

int __cdecl _heap_init()
{
	return g_pMemAlloc != NULL;
}

void __cdecl _heap_term()
{
}
	
}


//-----------------------------------------------------------------------------
// Override some debugging allocation methods in MSVC
// NOTE: These have to be here for release + debug builds in case we
// link to a debug static lib!!!
//-----------------------------------------------------------------------------
#ifdef _WIN32

// This here just hides the internal file names, etc of allocations
// made in the c runtime library
#define CRT_INTERNAL_FILE_NAME "C-runtime internal"

class CAttibCRT
{
public:
	CAttibCRT(int nBlockUse) : m_nBlockUse(nBlockUse)
	{
		if (m_nBlockUse == _CRT_BLOCK)
		{
			g_pMemAlloc->PushAllocDbgInfo(CRT_INTERNAL_FILE_NAME, 0);
		}
	}
	
	~CAttibCRT()
	{
		if (m_nBlockUse == _CRT_BLOCK)
		{
			g_pMemAlloc->PopAllocDbgInfo();
		}
	}
	
private:
	int m_nBlockUse;
};


#define AttribIfCrt() CAttibCRT _attrib(nBlockUse)
#elif _LINUX
#define AttribIfCrt()
#endif // _WIN32


extern "C"
{
	
void *__cdecl _nh_malloc_dbg( size_t nSize, int nFlag, int nBlockUse,
								const char *pFileName, int nLine )
{
	AttribIfCrt();
	return g_pMemAlloc->Alloc(nSize, pFileName, nLine);
}

void *__cdecl _malloc_dbg( size_t nSize, int nBlockUse,
							const char *pFileName, int nLine )
{
	AttribIfCrt();
	return g_pMemAlloc->Alloc(nSize, pFileName, nLine);
}

void *__cdecl _calloc_dbg( size_t nNum, size_t nSize, int nBlockUse,
							const char *pFileName, int nLine )
{
	AttribIfCrt();
	void *pMem = g_pMemAlloc->Alloc(nSize * nNum, pFileName, nLine);
	memset(pMem, 0, nSize * nNum);
	return pMem;
}

void *__cdecl _realloc_dbg( void *pMem, size_t nNewSize, int nBlockUse,
							const char *pFileName, int nLine )
{
	AttribIfCrt();
	return g_pMemAlloc->Realloc(pMem, nNewSize, pFileName, nLine);
}

void *__cdecl _expand_dbg( void *pMem, size_t nNewSize, int nBlockUse,
							const char *pFileName, int nLine )
{
	AttribIfCrt();
	return g_pMemAlloc->Expand(pMem, nNewSize, pFileName, nLine);
}

void __cdecl _free_dbg( void *pMem, int nBlockUse )
{
	AttribIfCrt();
	g_pMemAlloc->Free(pMem);
}

size_t __cdecl _msize_dbg( void *pMem, int nBlockUse )
{
#ifdef _WIN32
	return _msize(pMem);
#elif _LINUX
	Assert( "_msize_dbg unsupported" );
	return 0;
#endif
}


#ifdef _WIN32

#if defined(_DEBUG) && _MSC_VER >= 1300
void    __cdecl _aligned_free_base(
        void *
        );

void *  __cdecl _aligned_malloc_base(
        size_t,
        size_t
        );

void * __cdecl _aligned_malloc(
        size_t size,
        size_t align
        )
{
    return _aligned_malloc_base(size, align);
}

void __cdecl _aligned_free(
        void *memblock
        )
{
    _aligned_free_base(memblock);
}

#endif
#endif

} // end extern "C"


//-----------------------------------------------------------------------------
// Override some the _CRT debugging allocation methods in MSVC
//-----------------------------------------------------------------------------
#ifdef _WIN32


extern "C"
{
	
int _CrtDumpMemoryLeaks(void)
{
	return 0;
}

_CRT_DUMP_CLIENT _CrtSetDumpClient( _CRT_DUMP_CLIENT dumpClient )
{
	return NULL;
}

int _CrtSetDbgFlag( int nNewFlag )
{
	return g_pMemAlloc->CrtSetDbgFlag( nNewFlag );
}

long _crtBreakAlloc;      /* Break on this allocation */
int _crtDbgFlag = _CRTDBG_ALLOC_MEM_DF;

void __cdecl _CrtSetDbgBlockType( void *pMem, int nBlockUse )
{
	DebuggerBreak();
}

_CRT_ALLOC_HOOK __cdecl _CrtSetAllocHook( _CRT_ALLOC_HOOK pfnNewHook )
{
	DebuggerBreak();
	return NULL;
}

long __cdecl _CrtSetBreakAlloc( long lNewBreakAlloc )
{
	return g_pMemAlloc->CrtSetBreakAlloc( lNewBreakAlloc );
}
					 
int __cdecl _CrtIsValidHeapPointer( const void *pMem )
{
	return g_pMemAlloc->CrtIsValidHeapPointer( pMem );
}

int __cdecl _CrtIsValidPointer( const void *pMem, unsigned int size, int access )
{
	return g_pMemAlloc->CrtIsValidPointer( pMem, size, access );
}

int __cdecl _CrtCheckMemory( void )
{
	// FIXME: Remove this when we re-implement the heap
	return g_pMemAlloc->CrtCheckMemory( );
}

int __cdecl _CrtIsMemoryBlock( const void *pMem, unsigned int nSize,
    long *plRequestNumber, char **ppFileName, int *pnLine )
{
	DebuggerBreak();
	return 1;
}

int __cdecl _CrtMemDifference( _CrtMemState *pState, const _CrtMemState * oldState, const _CrtMemState * newState )
{
	DebuggerBreak();
	return FALSE;
}

void __cdecl _CrtMemDumpStatistics( const _CrtMemState *pState )
{
	DebuggerBreak();	
}

void __cdecl _CrtMemCheckpoint( _CrtMemState *pState )
{
	// FIXME: Remove this when we re-implement the heap
	g_pMemAlloc->CrtMemCheckpoint( pState );
}

void __cdecl _CrtMemDumpAllObjectsSince( const _CrtMemState *pState )
{
	DebuggerBreak();
}

void __cdecl _CrtDoForAllClientObjects( void (*pfn)(void *, void *), void * pContext )
{
	DebuggerBreak();
}


//-----------------------------------------------------------------------------
// Methods in dbgrpt.cpp 
//-----------------------------------------------------------------------------
long _crtAssertBusy = -1;

int __cdecl _CrtSetReportMode( int nReportType, int nReportMode )
{
	return g_pMemAlloc->CrtSetReportMode( nReportType, nReportMode );
}

_HFILE __cdecl _CrtSetReportFile( int nRptType, _HFILE hFile )
{
	return (_HFILE)g_pMemAlloc->CrtSetReportFile( nRptType, hFile );
}

_CRT_REPORT_HOOK __cdecl _CrtSetReportHook( _CRT_REPORT_HOOK pfnNewHook )
{
	return (_CRT_REPORT_HOOK)g_pMemAlloc->CrtSetReportHook( pfnNewHook );
}

int __cdecl _CrtDbgReport( int nRptType, const char * szFile,
        int nLine, const char * szModule, const char * szFormat, ... )
{
	static char output[1024];
	va_list args;
	va_start( args, szFormat );
	_vsnprintf( output, sizeof( output )-1, szFormat, args );
	va_end( args );

	return g_pMemAlloc->CrtDbgReport( nRptType, szFile, nLine, szModule, output );
}

	
} // end extern "C"
#endif // _WIN32

// Most files include this file, so when it's used it adds an extra .ValveDbg section,
// to help identify debug binaries.
#ifdef _WIN32

	#ifndef NDEBUG // _DEBUG
		#pragma data_seg("ValveDBG") 

		volatile const char* DBG = "*** DEBUG STUB ***";                     

	#endif

#endif

#endif // !STEAM && !NO_MALLOC_OVERRIDE