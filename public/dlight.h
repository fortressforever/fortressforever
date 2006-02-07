//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#if !defined ( DLIGHTH )
#define DLIGHTH
#ifdef _WIN32
#pragma once
#endif

#include "vector.h"

//-----------------------------------------------------------------------------
// Dynamic light structure
//-----------------------------------------------------------------------------

enum
{
	DLIGHT_NO_WORLD_ILLUMINATION = 0x1,
	DLIGHT_NO_MODEL_ILLUMINATION = 0x2,

	// NOTE: These two features are used to dynamically tweak the alpha on displacements
	// which is a special effect for selecting which texture to use. If 
	// we ever change how alpha is stored for displacements, we'll have to kill this feature
	DLIGHT_ADD_DISPLACEMENT_ALPHA = 0x4,
	DLIGHT_SUBTRACT_DISPLACEMENT_ALPHA = 0x8,
	DLIGHT_DISPLACEMENT_MASK = (DLIGHT_ADD_DISPLACEMENT_ALPHA | DLIGHT_SUBTRACT_DISPLACEMENT_ALPHA),
};

struct dlight_t
{
	int		flags;
	Vector	origin;
	float	radius;
	ColorRGBExp32	color;		// Light color with exponent
	float	die;				// stop lighting after this time
	float	decay;				// drop this each second
	float	minlight;			// don't add when contributing less
	int		key;
	int		style;				// lightstyle

	// For spotlights. Use m_OuterAngle == 0 for point lights
	Vector	m_Direction;		// center of the light cone
	float	m_InnerAngle;
	float	m_OuterAngle;
};

#endif