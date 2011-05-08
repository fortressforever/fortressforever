/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life2 ========
///
/// @file ff_weapon_basemelee.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF Melee weapon code, all melee weapons derived from here
///
/// REVISIONS
/// ---------
/// Jan 18, 2005 Mirv: Added to project


#include "cbase.h"
#include "ff_weapon_basemelee.h"

#ifdef CLIENT_DLL
	#include "c_ff_player.h"
#else
	#include "omnibot_interface.h"
	#include "ilagcompensationmanager.h"
#endif

#define MELEE_HULL_DIM		16 //If this is changed, need to change the hardcoded cube root of 2 around line 305

static const Vector g_meleeMins(-MELEE_HULL_DIM, -MELEE_HULL_DIM, -MELEE_HULL_DIM);
static const Vector g_meleeMaxs(MELEE_HULL_DIM, MELEE_HULL_DIM, MELEE_HULL_DIM);

//=============================================================================
// CFFWeaponMeleeBase tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponMeleeBase, DT_FFWeaponMeleeBase) 

BEGIN_NETWORK_TABLE(CFFWeaponMeleeBase, DT_FFWeaponMeleeBase) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponMeleeBase) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_basemelee, CFFWeaponMeleeBase);

//=============================================================================
// CFFWeaponMeleeBase implementation
//=============================================================================

ConVar melee_reach("ffdev_meleereach", "32.0", FCVAR_FF_FFDEV_REPLICATED);

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponMeleeBase::CFFWeaponMeleeBase() 
{
}

//----------------------------------------------------------------------------
// Purpose: Spawn the weapon
//----------------------------------------------------------------------------
void CFFWeaponMeleeBase::Spawn() 
{
	//Call base class first
	BaseClass::Spawn();

	CFFWeaponInfo wpndata = GetFFWpnData();
	m_fMinRange1	= 0;
	m_fMinRange2	= 0;

// 0000732: added convar to allow for tweakage
//	m_fMaxRange1	= wpndata.m_flRange;
//	m_fMaxRange2	= wpndata.m_flRange;
	m_fMaxRange1	= melee_reach.GetFloat();
	m_fMaxRange2	= melee_reach.GetFloat();

}

//----------------------------------------------------------------------------
// Purpose: Precache the weapon
//----------------------------------------------------------------------------
void CFFWeaponMeleeBase::Precache() 
{
	//Call base class first
	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Player is 'firing' the melee weapon, so swing it
//----------------------------------------------------------------------------
void CFFWeaponMeleeBase::PrimaryAttack() 
{
	CANCEL_IF_BUILDING();
	CANCEL_IF_CLOAKED();

	Swing();
}

//----------------------------------------------------------------------------
// Purpose: None
//----------------------------------------------------------------------------
void CFFWeaponMeleeBase::SecondaryAttack() 
{
}

//----------------------------------------------------------------------------
// Purpose: Implement impact function
//----------------------------------------------------------------------------
void CFFWeaponMeleeBase::Hit(trace_t &traceHit, Activity nHitActivity) 
{
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	//DevMsg("[CFFWeaponMeleeBase] Hit\n");
	
	//Do view kick
	AddViewKick();

	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	//Apply damage to a hit target
	if (pHitEntity != NULL) 
	{
		if (pHitEntity->m_takedamage != DAMAGE_NO) 
		{
			Vector hitDirection;
			pPlayer->EyeVectors(&hitDirection, NULL, NULL);
			VectorNormalize(hitDirection);

			CFFWeaponInfo wpndata = GetFFWpnData();
			CTakeDamageInfo info(this, GetOwner(), wpndata.m_iDamage, DMG_CLUB);
			info.SetDamageForce(hitDirection * MELEE_IMPACT_FORCE);

			if (!pHitEntity->IsPlayer())
			{
				info.ScaleDamageForce(10.0f);
			}

			info.SetDamagePosition(traceHit.endpos);

			pHitEntity->DispatchTraceAttack(info, hitDirection, &traceHit); 
			ApplyMultiDamage();

#ifdef GAME_DLL
			// Now hit all triggers along the ray that... 
			TraceAttackToTriggers(info, traceHit.startpos, traceHit.endpos, hitDirection);
#endif
		}
	}

	// Apply an impact effect
	ImpactEffect(traceHit);
}

//----------------------------------------------------------------------------
// Purpose: Calculate the point where melee weapon hits & animation to play
//----------------------------------------------------------------------------
Activity CFFWeaponMeleeBase::ChooseIntersectionPointAndActivity(trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner) 
{
	int			i, j, k;
	float		distance;
	const float	*minmaxs[2] = {mins.Base(), maxs.Base() };
	trace_t		tmpTrace;
	Vector		vecHullEnd = hitTrace.endpos;
	Vector		vecEnd;

	distance = 1e6f;
	Vector vecSrc = hitTrace.startpos;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) *2);
	UTIL_TraceLine(vecSrc, vecHullEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace);
	if (tmpTrace.fraction == 1.0) 
	{
		for (i = 0; i < 2; i++) 
		{
			for (j = 0; j < 2; j++) 
			{
				for (k = 0; k < 2; k++) 
				{
					vecEnd.x = vecHullEnd.x + minmaxs[i][0];
					vecEnd.y = vecHullEnd.y + minmaxs[j][1];
					vecEnd.z = vecHullEnd.z + minmaxs[k][2];

					UTIL_TraceLine(vecSrc, vecEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace);
					if (tmpTrace.fraction < 1.0) 
					{
						float thisDistance = (tmpTrace.endpos - vecSrc).Length();
						if (thisDistance < distance) 
						{
							hitTrace = tmpTrace;
							distance = thisDistance;
						}
					}
				}
			}
		}
	}
	else
	{
		hitTrace = tmpTrace;
	}


	return ACT_VM_HITCENTER;
} 

