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

#ifdef GAME_DLL
	#include "ff_team.h"
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
	
	if( Q_stricmp( szClassName, "scout" ) == 0 )
		return 1;
	else if( Q_stricmp( szClassName, "sniper" ) == 0 )
		return 2;
	else if( Q_stricmp( szClassName, "soldier" ) == 0 )
		return 3;
	else if( Q_stricmp( szClassName, "demoman" ) == 0 )
		return 4;
	else if( Q_stricmp( szClassName, "medic" ) == 0 )
		return 5;
	else if( Q_stricmp( szClassName, "hwguy" ) == 0 )
		return 6;
	else if( Q_stricmp( szClassName, "pyro" ) == 0 )
		return 7;
	else if( Q_stricmp( szClassName, "spy" ) == 0 )
		return 8;
	else if( Q_stricmp( szClassName, "engineer" ) == 0 )
		return 9;
	else if( Q_stricmp( szClassName, "civilian" ) == 0 )
		return 10;
	else if( Q_stricmp( szClassName, "dispenser" ) == 0 )
		return 11;
	else if( Q_stricmp( szClassName, "sentrygun" ) == 0 )
		return 12;
	else if( Q_stricmp( szClassName, "mancannon" ) == 0 )
		return 13;
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
		case 13: return "mancannon"; break;
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
		case 11: return "#FF_PLAYER_DISPENSER"; break;
		case 12: return "#FF_PLAYER_SENTRYGUN"; break;
		case 13: return "#FF_PLAYER_MANCANNON"; break;
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
		case 13: return "ManCannon"; break;
		default: Warning( "Class_IntToPrintString :: No match!\n" ); break;
	}

	return "\0";
}

// ELMO *** 
//I don't know if there is a function for this already.
//I put it here from speedometer to use in crosshair info and anything else we wish to colour/color fade!
Color ColorFade( int currentVal, int minVal, int maxVal, Color minColor, Color maxColor )
{
	int clampedCurrent = clamp(currentVal, minVal, maxVal);
	float full, f1, f2;
	full = maxVal - minVal;
	f1 = (maxVal - clampedCurrent) / full;
	f2 = (clampedCurrent - minVal) / full;
	return Color(
		(int) (maxColor.r() * f2 + minColor.r() * f1),
		(int) (maxColor.g() * f2 + minColor.g() * f1),
		(int) (maxColor.b() * f2 + minColor.b() * f1),
		255);
}

Color getIntensityColor( int iAmount, int iMaxAmount, int iColorSetting, int iAlpha, int iRed, int iOrange, int iYellow, int iGreen, bool invertScale )
{
	Color innerCol;
	if(!invertScale) 
	{
		if( iAmount <= iRed && iColorSetting > 0)
			innerCol = INTENSITYSCALE_COLOR_RED;
		else if(iAmount  <= iOrange && iColorSetting > 0)
			if(iColorSetting == 2)
				innerCol = ColorFade( iAmount, iRed, iOrange, INTENSITYSCALE_COLOR_RED, INTENSITYSCALE_COLOR_ORANGE );
			else
				innerCol = INTENSITYSCALE_COLOR_ORANGE;	
		else if(iAmount  <= iYellow && iColorSetting > 0)
			if(iColorSetting == 2)
				innerCol = ColorFade( iAmount, iOrange, iYellow, INTENSITYSCALE_COLOR_ORANGE, INTENSITYSCALE_COLOR_YELLOW );
			else
				innerCol = INTENSITYSCALE_COLOR_YELLOW;
		else if(iColorSetting > 0)
			if(iColorSetting == 2)
				innerCol = ColorFade( iAmount, iYellow, iGreen, INTENSITYSCALE_COLOR_YELLOW, INTENSITYSCALE_COLOR_GREEN );
			else
				innerCol = INTENSITYSCALE_COLOR_GREEN;
		else
			innerCol = INTENSITYSCALE_COLOR_DEFAULT;

		return *new Color(innerCol.r(), innerCol.g(), innerCol.b(), iAlpha);
	}
	else
	{
		//not working - currently no use for it.. someone can write this when it's needed
		// so that 0% will be green and red will be 100%
		if( iAmount > iRed && iColorSetting > 0)
			innerCol = INTENSITYSCALE_COLOR_RED;
		else if(iAmount  > iOrange && iColorSetting > 0)
			if(iColorSetting == 2)
				innerCol = ColorFade( iAmount, iOrange, iRed, INTENSITYSCALE_COLOR_ORANGE,INTENSITYSCALE_COLOR_RED );
			else
				innerCol = INTENSITYSCALE_COLOR_ORANGE;	
		else if(iAmount  > iYellow && iColorSetting> 0)
			if(iColorSetting == 2)
				innerCol = ColorFade( iAmount, iYellow, iOrange, INTENSITYSCALE_COLOR_YELLOW, INTENSITYSCALE_COLOR_ORANGE );
			else
				innerCol = INTENSITYSCALE_COLOR_YELLOW;
		else if(iColorSetting > 0)
			if(iColorSetting == 2)
				innerCol = ColorFade( iAmount, iGreen, iYellow, INTENSITYSCALE_COLOR_YELLOW, INTENSITYSCALE_COLOR_GREEN );
			else
				innerCol = INTENSITYSCALE_COLOR_GREEN;
		else
			innerCol = INTENSITYSCALE_COLOR_DEFAULT;

		return *new Color(innerCol.r(), innerCol.g(), innerCol.b(), iAlpha);
	}
}
// *** ELMO

