//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef DISP_VERTINDEX_H
#define DISP_VERTINDEX_H
#ifdef _WIN32
#pragma once
#endif


#include "tier0/dbg.h"


// ------------------------------------------------------------------------ //
// Helper class used for indexing vertices in the 2D grid.
// ------------------------------------------------------------------------ //

class CVertIndex
{
public:
				CVertIndex();
				CVertIndex( int ix, int iy );
	
	void		Init( int ix, int iy );

	int&		operator[]( int i );
	int const&	operator[]( int i ) const;
	void		operator+=( CVertIndex const &other );
	void		operator-=( CVertIndex const &other );
	CVertIndex	operator+( CVertIndex const &other ) const;
	CVertIndex	operator-( CVertIndex const &other ) const;
	void		operator<<=( int shift );
	void		operator>>=( int shift );
	bool		operator==( CVertIndex const &other ) const;
	bool		operator!=( CVertIndex const &other ) const;


public:

	int			x, y;
};


// ------------------------------------------------------------------ //
// Helper functions.
// ------------------------------------------------------------------ //

inline CVertIndex BuildOffsetVertIndex(
	CVertIndex const &nodeIndex,
	CVertIndex const &offset,
	int mul )
{
	return CVertIndex( nodeIndex.x + offset.x * mul, nodeIndex.y + offset.y * mul );
}


// ------------------------------------------------------------------ //
// CVertIndex inlines.
// ------------------------------------------------------------------ //

inline CVertIndex::CVertIndex()
{
}


inline CVertIndex::CVertIndex( int ix, int iy )
{
	x = ix;
	y = iy;
}


inline void CVertIndex::Init( int ix, int iy )
{
	x = ix;
	y = iy;
}


inline int& CVertIndex::operator[]( int i )
{
	Assert( i >= 0 && i <= 1 );
	return ((int*)this)[i];
}


inline int const& CVertIndex::operator[]( int i ) const
{
	Assert( i >= 0 && i <= 1 );
	return ((int*)this)[i];
}


inline void CVertIndex::operator+=( CVertIndex const &other )
{
	x += other.x;
	y += other.y;
}


inline void CVertIndex::operator-=( CVertIndex const &other )
{
	x -= other.x;
	y -= other.y;
}


inline CVertIndex CVertIndex::operator+( CVertIndex const &other ) const
{
	return CVertIndex( x + other.x, y + other.y );
}


inline CVertIndex CVertIndex::operator-( CVertIndex const &other ) const
{
	return CVertIndex( x - other.x, y - other.y );
}


inline void CVertIndex::operator<<=( int shift )
{
	x <<= shift;
	y <<= shift;
}


inline void CVertIndex::operator>>=( int shift )
{
	x >>= shift;
	y >>= shift;
}


inline bool CVertIndex::operator==( CVertIndex const &other ) const
{
	return x==other.x && y==other.y;
}


inline bool CVertIndex::operator!=( CVertIndex const &other ) const
{
	return x!=other.x || y!=other.y;
}


#endif // DISP_VERTINDEX_H
