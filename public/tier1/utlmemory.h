//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
	//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// $Header: $
// $NoKeywords: $
//
// A growable memory class.
//=============================================================================

#ifndef UTLMEMORY_H
#define UTLMEMORY_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/dbg.h"
#include <string.h>
#include "tier0/platform.h"

#include "tier0/memalloc.h"
#include "tier0/memdbgon.h"

#pragma warning (disable:4100)
#pragma warning (disable:4514)

//-----------------------------------------------------------------------------
// The CUtlMemory class:
// A growable memory class which doubles in size by default.
//-----------------------------------------------------------------------------
template< class T >
class CUtlMemory
{
public:
	// constructor, destructor
	CUtlMemory( int nGrowSize = 0, int nInitSize = 0 );
	CUtlMemory( T* pMemory, int numElements );
	~CUtlMemory();

	// element access
	T& operator[]( int i );
	T const& operator[]( int i ) const;
	T& Element( int i );
	T const& Element( int i ) const;

	// Can we use this index?
	bool IsIdxValid( int i ) const;

	// Gets the base address (can change when adding elements!)
	T* Base();
	T const* Base() const;

	// Attaches the buffer to external memory....
	void SetExternalBuffer( T* pMemory, int numElements );

	// Size
	int NumAllocated() const;
	int Count() const;

	// Grows the memory, so that at least allocated + num elements are allocated
	void Grow( int num = 1 );

	// Makes sure we've got at least this much memory
	void EnsureCapacity( int num );

	// Memory deallocation
	void Purge();

	// is the memory externally allocated?
	bool IsExternallyAllocated() const;

	// Set the size by which the memory grows
	void SetGrowSize( int size );

protected:
	enum
	{
		EXTERNAL_BUFFER_MARKER = -1,
	};

	T* m_pMemory;
	int m_nAllocationCount;
	int m_nGrowSize;
};


//-----------------------------------------------------------------------------
// The CUtlMemoryFixed class:
// A fixed memory class
//-----------------------------------------------------------------------------
template< typename T, size_t SIZE >
class CUtlMemoryFixed
{
public:
	// constructor, destructor
	CUtlMemoryFixed( int nGrowSize = 0, int nInitSize = 0 )	{ Assert( nInitSize == 0 || nInitSize == SIZE ); 	}
	CUtlMemoryFixed( T* pMemory, int numElements )			{ Assert( 0 ); 										}

	// Can we use this index?
	bool IsIdxValid( int i ) const							{ return (i >= 0) && (i < SIZE); }

	// Gets the base address
	T* Base()												{ return (T*)(&m_Memory[0]); }
	const T* Base() const									{ return (T*)(&m_Memory[0]); }

	// element access
	T& operator[]( int i )									{ Assert( IsIdxValid(i) ); return Base()[i];	}
	T const& operator[]( int i ) const						{ Assert( IsIdxValid(i) ); return Base()[i];	}
	T& Element( int i )										{ Assert( IsIdxValid(i) ); return Base()[i];	}
	T const& Element( int i ) const							{ Assert( IsIdxValid(i) ); return Base()[i];	}

	// Attaches the buffer to external memory....
	void SetExternalBuffer( T* pMemory, int numElements )	{ Assert( 0 ); }

	// Size
	int NumAllocated() const								{ return SIZE; }
	int Count() const										{ return SIZE; }

	// Grows the memory, so that at least allocated + num elements are allocated
	void Grow( int num = 1 )								{ Assert( 0 ); }

	// Makes sure we've got at least this much memory
	void EnsureCapacity( int num )							{ Assert( num <= SIZE ); }

	// Memory deallocation
	void Purge()											{}

	// is the memory externally allocated?
	bool IsExternallyAllocated() const						{ return false; }

	// Set the size by which the memory grows
	void SetGrowSize( int size )							{}

private:
	char m_Memory[SIZE*sizeof(T)];
};

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------
template< class T >
CUtlMemory<T>::CUtlMemory( int nGrowSize, int nInitAllocationCount ) : m_pMemory(0), 
	m_nAllocationCount( nInitAllocationCount ), m_nGrowSize( nGrowSize )
{
	Assert( (nGrowSize >= 0) && (nGrowSize != EXTERNAL_BUFFER_MARKER) );
	if (m_nAllocationCount)
	{
		MEM_ALLOC_CREDIT_CLASS();
		m_pMemory = (T*)malloc( m_nAllocationCount * sizeof(T) );
	}
}

