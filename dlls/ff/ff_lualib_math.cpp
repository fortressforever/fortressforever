
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

// Valve, you fucking suck
//std::ostream& operator<<(std::ostream&s, const Vector&v)
//{
//	s << v.x << " " << v.y << " " << v.z;
//	return s;
//}

namespace FFLib
{
	int BitwiseOr(int a, int b)
	{
		return a | b;
	}

	int BitwiseAnd(int a, int b)
	{
		return a & b;
	}

	int BitwiseXor(int a, int b)
	{
		return a ^ b;
	}

	int BitwiseNot(int a)
	{
		return ~a;
	}

	int BitwiseLShift(int a, int shift)
	{
		return a << shift;
	}

	int BitwiseRShift(int a, int shift)
	{
		return a >> shift;
	}
}

//---------------------------------------------------------------------------
void CFFLuaLib::InitMath(lua_State* L)
{
	ASSERT(L);

	module(L)
	[
		def("AngleVectors",		(void(*)(const QAngle&, Vector*))&AngleVectors),
		def("AngleVectors",		(void(*)(const QAngle&, Vector*, Vector*, Vector*))&AngleVectors),
		def("VectorAngles",		(void(*)(const Vector&, QAngle&))&VectorAngles),

		class_<Vector>("Vector")
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
			.def("LengthSqr",			&QAngle::LengthSqr),

		def("BitwiseOr",		&FFLib::BitwiseOr),
		def("BitwiseAnd",		&FFLib::BitwiseAnd),
		def("BitwiseXor",		&FFLib::BitwiseXor),
		def("BitwiseNot",		&FFLib::BitwiseNot),
		def("BitwiseLShift",	&FFLib::BitwiseLShift),
		def("BitwiseRShift",	&FFLib::BitwiseRShift)
	];
};
