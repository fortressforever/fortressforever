
// ff_lualib_omnibot.cpp

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"

#include "omnibot_interface.h"

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
class Omnibot_GoalTypes
{
public:
};

//---------------------------------------------------------------------------
void CFFLuaLib::InitOmnibot(lua_State* L)
{
	ASSERT(L);
	module(L)
	[
		class_<Omnibot_GoalTypes>("Bot")
			.enum_("GoalType")
			[
				value("kNone",				Omnibot::kNone),
				value("kBackPack_Ammo",		Omnibot::kBackPack_Ammo),
				value("kBackPack_Armor",	Omnibot::kBackPack_Armor),
				value("kBackPack_Health",	Omnibot::kBackPack_Health),
				value("kBackPack_Grenades",	Omnibot::kBackPack_Grenades),
				value("kFlag",				Omnibot::kFlag),
				value("kFlagCap",			Omnibot::kFlagCap)
			]
	];
};
