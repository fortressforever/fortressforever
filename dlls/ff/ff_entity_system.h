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

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef CHECKSUM_CRC_H
	#include "checksum_crc.h"
#endif

/////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////
// CFFEntitySystemHelper
/////////////////////////////////////////////////////////////////////////////
class CFFEntitySystemHelper : public CBaseEntity
{
	DECLARE_CLASS( CFFEntitySystemHelper, CBaseEntity );
	DECLARE_DATADESC();

public:
	// 'structors
	CFFEntitySystemHelper();
	virtual ~CFFEntitySystemHelper();

public:
	// CBaseEntity
	void Spawn();
	void OnThink();
	void Precache();

public:
	// spawns an instance of this type
	static CFFEntitySystemHelper* Create();

	// there can only be one!!
	static CFFEntitySystemHelper* GetInstance();

private:
	static CFFEntitySystemHelper* s_pInstance;
};

/////////////////////////////////////////////////////////////////////////////
// CFFEntitySystem
/////////////////////////////////////////////////////////////////////////////
class CFFEntitySystem
{
public:
	// 'structors
	CFFEntitySystem();
	~CFFEntitySystem();

public:
	// inserts the lua file into the script environment
	static bool LoadFile(lua_State*, const char* filePath);

public:
	// loads the scripts for the level
	void LevelInit(const char* szMapName);

	// cleans up the scripts for the most recent level
	void LevelShutdown();

private:
	// initializes the script VM
	void Init();
	void Shutdown();

	// surround code that loads scripts to capture crc checksum
	// of the scripts that are loaded
	void BeginScriptLoad();
	void EndScriptLoad();

private:
	// called when a script is loaded. internally computes the
	// crc checksum of the file contents
	void OnScriptLoad(const char* szFileName, const char* szFileContents);

public:
	// sets a global variable in the script environment
	static void SetVar( lua_State *L, const char *name, const char *value );
	static void SetVar( lua_State *L, const char *name, int value );
	static void SetVar( lua_State *L, const char *name, float value );
	void SetVar(const char* name, const char* value);
	void SetVar(const char* name, int value);
	void SetVar(const char* name, float value);

	// gets a global variable from the script environment
	const char* GetString(const char* name);
	int GetInt(const char* name);
	float GetFloat(const char* name);

	bool GetObject(CBaseEntity* pEntity, luabind::adl::object& outObject);
	bool GetObject(const char* szTableName, luabind::adl::object& outObject);

	bool GetFunction(CBaseEntity* pEntity,
					 const char* szFunctionName,
					 luabind::adl::object& outObject);

	bool GetFunction(luabind::adl::object& tableObject,
					 const char* szFunctionName,
					 luabind::adl::object& outObject);

public:
	int RunPredicates( CBaseEntity *pObject, CBaseEntity *pEntity, const char *szFunctionName = NULL);
	bool RunPredicates_LUA( CBaseEntity *pObject, CFFLuaSC *pContext, const char *szFunctionName );

public:
	// returns the lua interpreter
	lua_State* GetLuaState() const { return L; }
	bool ScriptExists( void ) const { return m_ScriptExists; }

	// returns the crc checksum of the currently active script
	CRC32_t GetScriptCRC() const { return m_scriptCRC; }

private:
	// private data
	lua_State*	L;				// lua VM
	bool		m_ScriptExists;
	bool		m_isLoading;
	CRC32_t		m_scriptCRC;
};

/////////////////////////////////////////////////////////////////////////////
// utility functions
bool FFScriptRunPredicates( CBaseEntity *pEntity, const char *pszFunction, bool bExpectedVal );

/////////////////////////////////////////////////////////////////////////////
// global externs
extern CFFEntitySystem entsys;

/////////////////////////////////////////////////////////////////////////////
#endif // FF_ENTITY_SYSTEM_H
