/********************************************************************
	created:	2006/08/14
	created:	14:8:2006   11:08
	filename: 	f:\ff-svn\code\trunk\game_shared\ff\ff_grenade_base.cpp
	file path:	f:\ff-svn\code\trunk\game_shared\ff
	file base:	ff_grenade_base
	file ext:	cpp
	author:		Various
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "ff_grenade_base.h"

#ifdef GAME_DLL
	#include "soundent.h"
	#include "te_effect_dispatch.h"
	#include "ff_player.h"
	#include "ff_utils.h"
	#include "ff_entity_system.h"
#else
	#include "c_te_effect_dispatch.h"
	#include "c_ff_player.h"
	#include "ff_grenade_parse.h"
#endif

//========================================================================
// CFFGrenadeBase tables
//========================================================================
#ifdef GAME_DLL
BEGIN_DATADESC(CFFGrenadeBase)
	DEFINE_THINKFUNC(GrenadeThink),
END_DATADESC()
#endif

//========================================================================
// Developer ConVars
//========================================================================
ConVar gren_grav("ffdev_gren_grav", "0.8", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar gren_fric("ffdev_gren_fric", "0.6", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar gren_elas("ffdev_gren_elas", "0.5", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar gren_radius("ffdev_gren_radius", "180.0f", FCVAR_REPLICATED | FCVAR_CHEAT, "Radius of grenade explosions");
ConVar gren_water_sink_rate("ffdev_gren_water_sink", "64.0", FCVAR_REPLICATED | FCVAR_CHEAT);
//ConVar gren_water_vel_dec("ffdev_gren_water_vel_dec", "0.5", FCVAR_REPLICATED | FCVAR_CHEAT);
#define GREN_WATER_VEL_DEC 0.5f
//ConVar gren_water_reduce_think("ffdev_gren_water_reduce_think", "0.2", FCVAR_REPLICATED);
#define GREN_WATER_REDUCE_THINK 0.2f

//=============================================================================
// CFFGrenadeBase implementation
//=============================================================================

#ifdef GAME_DLL

	extern short g_sModelIndexFireball;
	extern short g_sModelIndexWExplosion;

	//-----------------------------------------------------------------------------
	// Purpose: Set all the spawn stuff
	//-----------------------------------------------------------------------------
	void CFFGrenadeBase::Spawn()
	{
		BaseClass::Spawn();

		SetSolid(SOLID_BBOX);
		SetSolidFlags(FSOLID_NOT_STANDABLE);
		SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
		SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);

		// Rotate from the go
		SetLocalAngularVelocity(RandomAngle(-400, 400));

		// Grenade specific sets here
		SetGravity(GetGrenadeGravity());
		SetElasticity(GetGrenadeElasticity());
		SetFriction(GetGrenadeFriction());
		SetDamage(GetGrenadeDamage());
		SetDamageRadius(GetGrenadeRadius());

		// Don't take damage
		m_takedamage = DAMAGE_NO;

		// Some flag
		AddFlag(FL_GRENADE);

		// Flag for whether grenade has hit the water
		m_bHitwater = false;
		m_fIsHandheld = true;

		SetThink(&CFFGrenadeBase::GrenadeThink);
		SetNextThink(gpGlobals->curtime);
	}	

	//-----------------------------------------------------------------------------
	// Purpose: This'll be called once the grenade is actually thrown
	//-----------------------------------------------------------------------------
	void CFFGrenadeBase::SetDetonateTimerLength(float timer)
	{
		m_flDetonateTime = gpGlobals->curtime + timer;
	}

	//-----------------------------------------------------------------------------
	// Purpose: If we're trying to detonate, run through Lua first to check allowed
	//-----------------------------------------------------------------------------
	void CFFGrenadeBase::Detonate()
	{
		// Remove if not allowed by Lua 
		if (FFScriptRunPredicates(this, "onexplode", true) == false)
		{
			UTIL_Remove(this);
			return;
		}

		BaseClass::Detonate();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Check for end of fuse, stuff like that
	//-----------------------------------------------------------------------------
	void CFFGrenadeBase::GrenadeThink()
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

		// Bug #0000501: Doors can be blocked by shit that shouldn't block them.
		if( GetGroundEntity() && ( GetAbsVelocity() == vec3_origin ) )
		{
			if( GetGroundEntity()->GetMoveType() != MOVETYPE_PUSH )
				SetMoveType( MOVETYPE_NONE );
		}

		// Next think straight away
		SetNextThink(gpGlobals->curtime);

		// Check for water
		WaterCheck();		
	}

	//-----------------------------------------------------------------------------
	// Purpose: Mulch bug 0000273: make grens sink-ish in water
	//-----------------------------------------------------------------------------
	void CFFGrenadeBase::WaterCheck( void ) 
	{
		if (GetWaterLevel() != 0)
		{
			// 11/08/2005 - Defrag's content with this
			if (!m_bHitwater)
			{
				//DevMsg( "[greande] first hit the water: reducing velocity by %f\n", gren_water_vel_dec.GetFloat() );
				SetAbsVelocity(GetAbsVelocity() * GREN_WATER_VEL_DEC);
				m_bHitwater = true;

				m_flHitwaterTimer = gpGlobals->curtime;
			}

			if ((m_flHitwaterTimer + GREN_WATER_REDUCE_THINK) < gpGlobals->curtime)
			{
				//DevMsg( "[greande] under water: reducing velocity by %f\n", gren_water_vel_dec.GetFloat() );
				SetAbsVelocity(GetAbsVelocity() * GREN_WATER_VEL_DEC);
				m_flHitwaterTimer = gpGlobals->curtime;
			}
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: This is messy and probably shouldn't be done
	//-----------------------------------------------------------------------------
	void CFFGrenadeBase::ResolveFlyCollisionCustom(trace_t &trace, Vector &vecVelocity)
	{
		//Assume all surfaces have the same elasticity
		float flSurfaceElasticity = 1.0;

		//Don't bounce off of players with perfect elasticity
		if (trace.m_pEnt && trace.m_pEnt->IsPlayer())
		{
			flSurfaceElasticity = 0.3;
		}

		// if its breakable glass and we kill it, don't bounce.
		// give some damage to the glass, and if it breaks, pass
		// through it.
		bool breakthrough = false;

		if (trace.m_pEnt && FClassnameIs(trace.m_pEnt, "func_breakable"))
			breakthrough = true;

		if (trace.m_pEnt && FClassnameIs(trace.m_pEnt, "func_breakable_surf"))
			breakthrough = true;

		if (breakthrough)
		{
			CTakeDamageInfo info(this, this, 10, DMG_CLUB);
			trace.m_pEnt->DispatchTraceAttack(info, GetAbsVelocity(), &trace);

			ApplyMultiDamage();

			if (trace.m_pEnt->m_iHealth <= 0)
			{
				// slow our flight a little bit
				Vector vel = GetAbsVelocity();

				vel *= 0.4;

				SetAbsVelocity(vel);
				return;
			}
		}

		float flTotalElasticity = GetElasticity() * flSurfaceElasticity;
		flTotalElasticity = clamp(flTotalElasticity, 0.0f, 0.9f);

		// NOTE: A backoff of 2.0f is a reflection
		Vector vecAbsVelocity;
		PhysicsClipVelocity(GetAbsVelocity(), trace.plane.normal, vecAbsVelocity, 2.0f);
		vecAbsVelocity *= flTotalElasticity;

		// Get the total velocity (player + conveyors, etc.)
		VectorAdd(vecAbsVelocity, GetBaseVelocity(), vecVelocity);

		float flSpeedSqr = DotProduct(vecVelocity, vecVelocity);

		// Stop if on ground.
		if (trace.plane.normal.z > 0.7f)			// Floor
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
			}
			else
			{
				Vector vecDelta = GetBaseVelocity() - vecAbsVelocity;
				Vector vecBaseDir = GetBaseVelocity();
				VectorNormalize(vecBaseDir);

				float flScale = vecDelta.Dot(vecBaseDir);

				VectorScale(vecAbsVelocity, (1.0f - trace.fraction) * gpGlobals->frametime, vecVelocity);
				VectorMA(vecVelocity,
						(1.0f - trace.fraction) * gpGlobals->frametime,
						GetBaseVelocity() * flScale,
						vecVelocity);
				PhysicsPushEntity(vecVelocity, &trace);
			}
		}
		else
		{
			// If we get *too* slow, we'll stick without ever coming to rest because
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

	//-----------------------------------------------------------------------------
	// Purpose: Added so that grenades aren't using projectiles explode code.
	//			Grenades might need to look in more places than just below
	//			 them to see if scorch marks can be drawn.
	//-----------------------------------------------------------------------------
	void CFFGrenadeBase::Explode( trace_t *pTrace, int bitsDamageType )
	{
		SetModelName( NULL_STRING );//invisible
		AddSolidFlags( FSOLID_NOT_SOLID );

		m_takedamage = DAMAGE_NO;

		// Make sure grenade explosion is 32.0f above the ground.
		// In TFC exploding grenade explosions are ALWAYS 32.0f above the floor, except
		// in the case of the concussion grenade.
		if( pTrace->fraction != 1.0 )
			SetLocalOrigin( pTrace->endpos + ( pTrace->plane.normal * 32 ) );

		if( FFScriptRunPredicates( this, "onexplode", true ) )
		{
			Vector vecAbsOrigin = GetAbsOrigin();
			int contents = UTIL_PointContents( vecAbsOrigin );

			if( pTrace->fraction != 1.0 ) 
			{
				Vector vecNormal = pTrace->plane.normal;
				surfacedata_t *pdata = physprops->GetSurfaceData( pTrace->surface.surfaceProps );	
				CPASFilter filter( vecAbsOrigin );
				te->Explosion( filter, -1.0, // don't apply cl_interp delay
					&vecAbsOrigin, 
					! ( contents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion, 
					m_flDamage / 160, 
					25, 
					TE_EXPLFLAG_NONE, 
					m_DmgRadius, 
					m_flDamage, 
					&vecNormal, 
					( char )pdata->game.material );

				// Normal decals since trace hit something
				UTIL_DecalTrace( pTrace, "Scorch" );
			}
			else
			{
				CPASFilter filter( vecAbsOrigin );
				te->Explosion( filter, -1.0, // don't apply cl_interp delay
					&vecAbsOrigin, 
					! ( contents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion, 
					m_flDamage / 160, 
					25, 
					TE_EXPLFLAG_NONE, 
					m_DmgRadius, 
					m_flDamage );

				// Trace hit nothing so do custom scorch mark finding
				FF_DecalTrace( this, FF_DECALTRACE_TRACE_DIST, "Scorch" );
			}

			CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );

			CBaseEntity *pThrower = GetThrower();
			// Use the grenade's position as the reported position
			Vector vecReported = pTrace->endpos;
			CTakeDamageInfo info( this, pThrower, GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, 0, &vecReported );
			RadiusDamage( info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );

			EmitSound( "BaseGrenade.Explode" );
		}

		SetThink( &CBaseGrenade::SUB_Remove );
		SetTouch( NULL );

		AddEffects( EF_NODRAW );
		SetAbsVelocity( vec3_origin );
		SetNextThink( gpGlobals->curtime );
	}
#else
	//----------------------------------------------------------------------------
	// Purpose: Get script file grenade data
	//----------------------------------------------------------------------------
	const CFFGrenadeInfo &CFFGrenadeBase::GetFFGrenadeData() const
	{
		const CFFGrenadeInfo *pGrenadeInfo = GetFileGrenadeInfoFromHandle(m_hGrenadeFileInfo);

		Assert(pGrenadeInfo != NULL);

		return *pGrenadeInfo;
	}
#endif

void CFFGrenadeBase::Precache()
{
	//0000287: SV_StartSound: weapons/debris1.wav not precached (0)
	PrecacheScriptSound("BaseGrenade.Explode");

	BaseClass::Precache();
}