template< class T >
CUtlMemory<T>::CUtlMemory( T* pMemory, int numElements ) : m_pMemory(pMemory),
	m_nAllocationCount( numElements )
{
	// Special marker indicating externally supplied memory
	m_nGrowSize = EXTERNAL_BUFFER_MARKER;
}

template< class T >
CUtlMemory<T>::~CUtlMemory()
{
	Purge();
}


//-----------------------------------------------------------------------------
// Attaches the buffer to external memory....
//-----------------------------------------------------------------------------
template< class T >
void CUtlMemory<T>::SetExternalBuffer( T* pMemory, int numElements )
{
	// Blow away any existing allocated memory
	Purge();

	m_pMemory = pMemory;
	m_nAllocationCount = numElements;

	// Indicate that we don't own the memory
	m_nGrowSize = EXTERNAL_BUFFER_MARKER;
}


//-----------------------------------------------------------------------------
// element access
//-----------------------------------------------------------------------------
template< class T >
inline T& CUtlMemory<T>::operator[]( int i )
{
	Assert( IsIdxValid(i) );
	return m_pMemory[i];
}

template< class T >
inline T const& CUtlMemory<T>::operator[]( int i ) const
{
	Assert( IsIdxValid(i) );
	return m_pMemory[i];
}

template< class T >
inline T& CUtlMemory<T>::Element( int i )
{
	Assert( IsIdxValid(i) );
	return m_pMemory[i];
}

template< class T >
inline T const& CUtlMemory<T>::Element( int i ) const
{
	Assert( IsIdxValid(i) );
	return m_pMemory[i];
}


//-----------------------------------------------------------------------------
// is the memory externally allocated?
//-----------------------------------------------------------------------------
template< class T >
bool CUtlMemory<T>::IsExternallyAllocated() const
{
	return m_nGrowSize == EXTERNAL_BUFFER_MARKER;
}


template< class T >
void CUtlMemory<T>::SetGrowSize( int nSize )
{
	Assert( (nSize >= 0) && (nSize != EXTERNAL_BUFFER_MARKER) );
	m_nGrowSize = nSize;
}


//-----------------------------------------------------------------------------
// Gets the base address (can change when adding elements!)
//-----------------------------------------------------------------------------
template< class T >
inline T* CUtlMemory<T>::Base()
{
	return m_pMemory;
}

template< class T >
inline T const* CUtlMemory<T>::Base() const
{
	return m_pMemory;
}


//-----------------------------------------------------------------------------
// Size
//-----------------------------------------------------------------------------
template< class T >
inline int CUtlMemory<T>::NumAllocated() const
{
	return m_nAllocationCount;
}

template< class T >
inline int CUtlMemory<T>::Count() const
{
	return m_nAllocationCount;
}


//-----------------------------------------------------------------------------
// Is element index valid?
//-----------------------------------------------------------------------------
template< class T >
inline bool CUtlMemory<T>::IsIdxValid( int i ) const
{
	return (i >= 0) && (i < m_nAllocationCount);
}
 

//-----------------------------------------------------------------------------
// Grows the memory
//-----------------------------------------------------------------------------
template< class T >
void CUtlMemory<T>::Grow( int num )
{
	Assert( num > 0 );

	if (IsExternallyAllocated())
	{
		// Can't grow a buffer whose memory was externally allocated 
		Assert(0);
		return;
	}

	// Make sure we have at least numallocated + num allocations.
	// Use the grow rules specified for this memory (in m_nGrowSize)
	int nAllocationRequested = m_nAllocationCount + num;
	while (m_nAllocationCount < nAllocationRequested)
	{
		if ( m_nAllocationCount != 0 )
		{
			if (m_nGrowSize)
			{
				m_nAllocationCount += m_nGrowSize;
			}
			else
			{
				m_nAllocationCount += m_nAllocationCount;
			}
		}
		else
		{
			// Compute an allocation which is at least as big as a cache line...
			m_nAllocationCount = (31 + sizeof(T)) / sizeof(T);
			Assert(m_nAllocationCount != 0);
		}
	}


	if (m_pMemory)
	{
		MEM_ALLOC_CREDIT_CLASS();
		m_pMemory = (T*)realloc( m_pMemory, m_nAllocationCount * sizeof(T) );
		Assert( m_pMemory );
	}
	else
	{
		MEM_ALLOC_CREDIT_CLASS();
		m_pMemory = (T*)malloc( m_nAllocationCount * sizeof(T) );
		Assert( m_pMemory );
	}
}


