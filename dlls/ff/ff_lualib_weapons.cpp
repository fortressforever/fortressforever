
// ff_lualib_weapons.cpp

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"
#include "ff_grenade_base.h"

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
void CFFLuaLib::InitWeapons(lua_State* L)
{
	ASSERT(L);

	module(L)
	[
		// CFFGrenadeBase
		class_<CFFGrenadeBase, CBaseEntity>("Grenade")
			.def("Type",				&CFFGrenadeBase::GetGrenId)
			.enum_("GrenId")
			[
				value("kNormal",		CLASS_GREN),
				value("kNail",			CLASS_GREN_NAIL),
				value("kMirv",			CLASS_GREN_MIRV),
				value("kMirvlet",		CLASS_GREN_MIRVLET),
				value("kConc",			CLASS_GREN_CONC),
				value("kNapalm",		CLASS_GREN_NAPALM),
				value("kGas",			CLASS_GREN_GAS),
				value("kEmp",			CLASS_GREN_EMP),
				value("kSlowfield",		CLASS_GREN_SLOWFIELD),
				value("kLaser",			CLASS_GREN_LASER)
			],

		// CTakeDamageInfo
		class_<CTakeDamageInfo>("Damage")
			.def("GetAttacker",			&CTakeDamageInfo::GetAttacker)
			.def("GetInflictor",		&CTakeDamageInfo::GetInflictor)
			.def("GetDamage",			&CTakeDamageInfo::GetDamage)
			.def("GetDamageForce",		&CTakeDamageInfo::GetDamageForce)
			.def("GetDamagePosition",	&CTakeDamageInfo::GetDamagePosition)
			.def("GetDamageType",		(int(CTakeDamageInfo::*)(void))&CTakeDamageInfo::GetDamageType)
			.def("GetAmmoType",			&CTakeDamageInfo::GetAmmoTypeLua)
			.def("SetDamage",			&CTakeDamageInfo::SetDamage)
			.def("SetDamageForce",		&CTakeDamageInfo::SetDamageForce)
			.def("ScaleDamage",			&CTakeDamageInfo::ScaleDamage)
			.enum_("DamageTypes")
			[
				value("kGeneric",		DMG_GENERIC),
				value("kCrush",			DMG_CRUSH),
				value("kBullet",		DMG_BULLET),
				value("kSlash",			DMG_SLASH),
				value("kBurn",			DMG_BURN),
				value("kVehicle",		DMG_VEHICLE),
				value("kFall",			DMG_FALL),
				value("kBlast",			DMG_BLAST),
				value("kClub",			DMG_CLUB),
				value("kShock",			DMG_SHOCK),
				value("kSonic",			DMG_SONIC),
				value("kEnergyBeam",	DMG_ENERGYBEAM),
				value("kPreventPhysForce",	DMG_PREVENT_PHYSICS_FORCE),
				value("kNeverGib",		DMG_NEVERGIB),
				value("kAlwaysGib",		DMG_ALWAYSGIB),
				value("kDrown",			DMG_DROWN),
				value("kTimeBased",		DMG_TIMEBASED),
				value("kParalyze",		DMG_PARALYZE),
				value("kNerveGas",		DMG_NERVEGAS),
				value("kPoison",		DMG_POISON),
				value("kRadiation",		DMG_RADIATION),
				value("kDrownRecover",	DMG_DROWNRECOVER),
				value("kAcid",			DMG_ACID),
				value("kSlowBurn",		DMG_SLOWBURN),
				value("kRemoveNoRagdoll",	DMG_REMOVENORAGDOLL),
				value("kPhysgun",		DMG_PHYSGUN),
				value("kPlasma",		DMG_PLASMA),
				value("kAirboat",		DMG_AIRBOAT),
				value("kDissolve",		DMG_DISSOLVE),
				value("kBlastSurface",	DMG_BLAST_SURFACE),
				value("kDirect",		DMG_DIRECT),
				value("kBuckshot",		DMG_BUCKSHOT),
				value("kGibCorpse",		DMG_GIB_CORPSE),
				value("kShownHud",		DMG_SHOWNHUD),
				value("kNoPhysForce",	DMG_NO_PHYSICS_FORCE)
			]
	];
};
