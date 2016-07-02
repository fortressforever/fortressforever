
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
#include "luabind/operator.hpp"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------------------------
using namespace luabind;

/// tostring implemenation for Vector
std::ostream& operator<<(std::ostream& stream, const Vector& vec)
{
	return stream << "Vector(" << vec.x << "," << vec.y << "," << vec.z << ")";
}
/// tostring implemenation for QAngle
std::ostream& operator<<(std::ostream& stream, const QAngle& angle)
{
	return stream << "QAngle(" << angle.x << "," << angle.y << "," << angle.z << ")";
}

namespace FFLib
{
	void AngleVectors(::lua_State* L, const QAngle &angle)
	{
		Vector forward, right, up;
		::AngleVectors(angle, &forward, &right, &up);
		luabind::adl::object(L, forward).push(L);
		luabind::adl::object(L, right).push(L);
		luabind::adl::object(L, up).push(L);
	}

	void VectorAngles(::lua_State* L, const Vector &forward)
	{
		QAngle angles;
		::VectorAngles(forward, angles);
		luabind::adl::object(L, angles).push(L);
	}
}

//---------------------------------------------------------------------------
void CFFLuaLib::InitMath(lua_State* L)
{
	ASSERT(L);

	module(L)
	[
		def("AngleVectors",		(void(*)(lua_State*, const QAngle&))&FFLib::AngleVectors, raw(_1)),
		def("VectorAngles",		(void(*)(lua_State*, const Vector&))&FFLib::VectorAngles, raw(_1)),

		class_<Vector>("Vector")
			.def(tostring(const_self))
			.def(constructor<>())
			.def(constructor<float, float, float>())
			.def(self * float())
			.def(self / float())
			.def(self + const_self)
			.def(self - const_self)
			.def(self * const_self)
			.def(self / const_self)
			.def(const_self == const_self)
			.def_readwrite("x",			&Vector::x)
			.def_readwrite("y",			&Vector::y)
			.def_readwrite("z",			&Vector::z)
			.def("IsValid",				&Vector::IsValid)
			.def("IsZero",				&Vector::IsZero)
			.def("DistTo",				&Vector::DistTo)
			.def("DistToSq",			&Vector::DistToSqr)
			.def("Dot",					&Vector::Dot)
			.def("Cross",				&Vector::Cross)
			.def("Length",				&Vector::Length)
			.def("LengthSqr",			&Vector::LengthSqr)
			.def("Normalize",			&Vector::NormalizeInPlace)
			.def("ClampToAABB",			&Vector::ClampToAABB)
			.def("Random",				&Vector::Random)
			.def("Min",					&Vector::Min)
			.def("Max",					&Vector::Max)
			.def("Negate",				&Vector::Negate),

		class_<QAngle>("QAngle")
			.def(tostring(const_self))
			.def(constructor<>())
			.def(constructor<float, float, float>())
			.def(self * float())
			.def(self / float())
			.def(self + const_self)
			.def(self - const_self)
			.def(const_self == const_self)
			.def_readwrite("x",			&QAngle::x)
			.def_readwrite("y",			&QAngle::y)
			.def_readwrite("z",			&QAngle::z)
			.def("IsValid",				&QAngle::IsValid)
			.def("Random",				&QAngle::Random)
			.def("Length",				&QAngle::Length)
			.def("LengthSqr",			&QAngle::LengthSqr)
	];
};
