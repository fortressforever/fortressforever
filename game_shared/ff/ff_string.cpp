// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_string.cpp
// @author Patrick O'Leary (Mulchman) 
// @date 9/16/2006
// @brief Simple string class to wrap char*'s. Don't
//			care too much about speed, either!
//
// REVISIONS
// ---------
//	9/16/2006, Mulchman: 
//		First created

#include "cbase.h"
#include "ff_string.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFString::CFFString( void )
{
	m_pszString = NULL;
	m_iSize = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFString::CFFString( const char *pszString )
{
	m_pszString = NULL;
	m_iSize = 0;

	*this = pszString;
}

//-----------------------------------------------------------------------------
// Purpose: Copy constructor
//-----------------------------------------------------------------------------
CFFString::CFFString( const CFFString& hRHS )
{
	m_pszString = NULL;
	m_iSize = 0;

	*this = hRHS;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
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
	if( this == &hRHS )
		return *this;

	// Deallocate
	Cleanup();

	m_iSize = hRHS.m_iSize;

	// Reallocate
	m_pszString = new char[ m_iSize + 1 ];

	Assert( m_pszString );

	// Copy values
	for( int i = 0; i < m_iSize; i++ )
		m_pszString[ i ] = hRHS.m_pszString[ i ];

	// NULL terminate
	m_pszString[ m_iSize ] = '\0';
	
	return *this;
}

//-----------------------------------------------------------------------------
// Purpose: operator =
//-----------------------------------------------------------------------------
CFFString &CFFString::operator=( const char *pszString )
{
	// Deallocate
	Cleanup();

	m_iSize = Q_strlen( pszString );

	// Reallocate
	m_pszString = new char[ m_iSize + 1 ];

	Assert( m_pszString );

	// Copy values
	for( int i = 0; i < m_iSize; i++ )
		m_pszString[ i ] = pszString[ i ];

	// NULL terminate
	m_pszString[ m_iSize ] = '\0';
	
	return *this;
}

//-----------------------------------------------------------------------------
// Purpose: operator +=
//-----------------------------------------------------------------------------
CFFString &CFFString::operator+=( const CFFString& hRHS )
{
	// Allocate buffer big enough for both strings
	char *pTemp = new char[ m_iSize + hRHS.m_iSize + 1 ];

	Assert( pTemp );

	// Copy over current string
	for( int i = 0; i < m_iSize; i++ )
		pTemp[i] = m_pszString[i];

	// Copy over new string...
	for( int i = 0; i < hRHS.m_iSize; i++ )
		pTemp[i + m_iSize] = hRHS.m_pszString[i];

	// NULL terminate
	pTemp[m_iSize + hRHS.m_iSize] = '\0';

	int iOldSize = m_iSize;

	// Release old stuff
	Cleanup();

	// Update
	m_iSize = iOldSize + hRHS.m_iSize;	

	// Re-assign pointer
	m_pszString = pTemp;

	return *this;
}

//-----------------------------------------------------------------------------
// Purpose: operator +=
//-----------------------------------------------------------------------------
CFFString &CFFString::operator+=( const char *pszString )
{
	int iSize = Q_strlen( pszString );

	// Allocate buffer big enough for both strings
	char *pTemp = new char[ m_iSize + iSize + 1 ];

	Assert( pTemp );

	// Copy over current string
	for( int i = 0; i < m_iSize; i++ )
		pTemp[i] = m_pszString[i];

	// Copy over new string...
	for( int i = 0; i < iSize; i++ )
		pTemp[i + m_iSize] = pszString[i];

	// NULL terminate
	pTemp[m_iSize + iSize] = '\0';

	int iOldSize = m_iSize;

	// Release old stuff
	Cleanup();

	m_iSize = iOldSize + iSize;

	// Re-assign pointer
	m_pszString = pTemp;

	return *this;
}

//-----------------------------------------------------------------------------
// Purpose: operator +=
//-----------------------------------------------------------------------------
CFFString &CFFString::operator+=( const double& hRHS )
{
	char szBuf[64];
	Q_snprintf( szBuf, 64, "%f", hRHS );

	this->operator+=( szBuf );

	return *this;
}

//-----------------------------------------------------------------------------
// Purpose: operator +=
//-----------------------------------------------------------------------------
CFFString &CFFString::operator+=( const float& hRHS )
{
	char szBuf[64];
	Q_snprintf( szBuf, 64, "%f", hRHS );

	this->operator+=( szBuf );

	return *this;
}
//-----------------------------------------------------------------------------
// Purpose: operator +=
//-----------------------------------------------------------------------------

CFFString &CFFString::operator+=( const int& hRHS )
{
	char szBuf[64];
	Q_snprintf( szBuf, 64, "%i", hRHS );

	this->operator+=( szBuf );

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
// Purpose: operator []
//-----------------------------------------------------------------------------
const char CFFString::operator[]( int iElement ) const
{
	Assert( ( iElement >= 0 ) && ( iElement <= ( m_iSize - 1 ) ) );
	Assert( m_pszString );

	return m_pszString[ iElement ];
}

//-----------------------------------------------------------------------------
// Purpose: operator []
//-----------------------------------------------------------------------------
char &CFFString::operator[]( int iElement )
{
	Assert( ( iElement >= 0 ) && ( iElement <= ( m_iSize - 1 ) ) );
	Assert( m_pszString );

	return m_pszString[ iElement ];
}

//-----------------------------------------------------------------------------
// Purpose: Deallocate memory and reset
//-----------------------------------------------------------------------------
void CFFString::Cleanup( void )
{
	if( m_pszString )
	{
		delete [] m_pszString;
		m_pszString = NULL;		
	}

	m_iSize = 0;
}
