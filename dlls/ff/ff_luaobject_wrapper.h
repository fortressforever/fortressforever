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

#ifndef FF_LUAOBJECT_WRAPPER_H
#define FF_LUAOBJECT_WRAPPER_H

#define _GLIBCXX_EXPORT_TEMPLATE 1

#ifdef _WIN32
#pragma once
#endif

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "luabind/luabind.hpp"
#include "luabind/object.hpp"

//=============================================================================
//
// Class CFFLuaObjectWrapper
//
//=============================================================================
class CFFLuaObjectWrapper
{
public:
	// Accessors (add any you need...):
	float	GetFloat( void )	{ return luabind::object_cast< float >( m_hObject ); }
	int		GetInt( void )		{ return luabind::object_cast< int >( m_hObject ); }
	bool	GetBool( void )		{ return luabind::object_cast< bool >( m_hObject ); }
	Vector	GetVector( void )	{ return luabind::object_cast< Vector >( m_hObject ); }
	QAngle	GetQAngle( void )	{ return luabind::object_cast< QAngle >( m_hObject ); }

	// Get at the data!
	luabind::adl::object GetObject( void ) const { return m_hObject; }
	luabind::adl::object& GetObject( void ) { return m_hObject; }

protected:
	// The data:
	luabind::adl::object	m_hObject;
};

#endif // FF_LUAOBJECT_WRAPPER
