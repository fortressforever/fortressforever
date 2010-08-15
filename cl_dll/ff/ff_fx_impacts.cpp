//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Game-specific impact effect hooks
//
//=============================================================================//
#include "cbase.h"
#include "fx_impact.h"
#include "engine/IEngineSound.h"

// Cvar that controls the frequency of the dust effects
ConVar cl_effectsdetail("cl_effectsdetail", "1", FCVAR_ARCHIVE, "Effects detail: 0 - none, 1 - cutdown, 2 - full");

//-----------------------------------------------------------------------------
// Purpose: Some checks before we really do handle the custom effect
//			This allows us to switch off all effects quite easily
//-----------------------------------------------------------------------------
void FFPerformCustomEffects(const Vector &vecOrigin, trace_t &tr, const Vector &shotDir, int iMaterial, int iScale, int nFlags = 0)
{
	if (cl_effectsdetail.GetInt() == 0)
		return;

	PerformCustomEffects(vecOrigin, tr, shotDir, iMaterial, iScale, nFlags);
}

//-----------------------------------------------------------------------------
// Purpose: Handle weapon impacts
//-----------------------------------------------------------------------------
void ImpactCallback(const CEffectData &data)
{
	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;

	C_BaseEntity *pEntity = ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);

	if (!pEntity)
		return;

	// If we hit, perform our custom effects and play the sound
	if (Impact(vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr))
	{
		// Only do effect if not optional
		if (!(data.m_fFlags & CEFFECT_EFFECTNOTNEEDED && cl_effectsdetail.GetInt() == 1))
		{
			// Check for custom effects based on the Decal index
			FFPerformCustomEffects(vecOrigin, tr, vecShotDir, iMaterial, 1.0);

			//Play a ricochet sound some of the time
			if (random->RandomInt(1, 10) <= 3 && (iDamageType == DMG_BULLET))
			{
				CLocalPlayerFilter filter;
				C_BaseEntity::EmitSound(filter, SOUND_FROM_WORLD, "Bounce.Shrapnel", &vecOrigin);
			}
		}
	}

	// Play impact sound only if not optional
	if (!(data.m_fFlags & CEFFECT_SOUNDNOTNEEDED))
		PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
}

DECLARE_CLIENT_EFFECT("Impact", ImpactCallback);

//-----------------------------------------------------------------------------
// Purpose: Handle weapon impacts
//-----------------------------------------------------------------------------
void NailImpactCallback(const CEffectData &data)
{
	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;

	C_BaseEntity *pEntity = ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);

	if (!pEntity)
		return;

	// If we hit, perform our custom effects and play the sound
	if (Impact(vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr))
	{
		// Only do effect if not optional
		if (!(data.m_fFlags & CEFFECT_EFFECTNOTNEEDED && cl_effectsdetail.GetInt() == 1))
		{
			// Check for custom effects based on the Decal index
			FFPerformCustomEffects(vecOrigin, tr, vecShotDir, iMaterial, 1.0);

			//Play a ricochet sound some of the time
			if (random->RandomInt(1, 10) <= 3 && (iDamageType == DMG_BULLET))
			{
				CLocalPlayerFilter filter;
				C_BaseEntity::EmitSound(filter, SOUND_FROM_WORLD, "Bounce.Shrapnel", &vecOrigin);
			}
		}
	}

	// Play impact sound only if not optional
	if (!(data.m_fFlags & CEFFECT_SOUNDNOTNEEDED))
		PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
}

DECLARE_CLIENT_EFFECT("NailImpact", NailImpactCallback);

//-----------------------------------------------------------------------------
// Purpose: Handle weapon impacts
//-----------------------------------------------------------------------------
void RailImpactCallback(const CEffectData &data)
{
	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;

	C_BaseEntity *pEntity = ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);

	if (!pEntity)
		return;

	// If we hit, perform our custom effects and play the sound
	if (Impact(vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr))
	{
		// Check for custom effects based on the Decal index
		FFPerformCustomEffects(vecOrigin, tr, vecShotDir, iMaterial, 1.0);

		//Play a ricochet sound some of the time
		if (random->RandomInt(1, 10) <= 3 && (iDamageType == DMG_BULLET))
		{
			CLocalPlayerFilter filter;
			C_BaseEntity::EmitSound(filter, SOUND_FROM_WORLD, "Bounce.Shrapnel", &vecOrigin);
		}
	}

	PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
}

DECLARE_CLIENT_EFFECT("RailImpact", RailImpactCallback);

//-----------------------------------------------------------------------------
// Purpose: Handle weapon impacts
//-----------------------------------------------------------------------------
void DartImpactCallback(const CEffectData &data)
{
	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;

	C_BaseEntity *pEntity = ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);

	if (!pEntity)
		return;

	// If we hit, perform our custom effects and play the sound
	if (Impact(vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr))
	{
		// Check for custom effects based on the Decal index
		FFPerformCustomEffects(vecOrigin, tr, vecShotDir, iMaterial, 1.0);

		//Play a ricochet sound some of the time
		if (random->RandomInt(1, 10) <= 3 && (iDamageType == DMG_BULLET))
		{
			CLocalPlayerFilter filter;
			C_BaseEntity::EmitSound(filter, SOUND_FROM_WORLD, "Bounce.Shrapnel", &vecOrigin);
		}
	}

	PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
}

DECLARE_CLIENT_EFFECT("DartImpact", DartImpactCallback);

//-----------------------------------------------------------------------------
// Purpose: Handle weapon impacts
//-----------------------------------------------------------------------------
// copied from ff_projectile_goop.h
enum GoopImpacts
{
	GOOP_IMPACT_HEAL = 0,
	GOOP_IMPACT_DAMAGE,
	GOOP_IMPACT_WORLD,

	GOOP_IMPACT_INVALID,
};

void GoopImpactCallback(const CEffectData &data)
{
	//GaussExplosion adds some cool white ember explosion
	CBroadcastRecipientFilter filter;
	switch(data.m_nDamageType)
	{
	case GOOP_IMPACT_WORLD:
		te->BloodSprite( filter, 0, &data.m_vOrigin, &data.m_vNormal, 255, 255, 255, 100, 10 );
		break;
	case GOOP_IMPACT_DAMAGE:
		te->BloodSprite( filter, 0, &data.m_vOrigin, &data.m_vNormal, 200, 0, 0, 200, 10 );
		break;
	case GOOP_IMPACT_HEAL:
		te->BloodSprite( filter, 0, &data.m_vOrigin, &data.m_vNormal, 40, 255, 40, 200, 10 );
		break;
	}
}

DECLARE_CLIENT_EFFECT("GoopImpact", GoopImpactCallback);