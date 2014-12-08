
// ff_lualib_buildables.cpp

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"
#include "ff_buildableobjects_shared.h"

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
void CFFLuaLib::InitBuildables(lua_State* L)
{
	ASSERT(L);

	module(L)
	[
		// Buildable base
		class_<CFFBuildableObject, CBaseEntity>("BaseBuildable")
			.def("GetTeamId",			&CFFBuildableObject::GetTeamNumber)
			.def("GetOwner",			&CFFBuildableObject::GetOwnerPlayer)
			.def("GetTeam",				&CFFBuildableObject::GetOwnerTeam),
			//.def("GetTeamId",			&CFFBuildableObject::GetOwnerTeamId),

		// Dispenser
		class_<CFFDispenser, CFFBuildableObject>("Dispenser"),
		
		// Sentrygun
		class_<CFFSentryGun, CFFBuildableObject>("Sentrygun")
			.def("GetLevel",			&CFFSentryGun::GetLevel),
		
		// Detpack
		class_<CFFDetpack, CFFBuildableObject>("Detpack")
	];
};
