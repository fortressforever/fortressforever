// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildableobjects_shared.h
// @author Patrick O'Leary (Mulchman)
// @date 06/08/2005
// @brief Shared code for buildable objects
//
// REVISIONS
// ---------
// 06/08/2005, Mulchman: 
//		This file First created
// 22/01/2006, Mirv:
//		Rewritten a lot of this

#ifndef FF_BUILDABLEOBJECTS_SHARED_H
#define FF_BUILDABLEOBJECTS_SHARED_H

#ifdef _WIN32
	#pragma once
#endif

#ifdef CLIENT_DLL
	#define CFFPlayer C_FFPlayer
	#define CFFBuildableInfo C_FFBuildableInfo
	#include "c_ff_player.h"
#else
	#include "ff_player.h"
#endif

#define FF_DISPENSER_MODEL					"models/buildable/dispenser/dispenser.mdl"
#define FF_DISPENSER_GIB01_MODEL			"models/gibs/dispenser/gib1.mdl"
#define FF_DISPENSER_GIB02_MODEL			"models/gibs/dispenser/gib2.mdl"
#define FF_DISPENSER_GIB03_MODEL			"models/gibs/dispenser/gib3.mdl"
#define FF_DISPENSER_GIB04_MODEL			"models/gibs/dispenser/gib4.mdl"
#define FF_DISPENSER_BUILD_SOUND			"Dispenser.Build"
#define FF_DISPENSER_EXPLODE_SOUND			"Dispenser.Explode"

#define FF_DETPACK_MODEL					"models/buildable/detpack/detpack.mdl"
#define FF_DETPACK_BUILD_SOUND				"Detpack.Build"
#define FF_DETPACK_EXPLODE_SOUND			"Detpack.Explode"

#define FF_SENTRYGUN_MODEL					"models/buildable/sg/sg_lvl1.mdl"
#define FF_SENTRYGUN_MODEL_LVL2			"models/buildable/sg/sg_lvl2.mdl"
#define FF_SENTRYGUN_MODEL_LVL3			"models/buildable/sg/sg_lvl2.mdl"
#define FF_SENTRYGUN_BUILD_SOUND			"Sentry.One"
#define FF_SENTRYGUN_EXPLODE_SOUND			"Sentry.Explode"

//#define FF_SENTRYGUN_AIMSPHERE_MODEL		"models/buildable/sg/sentrygun_aimsphere.mdl"

#define FF_SEVTEST_MODEL					"models/weapons/w_missile.mdl"

#define FF_BUILDALBE_GENERIC_GIB_MODEL_01	"models/gibs/random/randGib1.mdl"
#define FF_BUILDALBE_GENERIC_GIB_MODEL_02	"models/gibs/random/randGib2.mdl"
#define FF_BUILDALBE_GENERIC_GIB_MODEL_03	"models/gibs/random/randGib3.mdl"
#define FF_BUILDALBE_GENERIC_GIB_MODEL_04	"models/gibs/random/randGib4.mdl"
#define FF_BUILDALBE_GENERIC_GIB_MODEL_05	"models/gibs/random/randGib5.mdl"

#define FF_BUILDABLE_TIMER_BUILD_STRING		"FF_Building"
#define FF_BUILDABLE_TIMER_DETPACK_STRING	"FF_Detpack_Primed"

#define FF_BUILD_NONE		0
#define FF_BUILD_DISPENSER	1
#define FF_BUILD_SENTRYGUN	2
#define FF_BUILD_DETPACK	3

#define FF_BUILD_DISP_BUILD_DIST	36.0f
#define FF_BUILD_DISP_RAISE_VAL		16.0f
#define FF_BUILD_SG_BUILD_DIST		64.0f //54.0f
#define FF_BUILD_SG_RAISE_VAL		16.0f
#define FF_BUILD_DET_BUILD_DIST		42.0f
#define FF_BUILD_DET_RAISE_VAL		48.0f

#define FF_BUILD_DISP_STRING_LEN	256

// 30 Degrees
#define FF_BUILD_EPSILON			0.13f

// Using this value based off of the mins/maxs
#define FF_BUILD_DISP_HALF_WIDTH	12.0f

