// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_betalist.cpp
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

#include "cbase.h"
#include "ff_betalist.h"

#ifdef FF_BETA

#include "ff_player.h"
#include <filesystem.h>
IFileSystem **pFFBetaListFilesystem = &filesystem;

// Forward declaration
class CFFPlayer;

// Location of our beta users file
#define FF_BETA_LIST_FILE "scripts/ff_betalist.txt"

//-----------------------------------------------------------------------------
// Global
//-----------------------------------------------------------------------------
CFFBetaList g_FFBetaList;

static ConVar ffbetalist_maxattempts( "ffbetalist_maxattempts", "5", FCVAR_ARCHIVE, "Maximum number of times we'll try to get a players Steam ID before kicking." );
static ConVar ffbetalist_validatetime( "ffbetalist_validatetime", "5", FCVAR_ARCHIVE, "Number of seconds between trying to validate un-validated players." );
static ConVar ffbetalist_allowlanids( "ffbetalist_allowlanids", "1", FCVAR_ARCHIVE, "Whether or not to allow STEAM_ID_LAN Steam IDs." );

//=============================================================================
//
// Class CFFBetaList_Player
//
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFBetaList_Player::CFFBetaList_Player( void )
{
	m_bValidated = false;
	m_iAttempts = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
CFFBetaList_Player::~CFFBetaList_Player( void )
{
}

//=============================================================================
//
// Class CFFBetaList
//
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFBetaList::CFFBetaList( void )
{
	Clear();
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
CFFBetaList::~CFFBetaList( void )
{
	Clear();
}

//-----------------------------------------------------------------------------
// Purpose: See if the name is in our name vector
//-----------------------------------------------------------------------------
bool CFFBetaList::IsValidName( const CFFBetaList_String& szString ) const
{
	if( m_hValidNames.Find( szString ) >= 0 )
		return true;
	
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: See if the steam id is in our steam id vector
//-----------------------------------------------------------------------------
bool CFFBetaList::IsValidSteamID( const CFFBetaList_String& szString ) const
{
	if( m_hValidSteamIDs.Find( szString ) >= 0 )
		return true;

	// Check if special conditions exist
	if( ffbetalist_allowlanids.GetBool() )
	{
		if( szString == "STEAM_ID_LAN" )
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Clear out our vectors
//-----------------------------------------------------------------------------
void CFFBetaList::Clear( void )
{
	m_hValidNames.RemoveAll();
	m_hValidSteamIDs.RemoveAll();

	m_flLastValidate = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Validate players on the server
//-----------------------------------------------------------------------------
void CFFBetaList::Validate( void )
{
	// We're going to console spam so we can track stuff during beta

	// Is it time to validate yet?
	if( ( m_flLastValidate + ffbetalist_validatetime.GetFloat() ) < gpGlobals->curtime )
	{
		Warning( "[FF Beta List] Time to validate players (time: %f)!\n", gpGlobals->curtime );

		for( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex( i ) );
			if( pPlayer )
			{
				DevMsg( "[FF Beta List] Player: %s", pPlayer->GetPlayerName() );

				// Get the player's ff beta list info
				CFFBetaList_Player *pFFBetaListInfo = pPlayer->GetFFBetaListInfo();
				if( !pFFBetaListInfo )
				{
					DevMsg( " failed to get FFBetaListInfo!\n" );
					Assert( 0 );
					continue;
				}

				// If they're not validated...
				if( !pFFBetaListInfo->IsValidated() )
				{
					// Grab the steam id
					const char *pszSteamID = pPlayer->GetSteamID();					

					// Steam id valid?
					if( IsValidSteamID( pszSteamID ) )
					{
						// Player is validated
						pFFBetaListInfo->m_bValidated = true;

						DevMsg( " is validated - SteamID: %s\n", pszSteamID );
					}
					else
					{
						// Player steam id still invalid. Could be
						// resolving or could be a non-beta-er.

						// Check attempts and see if we'll try
						// later or should just kick him now!

						DevMsg( " is not validated yet - SteamID: %s -", pszSteamID );

						if( pFFBetaListInfo->GetAttempts() < ffbetalist_maxattempts.GetInt() )
						{
							// We'll try again later
							pFFBetaListInfo->m_iAttempts++;

							DevMsg( " will try again later.\n" );
						}
						else
						{
							// Kick the guy!

							DevMsg( " over max attempts so kicking!\n" );

							// TODO: kick!
						}
					}
				}
				else
				{
					DevMsg( " is already validated - SteamID: %s\n", pPlayer->GetSteamID() );
				}
			}			
		}

		m_flLastValidate = gpGlobals->curtime;

		Warning( "[FF Beta List] Validation complete (next run @ time %f).\n", m_flLastValidate + ffbetalist_validatetime.GetFloat() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Loads valid names & valid steam ids from file. Gets re-loaded upon
//			map/changelevel change.
//-----------------------------------------------------------------------------
void CFFBetaList::Init( void )
{
	KeyValues *kv = new KeyValues( "FFBetaList" );
	if( kv->LoadFromFile( ( *pFFBetaListFilesystem ), FF_BETA_LIST_FILE, "MOD" ) )
	{
		for( KeyValues *pEntry = kv->GetFirstSubKey(); pEntry; pEntry = pEntry->GetNextKey() )
		{
			if( !Q_stricmp( pEntry->GetName(), "FFBetaUser" ) )
			{
				for( KeyValues *pItems = pEntry->GetFirstSubKey(); pItems; pItems = pItems->GetNextKey() )
				{
					if( !Q_stricmp( pItems->GetName(), "Name" ) )
					{
						//const char *pszString = pItems->GetString();
						CFFBetaList_String szString = pItems->GetString();
						DevMsg( "[FF Beta List] Read from file: %s\n", szString.GetString() );
						m_hValidNames.AddToTail( szString );
					}
					else if( !Q_stricmp( pItems->GetName(), "SteamID" ) )
					{
						//const char *pszString = pItems->GetString();
						CFFBetaList_String szString = pItems->GetString();
						DevMsg( "[FF Beta List] Read from file: %s\n", szString.GetString() );
						m_hValidSteamIDs.AddToTail( szString );
					}
				}
			}
		}
	}

	kv->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: Shuts down stuff
//-----------------------------------------------------------------------------
void CFFBetaList::Shutdown( void )
{
	Clear();
}

#endif // FF_BETA
