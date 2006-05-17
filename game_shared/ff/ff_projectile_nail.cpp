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

LINK_ENTITY_TO_CLASS(nail, CFFProjectileNail);
PRECACHE_WEAPON_REGISTER(nail);

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
		SetSize(-Vector(1, 1, 1), Vector(1, 1, 1));
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
	if (!pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS) || !g_pGameRules->ShouldCollide(GetCollisionGroup(), pOther->GetCollisionGroup())) 
		return;

	if (pOther->m_takedamage != DAMAGE_NO) 
	{
		trace_t	tr, tr2;
		tr = BaseClass::GetTouchTrace();
		Vector	vecNormalizedVel = GetAbsVelocity();

#ifdef GAME_DLL
		ClearMultiDamage();
		VectorNormalize(vecNormalizedVel);

		CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), m_flDamage, DMG_BULLET | DMG_NEVERGIB);
		CalculateBulletDamageForce(&dmgInfo, GetAmmoDef()->Index("AMMO_NAILS"), vecNormalizedVel, tr.endpos);
		dmgInfo.SetDamagePosition(tr.endpos);
		pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);

		ApplyMultiDamage();
#endif

		//Adrian: keep going through the glass.
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS) 
			 return;

		SetAbsVelocity(Vector(0, 0, 0));

		// play body "thwack" sound
		EmitSound("Nail.HitBody");

		Vector vForward;

		AngleVectors(GetAbsAngles(), &vForward);
		VectorNormalize(vForward);

		UTIL_TraceLine(GetAbsOrigin(), 	GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr2);

		if (tr2.fraction != 1.0f) 
		{
			if (tr2.m_pEnt == NULL || (tr2.m_pEnt && tr2.m_pEnt->GetMoveType() == MOVETYPE_NONE)) 
			{
				CEffectData	data;

				data.m_vOrigin = tr2.endpos;
				data.m_vNormal = vForward;
				data.m_nEntIndex = tr2.fraction != 1.0f;
			
				DispatchEffect("NailImpact", data);
			}
		}
		
		SetTouch(NULL);
		SetThink(NULL);

		Vector vecDir = GetAbsVelocity();
		float speed = VectorNormalize(vecDir);

		// Spark if we hit an entity which wasn't human(ie. player) 
		if (!pOther->IsPlayer() && UTIL_PointContents(GetAbsOrigin()) != CONTENTS_WATER && speed > 500) 
		{
            g_pEffects->Sparks(GetAbsOrigin());
		}

		Remove();
	}
	else
	{
		trace_t	tr;
		tr = BaseClass::GetTouchTrace();

		// See if we struck the world
		if (pOther->GetMoveType() == MOVETYPE_NONE && ! (tr.surface.flags & SURF_SKY)) 
		{
			EmitSound("Nail.HitWorld");

			// if what we hit is static architecture, can stay around for a while.
			Vector vecDir = GetAbsVelocity();
			float speed = VectorNormalize(vecDir);

			SetThink(&CFFProjectileNail::SUB_Remove);
			SetNextThink(gpGlobals->curtime + 2.0f);
			
			//FIXME: We actually want to stick(with hierarchy) to what we've hit
			SetMoveType(MOVETYPE_NONE);
		
			Vector vForward;

			AngleVectors(GetAbsAngles(), &vForward);
			VectorNormalize(vForward);

			CEffectData	data;

			data.m_vOrigin = tr.endpos;
			data.m_vNormal = vForward;
			data.m_nEntIndex = 0;
		
			DispatchEffect("NailImpact", data);
			
			UTIL_ImpactTrace(&tr, DMG_BULLET);

			AddEffects(EF_NODRAW);
			SetTouch(NULL);
			SetThink(&CFFProjectileNail::SUB_Remove);		// |-- Mirv: Account for GCC strictness
			SetNextThink(gpGlobals->curtime + 2.0f);

			// Shoot some sparks
			if (UTIL_PointContents(GetAbsOrigin()) != CONTENTS_WATER && speed > 500) 
			{
				g_pEffects->Sparks(GetAbsOrigin());
			}
		}
		else
		{
			// Put a mark unless we've hit the sky
			if ((tr.surface.flags & SURF_SKY) == false) 
			{
				UTIL_ImpactTrace(&tr, DMG_BULLET);
			}

			Remove();
		}
	}
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
CFFProjectileNail *CFFProjectileNail::CreateNail(const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwner, const int iDamage, const int iSpeed) 
{
	CFFProjectileNail *pNail = (CFFProjectileNail *) CreateEntityByName("nail");

	UTIL_SetOrigin(pNail, vecOrigin);
	pNail->SetAbsAngles(angAngles);
	pNail->Spawn();
	pNail->SetOwnerEntity(pentOwner);

	Vector vecForward;
	AngleVectors(angAngles, &vecForward);

	// Set the speed and the initial transmitted velocity
	pNail->SetAbsVelocity(vecForward * iSpeed);

	CEffectData data;
	data.m_vOrigin = vecOrigin;
	data.m_vAngles = angAngles;
	data.m_nEntIndex = pentOwner->entindex();
	DispatchEffect("Projectile_Nail", data);

#ifdef GAME_DLL
	pNail->SetupInitialTransmittedVelocity(vecForward * iSpeed);
#endif

	pNail->m_flDamage = iDamage;

	return pNail;
}
