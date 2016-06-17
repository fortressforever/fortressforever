#ifndef FF_CONVARMAN_H
#define FF_CONVARMAN_H

#include "convar.h"

#ifndef UTLVECTOR_H
	#include "utlvector.h"
#endif
#ifndef LUABIND_OBJECT_050419_HPP
	#include "luabind/object.hpp"
#endif

void OnConVarChange(ConVar *pConVar, const char *pszOldValue);

class LuaConCommand : public ConCommand
{
public:
	LuaConCommand(char const *pName, const luabind::adl::object& callback, int flags = 0, const char *pHelpString = NULL) : ConCommand(pName, NULL, pHelpString, flags)
	{
		m_luaCallbackFn = callback;
	}

	virtual void Dispatch()
	{
		if (m_luaCallbackFn.is_valid() && luabind::type(m_luaCallbackFn) == LUA_TFUNCTION)
		{
			m_luaCallbackFn();
		}
	}

private:
	luabind::adl::object m_luaCallbackFn;
};

class LuaConVar : public ConVar
{
public:
	LuaConVar(char const *pName, char const *pDefaultValue, int flags = 0, const char *pHelpString = 0, const luabind::adl::object& callback = luabind::adl::object()) : ConVar(pName, pDefaultValue, flags, pHelpString, OnConVarChange) 
	{
		m_luaCallbackFn = callback;
	}
	// TODO: min/max value constructors

	void DispatchCallback(const char *oldValue)
	{
		if (m_luaCallbackFn.is_valid() && luabind::type(m_luaCallbackFn) == LUA_TFUNCTION)
		{
			m_luaCallbackFn(oldValue, GetString());
		}
	}

private:
	luabind::adl::object m_luaCallbackFn;
};

class CFFConVarManager
{
public:
	CFFConVarManager();
	~CFFConVarManager();

public:
	void Init();
	void Shutdown();
	void Update();

public:
	LuaConVar* AddConVar(char const *pName, char const *pDefaultValue, int flags = 0, const char *pHelpString = 0, const luabind::adl::object& callback = luabind::adl::object());
	LuaConCommand* AddConCommand(char const *pName, const luabind::adl::object& callback, int flags = 0, const char *pHelpString = NULL);

private:
	CUtlVector<ConCommandBase*>	m_concommands;
};

extern CFFConVarManager _convarman;

#endif