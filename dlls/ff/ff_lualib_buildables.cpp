
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

namespace FFLib
{
	// helper functions to avoid manadatory bEmitSound parameter
	// Luabind doesn't handle default parameter values well
	void SetSGLevel(CFFSentryGun *pSentryGun, int iLevel)
	{
		pSentryGun->SetLevel(iLevel, false);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFFBuildableObject_BuildableTypes
// Purpose: this is a fake class to expose "FF_BUILD_" constants to lua
/////////////////////////////////////////////////////////////////////////////
class CFFBuildableObject_BuildableTypes
{
public:
};

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
			.def("GetLevel",			&CFFSentryGun::GetLevel)
			.def("SetLevel",			&FFLib::SetSGLevel)
			.def("Upgrade",				&CFFSentryGun::Upgrade)
			.def("CanUpgrade",			&CFFSentryGun::CanUpgrade)
			.def("CanBeUpgradedBy",		&CFFSentryGun::CanBeUpgradedBy)
			.def("IsUpgradeProgressComplete", &CFFSentryGun::IsUpgradeProgressComplete)
			.def("GetUpgradeProgress",	&CFFSentryGun::GetUpgradeProgress)
			.def("SetUpgradeProgress",	&CFFSentryGun::SetUpgradeProgress)
			.def("GetMaxUpgradeProgress", &CFFSentryGun::GetMaxUpgradeProgress)
			.def("DeltaUpgradeProgress", &CFFSentryGun::DeltaUpgradeProgress)
			.def("Repair",				&CFFSentryGun::Repair)
			.def("AddAmmo",				&CFFSentryGun::AddAmmo)
			.def("RocketPosition",		&CFFSentryGun::RocketPosition)
			.def("MuzzlePosition",		&CFFSentryGun::MuzzlePosition)
			.def("GetRockets",			&CFFSentryGun::GetRockets)
			.def("GetShells",			&CFFSentryGun::GetShells)
			.def("GetHealth",			&CFFSentryGun::GetHealth)
			.def("SetRockets",			&CFFSentryGun::SetRockets)
			.def("SetShells",			&CFFSentryGun::SetShells)
			.def("SetHealth",			&CFFSentryGun::SetHealth)
			.def("GetMaxRockets",		&CFFSentryGun::GetMaxRockets)
			.def("GetMaxShells",		&CFFSentryGun::GetMaxShells)
			.def("GetMaxHealth",		&CFFSentryGun::GetMaxHealth)
			.def("SetFocusPoint",		&CFFSentryGun::SetFocusPoint)
			.def("GetEnemy",			&CFFSentryGun::GetEnemy)
			.def("SetEnemy",			&CFFSentryGun::SetEnemy)
			.def("GetVecAiming",		&CFFSentryGun::GetVecAiming)
			.def("GetVecGoal",			&CFFSentryGun::GetVecGoal)
			.def("Shoot",				(void(CFFSentryGun::*)())&CFFSentryGun::Shoot)
			.def("ShootRocket",			(void(CFFSentryGun::*)())&CFFSentryGun::ShootRocket),
		
		// Detpack
		class_<CFFDetpack, CFFBuildableObject>("Detpack")
			.def("GetFuseTime",			&CFFDetpack::GetFuseTime)
			.def("GetDetonateTime",		&CFFDetpack::GetDetonateTime)
			.def("LastFiveSeconds",		&CFFDetpack::LastFiveSeconds),

		class_<CFFBuildableObject_BuildableTypes>("BuildableTypes")
			.enum_("BuildableTypes")
			[
				value("kNone",				FF_BUILD_NONE),
				value("kDispenser",			FF_BUILD_DISPENSER),
				value("kSentryGun",			FF_BUILD_SENTRYGUN),
				value("kDetpack",			FF_BUILD_DETPACK),
				value("kJumpPad",			FF_BUILD_MANCANNON),
				// For consistency with other names, even though jump pad should be preferred everywhere
				value("kManCannon",			FF_BUILD_MANCANNON)
			]
	];
};
