// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildableobjects_shared.cpp
// @author Patrick O'Leary(Mulchman) 
// @date 06/08/2005
// @brief Shared code for buildable objects
//
// REVISIONS
// ---------
// 06/08/2005, Mulchman: 
//		This file First created
// 22/01/2006, Mirv:
//		Rewritten a lot of this, an instance of this object will now hold all the information
//		needed to put a buildable on the ground(including pre-working out the orientation 
//		and whether or not the orientation is okay, since that is now part of this class) 
//		 (Previously calculating the orientation was left until last and not part of the
//		buildable spot validation).
//		Also now the SG orients away from walls.
//
// 9/16/2006, Mulchman:
//		Re-jigged the building process, hopefully it's a little better now
//
//	12/6/2007, Mulchman:
//		Added man cannon stuff

#include "cbase.h"
#include "ff_buildableobjects_shared.h"

#ifdef GAME_DLL
	#include "omnibot_interface.h"	
#else
	#include "c_gib.h"
	#include "ClientEffectPrecacheSystem.h"
	#include "c_te_effect_dispatch.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define FF_BUILD_DEBUG_VISUALIZATIONS

// Array of char *'s to dispenser models
const char *g_pszFFDispenserModels[ ] =
{
	FF_DISPENSER_MODEL,
	NULL
};

// Array of char *'s to gib models
const char *g_pszFFDispenserGibModels[ ] =
{
	FF_DISPENSER_GIB01_MODEL,
	FF_DISPENSER_GIB02_MODEL,
	FF_DISPENSER_GIB03_MODEL,
	FF_DISPENSER_GIB04_MODEL,
	FF_DISPENSER_GIB05_MODEL,
	FF_DISPENSER_GIB06_MODEL,
	NULL
};

// Array of char *'s to sounds
const char *g_pszFFDispenserSounds[ ] =
{
	FF_DISPENSER_BUILD_SOUND,
	FF_DISPENSER_EXPLODE_SOUND,
	FF_DISPENSER_UNBUILD_SOUND,
	FF_DISPENSER_OMNOMNOM_SOUND,
	NULL
};

// Array of char *'s to sentrygun models
const char *g_pszFFSentryGunModels[] =
{
	FF_SENTRYGUN_MODEL, 
	FF_SENTRYGUN_MODEL_LVL2, 
	FF_SENTRYGUN_MODEL_LVL3, 
	NULL
};

// Array of char *'s to gib models
const char *g_pszFFSentryGunGibModelsL1[] =
{
	FF_SENTRYGUN_GIB1A_MODEL,
	FF_SENTRYGUN_GIB1B_MODEL,
	FF_SENTRYGUN_GIB1C_MODEL,
	FF_SENTRYGUN_GIB1D_MODEL,
	FF_SENTRYGUN_GIBTRIPOD_MODEL,
	NULL
};

const char *g_pszFFSentryGunGibModelsL2[] =
{
	FF_SENTRYGUN_GIB2A_MODEL,
	FF_SENTRYGUN_GIB2B_MODEL,
	FF_SENTRYGUN_GIB2C_MODEL,
	FF_SENTRYGUN_GIB2D_MODEL,
	FF_SENTRYGUN_GIBTRIPOD_MODEL,
	NULL
};

const char *g_pszFFSentryGunGibModelsL3[] =
{
	FF_SENTRYGUN_GIB3A_MODEL,
	FF_SENTRYGUN_GIB3B_MODEL,
	FF_SENTRYGUN_GIB3C_MODEL,
	FF_SENTRYGUN_GIB3D_MODEL,
	FF_SENTRYGUN_GIB3D_MODEL,
	FF_SENTRYGUN_GIB3E_MODEL,
	FF_SENTRYGUN_GIBTRIPOD_MODEL,
	NULL
};

// Array of char *'s to ALL sg gib models to precache
const char *g_pszFFSentryGunGibModels[] =
{
	FF_SENTRYGUN_GIB1A_MODEL,
	FF_SENTRYGUN_GIB1B_MODEL,
	FF_SENTRYGUN_GIB1C_MODEL,
	FF_SENTRYGUN_GIB1D_MODEL,
	FF_SENTRYGUN_GIB2A_MODEL,
	FF_SENTRYGUN_GIB2B_MODEL,
	FF_SENTRYGUN_GIB2C_MODEL,
	FF_SENTRYGUN_GIB2D_MODEL,
	FF_SENTRYGUN_GIB3A_MODEL,
	FF_SENTRYGUN_GIB3B_MODEL,
	FF_SENTRYGUN_GIB3C_MODEL,
	FF_SENTRYGUN_GIB3D_MODEL,
	FF_SENTRYGUN_GIB3D_MODEL,
	FF_SENTRYGUN_GIB3E_MODEL,
	FF_SENTRYGUN_GIBTRIPOD_MODEL,
	NULL
};

