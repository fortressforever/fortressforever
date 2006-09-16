// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_string.cpp
// @author Patrick O'Leary (Mulchman) 
// @date 9/16/2006
// @brief Simple string class
//
// REVISIONS
// ---------
//	9/16/2006, Mulchman: 
//		First created

// NOTE: keeps crashing the game when i make the size adjustable
// so for now it's fixed as people on irc want to play and tables
// aren't matching.

#include "cbase.h"
#include "ff_string.h"

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFString::CFFString( void )
{
	//m_pszString = NULL;
	m_pszString[ 0 ] = '\0';
	m_iSize = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFString::CFFString( const char *pszString )
{
	*this = pszString;
}

//-----------------------------------------------------------------------------
// Purpose: Copy constructor
//-----------------------------------------------------------------------------
CFFString::CFFString( const CFFString& hRHS )
{
	*this = hRHS;
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
CFFString::~CFFString( void )
{
	Cleanup();
}

//-----------------------------------------------------------------------------
// Purpose: operator =
//-----------------------------------------------------------------------------
CFFString &CFFString::operator=( const CFFString& hRHS )
{
	Q_snprintf( m_pszString, sizeof( m_pszString ), "%s", hRHS.m_pszString );
	/*
	// Deallocate
	Cleanup();

	m_iSize = hRHS.m_iSize;

	if( m_iSize == 0 )
		return *this;

	// Reallocate
	m_pszString = new char[ m_iSize + 1 ];

	Assert( m_pszString );

	// Copy values
	for( int i = 0; i < m_iSize; i++ )
		m_pszString[ i ] = hRHS.m_pszString[ i ];

	// NULL terminate
	m_pszString[ m_iSize ] = '\0';
	*/

	return *this;
}

//-----------------------------------------------------------------------------
// Purpose: operator =
//-----------------------------------------------------------------------------
CFFString &CFFString::operator=( const char *pszString )
{
	Q_snprintf( m_pszString, sizeof( m_pszString ), "%s", pszString );
	/*
	// Deallocate
	Cleanup();

	m_iSize = Q_strlen( pszString );

	if( m_iSize == 0 )
		return *this;

	// Reallocate
	m_pszString = new char[ m_iSize + 1 ];

	Assert( m_pszString );

	// Copy values
	for( int i = 0; i < m_iSize; i++ )
		m_pszString[ i ] = pszString[ i ];

	// NULL terminate
	m_pszString[ m_iSize ] = '\0';
	*/

	return *this;
}

//-----------------------------------------------------------------------------
// Purpose: operator ==
//-----------------------------------------------------------------------------
bool CFFString::operator==( const CFFString& hRHS ) const
{
	return Q_strcmp( m_pszString, hRHS.m_pszString ) == 0;
}

//-----------------------------------------------------------------------------
// Purpose: operator ==
//-----------------------------------------------------------------------------
bool CFFString::operator==( const char *pszString ) const
{
	return Q_strcmp( m_pszString, pszString ) == 0;
}

//-----------------------------------------------------------------------------
// Purpose: operator !=
//-----------------------------------------------------------------------------
bool CFFString::operator!=( const CFFString& hRHS ) const
{
	return Q_strcmp( m_pszString, hRHS.m_pszString ) != 0;
}

//-----------------------------------------------------------------------------
// Purpose: operator !=
//-----------------------------------------------------------------------------
bool CFFString::operator!=( const char *pszString ) const
{
	return Q_strcmp( m_pszString, pszString ) != 0;
}

//-----------------------------------------------------------------------------
// Purpose: operator <
//-----------------------------------------------------------------------------
bool CFFString::operator<( const CFFString& hRHS ) const
{
	return Q_strcmp( m_pszString, hRHS.m_pszString ) < 0;
}

//-----------------------------------------------------------------------------
// Purpose: operator <
//-----------------------------------------------------------------------------
bool CFFString::operator<( const char *pszString ) const
{
	return Q_strcmp( m_pszString, pszString ) < 0;
}

//-----------------------------------------------------------------------------
// Purpose: operator >
//-----------------------------------------------------------------------------
bool CFFString::operator>( const CFFString& hRHS ) const
{
	return Q_strcmp( m_pszString, hRHS.m_pszString ) > 0;
}

//-----------------------------------------------------------------------------
// Purpose: operator >
//-----------------------------------------------------------------------------
bool CFFString::operator>( const char *pszString ) const
{
	return Q_strcmp( m_pszString, pszString ) > 0;
}

//-----------------------------------------------------------------------------
// Purpose: operator <=
//-----------------------------------------------------------------------------
bool CFFString::operator<=( const CFFString& hRHS ) const
{
	return Q_strcmp( m_pszString, hRHS.m_pszString ) <= 0;
}

//-----------------------------------------------------------------------------
// Purpose: operator <=
//-----------------------------------------------------------------------------
bool CFFString::operator<=( const char *pszString ) const
{
	return Q_strcmp( m_pszString, pszString ) <= 0;
}

//-----------------------------------------------------------------------------
// Purpose: operator >=
//-----------------------------------------------------------------------------
bool CFFString::operator>=( const CFFString& hRHS ) const
{
	return Q_strcmp( m_pszString, hRHS.m_pszString ) >= 0;
}

//-----------------------------------------------------------------------------
// Purpose: operator >=
//-----------------------------------------------------------------------------
bool CFFString::operator>=( const char *pszString ) const
{
	return Q_strcmp( m_pszString, pszString ) >= 0;
}

//-----------------------------------------------------------------------------
// Purpose: Deallocate memory and reset
//-----------------------------------------------------------------------------
void CFFString::Cleanup( void )
{
	/*
	if( m_pszString )
	{
		delete [] m_pszString;
		m_pszString = NULL;		
	}*/
	m_pszString[ 0 ] = '\0';

	m_iSize = 0;
}
