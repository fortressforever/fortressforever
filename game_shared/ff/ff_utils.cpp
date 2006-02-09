//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_utils.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date Unknown
//	@brief Utility functions
//
//	REVISIONS
//	---------
//	Unknown, Mulchman: 
//		First created
//
//	Unknown, Mulchman: 
//		Added a bunch of utility type stuff mainly to be used with the
//		Scout Radar
//
//	05/10/2005, Mulchman:
//		Moved some generic buildable stuff over here so it can be used
//		easily on both the client and server - mainly to comply
//		w/ the new buildable object slots (dispenser & sg slot on engy)
//		Modified the build code a bunch, too - completely overhauled.
//
//	06/08/2005, Mulchman:
//		Put more buildable object stuff in here to make it universal
//		and easier to update client/server stuff in one file. Ended
//		up moving buildable stuff out to a buildable shared file.

#include "cbase.h"
#include "ff_utils.h"
#include "Color.h"		// |-- Mirv: Fixed case for GCC
#include "ammodef.h"

#ifdef CLIENT_DLL
#include <igameresources.h>
#endif

// This function takes a class name like "scout"
// and returns its integer value
// 1 - scout
// 2 - sniper
// 3 - soldier
// 4 - demoman
// 5 - medic
// 6 - hwguy
// 7 - pyro
// 8 - spy
// 9 - engineer
// 10 - civilian
// 11 - dispenser
// 12 - sentrygun
int Class_StringToInt( const char *szClassName )
{
	// Doing case insensitive compares and also
	// trying to do so in the order of most popular
	// classes to least popular [eh, kind of...]
	if( Q_stricmp( szClassName, "soldier" ) == 0 )
		return 3;
	else if( Q_stricmp( szClassName, "medic" ) == 0 )
		return 5;
	else if( Q_stricmp( szClassName, "engineer" ) == 0 )
		return 9;
	else if( Q_stricmp( szClassName, "scout" ) == 0 )
		return 1;
	else if( Q_stricmp( szClassName, "hwguy" ) == 0 )
		return 6;
	else if( Q_stricmp( szClassName, "demoman" ) == 0 )
		return 4;
	else if( Q_stricmp( szClassName, "spy" ) == 0 )
		return 8;
	else if( Q_stricmp( szClassName, "sniper" ) == 0 )
		return 2;
	else if( Q_stricmp( szClassName, "pyro" ) == 0 )
		return 7;
	else if( Q_stricmp( szClassName, "dispenser" ) == 0 )
		return 11;
	else if( Q_stricmp( szClassName, "sentrygun" ) == 0 )
		return 12;
	else if( Q_stricmp( szClassName, "civilian" ) == 0 )
		return 10;
	else
		Warning( "Class_StringToInt :: No match!\n" );

	return 0;
}

// Takes an integer and returns back a string referring
// to the classname of that that integer represents
// Read the above chart backwards to figure out what
// everything means
const char *Class_IntToString( int iClassIndex )
{
	// yeah, the breaks aren't necessary but it's habit
	switch( iClassIndex )
	{
		case 0: return "unassigned"; break;
		case 1: return "scout"; break;
		case 2: return "sniper"; break;
		case 3: return "soldier"; break;
		case 4: return "demoman"; break;
		case 5: return "medic"; break;
		case 6: return "hwguy"; break;
		case 7: return "pyro"; break;
		case 8: return "spy"; break;
		case 9: return "engineer"; break;
		case 10: return "civilian"; break;
		case 11: return "dispenser"; break;
		case 12: return "sentrygun"; break;
		default: Warning( "Class_IntToString :: No match!\n" ); break;
	}

	return "\0";
}

// Returns the string to use to localize
const char *Class_IntToResourceString( int iClassIndex )
{
	// yeah, the breaks aren't necessary but it's habit
	switch( iClassIndex )
	{
		case 1: return "#FF_PLAYER_SCOUT"; break;
		case 2: return "#FF_PLAYER_SNIPER"; break;
		case 3: return "#FF_PLAYER_SOLDIER"; break;
		case 4: return "#FF_PLAYER_DEMOMAN"; break;
		case 5: return "#FF_PLAYER_MEDIC"; break;
		case 6: return "#FF_PLAYER_HWGUY"; break;
		case 7: return "#FF_PLAYER_PYRO"; break;
		case 8: return "#FF_PLAYER_SPY"; break;
		case 9: return "#FF_PLAYER_ENGINEER"; break;
		case 10: return "#FF_PLAYER_CIVILIAN"; break;
	}

	return "#FF_PLAYER_INVALID";
}

