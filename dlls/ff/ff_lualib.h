
// ff_lualib.h

//---------------------------------------------------------------------------
#ifndef FF_LUALIB_H
#define FF_LUALIB_H

//---------------------------------------------------------------------------
// includes

//---------------------------------------------------------------------------
// foward declarations
struct lua_State;

//---------------------------------------------------------------------------
class CFFLuaLib
{
public:
	// initializes the lua library for FF
	static void Init(lua_State* L);

private:
	// initialization helpers
	static void InitBase(lua_State* L);
	static void InitBuildables(lua_State* L);
	static void InitConstants(lua_State* L);
	static void InitGlobals(lua_State* L);
	static void InitPlayer(lua_State* L);
	static void InitMath(lua_State* L);
	static void InitMisc(lua_State* L);
	static void InitOmnibot(lua_State* L);
	static void InitTeam(lua_State* L);
	static void InitUtil(lua_State* L);
	static void InitWeapons(lua_State* L);
	static void InitData(lua_State* L);
};

//---------------------------------------------------------------------------
#endif
