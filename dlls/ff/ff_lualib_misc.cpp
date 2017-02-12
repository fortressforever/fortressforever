
// ff_lualib_misc.cpp

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"

#include "beam_shared.h"

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "luabind/luabind.hpp"
#include "luabind/operator.hpp"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------------------------
using namespace luabind;

/// tostring implemenation for Color
std::ostream& operator<<(std::ostream& stream, const Color& color)
{
	return stream << "(" << color.r() << "," << color.g() << "," << color.b() << "," << color.a() << ")";
}

//---------------------------------------------------------------------------
void CFFLuaLib::InitMisc(lua_State* L)
{
	ASSERT(L);

	module(L)
	[
		// CBeam
		class_<CBeam, CBaseEntity>("Beam")
			.def("SetColor",				&CBeam::SetColor)
			.def("SetStartAndEndEntities",				&CBeam::EntsInit)
			.def("SetSpriteAndWidth",				&CBeam::BeamInit)
			
		class_<Color>("CustomColor")
			.def(tostring(self))
			.def(constructor<>())
			.def(constructor<int, int, int>())
			.def(constructor<int, int, int, int>())
			.property("r", &Color::r)
			.property("g", &Color::g)
			.property("b", &Color::b)
			.property("a", &Color::a)
			.def("SetColor",			&Color::SetColor)
	];
};
