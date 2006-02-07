//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef PROP_COMBINE_BALL_H
#define PROP_COMBINE_BALL_H
#ifdef _WIN32
#pragma once
#endif

// Creates a combine ball
CBaseEntity *CreateCombineBall( const Vector &origin, const Vector &velocity, float radius, float mass, float lifetime, CBaseEntity *pOwner );

// Query function to find out if a physics object is a combine ball (used for collision checks)
bool UTIL_IsCombineBall( CBaseEntity *pEntity );
bool UTIL_IsAR2CombineBall( CBaseEntity *pEntity );

#endif // PROP_COMBINE_BALL_H