void SetColorByTeam( int iTeam, Color& cColor )
{
	// Assuming correct team values...
	// 1 - blue, etc.

	switch( iTeam )
	{
		case TEAM_BLUE: cColor.SetColor( 64, 128, 255 ); break;
		case TEAM_RED: cColor.SetColor( 255, 64, 64 ); break;
		case TEAM_YELLOW: cColor.SetColor( 255, 255, 64 ); break;
		case TEAM_GREEN: cColor.SetColor( 100, 255, 100 ); break;
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
			pHudHintHelper->AddHudHint( bType, wID, pszMessage, pszSound );
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

// Jiggles: For sending hints to the Hint Center
// I left HudHint intact for reference
// FF_SendHint
//		Client side: Just adds a HudHint message
//		Server side: Sends a HudHint message to the client
//
void FF_SendHint( 
#ifndef CLIENT_DLL 
				CFFPlayer *pPlayer,
#endif
				unsigned short wID,
				short	HintCount,
				short	HintPriority,
				const char *pszMessage)
{
	//if( 1 )
#ifdef CLIENT_DLL
	{		
		if( g_pHintHelper )
			g_pHintHelper->AddHudHint( wID, HintCount, HintPriority, pszMessage );
		else
			Warning( "[Hud Hint] Pointer not set up yet! Hud Hint lost!\n" );
	}
#else
	{
		if( !pPlayer )
			return;

		CSingleUserRecipientFilter user( pPlayer );
		user.MakeReliable( );
		UserMessageBegin( user, "FF_SendHint" );
			//WRITE_BYTE(bType);
			WRITE_WORD( wID );
			WRITE_SHORT( HintCount );
			WRITE_SHORT( HintPriority );
			WRITE_STRING( pszMessage );
			
			//if (pszSound)
			//	WRITE_STRING(pszSound);
			//else
			//	WRITE_STRING("");

			
		MessageEnd( );
	}
#endif
}
// Jiggles: End Test


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
	if( !pTrace )
		return false;

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
			FClassnameIs( pTrace->m_pEnt, "func_door_rotating" ) ||
			FClassnameIs( pTrace->m_pEnt, "func_breakable" ) )	// Jiggles: Detpacks were tracing through the Dustbowl gates
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Is the entity a player?
//-----------------------------------------------------------------------------
bool FF_IsPlayer( CBaseEntity *pEntity )
{
	if( !pEntity )
		return false;

	return pEntity->IsPlayer();
}

//-----------------------------------------------------------------------------
// Purpose: Is the entity a grenade?
//-----------------------------------------------------------------------------
bool FF_IsGrenade( CBaseEntity *pEntity )
{
	if( !pEntity )
		return false;

	return ( pEntity->GetFlags() & FL_GRENADE ) ? true : false;
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: Set an icon on the hud - default alignment
//-----------------------------------------------------------------------------
void FF_LuaHudIcon(CFFPlayer *pPlayer, const char *pszIdentifier, int x, int y, const char *pszImage, int iWidth, int iHeight)
{
	if (!pPlayer)
		return;

	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();

	UserMessageBegin(user, "FF_HudLua");
	WRITE_BYTE(HUD_ICON);	// HUD_ICON
	WRITE_STRING(pszIdentifier);
	WRITE_SHORT(x);
	WRITE_SHORT(y);
	WRITE_STRING(pszImage);
	WRITE_SHORT(iWidth);
	WRITE_SHORT(iHeight);

	MessageEnd();
}
//-----------------------------------------------------------------------------
// Purpose: Set an icon on the hud
//-----------------------------------------------------------------------------
void FF_LuaHudIcon(CFFPlayer *pPlayer, const char *pszIdentifier, int x, int y, const char *pszImage, int iWidth, int iHeight, int iAlign)
{
	if (!pPlayer)
		return;

	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();

	UserMessageBegin(user, "FF_HudLua");
	WRITE_BYTE(HUD_ICON_ALIGN);	// HUD_ICON_ALIGN
	WRITE_STRING(pszIdentifier);
	WRITE_SHORT(x);
	WRITE_SHORT(y);
	WRITE_STRING(pszImage);
	WRITE_SHORT(iWidth);
	WRITE_SHORT(iHeight);
	WRITE_SHORT(iAlign);

	MessageEnd();
}

//-----------------------------------------------------------------------------
// Purpose: Set an icon on the hud (added y alignment)
//-----------------------------------------------------------------------------
void FF_LuaHudIcon(CFFPlayer *pPlayer, const char *pszIdentifier, int x, int y, const char *pszImage, int iWidth, int iHeight, int iAlignX, int iAlignY)
{
	if (!pPlayer)
		return;

	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();

	UserMessageBegin(user, "FF_HudLua");
		WRITE_BYTE(HUD_ICON_ALIGNXY);	// HUD_ICON_ALIGNXY
		WRITE_STRING(pszIdentifier);
		WRITE_SHORT(x);
		WRITE_SHORT(y);
		WRITE_STRING(pszImage);
		WRITE_SHORT(iWidth);
		WRITE_SHORT(iHeight);
		WRITE_SHORT(iAlignX);
		WRITE_SHORT(iAlignY);
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
		WRITE_BYTE(HUD_TEXT);	// HUD_TEXT
		WRITE_STRING(pszIdentifier);
		WRITE_SHORT(x);
		WRITE_SHORT(y);
		WRITE_STRING(pszText);
	MessageEnd();
}

//-----------------------------------------------------------------------------
// Purpose: Set some text on the hud
//-----------------------------------------------------------------------------
void FF_LuaHudText(CFFPlayer *pPlayer, const char *pszIdentifier, int x, int y, const char *pszText, int iAlign)
{
	if (!pPlayer)
		return;

	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();

	UserMessageBegin(user, "FF_HudLua");
	WRITE_BYTE(HUD_TEXT_ALIGN);	// HUD_TEXT_ALIGN
	WRITE_STRING(pszIdentifier);
	WRITE_SHORT(x);
	WRITE_SHORT(y);
	WRITE_STRING(pszText);
	WRITE_SHORT(iAlign);
	MessageEnd();
}

//-----------------------------------------------------------------------------
// Purpose: Set some text on the hud
//-----------------------------------------------------------------------------
void FF_LuaHudText(CFFPlayer *pPlayer, const char *pszIdentifier, int x, int y, const char *pszText, int iAlignX, int iAlignY)
{
	if (!pPlayer)
		return;

	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();

	UserMessageBegin(user, "FF_HudLua");
	WRITE_BYTE(HUD_TEXT_ALIGNXY);	// HUD_TEXT_ALIGNXY
	WRITE_STRING(pszIdentifier);
	WRITE_SHORT(x);
	WRITE_SHORT(y);
	WRITE_STRING(pszText);
	WRITE_SHORT(iAlignX);
	WRITE_SHORT(iAlignY);
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
		WRITE_BYTE(HUD_TIMER);	// HUD_TIMER
		WRITE_STRING(pszIdentifier);
		WRITE_SHORT(x);
		WRITE_SHORT(y);
		WRITE_SHORT(iStartValue);
		WRITE_FLOAT(flSpeed);
	MessageEnd();
}

//-----------------------------------------------------------------------------
// Purpose: Set a timer on the hud
//-----------------------------------------------------------------------------
void FF_LuaHudTimer(CFFPlayer *pPlayer, const char *pszIdentifier, int x, int y, int iStartValue, float flSpeed, int iAlign)
{
	if (!pPlayer)
		return;

	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();

	UserMessageBegin(user, "FF_HudLua");
	WRITE_BYTE(HUD_TIMER_ALIGN);	// HUD_TIMER_ALIGN
	WRITE_STRING(pszIdentifier);
	WRITE_SHORT(x);
	WRITE_SHORT(y);
	WRITE_SHORT(iStartValue);
	WRITE_FLOAT(flSpeed);
	WRITE_SHORT(iAlign);
	MessageEnd();
}

//-----------------------------------------------------------------------------
// Purpose: Set a timer on the hud
//-----------------------------------------------------------------------------
void FF_LuaHudTimer(CFFPlayer *pPlayer, const char *pszIdentifier, int x, int y, int iStartValue, float flSpeed, int iAlignX, int iAlignY)
{
	if (!pPlayer)
		return;

	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();

	UserMessageBegin(user, "FF_HudLua");
	WRITE_BYTE(HUD_TIMER_ALIGNXY);	// HUD_TIMER_ALIGNXY
	WRITE_STRING(pszIdentifier);
	WRITE_SHORT(x);
	WRITE_SHORT(y);
	WRITE_SHORT(iStartValue);
	WRITE_FLOAT(flSpeed);
	WRITE_SHORT(iAlignX);
	WRITE_SHORT(iAlignY);
	MessageEnd();
}

void FF_LuaHudRemove(CFFPlayer *pPlayer, const char *pszIdentifier)
{
	if (!pPlayer)
		return;

	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();

	UserMessageBegin(user, "FF_HudLua");
		WRITE_BYTE(HUD_REMOVE);	// HUD_REMOVE
		WRITE_STRING(pszIdentifier);
	MessageEnd();
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Returns whether we're a spectator or not
//-----------------------------------------------------------------------------
bool FF_IsPlayerSpec( CFFPlayer *pPlayer )
{
	if( !pPlayer )
		return false;

	return !( ( pPlayer->GetTeamNumber() >= TEAM_BLUE ) && ( pPlayer->GetTeamNumber() <= TEAM_GREEN ) );
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether we've picked a class yet
//-----------------------------------------------------------------------------
bool FF_HasPlayerPickedClass( CFFPlayer *pPlayer )
{
	if( !pPlayer )
		return false;

	return ( ( pPlayer->GetClassSlot() >= CLASS_SCOUT ) && ( pPlayer->GetClassSlot() <= CLASS_CIVILIAN ) );
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Get the current map name, but formatted
//-----------------------------------------------------------------------------
char *UTIL_GetFormattedMapName( void )
{	
	static char szText[ 256 ];
	Q_strcpy( szText, engine->GetLevelName() + 5 ); // Skip the "maps/" part
	szText[ ( int )strlen( szText ) - 4 ] = '\0'; // Skip the ".bsp" part

	return szText;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: An array with current numbers on teams
//-----------------------------------------------------------------------------
void UTIL_GetTeamNumbers(char nTeamNumbers[4])
{
	// Make sure we always zero this first
	memset(nTeamNumbers, 0, sizeof(char) * 4);

#ifdef CLIENT_DLL
	// If there's no game resources (a weird thing indeed) then we'll
	// be returning with a zero'd out array which is okay with me.
	IGameResources *pGR = GameResources();
	
	if (pGR == NULL)
		return;
#endif

	// Now loop through the players and take different branches to find out 
	// what team they are on.
	for (int iClient = 1; iClient <= gpGlobals->maxClients; iClient++)
	{
#ifdef GAME_DLL
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(iClient - 1);
		
		if (pPlayer == NULL || !pPlayer->IsConnected())
			continue;
		
		int iTeamIndex = pPlayer->GetTeamNumber() - TEAM_BLUE;
#else
		if (!pGR->IsConnected(iClient))
			continue;

		int iTeamIndex = pGR->GetTeam(iClient) - TEAM_BLUE;
#endif

		// Finally add this team if it is valid
		if (iTeamIndex >= 0 && iTeamIndex < 4)
		{
			nTeamNumbers[iTeamIndex]++;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: An array with team limits
//-----------------------------------------------------------------------------
void UTIL_GetTeamLimits(char nTeamLimits[4])
{
	// Make sure we always zero this first
	memset(nTeamLimits, 0, sizeof(char) * 4);

#ifdef CLIENT_DLL
	// If there's no game resources (a weird thing indeed) then we'll
	// be returning with a zero'd out array which is okay with me.
	IGameResources *pGR = GameResources();

	if (pGR == NULL)
		return;
#endif

	// Loop through teams getting limits
	for (int iTeamID = TEAM_BLUE; iTeamID <= TEAM_GREEN; iTeamID++)
	{
		int iTeamIndex = iTeamID - TEAM_BLUE;

#ifdef GAME_DLL
		CFFTeam *pTeam = GetGlobalFFTeam(iTeamID);

		// This team doesn't exist so keep it disabled
		if (pTeam == NULL)
		{
			nTeamLimits[iTeamIndex] = -1;
			continue;
		}

		nTeamLimits[iTeamIndex] = pTeam->GetTeamLimits();
#else
		nTeamLimits[iTeamIndex] = pGR->GetTeamLimits(iTeamID);
#endif
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get the space left on a team
//				-1 : team disabled completely
//				 0 : no space
//				 n : n places left
//			    99 : no space limit at all
//
//				Returns number of free teams
//-----------------------------------------------------------------------------
int UTIL_GetTeamSpaces(char nSpacesRemaining[4])
{
	char nTeamNumbers[4];
	UTIL_GetTeamNumbers(nTeamNumbers);

	char nTeamLimits[4];
	UTIL_GetTeamLimits(nTeamLimits);

	int nFreeTeams = 0;

	// Now loop through the teams and take different branches to find out
	// what their limits are and calculate places remaining from these
	for (int iTeamID = TEAM_BLUE; iTeamID <= TEAM_GREEN; iTeamID++)
	{
		int iTeamIndex = iTeamID - TEAM_BLUE;

		// No limit at all
		if (nTeamLimits[iTeamIndex] == 0)
		{
			nSpacesRemaining[iTeamIndex] = 99;
			nFreeTeams++;
			continue;
		}

		// Team disabled
		if (nTeamLimits[iTeamIndex] == -1)
		{
			nSpacesRemaining[iTeamIndex] = -1;
			continue;
		}

		nSpacesRemaining[iTeamIndex] = nTeamLimits[iTeamIndex] - nTeamNumbers[iTeamIndex];

		// It shouldn't get below 0 but nevermind
		nSpacesRemaining[iTeamIndex] = max(nSpacesRemaining[iTeamIndex], 0);

		if (nSpacesRemaining[iTeamIndex] > 0)
		{
			nFreeTeams++;
		}
	}

	return nFreeTeams;
}

//-----------------------------------------------------------------------------
// Purpose: An array with current numbers on Classs
//-----------------------------------------------------------------------------
void UTIL_GetClassNumbers(int iTeam, char nClassNumbers[10])
{
	// Make sure we always zero this first
	memset(nClassNumbers, 0, sizeof(char) * 10);

#ifdef CLIENT_DLL
	// If there's no game resources (a weird thing indeed) then we'll
	// be returning with a zero'd out array which is okay with me.
	IGameResources *pGR = GameResources();

	if (pGR == NULL)
		return;
#endif

	// Now loop through the players and take different branches to find out 
	// what Class they are on.
	for (int iClient = 1; iClient <= gpGlobals->maxClients; iClient++)
	{
#ifdef GAME_DLL
		CFFPlayer *pPlayer = (CFFPlayer *) UTIL_PlayerByIndex(iClient - 1);

		if (pPlayer == NULL || !pPlayer->IsConnected() || pPlayer->GetTeamNumber() != iTeam)
			continue;

		int iClassIndex = pPlayer->GetClassSlot() - CLASS_SCOUT;
#else
		if (!pGR->IsConnected(iClient) || pGR->GetTeam(iClient) != iTeam)
			continue;

		int iClassIndex = pGR->GetClass(iClient) - CLASS_SCOUT;
#endif

		// Finally add this Class if it is valid
		if (iClassIndex >= 0 && iClassIndex < 10)
		{
			nClassNumbers[iClassIndex]++;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: An array with Class limits
//-----------------------------------------------------------------------------
void UTIL_GetClassLimits(int iTeamID, char nClassLimits[10])
{
	// Make sure we always zero this first
	memset(nClassLimits, 0, sizeof(char) * 10);

#ifdef CLIENT_DLL
	// If there's no game resources (a weird thing indeed) then we'll
	// be returning with a zero'd out array which is okay with me.
	IGameResources *pGR = GameResources();

	if (pGR == NULL)
		return;
#endif

	// Loop through Classs getting limits
	for (int iClassID = CLASS_SCOUT; iClassID <= CLASS_CIVILIAN; iClassID++)
	{
		int iClassIndex = iClassID - CLASS_SCOUT;

#ifdef GAME_DLL
		CFFTeam *pTeam = GetGlobalFFTeam(iTeamID);

		// This team doesn't exist so keep all classes on 
		if (pTeam == NULL)
		{
			nClassLimits[iClassIndex] = -1;
			continue;
		}

		nClassLimits[iClassIndex] = pTeam->GetClassLimit(iClassID);
#else
		nClassLimits[iClassIndex] = pGR->GetTeamClassLimits(iTeamID, iClassID);
#endif
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get the space left on a Class
//				-1 : Class disabled completely
//				 0 : no space
//				 n : n places left
//			    99 : no space limit at all
//
//			Returns number of free class slots
//-----------------------------------------------------------------------------
int UTIL_GetClassSpaces(int iTeamID, char nSpacesRemaining[10])
{
	char nClassNumbers[10];
	UTIL_GetClassNumbers(iTeamID, nClassNumbers);

	char nClassLimits[10];
	UTIL_GetClassLimits(iTeamID, nClassLimits);

	int nFreeClasses = 0;

	// Now loop through the Classs and take different branches to find out
	// what their limits are and calculate places remaining from these
	for (int iClassID = CLASS_SCOUT; iClassID <= CLASS_CIVILIAN; iClassID++)
	{
		int iClassIndex = iClassID - CLASS_SCOUT;

		// No limit at all
		if (nClassLimits[iClassIndex] == 0)
		{
			nSpacesRemaining[iClassIndex] = 99;
			nFreeClasses++;
			continue;
		}

		// Class disabled
		if (nClassLimits[iClassIndex] == -1)
		{
			nSpacesRemaining[iClassIndex] = -1;
			continue;
		}

		nSpacesRemaining[iClassIndex] = nClassLimits[iClassIndex] - nClassNumbers[iClassIndex];

		// It shouldn't get below 0 but nevermind
		nSpacesRemaining[iClassIndex] = max(nSpacesRemaining[iClassIndex], 0);

		if (nSpacesRemaining[iClassIndex] > 0)
		{
			nFreeClasses++;
		}
	}

	return nFreeClasses;
}

#ifdef GAME_DLL

//-----------------------------------------------------------------------------
// Purpose: Fixed this so it selects random classes properly.
//-----------------------------------------------------------------------------
int UTIL_PickRandomClass(int _curteam)
{
	char cClassesAvailable[10];
	int nClassesAvailable = UTIL_GetClassSpaces(_curteam, cClassesAvailable);

	// Make a random choice for randompcs
	if (nClassesAvailable == 0)
		return 0;

	int iRandomClassNumber = random->RandomInt(0, nClassesAvailable - 1);
	int iClassIndex = 0;
	for (;;)
	{
		if (cClassesAvailable[iClassIndex] > 0)
			iRandomClassNumber--;

		if (iRandomClassNumber < 0)
			break;

		iClassIndex++;
	}

	return (iClassIndex + CLASS_SCOUT);
}

int UTIL_PickRandomTeam()
{
	int iBestTeam = -1;
	float flBestCapacity = 9999.0f;

	int iTeamNumbers[8] = {0};

	// Count the number of people each team
	for( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CFFPlayer *pPlayer = (CFFPlayer *) UTIL_PlayerByIndex( i );

		if( pPlayer )
			iTeamNumbers[pPlayer->GetTeamNumber()]++;
	}

	for( int iTeamToCheck = FF_TEAM_BLUE; iTeamToCheck <= FF_TEAM_GREEN; iTeamToCheck++ )
	{
		CFFTeam *pTeam = GetGlobalFFTeam(iTeamToCheck);

		// Don't bother with non-existant teams
		if( !pTeam )
			continue;

		// Take team limits into account when calculating the best team to join
		float flTeamCapacity = (float)iTeamNumbers[iTeamToCheck] / ( pTeam->GetTeamLimits() == 0 ? 32 : pTeam->GetTeamLimits() );

		//DevMsg( "Team %d: %d (%f)\n", iTeamToCheck, iTeamNumbers[iTeamToCheck], flTeamCapacity );

		// Is this the best team to join so far (and there is space on it)
		if( flTeamCapacity < flBestCapacity && ( pTeam->GetTeamLimits() == 0 || iTeamNumbers[iTeamToCheck] < pTeam->GetTeamLimits() ) )
		{
			flBestCapacity = flTeamCapacity;
			iBestTeam = iTeamToCheck;
		}
	}
	return iBestTeam;
}

//-----------------------------------------------------------------------------
// Purpose: Works out the position & angles of the info intermission.  
// In: pointers to existing vector & angles to be filled out.
// Returns: Nonzero for success, 0 for failure.
//-----------------------------------------------------------------------------

int UTIL_GetIntermissionData( Vector *pPosition, QAngle *pAngles )
{
	assert( pPosition && pAngles );

	// Find an info_player_start place to spawn observer mode at
	//CBaseEntity *pSpawnSpot = gEntList.FindEntityByClassname( NULL, "info_intermission");
	CPointEntity *pIntermission = dynamic_cast< CPointEntity * >( gEntList.FindEntityByClassname( NULL, "info_intermission" ) );

	// We could find one
	if( pIntermission )
	{
		// Try to point at the target of the info_intermission
		CBaseEntity *pTarget = gEntList.FindEntityByName( NULL, pIntermission->m_target );
		if( pTarget )
		{
			Vector vecDir = pTarget->GetAbsOrigin() - pIntermission->GetAbsOrigin();
			VectorNormalize( vecDir );

			QAngle vecAngles;
			VectorAngles( vecDir, vecAngles );

			*pAngles = vecAngles;
			*pPosition = pIntermission->GetAbsOrigin();

			//SetAbsOrigin( pIntermission->GetLocalOrigin() );
			//SetAbsAngles( vecAngles );

			// Send this to client so the client will
			// actually look at what we want it to!
			//m_vecInfoIntermission = vecAngles;
		}
		else
		{
			//SetLocalOrigin(pIntermission->GetLocalOrigin() + Vector(0, 0, 1));
			//SetLocalAngles(pIntermission->GetLocalAngles());
			*pAngles = pIntermission->GetAbsAngles();
			*pPosition = pIntermission->GetAbsOrigin();
		}		

		return 1;
	}
	// We couldn't find one
	else
	{
		*pPosition = Vector( 0, 0, 72 );
		*pAngles = QAngle( 0, 0, 0 );

		return 0;
	}
}

#endif
