/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_rocket.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date August 3, 2005
/// @brief The FF incendiary rocket projectile code.
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation logged


#include "cbase.h"
#include "ff_utils.h"
#include "ff_projectile_incendiaryrocket.h"

#define INCENDIARYROCKET_MODEL "models/gibs/AGIBS.mdl"

#ifdef GAME_DLL
	#include "smoke_trail.h"
#endif

//=============================================================================
// CFFProjectileIncendiaryRocket tables
//=============================================================================

LINK_ENTITY_TO_CLASS(incendiaryrocket, CFFProjectileIncendiaryRocket);
PRECACHE_WEAPON_REGISTER(incendiaryrocket);

//=============================================================================
// CFFProjectileIncendiaryRocket implementation
//=============================================================================

void CFFProjectileIncendiaryRocket::Explode(trace_t *pTrace, int bitsDamageType)
{
#ifdef GAME_DLL
	DevMsg("[CFFProjectileIncendiaryRocket::Explode] Start\n");

	Vector vecSrc = GetAbsOrigin();
	vecSrc.z += 1;// in case grenade is lying on the ground
	BEGIN_ENTITY_SPHERE_QUERY(vecSrc, m_DmgRadius)
		Class_T cls = pEntity->Classify();
		switch (cls)
		{
		case CLASS_PLAYER:
			{
				if (pPlayer)
				{
					// damage enemies and self
					if (pPlayer == GetThrower() || g_pGameRules->FPlayerCanTakeDamage(pPlayer, GetThrower()))
					{
						DevMsg("[Grenade Debug] damaging enemy or self\n");
						CTakeDamageInfo info(this, pPlayer, 1.0f, DMG_BURN);
						pEntity->TakeDamage(info);

						pPlayer->ApplyBurning(ToFFPlayer(GetThrower()), 1.0f);
					}
				}
			}
			break;
		case CLASS_SENTRYGUN:
			{
			}
			break;
		case CLASS_DISPENSER:
			{
			}
			break;
		default:
			break;
		}
		DevMsg("[Grenade Debug] Checking next entity\n");
	END_ENTITY_SPHERE_QUERY();

	// set the damage to 0 since we already did damage
	m_flDamage = 0;

	Vector vecDisp = GetOwnerEntity()->GetAbsOrigin() - GetAbsOrigin();

	DevMsg("Distance: %f\n", vecDisp.Length());

#endif

	BaseClass::Explode(pTrace, bitsDamageType);
}


#ifdef GAME_DLL

	//----------------------------------------------------------------------------
	// Purpose: Creata a trail of smoke for the rocket
	//----------------------------------------------------------------------------
	void CFFProjectileIncendiaryRocket::CreateSmokeTrail()
	{
		// Smoke trail.
		//if ((m_hRocketTrail = RocketTrail::CreateRocketTrail()) != NULL)
		if ((m_hRocketTrail = RocketTrail::CreateRocketTrail()) != NULL)
		{
			m_hRocketTrail->m_Opacity = 0.2f;
			m_hRocketTrail->m_SpawnRate = 100;
			m_hRocketTrail->m_ParticleLifetime = 0.5f;
			m_hRocketTrail->m_StartColor.Init(0.6f, 0.1f, 0.1f);
			m_hRocketTrail->m_EndColor.Init(0.0f, 0.0f, 0.0f);
			m_hRocketTrail->m_StartSize = 8;
			m_hRocketTrail->m_EndSize = 32;
			m_hRocketTrail->m_SpawnRadius = 4;
			m_hRocketTrail->m_MinSpeed = 2;
			m_hRocketTrail->m_MaxSpeed = 16;
			
			m_hRocketTrail->SetLifetime(999);
			m_hRocketTrail->FollowEntity(this, "0");
		}
	}

	//----------------------------------------------------------------------------
	// Purpose: Spawn a rocket, set up model, size, etc
	//----------------------------------------------------------------------------
	void CFFProjectileIncendiaryRocket::Spawn()
	{
		// Setup
		SetModel(INCENDIARYROCKET_MODEL);
		SetMoveType(MOVETYPE_FLYGRAVITY);
		SetSize(Vector(-2, -2, -2), Vector(2, 2, 2)); // smaller, cube bounding box so we rest on the ground
		SetSolid(SOLID_BBOX);	// So it will collide with physics props!
		SetSolidFlags(FSOLID_NOT_STANDABLE);

		// Set the correct think & touch for the nail
		SetTouch(&CFFProjectileIncendiaryRocket::ExplodeTouch); // No we're going to explode when we touch something
		SetThink(NULL);		// no thinking!

		// Next think
		SetNextThink(gpGlobals->curtime);

		// Creates the smoke trail
		CreateSmokeTrail();

		BaseClass::Spawn();
	}

#endif

//----------------------------------------------------------------------------
// Purpose: Precache the rocket model
//----------------------------------------------------------------------------
void CFFProjectileIncendiaryRocket::Precache()
{
	PrecacheModel(INCENDIARYROCKET_MODEL);
	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Create a new rocket
//----------------------------------------------------------------------------
CFFProjectileIncendiaryRocket * CFFProjectileIncendiaryRocket::CreateRocket(const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iSpeed)
{
	CFFProjectileIncendiaryRocket *pRocket = (CFFProjectileIncendiaryRocket *) CreateEntityByName("incendiaryrocket");

	UTIL_SetOrigin(pRocket, vecOrigin);
	pRocket->SetAbsAngles(angAngles);
	pRocket->Spawn();
	pRocket->SetOwnerEntity(pentOwner);
	pRocket->SetGravity(0.6f);

	Vector vecForward;
	AngleVectors(angAngles, &vecForward);

	// Set the speed and the initial transmitted velocity
	pRocket->SetAbsVelocity(vecForward * iSpeed);

#ifdef GAME_DLL
	pRocket->SetupInitialTransmittedVelocity(vecForward * iSpeed);
#endif

	pRocket->m_flDamage = iDamage;
	pRocket->m_DmgRadius = pRocket->m_flDamage * 3.5f;

	return pRocket; 
}
