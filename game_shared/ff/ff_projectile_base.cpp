/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_base.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief All projectiles derived from here; takes advantage of base_grenade code
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation


#include "cbase.h"
#include "ff_projectile_base.h"

#ifdef GAME_DLL
	#include "ff_player.h"
	#include "soundent.h"
	#include "util.h"
#else
	#include "c_ff_player.h"
#endif 

extern short	g_sModelIndexFireball;		// (in combatweapon.cpp) holds the index for the fireball 
extern short	g_sModelIndexWExplosion;	// (in combatweapon.cpp) holds the index for the underwater explosion
extern short	g_sModelIndexSmoke;			// (in combatweapon.cpp) holds the index for the smoke cloud

static ConVar damage_force_multiplier("ffdev_force_multiplier", "8.0f");

void FFRadiusDamage(const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore, CBaseEntity *pEntityIgnore);

//=============================================================================
// CFFProjectileBase tables
//=============================================================================

#ifdef GAME_DLL
	BEGIN_DATADESC(CFFProjectileBase) 
	END_DATADESC() 
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFProjectileBase, DT_FFProjectileBase) 

BEGIN_NETWORK_TABLE(CFFProjectileBase, DT_FFProjectileBase) 
	#ifdef CLIENT_DLL
		RecvPropVector(RECVINFO(m_vInitialVelocity)) 
	#else
		SendPropVector(SENDINFO(m_vInitialVelocity), 
			20, 		// nbits
			0, 		// flags
			-3000, 	// low value
			3000	// high value
			) 
	#endif
END_NETWORK_TABLE() 

//=============================================================================
// CFFProjectileBase implementation
//=============================================================================

#ifdef CLIENT_DLL

	//----------------------------------------------------------------------------
	// Purpose: Client constructor
	//----------------------------------------------------------------------------
	CFFProjectileBase::CFFProjectileBase()
	{
		m_fNeedsEngineSoundAttached = false;
	}

	//----------------------------------------------------------------------------
	// Purpose: Add initial velocity into the interpolation history so that
	//			interp works okay
	//----------------------------------------------------------------------------
	void CFFProjectileBase::PostDataUpdate(DataUpdateType_t type) 
	{
		BaseClass::PostDataUpdate(type);

		if (type == DATA_UPDATE_CREATED) 
		{
			// Now stick our initial velocity into the interpolation history 
			CInterpolatedVar< Vector > &interpolator = GetOriginInterpolator();
			
			interpolator.ClearHistory();
			float changeTime = GetLastChangeTime(LATCH_SIMULATION_VAR);

			// Add a sample 1 second back.
			//VOOGRU: Taken this out due to #0000706
			/*
			Vector vCurOrigin = GetLocalOrigin() - m_vInitialVelocity;
			interpolator.AddToHead(changeTime - 1.0f, &vCurOrigin, false);
			*/

			// Add the current sample.
			Vector vCurOrigin = GetLocalOrigin();
			interpolator.AddToHead(changeTime, &vCurOrigin, false);

			// This projectile has entered the client's PVS so flag it as needing
			// a sound attached. We can't directly start the sound here because
			// the entity is not yet ready for queries (eg. GetAbsOrigin())
			if (GetFlightSound())
				m_fNeedsEngineSoundAttached = true;
		}
	}

	//----------------------------------------------------------------------------
	// Purpose: 
	//----------------------------------------------------------------------------
	int CFFProjectileBase::DrawModel(int flags) 
	{
		// Just putting this here for now.. not the best place for it admittedly
		if (m_fNeedsEngineSoundAttached)
		{
			EmitSound(GetFlightSound());
			m_fNeedsEngineSoundAttached = false;
		}

		return BaseClass::DrawModel(flags);
	}

	//----------------------------------------------------------------------------
	// Purpose: Entity is being released, so stop the sound effect
	//----------------------------------------------------------------------------
	void CFFProjectileBase::Release()
	{
		if (GetFlightSound())
			StopSound(GetFlightSound());

		BaseClass::Release();
	}

