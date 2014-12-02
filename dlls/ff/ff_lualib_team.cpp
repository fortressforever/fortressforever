
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
#include "luabind/operator.hpp"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------------------------
using namespace luabind;

/// tostring implemenation for CTeam
std::ostream& operator<<(std::ostream& stream, const CTeam& team)
{
	return stream << "team" << ":" << const_cast<CTeam&>(team).GetName() << ":" << const_cast<CTeam&>(team).GetTeamNumber();
}

//---------------------------------------------------------------------------
void CFFLuaLib::InitTeam(lua_State* L)
{
	ASSERT(L);

	module(L)
	[
		// CTeam
		class_<CTeam>("BaseTeam")
			.def(tostring(self))
			.def("AddScore",			&CTeam::AddScore)
			.def("GetScore",			&CTeam::GetScore)
			.def("SetScore",			&CTeam::SetScore)
			.def("GetScoreTime",		&CTeam::GetScoreTime)
			.def("AddFortPoints",		&CTeam::AddFortPoints)
			.def("SetFortPoints",		&CTeam::SetFortPoints)
			.def("GetFortPoints",		&CTeam::GetFortPoints)
			.def("AddDeaths",			&CTeam::AddDeaths)
			.def("GetDeaths",			&CTeam::GetDeaths)
			.def("SetDeaths",			&CTeam::SetDeaths)
			.def("GetNumPlayers",		&CTeam::GetNumPlayers)
			.def("GetPlayer",			&CTeam::GetPlayer)
			.def("GetTeamId",			&CTeam::GetTeamNumber)
			.def("GetName",				&CTeam::GetName)
			.def("SetName",				&CTeam::SetName),

		// CFFTeam
		class_<CFFTeam, CTeam>("Team")
			.def("SetAllies",			&CFFTeam::SetEasyAllies)
			.def("ClearAllies",			&CFFTeam::ClearAllies)
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