// Takes an integer and returns back a string referring
// to the classname of that that integer represents
// Read the above chart backwards to figure out what
// everything means
// NOTE: It returns a capitalized version
const char *Class_IntToPrintString( int iClassIndex )
{
	// yeah, the breaks aren't necessary but it's habit
	switch( iClassIndex )
	{
		case 1: return "Scout"; break;
		case 2: return "Sniper"; break;
		case 3: return "Soldier"; break;
		case 4: return "Demoman"; break;
		case 5: return "Medic"; break;
		case 6: return "HWGuy"; break;
		case 7: return "Pyro"; break;
		case 8: return "Spy"; break;
		case 9: return "Engineer"; break;
		case 10: return "Civilian"; break;
		case 11: return "Dispenser"; break;
		case 12: return "SentryGun"; break;
		default: Warning( "Class_IntToPrintString :: No match!\n" ); break;
	}

	return "\0";
}

// TODO: Will need to change this when models are
// put into the correct locations
char *Class_GetModelByInt( int iClassIndex )
{
	char szModelPath[ 512 ];
	char szClass[ 16 ];

	// Get the class name
	Q_strcpy( szClass, Class_IntToString( iClassIndex ) );

	// Start building our string
	Q_strcpy( szModelPath, "models/" );
	
	if( iClassIndex < 11 )
	{
		Q_strcat( szModelPath, "player/" );
	}
	else if( iClassIndex == 11 )
	{
		Q_strcat( szModelPath, "buildable/" );
	}
	else if( iClassIndex == 12 )
	{
		Q_strcpy( szModelPath, "models/weapons/sg/sg.mdl" );

		return &szModelPath[ 0 ];
	}
	
	Q_strcat( szModelPath, szClass );
	Q_strcat( szModelPath, "/" );
	Q_strcat( szModelPath, szClass );
	Q_strcat( szModelPath, ".mdl" );
	Q_strcat( szModelPath, "\0" );

	return &szModelPath[ 0 ];
}

void SetColorByTeam( int iTeam, Color& cColor )
{
	// Assuming correct team values...
	// 1 - blue, etc.

	switch( iTeam )
	{
		case 1: cColor.SetColor( 0, 0, 255 ); break;
		case 2: cColor.SetColor( 255, 0, 0 ); break;
		case 3: cColor.SetColor( 255, 255, 0 ); break;
		case 4: cColor.SetColor( 0, 255, 0 ); break;
		default: cColor.SetColor( 255, 255, 255 ); break;
	}
}

// Finds out how many players are on a team
// but uses game resources so it actually works
// on the client as well as the server
int FF_NumPlayersOnTeam( int iTeam )
{
#ifdef CLIENT_DLL
	int iCount = -1;

	// Get at the game resources
	IGameResources *pGR = GameResources();
	if( pGR )
	{
		iCount = 0;

		for( int i = 1; i < gpGlobals->maxClients; i++ )
		{
			if( pGR->IsConnected( i ) )
			{
				if( pGR->GetTeam( i ) == iTeam )
					iCount++;
			}
		}
	}

	return iCount;
#else
	int ct = 0;

	for (int i=1; i<=gpGlobals->maxClients; i++)
	{
		CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex(i) );
		if (pPlayer && pPlayer->IsPlayer() && pPlayer->GetTeamNumber() == iTeam)
		{
			ct++;
		}
	}

	return ct;
#endif
}

// find out how many players are on a team
int FF_NumPlayers( )
{
	int ct = 0;

	for (int i=1; i<=gpGlobals->maxClients; i++)
	{
		CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex(i) );
		if (pPlayer && pPlayer->IsPlayer())
		{
			ct++;
		}
	}

	return ct;
}


int FF_GetPlayerOnTeam( int iTeam, int iNum )
{
	for (int i=1; i<=gpGlobals->maxClients; i++)
	{
		CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex(i) );
		if (pPlayer && pPlayer->IsPlayer() && pPlayer->GetTeamNumber() == iTeam)
		{
			iNum--;
			if (iNum == 0)
				return i;
		}
	}

	return 0;
}

int FF_GetPlayer( int iNum )
{
	for (int i=1; i<=gpGlobals->maxClients; i++)
	{
		CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex(i) );
		if (pPlayer && pPlayer->IsPlayer())
		{
			iNum--;
			if (iNum == 0)
				return i;
		}
	}

	return 0;
}


//
// FF_HudHint
//		Client side: Just adds a HudHint message
//		Server side: Sends a HudHint message to the client
//
void FF_HudHint( 
#ifndef CLIENT_DLL 
				CFFPlayer *pPlayer,
#endif
				const char *pszMessage )
{
	if( 1 )
#ifdef CLIENT_DLL
	{		
		if( pHudHintHelper )
			pHudHintHelper->AddHudHint( pszMessage );
		else
			Warning( "[Hud Hint] Pointer not set up yet! Hud Hint lost!\n" );
	}
#else
	{
		if( !pPlayer )
			return;

		CSingleUserRecipientFilter user( pPlayer );
		user.MakeReliable( );
		UserMessageBegin( user, "FF_HudHint" );
			WRITE_STRING( pszMessage );
		MessageEnd( );
	}
#endif
}

const char *FF_GetAmmoName(int i)
{
	Ammo_t *ammo = GetAmmoDef()->GetAmmoOfIndex(i);

	if (ammo)
		return ammo->pName;

	return NULL;
}