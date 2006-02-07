//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef COMPRESSED_VECTOR_H
#define COMPRESSED_VECTOR_H

#ifdef _WIN32
#pragma once
#endif

#include <math.h>
#include <float.h>

// For vec_t, put this somewhere else?
#include "basetypes.h"

// For rand(). We really need a library!
#include <stdlib.h>

#include "tier0/dbg.h"
#include "vector.h"

#ifdef __cplusplus


// HACKHACK: Declare these directly from mathlib.h for now
extern "C" 
{
extern float (*pfSqrt)(float x);
};

#define FastSqrt(x)			(*pfSqrt)(x)

//=========================================================
// fit a 3D vector into 32 bits
//=========================================================

class Vector32
{
public:
	// Construction/destruction:
	Vector32(void); 
	Vector32(vec_t X, vec_t Y, vec_t Z);

	// assignment
	Vector32& operator=(const Vector &vOther);
	operator Vector ();

private:
	unsigned short x:10;
	unsigned short y:10;
	unsigned short z:10;
	unsigned short exp:2;
};

inline Vector32& Vector32::operator=(const Vector &vOther)	
{
	CHECK_VALID(vOther);

	static float expScale[4] = { 4.0f, 16.0f, 32.f, 64.f };

	float fmax = max( fabs( vOther.x ), fabs( vOther.y ) );
	fmax = max( fmax, fabs( vOther.z ) );

	for (exp = 0; exp < 3; exp++)
	{
		if (fmax < expScale[exp])
			break;
	}
	Assert( fmax < expScale[exp] );

	float fexp = 512.0f / expScale[exp];

	x = clamp( (int)(vOther.x * fexp) + 512, 0, 1023 );
	y = clamp( (int)(vOther.y * fexp) + 512, 0, 1023 );
	z = clamp( (int)(vOther.z * fexp) + 512, 0, 1023 );
	return *this; 
}


inline Vector32::operator Vector ()
{
	static Vector tmp;

	static float expScale[4] = { 4.0f, 16.0f, 32.f, 64.f };

	float fexp = expScale[exp] / 512.0f;

	tmp.x = (((int)x) - 512) * fexp;
	tmp.y = (((int)y) - 512) * fexp;
	tmp.z = (((int)z) - 512) * fexp; 
	return tmp; 
}


//=========================================================
// Fit a unit vector into 32 bits
//=========================================================

class Normal32
{
public:
	// Construction/destruction:
	Normal32(void); 
	Normal32(vec_t X, vec_t Y, vec_t Z);

	// assignment
	Normal32& operator=(const Vector &vOther);
	operator Vector ();

private:
	unsigned short x:15;
	unsigned short y:15;
	unsigned short zneg:1;
};


inline Normal32& Normal32::operator=(const Vector &vOther)	
{
	CHECK_VALID(vOther);

	x = clamp( (int)(vOther.x * 16384) + 16384, 0, 32767 );
	y = clamp( (int)(vOther.y * 16384) + 16384, 0, 32767 );
	zneg = (vOther.z < 0);
	//x = vOther.x; 
	//y = vOther.y; 
	//z = vOther.z; 
	return *this; 
}


inline Normal32::operator Vector ()
{
	static Vector tmp;

	tmp.x = ((int)x - 16384) * (1 / 16384.0);
	tmp.y = ((int)y - 16384) * (1 / 16384.0);
	tmp.z = sqrt( 1 - tmp.x * tmp.x - tmp.y * tmp.y );
	if (zneg)
		tmp.z = -tmp.z;
	return tmp; 
}


//=========================================================
// 48 bit Quaternion
//=========================================================

class Quaternion48
{
public:
	// Construction/destruction:
	Quaternion48(void); 
	Quaternion48(vec_t X, vec_t Y, vec_t Z);

	// assignment
	// Quaternion& operator=(const Quaternion48 &vOther);
	Quaternion48& operator=(const Quaternion &vOther);
	operator Quaternion ();
private:
	unsigned short x:16;
	unsigned short y:16;
	unsigned short z:15;
	unsigned short wneg:1;
};


