
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

	// helper function to get saboteur
	CFFPlayer* GetBuildableSaboteur(CFFBuildableObject *pBuildable)
	{
		return pBuildable->m_hSaboteur;
	}

	// helper function to remove saboteur, defaulted to suppress notifications
	void RemoveSaboteurQuietly(CFFBuildableObject *pBuildable)
	{
		pBuildable->RemoveSaboteur(true);
	}

	void SafeSabotage(CFFBuildableObject *pBuildable, CFFPlayer *pSaboteur)
	{
		if (pBuildable->CanSabotage())
			pBuildable->Sabotage(pSaboteur);
	}

	void SafeMaliciouslySabotage(CFFBuildableObject *pBuildable, CFFPlayer *pSaboteur)
	{
		if (pSaboteur == GetBuildableSaboteur())
			pBuildable->MaliciouslySabotage(pSaboteur);
	}
}

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
			.def("GetTeam",				&CFFBuildableObject::GetOwnerTeam)
			.def("IsBuilt",				&CFFBuildableObject::IsBuilt)
			.def("Detonate",			&CFFBuildableObject::Detonate)
			.def("IsSabotaged",			&CFFBuildableObject::IsSabotaged)
			.def("IsMaliciouslySabotaged", &CFFBuildableObject::IsMaliciouslySabotaged)
			.def("CanSabotage",			&CFFBuildableObject::CanSabotage)
			.def("Sabotage",			&FFLib::SafeSabotage)
			.def("MaliciouslySabotage",	&FFLib::SafeMaliciouslySabotage)
			.def("RemoveSaboteur",		&FFLib::RemoveSaboteurQuietly))
			.def("RemoveQuietly",		&CFFBuildableObject::RemoveQuietly)
			.def("GetSaboteur",			&FFLib::GetBuildableSaboteur),

		// Dispenser
		class_<CFFDispenser, CFFBuildableObject>("Dispenser"),
		
		// Sentrygun
		class_<CFFSentryGun, CFFBuildableObject>("Sentrygun")
			.def("GetLevel",			&CFFSentryGun::GetLevel)
			.def("SetLevel",			&FFLib::SetSGLevel)
			.def("Upgrade",				&CFFSentryGun::Upgrade)
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
	];
};
