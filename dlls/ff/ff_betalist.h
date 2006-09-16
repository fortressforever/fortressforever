// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_betalist.h
// @author Patrick O'Leary (Mulchman) 
// @date 9/11/2006
// @brief Validate users for the beta
//
// REVISIONS
// ---------
//	9/11/2006, Mulchman: 
//		First created
//
//	9/16/2006, Mulchman: 
//		Updates

#ifndef FF_BETALIST_H
#define FF_BETALIST_H

#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Turn beta on and off w/ this define
//-----------------------------------------------------------------------------
#define FF_BETA
//-----------------------------------------------------------------------------
// Turn beta on and off w/ this define
//-----------------------------------------------------------------------------

#ifdef FF_BETA

#include "UtlVector.h"
#include "ff_string.h"

// Forward declaration
class CFFBetaList;

typedef CFFString CFFBetaList_String;

//=============================================================================
//
// Class CFFBetaList_Player
//
//=============================================================================
class CFFBetaList_Player
{
public:
	CFFBetaList_Player( void );
	~CFFBetaList_Player( void );

public:
	bool	IsValidated( void ) const	{ return m_bValidated; }
	int		GetAttempts( void ) const	{ return m_iAttempts; }

	// Only CFFBetaList can validate us
	friend class CFFBetaList;

private:
	bool	m_bValidated;
	int		m_iAttempts;

};

//=============================================================================
//
// Class CFFBetaList
//
//=============================================================================
class CFFBetaList
{
public:
	CFFBetaList( void );
	~CFFBetaList( void );

public:
	class CFFBetaListLess
	{
	public:
		bool Less( const CFFBetaList_String& src1, const CFFBetaList_String& src2, void *pCtx )
		{
			return src1 < src2;
			/*if( src1 >= src2 )
				return false;
			else
				return true;*/
			/*if( Q_strcmp( src1, src2 ) >= 0 )
				return false;
			else
				return true;*/
		}
	};

public:
	// Used upon connection to see if a player's
	// name is in our valid name vector
	bool IsValidName( const CFFBetaList_String& szString ) const;

	// Used to see if a steam id is in our valid
	// steam id vector
	bool IsValidSteamID( const CFFBetaList_String& szString ) const;

	// Validates players on the server
	void Validate( void );

	// Loads the name & steam id file
	void Init( void );
	// Shuts down stuff
	void Shutdown( void );

private:
	void Clear( void );

private:
	//CUtlSortVector< CFFBetaList_String, CFFBetaListLess > m_hValidNames;
	CUtlVector< CFFBetaList_String > m_hValidNames;
	//CUtlSortVector< CFFBetaList_String, CFFBetaListLess > m_hValidSteamIDs;
	CUtlVector< CFFBetaList_String > m_hValidSteamIDs;

	float	m_flLastValidate;

};

//-----------------------------------------------------------------------------
// Global
//-----------------------------------------------------------------------------
extern CFFBetaList g_FFBetaList;

#endif // FF_BETA

#endif // FF_BETALIST_H
