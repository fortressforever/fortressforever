/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_grenade.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF grenade projectile code.
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation logged


#include "cbase.h"
#include "ff_projectile_grenade.h"

#define GRENADE_MODEL "models/weapons/pipelauncher/pipe.mdl"

#ifdef GAME_DLL
	#include "smoke_trail.h"

//=============================================================================
// CFFProjectileGrenade tables
//=============================================================================

	BEGIN_DATADESC(CFFProjectileGrenade) 
		DEFINE_THINKFUNC(GrenadeThink), 
	END_DATADESC() 
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFProjectileGrenade, DT_FFProjectileGrenade) 

BEGIN_NETWORK_TABLE(CFFProjectileGrenade, DT_FFProjectileGrenade) 
END_NETWORK_TABLE() 

LINK_ENTITY_TO_CLASS(glgrenade, CFFProjectileGrenade);
PRECACHE_WEAPON_REGISTER(glgrenade);

//=============================================================================
// CFFProjectileGrenade implementation
//=============================================================================

ConVar projectile_gren_friction("ffdev_projectile_gren_friction", "0.05", 0, "");
ConVar projectile_gren_elasticity("ffdev_projectile_gren_elasticity", "0.5", 0, "");
ConVar projectile_gren_gravity("ffdev_projectile_gren_gravity", "0.5", 0, "");

#ifdef GAME_DLL

	//----------------------------------------------------------------------------
	// Purpose: Creata a trail of smoke for the grenade
	//----------------------------------------------------------------------------
	void CFFProjectileGrenade::CreateSmokeTrail() 
	{
		if ((m_hSmokeTrail = SmokeTrail::CreateSmokeTrail()) != NULL) 
		{
			m_hSmokeTrail->m_Opacity = 0.2f;
			m_hSmokeTrail->m_SpawnRate = 100;
			m_hSmokeTrail->m_ParticleLifetime = 0.5f;
			m_hSmokeTrail->m_StartColor.Init(0.65f, 0.65f , 0.65f);
			m_hSmokeTrail->m_EndColor.Init(0.0, 0.0, 0.0);
			m_hSmokeTrail->m_StartSize = 4;	// 8
			m_hSmokeTrail->m_EndSize = 12;	// 32
			m_hSmokeTrail->m_SpawnRadius = 4;
			m_hSmokeTrail->m_MinSpeed = 2;
			m_hSmokeTrail->m_MaxSpeed = 16;
			
			m_hSmokeTrail->SetLifetime(999);
			m_hSmokeTrail->FollowEntity(this, "0");
		}
	}

	//----------------------------------------------------------------------------
	// Purpose: Spawn a grenade, set up model, size, etc
	//----------------------------------------------------------------------------
	void CFFProjectileGrenade::Spawn() 
	{
		// Setup
		SetModel(GRENADE_MODEL);
		m_nSkin = 1;	// Blue skin(#2) 

		SetSolidFlags(FSOLID_NOT_STANDABLE);
		SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
		SetSolid(SOLID_BBOX);	// So it will collide with physics props!

		// Hits everything but debris
		SetCollisionGroup(COLLISION_GROUP_PROJECTILE);

		// smaller, cube bounding box so we rest on the ground
		SetSize(Vector(-3, -3, -3), Vector(3, 3, 3));

		// Set the think
		SetThink(&CFFProjectileGrenade::GrenadeThink);		// |-- Mirv: Account for GCC strictness
		SetNextThink(gpGlobals->curtime);

		// Creates the smoke trail
		CreateSmokeTrail();

		BaseClass::Spawn();
	}

	//----------------------------------------------------------------------------
	// Purpose: Sets the time at which the grenade will explode
	//----------------------------------------------------------------------------
	void CFFProjectileGrenade::SetDetonateTimerLength(float timer) 
	{
		m_flDetonateTime = gpGlobals->curtime + timer;
	}

	//----------------------------------------------------------------------------
	// Purpose: Handles the bouncing of grenades
	//----------------------------------------------------------------------------
	void CFFProjectileGrenade::ResolveFlyCollisionCustom(trace_t &trace, Vector &vecVelocity) 
	{
		//Assume all surfaces have the same elasticity
		float flSurfaceElasticity = 1.0;

		//Don't bounce off of players with perfect elasticity
		if (trace.m_pEnt && trace.m_pEnt->IsPlayer()) 
		{
			// Explode on contact with people	
			if (m_bIsLive) 
				Detonate();
			else
				flSurfaceElasticity = 0.3;
		}

		// Bug #0000512: Blue pipes don't explode on impact on buildables
		// Explode on contact w/ buildables if pipe bomb
		if( trace.m_pEnt && ( ( trace.m_pEnt->Classify() == CLASS_SENTRYGUN ) || ( trace.m_pEnt->Classify() == CLASS_DISPENSER ) ) )
		{
			if( m_bIsLive )
				Detonate();
		}

		float flTotalElasticity = GetElasticity() * flSurfaceElasticity;
		flTotalElasticity = clamp(flTotalElasticity, 0.0f, 0.9f);

		// NOTE: A backoff of 2.0f is a reflection
		Vector vecAbsVelocity;
		PhysicsClipVelocity(GetAbsVelocity(), trace.plane.normal, vecAbsVelocity, 2.0f);
		vecAbsVelocity *= flTotalElasticity;

		// Get the total velocity(player + conveyors, etc.) 
		VectorAdd(vecAbsVelocity, GetBaseVelocity(), vecVelocity);
		float flSpeedSqr = DotProduct(vecVelocity, vecVelocity);

		// Stop if on ground.
		if (trace.plane.normal.z > 0.7f) 			// Floor
		{
			// Verify that we have an entity.
			CBaseEntity *pEntity = trace.m_pEnt;
			Assert(pEntity);

			SetAbsVelocity(vecAbsVelocity);

			if (flSpeedSqr < (30 * 30)) 
			{
				if (pEntity->IsStandable()) 
				{
					SetGroundEntity(pEntity);
				}

				// Reset velocities.
				SetAbsVelocity(vec3_origin);
				SetLocalAngularVelocity(vec3_angle);

				// Remove smoke BUG #0000126: Pipes from Launcher keeps emitting smoke after they are at rest.
				m_hSmokeTrail->SetEmit(false);

				////align to the ground so we're not standing on end
				//QAngle angle;
				//VectorAngles(trace.plane.normal, angle);

				//// rotate randomly in yaw
				//angle[1] = random->RandomFloat(0, 360);
				//
				//// TODO: rotate around trace.plane.normal
				//
				//SetAbsAngles(angle);

			}
			else
			{
				Vector vecDelta = GetBaseVelocity() - vecAbsVelocity;	
				Vector vecBaseDir = GetBaseVelocity();
				VectorNormalize(vecBaseDir);
				float flScale = vecDelta.Dot(vecBaseDir);

				VectorScale(vecAbsVelocity, (1.0f - trace.fraction) * gpGlobals->frametime, vecVelocity); 
				VectorMA(vecVelocity, (1.0f - trace.fraction) * gpGlobals->frametime, GetBaseVelocity() * flScale, vecVelocity);
				PhysicsPushEntity(vecVelocity, &trace);
			}
		}
		else
		{
			// If we get *too * slow, we'll stick without ever coming to rest because
			// we'll get pushed down by gravity faster than we can escape from the wall.
			if (flSpeedSqr < (30 * 30)) 
			{
				// Reset velocities.
				SetAbsVelocity(vec3_origin);
				SetLocalAngularVelocity(vec3_angle);
			}
			else
			{
				SetAbsVelocity(vecAbsVelocity);
			}
		}
		
		BounceSound();
	}


