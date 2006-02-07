//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: A fast stack memory allocator that uses virtual memory if available
//
//=============================================================================//

#ifndef MEMSTACK_H
#define MEMSTACK_H

#if defined( _WIN32 )
#pragma once
#endif

//-----------------------------------------------------------------------------

class CMemoryStack
{
public:
	CMemoryStack();
	~CMemoryStack();

	bool Init( unsigned maxSize = 0, unsigned commitSize = 0, unsigned initialCommit = 0 );
	void Term();

	int GetSize() { return m_size; }
	int	GetUsed() { return ( m_pNextAlloc - m_pBase ); }
	
	void *Alloc( unsigned bytes );
	void *GetBase();
	void *GetCurrentAllocPoint();
	void FreeToAllocPoint( void * );
	void FreeAll();
	
	void Access( void **ppRegion, unsigned *pBytes );

private:
	unsigned char *m_pNextAlloc;
	unsigned char *m_pCommitLimit;
	unsigned char *m_pAllocLimit;
	
	unsigned char *m_pBase;

	unsigned	   m_size;
#ifdef _WIN32
	unsigned	   m_commitSize;
	unsigned	   m_minCommit;
#endif
};

//-----------------------------------------------------------------------------

#endif // MEMSTACK_H
