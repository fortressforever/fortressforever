//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: This header, which must be the final include in a .cpp (or .h) file,
// causes all crt methods to use debugging versions of the memory allocators.
// NOTE: Use memdbgoff.h to disable memory debugging.
//
// $NoKeywords: $
//=============================================================================//

// SPECIAL NOTE! This file must *not* use include guards; we need to be able
// to include this potentially multiple times (since we can deactivate debugging
// by including memdbgoff.h)

#if !defined(STEAM) && !defined(NO_MALLOC_OVERRIDE)

// SPECIAL NOTE #2: This must be the final include in a .cpp or .h file!!!
#if defined(_DEBUG) || defined(USE_MEM_DEBUG)
#if (defined(_DEBUG) || !defined(_INC_CRTDBG)) || defined(MEMDBGON_H)
#define MEMDBGON_H

#include <tchar.h>
#include <string.h>
#include <malloc.h>
#if !defined(_DEBUG)
#define _DEBUG 1
#include <crtdbg.h>
#undef _DEBUG
#else
#include <crtdbg.h>
#endif

#include "tier0/memdbgoff.h"

#define MEM_DEBUG_ON 1

#undef malloc
#undef realloc
#undef calloc
#undef _expand
#undef free
#undef _msize

#define malloc(s)		_malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define calloc(c, s)	_calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define realloc(p, s)	_realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define free(p)			_free_dbg(p, _NORMAL_BLOCK)
#define _msize(p)		_msize_dbg(p, _NORMAL_BLOCK)
#define _expand(p, s)	_expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)


#if defined(__AFX_H__) && defined(DEBUG_NEW)
	#define new DEBUG_NEW
#else
	#undef new
	#define MEMALL_DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
	#define new MEMALL_DEBUG_NEW
#endif

#undef _strdup
#undef strdup
#undef _wcsdup
#undef wcsup

#define _strdup(s) strdup_dbg(s, __FILE__, __LINE__)
#define strdup(s)  strdup_dbg(s, __FILE__, __LINE__)
#define _wcsdup(s) wcsdup_dbg(s, __FILE__, __LINE__)
#define wcsdup(s)  wcsdup_dbg(s, __FILE__, __LINE__)

// Make sure we don't define strdup twice
#ifndef MEM_DBG_DEFINED_STRDUP
#define MEM_DBG_DEFINED_STRDUP 1

inline char *strdup_dbg(const char *pString, const char *pFileName, unsigned nLine)
{
	char *pMemory;
	
	if (!pString)
		return NULL;
	
	int len = strlen(pString) + 1;
	if ((pMemory = (char *)_malloc_dbg(len, _NORMAL_BLOCK, pFileName, nLine)) != NULL)
	{
		return strcpy( pMemory, pString );
	}
	
	return NULL;
}

inline wchar_t *wcsdup_dbg(const wchar_t *pString, const char *pFileName, unsigned nLine)
{
	wchar_t *pMemory;
	
	if (!pString)
		return NULL;
	
	int len = (wcslen(pString) + 1);
	if ((pMemory = (wchar_t *)_malloc_dbg(len * sizeof(wchar_t), _NORMAL_BLOCK, pFileName, nLine)) != NULL)
	{
		return wcscpy( pMemory, pString );
	}
	
	return NULL;
}

#endif // DBMEM_DEFINED_STRDUP

#else
#pragma message ("Note: file includes crtdbg.h directly, therefore will cannot use memdbgon.h in non-debug build")
#endif // _INC_CRTDBG

#endif // _DEBUG

#endif // !STEAM && !NO_MALLOC_OVERRIDE