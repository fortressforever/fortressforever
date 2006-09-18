// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_string.h
// @author Patrick O'Leary (Mulchman) 
// @date 9/16/2006
// @brief Simple string class to wrap char*'s. Don't
//			care too much about speed, either!
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
	virtual ~CFFString( void );

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

	const char operator[]( int iElement ) const;
	char &operator[]( int iElement );

	int GetLength( void ) const	{ return m_iSize; }

	const char *GetString( void ) const	{ return c_str(); }
	const char *c_str( void ) const { return m_pszString ? m_pszString : ""; }
	const char *operator()( void ) const { return c_str(); }

private:
	char	*m_pszString;
	int		m_iSize;

private:
	void	Cleanup();

};

#endif // FF_STRING_H
