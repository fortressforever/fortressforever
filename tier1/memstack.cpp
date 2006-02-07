//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifdef _WIN32
#define WIN_32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "tier0/dbg.h"
#include "memstack.h"

template <typename T>
inline T MemAlign( T val, unsigned alignment )
{
	return (T)( ( (unsigned)val + alignment - 1 ) & ~( alignment - 1 ) );
}

//-------------------------------------

CMemoryStack::CMemoryStack()
 : 	m_pBase( NULL ),
#ifdef _WIN32
 	m_commitSize( 0 ),
	m_minCommit( 0 ),
#endif
 	m_size( 0 )
{
}
	
//-------------------------------------

CMemoryStack::~CMemoryStack()
{
	if ( m_pBase )
		Term();
}

//-------------------------------------

bool CMemoryStack::Init( unsigned maxSize, unsigned commitSize, unsigned initialCommit )
{
	Assert( !m_pBase );

	m_size = maxSize;
	Assert( m_size > 0 );

#ifdef _WIN32

	if ( commitSize != 0 )
	{
		m_commitSize = commitSize;
	}
	

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	Assert( !( sysInfo.dwPageSize & (sysInfo.dwPageSize-1)) );
	
	if ( m_commitSize == 0 )
	{
		m_commitSize = sysInfo.dwPageSize;
	}
	else
	{
		m_commitSize = MemAlign( m_commitSize, sysInfo.dwPageSize );
	}

	m_size = MemAlign( m_size, m_commitSize );
	
	Assert( m_size % sysInfo.dwPageSize == 0 && m_commitSize % sysInfo.dwPageSize == 0 && m_commitSize <= m_size );

	m_pBase = (unsigned char *)VirtualAlloc( NULL, m_size, MEM_RESERVE, PAGE_NOACCESS );
	Assert( m_pBase );
	m_pCommitLimit = m_pNextAlloc = m_pBase;

	if ( initialCommit )
	{
		initialCommit = MemAlign( initialCommit, m_commitSize );
		Assert( initialCommit < m_size );
		if ( !VirtualAlloc( m_pCommitLimit, initialCommit, MEM_COMMIT, PAGE_READWRITE ) )
			return false;
		m_minCommit = initialCommit;
		m_pCommitLimit += initialCommit;
	}

#else
	m_pBase = new unsigned char[m_size];
	m_pNextAlloc = m_pBase;
	m_pCommitLimit = m_pBase + m_size;
#endif

	m_pAllocLimit = m_pBase + m_size;

	return ( m_pBase != NULL );
}

//-------------------------------------

void CMemoryStack::Term()
{
	FreeAll();
	if ( m_pBase )
	{
#ifdef _WIN32
		VirtualFree( m_pBase, 0, MEM_RELEASE );
#else
		delete m_pBase;
#endif
		m_pBase = NULL;
	}
}

//-------------------------------------

void *CMemoryStack::Alloc( unsigned bytes )
{
	Assert( m_pBase );
	
	if ( !bytes )
		bytes = 1;

	bytes = MemAlign( bytes, 16 );

	void *pResult = m_pNextAlloc;
	m_pNextAlloc += bytes;
	
	if ( m_pNextAlloc > m_pCommitLimit )
	{
#ifdef _WIN32
		unsigned char *	pNewCommitLimit = MemAlign( m_pNextAlloc, m_commitSize );
		unsigned 		commitSize 		= pNewCommitLimit - m_pCommitLimit;
		
		Assert( m_pCommitLimit + commitSize < m_pAllocLimit );
		if ( !VirtualAlloc( m_pCommitLimit, commitSize, MEM_COMMIT, PAGE_READWRITE ) )
		{
			Assert( 0 );
			return NULL;
		}
		m_pCommitLimit = pNewCommitLimit;
#else
		Assert( 0 );
		return NULL;
#endif
	}
	else
	{
		// MEM_COMMIT zeros out the memory, simulate that if mem is reused
		memset( pResult, 0, bytes );
	}
	
	return pResult;
}

//-------------------------------------

void *CMemoryStack::GetBase()
{
	return m_pBase;
}

//-------------------------------------

void *CMemoryStack::GetCurrentAllocPoint()
{
	return m_pNextAlloc;
}

//-------------------------------------

void CMemoryStack::FreeToAllocPoint( void *pAllocPoint )
{
	Assert( pAllocPoint >= m_pBase && pAllocPoint <= m_pNextAlloc );
	
	if ( pAllocPoint >= m_pBase && pAllocPoint < m_pNextAlloc )
	{
#ifdef _WIN32
		unsigned char *pDecommitPoint = MemAlign( (unsigned char *)pAllocPoint, m_commitSize );

		if ( pDecommitPoint < m_pBase + m_minCommit )
		{
			pDecommitPoint = m_pBase + m_minCommit;
		}

		unsigned decommitSize = m_pCommitLimit - pDecommitPoint;
		
		if ( decommitSize > 0 )
		{
			VirtualFree( pDecommitPoint, decommitSize, MEM_DECOMMIT );
			m_pCommitLimit = pDecommitPoint;
		}
#endif
		m_pNextAlloc = (unsigned char *)pAllocPoint;
	}
}

//-------------------------------------

void CMemoryStack::FreeAll()
{
	if ( m_pBase && m_pCommitLimit - m_pBase > 0 )
	{
#ifdef _WIN32
		VirtualFree( m_pBase, m_pCommitLimit - m_pBase, MEM_DECOMMIT );
		m_pCommitLimit = m_pBase;
#endif
		m_pNextAlloc = m_pBase;
	}
}

//-------------------------------------

void CMemoryStack::Access( void **ppRegion, unsigned *pBytes )
{
	*ppRegion = m_pBase;
	*pBytes = ( m_pNextAlloc - m_pBase);
}

//-----------------------------------------------------------------------------
