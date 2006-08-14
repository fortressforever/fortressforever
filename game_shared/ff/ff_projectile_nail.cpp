/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_nail.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF nail projectile code.
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation logged

// NB. THIS SHOULD BE A TEMPENT!!!!
//		EDIT: THIS *IS* A TEMPENT NOW!!!!

#include "cbase.h"
#include "ff_projectile_nail.h"
#include "effect_dispatch_data.h"
#include "IEffects.h"
#include "ammodef.h"

#define NAIL_MODEL "models/projectiles/nail/w_nail.mdl"

ConVar ffdev_nail_speed("ffdev_nail_speed", "2000.0", FCVAR_REPLICATED, "Nail speed");
ConVar ffdev_nail_bbox("ffdev_nail_bbox", "2.0", FCVAR_REPLICATED, "Nail bbox");


#ifdef CLIENT_DLL
	#include "c_te_effect_dispatch.h"
#else
	#include "te_effect_dispatch.h"

//=============================================================================
// CFFProjectileNail tables
//=============================================================================

	BEGIN_DATADESC(CFFProjectileNail) 
		DEFINE_THINKFUNC(BubbleThink), 
		DEFINE_ENTITYFUNC(NailTouch), 
	END_DATADESC() 
#endif

LINK_ENTITY_TO_CLASS(ff_projectile_nail, CFFProjectileNail);
PRECACHE_WEAPON_REGISTER(ff_projectile_nail);

//=============================================================================
// CFFProjectileNail implementation
//=============================================================================

#ifdef GAME_DLL

	//----------------------------------------------------------------------------
	// Purpose: Spawn a nail, set up model, size, etc
	//----------------------------------------------------------------------------
	void CFFProjectileNail::Spawn() 
	{
		// Setup
		SetModel(NAIL_MODEL);
		SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
		SetSize(-Vector(1.0f, 1.0f, 1.0f) * ffdev_nail_bbox.GetFloat(), Vector(1.0f, 1.0f, 1.0f) * ffdev_nail_bbox.GetFloat());
		SetSolid(SOLID_BBOX);
		SetGravity(0.01f);
		SetEffects(EF_NODRAW);
		
		// Set the correct think & touch for the nail
		SetTouch(&CFFProjectileNail::NailTouch);		// |-- Mirv: Account for GCC strictness
		SetThink(&CFFProjectileNail::BubbleThink);	// |-- Mirv: Account for GCC strictness

		// Next think(ie. how bubbly it'll be) 
		SetNextThink(gpGlobals->curtime + 0.1f);
		
		// Make sure we're updated if we're underwater
		UpdateWaterState();

		// Initialize
		m_bNailGrenadeNail = false;

		BaseClass::Spawn();
	}

#endif

//----------------------------------------------------------------------------
// Purpose: Precache the nail model
//----------------------------------------------------------------------------
void CFFProjectileNail::Precache() 
{
	PrecacheModel(NAIL_MODEL);

	PrecacheScriptSound("Nail.HitBody");
	PrecacheScriptSound("Nail.HitWorld");

	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: The nail touch function
//----------------------------------------------------------------------------
void CFFProjectileNail::NailTouch(CBaseEntity *pOther) 
{
	// The projectile has not hit anything valid so far
	if (!pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS) || !g_pGameRules->ShouldCollide(GetCollisionGroup(), pOther->GetCollisionGroup())) 
		return;

	trace_t	tr;
	tr = BaseClass::GetTouchTrace();

	// This entity can take damage, so deal it out
	if (pOther->m_takedamage != DAMAGE_NO) 
	{
#ifdef GAME_DLL
		Vector	vecNormalizedVel = GetAbsVelocity();
		VectorNormalize(vecNormalizedVel);

		ClearMultiDamage();

		CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), m_flDamage, DMG_BULLET | DMG_NEVERGIB);
		CalculateBulletDamageForce(&dmgInfo, GetAmmoDef()->Index("AMMO_NAILS"), vecNormalizedVel, tr.endpos);
		dmgInfo.SetDamagePosition(tr.endpos);

		if (pOther->IsPlayer())
		{
			dmgInfo.ScaleDamageForce(0.1f);
		}

		pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);

		ApplyMultiDamage();
#endif

		// Keep going through the glass.
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS) 
			 return;

		// Play body "thwack" sound
		EmitSound("Nail.HitBody");
	}

	// Now just remove the nail
	Remove();
}

//----------------------------------------------------------------------------
// Purpose: Make a trail of bubbles
//----------------------------------------------------------------------------
void CFFProjectileNail::BubbleThink() 
{
	QAngle angNewAngles;

	VectorAngles(GetAbsVelocity(), angNewAngles);
	SetAbsAngles(angNewAngles);

	SetNextThink(gpGlobals->curtime + 5.0f);

	if (GetWaterLevel() == 0) 
		return;

#ifdef GAME_DLL
	
	// Nails from nailgrens won't necessarily show any bubbles at all
	if (GetOwnerEntity() && GetOwnerEntity()->Classify() == CLASS_GREN_NAIL && random->RandomInt(0, 4) > 0)
		return;

	UTIL_BubbleTrail(GetAbsOrigin() - GetAbsVelocity() * 0.1f, GetAbsOrigin(), 1);
#endif
}

//----------------------------------------------------------------------------
// Purpose: Create a new nail
//----------------------------------------------------------------------------
CFFProjectileNail *CFFProjectileNail::CreateNail(const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwner, const int iDamage, const int iSpeed) 
{
	CFFProjectileNail *pNail = (CFFProjectileNail *) CreateEntityByName("nail");

	UTIL_SetOrigin(pNail, vecOrigin);
	pNail->SetAbsAngles(angAngles);
	pNail->Spawn();
	pNail->SetOwnerEntity(pentOwner);
	pNail->m_iSourceClassname = (pSource ? pSource->m_iClassname : NULL_STRING);

	Vector vecForward;
	AngleVectors(angAngles, &vecForward);

	vecForward *= /*iSpeed*/ ffdev_nail_speed.GetFloat();

	// Set the speed and the initial transmitted velocity
	pNail->SetAbsVelocity(vecForward);

	CEffectData data;
	data.m_vOrigin = vecOrigin;
	data.m_vAngles = angAngles;

#ifdef GAME_DLL
	data.m_nEntIndex = pentOwner->entindex();
#else
	data.m_hEntity = pentOwner;
#endif

	DispatchEffect("Projectile_Nail", data);

#ifdef GAME_DLL
	pNail->SetupInitialTransmittedVelocity(vecForward);
#endif

	pNail->m_flDamage = iDamage;

	return pNail;
}
