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

#include <vector>

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

typedef std::vector< CBaseEntity * > CollectionContainer;

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

//============================================================================
// CFFEntity_Collection
// Purpose: basically a list/vector of entities that lua can have access to
//============================================================================
class CFFEntity_Collection
{
public:
	CFFEntity_Collection( void );
	CFFEntity_Collection( const luabind::adl::object& table );

	void AddItem( CBaseEntity *pItem = NULL );
	void AddItem( const luabind::adl::object& table );
	
	void RemoveItem( CBaseEntity *pItem = NULL );
	void RemoveItem( const luabind::adl::object& table );
	
	void RemoveAllItems( void );
	
	bool IsEmpty( void ) const		{ return m_vCollection.empty(); }
	
	int Count( void ) const			{ return ( int )m_vCollection.size(); }

	bool HasItem( CBaseEntity *pItem = NULL );
	bool HasItem( const luabind::adl::object& table );

	CBaseEntity *GetItem( CBaseEntity *pItem = NULL );
	CBaseEntity *GetItem( const luabind::adl::object& table );

	void GetInSphere( const Vector& vecOrigin, float flRadius, const luabind::adl::object& hFilterTable );
	void GetTouching( CBaseEntity *pTouchee, const luabind::adl::object& hFilterTable );

	CBaseEntity *Element( int iElement );

	CollectionContainer	m_vCollection;

protected:
	CBaseEntity *InternalFindPtr( CBaseEntity *pItem = NULL );
	CollectionContainer::iterator InternalFindItr( CBaseEntity *pItem = NULL );
};

//============================================================================
// CFFEntity_CollectionFilter
// Purpose: this is a fake class to expose collection filter enums to lua
//============================================================================
class CFFEntity_CollectionFilter
{
public:
};

enum CollectionFilter
{
	CF_PLAYERS = 0,
	CF_PLAYER_SCOUT,
	CF_PLAYER_SNIPER,
	CF_PLAYER_SOLDIER,
	CF_PLAYER_DEMOMAN,
	CF_PLAYER_MEDIC,
	CF_PLAYER_HWGUY,
	CF_PLAYER_PYRO,
	CF_PLAYER_SPY,
	CF_PLAYER_ENGY,
	CF_PLAYER_CIVILIAN,

	CF_TEAMS,
	CF_TEAM_SPECTATOR,
	CF_TEAM_BLUE,
	CF_TEAM_RED,
	CF_TEAM_YELLOW,
	CF_TEAM_GREEN,

	CF_PROJECTILES,
	CF_GRENADES,

	CF_BUILDABLES,
	CF_BUILDABLE_DISPENSER,
	CF_BUILDABLE_SENTRYGUN,
	CF_BUILDABLE_DETPACK,

	CF_MAX_FLAG
};

#endif // FF_ENTITY_SYSTEM_H
