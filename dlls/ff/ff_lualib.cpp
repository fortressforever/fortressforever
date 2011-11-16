
// ff_lualib.cpp

//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------------------------
void CFFLuaLib::Init(lua_State* L)
{
	InitBase(L);
	InitMath(L);
	InitConstants(L);
	InitPlayer(L);
	InitTeam(L);
	InitBuildables(L);
	InitWeapons(L);
	InitMisc(L);
	InitGlobals(L);
	InitOmnibot(L);
	InitUtil(L);
	InitData(L);
}
