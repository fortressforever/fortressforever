//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
// A stack based on a growable array
//=============================================================================//

#ifndef UTLSTACK_H
#define UTLSTACK_H

#include <assert.h>
#include <string.h>
#include "utlmemory.h"


//-----------------------------------------------------------------------------
// The CUtlStack class:
// A growable stack class which doubles in size by default.
// It will always keep all elements consecutive in memory, and may move the
// elements around in memory (via a realloc) when elements are pushed or
// popped. Clients should therefore refer to the elements of the stack
// by index (they should *never* maintain pointers to elements in the stack).
//-----------------------------------------------------------------------------

template< class T >
class CUtlStack
{
public:
	// constructor, destructor
	CUtlStack( int growSize = 0, int initSize = 0 );
	~CUtlStack();

	// element access
	T& operator[]( int i );
	T const& operator[]( int i ) const;
	T& Element( int i );
	T const& Element( int i ) const;

	// Gets the base address (can change when adding elements!)
	T* Base();
	T const* Base() const;

	// Looks at the stack top
	T& Top();
	T const& Top() const;

	// Size
	int Count() const;

	// Is element index valid?
	bool IsIdxValid( int i ) const;

	// Adds an element, uses default constructor
	int Push();

	// Adds an element, uses copy constructor
	int Push( T const& src );

	// Pops the stack
	void Pop();
	void Pop( T& oldTop );
	void PopMultiple( int num );

	// Makes sure we have enough memory allocated to store a requested # of elements
	void EnsureCapacity( int num );

	// Clears the stack, no deallocation
	void Clear();

	// Memory deallocation
	void Purge();

private:
	// Grows the stack allocation
	void GrowStack();

	// For easier access to the elements through the debugger
	void ResetDbgInfo();

	CUtlMemory<T> m_Memory;
	int m_Size;

	// For easier access to the elements through the debugger
	T* m_pElements;
};


//-----------------------------------------------------------------------------
// For easier access to the elements through the debugger
//-----------------------------------------------------------------------------

template< class T >
inline void CUtlStack<T>::ResetDbgInfo()
{
	m_pElements = m_Memory.Base();
}

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------

template< class T >
CUtlStack<T>::CUtlStack( int growSize, int initSize )	: 
	m_Memory(growSize, initSize), m_Size(0)
{
	ResetDbgInfo();
}

template< class T >
CUtlStack<T>::~CUtlStack()
{
	Purge();
}


//-----------------------------------------------------------------------------
// element access
//-----------------------------------------------------------------------------

template< class T >
inline T& CUtlStack<T>::operator[]( int i )
{
	assert( IsIdxValid(i) );
	return m_Memory[i];
}

template< class T >
inline T const& CUtlStack<T>::operator[]( int i ) const
{
	assert( IsIdxValid(i) );
	return m_Memory[i];
}

template< class T >
inline T& CUtlStack<T>::Element( int i )
{
	assert( IsIdxValid(i) );
	return m_Memory[i];
}

template< class T >
inline T const& CUtlStack<T>::Element( int i ) const
{
	assert( IsIdxValid(i) );
	return m_Memory[i];
}


//-----------------------------------------------------------------------------
// Gets the base address (can change when adding elements!)
//-----------------------------------------------------------------------------

template< class T >
inline T* CUtlStack<T>::Base()
{
	return m_Memory.Base();
}

template< class T >
inline T const* CUtlStack<T>::Base() const
{
	return m_Memory.Base();
}

//-----------------------------------------------------------------------------
// Returns the top of the stack
//-----------------------------------------------------------------------------

template< class T >
inline T& CUtlStack<T>::Top()
{
	assert( m_Size > 0 );
	return Element(m_Size-1);
}

template< class T >
inline T const& CUtlStack<T>::Top() const
{
	assert( m_Size > 0 );
	return Element(m_Size-1);
}

//-----------------------------------------------------------------------------
// Size
//-----------------------------------------------------------------------------

template< class T >
inline int CUtlStack<T>::Count() const
{
	return m_Size;
}


//-----------------------------------------------------------------------------
// Is element index valid?
//-----------------------------------------------------------------------------

template< class T >
inline bool CUtlStack<T>::IsIdxValid( int i ) const
{
	return (i >= 0) && (i < m_Size);
}
 
//-----------------------------------------------------------------------------
// Grows the stack
//-----------------------------------------------------------------------------

template< class T >
void CUtlStack<T>::GrowStack()
{
	if (m_Size >= m_Memory.NumAllocated())
		m_Memory.Grow();

	++m_Size;

	ResetDbgInfo();
}

//-----------------------------------------------------------------------------
// Makes sure we have enough memory allocated to store a requested # of elements
//-----------------------------------------------------------------------------

template< class T >
void CUtlStack<T>::EnsureCapacity( int num )
{
	m_Memory.EnsureCapacity(num);
	ResetDbgInfo();
}


//-----------------------------------------------------------------------------
// Adds an element, uses default constructor
//-----------------------------------------------------------------------------

template< class T >
int CUtlStack<T>::Push()
{
	GrowStack();
	Construct( &Element(m_Size-1) );
	return m_Size - 1;
}

//-----------------------------------------------------------------------------
// Adds an element, uses copy constructor
//-----------------------------------------------------------------------------

template< class T >
int CUtlStack<T>::Push( T const& src )
{
	GrowStack();
	CopyConstruct( &Element(m_Size-1), src );
	return m_Size - 1;
}


//-----------------------------------------------------------------------------
// Pops the stack
//-----------------------------------------------------------------------------

template< class T >
void CUtlStack<T>::Pop()
{
	assert( m_Size > 0 );
	Destruct( &Element(m_Size-1) );
	--m_Size;
}

template< class T >
void CUtlStack<T>::Pop( T& oldTop )
{
	assert( m_Size > 0 );
	oldTop = Top();
	Pop();
}

template< class T >
void CUtlStack<T>::PopMultiple( int num )
{
	assert( m_Size >= num );
	for ( int i = 0; i < num; ++i )
		Destruct( &Element( m_Size - i - 1 ) );
	m_Size -= num;
}


//-----------------------------------------------------------------------------
// Element removal
//-----------------------------------------------------------------------------

template< class T >
void CUtlStack<T>::Clear()
{
	for (int i = m_Size; --i >= 0; )
		Destruct(&Element(i));

	m_Size = 0;
}


//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------

template< class T >
void CUtlStack<T>::Purge()
{
	Clear();
	m_Memory.Purge( );
	ResetDbgInfo();
}

#endif // UTLSTACK_H