//-----------------------------------------------------------------------------
// Makes sure we've got at least this much memory
//-----------------------------------------------------------------------------
template< class T >
inline void CUtlMemory<T>::EnsureCapacity( int num )
{
	if (m_nAllocationCount >= num)
		return;

	if (IsExternallyAllocated())
	{
		// Can't grow a buffer whose memory was externally allocated 
		Assert(0);
		return;
	}

	m_nAllocationCount = num;
	if (m_pMemory)
	{
		MEM_ALLOC_CREDIT_CLASS();
		m_pMemory = (T*)realloc( m_pMemory, m_nAllocationCount * sizeof(T) );
	}
	else
	{
		MEM_ALLOC_CREDIT_CLASS();
		m_pMemory = (T*)malloc( m_nAllocationCount * sizeof(T) );
	}
}


//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------
template< class T >
void CUtlMemory<T>::Purge()
{
	if (!IsExternallyAllocated())
	{
		if (m_pMemory)
		{
			free( (void*)m_pMemory );
			m_pMemory = 0;
		}
		m_nAllocationCount = 0;
	}
}


//-----------------------------------------------------------------------------
// The CUtlMemory class:
// A growable memory class which doubles in size by default.
//-----------------------------------------------------------------------------
template< class T, int nAlignment >
class CUtlMemoryAligned	: public CUtlMemory<T>
{
public:
	// constructor, destructor
	CUtlMemoryAligned( int nGrowSize = 0, int nInitSize = 0 );
	CUtlMemoryAligned( T* pMemory, int numElements );
	~CUtlMemoryAligned();

	// Attaches the buffer to external memory....
	void SetExternalBuffer( T* pMemory, int numElements );

	// Grows the memory, so that at least allocated + num elements are allocated
	void Grow( int num = 1 );

	// Makes sure we've got at least this much memory
	void EnsureCapacity( int num );

	// Memory deallocation
	void Purge();

private:
	void *Align( void *pAddr );

	void* m_pMemoryBase;
};


//-----------------------------------------------------------------------------
// Aligns a pointer
//-----------------------------------------------------------------------------
template< class T, int nAlignment >
void *CUtlMemoryAligned<T, nAlignment>::Align( void *pAddr )
{
	int nAlignmentMask = nAlignment - 1;
	return (void*)( ((int)pAddr + nAlignmentMask) & (~nAlignmentMask) );
}


//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------
template< class T, int nAlignment >
CUtlMemoryAligned<T, nAlignment>::CUtlMemoryAligned( int nGrowSize, int nInitAllocationCount ) : m_pMemoryBase(0)
{
	CUtlMemory<T>::m_pMemory = 0; 
	CUtlMemory<T>::m_nAllocationCount = nInitAllocationCount;
	CUtlMemory<T>::m_nGrowSize = nGrowSize;

	// Alignment must be a power of two
	COMPILE_TIME_ASSERT( (nAlignment & (nAlignment-1)) == 0 );
	Assert( (nGrowSize >= 0) && (nGrowSize != EXTERNAL_BUFFER_MARKER) );
	if ( CUtlMemory<T>::m_nAllocationCount )
	{
		MEM_ALLOC_CREDIT_CLASS();
		m_pMemoryBase = malloc( nInitAllocationCount * sizeof(T) + (nAlignment - 1) );
		CUtlMemory<T>::m_pMemory = (T*)Align( m_pMemoryBase );
	}
}

template< class T, int nAlignment >
CUtlMemoryAligned<T, nAlignment>::CUtlMemoryAligned( T* pMemory, int numElements ) : m_pMemoryBase(pMemory)
{
	// Special marker indicating externally supplied memory
	CUtlMemory<T>::m_nGrowSize = CUtlMemory<T>::EXTERNAL_BUFFER_MARKER;

	CUtlMemory<T>::m_pMemory = (T*)Align( pMemory );
	CUtlMemory<T>::m_nAllocationCount = ( (int)(pMemory + numElements) - (int)CUtlMemory<T>::m_pMemory ) / sizeof(T);
}

template< class T, int nAlignment >
CUtlMemoryAligned<T, nAlignment>::~CUtlMemoryAligned()
{
	Purge();
}


