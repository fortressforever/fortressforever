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
//		w/ the new buildable object slots (dispenser & sg slot on engy).
//		Modified the build code a bunch, too - completely overhauled.
//
//	06/08/2005, Mulchman:
//		Put more buildable object stuff in here to make it universal
//		and easier to update client/server stuff in one file. Ended
//		up moving buildable stuff out to a buildable shared file.
//	06/18/2005, L0ki:
//		Added a utility macro for sphere queries, mainly for the grenades
//
//	05/28/2006, Mulchman:
//		Added FF_DecalTrace

#ifndef FF_UTILS_H
#define FF_UTILS_H

#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
	#define CFFPlayer C_FFPlayer
	#include "c_ff_player.h"	

	// Once initialized, a pointer to
	// our client side Hud Hint class
	#include "ff_hud_hint.h"
	extern CHudHint *pHudHintHelper;
#else
	#include "ff_player.h"
#endif

// Distance to trace from an entity outward when
// trying to hit solids for placing scorch marks
#define FF_DECALTRACE_TRACE_DIST	48.0f

int Class_StringToInt( const char *szClassName );
const char *Class_IntToString( int iClassIndex );
const char *Class_IntToResourceString( int iClassIndex );
const char *Class_IntToPrintString( int iClassIndex );
void SetColorByTeam( int iTeam, Color& cColor );

int FF_NumPlayersOnTeam( int iTeam );
int FF_GetPlayerOnTeam( int iTeam, int iNum );
int FF_NumPlayers( );
int FF_GetPlayer( int iNum );

void UTIL_GetTeamNumbers(char nTeamNumbers[4]);
void UTIL_GetTeamLimits(char nTeamLimits[4]);
int UTIL_GetTeamSpaces(char nSpacesRemaining[4]);

void UTIL_GetClassNumbers(int iTeamID, char nClassNumbers[10]);
void UTIL_GetClassLimits(int iTeamID, char nClassLimits[10]);
int UTIL_GetClassSpaces(int iTeamID, char nSpacesRemaining[10]);

bool IsPlayerRadioTagTarget( CFFPlayer *pPlayer, int iTeamDoingTargetting );

void FF_DecalTrace( CBaseEntity *pEntity, float flRadius, const char *pszDecalName );

#ifdef GAME_DLL
void FF_LuaHudText(CFFPlayer *pPlayer, const char *pszIdentifier, int x, int y, const char *pszText);
void FF_LuaHudIcon(CFFPlayer *pPlayer, const char *pszIdentifier, int x, int y, const char *pszImage, int iWidth = 0, int iHeight = 0);
void FF_LuaHudTimer(CFFPlayer *pPlayer, const char *pszIdentifier, int x, int y, int iStartValue, float flSpeed);
void FF_LuaHudRemove(CFFPlayer *pPlayer, const char *pszIdentifier);
#endif

bool FF_IsPlayerSpec( CFFPlayer *pPlayer );
bool FF_HasPlayerPickedClass( CFFPlayer *pPlayer );

// Do a HudHint
void FF_HudHint(
#ifndef CLIENT_DLL 
				CFFPlayer *pPlayer,
#endif
				byte bType,
				unsigned short wID,
				const char *pszMessage,
				const char *pszSound = NULL);

#define BEGIN_ENTITY_SPHERE_QUERY( origin, radius ) CBaseEntity *pEntity = NULL; \
	CFFPlayer *pPlayer = NULL; \
	for ( CEntitySphereQuery sphere( GetAbsOrigin(), radius); (pEntity = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity() ) \
	{ \
		if( pEntity->IsPlayer() ) \
			pPlayer = ToFFPlayer( pEntity );

#define END_ENTITY_SPHERE_QUERY( ) }

const char *FF_GetAmmoName(int i);

bool FF_TraceHitWorld( trace_t *pTrace );

#endif // FF_UTILS_H
