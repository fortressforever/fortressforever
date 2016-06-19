
// ff_lualib_team.cpp

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"
#include "ff_team.h"
#include "ff_scriptman.h"

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

namespace FFLib
{
	// helper function to turn the bitmask into a lua table of teams
	luabind::adl::object GetAllies(CFFTeam *pTeam)
	{
		luabind::adl::object luatblAllies = luabind::newtable(_scriptman.GetLuaState());

		int iTableKey = 1;
		int alliesMask = pTeam->GetAllies();
		for (int teamId = TEAM_UNASSIGNED; teamId < TEAM_COUNT; teamId++)
		{
			if (!(alliesMask & (1<<teamId)))
				continue;

			CFFTeam *pAlliedTeam = dynamic_cast<CFFTeam*>(g_Teams[teamId]);
			luatblAllies[iTableKey++] = luabind::adl::object(_scriptman.GetLuaState(), pAlliedTeam);
		}

		return luatblAllies;
	}
};

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
			.def("GetAllies",			&FFLib::GetAllies)
			.def("SetClassLimit",		&CFFTeam::SetClassLimit)
			.def("GetClassLimit",		&CFFTeam::GetClassLimit)
			.def("SetPlayerLimit",		&CFFTeam::SetTeamLimits)
			.def("GetPlayerLimit",		&CFFTeam::GetTeamLimits)
			.def("IsFFA",				&CFFTeam::IsFFA)
			.def("SetFFA",				&CFFTeam::SetFFA)
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