// Array of char *'s to sounds
const char *g_pszFFSentryGunSounds[] =
{
	FF_SENTRYGUN_BUILD_SOUND, 
	FF_SENTRYGUN_EXPLODE_SOUND, 
	"Sentry.Fire", 
	"Sentry.Spot", 
	"Sentry.Scan", 
	"Sentry.Two", 
	"Sentry.Three", 
	"Sentry.Aim",
	FF_SENTRYGUN_UNBUILD_SOUND,
	"Spanner.HitSG",
	"Sentry.RocketFire",
	"Sentry.SabotageActivate",
	//"Sentry.SabotageFinish",
	"Sentry.CloakDetection",
	"Sentry.CloakSonar",
	"DoSpark",
	NULL
};

// Array of char *'s to dispenser models
const char *g_pszFFDetpackModels[ ] =
{
	FF_DETPACK_MODEL,
	NULL
};

// Array of char *'s to gib models
const char *g_pszFFDetpackGibModels[ ] =
{
	NULL
};

// Array of char *'s to sounds
const char *g_pszFFDetpackSounds[ ] =
{
	FF_DETPACK_BUILD_SOUND,
	FF_DETPACK_EXPLODE_SOUND,
	"Detpack.FiveSeconds",
	"Detpack.Defuse",
	NULL
};

// Array of char *'s to dispenser models
const char *g_pszFFManCannonModels[] =
{
	FF_MANCANNON_MODEL,
	NULL
};

// Array of char *'s to gib models
const char *g_pszFFManCannonGibModels[] =
{
	NULL
};

// Array of char *'s to sounds
const char *g_pszFFManCannonSounds[] =
{
	FF_MANCANNON_BUILD_SOUND,
	FF_MANCANNON_EXPLODE_SOUND,
	//"JumpPad.WarmUp",
	//"JumpPad.PowerDown",
	"JumpPad.Fire",
	"JumpPad.Heal",//For the healing sound on jumppads -GreenMushy
	NULL
};

//ConVar ffdev_mancannon_combatcooldown( "ffdev_mancannon_combatcooldown", "3", FCVAR_FF_FFDEV_REPLICATED );
#define MANCANNON_COMBATCOOLDOWN 3.0f