#define FF_DISPENSER_MINS	Vector( -12, -12, 0 )
#define FF_DISPENSER_MAXS	Vector( 12, 12, 48 )

#define FF_SENTRYGUN_MINS	Vector( -24, -21, 0 )
#define FF_SENTRYGUN_MAXS	Vector( 24, 21, 43 )

#define FF_DETPACK_MINS		Vector( -14, -14, 0 )
#define FF_DETPACK_MAXS		Vector( 14, 14, 11 )

#define FF_SOUND_BUILD		0	// Don't change these two values
#define FF_SOUND_EXPLODE	1

// Radius of the aimsphere model
//#define FF_SENTRYGUN_AIMSPHERE_RADIUS	128.0f
//#define FF_SENTRYGUN_AIMSPHERE_VISIBLE	(FF_SENTRYGUN_AIMSPHERE_RADIUS + 64.0f)

// Currently only the server uses these...
#ifdef CLIENT_DLL 
#else
	extern const char *g_pszFFDispenserModels[ ];
	extern const char *g_pszFFDispenserGibModels[ ];
	extern const char *g_pszFFDispenserSounds[ ];

	extern const char *g_pszFFDetpackModels[ ];
	extern const char *g_pszFFDetpackGibModels[ ];
	extern const char *g_pszFFDetpackSounds[ ];

	extern const char *g_pszFFSentryGunModels[ ];
	extern const char *g_pszFFSentryGunGibModels[ ];
	extern const char *g_pszFFSentryGunSounds[ ];

	extern const char *g_pszFFSevTestModels[ ];
	extern const char *g_pszFFSevTestGibModels[ ];
	extern const char *g_pszFFSevTestSounds[ ];

	extern const char *g_pszFFGenGibModels[ ];
#endif

enum BuildInfoResult_t
{
	BUILD_ALLOWED = 0,

	BUILD_NOROOM,
	BUILD_NOPLAYER,
	BUILD_TOOSTEEP,
	BUILD_TOOFAR,
	BUILD_INVALIDGROUND,

	BUILD_ERROR
};

class CFFBuildableInfo
{
private:
	CFFBuildableInfo( void ) {}

public:
	CFFBuildableInfo( CFFPlayer *pPlayer, int iBuildObject, float flBuildDist, float m_flRaiseVal );
	~CFFBuildableInfo( void ) {}

	// Returns true if the area in front of the player is
	// able to be built on (for the particular object trying
	// to be built)
	BuildInfoResult_t BuildResult( void ) const { return m_BuildResult; }

	Vector	GetBuildAirOrigin( void ) const { return m_vecBuildAirOrigin; }
	QAngle	GetBuildAirAngles( void ) const { return m_angBuildAirAngles; }

	Vector	GetBuildGroundOrigin( void ) const { return m_vecBuildGroundOrigin; }
	QAngle	GetBuildGroundAngles( void ) const { return m_angBuildGroundAngles; }

	//int OrientBuildableToGround( CBaseEntity *pEntity, int iBuildObject );

	// Tells the player why they couldn't build
	void	GetBuildError( void );

protected:
	// If the area in front of the player is able to be built on
	//bool	m_bBuildAreaClear;
	// Type of object we're trying to build
	int		m_iBuildObject;

	float	m_flBuildDist;
	float	m_flRaiseVal;
	float	m_flTestDist;

	// Player's info
	CFFPlayer *m_pPlayer;
	Vector	m_vecPlayerForward;
	Vector	m_vecPlayerRight;
	Vector	m_vecPlayerOrigin;

	// Since we spawn the object in the air when building
	// then drop it we need to have these
	Vector	m_vecBuildAirOrigin;
	QAngle	m_angBuildAirAngles;

	// Final position/angle on the ground of the object (if it can be built, of course)
	Vector	m_vecBuildGroundOrigin;
	QAngle	m_angBuildGroundAngles;

	// For the error parsing
	//int		m_iBuildError;

	BuildInfoResult_t	m_BuildResult;

protected:
	bool				IsGeometryInTheWay();
	//bool				IsGroundTooSteep();
	BuildInfoResult_t	CanOrientToGround();

};

#endif // FF_BUILDABLEOBJECTS_SHARED_H
