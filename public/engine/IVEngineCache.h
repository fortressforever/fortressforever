//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef IVENGINECACHE_H
#define IVENGINECACHE_H
#ifdef _WIN32
#pragma once
#endif

#include "cache_user.h"

//-----------------------------------------------------------------------------
// IVEngineCache
//-----------------------------------------------------------------------------

// change this when the new version is incompatable with the old
#define VENGINE_CACHE_INTERFACE_VERSION		"VEngineCache001"

class IVEngineCache
{
public:
	virtual void Flush( void ) = 0;
	
	// returns the cached data, and moves to the head of the LRU list
	// if present, otherwise returns NULL
	virtual void *Check( cache_user_t *c ) = 0;

	virtual void Free( cache_user_t *c ) = 0;
	
	// Returns NULL if all purgable data was tossed and there still
	// wasn't enough room.
	virtual void *Alloc( cache_user_t *c, int size, const char *name ) = 0;
	
	virtual void Report( void ) = 0;
	
	// all cache entries that subsequently allocated or successfully checked 
	// are considered "locked" and will not be freed when additional memory is needed
	virtual void EnterCriticalSection() = 0;

	// reset all protected blocks to normal
	virtual void ExitCriticalSection() = 0;
};

class CEngineCacheCriticalSection
{
public:
	CEngineCacheCriticalSection( IVEngineCache *pCache )
		: m_pCache( pCache )
	{
		m_pCache->EnterCriticalSection();
	}

	~CEngineCacheCriticalSection()
	{
		m_pCache->ExitCriticalSection();
	}

private:
	IVEngineCache *m_pCache;
};

#endif // IVENGINECACHE_H