//-----------------------------------------------------------------------------
// Attaches the buffer to external memory....
//-----------------------------------------------------------------------------
template< class T, int nAlignment >
void CUtlMemoryAligned<T, nAlignment>::SetExternalBuffer( T* pMemory, int numElements )
{
	// Blow away any existing allocated memory
	Purge();

	m_pMemoryBase = pMemory;
	CUtlMemory<T>::m_pMemory = (T*)Align( pMemory );
	CUtlMemory<T>::m_nAllocationCount = ( (int)(pMemory + numElements) - (int)CUtlMemory<T>::m_pMemory ) / sizeof(T);

	// Indicate that we don't own the memory
	CUtlMemory<T>::m_nGrowSize = CUtlMemory<T>::EXTERNAL_BUFFER_MARKER;
}


//-----------------------------------------------------------------------------
// Grows the memory
//-----------------------------------------------------------------------------
template< class T, int nAlignment >
void CUtlMemoryAligned<T, nAlignment>::Grow( int num )
{
	Assert( num > 0 );

	if (IsExternallyAllocated())
	{
		// Can't grow a buffer whose memory was externally allocated 
		Assert(0);
		return;
	}

	// Make sure we have at least numallocated + num allocations.
	// Use the grow rules specified for this memory (in m_nGrowSize)
	int nAllocationRequested = CUtlMemory<T>::m_nAllocationCount + num;
	while (CUtlMemory<T>::m_nAllocationCount < nAllocationRequested)
	{
		if ( CUtlMemory<T>::m_nAllocationCount != 0 )
		{
			if (CUtlMemory<T>::m_nGrowSize)
			{
				CUtlMemory<T>::m_nAllocationCount += CUtlMemory<T>::m_nGrowSize;
			}
			else
			{
				CUtlMemory<T>::m_nAllocationCount += CUtlMemory<T>::m_nAllocationCount;
			}
		}
		else
		{
			// Compute an allocation which is at least as big as a cache line...
			CUtlMemory<T>::m_nAllocationCount = (31 + sizeof(T)) / sizeof(T);
			Assert(CUtlMemory<T>::m_nAllocationCount != 0);
		}
	}

	if (m_pMemoryBase)
	{
		MEM_ALLOC_CREDIT_CLASS();
		m_pMemoryBase = realloc( m_pMemoryBase, CUtlMemory<T>::m_nAllocationCount * sizeof(T) + (nAlignment - 1) );
		Assert( m_pMemoryBase );
	}
	else
	{
		MEM_ALLOC_CREDIT_CLASS();
		m_pMemoryBase = malloc( CUtlMemory<T>::m_nAllocationCount * sizeof(T) + (nAlignment - 1) );
		Assert( m_pMemoryBase );
	}
	CUtlMemory<T>::m_pMemory = (T*)Align( m_pMemoryBase );
}


//-----------------------------------------------------------------------------
// Makes sure we've got at least this much memory
//-----------------------------------------------------------------------------
template< class T, int nAlignment >
inline void CUtlMemoryAligned<T, nAlignment>::EnsureCapacity( int num )
{
	if (CUtlMemory<T>::m_nAllocationCount >= num)
		return;

	if (IsExternallyAllocated())
	{
		// Can't grow a buffer whose memory was externally allocated 
		Assert(0);
		return;
	}

	CUtlMemory<T>::m_nAllocationCount = num;
	if (m_pMemoryBase)
	{
		MEM_ALLOC_CREDIT_CLASS();
		m_pMemoryBase = realloc( m_pMemoryBase, CUtlMemory<T>::m_nAllocationCount * sizeof(T) + (nAlignment - 1) );
	}
	else
	{
		MEM_ALLOC_CREDIT_CLASS();
		m_pMemoryBase = malloc( CUtlMemory<T>::m_nAllocationCount * sizeof(T) + (nAlignment - 1) );
	}
	CUtlMemory<T>::m_pMemory = (T*)Align( m_pMemoryBase );
}


//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------
template< class T, int nAlignment >
void CUtlMemoryAligned<T, nAlignment>::Purge()
{
	if (!IsExternallyAllocated())
	{
		if (m_pMemoryBase)
		{
			free( (void*)m_pMemoryBase );
			m_pMemoryBase = 0;
			CUtlMemory<T>::m_pMemory = 0;
		}
		CUtlMemory<T>::m_nAllocationCount = 0;
	}
}

#include "tier0/memdbgoff.h"

#endif // UTLMEMORY_H
