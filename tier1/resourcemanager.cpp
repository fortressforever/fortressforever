//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "basetypes.h"
#include "resourcemanager.h"

DECLARE_POINTER_HANDLE( memhandle_t );


CResourceLRUBase::CResourceLRUBase( unsigned int maxSize )
{
	m_targetMemorySize = maxSize;
	m_memUsed = 0;
	m_lruList = m_memoryLists.CreateList();
	m_lockList = m_memoryLists.CreateList();
	m_freeList = m_memoryLists.CreateList();
	m_listsAreFreed = 0;
}

CResourceLRUBase::~CResourceLRUBase() 
{
	Assert( m_listsAreFreed );
}

void CResourceLRUBase::SetTargetSize( unsigned int targetSize )
{
	m_targetMemorySize = targetSize;
}

// Frees everything!  The LRU AND the LOCKED items.  This is only used to forcibly free the resources,
// not to make space.
void CResourceLRUBase::FreeAllLists() 
{
	int node;
	int nextNode;

	node = m_memoryLists.Head(m_lruList);
	while ( node != m_memoryLists.InvalidIndex() )
	{
		nextNode = m_memoryLists.Next(node);
		m_memoryLists.Unlink( m_lruList, node );
		FreeByIndex( node );
		node = nextNode;
	}

	node = m_memoryLists.Head(m_lockList);
	while ( node != m_memoryLists.InvalidIndex() )
	{
		nextNode = m_memoryLists.Next(node);
		m_memoryLists.Unlink( m_lockList, node );
		m_memoryLists[node].lockCount = 0;
		FreeByIndex( node );
		node = nextNode;
	}
	m_listsAreFreed = true;
}


void CResourceLRUBase::FlushAllUnlocked()
{
	int node = m_memoryLists.Head(m_lruList);
	while ( node != m_memoryLists.InvalidIndex() )
	{
		int next = m_memoryLists.Next(node);
		m_memoryLists.Unlink( m_lruList, node );
		FreeByIndex( node );
		node = next;
	}
}

void CResourceLRUBase::FlushToTargetSize()
{
	EnsureCapacity(0);
}

void CResourceLRUBase::FlushAll()
{
	FreeAllLists();
	m_listsAreFreed = false;
}


void CResourceLRUBase::DestroyResource( memhandle_t handle )
{
	unsigned short index = FromHandle( handle );
	if ( !m_memoryLists.IsValidIndex(index) )
		return;
	
	Assert( m_memoryLists[index].lockCount == 0  );
	m_memoryLists.Unlink( m_lruList, index );
	FreeByIndex( index );
}


void *CResourceLRUBase::LockResource( memhandle_t handle )
{
	return LockByIndex( FromHandle(handle) );
}

int CResourceLRUBase::UnlockResource( memhandle_t handle )
{
	return UnlockByIndex( FromHandle(handle) );
}

void *CResourceLRUBase::GetResource_NoLockNoLRUTouch( memhandle_t handle )
{
	unsigned short memoryIndex = FromHandle(handle);
	if ( memoryIndex != m_memoryLists.InvalidIndex() )
	{
		return m_memoryLists[memoryIndex].pStore;
	}
	return NULL;
}


void *CResourceLRUBase::GetResource_NoLock( memhandle_t handle )
{
	unsigned short memoryIndex = FromHandle(handle);
	if ( memoryIndex != m_memoryLists.InvalidIndex() )
	{
		TouchByIndex( memoryIndex );
		return m_memoryLists[memoryIndex].pStore;
	}
	return NULL;
}

void CResourceLRUBase::TouchResource( memhandle_t handle )
{
	TouchByIndex( FromHandle(handle) );
}

void CResourceLRUBase::MarkAsStale( memhandle_t handle )
{
	MarkAsStaleByIndex( FromHandle(handle) );
}

int CResourceLRUBase::BreakLock( memhandle_t handle )
{
	unsigned short memoryIndex = FromHandle(handle);
	if ( memoryIndex != m_memoryLists.InvalidIndex() && m_memoryLists[memoryIndex].lockCount )
	{
		int nBroken = m_memoryLists[memoryIndex].lockCount;
		m_memoryLists[memoryIndex].lockCount = 0;
		m_memoryLists.Unlink( m_lockList, memoryIndex );
		m_memoryLists.LinkToTail( m_lruList, memoryIndex );

		return nBroken;
	}
	return 0;
}

int CResourceLRUBase::BreakAllLocks()
{
	int nBroken = 0;
	int node;
	int nextNode;

	node = m_memoryLists.Head(m_lockList);
	while ( node != m_memoryLists.InvalidIndex() )
	{
		nBroken++;
		nextNode = m_memoryLists.Next(node);
		m_memoryLists[node].lockCount = 0;
		m_memoryLists.Unlink( m_lockList, node );
		m_memoryLists.LinkToTail( m_lruList, node );
		node = nextNode;
	}

	return nBroken;

}

