//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef UTLSORTVECTOR_H
#define UTLSORTVECTOR_H

#ifdef _WIN32
#pragma once
#endif

#include "utlvector.h"


//-----------------------------------------------------------------------------
// class CUtlSortVector:
// description:
//   This in an sorted order-preserving vector. Items may be inserted or removed
//   at any point in the vector. When an item is inserted, all elements are
//   moved down by one element using memmove. When an item is removed, all 
//   elements are shifted back down. Items are searched for in the vector
//   using a binary search technique. Clients must pass in a Less() function
//   into the constructor of the vector to determine the sort order.
//-----------------------------------------------------------------------------

template <class T>
class CUtlSortVector : public CUtlVector<T>
{
public:
	typedef bool (*LessFunc_t)( const T& src1, const T& src2, void *pCtx );
	
	// constructor
	CUtlSortVector( LessFunc_t lessFunc, int nGrowSize = 0, int initSize = 0 );
	CUtlSortVector( LessFunc_t lessFunc, T* pMemory, int numElements );
	
	// inserts (copy constructs) an element in sorted order into the list
	int		Insert( const T& src );
	
	// Finds an element within the list using a binary search
	int		Find( const T& search ) const;
	int		FindLessOrEqual( const T& search ) const;
	
	// Removes a particular element
	void	Remove( const T& search );
	
	// Allows methods to set a context to be used with the less function..
	void	SetLessContext( void *pCtx );

protected:
	// No copy constructor
	CUtlSortVector( const CUtlSortVector<T> & );

	// never call these; illegal for this class
	int AddToHead();
	int AddToTail();
	int InsertBefore( int elem );
	int InsertAfter( int elem );

	// Adds an element, uses copy constructor
	int AddToHead( const T& src );
	int AddToTail( const T& src );
	int InsertBefore( int elem, const T& src );
	int InsertAfter( int elem, const T& src );

	// Adds multiple elements, uses defaulconst Tructor
	int AddMultipleToHead( int num );
	int AddMultipleToTail( int num, const T *pToCopy=NULL );	   
	int InsertMultipleBefore( int elem, int num, const T *pToCopy=NULL );
	int InsertMultipleAfter( int elem, int num );
	
	// Add the specified array to the tail.
	int AddVectorToTail( CUtlVector<T> const &src );

	LessFunc_t   m_Less;
	void *m_pLessContext;
};


//-----------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------
template <class T> 
CUtlSortVector<T>::CUtlSortVector( LessFunc_t lessFunc, int nGrowSize, int initSize ) : 
	m_Less(lessFunc), m_pLessContext(NULL), CUtlVector<T>( nGrowSize, initSize )
{
}

template <class T> 
CUtlSortVector<T>::CUtlSortVector( LessFunc_t lessFunc, T* pMemory, int numElements ) :
	m_Less(lessFunc), m_pLessContext(NULL), CUtlVector<T>( pMemory, numElements )
{
}


//-----------------------------------------------------------------------------
// Allows methods to set a context to be used with the less function..
//-----------------------------------------------------------------------------
template <class T> 
void CUtlSortVector<T>::SetLessContext( void *pCtx )
{
	m_pLessContext = pCtx;
}


//-----------------------------------------------------------------------------
// grows the vector
//-----------------------------------------------------------------------------
template <class T> 
int CUtlSortVector<T>::Insert( const T& src )
{
	int pos = FindLessOrEqual( src ) + 1;
	GrowVector();
	ShiftElementsRight(pos);
	CopyConstruct<T>( &Element(pos), src );
	return pos;
}


//-----------------------------------------------------------------------------
// finds a particular element
//-----------------------------------------------------------------------------
template <class T> 
int CUtlSortVector<T>::Find( const T& src ) const
{
	int start = 0, end = Count() - 1;
	while (start <= end)
	{
		int mid = (start + end) >> 1;
		if ( m_Less( Element(mid), src, m_pLessContext ) )
		{
			start = mid + 1;
		}
		else if ( m_Less( src, Element(mid), m_pLessContext ) )
		{
			end = mid - 1;
		}
		else
		{
			return mid;
		}
	}
	return -1;
}


//-----------------------------------------------------------------------------
// finds a particular element
//-----------------------------------------------------------------------------
template <class T> 
int CUtlSortVector<T>::FindLessOrEqual( const T& src ) const
{
	int start = 0, end = Count() - 1;
	while (start <= end)
	{
		int mid = (start + end) >> 1;
		if ( m_Less( Element(mid), src, m_pLessContext ) )
		{
			start = mid + 1;
		}
		else if ( m_Less( src, Element(mid), m_pLessContext ) )
		{
			end = mid - 1;
		}
		else
		{
			return mid;
		}
	}
	return end;
}


//-----------------------------------------------------------------------------
// Removes a particular element
//-----------------------------------------------------------------------------
template <class T> 
void CUtlSortVector<T>::Remove( const T& search )
{
	int pos = Find(search);
	if (pos != -1)
	{
		Remove(pos);
	}
}


#endif // UTLSORTVECTOR_H
