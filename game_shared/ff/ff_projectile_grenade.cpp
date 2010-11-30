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
#include "IEffects.h"
#include "ff_buildableobjects_shared.h"

#define GRENADE_MODEL "models/projectiles/pipe/w_pipe.mdl"

#ifdef GAME_DLL
	#include "smoke_trail.h"

//=============================================================================
// CFFProjectileGrenade tables
//=============================================================================

	BEGIN_DATADESC(CFFProjectileGrenade) 
		DEFINE_THINKFUNC( GrenadeThink ), 
		DEFINE_ENTITYFUNC( ExplodeTouch ),
	END_DATADESC() 
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFProjectileGrenade, DT_FFProjectileGrenade) 

BEGIN_NETWORK_TABLE(CFFProjectileGrenade, DT_FFProjectileGrenade) 
END_NETWORK_TABLE() 

LINK_ENTITY_TO_CLASS(ff_projectile_gl, CFFProjectileGrenade);
PRECACHE_WEAPON_REGISTER(ff_projectile_gl);

//=============================================================================
// CFFProjectileGrenade implementation
//=============================================================================

ConVar projectile_gren_friction("ffdev_projectile_gren_friction", "0.375", FCVAR_REPLICATED | FCVAR_CHEAT, "");
ConVar projectile_gren_elasticity("ffdev_projectile_gren_elasticity", "0.5", FCVAR_REPLICATED | FCVAR_CHEAT, "");
ConVar projectile_gren_gravity("ffdev_projectile_gren_gravity", "1.0", FCVAR_REPLICATED | FCVAR_CHEAT, "");
//ConVar projectile_gren_fusetime("ffdev_projectile_gren_fusetime", "1.3", FCVAR_REPLICATED | FCVAR_CHEAT, "");
#define FF_PROJECTILE_GREN_FUSETIME 1.3f //projectile_gren_fusetime.GetFloat();
ConVar ffdev_bluepipes_bonusdirectdmg("ffdev_bluepipes_bonusdirectdmg", "18.0", FCVAR_REPLICATED, "");
#define FF_PROJECTILE_GREN_BONUSDIRECTDMG ffdev_bluepipes_bonusdirectdmg.GetFloat()
ConVar ffdev_bluepipes_normaldmg("ffdev_bluepipes_normaldmg", "70.0", FCVAR_REPLICATED, "");
#define FF_PROJECTILE_GREN_NORMALDMG ffdev_bluepipes_normaldmg.GetFloat()
ConVar ffdev_bluepipes_size("ffdev_bluepipes_size", "2.0", FCVAR_REPLICATED, "(int) size of bounding box, 1-5 are good values");
ConVar ffdev_bluepipes_usefusetime("ffdev_bluepipes_usefusetime", "1", FCVAR_REPLICATED,"Use the blue pipe fuse time?" );
ConVar ffdev_bluepipes_num_bounces("ffdev_bluepipes_num_bounces", "0", FCVAR_REPLICATED );
ConVar ffdev_bluepipes_sticky("ffdev_bluepipes_sticky", "1", FCVAR_REPLICATED );
ConVar ffdev_pipe_sticky("ffdev_pipe_sticky", "1", FCVAR_REPLICATED );

#define FFDEV_PIPE_STICKY ffdev_pipe_sticky.GetBool()
#define FFDEV_BLUEPIPES_STICKY ffdev_bluepipes_sticky.GetBool()
#define FFDEV_BLUEPIPES_NUM_BOUNCES ffdev_bluepipes_num_bounces.GetInt()
#define FFDEV_BLUEPIPES_SIZE ffdev_bluepipes_size.GetInt()
#define FFDEV_BLUEPIPES_USEFUSETIME ffdev_bluepipes_usefusetime.GetBool()


