
// ff_luacontext.cpp

//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_luacontext.h"
#include "ff_scriptman.h"
#include "ff_entity_system.h"

#include "ff_buildableobjects_shared.h"
#include "ff_team.h"
#include "ff_grenade_base.h"
#include "ff_player.h"
#include "ff_item_flag.h"

#include "beam_shared.h"
#include "player.h"
#include "team.h"
#include "takedamageinfo.h"

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#undef MINMAX_H
#undef min
#undef max

#include "luabind/luabind.hpp"
#include "luabind/object.hpp"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------------------------
// defines
#define SETOBJECT(val)		new luabind::adl::object(_scriptman.GetLuaState(), val)
#define SETOBJECTREF(val)	new luabind::adl::object(_scriptman.GetLuaState(), boost::ref(val))

#define RETURN_OBJECTCAST(type, defaultVal, idx)	\
			try					\
			{					\
				if(idx < m_returnVals.Count())		\
					return luabind::object_cast<type>((*m_returnVals[idx]));	\
				else			\
					return defaultVal;	\
			}					\
			catch(...)			\
			{					\
				DevWarning("[SCRIPT] Wrong value type returned from function %s\n", m_szFunction);	\
				return defaultVal;	\
			}					\

//---------------------------------------------------------------------------
// Purpose: Constructor to use a bunch of args
//---------------------------------------------------------------------------
CFFLuaSC::CFFLuaSC( int iArgs, ... )
{
	// TODO: Make the constructor and setparams use this same code

	va_list ap;		
	va_start( ap, iArgs );

	try
	{
		for( int i = 0; i < iArgs; i++ )
		{
			Push( va_arg( ap, CBaseEntity* ) );
		}
	}
	catch( ... )
	{
	}	

	va_end( ap );
}

//---------------------------------------------------------------------------
// Purpose: Constructor to use a bunch of args
//---------------------------------------------------------------------------
void CFFLuaSC::SetParams( int iArgs, ... )
{	
	// TODO: Make the constructor and setparams use this same code

	va_list ap;		
	va_start( ap, iArgs );

	try
	{
		for( int i = 0; i < iArgs; i++ )
		{
			Push( va_arg( ap, CBaseEntity* ) );
		}
	}
	catch( ... )
	{
	}	

	va_end( ap );	
}

//---------------------------------------------------------------------------
CFFLuaSC::~CFFLuaSC()
{
	m_params.PurgeAndDeleteElements();
	m_returnVals.PurgeAndDeleteElements();
}

//---------------------------------------------------------------------------
void CFFLuaSC::Push(float value) { m_params.AddToTail(SETOBJECT(value)); }
void CFFLuaSC::Push(int value) { m_params.AddToTail(SETOBJECT(value)); }
void CFFLuaSC::Push(bool value) { m_params.AddToTail(SETOBJECT(value)); }
void CFFLuaSC::Push(const char *value) { m_params.AddToTail(SETOBJECT(value)); }
void CFFLuaSC::Push(luabind::adl::object& luabindObject) { m_params.AddToTail(SETOBJECT(luabindObject)); }
void CFFLuaSC::Push(CBaseEntity* pEntity) { m_params.AddToTail(SETOBJECT(pEntity)); }
void CFFLuaSC::Push(CFFBuildableObject* pEntity) { m_params.AddToTail(SETOBJECT(pEntity)); }
void CFFLuaSC::Push(CFFDispenser* pEntity) { m_params.AddToTail(SETOBJECT(pEntity)); }
void CFFLuaSC::Push(CFFSentryGun* pEntity) { m_params.AddToTail(SETOBJECT(pEntity)); }
void CFFLuaSC::Push(CFFDetpack* pEntity) { m_params.AddToTail(SETOBJECT(pEntity)); }
void CFFLuaSC::Push(CTeam* pEntity) { m_params.AddToTail(SETOBJECT(pEntity)); }
void CFFLuaSC::Push(CFFTeam* pEntity) { m_params.AddToTail(SETOBJECT(pEntity)); }
void CFFLuaSC::Push(CFFGrenadeBase* pEntity) { m_params.AddToTail(SETOBJECT(pEntity)); }
void CFFLuaSC::Push(CBasePlayer* pEntity) { m_params.AddToTail(SETOBJECT(pEntity)); }
void CFFLuaSC::Push(CFFPlayer* pEntity) { m_params.AddToTail(SETOBJECT(pEntity)); }
void CFFLuaSC::Push(CFFInfoScript* pEntity) { m_params.AddToTail(SETOBJECT(pEntity)); }
void CFFLuaSC::Push(CBeam* pEntity) { m_params.AddToTail(SETOBJECT(pEntity)); }
void CFFLuaSC::Push(Vector vector) { m_params.AddToTail(SETOBJECT(vector)); }
void CFFLuaSC::Push(QAngle angle) { m_params.AddToTail(SETOBJECT(angle)); }
void CFFLuaSC::Push(const CTakeDamageInfo* pInfo) { m_params.AddToTail(SETOBJECT(pInfo)); }

