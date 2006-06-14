//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Game-specific impact effect hooks
//
//=============================================================================//
#include "cbase.h"
#include "fx_impact.h"
#include "engine/IEngineSound.h"

// Cvar that controls the frequency of the flecks effects
extern ConVar cl_effectfrequency;

//-----------------------------------------------------------------------------
// Purpose: Handle weapon impacts
//-----------------------------------------------------------------------------
void ImpactCallback( const CEffectData &data )
{
	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;

	C_BaseEntity *pEntity = ParseImpactData( data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox );

	if ( !pEntity )
		return;

	// If we hit, perform our custom effects and play the sound
	if ( Impact( vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr ) )
	{
		// Check for custom effects based on the Decal index
		PerformCustomEffects( vecOrigin, tr, vecShotDir, iMaterial, 1.0 );

		//Play a ricochet sound some of the time
		if( random->RandomInt(1,10) <= 3 && (iDamageType == DMG_BULLET) )
		{
			CLocalPlayerFilter filter;
			C_BaseEntity::EmitSound( filter, SOUND_FROM_WORLD, "Bounce.Shrapnel", &vecOrigin );
		}
	}

	PlayImpactSound( pEntity, tr, vecOrigin, nSurfaceProp );
}

DECLARE_CLIENT_EFFECT( "Impact", ImpactCallback );


//-----------------------------------------------------------------------------
// Purpose: Handle weapon impacts
//-----------------------------------------------------------------------------
void NailImpactCallback( const CEffectData &data )
{
	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;

	C_BaseEntity *pEntity = ParseImpactData( data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox );

	if ( !pEntity )
		return;

	// If we hit, perform our custom effects and play the sound
	if ( Impact( vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr ) )
	{
		// Check for custom effects based on the Decal index
		// Mirv: Only do the custom fleck effect some of the time
		if (random->RandomFloat(0.0f, 1.0f) <= cl_effectfrequency.GetFloat())
			PerformCustomEffects( vecOrigin, tr, vecShotDir, iMaterial, 1.0 );

		//Play a ricochet sound some of the time
		if( random->RandomInt(1,10) <= 3 && (iDamageType == DMG_BULLET) )
		{
			CLocalPlayerFilter filter;
			C_BaseEntity::EmitSound( filter, SOUND_FROM_WORLD, "Bounce.Shrapnel", &vecOrigin );
		}
	}

	PlayImpactSound( pEntity, tr, vecOrigin, nSurfaceProp );
}

DECLARE_CLIENT_EFFECT( "NailImpact", NailImpactCallback );

//-----------------------------------------------------------------------------
// Purpose: Handle weapon impacts
//-----------------------------------------------------------------------------
void RailImpactCallback( const CEffectData &data )
{
	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;

	C_BaseEntity *pEntity = ParseImpactData( data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox );

	if ( !pEntity )
		return;

	// If we hit, perform our custom effects and play the sound
	if ( Impact( vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr ) )
	{
		// Check for custom effects based on the Decal index
		PerformCustomEffects( vecOrigin, tr, vecShotDir, iMaterial, 1.0 );

		//Play a ricochet sound some of the time
		if( random->RandomInt(1,10) <= 3 && (iDamageType == DMG_BULLET) )
		{
			CLocalPlayerFilter filter;
			C_BaseEntity::EmitSound( filter, SOUND_FROM_WORLD, "Bounce.Shrapnel", &vecOrigin );
		}
	}

	PlayImpactSound( pEntity, tr, vecOrigin, nSurfaceProp );
}

DECLARE_CLIENT_EFFECT( "RailImpact", RailImpactCallback );

//-----------------------------------------------------------------------------
// Purpose: Handle weapon impacts
//-----------------------------------------------------------------------------
void DartImpactCallback( const CEffectData &data )
{
	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;

	C_BaseEntity *pEntity = ParseImpactData( data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox );

	if ( !pEntity )
		return;

	// If we hit, perform our custom effects and play the sound
	if ( Impact( vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr ) )
	{
		// Check for custom effects based on the Decal index
		PerformCustomEffects( vecOrigin, tr, vecShotDir, iMaterial, 1.0 );

		//Play a ricochet sound some of the time
		if( random->RandomInt(1,10) <= 3 && (iDamageType == DMG_BULLET) )
		{
			CLocalPlayerFilter filter;
			C_BaseEntity::EmitSound( filter, SOUND_FROM_WORLD, "Bounce.Shrapnel", &vecOrigin );
		}
	}

	PlayImpactSound( pEntity, tr, vecOrigin, nSurfaceProp );
}

DECLARE_CLIENT_EFFECT( "DartImpact", DartImpactCallback );