#endif

//----------------------------------------------------------------------------
// Purpose: Precache the grenade model
//----------------------------------------------------------------------------
void CFFProjectileGrenade::Precache() 
{
	PrecacheModel(GRENADE_MODEL);
	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Create a new grenade
//----------------------------------------------------------------------------
CFFProjectileGrenade * CFFProjectileGrenade::CreateGrenade(const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iSpeed) 
{
	CFFProjectileGrenade *pGrenade = (CFFProjectileGrenade *) CreateEntityByName("glgrenade");

	UTIL_SetOrigin(pGrenade, vecOrigin);
	pGrenade->SetAbsAngles(angAngles);
	pGrenade->Spawn();
	pGrenade->SetOwnerEntity(pentOwner);

	Vector vecForward;
	AngleVectors(angAngles, &vecForward);

	// Set the speed and the initial transmitted velocity
	pGrenade->SetAbsVelocity(vecForward * iSpeed);

#ifdef GAME_DLL
	pGrenade->SetupInitialTransmittedVelocity(vecForward * iSpeed);
	
	pGrenade->SetDetonateTimerLength(2.5);

	pGrenade->SetElasticity(GetGrenadeElasticity());
#endif

	pGrenade->m_flDamage = iDamage;
	pGrenade->m_DmgRadius = pGrenade->m_flDamage * 3.5f;

	pGrenade->m_bIsLive = true;

	pGrenade->SetThrower(pentOwner); 

	pGrenade->SetGravity(GetGrenadeGravity());
	pGrenade->SetFriction(GetGrenadeFriction());
	pGrenade->SetGravity(GetGrenadeGravity());

	pGrenade->SetLocalAngularVelocity(RandomAngle(-400, 400));

	return pGrenade; 
}

//----------------------------------------------------------------------------
// Purpose: Grenade think function
//----------------------------------------------------------------------------
void CFFProjectileGrenade::GrenadeThink() 
{
	// Remove if we're nolonger in the world
	if (!IsInWorld()) 
	{
		Remove();
		return;
	}

	// Blow up if we've reached the end of our fuse
	if (gpGlobals->curtime > m_flDetonateTime) 
	{
		Detonate();
		return;
	}

	// Next think straight away
	SetNextThink(gpGlobals->curtime);

	// BEG: Mulch - don't slow down in water
//	
//	// Slow down in water(need to fix this, will slow to a halt) 
//	if (GetWaterLevel() != 0) 
//	{
//		SetAbsVelocity(GetAbsVelocity() * 0.5);
//	}
	// END: Mulch
}
