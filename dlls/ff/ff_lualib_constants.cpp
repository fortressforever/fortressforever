
// ff_lualib_constants.cpp

//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib_constants.h"
#include "ff_lualib.h"
#include "ff_entity_system.h"
#include "ff_gamerules.h"
#include "ff_shareddefs.h"
#include "ff_item_flag.h"
#include "ff_triggerclip.h"

#include "ammodef.h"

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "luabind/luabind.hpp"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------------------------
using namespace luabind;

/////////////////////////////////////////////////////////////////////////////-
// Purpose: Convert lua ammo type (int) to game ammo type (string)
/////////////////////////////////////////////////////////////////////////////-
const char *LookupLuaAmmo( int iLuaAmmoType )
{
	switch( iLuaAmmoType )
	{
		case LUA_AMMO_SHELLS: return AMMO_SHELLS; break;
		case LUA_AMMO_CELLS: return AMMO_CELLS; break;
		case LUA_AMMO_NAILS: return AMMO_NAILS; break;
		case LUA_AMMO_ROCKETS: return AMMO_ROCKETS; break;
		case LUA_AMMO_DETPACK: return AMMO_DETPACK; break;
		case LUA_AMMO_MANCANNON: return AMMO_MANCANNON; break;
		case LUA_AMMO_GREN1: return AMMO_GREN1; break;
		case LUA_AMMO_GREN2: return AMMO_GREN2; break;
	}

	AssertMsg( false, "LookupLuaAmmo - invalid ammo type!" );

	return "";
}

/////////////////////////////////////////////////////////////////////////////-
// Purpose: Convert ammo to lua ammo
/////////////////////////////////////////////////////////////////////////////-
int LookupAmmoLua( int iAmmoType )
{
	// NOTE: this is kind of lame as in i don't think we even setup the ammo
	// type in our CTakeDamageInfo classes ... except for radio tag rifle.

	if( GetAmmoDef() )
	{
		char *pszName = GetAmmoDef()->GetAmmoOfIndex( iAmmoType )->pName;

		if( pszName && Q_strlen( pszName ) )
		{
			if( !Q_strcmp( pszName, AMMO_SHELLS ) )
				return LUA_AMMO_SHELLS;
			else if( !Q_strcmp( pszName, AMMO_CELLS ) )
				return LUA_AMMO_CELLS;
			else if( !Q_strcmp( pszName, AMMO_NAILS ) )
				return LUA_AMMO_NAILS;
			else if( !Q_strcmp( pszName, AMMO_ROCKETS ) )
				return LUA_AMMO_ROCKETS;
			else if( !Q_strcmp( pszName, AMMO_DETPACK ) )
				return LUA_AMMO_DETPACK;
			else if( !Q_strcmp( pszName, AMMO_MANCANNON ) )
				return LUA_AMMO_MANCANNON;
			// TODO: Maybe figure these in somehow?
			/*
			else if( !Q_strcmp( pszName, AMMO_GREN1 ) )
				return LUA_AMMO_GREN1;
			else if( !Q_strcmp( pszName, AMMO_GREN2 ) )
				return LUA_AMMO_GREN2;
				*/
		}
	}

	return LUA_AMMO_INVALID;
}