//---------------------------------------------------------------------------
void CFFLuaSC::PushRef(luabind::adl::object& luabindObject) { m_params.AddToTail(SETOBJECTREF(luabindObject)); }
void CFFLuaSC::PushRef(Vector &vector) { m_params.AddToTail(SETOBJECTREF(vector)); }
void CFFLuaSC::PushRef(QAngle &angle) { m_params.AddToTail(SETOBJECTREF(angle)); }
void CFFLuaSC::PushRef(CTakeDamageInfo& info) { m_params.AddToTail(SETOBJECTREF(info)); }

//---------------------------------------------------------------------------
bool CFFLuaSC::CallFunction(CBaseEntity* pEntity, const char* szFunctionName, const char *szTargetEntName)
{
	VPROF_BUDGET( "CFFLuaSC::CallFunction", VPROF_BUDGETGROUP_FF_LUA );

	m_returnVals.PurgeAndDeleteElements();

	lua_State* L = _scriptman.GetLuaState();

	// If there is no active script then allow the ents to always go
	if(!_scriptman.ScriptExists() || !L)
		return false;

	// set lua's reference to the calling entity
	luabind::object globals = luabind::globals(L);
	try
	{
		if(pEntity)
			globals["entity"] = luabind::object(L, pEntity);
		else
			globals["entity"] = luabind::adl::object();
	}
	catch(...)
	{
		// CBaseEntity was not registered with luabind
		// if this happens, something very bad has happened
		ASSERT(false);
		return false;
	}

	// look up the function
	if(pEntity)
	{
		if(!szFunctionName || !strlen(szFunctionName))
		{
			Msg("Can't call entsys.runpredicates with an entity and no addname\n");
			return false;
		}

		if (!strlen(STRING(pEntity->GetEntityName())))
		{
			// REMOVED: Really annoying to see this
			//Warning( "[entsys] ent did not have an entity name!\n" );
			return false;
		}

		// check if the function exists
		luabind::adl::object func;
		if(!_scriptman.GetFunction(pEntity, szFunctionName, func))
			return false;

		// push the function onto stack ( entname:addname )
		lua_getglobal( L, STRING(pEntity->GetEntityName()) );
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 1);
			return false;
		}
		lua_pushstring(L, szFunctionName);
		lua_gettable(L, -2);
		lua_insert(L, -2);

		// store the name of the entity and function for debugging purposes
		Q_snprintf(m_szFunction,
				   sizeof(m_szFunction),
				   "%s:%s()",
				   STRING(pEntity->GetEntityName()),
				   szFunctionName);		
	}
	else if(szTargetEntName)
	{
		luabind::adl::object func;
		luabind::adl::object tableObject;
		if(_scriptman.GetObject(szTargetEntName, tableObject) &&
			_scriptman.GetFunction(tableObject, szFunctionName, func))
		{
			// push the function onto stack ( entname:addname )
			lua_getglobal( L, szTargetEntName );
			if (lua_isnil(L, -1))
			{
				lua_pop(L, 1);
				return false;
			}
			lua_pushstring(L, szFunctionName);
			lua_gettable(L, -2);
			lua_insert(L, -2);

			// store the name of the entity and function for debugging purposes
			Q_snprintf(m_szFunction,
				sizeof(m_szFunction),
				"%s:%s()",
				szTargetEntName,
				szFunctionName);	
		}
		else
			return false;
	}
	else
	{
		// get the function
		lua_getglobal(L, szFunctionName);
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 1);
			return false;
		}

		// store the name of the function for debugging purposes
		Q_strncpy(m_szFunction, szFunctionName, sizeof(m_szFunction));
	}

	// push all the parameters
	int nParams = GetNumParams();
	for(int iParam = 0 ; iParam < nParams ; ++iParam)
		(*m_params[iParam]).push(L);

	// call out to the script
	if(lua_pcall(L, pEntity||szTargetEntName ? nParams + 1 : nParams, 1, 0) != 0)
	{
		const char* szErrorMsg = lua_tostring(L, -1);
		Msg("[SCRIPT] Error calling %s (%s) ent: %s\n",
				   szFunctionName, 
				   szErrorMsg,
				   pEntity ? STRING(pEntity->GetEntityName()) : "NULL");

		return false;
	}

	// get the return values
	int nRetVals = 1;
	for(int iRet = 0 ; iRet < nRetVals ; ++iRet)
	{
		luabind::adl::object* pRetObj = new luabind::adl::object(luabind::from_stack(L, -1 - iRet));
		m_returnVals.AddToHead(pRetObj);
	}

	lua_pop(L, nRetVals);

	// cleanup
	luabind::adl::object dummy;
	globals[ "entity" ] = dummy;

	return true;
}

