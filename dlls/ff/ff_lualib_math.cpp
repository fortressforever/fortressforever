
// ff_lualib_math.cpp

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "luabind/luabind.hpp"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------------------------
using namespace luabind;

//---------------------------------------------------------------------------
void CFFLuaLib::InitMath(lua_State* L)
{
	ASSERT(L);

	module(L)
	[
		class_<Vector>("Vector")
			.def(constructor<>())
			.def(constructor<float, float, float>())
			.def_readwrite("x",			&Vector::x)
			.def_readwrite("y",			&Vector::y)
			.def_readwrite("z",			&Vector::z)
			.def("IsValid",				&Vector::IsValid)
			.def("IsZero",				&Vector::IsZero)
			.def("DistTo",				&Vector::DistTo)
			.def("DistToSq",			&Vector::DistToSqr)
			.def("Dot",					&Vector::Dot)
			.def("Length",				&Vector::Length)
			.def("LengthSqr",			&Vector::LengthSqr)
			.def("Normalize",			&Vector::NormalizeInPlace)
			.def("ClampToAABB",			&Vector::ClampToAABB)
			.def("Negate",				&Vector::Negate),

		class_<QAngle>("QAngle")
			.def(constructor<>())
			.def(constructor<float, float, float>())
			.def_readwrite("x",			&QAngle::x)
			.def_readwrite("y",			&QAngle::y)
			.def_readwrite("z",			&QAngle::z)
			.def("IsValid",				&QAngle::IsValid)
			.def("Length",				&QAngle::Length)
			.def("LengthSqr",			&QAngle::LengthSqr)
	];
};
