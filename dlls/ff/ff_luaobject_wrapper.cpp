// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_luabobject_wrapper.h
// @author Patrick O'Leary (Mulchman) 
// @date 7/22/2006
// @brief A LUA object wrapper
//
// REVISIONS
// ---------
//	7/22/2006, Mulchman: 
//		First created

#include "cbase.h"
#include "ff_luaobject_wrapper.h"

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "luabind/luabind.hpp"
#include "luabind/object.hpp"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

bool GetBool( const luabind::adl::object& hObject )
{
	return luabind::object_cast< bool >( hObject );
}

int GetInt( const luabind::adl::object& hObject )
{
	return luabind::object_cast< int >( hObject );
}

float GetFloat( const luabind::adl::object& hObject )
{
	return luabind::object_cast< float >( hObject );
}

Vector GetVector( const luabind::adl::object& hObject )
{
	return luabind::object_cast< Vector >( hObject );
}

QAngle GetQAngle( const luabind::adl::object& hObject )
{
	return luabind::object_cast< QAngle >( hObject );
}