inline Quaternion48::operator Quaternion ()	
{
	static Quaternion tmp;

	tmp.x = ((int)x - 32768) * (1 / 32768.0);
	tmp.y = ((int)y - 32768) * (1 / 32768.0);
	tmp.z = ((int)z - 16384) * (1 / 16384.0);
	tmp.w = sqrt( 1 - tmp.x * tmp.x - tmp.y * tmp.y - tmp.z * tmp.z );
	if (wneg)
		tmp.w = -tmp.w;
	return tmp; 
}

inline Quaternion48& Quaternion48::operator=(const Quaternion &vOther)	
{
	CHECK_VALID(vOther);

	x = clamp( (int)(vOther.x * 32768) + 32768, 0, 65535 );
	y = clamp( (int)(vOther.y * 32768) + 32768, 0, 65535 );
	z = clamp( (int)(vOther.z * 16384) + 16384, 0, 32767 );
	wneg = (vOther.w < 0);
	//x = vOther.x; 
	//y = vOther.y; 
	//z = vOther.z; 
	return *this; 
}


//=========================================================
// 16 bit float
//=========================================================


const int float32bias = 127;
const int float16bias = 15;

const float maxfloat16bits = (float)(1<<16) * (1 + ((float)((1<<10)-1)) / ((float)(1<<10)));

class float16
{
public:
	//float16() {}
	//float16( float f ) { m_storage.rawWord = ConvertFloatTo16bits(f); }

	void Init() { m_storage.rawWord = 0; }
	float16& operator=(const float16 &other) { m_storage.rawWord = other.m_storage.rawWord; return *this; }
	float16& operator=(const float &other) { m_storage.rawWord = ConvertFloatTo16bits(other); return *this; }
	operator unsigned short () { return m_storage.rawWord; }
	operator float () { return Convert16bitFloatTo32bits( m_storage.rawWord ); }
private:

	union float32bits
	{
		float rawFloat;
		struct 
		{
			unsigned int mantissa : 23;
			unsigned int biased_exponent : 8;
			unsigned int sign : 1;
		} bits;
	};

	union float16bits
	{
		unsigned short rawWord;
		struct
		{
			unsigned short mantissa : 10;
			unsigned short biased_exponent : 5;
			unsigned short sign : 1;
		} bits;
	};
	unsigned short ConvertFloatTo16bits( float input )
	{
		if ( input > maxfloat16bits )
			input = maxfloat16bits;
		else if ( input < -maxfloat16bits )
			input = -maxfloat16bits;

		float16bits output;
		float32bits inFloat;

		inFloat.rawFloat = input;

		if (inFloat.bits.biased_exponent > float32bias - float16bias)
		{
			output.bits.mantissa = inFloat.bits.mantissa >> (23-10);
			output.bits.biased_exponent = inFloat.bits.biased_exponent - float32bias + float16bias;
		}
		else
		{
			// too small, slam to 0
			output.bits.mantissa = 0;
			output.bits.biased_exponent = 0;
		}

		output.bits.sign = inFloat.bits.sign;
		
		return output.rawWord;
	}

	float Convert16bitFloatTo32bits( unsigned short input )
	{
		float16bits inFloat;
		float32bits output;
		inFloat.rawWord = input;
		output.bits.mantissa = inFloat.bits.mantissa << (23-10);
		output.bits.biased_exponent = (inFloat.bits.biased_exponent - float16bias + float32bias) * (inFloat.bits.biased_exponent != 0);
		output.bits.sign = inFloat.bits.sign;
		
		return output.rawFloat;
	}


	float16bits m_storage;
};


//=========================================================
// Fit a 3D vector in 48 bits
//=========================================================

class Vector48
{
public:
	// Construction/destruction:
	Vector48(void); 
	Vector48(vec_t X, vec_t Y, vec_t Z);

	// assignment
	Vector48& operator=(const Vector &vOther);
	operator Vector ();

	float16 x;
	float16 y;
	float16 z;
};

inline Vector48& Vector48::operator=(const Vector &vOther)	
{
	CHECK_VALID(vOther);

	x = vOther.x;
	y = vOther.y;
	z = vOther.z;
	return *this; 
}


inline Vector48::operator Vector ()
{
	static Vector tmp;

	tmp.x = x;
	tmp.y = y;
	tmp.z = z; 

	return tmp;
}


#endif // _cplusplus

#endif