//---------------------------------------------------------------------------
bool CFFLuaSC::CallFunction(const char* szFunctionName)
{
	VPROF_BUDGET( "CFFLuaSC::CallFunction", VPROF_BUDGETGROUP_FF_LUA );

	return CallFunction(NULL, szFunctionName);
}

//---------------------------------------------------------------------------
void CFFLuaSC::ClearParams()
{
	m_params.PurgeAndDeleteElements();
}

//---------------------------------------------------------------------------
bool CFFLuaSC::GetBool()
{
	RETURN_OBJECTCAST(bool, false, 0);
}

//---------------------------------------------------------------------------
float CFFLuaSC::GetFloat()
{
	RETURN_OBJECTCAST(float, 0.0f, 0);
}

//---------------------------------------------------------------------------
int CFFLuaSC::GetInt()
{
	RETURN_OBJECTCAST(int, 0, 0);
}

//---------------------------------------------------------------------------
luabind::adl::object* CFFLuaSC::GetObject()
{
	return m_returnVals[0];
}

//---------------------------------------------------------------------------
QAngle CFFLuaSC::GetQAngle()
{
	QAngle dummy;
	RETURN_OBJECTCAST(QAngle, dummy, 0);
}

//---------------------------------------------------------------------------
Vector CFFLuaSC::GetVector()
{
	Vector vec;
	RETURN_OBJECTCAST(Vector, vec, 0);
}

//---------------------------------------------------------------------------
void CFFLuaSC::QuickCallFunction(const char* szFunctionName)
{
	VPROF_BUDGET( "CFFLuaSC::QuickCallFunction", VPROF_BUDGETGROUP_FF_LUA );

	CFFLuaSC sc;
	sc.CallFunction(NULL, szFunctionName);
}

//---------------------------------------------------------------------------
void CFFLuaSC::QuickCallFunction(CBaseEntity* pEntity, const char* szFunctionName)
{
	VPROF_BUDGET( "CFFLuaSC::QuickCallFunction", VPROF_BUDGETGROUP_FF_LUA );

	CFFLuaSC sc;
	sc.CallFunction(pEntity, szFunctionName);
}
