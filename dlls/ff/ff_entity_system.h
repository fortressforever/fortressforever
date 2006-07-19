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

// forward declarations
struct lua_State;

namespace luabind
{
	namespace adl
	{
		class object;
	}
}

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

	static bool LoadLuaFile( lua_State*, const char *);
	bool StartForMap();
	void FFLibOpen();

	lua_State* GetLuaState() const { return L; }

	static void SetVar( lua_State *L, const char *name, const char *value );
	static void SetVar( lua_State *L, const char *name, int value );
	static void SetVar( lua_State *L, const char *name, float value );
	void SetVar( const char *name, const char *value );
	void SetVar( const char *name, int value );
	void SetVar( const char *name, float value );
	static int HandleError( lua_State* );
	void DoString( const char *buffer );

	const char *GetString( const char *name );
	int GetInt( const char *name );
	float GetFloat( const char *name );
	int RunPredicates( CBaseEntity*, CBaseEntity*, const char * = NULL);

	bool GetObject(CBaseEntity* pEntity, luabind::adl::object& outObject);
	bool GetObject(const char* szTableName, luabind::adl::object& outObject);

	bool GetFunction(CBaseEntity* pEntity,
					 const char* szFunctionName,
					 luabind::adl::object& outObject);

	bool GetFunction(luabind::adl::object& tableObject,
					 const char* szFunctionName,
					 luabind::adl::object& outObject);

	// Just checks if object exists
	bool GetObject( CBaseEntity *pEntity );

	// Just checks if a function for an object exists
	bool GetFunction( CBaseEntity *pEntity, const char *szFunctionName );

	// This is temp... needs to be templatable
	bool GetFunctionValue_Bool( CBaseEntity *pEntity, const char *szFunction, CBaseEntity *pArg );
};

bool FFScriptRunPredicates( CBaseEntity *pEntity, const char *pszFunction, bool bExpectedVal );

extern CFFEntitySystem entsys;

//============================================================================
// CFFEntity_ApplyToFlags
// Purpose: this is a fake class to expose AT_ flags to lua easily
//============================================================================
class CFFEntity_ApplyTo_Flags
{
public:
};

#endif // FF_ENTITY_SYSTEM_H
