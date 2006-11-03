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
	#include "ff_buildableobjects_shared.h"
	#include "omnibot_interface.h"
#endif

//=============================================================================
// CFFProjectileIncendiaryRocket tables
//=============================================================================

LINK_ENTITY_TO_CLASS(ff_projectile_ic, CFFProjectileIncendiaryRocket);
PRECACHE_WEAPON_REGISTER(ff_projectile_ic);

#ifdef GAME_DLL
BEGIN_DATADESC(CFFProjectileIncendiaryRocket)
DEFINE_THINKFUNC(ArcThink),
END_DATADESC()
#endif

//=============================================================================
// CFFProjectileIncendiaryRocket implementation
//=============================================================================

void CFFProjectileIncendiaryRocket::Explode(trace_t *pTrace, int bitsDamageType)
{
	// Make sure grenade explosion is 32.0f away from hit point
	if (pTrace->fraction != 1.0)
		SetLocalOrigin(pTrace->endpos + (pTrace->plane.normal * 32));

#ifdef GAME_DLL

	Vector vecSrc = GetAbsOrigin();
	vecSrc.z += 1;

	// Do normal radius damage then do a trace sphere to set things alight
	Vector vecReported = vec3_origin;
	CTakeDamageInfo info(this, GetOwnerEntity(), GetBlastForce(), GetAbsOrigin(), m_flDamage, DMG_BURN, 0, &vecReported);
	RadiusDamage(info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL);
	
	// Sorry, not fond of the BEGIN_ENTITY_SPHERE_QUERY macro
	CBaseEntity *pEntity = NULL;
	for( CEntitySphereQuery sphere( vecSrc, m_DmgRadius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if( !pEntity || !pEntity->IsPlayer() )
			continue;

		CFFPlayer *pPlayer = ToFFPlayer( pEntity );
		if( g_pGameRules->FPlayerCanTakeDamage( pPlayer, GetOwnerEntity() ) )
		{
			pPlayer->ApplyBurning(pPlayer, 0.5f, 10.0f, BURNTYPE_ICCANNON);
		}
	}

	// We don't use this and it also caused a NULL pointer assert from
	// GetOwnerEntity()
	//Vector vecDisp = GetOwnerEntity()->GetAbsOrigin() - GetAbsOrigin();

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
		if ((m_hRocketTrail = RocketTrail::CreateRocketTrail()) != NULL)
		{
			m_hRocketTrail->m_Opacity = 0.2f;
			m_hRocketTrail->m_SpawnRate = 100;
			m_hRocketTrail->m_ParticleLifetime = 0.5f;
			m_hRocketTrail->m_StartColor.Init(1.0f, 0.3f, 0.0f);
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
		SetThink(&CFFProjectileIncendiaryRocket::ArcThink);

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
CFFProjectileIncendiaryRocket * CFFProjectileIncendiaryRocket::CreateRocket(const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iSpeed)
{
	CFFProjectileIncendiaryRocket *pRocket = (CFFProjectileIncendiaryRocket *) CreateEntityByName("ff_projectile_ic");

	UTIL_SetOrigin(pRocket, vecOrigin);
	pRocket->SetAbsAngles(angAngles);
	pRocket->Spawn();
	pRocket->SetOwnerEntity(pentOwner);
	pRocket->SetGravity(0.6f);
	pRocket->m_iSourceClassname = (pSource ? pSource->m_iClassname : NULL_STRING);

	Vector vecForward;
	AngleVectors(angAngles, &vecForward);

	// Set the speed and the initial transmitted velocity
	pRocket->SetAbsVelocity(vecForward * iSpeed);

#ifdef GAME_DLL
	pRocket->SetupInitialTransmittedVelocity(vecForward * iSpeed);
#endif

	pRocket->m_flDamage = iDamage;
	pRocket->m_DmgRadius = pRocket->m_flDamage * 2.0f;

#ifdef GAME_DLL
	{
		CBasePlayer *pPlayer = ToBasePlayer(pentOwner);
		if(pPlayer->IsBot())
			Omnibot::Notify_PlayerShootProjectile(pPlayer, pRocket->edict());
	}
#endif

	return pRocket; 
}

//-----------------------------------------------------------------------------
// Purpose: Orient the model to follow the arc
//-----------------------------------------------------------------------------
void CFFProjectileIncendiaryRocket::ArcThink()
{
	QAngle angDir;
	VectorAngles(GetAbsVelocity(), angDir);

	SetAbsAngles(angDir);

	SetNextThink(gpGlobals->curtime + 0.1f);
}