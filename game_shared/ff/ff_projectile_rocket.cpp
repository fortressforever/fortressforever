/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_rocket.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF rocket projectile code.
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation logged


#include "cbase.h"
#include "ff_projectile_rocket.h"

#define ROCKET_MODEL "models/projectiles/rocket/w_rocket.mdl"

#ifdef GAME_DLL
	#include "smoke_trail.h"
#endif

//=============================================================================
// CFFProjectileRocket tables
//=============================================================================

LINK_ENTITY_TO_CLASS(rocket, CFFProjectileRocket);
PRECACHE_WEAPON_REGISTER(rocket);

//=============================================================================
// CFFProjectileRocket implementation
//=============================================================================

#ifdef GAME_DLL

	//----------------------------------------------------------------------------
	// Purpose: Creata a trail of smoke for the rocket
	//----------------------------------------------------------------------------
	void CFFProjectileRocket::CreateSmokeTrail() 
	{
		// Smoke trail.
		if ((m_hRocketTrail = RocketTrail::CreateRocketTrail()) != NULL) 
		{
			m_hRocketTrail->m_Opacity = 0.2f;
			m_hRocketTrail->m_SpawnRate = 100;
			m_hRocketTrail->m_ParticleLifetime = 0.5f;
			m_hRocketTrail->m_StartColor.Init(0.65f, 0.65f , 0.65f);
			m_hRocketTrail->m_EndColor.Init(0.0, 0.0, 0.0);
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
	void CFFProjectileRocket::Spawn() 
	{
		// Setup
		SetModel(ROCKET_MODEL);
		SetMoveType(MOVETYPE_FLY);
		SetSize(Vector(-2, -2, -2), Vector(2, 2, 2)); // smaller, cube bounding box so we rest on the ground
		SetSolid(SOLID_BBOX);	// So it will collide with physics props!
		SetSolidFlags(FSOLID_NOT_STANDABLE);

		// Set the correct think & touch for the nail
		SetTouch(&CFFProjectileRocket::ExplodeTouch); // No we're going to explode when we touch something
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
void CFFProjectileRocket::Precache() 
{
	PrecacheModel(ROCKET_MODEL);
	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Create a new rocket
//----------------------------------------------------------------------------
CFFProjectileRocket * CFFProjectileRocket::CreateRocket(const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iSpeed) 
{
	CFFProjectileRocket *pRocket = (CFFProjectileRocket *) CreateEntityByName("rocket");

	UTIL_SetOrigin(pRocket, vecOrigin);
	pRocket->SetAbsAngles(angAngles);
	pRocket->Spawn();
	pRocket->SetOwnerEntity(pentOwner);

	Vector vecForward;
	AngleVectors(angAngles, &vecForward);

	// Set the speed and the initial transmitted velocity
	pRocket->SetAbsVelocity(vecForward * iSpeed);

#ifdef GAME_DLL
	pRocket->SetupInitialTransmittedVelocity(vecForward * iSpeed);
#endif

	pRocket->m_flDamage = iDamage;

	// Bug #0000231: Rocket splash is incorrect
	// TFC doesn't multiply the radius at all, dont you know
	pRocket->m_DmgRadius = pRocket->m_flDamage;

	return pRocket; 
}
