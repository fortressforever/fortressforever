//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef STRINGPOOL_H
#define STRINGPOOL_H

#if defined( _WIN32 )
#pragma once
#endif

#include "utlrbtree.h"

//-----------------------------------------------------------------------------
// Purpose: Allocates memory for strings, checking for duplicates first,
//			reusing exising strings if duplicate found.
//-----------------------------------------------------------------------------

class CStringPool
{
public:
	CStringPool();
	~CStringPool();

	unsigned int Count() const;

	const char * Allocate( const char *pszValue );
	void FreeAll();

	// searches for a string already in the pool
	const char * CStringPool::Find( const char *pszValue );

private:
	typedef CUtlRBTree<const char *, unsigned short> CStrSet;

	CStrSet m_Strings;
};

#endif // STRINGPOOL_H
