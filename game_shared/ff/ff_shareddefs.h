//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef FF_SHAREDDEFS_H
#define FF_SHAREDDEFS_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
	#include "materialsystem/IMaterialSystem.h"
	#include "materialsystem/IMesh.h"
	#include "ClientEffectPrecacheSystem.h"
	#include "view_scene.h"
#endif

#define FF_PLAYER_VIEW_OFFSET	Vector( 0, 0, 53.5 )
#define EXTRA_LOCAL_ORIGIN_ACCURACY

// Leave this commented out unless you're compiling for the beta testers!
// Mulch or Mirv should be the only ones messing with this!
//#define FF_BETA_TEST_COMPILE
#define FF_BASTARD_HACKERS true

extern ConVar ffdev_spy_maxcloakspeed;

enum FFPlayerGrenadeState
{
    FF_GREN_NONE,
    FF_GREN_PRIMEONE,
    FF_GREN_PRIMETWO
};

enum FFStatusIconTypes
{
    FF_STATUSICON_CONCUSSION,
    FF_STATUSICON_INFECTION,
    FF_STATUSICON_LEGINJURY,
	FF_STATUSICON_CALTROPPED,
	FF_STATUSICON_TRANQUILIZED,
	FF_STATUSICON_HALLUCINATIONS,
	FF_STATUSICON_BURNING,
	FF_STATUSICON_DROWNING,
	FF_STATUSICON_RADIATION,
	FF_STATUSICON_COLD,
	FF_STATUSICON_IMMUNE,
	FF_STATUSICON_RADIOTAG,
	FF_STATUSICON_BURNING1,
	FF_STATUSICON_BURNING2,
	FF_STATUSICON_BURNING3,
	FF_STATUSICON_LOCKEDON,
	FF_STATUSICON_MAX
};

enum FF_View_Effects_t
{
	FF_VIEWEFFECT_BURNING,
	FF_VIEWEFFECT_TRANQUILIZED,
	FF_VIEWEFFECT_INFECTED,
	FF_VIEWEFFECT_GASSED,
	FF_VIEWEFFECT_MAX
};

// LUA Effect Flags
// Some of these just map to speed effect values
// but the mapping is through code and not actual
// integer values
enum LuaEffectType
{
	LUA_EF_ONFIRE = 0,		// put a player on fire
	LUA_EF_CONC,				// concuss a player
	LUA_EF_GAS,				// gas a player
	LUA_EF_INFECT,			// infect a player
	LUA_EF_RADIOTAG,		// radio tag a player
	LUA_EF_HEADSHOT,		// player is headshotted (currently only by sniperrifle)
	LUA_EF_LEGSHOT,			// left shot a player
	LUA_EF_TRANQ,			// tranq a player
	LUA_EF_CALTROP,			// caltrop a player
	LUA_EF_ACSPINUP,		// ac spinning up
	LUA_EF_SNIPERRIFLE,		// player slowed due to sniper rifle charging
	LUA_EF_SPEED_LUA1,		// custom speed effect (etc.)
	LUA_EF_SPEED_LUA2,
	LUA_EF_SPEED_LUA3,
	LUA_EF_SPEED_LUA4,
	LUA_EF_SPEED_LUA5,
	LUA_EF_SPEED_LUA6,
	LUA_EF_SPEED_LUA7,
	LUA_EF_SPEED_LUA8,
	LUA_EF_SPEED_LUA9,
	LUA_EF_SPEED_LUA10,

	LUA_EF_MAX_FLAG
};

/////////////////////////////////////////////////////////////////////////////
// Purpose: Colors in lua
/////////////////////////////////////////////////////////////////////////////
enum LuaColors
{
	LUA_COLOR_DEFAULT = 0,
	LUA_COLOR_BLUE,
	LUA_COLOR_RED,
	LUA_COLOR_YELLOW,
	LUA_COLOR_GREEN,
	LUA_COLOR_WHITE,
	LUA_COLOR_BLACK,
	LUA_COLOR_ORANGE,
	LUA_COLOR_PINK,
	LUA_COLOR_PURPLE,
	LUA_COLOR_GREY,

	LUA_COLOR_INVALID,
};

struct SpyDisguiseWeapon
{
	char szWeaponClassName[6][MAX_WEAPON_STRING];
	char szWeaponModel[6][MAX_WEAPON_STRING];
	char szAnimExt[6][MAX_WEAPON_PREFIX];
};

// Decapitation flags
// These should be in the same order as the attachments + bodygroups in the model
#define DECAP_HEAD			( 1 << 0 )
#define DECAP_LEFT_ARM		( 1 << 1 )
#define DECAP_RIGHT_ARM		( 1 << 2 )
#define DECAP_LEFT_LEG		( 1 << 3 )
#define DECAP_RIGHT_LEG		( 1 << 4 )

// Some kill modifiers.
enum KillTypes_t
{
	KILLTYPE_HEADSHOT = 1,
	KILLTYPE_BODYSHOUT,
	KILLTYPE_INFECTION,
	KILLTYPE_BURN_LEVEL1,
	KILLTYPE_BURN_LEVEL2,
	KILLTYPE_BURN_LEVEL3,
	KILLTYPE_GASSED,
	KILLTYPE_BACKSTAB,
	KILLTYPE_SENTRYGUN_DET,
	KILLTYPE_HEADCRUSH
};

extern ConVar sniperrifle_chargetime;
#define FF_SNIPER_MAXCHARGE sniperrifle_chargetime.GetFloat()

#endif // FF_SHAREDDEFS_H