//-----------------------------------------------------------------------------
// Purpose: Constructor - initializes a bunch of stuff and figures out if
//			we can build here or not!
//-----------------------------------------------------------------------------
CFFBuildableInfo::CFFBuildableInfo( CFFPlayer *pPlayer, int iBuildObject ) 
{
	// Default
	m_BuildResult = BUILD_ERROR;
	
	// Ack, no player?
	if( !pPlayer )
	{
		Assert( 0 );
		m_BuildResult = BUILD_NOPLAYER;		
		return;
	}

	// Set up some build parameters
	float flBuildDist = 0.0f, flOffset = 0.0f;
	switch( iBuildObject )
	{
		case FF_BUILD_DISPENSER: flBuildDist = FF_BUILD_DISP_BUILD_DIST; flOffset = -22.0f; break;
		case FF_BUILD_SENTRYGUN: flBuildDist = FF_BUILD_SG_BUILD_DIST; flOffset = -22.0f; break;
		case FF_BUILD_DETPACK: flBuildDist = FF_BUILD_DET_BUILD_DIST; break;
		case FF_BUILD_MANCANNON: flBuildDist = FF_BUILD_MC_BUILD_DIST; break;
	}

	// Player building the object
	m_pPlayer = pPlayer;
	// Object we're building
	m_iBuildObject = iBuildObject;
	// Distance in front of player to build
	m_flBuildDist = flBuildDist;

	// Check ducking
	if( m_pPlayer->GetFlags() & FL_DUCKING )
		flOffset /= 2.0f;

	// Get some info from the player...
	m_pPlayer->EyeVectors( &m_vecPlayerForward, &m_vecPlayerRight );

	// Level off
	m_vecPlayerForward.z = 0;
	m_vecPlayerRight.z = 0;

	// Normalize
	VectorNormalize( m_vecPlayerForward );
	VectorNormalize( m_vecPlayerRight );

	// Store player origin (offsetted)
	m_vecPlayerOrigin = m_pPlayer->GetAbsOrigin() + ( m_vecPlayerForward * 16.0f ) + Vector( 0, 0, flOffset );

	// Get a position in front of the player a certain distance & raise it up.
	// This is the first position we'll try to build from (offsetted)
	m_vecBuildAirOrigin = m_vecPlayerOrigin + ( m_vecPlayerForward * m_flBuildDist );	

	// Original angle is same as player's
	VectorAngles( m_vecPlayerForward, m_angBuildAirAngles );

	// Set these for the build helpers in case building fails the client
	// side helper still needs to draw an icon using valid positions
	m_vecBuildGroundOrigin = m_vecBuildAirOrigin;
	m_angBuildGroundAngles = m_angBuildAirAngles;

	switch( iBuildObject )
	{
	case FF_BUILD_DISPENSER:
		if(pPlayer->GetDispenser())
			m_BuildResult = BUILD_ALREADYBUILT;
		else if(pPlayer->GetAmmoCount( AMMO_CELLS ) < FF_BUILDCOST_DISPENSER)
			m_BuildResult = BUILD_NEEDAMMO;
		else
			break;
		return;
	case FF_BUILD_SENTRYGUN: 
		if(pPlayer->GetSentryGun())
			m_BuildResult = BUILD_ALREADYBUILT;
		else if(pPlayer->GetAmmoCount( AMMO_CELLS ) < FF_BUILDCOST_SENTRYGUN)
			m_BuildResult = BUILD_NEEDAMMO;
		else
			break;
		return;		
	case FF_BUILD_DETPACK: 
		if(pPlayer->GetDetpack())
			m_BuildResult = BUILD_ALREADYBUILT;
		else if(pPlayer->GetAmmoCount( AMMO_DETPACK ) < 1)
			m_BuildResult = BUILD_NEEDAMMO;
		else
			break;
		return;
	case FF_BUILD_MANCANNON:
		if( pPlayer->GetManCannon() )
			m_BuildResult = BUILD_ALREADYBUILT;
		else if( pPlayer->GetAmmoCount( AMMO_MANCANNON ) < 1 )
			m_BuildResult = BUILD_NEEDAMMO;
		else
			break;
	}

	// For the sg, because its so large, the hull (when doing the trace later) will stick
	// into walls if we are near them [like having our back towards them] so we need to
	// move the build origin further forward to remedy this
	if( iBuildObject == FF_BUILD_SENTRYGUN || iBuildObject == FF_BUILD_MANCANNON )
	{
		// Move forward an arbitrary amount... but don't go more
		// than 32 otherwise that leaves a gap between the players
		// bbox and the object you're building so really thin brushes
		// could be in the way and you could technically build 'through' them.
		m_vecPlayerOrigin += m_vecPlayerForward * 32.0f;
	}

	// Check if player is even on the ground or not
	if( !( m_pPlayer->GetFlags() & FL_ONGROUND ) )
	{
		m_BuildResult = BUILD_PLAYEROFFGROUND;
		return;
	}

	// Assume stuff is in the way. Next function can
	// set other build errors than just one, that's why
	// we're setting NOROOM now - in case we don't trickle
	// into one of the other errors and we do error.
	m_BuildResult = BUILD_NOROOM;

	// If we can't trace the hull
	if( IsGeometryInTheWay() )
		return;

	// If we're dealing w/ a detpack then we're finished here
	if( (m_iBuildObject == FF_BUILD_DETPACK) || (m_iBuildObject == FF_BUILD_MANCANNON) )
	{
		m_BuildResult = BUILD_ALLOWED;
		return;
	}

	// Test to see if can be oriented
	m_BuildResult = CanOrientToGround();
}

