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
char *Class_GetModelByInt( int iClassIndex );
void SetColorByTeam( int iTeam, Color& cColor );

int FF_NumPlayersOnTeam( int iTeam );
int FF_GetPlayerOnTeam( int iTeam, int iNum );
int FF_NumPlayers( );
int FF_GetPlayer( int iNum );

void FF_DecalTrace( CBaseEntity *pEntity, float flRadius, const char *pszDecalName );

// Do a HudHint
void FF_HudHint(
#ifndef CLIENT_DLL 
				CFFPlayer *pPlayer,
#endif
				const char *pszMessage );

#define BEGIN_ENTITY_SPHERE_QUERY( origin, radius ) CBaseEntity *pEntity = NULL; \
	CFFPlayer *pPlayer = NULL; \
	for ( CEntitySphereQuery sphere( GetAbsOrigin(), radius); (pEntity = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity() ) \
	{ \
		pPlayer = ToFFPlayer( pEntity );

#define END_ENTITY_SPHERE_QUERY( ) }

const char *FF_GetAmmoName(int i);

#endif // FF_UTILS_H