#else

	//----------------------------------------------------------------------------
	// Purpose: Specify what velocity we want to have on the client immediately.
	//			Without this, the entity wouldn't have an interpolation history initially, so it would
	//			sit still until it had gotten a few updates from the server.
	//
	//			[FIXME] The problem with this is that the projectiles now appear
	//					further back than their actual origin
	//----------------------------------------------------------------------------
	void CFFProjectileBase::SetupInitialTransmittedVelocity(const Vector &velocity) 
	{
		m_vInitialVelocity = velocity;
	}

	//------------------------------------------------------------------------
	// Purpose: Currently using this so we can track where a player was hit
	//------------------------------------------------------------------------
	void FFRadiusDamage(const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore, CBaseEntity *pEntityIgnore) 
	{
		CBaseEntity *pEntity = NULL;
		trace_t		tr;
		float		flAdjustedDamage, falloff;
		Vector		vecSpot;

		Vector vecSrc = vecSrcIn;

		if (flRadius) 
			falloff = info.GetDamage() / flRadius;
		else
			falloff = 1.0;

		// int bInWater = (UTIL_PointContents(vecSrc) & MASK_WATER) ? true : false;

		vecSrc.z += 1;// in case grenade is lying on the ground

		// iterate on all entities in the vicinity.
		for (CEntitySphereQuery sphere(vecSrc, flRadius); (pEntity = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity()) 
		{
			if (pEntity == pEntityIgnore) 
				continue;

			if (pEntity->m_takedamage == DAMAGE_NO) 
				continue;

			// Check that the explosion can 'see' this entity.
			vecSpot = pEntity->BodyTarget(vecSrc, true);

			// Bugfix for #0000598: Backpacks blocking grenade damage
			UTIL_TraceLine(vecSrc, vecSpot, MASK_SHOT, info.GetInflictor(), /*COLLISION_GROUP_NONE*/ COLLISION_GROUP_PROJECTILE, &tr);

			// Hmmm?????????
			DevMsg("Explode length: %f %f %f\n", (pEntity->GetAbsOrigin() - vecSrc).Length(), (vecSpot - vecSrc).Length(), (tr.endpos - vecSrc).Length());

			if (tr.fraction != 1.0 && tr.m_pEnt != pEntity) 
				continue;

			// decrease damage for an ent that's farther from the bomb.
			flAdjustedDamage = (vecSrc - tr.endpos).Length() * falloff;
			flAdjustedDamage = info.GetDamage() - flAdjustedDamage;

			if (flAdjustedDamage <= 0) 
				continue;

			// the explosion can 'see' this entity, so hurt them!

			// if we're stuck inside them, fixup the position and distance
			if (tr.startsolid) 
			{
				tr.endpos = vecSrc;
				tr.fraction = 0.0;
			}
			
			CTakeDamageInfo adjustedInfo = info;

			adjustedInfo.SetDamage(flAdjustedDamage);

			Vector dir = vecSpot - vecSrc;
			VectorNormalize(dir);

			float origforce = 0;


			// If we don't have a damage force, manufacture one
			if (adjustedInfo.GetDamagePosition() == vec3_origin || adjustedInfo.GetDamageForce() == vec3_origin) 
			{
				float coeff = 1.0f;

				CFFPlayer *ffplayer = ToFFPlayer(pEntity);

				// The HWG isn't budged as much by explosions
				if (ffplayer && ffplayer->GetClassSlot() == CLASS_HWGUY) 
					coeff = 0.15f;

				adjustedInfo.SetDamageForce(dir * info.GetDamage() * damage_force_multiplier.GetFloat() * coeff);
				adjustedInfo.SetDamagePosition(vecSrc);

				origforce = adjustedInfo.GetDamageForce().Length();
			}

			// We'll have a damage force by now, 
			// so work out how much it should be at this point
			float forcefalloff, flAdjustedForce;

			if (flRadius) 
				forcefalloff = adjustedInfo.GetDamageForce().Length() / flRadius;
			else
				forcefalloff = 1.0;

			// decrease damage for an ent that's farther from the bomb.
			flAdjustedForce = (vecSrc - vecSpot).Length() * forcefalloff;
			flAdjustedForce = adjustedInfo.GetDamageForce().Length() - flAdjustedForce;

			//DevMsg("%f %f %f\n", info.GetDamageForce().Length(), forcefalloff, flAdjustedForce);

			// Set this new force
			adjustedInfo.SetDamageForce(dir * flAdjustedForce);
			adjustedInfo.SetDamagePosition(vecSrc);

			// Some stuff
//			DevMsg("info.damage %f\ninfo.force %f\nadjusted.damage %f\nadjusted.force %f\n", info.GetDamage(), (info.GetDamageForce().Length() != 0 ? info.GetDamageForce().Length() : origforce), adjustedInfo.GetDamage(), adjustedInfo.GetDamageForce().Length());

			// Now deal the damage
			pEntity->TakeDamage(adjustedInfo);

			// For the moment we'll play blood effects if its a teammate too so its consistant with other weapons
//			if (pEntity->IsPlayer() && g_pGameRules->FPlayerCanTakeDamage(ToFFPlayer(pEntity), info.GetAttacker())) 
//			{
				// Bug #0000539: Blood decals are projected onto shit
				// (direction needed normalising)
				Vector vecTraceDir = (tr.endpos - tr.startpos);
				VectorNormalize(vecTraceDir);

				// Bug #0000168: Blood sprites for damage on players do not display
				SpawnBlood(tr.endpos, vecTraceDir, pEntity->BloodColor(), adjustedInfo.GetDamage() * 3.0f);

                pEntity->TraceBleed(adjustedInfo.GetDamage(), vecTraceDir, &tr, adjustedInfo.GetDamageType());
//			}

			// Now hit all triggers along the way that respond to damage... 
			pEntity->TraceAttackToTriggers(adjustedInfo, vecSrc, tr.endpos, dir);
		}
	}

	//----------------------------------------------------------------------------
	// Purpose: If caught in an emp then destroy
	//----------------------------------------------------------------------------
	int CFFProjectileBase::TakeEmp() 
	{
		UTIL_Remove(this);
		return m_flDamage;
	}

#endif

//----------------------------------------------------------------------------
// Purpose: Keep track of when spawned
//----------------------------------------------------------------------------
void CFFProjectileBase::Spawn() 
{
	m_flSpawnTime = gpGlobals->curtime;
	m_flNextBounceSoundTime = gpGlobals->curtime;

	BaseClass::Spawn();

	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
}

//----------------------------------------------------------------------------
// Purpose: Precache the bounce and flight sounds
//----------------------------------------------------------------------------
void CFFProjectileBase::Precache() 
{
	PrecacheScriptSound(GetBounceSound());

	if (GetFlightSound())
		PrecacheScriptSound(GetFlightSound());
}

//----------------------------------------------------------------------------
// Purpose: Play a bounce sound with a minimum interval
//----------------------------------------------------------------------------
void CFFProjectileBase::BounceSound()
{
	if (gpGlobals->curtime > m_flNextBounceSoundTime && GetAbsVelocity().LengthSqr() > 1)
	{
		EmitSound(GetBounceSound());

		m_flNextBounceSoundTime = gpGlobals->curtime + 0.1;
	}	
}