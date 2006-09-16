// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_string.h
// @author Patrick O'Leary (Mulchman) 
// @date 9/16/2006
// @brief Simple string class to wrap char*'s. Don't
//			care too much about speed, either, or
//			making the size adjustable!
//
// REVISIONS
// ---------
//	9/16/2006, Mulchman: 
//		First created

#ifndef FF_STRING_H
#define FF_STRING_H

class CFFString
{
public:
	CFFString( void );
	CFFString( const char *pszString );
	CFFString( const CFFString& hRHS );
	~CFFString( void );

	CFFString &operator=( const CFFString& hRHS );
	CFFString &operator=( const char *pszString );

	bool operator==( const CFFString& hRHS ) const;
	bool operator==( const char *pszString ) const;

	bool operator!=( const CFFString& hRHS ) const;
	bool operator!=( const char *pszString ) const;

	bool operator<( const CFFString& hRHS ) const;
	bool operator<( const char *pszString ) const;

	bool operator>( const CFFString& hRHS ) const;
	bool operator>( const char *pszString ) const;

	bool operator<=( const CFFString& hRHS ) const;
	bool operator<=( const char *pszString ) const;

	bool operator>=( const CFFString& hRHS ) const;
	bool operator>=( const char *pszString ) const;

	int GetLength( void ) const	{ return m_iSize; }

	const char *GetString( void ) const	{ return c_str(); }
	const char *c_str( void ) const { return m_pszString; }

private:
	//char	*m_pszString;
	char	m_pszString[ 128 ];
	int		m_iSize;

private:
	void	Cleanup();

};

#endif // FF_STRING_H