//-----------------------------------------------------------------------------
// Purpose: Checks for geometry/players where we want to build
//-----------------------------------------------------------------------------
bool CFFBuildableInfo::IsGeometryInTheWay( void )
{
	// Get the correct mins/maxs 
	Vector vecMins, vecMaxs;
	switch( m_iBuildObject )
	{
		case FF_BUILD_DISPENSER: vecMins = FF_DISPENSER_MINS; vecMaxs = FF_DISPENSER_MAXS; break;
		case FF_BUILD_SENTRYGUN: vecMins = FF_SENTRYGUN_MINS; vecMaxs = FF_SENTRYGUN_MAXS; break;
		case FF_BUILD_DETPACK:   vecMins = FF_DETPACK_MINS;   vecMaxs = FF_DETPACK_MAXS;   break;
		case FF_BUILD_MANCANNON: vecMins = FF_MANCANNON_MINS; vecMaxs = FF_MANCANNON_MAXS; break;
	}

	// We're going to do this test 3 times... building on an incline always kills us
	bool bValid = false;

	// Detpack was climbing up short walls and stuff, nice! But a bug!
	int iIterations = ( (m_iBuildObject == FF_BUILD_DETPACK) || (m_iBuildObject == FF_BUILD_MANCANNON) ) ? 1 : 3;

	for( int i = 0; ( i < iIterations ) && !bValid; i++ )
	{
		// If past the first iteration, try adjusting
		// player origin if building isn't working
		if( i > 0 )
		{
			m_vecPlayerOrigin.z += 16.0f;
		}

#ifdef FF_BUILD_DEBUG_VISUALIZATIONS
#ifdef GAME_DLL
#ifdef _DEBUG
		// Some visualizations
		if( !engine->IsDedicatedServer() )
		{
			// Show the players hull in yellow
			NDebugOverlay::Box( m_pPlayer->GetFeetOrigin(), Vector( -16, -16, 0 ), Vector( 16, 16, 70 ), 255, 255, 0, 100, 10.0f );
			// Show the hull in blue at the players origin
			NDebugOverlay::Box( m_vecPlayerOrigin, vecMins, vecMaxs, 0, 0, 255, 100, 10.0f );
			// Show the hull in red at the build origin
			NDebugOverlay::Box( m_vecBuildAirOrigin, vecMins, vecMaxs, 255, 0, 0, 100, 10.0f );
		}
#endif // _DEBUG
#endif // GAME_DLL
#endif // FF_BUILD_DEBUG_VISUALIZATIONS

		// Now, using the mins/maxs, do a trace
		// If the trace does not complete them we hit something and the area is not clear
		trace_t trHull;
		UTIL_TraceHull( m_vecPlayerOrigin, m_vecBuildAirOrigin, vecMins, vecMaxs, MASK_PLAYERSOLID, m_pPlayer, COLLISION_GROUP_BUILDABLE_BUILDING, &trHull );

		// See if we started in a solid
		if( trHull.startsolid )
		{
			// Raise build origin
			m_vecBuildAirOrigin.z += 16.0f;
			continue;
		}
		// See if the trace completed - if fraction == 1.0 then it completed
		else if( trHull.fraction != 1.0 )
		{
			// Raise build origin
			m_vecBuildAirOrigin.z += 16.0f;
			continue;
		}

		// Jiggles: Added to prevent people from building through certain displacements
		trace_t	evilDisp;
		UTIL_TraceLine( m_pPlayer->GetAbsOrigin(), m_vecBuildAirOrigin, MASK_PLAYERSOLID, m_pPlayer, COLLISION_GROUP_BUILDABLE_BUILDING, &evilDisp );
		if ( evilDisp.fraction != 1.0 )
			continue;

		bValid = true;
	}

	// Stuff was constantly blocking us
	if( !bValid )
	{
		m_BuildResult = BUILD_NOROOM;
		return true;	
	}

	// Can't build while standing on ele or object is on ele
	trace_t tr1;
	UTIL_TraceLine( m_vecPlayerOrigin, m_vecPlayerOrigin - Vector( 0, 0, 64 ), CONTENTS_MOVEABLE | CONTENTS_SOLID, m_pPlayer, COLLISION_GROUP_PLAYER, &tr1 );
	if( tr1.DidHit() && tr1.m_pEnt )
	{
		if( tr1.m_pEnt->GetMoveType() == MOVETYPE_PUSH )
		{
			m_BuildResult = BUILD_MOVEABLE;
			return true;
		}
	}

	// Check if the buildable is on an ele
	trace_t tr2;
	UTIL_TraceLine( m_vecBuildAirOrigin, m_vecBuildAirOrigin - Vector( 0, 0, 128 ), CONTENTS_MOVEABLE|CONTENTS_SOLID, m_pPlayer, COLLISION_GROUP_PLAYER, &tr2 );
	if( tr2.DidHit() && tr2.m_pEnt )
	{
		if( tr2.m_pEnt->GetMoveType() == MOVETYPE_PUSH )
		{
			m_BuildResult = BUILD_MOVEABLE;
			return true;
		}
	}

	// No geometry/objects in the way, hooray!
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Displays the reason the build failed 
//-----------------------------------------------------------------------------
void CFFBuildableInfo::GetBuildError( void )
{
	// TODO: Hook into the hint system so hints are displayed
	// and not the devmsg/warning stuff

	if( m_BuildResult != BUILD_ALLOWED )
	{
		char szError[512];
		Q_strcpy( szError, "#FF_BUILDERROR_" );

		switch( m_BuildResult ) 
		{
			case BUILD_NOPLAYER: Q_strncat(szError, "NOPLAYER", sizeof(szError)); break;
			case BUILD_NOROOM: Q_strncat(szError, "WORLDBLOCK", sizeof(szError)); break;
			case BUILD_TOOSTEEP: Q_strncat(szError, "GROUNDSTEEP", sizeof(szError)); break;
			case BUILD_TOOFAR: Q_strncat(szError, "GROUNDDISTANCE", sizeof(szError)); break;
			case BUILD_PLAYEROFFGROUND: Q_strncat( szError, "OFFGROUND", sizeof( szError ) ); break;
			case BUILD_MOVEABLE: Q_strncat( szError, "MOVEABLE", sizeof( szError ) ); break;

			default: Q_strncat(szError, "GENERIC", sizeof(szError)); break;
		}

#ifdef GAME_DLL
		ClientPrint( m_pPlayer, HUD_PRINTCENTER, szError );

		if(m_pPlayer)
		{
			Omnibot::Notify_Build_CantBuild(m_pPlayer, m_pPlayer->GetWantBuild());
		}
#endif
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns a proper angle orientation for the buidlable depending
//			on the approximate ground normal
//-----------------------------------------------------------------------------
QAngle OrientToVectors(const Vector &vecGroundNormal, const Vector &vecPlayerForward) 
{
	// Get correct forward & right vector
	Vector vecUp = vecGroundNormal;
	Vector vecRight = CrossProduct(vecUp, -vecPlayerForward);
	Vector vecForward = CrossProduct(vecUp, vecRight);

	// Normalise(jic) 
	VectorNormalize(vecForward);
	VectorNormalize(vecRight);
	VectorNormalize(vecUp);

	// Quaternions are the handiest things
	Quaternion q;
	BasisToQuaternion(vecForward, vecRight, vecUp, q);

	// Now for the angles
	QAngle angNew;
	QuaternionAngles(q, angNew);

	return angNew;
}

//-----------------------------------------------------------------------------
// Purpose: Vectors must be	0 1
//							2 3
//-----------------------------------------------------------------------------
void ComputeRectangularPlane(const Vector &v0, const Vector &v1, const Vector &v2, const Vector &v3, Vector &vOut) 
{
	Vector vecOne = v0 - v3;
	Vector vecTwo = v2 - v1;

	VectorNormalize(vecOne);
	VectorNormalize(vecTwo);

	vOut = CrossProduct(vecOne, vecTwo);

	VectorNormalize(vOut);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Vector GetMiddle(const Vector *v1, const Vector *v2, const Vector *v3 = NULL, const Vector *v4 = NULL) 
{
	float c = 2.0f;
	Vector vecMiddle = *v1 + *v2;

	if (v3) 
	{
		vecMiddle += *v3;
		c++;
	}
	if (v4) 
	{
		vecMiddle += *v4;
		c++;
	}

	vecMiddle /= c;

	return vecMiddle;
}

//-----------------------------------------------------------------------------
// Purpose: Sees if a buildable can be placed on the ground
//-----------------------------------------------------------------------------
BuildInfoResult_t CFFBuildableInfo::CanOrientToGround( void ) 
{
	Vector vecNormal, vecGround;
	float flTestDist = 64.0f;

	switch( m_iBuildObject )
	{
		case FF_BUILD_DISPENSER:
		{
			Vector vecRightAdjWidth = m_vecPlayerRight * FF_BUILD_DISP_HALF_WIDTH;
			Vector vecForwardAdjWidth = m_vecPlayerForward * FF_BUILD_DISP_HALF_WIDTH;

			// These are object aligned positions - to orient us to the ground correctly
			Vector vecCorners[ 4 ];
			vecCorners[ 0 ] = m_vecBuildAirOrigin + ( vecRightAdjWidth ) + ( vecForwardAdjWidth ) + Vector( 0, 0, 2.0f ); // Front right corner
			vecCorners[ 1 ] = m_vecBuildAirOrigin + ( vecRightAdjWidth ) - ( vecForwardAdjWidth ) + Vector( 0, 0, 2.0f ); // Back right corner
			vecCorners[ 2 ] = m_vecBuildAirOrigin - ( vecRightAdjWidth ) + ( vecForwardAdjWidth ) + Vector( 0, 0, 2.0f ); // Front left corner
			vecCorners[ 3 ] = m_vecBuildAirOrigin - ( vecRightAdjWidth ) - ( vecForwardAdjWidth ) + Vector( 0, 0, 2.0f ); // Back left corner

#ifdef FF_BUILD_DEBUG_VISUALIZATIONS
#ifdef GAME_DLL
#ifdef _DEBUG
			// Some visualizations
			if( !engine->IsDedicatedServer() )
			{				
				for( int j = 0; j < 4; j++ )
				{
					// Draw object aligned corners in blue
					NDebugOverlay::Line( vecCorners[ j ], vecCorners[ j ] - Vector( 0, 0, flTestDist ), 0, 0, 255, false, 10.0f );
				}
			}
#endif // _DEBUG
#endif // GAME_DLL
#endif // FF_BUILD_DEBUG_VISUALIZATIONS

			// For feet traces to come
			trace_t tr[ 4 ];

			// Loop through and do traces from each corner of the dispenser to the ground
			// The mask is normal PLAYER_SOLID minus PLAYER_CLIP as we don't want to build on those(Bug #0000185: Buildable objects can be built on clips.) 
			// HACK Changed to COLLISION_GROUP_PROJECTILE so it doesn't clip healthpacks (Bug #0000242: SG/Disp when building clips on health pack.)
			for( int i = 0; i < 4; i++ ) 
			{
				UTIL_TraceLine(vecCorners[i], vecCorners[i] - Vector(0, 0, flTestDist), CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE, m_pPlayer, COLLISION_GROUP_BUILDABLE_BUILDING, &tr[i]);				

				// Bug #0000246: Dispenser and sg overlap if built on each other
				if( !tr[i].DidHit() )
					return BUILD_TOOFAR;
				else if( tr[i].startsolid )
					return BUILD_NOROOM;
				// squeek: allow dispensers to be built on other buildables
				//else if( ( tr[i].m_pEnt->Classify() == CLASS_SENTRYGUN ) || ( tr[i].m_pEnt->Classify() == CLASS_DISPENSER ) )
				//	return BUILD_NOROOM;
			}

			// Make an X shaped vector from our 4 corner position traces and do the cross product
			ComputeRectangularPlane( tr[0].endpos, tr[1].endpos, tr[2].endpos, tr[3].endpos, vecNormal );
			vecGround = GetMiddle( &tr[0].endpos, &tr[1].endpos, &tr[2].endpos, &tr[3].endpos );
		}
		break;

		case FF_BUILD_SENTRYGUN:
		{
			// Trace out in front of us from build origin (but raised a little as building
			// on steep ramps will auto rotate us cause the forward trace is hitting the ramp)
			trace_t tr_fwd;
			UTIL_TraceLine( m_vecBuildAirOrigin, 
				m_vecBuildAirOrigin + ( m_vecPlayerForward * 96.0f ) + Vector( 0, 0, 32.0f ), 
				MASK_SHOT, NULL, COLLISION_GROUP_NONE, &tr_fwd );

			// If we hit a wall then we want to face towards us instead
			if( tr_fwd.DidHit() ) 
			{				
				m_vecPlayerForward *= -1;
				// Redo the right vector (was making the normal negative later on)
				m_vecPlayerRight = CrossProduct( m_vecPlayerForward, Vector( 0, 0, 1 ) );
				VectorAngles( m_vecPlayerForward, m_angBuildAirAngles );
			}

			Vector vecRightAdjWidth = m_vecPlayerRight * 4.5f;//18.0f;
			Vector vecForwardAdjWidth = m_vecPlayerForward * 2.0f;//8.0f;

			// Set up these vectors to reflect the position above the ground at each
			// of the feet of the sg where we want to start traces from
			Vector vecFeet[ 3 ];
			vecFeet[ 0 ] = m_vecBuildAirOrigin + ( m_vecPlayerForward * 5.0f );//20.0f );
			vecFeet[ 1 ] = m_vecBuildAirOrigin - ( vecRightAdjWidth ) - ( vecForwardAdjWidth );
			vecFeet[ 2 ] = m_vecBuildAirOrigin + ( vecRightAdjWidth ) - ( vecForwardAdjWidth );

#ifdef FF_BUILD_DEBUG_VISUALIZATIONS
#ifdef GAME_DLL
#ifdef _DEBUG
			// Some visualizations
			if( !engine->IsDedicatedServer() )
			{
				// Trace down from the feet in the best colors in the world
				for( int j = 0; j < 3; j++ )
				{
					NDebugOverlay::Line( vecFeet[ j ], vecFeet[ j ] - Vector( 0, 0, flTestDist ), ( ( ( j == 0 ) || ( j == 1 ) ) ? 255 : 0 ), ( ( j == 1 ) ? 255 : 0 ), ( ( ( j == 1 ) || ( j == 2 ) ) ? 255 : 0 ), false, 10.0f );
				}
			}
#endif // _DEBUG
#endif // GAME_DLL
#endif // FF_BUILD_DEBUG_VISUALIZATIONS

			// For feet traces to come
			trace_t tr[3];

			// Loop through and do traces from each corner of the sentry to the ground
			// The mask is normal PLAYER_SOLID minus PLAYER_CLIP and CONTENTS_MOVEABLE as we don't want to build on those(Bug #0000185: Buildable objects can be built on clips.) 
			// HACK Changed to COLLISION_GROUP_PROJECTILE so it doesn't clip healthpacks (Bug #0000242: SG/Disp when building clips on health pack.)
			for (int i = 0; i < 3; i++) 
			{
				UTIL_TraceLine(vecFeet[i], vecFeet[i] - Vector(0, 0, flTestDist), CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE, m_pPlayer, COLLISION_GROUP_BUILDABLE_BUILDING, &tr[i]);

				// Bug #0000246: Dispenser and sg overlap if built on each other
				if( !tr[i].DidHit() )
					return BUILD_TOOFAR;
				else if( tr[i].startsolid )
					return BUILD_NOROOM;
				// squeek: allow SGs to be built on other buildables
				//else if( ( tr[i].m_pEnt->Classify() == CLASS_SENTRYGUN ) || ( tr[i].m_pEnt->Classify() == CLASS_DISPENSER ) )
				//	return BUILD_NOROOM;
			}

			float flIntercept;
			ComputeTrianglePlane( tr[0].endpos, tr[1].endpos, tr[2].endpos, vecNormal, flIntercept );
			vecGround = GetMiddle( &tr[0].endpos, &tr[1].endpos, &tr[2].endpos );
		}
		break;

		// Shouldn't get here w/ the detpack
		case FF_BUILD_DETPACK: Assert( 0 ); return BUILD_ALLOWED; break;
		case FF_BUILD_MANCANNON: Assert( 0 ); return BUILD_ALLOWED; break;

		default: AssertMsg( 0, "Invalid buildable!" ); return BUILD_ERROR; break;
	}

	float flEpsilon = vecNormal.z / VectorLength( vecNormal );

	// AfterShock: comment these 2 lines for near vertical building (but this won't allow you to build on walls unfortunately :)
	if (flEpsilon < 0.90f) 
		return BUILD_TOOSTEEP;

	m_angBuildGroundAngles = OrientToVectors( vecNormal, m_vecPlayerForward );
	m_vecBuildGroundOrigin = vecGround;

	return BUILD_ALLOWED;
}

//-----------------------------------------------------------------------------
// Purpose: Get health as a percentage
//-----------------------------------------------------------------------------
int CFFBuildableObject::GetHealthPercent( void ) const
{
	float flPercent = ( ( float )GetHealth() / ( float )GetMaxHealth() ) * 100.0f;

	// Don't display 0% it looks stupid
	if (flPercent < 1.0f)
		return 1;
	else
		return ( int )flPercent;
}

//-----------------------------------------------------------------------------
// Purpose: Team accessor [mirv]
//-----------------------------------------------------------------------------
int CFFBuildableObject::GetTeamNumber()
{
	CFFPlayer *pOwner = GetOwnerPlayer();

	if (!pOwner)
		return TEAM_UNASSIGNED;

	return pOwner->GetTeamNumber();
}

//-----------------------------------------------------------------------------
// Purpose: Get a buildables owner
//-----------------------------------------------------------------------------
CFFPlayer *CFFBuildableObject::GetOwnerPlayer( void )
{
	if( m_hOwner.Get() )
		return ToFFPlayer( m_hOwner.Get() );

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Get a buildables team
//-----------------------------------------------------------------------------
CFFTeam *CFFBuildableObject::GetOwnerTeam( void )
{
	CFFPlayer *pOwner = GetOwnerPlayer();
	if( pOwner )
	{
#ifdef _DEBUG
		Assert( dynamic_cast< CFFTeam * >( pOwner->GetTeam() ) != 0 );
#endif
		return static_cast< CFFTeam * >( pOwner->GetTeam() );
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Get a buildables team id
//-----------------------------------------------------------------------------
int CFFBuildableObject::GetOwnerTeamId( void )
{
	CFFPlayer *pOwner = GetOwnerPlayer();
	if( pOwner )
		return pOwner->GetTeamNumber();

	return TEAM_UNASSIGNED;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFDispenser::CFFDispenser( void )
{
#ifdef GAME_DLL
	// Overwrite the base class stubs
	m_ppszModels = g_pszFFDispenserModels;
	m_ppszGibModels = g_pszFFDispenserGibModels;
	m_ppszSounds = g_pszFFDispenserSounds;

	// Time in seconds between generating shiz
	m_flThinkTime = 10.0f;

	// Initialize
	m_pLastTouch = NULL;
	m_flLastTouch = 0.0f;

	// Store a value from the base class
	m_flOrigExplosionMagnitude = m_flExplosionMagnitude;

	m_flLastClientUpdate = 0;
	m_iLastState = 0;
#endif

	// Initial values
	m_iCells = 30;
	m_iNails = 40;
	m_iShells = 40;
	m_iRockets = 20;
	m_iArmor = 40;

	// Max values
	m_iMaxCells		= 400;
	m_iMaxNails		= 500;
	m_iMaxShells	= 400;
	m_iMaxRockets	= 250;
	m_iMaxArmor		= 500;

	// Give values - values to give a player when they touch us
	m_iGiveCells	= 20; // Give engies 75, though
	m_iGiveNails	= 40;
	m_iGiveShells	= 40;
	m_iGiveRockets	= 20;
	m_iGiveArmor	= 40;

	// Health
	m_iMaxHealth = m_iHealth = 150;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CFFDispenser::~CFFDispenser( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFManCannon::CFFManCannon( void )
{
#ifdef GAME_DLL
	// Overwrite the base class stubs
	m_ppszModels = g_pszFFManCannonModels;
	m_ppszGibModels = g_pszFFManCannonGibModels;
	m_ppszSounds = g_pszFFManCannonSounds;

	m_bUsePhysics = true;
	m_flLastClientUpdate = 0;
	m_iLastState = 0;
#endif

	// Health
	m_iMaxHealth = m_iHealth = 100;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CFFManCannon::~CFFManCannon( void )
{
}

#ifdef CLIENT_DLL

void DispenserGib_Callback(const CEffectData &data)
{
	Vector vecPosition = data.m_vOrigin;
	Vector vecOffset;
	int nSkin = data.m_nMaterial;

	// Now spawn a number of gibs
	int iGib = 0;
	while (g_pszFFDispenserGibModels[iGib])
	{
		C_Gib *pGib = C_Gib::CreateClientsideGib(g_pszFFDispenserGibModels[iGib], vecPosition, Vector(random->RandomFloat(-150, 150), random->RandomFloat(-150, 150), random->RandomFloat(100, 400)), RandomAngularImpulse( -60, 60 ), 10.0f);
	
		if (pGib)
		{
			pGib->m_nSkin = nSkin;
			pGib->LeaveBloodDecal(false);
		}
		++iGib;
	}
}

DECLARE_CLIENT_EFFECT("DispenserGib", DispenserGib_Callback);

void SentryGunGib_Callback(const CEffectData &data)
{
	Vector vecPosition = data.m_vOrigin;
	Vector vecOffset;
	int nSkin = data.m_nMaterial;

	// Now spawn a number of gibs
	int iGib = 0;
	if (data.m_nDamageType == 1) //HACK: Using m_nDamageType as SG level
	{
		while (g_pszFFSentryGunGibModelsL1[iGib])
		{
			C_Gib *pGib = C_Gib::CreateClientsideGib(g_pszFFSentryGunGibModelsL1[iGib], vecPosition, Vector(random->RandomFloat(-150, 150), random->RandomFloat(-150, 150), random->RandomFloat(100, 400)), RandomAngularImpulse( -90, 90 ), 4.0f);
		
			if (pGib)
			{
				pGib->m_nSkin = nSkin;
				pGib->LeaveBloodDecal(false);
			}
			++iGib;
		}
	}
	else if (data.m_nDamageType == 2) //HACK: Using m_nDamageType as SG level
	{
		while (g_pszFFSentryGunGibModelsL2[iGib])
		{
			C_Gib *pGib = C_Gib::CreateClientsideGib(g_pszFFSentryGunGibModelsL2[iGib], vecPosition, Vector(random->RandomFloat(-150, 150), random->RandomFloat(-150, 150), random->RandomFloat(100, 500)), RandomAngularImpulse( -90, 90 ), 4.0f);
		
			if (pGib)
			{
				pGib->m_nSkin = nSkin;
				pGib->LeaveBloodDecal(false);
			}
			++iGib;
		}
	}
	else if (data.m_nDamageType == 3) //HACK: Using m_nDamageType as SG level
	{
		while (g_pszFFSentryGunGibModelsL3[iGib])
		{
			C_Gib *pGib = C_Gib::CreateClientsideGib(g_pszFFSentryGunGibModelsL3[iGib], vecPosition, Vector(random->RandomFloat(-150, 150), random->RandomFloat(-150, 150), random->RandomFloat(100, 600)), RandomAngularImpulse( -90, 90 ), 4.0f);
		
			if (pGib)
			{
				pGib->m_nSkin = nSkin;
				pGib->LeaveBloodDecal(false);
			}
			++iGib;
		}
	}
}

DECLARE_CLIENT_EFFECT("SentryGunGib", SentryGunGib_Callback);

#endif