//----------------------------------------------------------------------------
// Purpose: Handle water splashes
//----------------------------------------------------------------------------
bool CFFWeaponMeleeBase::ImpactWater(const Vector &start, const Vector &end) 
{
	//FIXME: This doesn't handle the case of trying to splash while being underwater, but that's not going to look good
	//		 right now anyway...
	
	// We must start outside the water
	if (UTIL_PointContents(start) & (CONTENTS_WATER|CONTENTS_SLIME)) 
		return false;

	// We must end inside of water
	if (! (UTIL_PointContents(end) & (CONTENTS_WATER|CONTENTS_SLIME))) 
		return false;

	trace_t	waterTrace;

	UTIL_TraceLine(start, end, (CONTENTS_WATER|CONTENTS_SLIME), GetOwner(), COLLISION_GROUP_NONE, &waterTrace);

	if (waterTrace.fraction < 1.0f) 
	{
		CEffectData	data;

		data.m_fFlags  = 0;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = 8.0f;

		// See if we hit slime
		if (waterTrace.contents & CONTENTS_SLIME) 
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}

		DispatchEffect("watersplash", data);			
	}

	return true;
}

//----------------------------------------------------------------------------
// Purpose: Handle decals/debris from hitting something
//----------------------------------------------------------------------------
void CFFWeaponMeleeBase::ImpactEffect(trace_t &traceHit) 
{
	// See if we hit water(we don't do the other impact effects in this case) 
	if (ImpactWater(traceHit.startpos, traceHit.endpos)) 
		return;

	//FIXME: need new decals
	UTIL_ImpactTrace(&traceHit, DMG_CLUB);
}

//----------------------------------------------------------------------------
// Purpose: Starts the swing of the weapon and determines the animation
//----------------------------------------------------------------------------
void CFFWeaponMeleeBase::Swing() 
{
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();
	
	trace_t traceHit;

	// Try a ray
	CFFPlayer *pOwner = ToFFPlayer(GetOwner());
	
	if (!pOwner) 
		return;

#ifdef GAME_DLL
	// Since spies only actually have the knife, this shouldn't really be needed.
	// But you can never tell what direction this mod is going to go in and whether
	// spies will end up somehow getting other melee weapons.
	if (GetWeaponID() != FF_WEAPON_KNIFE)
	{
		pOwner->ResetDisguise();
	}

	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation(pOwner, pOwner->GetCurrentCommand());
#endif

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	pOwner->EyeVectors(&forward, NULL, NULL);

	// 0000732 Vector swingEnd = swingStart + forward * (pWeaponInfo.m_flRange + 20.0f);
	Vector swingEnd = swingStart + forward * (melee_reach.GetFloat() + 20.0f);
	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);

	Activity nHitActivity = ACT_VM_HITCENTER;

	// Like bullets, melee traces have to trace against triggers.
	CTakeDamageInfo triggerInfo(this, GetOwner(), GetDamageForActivity(nHitActivity), DMG_CLUB);

#ifdef GAME_DLL
	TraceAttackToTriggers(triggerInfo, traceHit.startpos, traceHit.endpos, vec3_origin);
#endif

	if (traceHit.fraction == 1.0) 
	{
		float meleeHullRadius = 1.732f * MELEE_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * meleeHullRadius;

		UTIL_TraceHull(swingStart, swingEnd, g_meleeMins, g_meleeMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);

		if (traceHit.fraction < 1.0 && traceHit.m_pEnt) 
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize(vecToTarget);

			float dot = vecToTarget.Dot(forward);

			// YWB:  Make sure they are sort of facing the guy at least...
			if (dot < 0.70721f) 
			{
				// Force amiss
				traceHit.fraction = 1.0f;
			}
			else
			{
				nHitActivity = ChooseIntersectionPointAndActivity(traceHit, g_meleeMins, g_meleeMaxs, pOwner);
			}
		}
	}

	// Play swing sound first
	WeaponSound(SINGLE);

	//	Miss
	if (traceHit.fraction == 1.0f) 
	{
		nHitActivity = ACT_VM_MISSCENTER;

		// We want to test the first swing again
		// 0000732		Vector testEnd = swingStart + forward * pWeaponInfo.m_flRange;
		Vector testEnd = swingStart + forward * melee_reach.GetFloat();
		// See if we happened to hit water
		ImpactWater(swingStart, testEnd);

#ifdef GAME_DLL
		// If this _IS_ a knife, then always undisguise if they miss with it
		// The rest of the undisguise logic is handled by the knife itself
		if (GetWeaponID() == FF_WEAPON_KNIFE)
		{
			pOwner->ResetDisguise();
		}
#endif
	}
	else
	{
		Hit(traceHit, nHitActivity);
	}

	// Send the anim
	SendWeaponAnim(nHitActivity);

	pOwner->DoAnimationEvent(PLAYERANIMEVENT_FIRE_GUN_PRIMARY);

	//Setup our next attack times
	m_flNextPrimaryAttack = gpGlobals->curtime + pWeaponInfo.m_flCycleTime;
	//m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

#ifdef GAME_DLL
	lagcompensation->FinishLagCompensation(pOwner);
#endif

#ifdef GAME_DLL
	Omnibot::Notify_PlayerShoot(pOwner, Omnibot::obUtilGetBotWeaponFromGameWeapon(GetWeaponID()), 0);
#endif
}
