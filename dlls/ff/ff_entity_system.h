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
#pragma once

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

	// opens the FF-specific lua library
	void FFLibOpen();

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
	bool RunPredicates_LUA( CBaseEntity *pObject, CBaseEntity *pEntity, const char *szFunctionName );
	bool RunPredicates_LUA( CBaseEntity *pObject, CBaseEntity *pEntity, const char *szFunctionName, luabind::adl::object& hOutput );

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
enum LuaAmmoTypes
{
	LUA_AMMO_SHELLS = 0,
	LUA_AMMO_NAILS,
	LUA_AMMO_CELLS,
	LUA_AMMO_ROCKETS,
	LUA_AMMO_RADIOTAG,
	LUA_AMMO_DETPACK,
	LUA_AMMO_GREN1,
	LUA_AMMO_GREN2
};

class CFFEntity_AmmoTypes
{
public:
};

#endif // FF_ENTITY_SYSTEM_H
