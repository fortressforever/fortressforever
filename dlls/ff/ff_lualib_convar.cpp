
#include "cbase.h"
#include "ff_lualib.h"
#include "ff_player.h"
#include "convar.h"
#include "ff_convarman.h"

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "luabind/luabind.hpp"
#include "luabind/object.hpp"
#include "luabind/operator.hpp"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace luabind;

/// tostring implemenation for CTeam
std::ostream& operator<<(std::ostream& stream, const ConCommandBase& conCommand)
{
	return stream << "ConCommandBase" << ":" << conCommand.GetName();
}
std::ostream& operator<<(std::ostream& stream, const ConCommand& conCommand)
{
	return stream << "ConCommand" << ":" << conCommand.GetName();
}
std::ostream& operator<<(std::ostream& stream, const ConVar& conCommand)
{
	return stream << "ConVar" << ":" << conCommand.GetName();
}
bool operator==(const ConCommandBase& conCommand1, const ConCommandBase& conCommand2)
{
	return Q_stricmp(conCommand1.GetName(), conCommand2.GetName()) == 0;
}

namespace FFLib
{
	const ConVar* AddConVar(char const *pName, char const *pDefaultValue, int flags, const char *pHelpString, const luabind::adl::object& callback)
	{
		const ConCommandBase *pExistingConVar = ConCommandBase::FindCommand(pName);
		if(pExistingConVar)
		{
			return !pExistingConVar->IsCommand() ? (ConVar*)pExistingConVar : NULL;
		}

		return _convarman.AddConVar(pName, pDefaultValue, flags, pHelpString, callback);
	}

	const ConCommand* AddConCommand(char const *pName, const luabind::adl::object& callback, int flags, const char *pHelpString)
	{
		const ConCommandBase *pExistingConCommand = ConCommandBase::FindCommand(pName);
		if(pExistingConCommand)
		{
			return pExistingConCommand->IsCommand() ? (ConCommand*)pExistingConCommand : NULL;
		}

		return _convarman.AddConCommand(pName, callback, flags, pHelpString);
	}

	const ConCommandBase* GetConCommandBase(const char *name)
	{
		// remove access to anything that starts with rcon
		if (Q_strnicmp(name, "rcon", Q_strlen("rcon")) == 0)
			return NULL;

		return ConCommandBase::FindCommand(name);
	}

	const ConCommand* GetConCommand(const char *name)
	{
		const ConCommandBase *pConCommandBase = GetConCommandBase(name);
		return pConCommandBase && pConCommandBase->IsCommand() ? (ConCommand*) pConCommandBase : NULL;
	}

	const ConVar* GetConVar(const char *name)
	{
		const ConCommandBase *pConCommandBase = GetConCommandBase(name);
		return pConCommandBase && !pConCommandBase->IsCommand() ? (ConVar*) pConCommandBase : NULL;
	}

	// dummy class for the enum namespace
	class CvarEnumDummy {};
} // namespace FFLib

/** Do the bindings
*/
void CFFLuaLib::InitConVar(lua_State* L)
{
	ASSERT(L);

	module(L)
	[
		// TODO: Protect rcon_password
		def("GetConCommandBase", &FFLib::GetConCommandBase),
		def("GetConCommand", &FFLib::GetConCommand),
		def("GetConVar", &FFLib::GetConVar),
		def("AddConVar", &FFLib::AddConVar),
		def("AddConCommand", &FFLib::AddConCommand),

		class_<ConCommandBase>("ConCommandBase")
			.def(tostring(self))
			.def(const_self == const_self)
			.def("GetName",				&ConCommandBase::GetName)
			.def("GetHelpText",			&ConCommandBase::GetHelpText)
			.def("IsRegistered",		&ConCommandBase::IsRegistered)
			.def("IsFlagSet",			&ConCommandBase::IsBitSet)
			.def("AddFlag",				&ConCommandBase::AddFlags)
			.def("IsCommand",			&ConCommandBase::IsCommand),

		class_<ConCommand, ConCommandBase>("ConCommand")
			.def(tostring(self)),

		class_<ConVar, ConCommandBase>("ConVar")
			.def(tostring(self))
			.def("GetString",			&ConVar::GetString)
			.def("GetFloat",			&ConVar::GetFloat)
			.def("GetInt",				&ConVar::GetInt)
			.def("GetBool",				&ConVar::GetBool)
			.def("SetValue",			(void(ConVar::*)(const char*))&ConVar::SetValue)
			.def("SetValue",			(void(ConVar::*)(int))&ConVar::SetValue)
			.def("SetValue",			(void(ConVar::*)(float))&ConVar::SetValue)
			.def("Revert",				&ConVar::Revert)
			.def("GetDefault",			&ConVar::GetDefault),

		class_<FFLib::CvarEnumDummy>("Cvar")
			.enum_("CvarFlags")
			[
				value("kProtected", FCVAR_PROTECTED),
				value("kArchive", FCVAR_ARCHIVE),
				value("kNotify", FCVAR_NOTIFY),
				value("kUserInfo", FCVAR_USERINFO),
				value("kCheats", FCVAR_CHEAT),
				value("kPrintableOnly", FCVAR_PRINTABLEONLY),
				value("kUnlogged", FCVAR_UNLOGGED),
				value("kReplicated", FCVAR_REPLICATED)
			]

	];
};
