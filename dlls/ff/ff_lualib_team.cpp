
// ff_lualib_team.cpp

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"
#include "ff_team.h"

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

//---------------------------------------------------------------------------
void CFFLuaLib::InitTeam(lua_State* L)
{
	ASSERT(L);

	module(L)
	[
		// CTeam
		class_<CTeam>("BaseTeam")
			.def("AddScore",			&CTeam::AddScore)
			.def("GetNumPlayers",		&CTeam::GetNumPlayers)
			.def("GetPlayer",			&CTeam::GetPlayer)
			.def("GetTeamId",			&CTeam::GetTeamNumber)
			.def("SetName",				&CTeam::SetName),

		// CFFTeam
		class_<CFFTeam, CTeam>("Team")
			.def("SetAllies",			&CFFTeam::SetEasyAllies)
			.def("SetClassLimit",		&CFFTeam::SetClassLimit)
			.def("SetPlayerLimit",		&CFFTeam::SetTeamLimits)
			.enum_("TeamId")
			[
				value("kUnassigned",	TEAM_UNASSIGNED),
				value("kSpectator",		TEAM_SPECTATOR),
				value("kBlue",			TEAM_BLUE),
				value("kRed",			TEAM_RED),
				value("kYellow",		TEAM_YELLOW),
				value("kGreen",			TEAM_GREEN)
			]
	];
};
