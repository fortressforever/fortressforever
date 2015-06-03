
// ff_lualib_util.cpp

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"
#include "ff_player.h"
#include "ff_projectile_base.h"
#include "ff_item_flag.h"
#include "ff_triggerclip.h"
#include "ff_utils.h"

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "luabind/luabind.hpp"
#include "luabind/object.hpp"
#include "luabind/iterator_policy.hpp"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------------------------
using namespace luabind;

//============================================================================
// CFFEntity_CollectionFilter
// Purpose: this is a fake class to expose collection filter enums to lua
//============================================================================
class CFFEntity_CollectionFilter
{
public:
};

enum CollectionFilter
{
	CF_NONE = 0,

	CF_PLAYERS,
	CF_HUMAN_PLAYERS,
	CF_BOT_PLAYERS,
	CF_PLAYER_SCOUT,
	CF_PLAYER_SNIPER,
	CF_PLAYER_SOLDIER,
	CF_PLAYER_DEMOMAN,
	CF_PLAYER_MEDIC,
	CF_PLAYER_HWGUY,
	CF_PLAYER_PYRO,
	CF_PLAYER_SPY,
	CF_PLAYER_ENGY,
	CF_PLAYER_CIVILIAN,

	CF_TEAMS,
	CF_TEAM_SPECTATOR,
	CF_TEAM_BLUE,
	CF_TEAM_RED,
	CF_TEAM_YELLOW,
	CF_TEAM_GREEN,

	CF_PROJECTILES,
	CF_GRENADES,
	CF_INFOSCRIPTS,

	CF_INFOSCRIPT_CARRIED,
	CF_INFOSCRIPT_DROPPED,
	CF_INFOSCRIPT_RETURNED,
	CF_INFOSCRIPT_ACTIVE,
	CF_INFOSCRIPT_INACTIVE,
	CF_INFOSCRIPT_REMOVED,

	CF_BUILDABLES,
	CF_BUILDABLE_DISPENSER,
	CF_BUILDABLE_SENTRYGUN,
	CF_BUILDABLE_DETPACK,
	CF_BUILDABLE_JUMPPAD,

	CF_TRACE_BLOCK_WALLS,

	CF_MAX_FLAG
};

//---------------------------------------------------------------------------
void CFFLuaLib::InitUtil(lua_State* L)
{
	ASSERT(L);

	module(L)
	[
		// CFFEntity_CollectionFilter
		class_<CFFEntity_CollectionFilter>("CF")
			.enum_("FilterId")
			[
				value("kNone",				CF_NONE),

				value("kPlayers",			CF_PLAYERS),
				value("kHumanPlayers",		CF_HUMAN_PLAYERS),
				value("kBotPlayers",		CF_BOT_PLAYERS),
				value("kPlayerScout",		CF_PLAYER_SCOUT),
				value("kPlayerSniper",		CF_PLAYER_SNIPER),
				value("kPlayerSoldier",		CF_PLAYER_SOLDIER),
				value("kPlayerDemoman",		CF_PLAYER_DEMOMAN),
				value("kPlayerMedic",		CF_PLAYER_DEMOMAN),
				value("kPlayerHWGuy",		CF_PLAYER_HWGUY),
				value("kPlayerPyro",		CF_PLAYER_PYRO),
				value("kPlayerSpy",			CF_PLAYER_SPY),
				value("kPlayerEngineer",	CF_PLAYER_ENGY),
				value("kPlayerCivilian",	CF_PLAYER_CIVILIAN),

				value("kTeams",				CF_TEAMS),
				value("kTeamSpec",			CF_TEAM_SPECTATOR),
				value("kTeamBlue",			CF_TEAM_BLUE),
				value("kTeamRed",			CF_TEAM_RED),
				value("kTeamYellow",		CF_TEAM_YELLOW),
				value("kTeamGreen",			CF_TEAM_GREEN),

				value("kProjectiles",		CF_PROJECTILES),
				value("kGrenades",			CF_GRENADES),
				value("kInfoScipts",		CF_INFOSCRIPTS), // typo; kept for backwards compatibility
				value("kInfoScripts",		CF_INFOSCRIPTS),

				value("kInfoScript_Carried",	CF_INFOSCRIPT_CARRIED),
				value("kInfoScript_Dropped",	CF_INFOSCRIPT_DROPPED),
				value("kInfoScript_Returned",	CF_INFOSCRIPT_RETURNED),
				value("kInfoScript_Active",		CF_INFOSCRIPT_ACTIVE),
				value("kInfoScript_Inactive",	CF_INFOSCRIPT_INACTIVE),
				value("kInfoScript_Removed",	CF_INFOSCRIPT_REMOVED),

				value("kTraceBlockWalls",	CF_TRACE_BLOCK_WALLS),
				
				value("kBuildables",		CF_BUILDABLES),
				value("kDispenser",			CF_BUILDABLE_DISPENSER),
				value("kSentrygun",			CF_BUILDABLE_SENTRYGUN),
				value("kDetpack",			CF_BUILDABLE_DETPACK),
				value("kJumpPad",			CF_BUILDABLE_JUMPPAD)
			]
	];
};
