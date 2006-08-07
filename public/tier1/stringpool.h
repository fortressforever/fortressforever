//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
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
#include "utlvector.h"

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

protected:
	typedef CUtlRBTree<const char *, unsigned short> CStrSet;

	CStrSet m_Strings;
};

//-----------------------------------------------------------------------------
// Purpose: A reference counted string pool.  
//
// Elements are stored more efficiently than in the conventional string pool, 
// quicker to look up, and storage is tracked via reference counts.  
//
// At some point this should replace CStringPool
//-----------------------------------------------------------------------------
class CCountedStringPool
{
public: // HACK, hash_item_t structure should not be public.

#pragma pack(1)
	struct hash_item_t
	{
		char* pString;
		unsigned short nNextElement;
		unsigned char  nReferenceCount;
	};
#pragma pack()

	enum
	{
		INVALID_ELEMENT = 0,
		MAX_REFERENCE = 0xFF,
		HASH_TABLE_SIZE = 1024
	};

	CUtlVector<unsigned short>	m_HashTable;	// Points to each element
	CUtlVector<hash_item_t>		m_Elements;
	unsigned short m_FreeListStart;

public:
	CCountedStringPool();
	virtual ~CCountedStringPool();

	void FreeAll();

	char* FindString( const char* pIntrinsic ); 
	char* ReferenceString( const char* pIntrinsic );
	void  DereferenceString( const char* pIntrinsic );

	// These are only reliable if there are less than 64k strings in your string pool
	unsigned short FindStringHandle( const char* pIntrinsic ); 
	unsigned short ReferenceStringHandle( const char* pIntrinsic );
	char*		   HandleToString( unsigned short handle );


};

extern CCountedStringPool g_CountedStringPool;

#endif // STRINGPOOL_H