#ifdef GAME_DLL

	//----------------------------------------------------------------------------
	// Purpose: Creata a trail of smoke for the grenade
	//----------------------------------------------------------------------------
	void CFFProjectileGrenade::CreateProjectileEffects() 
	{
		int nAttachment = LookupAttachment( "fuse" );

		// Start up the eye trail
		m_hGlowTrail = CSpriteTrail::SpriteTrailCreate( "sprites/bluelaser1.vmt", GetLocalOrigin(), false );

		if ( m_hGlowTrail != NULL )
		{
			m_hGlowTrail->FollowEntity( this );
			m_hGlowTrail->SetAttachment( this, nAttachment );
			m_hGlowTrail->SetTransparency( kRenderTransAdd, 85, 95, 205, 255, kRenderFxNone );
			m_hGlowTrail->SetStartWidth( 10.0f );
			m_hGlowTrail->SetEndWidth( 5.0f );
			m_hGlowTrail->SetLifeTime( 0.5f );
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
		m_nNumBounces = 0;

		SetSolidFlags(FSOLID_NOT_STANDABLE);
		SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
		SetSolid(SOLID_BBOX);	// So it will collide with physics props!

		// smaller, cube bounding box so we rest on the ground
		SetSize(Vector(-FFDEV_BLUEPIPES_SIZE, -FFDEV_BLUEPIPES_SIZE, -FFDEV_BLUEPIPES_SIZE), Vector(FFDEV_BLUEPIPES_SIZE, FFDEV_BLUEPIPES_SIZE, FFDEV_BLUEPIPES_SIZE));
		
		SetThink(&CFFProjectileGrenade::GrenadeThink);		// |-- Mirv: Account for GCC strictness
		SetNextThink(gpGlobals->curtime);


		// Creates the smoke trail
		CreateProjectileEffects();

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
			if (ExplodeOnHitPlayer()) 
			{
				CBasePlayer *pVictim = dynamic_cast< CBasePlayer* > ( trace.m_pEnt ); // (AFTERSHOCK): Extra damage applied to player here
				pVictim->TakeDamage( CTakeDamageInfo( this, GetOwnerEntity(), FF_PROJECTILE_GREN_BONUSDIRECTDMG , DMG_BLAST ) );
							//CTakeDamageInfo info( this, pThrower, GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, 0, &vecReported );
				Detonate();
				return;
			}
			else
				flSurfaceElasticity = 0.3;
		}

		// Bug #0000512: Blue pipes don't explode on impact on buildables
		// Explode on contact w/ buildables if pipe bomb
		if( trace.m_pEnt && ( ( trace.m_pEnt->Classify() == CLASS_SENTRYGUN ) || ( trace.m_pEnt->Classify() == CLASS_DISPENSER ) ) )
		{
			if( m_bIsLive )
			{
				CFFBuildableObject *pVictim = dynamic_cast< CFFBuildableObject* > ( trace.m_pEnt ); // (AFTERSHOCK): Extra damage applied to buildable here
				pVictim->TakeDamage( CTakeDamageInfo( this, GetOwnerEntity(), FF_PROJECTILE_GREN_BONUSDIRECTDMG , DMG_BLAST ) );
							//CTakeDamageInfo info( this, pThrower, GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, 0, &vecReported );
				Detonate();// TODO: (AFTERSHOCK): Extra damage applied to buildable here
				return;
			}
		}

		//Check if it's sticky - AfterShock
		// WARNING: You must turn on fusetime variable :)
		if( trace.m_pEnt && ( Classify() == CLASS_GLGRENADE && FFDEV_BLUEPIPES_STICKY ) || ( Classify() == CLASS_PIPEBOMB && FFDEV_PIPE_STICKY ))
		{
			SetLocalAngularVelocity(QAngle(0, 0, 0));	// stop spinning
			SetAbsVelocity(Vector(0,0,0));				// stop moving
			//SetGravity(0);					// Stop falling
			if (trace.m_pEnt && trace.m_pEnt->IsStandable()) 
			{
				SetGroundEntity(trace.m_pEnt);
			}
			if ( UTIL_PointContents( GetAbsOrigin() ) != CONTENTS_WATER)
			{
				g_pEffects->Sparks( GetAbsOrigin() );
			}
			return;
		}

		//Check if it should blow up or continue bouncing -GreenMushy
		if( trace.m_pEnt && Classify() == CLASS_GLGRENADE && m_nNumBounces >= FFDEV_BLUEPIPES_NUM_BOUNCES )
		{
			Detonate();
			return;
		}

		float flTotalElasticity = GetElasticity() * flSurfaceElasticity;
		flTotalElasticity = clamp(flTotalElasticity, 0.0f, 0.9f);

		// UNDONE: A backoff of 2.0f is a reflection
		// In HL it seems to use an overbounce of 1.5, so we're using 
		// 1.0 + default elasticity = 1.5
		Vector vecAbsVelocity;
		PhysicsClipVelocity(GetAbsVelocity(), trace.plane.normal, vecAbsVelocity, 1.0f + GetElasticity());
		//vecAbsVelocity *= flTotalElasticity;

		// Some new friction calculating. 200 is a base figure which *looks* correct
		float flSpeed = vecAbsVelocity.Length();
		float flNewSpeed = flSpeed - 200 * GetFriction();

		flNewSpeed /= flSpeed;

		if (flNewSpeed < 0)
			flNewSpeed = 0;

		// Better download slope handling
		if (vecAbsVelocity.z < 0)
			vecAbsVelocity.z = 0;
		
		vecAbsVelocity.x *= flNewSpeed;
		vecAbsVelocity.y *= flNewSpeed;

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

			// Fix for #0001538: pipes bounce indefinitely (with tickrate 33)
			if (flSpeedSqr < (50 * 50) || (vecAbsVelocity.x == 0.0f && flSpeedSqr < (70*70))) 
			{
				if (pEntity->IsStandable()) 
				{
					SetGroundEntity(pEntity);
				}

				// Reset velocities.
				SetAbsVelocity(vec3_origin);
				SetLocalAngularVelocity(vec3_angle);

				// Remove smoke BUG #0000126: Pipes from Launcher keeps emitting smoke after they are at rest.

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
		m_nNumBounces++;
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
CFFProjectileGrenade * CFFProjectileGrenade::CreateGrenade(const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iDamageRadius, const int iSpeed) 
{
	CFFProjectileGrenade *pGrenade = (CFFProjectileGrenade *) CreateEntityByName("ff_projectile_gl");

	UTIL_SetOrigin(pGrenade, vecOrigin);
	pGrenade->SetAbsAngles(angAngles);
	pGrenade->Spawn();
	pGrenade->SetOwnerEntity(pentOwner);
	pGrenade->m_iSourceClassname = (pSource ? pSource->m_iClassname : NULL_STRING);

	Vector vecForward;
	AngleVectors(angAngles, &vecForward);

	// Set the speed and the initial transmitted velocity
	pGrenade->SetAbsVelocity(vecForward * iSpeed);

#ifdef GAME_DLL
	pGrenade->SetupInitialTransmittedVelocity(vecForward * iSpeed);
	
	pGrenade->SetDetonateTimerLength( FF_PROJECTILE_GREN_FUSETIME );

	pGrenade->SetElasticity(GetGrenadeElasticity());
#endif

	//pGrenade->SetDamage(iDamage); 
	pGrenade->SetDamage(FF_PROJECTILE_GREN_NORMALDMG);//AfterShock: cvar for damage while we test direct damage bonus
	pGrenade->SetDamageRadius(iDamageRadius);

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
	if ( FFDEV_BLUEPIPES_USEFUSETIME && gpGlobals->curtime > m_flDetonateTime) 
	{
		Detonate();
		return;
	}

	// Bug #0000501: Doors can be blocked by shit that shouldn't block them.
	if( GetGroundEntity() && ( GetAbsVelocity() == vec3_origin ) )
	{
		if( GetGroundEntity()->GetMoveType() != MOVETYPE_PUSH )
			SetMoveType( MOVETYPE_NONE );
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
