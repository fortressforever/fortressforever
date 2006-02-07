//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H
#ifdef _WIN32
#pragma once
#endif

FORWARD_DECLARE_HANDLE( memhandle_t );
#include "utlmultilist.h"
#include "utlvector.h"

class CResourceLRUBase
{
public:

	// public API
	// -----------------------------------------------------------------------------
	// memhandle_t			CreateResource( params ) // implemented by derived class
	void					DestroyResource( memhandle_t handle );

	// type-safe implementation in derived class
	//void					*LockResource( memhandle_t handle );
	int						UnlockResource( memhandle_t handle );
	void					TouchResource( memhandle_t handle );
	void					MarkAsStale( memhandle_t handle );		// move to tail of LRU

	int						LockCount( memhandle_t handle );
	int						BreakLock( memhandle_t handle );
	int						BreakAllLocks();

	// HACKHACK: For convenience - offers no lock protection 
	// type-safe implementation in derived class
	//void					*GetResource_NoLock( memhandle_t handle );

	unsigned int			TargetSize();
	unsigned int			AvailableSize();
	unsigned int			UsedSize();

	void					SetTargetSize( unsigned int targetSize );

	// NOTE: flush is equivalent to Destroy
	void					FlushAllUnlocked();
	void					FlushToTargetSize();
	void					FlushAll();
	// -----------------------------------------------------------------------------

	// Debugging only!!!!
	void					GetLRUHandleList( CUtlVector< memhandle_t >& list );


protected:
	// derived class must call these to implement public API
	unsigned short			CreateHandle();
	memhandle_t				StoreResourceInHandle( unsigned short memoryIndex, void *pStore, unsigned int realSize );
	void					EnsureCapacity( unsigned int size );
	void					*GetResource_NoLock( memhandle_t handle );
	void					*GetResource_NoLockNoLRUTouch( memhandle_t handle );
	void					*LockResource( memhandle_t handle );

	// NOTE: you must call this from the destructor of the derived class! (will assert otherwise)
	void					FreeAllLists();

							CResourceLRUBase( unsigned int maxSize );
	virtual					~CResourceLRUBase();
	
	
	inline unsigned int		MemTotal_Inline() const { return m_targetMemorySize; }
	inline unsigned int		MemAvailable_Inline() const { return m_targetMemorySize - m_memUsed; }
	inline unsigned int		MemUsed_Inline() const { return m_memUsed; }

private:
// Implemented by derived class:
	virtual void			DestroyResourceStorage( void * ) = 0;
	virtual unsigned int	GetRealSize( void * ) = 0;


private:
	memhandle_t				ToHandle( unsigned short index );
	unsigned short			FromHandle( memhandle_t handle );
	bool					FreeLRU();
	
	void					*LockByIndex( unsigned short memoryIndex );
	int						UnlockByIndex( unsigned short memoryIndex );
	void					TouchByIndex( unsigned short memoryIndex );
	void					MarkAsStaleByIndex( unsigned short memoryIndex );
	void					FreeByIndex( unsigned short memoryIndex );

	// One of these is stored per active allocation
	struct resource_lru_element_t
	{
		resource_lru_element_t()
		{
			lockCount = 0;
			serial = 0;
			pStore = 0;
		}

		unsigned short lockCount;
		unsigned short serial;
		void	*pStore;
	};

	unsigned int m_targetMemorySize;
	unsigned int m_memUsed;
	
	CUtlMultiList< resource_lru_element_t, unsigned short >  m_memoryLists;
	
	unsigned short m_lruList;
	unsigned short m_lockList;
	unsigned short m_freeList;
	unsigned short m_listsAreFreed : 1;
	unsigned short m_unused : 15;

};

template< class STORAGE_TYPE, class CREATE_PARAMS, class LOCK_TYPE = STORAGE_TYPE * >
class CResourceManager : public CResourceLRUBase
{
	typedef CResourceLRUBase BaseClass;
public:

	CResourceManager<STORAGE_TYPE, CREATE_PARAMS, LOCK_TYPE>( unsigned int size ) : BaseClass(size) {}
	

	~CResourceManager<STORAGE_TYPE, CREATE_PARAMS, LOCK_TYPE>()
	{
		// NOTE: This must be called in all implementations of CResourceLRUBase
		FreeAllLists();
	}

	// Use GetData() to translate pointer to LOCK_TYPE
	LOCK_TYPE LockResource( memhandle_t hMem )
	{
		void *pLock = BaseClass::LockResource( hMem );
		if ( pLock )
		{
			return StoragePointer(pLock)->GetData();
		}

		return NULL;
	}

	// Use GetData() to translate pointer to LOCK_TYPE
	LOCK_TYPE GetResource_NoLock( memhandle_t hMem )
	{
		void *pLock = const_cast<void *>(BaseClass::GetResource_NoLock( hMem ));
		if ( pLock )
		{
			return StoragePointer(pLock)->GetData();
		}
		return NULL;
	}

	// Use GetData() to translate pointer to LOCK_TYPE
	// Doesn't touch the memory LRU
	LOCK_TYPE GetResource_NoLockNoLRUTouch( memhandle_t hMem )
	{
		void *pLock = const_cast<void *>(BaseClass::GetResource_NoLockNoLRUTouch( hMem ));
		if ( pLock )
		{
			return StoragePointer(pLock)->GetData();
		}
		return NULL;
	}

	// Wrapper to match implementation of allocation with typed storage & alloc params.
	memhandle_t CreateResource( const CREATE_PARAMS &createParams )
	{
		BaseClass::EnsureCapacity(STORAGE_TYPE::EstimatedSize(createParams));
		unsigned short memoryIndex = BaseClass::CreateHandle();
		STORAGE_TYPE *pStore = STORAGE_TYPE::CreateResource( createParams );
		return BaseClass::StoreResourceInHandle( memoryIndex, pStore, pStore->Size() );
	}

private:
	STORAGE_TYPE *StoragePointer( void *pMem )
	{
		return static_cast<STORAGE_TYPE *>(pMem);
	}

	virtual void DestroyResourceStorage( void *pStore )
	{
		StoragePointer(pStore)->DestroyResource();
	}
	
	virtual unsigned int GetRealSize( void *pStore )
	{
		return StoragePointer(pStore)->Size();
	}
};

//-----------------------------------------------------------------------------

inline unsigned short CResourceLRUBase::FromHandle( memhandle_t handle )
{
	unsigned int fullWord = (unsigned int)handle;
	unsigned short serial = fullWord>>16;
	unsigned short index = fullWord & 0xFFFF;
	index--;
	if ( m_memoryLists.IsValidIndex(index) && m_memoryLists[index].serial == serial )
		return index;
	return m_memoryLists.InvalidIndex();
}

inline int CResourceLRUBase::LockCount( memhandle_t handle )
{
	unsigned short memoryIndex = FromHandle(handle);
	if ( memoryIndex != m_memoryLists.InvalidIndex() )
	{
		return m_memoryLists[memoryIndex].lockCount;
	}
	return 0;
}


#endif // RESOURCEMANAGER_H
