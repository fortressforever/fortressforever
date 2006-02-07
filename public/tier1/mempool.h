//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef MEMPOOL_H
#define MEMPOOL_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/memalloc.h"
#include "tier0/platform.h"

//-----------------------------------------------------------------------------
// Purpose: Optimized pool memory allocator
//-----------------------------------------------------------------------------

typedef void (*MemoryPoolReportFunc_t)( char const* pMsg, ... );

class CMemoryPool
{
public:
	// Ways the memory pool can grow when it needs to make a new blob.
	enum
	{
		GROW_NONE=0,		// Don't allow new blobs.
		GROW_FAST=1,		// New blob size is numElements * (i+1)  (ie: the blocks it allocates
							// get larger and larger each time it allocates one).
		GROW_SLOW=2			// New blob size is numElements.
	};

				CMemoryPool(int blockSize, int numElements, int growMode = GROW_FAST, const char *pszAllocOwner = NULL);
				~CMemoryPool();

	void*		Alloc();	// Allocate the element size you specified in the constructor.
	void*		Alloc( unsigned int amount );
	void*		AllocZero();	// Allocate the element size you specified in the constructor, zero the memory before construction
	void*		AllocZero( unsigned int amount );
	void		Free(void *pMem);
	
	// Frees everything
	void		Clear();

	// Error reporting... 
	static void SetErrorReportFunc( MemoryPoolReportFunc_t func );

	// returns number of allocated blocks
	int Count() { return m_BlocksAllocated; }

private:
	class CBlob
	{
	public:
		CBlob	*m_pPrev, *m_pNext;
		int		m_NumBytes;		// Number of bytes in this blob.
		char	m_Data[1];
	};


	// Resets the pool
	void		Init();
	void		AddNewBlob();
	void		ReportLeaks();


private:

	int			m_BlockSize;
	int			m_BlocksPerBlob;

	int			m_GrowMode;	// GROW_ enum.

	// FIXME: Change m_ppMemBlob into a growable array?
	CBlob			m_BlobHead;
	void			*m_pHeadOfFreeList;
	int				m_BlocksAllocated;
	int				m_PeakAlloc;
	unsigned short	m_NumBlobs;
	const char *	m_pszAllocOwner;

	static MemoryPoolReportFunc_t g_ReportFunc;
};


//-----------------------------------------------------------------------------
// Wrapper macro to make an allocator that returns particular typed allocations
// and construction and destruction of objects.
//-----------------------------------------------------------------------------
template< class T >
class CClassMemoryPool : public CMemoryPool
{
public:
	CClassMemoryPool(int numElements, int growMode = GROW_FAST)	:
		CMemoryPool( sizeof(T), numElements, growMode, MEM_ALLOC_CLASSNAME(T) ) {}

	T*		Alloc();
	T*		AllocZero();
	void	Free( T *pMem );
};


template< class T >
T* CClassMemoryPool<T>::Alloc()
{
	T *pRet;

	{
	MEM_ALLOC_CREDIT_(MEM_ALLOC_CLASSNAME(T));
	pRet = (T*)CMemoryPool::Alloc();
	}

	if ( pRet )
	{
		Construct( pRet );
	}
	return pRet;
}

template< class T >
T* CClassMemoryPool<T>::AllocZero()
{
	T *pRet;

	{
	MEM_ALLOC_CREDIT_(MEM_ALLOC_CLASSNAME(T));
	pRet = (T*)CMemoryPool::AllocZero();
	}

	if ( pRet )
	{
		Construct( pRet );
	}
	return pRet;
}

template< class T >
void CClassMemoryPool<T>::Free(T *pMem)
{
	if ( pMem )
	{
		Destruct( pMem );
	}

	CMemoryPool::Free( pMem );
}


//-----------------------------------------------------------------------------
// Macros that make it simple to make a class use a fixed-size allocator
// Put DECLARE_FIXEDSIZE_ALLOCATOR in the private section of a class,
// Put DEFINE_FIXEDSIZE_ALLOCATOR in the CPP file
//-----------------------------------------------------------------------------
#define DECLARE_FIXEDSIZE_ALLOCATOR( _class )									\
   public:																		\
      inline void* operator new( size_t size ) { MEM_ALLOC_CREDIT_(#_class " pool"); return s_Allocator.Alloc(size); }   \
      inline void* operator new( size_t size, int nBlockUse, const char *pFileName, int nLine ) { MEM_ALLOC_CREDIT_(#_class " pool"); return s_Allocator.Alloc(size); }   \
      inline void  operator delete( void* p ) { s_Allocator.Free(p); }		\
      inline void  operator delete( void* p, int nBlockUse, const char *pFileName, int nLine ) { s_Allocator.Free(p); }   \
  private:																		\
      static   CMemoryPool   s_Allocator
    
#define DEFINE_FIXEDSIZE_ALLOCATOR( _class, _initsize, _grow )					\
   CMemoryPool   _class::s_Allocator(sizeof(_class), _initsize, _grow, #_class " pool")


//-----------------------------------------------------------------------------
// Macros that make it simple to make a class use a fixed-size allocator
// This version allows us to use a memory pool which is externally defined...
// Put DECLARE_FIXEDSIZE_ALLOCATOR_EXTERNAL in the private section of a class,
// Put DEFINE_FIXEDSIZE_ALLOCATOR_EXTERNAL in the CPP file
//-----------------------------------------------------------------------------

#define DECLARE_FIXEDSIZE_ALLOCATOR_EXTERNAL( _class, _allocator )				\
   public:																		\
      inline void* operator new( size_t size )  { MEM_ALLOC_CREDIT_(#_class " pool"); return s_pAllocator->Alloc(size); }   \
      inline void* operator new( size_t size, int nBlockUse, const char *pFileName, int nLine )  { MEM_ALLOC_CREDIT_(#_class " pool"); return s_pAllocator->Alloc(size); }   \
      inline void  operator delete( void* p )   { s_pAllocator->Free(p); }		\
   private:																		\
      static   CMemoryPool*   s_pAllocator
    
#define DEFINE_FIXEDSIZE_ALLOCATOR_EXTERNAL( _class, _allocator )				\
   CMemoryPool*   _class::s_pAllocator = _allocator

#endif // MEMPOOL_H