//---------------------------------------------------------------------------
void CFFLuaLib::InitConstants(lua_State* L)
{
	ASSERT(L);

	module(L)
	[	
		class_<CFFEntity_AmmoTypes>("Ammo")
			.enum_("AmmoType")
			[
				value("kShells",			LUA_AMMO_SHELLS),
				value("kCells",				LUA_AMMO_CELLS),
				value("kNails",				LUA_AMMO_NAILS),
				value("kRockets",			LUA_AMMO_ROCKETS),
				value("kDetpack",			LUA_AMMO_DETPACK),
				value("kManCannon",			LUA_AMMO_MANCANNON),
				value("kGren1",				LUA_AMMO_GREN1),
				value("kGren2",				LUA_AMMO_GREN2),
				value("kInvalid",			LUA_AMMO_INVALID)
			],

		class_<CFFEntity_Effect_Flags>("EF")
			.enum_("EffectFlagId")
			[
				value("kOnfire",			LUA_EF_ONFIRE),
				value("kConc",				LUA_EF_CONC),
				value("kGas",				LUA_EF_GAS),
				value("kInfect",			LUA_EF_INFECT),
				value("kRadiotag",			LUA_EF_RADIOTAG),
				value("kHeadshot",			LUA_EF_HEADSHOT),
				value("kLegshot",			LUA_EF_LEGSHOT),
				value("kTranq",				LUA_EF_TRANQ),
				value("kACSpinup",			LUA_EF_ACSPINUP),
				value("kSniperrifle",		LUA_EF_SNIPERRIFLE),
				value("kSpeedlua1",			LUA_EF_SPEED_LUA1),
				value("kSpeedlua2",			LUA_EF_SPEED_LUA2),
				value("kSpeedlua3",			LUA_EF_SPEED_LUA3),
				value("kSpeedlua4",			LUA_EF_SPEED_LUA4),
				value("kSpeedlua5",			LUA_EF_SPEED_LUA5),
				value("kSpeedlua6",			LUA_EF_SPEED_LUA6),
				value("kSpeedlua7",			LUA_EF_SPEED_LUA7),
				value("kSpeedlua8",			LUA_EF_SPEED_LUA8),
				value("kSpeedlua9",			LUA_EF_SPEED_LUA9),
				value("kSpeedlua10",		LUA_EF_SPEED_LUA10)
			],

		class_<CFFEntity_ApplyTo_Flags>("AT")
			.enum_("ApplyToFlagId")
			[
				value("kKillPlayers",		AT_KILL_PLAYERS),
				value("kRespawnPlayers",	AT_RESPAWN_PLAYERS),
				value("kDropItems",			AT_DROP_ITEMS),
				value("kForceDropItems",	AT_FORCE_DROP_ITEMS),
				value("kThrowItems",		AT_THROW_ITEMS),
				value("kForceThrowItems",	AT_FORCE_THROW_ITEMS),
				value("kReturnCarriedItems",	AT_RETURN_CARRIED_ITEMS),
				value("kReturnDroppedItems",	AT_RETURN_DROPPED_ITEMS),
				value("kRemoveRagdolls",	AT_REMOVE_RAGDOLLS),
				value("kRemovePacks",		AT_REMOVE_PACKS),
				value("kRemoveProjectiles",	AT_REMOVE_PROJECTILES),
				value("kRemoveBuildables",	AT_REMOVE_BUILDABLES),
				value("kRemoveDecals",		AT_REMOVE_DECALS),
				value("kEndMap",			AT_END_MAP),
				value("kReloadClips",		AT_RELOAD_CLIPS),
				value("kAllowRespawn",		AT_ALLOW_RESPAWN),
				value("kDisallowRespawn",		AT_DISALLOW_RESPAWN),

				value("kChangeClassScout",	AT_CHANGECLASS_SCOUT),
				value("kChangeClassSniper",	AT_CHANGECLASS_SNIPER),
				value("kChangeClassSoldier",	AT_CHANGECLASS_SOLDIER),
				value("kChangeClassDemoman",	AT_CHANGECLASS_DEMOMAN),
				value("kChangeClassMedic",	AT_CHANGECLASS_MEDIC),
				value("kChangeClassHWGuy",	AT_CHANGECLASS_HWGUY),
				value("kChangeClassPyro",	AT_CHANGECLASS_PYRO),
				value("kChangeClassSpy",	AT_CHANGECLASS_SPY),
				value("kChangeClassEngineer",	AT_CHANGECLASS_ENGINEER),
				value("kChangeClassCivilian",	AT_CHANGECLASS_CIVILIAN),
				value("kChangeClassRandom",	AT_CHANGECLASS_RANDOM),

				value("kChangeTeamBlue",	AT_CHANGETEAM_BLUE),
				value("kChangeTeamRed",		AT_CHANGETEAM_RED),
				value("kChangeTeamYellow",	AT_CHANGETEAM_YELLOW),
				value("kChangeTeamGreen",	AT_CHANGETEAM_GREEN),
				value("kChangeTeamSpectator", AT_CHANGETEAM_SPEC),

				value("kStopPrimedGrens",	AT_STOP_PRIMED_GRENS)
			],

		class_<CFFEntity_HudItemTypes>("Hud")
			.enum_("HudItemTypes")
			[
				value("kIcon",				LUA_HUD_ITEM_ICON),
				value("kText",				LUA_HUD_ITEM_TEXT),
				value("kTimer",				LUA_HUD_ITEM_TIMER),
				value("kRemove",			LUA_HUD_ITEM_REMOVE)
			],

		class_<CFFEntity_HudColors>("Color")
			.enum_("HudColors")
			[
				value("kDefault",			LUA_COLOR_DEFAULT),
				value("kBlue",				LUA_COLOR_BLUE),
				value("kRed",				LUA_COLOR_RED),
				value("kYellow",			LUA_COLOR_YELLOW),
				value("kGreen",				LUA_COLOR_GREEN),
				value("kWhite",				LUA_COLOR_WHITE),
				value("kBlack",				LUA_COLOR_BLACK),
				value("kOrange",			LUA_COLOR_ORANGE),
				value("kPink",				LUA_COLOR_PINK),
				value("kPurple",			LUA_COLOR_PURPLE),
				value("kGrey",				LUA_COLOR_GREY),
				value("kInvalid",			LUA_COLOR_INVALID)
			],

		class_<CFFEntity_AllowFlags>("AllowFlags")
			.enum_("AllowFlags")
			[
				value("kOnlyPlayers",	kAllowOnlyPlayers),
				value("kBlue",			kAllowBlueTeam),
				value("kRed",			kAllowRedTeam),
				value("kYellow",		kAllowYellowTeam),
				value("kGreen",			kAllowGreenTeam),
				value("kScout",			kAllowScout),
				value("kSniper",		kAllowSniper),
				value("kSoldier",		kAllowSoldier),
				value("kDemoman",		kAllowDemoman),
				value("kMedic",			kAllowMedic),
				value("kHwguy",			kAllowHwguy),
				value("kPyro",			kAllowPyro),
				value("kSpy",			kAllowSpy),
				value("kEngineer",		kAllowEngineer),
				value("kCivilian",		kAllowCivilian)
			],
		class_<CFFEntity_ClipFlags>("ClipFlags")
			.enum_("ClipFlags")
			[
				value("kClipPlayers",		LUA_CLIP_FLAG_PLAYERS),
				value("kClipTeamBlue",		LUA_CLIP_FLAG_TEAMBLUE),
				value("kClipTeamRed",		LUA_CLIP_FLAG_TEAMRED),
				value("kClipTeamYellow",	LUA_CLIP_FLAG_TEAMYELLOW),
				value("kClipTeamGreen",		LUA_CLIP_FLAG_TEAMGREEN),
				value("kClipGrenades",		LUA_CLIP_FLAG_GRENADES),
				value("kClipTeamEntities",	LUA_CLIP_FLAG_TEAM_ENTITIES)
			],
		class_<CFFEntity_SpecMode>("SpecMode")
			.enum_("SpecMode")
			[
				value("kNone",		(int) OBS_MODE_NONE),
				value("kDeathCam",	(int) OBS_MODE_DEATHCAM),
				value("kFixed",		(int) OBS_MODE_FIXED),				
				value("kInEye",		(int) OBS_MODE_IN_EYE),
				value("kChase",		(int) OBS_MODE_CHASE),
				value("kRoaming",	(int) OBS_MODE_ROAMING)
			],
		class_<CFFRenderFX>("RenderFx")
			.enum_("RenderFx")
			[
				value("kPulseSlow",		(int) kRenderFxPulseSlow),
				value("kPulseFast",		(int) kRenderFxPulseFast),
				value("kPulseSlowWide",	(int) kRenderFxPulseSlowWide),				
				value("kPulseFastWide",	(int) kRenderFxPulseFastWide),
				value("kFadeSlow",		(int) kRenderFxFadeSlow),
				value("kFadeFast",		(int) kRenderFxFadeFast),
				value("kSolidSlow",		(int) kRenderFxSolidSlow),
				value("kSolidFast",		(int) kRenderFxSolidFast),
				value("kStrobeSlow",	(int) kRenderFxStrobeSlow),				
				value("kStrobeFast",	(int) kRenderFxStrobeFast),
				value("kStrobeFaster",	(int) kRenderFxStrobeFaster),
				value("kFlickerSlow",	(int) kRenderFxFlickerSlow),
				value("kFlickerFast",	(int) kRenderFxFlickerFast),
				value("kNoDissipation",	(int) kRenderFxNoDissipation),
				value("kDistort",		(int) kRenderFxDistort),				
				value("kHologram",		(int) kRenderFxHologram),
				value("kExplode",		(int) kRenderFxExplode),
				value("kGlowShell",		(int) kRenderFxGlowShell)
			]
	];
};
