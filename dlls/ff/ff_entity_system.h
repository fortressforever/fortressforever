/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_entity_system.h
/// @author Gavin Bramhill (Mirvin_Monkey)
/// @date 21 April 2005
/// @brief Handles the entity system
///
/// REVISIONS
/// ---------
/// Apr 21, 2005 Mirv: Begun

#ifndef FF_ENTITY_SYSTEM_H
#define FF_ENTITY_SYSTEM_H

#ifdef _WIN32
#pragma once
#endif

//#include <vector>

//----------------------------------------------------------------------------
// forward declarations
struct lua_State;

namespace luabind
{
	namespace adl
	{
		class object;
	}
}

class CFFLuaSC;

//----------------------------------------------------------------------------
// extern declarations
extern ConVar mp_respawndelay;

//============================================================================
// CFFEntitySystemHelper
//============================================================================
class CFFEntitySystemHelper : public CBaseEntity
{
public:
	DECLARE_CLASS( CFFEntitySystemHelper, CBaseEntity );
	DECLARE_DATADESC();

	void Spawn( void );
	void OnThink( void );
	void Precache( void );
};

extern CFFEntitySystemHelper *helper;

//============================================================================
// CFFEntitySystem
//============================================================================
class CFFEntitySystem
{
private:
	lua_State* L;
	bool m_ScriptExists;

public:
	CFFEntitySystem();
	~CFFEntitySystem();

	// runs the lua file into the script environment
	static bool LoadLuaFile( lua_State*, const char *);

	// loads the script for the current map
	bool StartForMap();

	// returns the lua interpreter
	lua_State* GetLuaState() const { return L; }
	bool ScriptExists( void ) const { return m_ScriptExists; }

	// handles lua error
	static int HandleError( lua_State *L );

	// sets a global variable in the script environment
	static void SetVar( lua_State *L, const char *name, const char *value );
	static void SetVar( lua_State *L, const char *name, int value );
	static void SetVar( lua_State *L, const char *name, float value );
	void SetVar( const char *name, const char *value );
	void SetVar( const char *name, int value );
	void SetVar( const char *name, float value );

	// gets a global variable from the script environment
	const char *GetString( const char *name );
	int GetInt( const char *name );
	float GetFloat( const char *name );

	// runs the script
	void DoString( const char *buffer );

	int RunPredicates( CBaseEntity *pObject, CBaseEntity *pEntity, const char *szFunctionName = NULL);

	// A better run predicates
	bool RunPredicates_LUA( CBaseEntity *pObject, CFFLuaSC *pContext, const char *szFunctionName );

	bool GetObject(CBaseEntity* pEntity, luabind::adl::object& outObject);
	bool GetObject(const char* szTableName, luabind::adl::object& outObject);

	bool GetFunction(CBaseEntity* pEntity,
					 const char* szFunctionName,
					 luabind::adl::object& outObject);

	bool GetFunction(luabind::adl::object& tableObject,
					 const char* szFunctionName,
					 luabind::adl::object& outObject);
};

//----------------------------------------------------------------------------
// utility functions
bool FFScriptRunPredicates( CBaseEntity *pEntity, const char *pszFunction, bool bExpectedVal );

//----------------------------------------------------------------------------
// global externs
extern CFFEntitySystem entsys;

//============================================================================
// CFFEntity_ApplyToFlags
// Purpose: this is a fake class to expose AT_ flags to lua easily
//============================================================================
class CFFEntity_ApplyTo_Flags
{
public:
};

//============================================================================
// CFFEntity_EffectFlags
// Purpose: this is a fake class to expose "EF_" ["effect"] flags to lua easily
//============================================================================
class CFFEntity_Effect_Flags
{
public:
};

//============================================================================
// CFFEntity_AmmoTypes
// Purpose: this is a fake class to expose "AMMO" to lua
//============================================================================
class CFFEntity_AmmoTypes
{
public:
};

//-----------------------------------------------------------------------------
// Purpose: Ammo types in lua
//-----------------------------------------------------------------------------
enum LuaAmmoTypes
{
	LUA_AMMO_SHELLS = 0,
	LUA_AMMO_NAILS,
	LUA_AMMO_CELLS,
	LUA_AMMO_ROCKETS,
	LUA_AMMO_RADIOTAG,
	LUA_AMMO_DETPACK,
	LUA_AMMO_GREN1,
	LUA_AMMO_GREN2,

	LUA_AMMO_INVALID
};

//-----------------------------------------------------------------------------
// Purpose: Convert lua ammo type (int) to game ammo type (string)
//-----------------------------------------------------------------------------
const char *LookupLuaAmmo( int iLuaAmmoType );

//-----------------------------------------------------------------------------
// Purpose: Convert ammo to lua ammo
//-----------------------------------------------------------------------------
int LookupAmmoLua( int iAmmoType );

//============================================================================
// CFFEntity_AmmoTypes
// Purpose: this is a fake class to expose hud items to LUA
//============================================================================
class CFFEntity_HudItemTypes
{
public:
};

enum LuaHudItemTypes
{
	LUA_HUD_ITEM_ICON = 0,
	LUA_HUD_ITEM_TEXT,
	LUA_HUD_ITEM_TIMER,
	LUA_HUD_ITEM_REMOVE,

	LUA_HUD_ITEM_MAX
};

#endif // FF_ENTITY_SYSTEM_H
