
#include "cbase.h"
#include "ff_convarman.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void OnConVarChange(ConVar *pConVar, const char *pszOldValue)
{
	((LuaConVar*) pConVar)->DispatchCallback(pszOldValue);
}

CFFConVarManager _convarman;

CFFConVarManager::CFFConVarManager()
{
}

CFFConVarManager::~CFFConVarManager()
{
}

LuaConVar* CFFConVarManager::AddConVar(char const *pName, char const *pDefaultValue, int flags, const char *pHelpString, const luabind::adl::object& callback)
{
	if(ConCommandBase::FindCommand(pName))
		return NULL;

	LuaConVar* pConVar = new LuaConVar(pName, pDefaultValue, flags, pHelpString, callback);

	m_concommands.AddToTail(pConVar);
	return pConVar;
}

LuaConCommand* CFFConVarManager::AddConCommand(char const *pName, const luabind::adl::object& callback, int flags, const char *pHelpString)
{
	if(ConCommandBase::FindCommand(pName))
		return NULL;

	LuaConCommand* pConCommand = new LuaConCommand(pName, callback, flags, pHelpString);

	m_concommands.AddToTail(pConCommand);
	return pConCommand;
}

void CFFConVarManager::Init()
{
	Shutdown();
}

void CFFConVarManager::Shutdown()
{
	m_concommands.RemoveAll();
}

void CFFConVarManager::Update()
{
}
