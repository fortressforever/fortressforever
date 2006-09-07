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

#define FF_PLAYER_VIEW_OFFSET	Vector( 0, 0, 53.5 )
#define EXTRA_LOCAL_ORIGIN_ACCURACY

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
	FF_STATUSICON_MAX
};

enum FF_View_Effects_t
{
	FF_VIEWEFFECT_BURNING,
	FF_VIEWEFFECT_TRANQUILIZED,
	FF_VIEWEFFECT_INFECTED,
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

struct SpyDisguiseWeapon
{
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
	KILLTYPE_BODYSHOUT = 2
};


#ifdef CLIENT_DLL

#endif

#endif // FF_SHAREDDEFS_H
