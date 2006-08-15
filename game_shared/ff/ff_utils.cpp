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
//
//	05/28/2006, Mulchman:
//		Added FF_DecalTrace

#include "cbase.h"
#include "ff_utils.h"
#include "Color.h"		// |-- Mirv: Fixed case for GCC
#include "ammodef.h"

#ifdef CLIENT_DLL
	#include <igameresources.h>
#else
#ifdef _DEBUG
	#include "debugoverlay_shared.h"
#endif // _DEBUG
	#include "ff_gamerules.h"
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

void SetColorByTeam( int iTeam, Color& cColor )
{
	// Assuming correct team values...
	// 1 - blue, etc.

	switch( iTeam )
	{
		case 1: cColor.SetColor( 56, 100, 171 ); break;
		case 2: cColor.SetColor( 188, 0, 0 ); break;
		case 3: cColor.SetColor( 202, 173, 33 ); break;
		case 4: cColor.SetColor( 68, 144, 65 ); break;
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
				byte bType,
				unsigned short wID,
				const char *pszMessage,
				const char *pszSound)
{
	if( 1 )
#ifdef CLIENT_DLL
	{		
		if( pHudHintHelper )
			pHudHintHelper->AddHudHint(bType, wID, pszMessage, pszSound);
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
			WRITE_BYTE(bType);
			WRITE_WORD(wID);
			WRITE_STRING( pszMessage );
			
			if (pszSound)
				WRITE_STRING(pszSound);
			else
				WRITE_STRING("");

		MessageEnd( );
	}
#endif
}


bool IsPlayerRadioTagTarget( CFFPlayer *pPlayer, int iTeamDoingTargetting )
{
#ifdef CLIENT_DLL
	return false;
#else
	// iTeamDoingTargetting is the team of the object/player that is checking
	// to see if this radio tagged player is a valid target

	// pPlayer is the radio tagged player being checked

	if( !pPlayer->IsRadioTagged() )
		return false;

	// Purpose of this function is to check whether or not the radio tagged
	// player got tagged by someone who is an ally or teammate to the team
	// "iTeamDoingTargetting". If yes, return true, otherwise return false.

	CFFPlayer *pWhoTaggedTheGuy = pPlayer->GetPlayerWhoTaggedMe();
	if( pWhoTaggedTheGuy )
	{
		if( FFGameRules()->IsTeam1AlliedToTeam2( pWhoTaggedTheGuy->GetTeamNumber(), iTeamDoingTargetting ) == GR_TEAMMATE )
			return true;
	}

	return false;
#endif
}

void FF_DecalTrace( CBaseEntity *pEntity, float flRadius, const char *pszDecalName )
{
#ifdef CLIENT_DLL 
#else
	// If we've gotten here then the normal trace_t passed
	// to the explode function did not find ground below the
	// object. But, that doesn't mean we aren't near something
	// we should draw scorch marks on. So, check above and then
	// around the object for stuff to draw the scorch mark on.

	AssertMsg( pEntity, "FF_DecalTrace - Entity was NULL" );

	Vector vecOrigin = pEntity->GetAbsOrigin();

	trace_t trUp;
	UTIL_TraceLine( vecOrigin, vecOrigin + Vector( 0, 0, flRadius ), MASK_SHOT_HULL, pEntity, COLLISION_GROUP_NONE, &trUp );

	// If the trace never finished (we hit something)
	if( trUp.fraction != 1.0f )
	{
		UTIL_DecalTrace( &trUp, pszDecalName );
		return;
	}

	// Well, didn't hit anything below us (if we did we wouldn't
	// have been in this function in the first place) and we didn't
	// hit anything above us so now "reach out" and try to find
	// something nearby to do scorch marks on for:
	// Bug #0000211: Grens and pipe not drawing explosion soot decal on walls.

	// TODO: Anyone got a better idea?
	
	Vector vecEndPos[ 8 ], vecForward, vecRight, vecUp;

	AngleVectors( pEntity->GetAbsAngles(), &vecForward, &vecRight, &vecUp );	

	VectorNormalize( vecForward );
	VectorNormalize( vecRight );
	VectorNormalize( vecUp );

	// Make some points around the object where "O" is the object:
	// . . .
	// . O .
	// . . .

	vecEndPos[ 0 ] = vecOrigin + ( vecForward * flRadius ) - ( vecRight * flRadius );
	vecEndPos[ 1 ] = vecOrigin + ( vecForward * flRadius );
	vecEndPos[ 2 ] = vecOrigin + ( vecForward * flRadius ) + ( vecRight * flRadius );
	vecEndPos[ 3 ] = vecOrigin - ( vecRight * flRadius );
	vecEndPos[ 4 ] = vecOrigin + ( vecRight * flRadius );
	vecEndPos[ 5 ] = vecOrigin - ( vecForward * flRadius ) - ( vecRight * flRadius );
	vecEndPos[ 6 ] = vecOrigin - ( vecForward * flRadius );
	vecEndPos[ 7 ] = vecOrigin - ( vecForward * flRadius ) + ( vecRight * flRadius );

	// Go ahead and compute this now to use later
	vecUp *= flRadius;

	// For the traces
	trace_t tr[ 24 ];
	// Which trace we're on
	int iTraceCount = 0;
	// Index to use for the shortest trace
	int iIndex = -1;
	// To keep track of shortest distance
	float flDist = flRadius * flRadius;
	
	// Do 24 traces - EEK
	for( int j = -1; j <= 1; j++ )
	{
		for( int i = 0; i < 8; i++ )
		{
			// Want to make sure we're only tracing out flRadius units
			// so get a direction vector facing the outward point(s)
			Vector vecDir = ( vecEndPos[ i ] + ( j * vecUp ) ) - vecOrigin;
			VectorNormalize( vecDir );

			UTIL_TraceLine( vecOrigin, vecOrigin + ( vecDir * flRadius ), MASK_SHOT_HULL, pEntity, COLLISION_GROUP_NONE, &tr[ iTraceCount++ ] );

#ifdef _DEBUG
			// Draw the trace
			//debugoverlay->AddLineOverlay( tr[ iTraceCount - 1 ].startpos, tr[ iTraceCount - 1 ].endpos, 255, 0, 0, false, 2.0f );
#endif

			// [Trace didn't finish so] we hit something
			if( tr[ iTraceCount - 1 ].fraction != 1.0f )
			{
				// Is this distance closer?
				if( vecOrigin.DistTo( tr[ iTraceCount - 1 ].endpos ) < flDist )
				{
					// Store off this trace index since it's the closet so far
					iIndex = iTraceCount - 1;
				}
			}
		}
	}

	if( iIndex != -1 )
	{
		//DevMsg( "[FF_DecalTrace] Drawing a scorch mark!\n" );
		UTIL_DecalTrace( &tr[ iIndex ], pszDecalName );
	}
	//else
		//DevMsg( "[FF_DecalTrace] Didn't hit anything - no scorch mark!\n" );
#endif
}

const char *FF_GetAmmoName(int i)
{
	Ammo_t *ammo = GetAmmoDef()->GetAmmoOfIndex(i);

	if (ammo)
		return ammo->pName;

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: See if a trace hit the world
//-----------------------------------------------------------------------------
bool FF_TraceHitWorld( trace_t *pTrace )
{
	if( pTrace->DidHitWorld() )
		return true;

	if( pTrace->m_pEnt == NULL )
		return false;

	if( pTrace->m_pEnt->GetMoveType() == MOVETYPE_PUSH )
	{
		// All doors are push, but not all things that push are doors. This 
		// narrows the search before we start to do classname compares.
		if( FClassnameIs( pTrace->m_pEnt, "prop_door_rotating" ) ||
			FClassnameIs( pTrace->m_pEnt, "func_door" ) ||
			FClassnameIs( pTrace->m_pEnt, "func_door_rotating" ) )
			return true;
	}

	return false;
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: Set an icon on the hud
//-----------------------------------------------------------------------------
void FF_LuaHudIcon(CFFPlayer *pPlayer, const char *pszIdentifier, int x, int y, const char *pszImage)
{
	if (!pPlayer)
		return;

	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();

	UserMessageBegin(user, "FF_HudLua");
		WRITE_BYTE(0);	// HUD_ICON
		WRITE_STRING(pszIdentifier);
		WRITE_SHORT(x);
		WRITE_SHORT(y);
		WRITE_STRING(pszImage);
	MessageEnd();
}

//-----------------------------------------------------------------------------
// Purpose: Set some text on the hud
//-----------------------------------------------------------------------------
void FF_LuaHudText(CFFPlayer *pPlayer, const char *pszIdentifier, int x, int y, const char *pszText)
{
	if (!pPlayer)
		return;

	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();

	UserMessageBegin(user, "FF_HudLua");
		WRITE_BYTE(1);	// HUD_TEXT
		WRITE_STRING(pszIdentifier);
		WRITE_SHORT(x);
		WRITE_SHORT(y);
		WRITE_STRING(pszText);
	MessageEnd();
}

//-----------------------------------------------------------------------------
// Purpose: Set a timer on the hud
//-----------------------------------------------------------------------------
void FF_LuaHudTimer(CFFPlayer *pPlayer, const char *pszIdentifier, int x, int y, int iStartValue, float flSpeed)
{
	if (!pPlayer)
		return;

	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();

	UserMessageBegin(user, "FF_HudLua");
		WRITE_BYTE(2);	// HUD_TIMER
		WRITE_STRING(pszIdentifier);
		WRITE_SHORT(x);
		WRITE_SHORT(y);
		WRITE_SHORT(iStartValue);
		WRITE_FLOAT(flSpeed);
	MessageEnd();
}

void FF_LuaHudRemove(CFFPlayer *pPlayer, const char *pszIdentifier)
{
	if (!pPlayer)
		return;

	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();

	UserMessageBegin(user, "FF_HudLua");
		WRITE_BYTE(3);	// HUD_REMOVE
		WRITE_STRING(pszIdentifier);
	MessageEnd();
}
#endif