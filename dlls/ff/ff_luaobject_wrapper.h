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

// This is to stop some linux gayness
//#define _GLIBCXX_EXPORT_TEMPLATE 1
//#undef _GLIBCXX_USE_WCHAR_T

#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "luabind/luabind.hpp"
#include "luabind/object.hpp"
#include "luabind/iterator_policy.hpp"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
//
// Class CFFLuaObjectWrapper
//
//=============================================================================
class CFFLuaObjectWrapper
{
public:
	// Accessors (add any you need...):
	float	GetFloat( void )
	{
		try { return luabind::object_cast< float >( m_hObject ); }
		catch(...)
		{
			return 0.0f;
		}
	}
	int		GetInt( void )
	{ 
		try { return luabind::object_cast< int >( m_hObject ); }
		catch(...)
		{
			return 0;
		}
	}

	bool	GetBool( void )
	{
		try {return luabind::object_cast< bool >( m_hObject ); }
		catch(...)
		{
			return false;
		}
	}

	Vector	GetVector( void )
	{ 
		try { return luabind::object_cast< Vector >( m_hObject ); }
		catch(...)
		{ 
			Vector vec; 
			return vec;
		}
	}

	QAngle	GetQAngle( void )
	{
		try { return luabind::object_cast< QAngle >( m_hObject ); }
		catch(...)
		{
			QAngle angle;
			return angle;
		}
	}

	// Get at the data!
	luabind::adl::object GetObject( void ) const { return m_hObject; }
	luabind::adl::object& GetObject( void ) { return m_hObject; }

protected:
	// The data:
	luabind::adl::object	m_hObject;
};

#endif // FF_LUAOBJECT_WRAPPER