unsigned short CResourceLRUBase::CreateHandle()
{
	int memoryIndex = m_memoryLists.Head(m_freeList);
	if ( memoryIndex != m_memoryLists.InvalidIndex() )
	{
		m_memoryLists.Unlink( m_freeList, memoryIndex );
		m_memoryLists.LinkToTail( m_lruList, memoryIndex );
	}
	else
	{
		memoryIndex = m_memoryLists.AddToTail( m_lruList );
	}
	return memoryIndex;
}

memhandle_t CResourceLRUBase::StoreResourceInHandle( unsigned short memoryIndex, void *pStore, unsigned int realSize )
{
	resource_lru_element_t &mem = m_memoryLists[memoryIndex];
	mem.pStore = pStore;
	m_memUsed += realSize;
	return ToHandle(memoryIndex);
}

void *CResourceLRUBase::LockByIndex( unsigned short memoryIndex )
{
	if ( memoryIndex != m_memoryLists.InvalidIndex() )
	{
		if ( m_memoryLists[memoryIndex].lockCount == 0 )
		{
			m_memoryLists.Unlink( m_lruList, memoryIndex );
			m_memoryLists.LinkToTail( m_lockList, memoryIndex );
		}
		Assert(m_memoryLists[memoryIndex].lockCount != (unsigned short)-1);
		m_memoryLists[memoryIndex].lockCount++;
		return m_memoryLists[memoryIndex].pStore;
	}

	return NULL;
}

void CResourceLRUBase::TouchByIndex( unsigned short memoryIndex )
{
	if ( memoryIndex != m_memoryLists.InvalidIndex() )
	{
		if ( m_memoryLists[memoryIndex].lockCount == 0 )
		{
			m_memoryLists.Unlink( m_lruList, memoryIndex );
			m_memoryLists.LinkToTail( m_lruList, memoryIndex );
		}
	}
}


void CResourceLRUBase::MarkAsStaleByIndex( unsigned short memoryIndex )
{
	if ( memoryIndex != m_memoryLists.InvalidIndex() )
	{
		if ( m_memoryLists[memoryIndex].lockCount == 0 )
		{
			m_memoryLists.Unlink( m_lruList, memoryIndex );
			m_memoryLists.LinkToHead( m_lruList, memoryIndex );
		}
	}
}

memhandle_t CResourceLRUBase::ToHandle( unsigned short index )
{
	unsigned int hiword = m_memoryLists.Element(index).serial;
	hiword <<= 16;
	index++;
	return (memhandle_t)( hiword|index );
}

unsigned int CResourceLRUBase::TargetSize() 
{ 
	return MemTotal_Inline(); 
}

unsigned int CResourceLRUBase::AvailableSize()
{ 
	return MemAvailable_Inline(); 
}


unsigned int CResourceLRUBase::UsedSize()
{ 
	return MemUsed_Inline(); 
}

bool CResourceLRUBase::FreeLRU()
{
	int lruIndex = m_memoryLists.Head( m_lruList );
	if ( lruIndex == m_memoryLists.InvalidIndex() )
		return false;
	m_memoryLists.Unlink( m_lruList, lruIndex );
	FreeByIndex( lruIndex );
	return true;
}


// free resources until there is enough space to hold "size"
void CResourceLRUBase::EnsureCapacity( unsigned int size )
{
	while ( MemUsed_Inline() > MemTotal_Inline() || MemAvailable_Inline() < size )
	{
		if ( !FreeLRU() )
			break;
	}
}

// unlock this resource, moving out of the locked list if ref count is zero
int CResourceLRUBase::UnlockByIndex( unsigned short memoryIndex )
{
	if ( memoryIndex != m_memoryLists.InvalidIndex() )
	{
		Assert( m_memoryLists[memoryIndex].lockCount > 0 );
		if ( m_memoryLists[memoryIndex].lockCount > 0 )
		{
			m_memoryLists[memoryIndex].lockCount--;
			if ( m_memoryLists[memoryIndex].lockCount == 0 )
			{
				m_memoryLists.Unlink( m_lockList, memoryIndex );
				m_memoryLists.LinkToTail( m_lruList, memoryIndex );
			}
		}
		return m_memoryLists[memoryIndex].lockCount;
	}

	return 0;
}


// free this resource and move the handle to the free list
void CResourceLRUBase::FreeByIndex( unsigned short memoryIndex )
{
	if ( memoryIndex != m_memoryLists.InvalidIndex() )
	{
		Assert( m_memoryLists[memoryIndex].lockCount == 0 );

		resource_lru_element_t &mem = m_memoryLists[memoryIndex];
		m_memUsed -= GetRealSize( mem.pStore );
		this->DestroyResourceStorage( mem.pStore );
		mem.pStore = NULL;
		mem.serial++;
		m_memoryLists.LinkToTail( m_freeList, memoryIndex );
	}
}

// get a list of everything in the LRU
void CResourceLRUBase::GetLRUHandleList( CUtlVector< memhandle_t >& list )
{
	for ( int node = m_memoryLists.Head(m_lruList);
			node != m_memoryLists.InvalidIndex();
			node = m_memoryLists.Next(node) )
	{
		list.AddToTail( ToHandle( node ) );
	}
}

