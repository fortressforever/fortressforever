//====== Copyright � 1996-2004, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "tier1/utlstring.h"
#include "vstdlib/strtools.h"


//-----------------------------------------------------------------------------
// Base class, containing simple memory management
//-----------------------------------------------------------------------------
CUtlBinaryBlock::CUtlBinaryBlock( int growSize, int initSize ) : m_Memory( growSize, initSize )
{
	m_nActualLength = 0;
}

CUtlBinaryBlock::CUtlBinaryBlock( void* pMemory, int nSizeInBytes, int nInitialLength ) : m_Memory( (unsigned char*)pMemory, nSizeInBytes )
{
	m_nActualLength = nInitialLength;
}

CUtlBinaryBlock::CUtlBinaryBlock( const void* pMemory, int nSizeInBytes ) : m_Memory( (const unsigned char*)pMemory, nSizeInBytes )
{
	m_nActualLength = nSizeInBytes;
}

CUtlBinaryBlock::CUtlBinaryBlock( const CUtlBinaryBlock& src )
{
	Set( src.Get(), src.Length() );
}

void CUtlBinaryBlock::Get( void *pValue, int nLen ) const
{
	Assert( nLen > 0 );
	if ( m_nActualLength < nLen )
	{
		nLen = m_nActualLength;
	}

	if ( nLen > 0 )
	{
		memcpy( pValue, m_Memory.Base(), nLen );
	}
}

void CUtlBinaryBlock::SetLength( int nLength )
{
	Assert( !m_Memory.IsReadOnly() );

	m_nActualLength = nLength;
	if ( nLength > m_Memory.NumAllocated() )
	{
		int nOverFlow = nLength - m_Memory.NumAllocated();
		m_Memory.Grow( nOverFlow );

		// If the reallocation failed, clamp length
		if ( nLength > m_Memory.NumAllocated() )
		{
			m_nActualLength = m_Memory.NumAllocated();
		}
	}

#ifdef _DEBUG
	if ( m_Memory.NumAllocated() > m_nActualLength )
	{
		memset( ( ( char * )m_Memory.Base() ) + m_nActualLength, 0xEB, m_Memory.NumAllocated() - m_nActualLength );
	}
#endif
}

void CUtlBinaryBlock::Set( const void *pValue, int nLen )
{
	Assert( !m_Memory.IsReadOnly() );

	if ( !pValue )
	{
		nLen = 0;
	}

	SetLength( nLen );

	if ( m_nActualLength )
	{
		if ( ( ( const char * )m_Memory.Base() ) >= ( ( const char * )pValue ) + nLen ||
			 ( ( const char * )m_Memory.Base() ) + m_nActualLength <= ( ( const char * )pValue ) )
		{
			memcpy( m_Memory.Base(), pValue, m_nActualLength );
		}
		else
		{
			memmove( m_Memory.Base(), pValue, m_nActualLength );
		}
	}
}


CUtlBinaryBlock &CUtlBinaryBlock::operator=( const CUtlBinaryBlock &src )
{
	Assert( !m_Memory.IsReadOnly() );
	Set( src.Get(), src.Length() );
	return *this;
}


bool CUtlBinaryBlock::operator==( const CUtlBinaryBlock &src ) const
{
	if ( src.Length() != Length() )
		return false;

	return !memcmp( src.Get(), Get(), Length() );
}


//-----------------------------------------------------------------------------
// Simple string class. 
//-----------------------------------------------------------------------------
CUtlString::CUtlString()
{
}

CUtlString::CUtlString( const char *pString )
{
	Set( pString );
}

CUtlString::CUtlString( const CUtlString& string )
{
	Set( string.Get() );
}

// Attaches the string to external memory. Useful for avoiding a copy
CUtlString::CUtlString( void* pMemory, int nSizeInBytes, int nInitialLength ) : m_Storage( pMemory, nSizeInBytes, nInitialLength )
{
}

CUtlString::CUtlString( const void* pMemory, int nSizeInBytes ) : m_Storage( pMemory, nSizeInBytes )
{
}

void CUtlString::Set( const char *pValue )
{
	Assert( !m_Storage.IsReadOnly() );
	int nLen = pValue ? Q_strlen(pValue) + 1 : 0;
	m_Storage.Set( pValue, nLen );
}

// Returns strlen
int CUtlString::Length() const
{
	return m_Storage.Length() ? m_Storage.Length() - 1 : 0;
}

// Sets the length (used to serialize into the buffer )
void CUtlString::SetLength( int nLen )
{
	Assert( !m_Storage.IsReadOnly() );

	// Add 1 to account for the NULL
	m_Storage.SetLength( nLen > 0 ? nLen + 1 : 0 );
}

const char *CUtlString::Get( ) const
{
	if ( m_Storage.Length() == 0 )
	{
		return "";
	}

	return reinterpret_cast< const char* >( m_Storage.Get() );
}

// Converts to c-strings
CUtlString::operator const char*() const
{
	return Get();
}

char *CUtlString::Get()
{
	Assert( !m_Storage.IsReadOnly() );

	if ( m_Storage.Length() == 0 )
	{
		// In general, we optimise away small mallocs for empty strings
		// but if you ask for the non-const bytes, they must be writable
		// so we can't return "" here, like we do for the const version - jd
		m_Storage.SetLength( 1 );
		m_Storage[ 0 ] = '\0';
	}

	return reinterpret_cast< char* >( m_Storage.Get() );
}

CUtlString &CUtlString::operator=( const CUtlString &src )
{
	Assert( !m_Storage.IsReadOnly() );
	m_Storage = src.m_Storage;
	return *this;
}

CUtlString &CUtlString::operator=( const char *src )
{
	Assert( !m_Storage.IsReadOnly() );
	Set( src );
	return *this;
}

bool CUtlString::operator==( const CUtlString &src ) const
{
	return m_Storage == src.m_Storage;
}

bool CUtlString::operator==( const char *src ) const
{
	return ( strcmp( Get(), src ) == 0 );
}
