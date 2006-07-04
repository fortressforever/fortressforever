
/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_grenade_base.cpp
/// @author Shawn Smith (L0ki)
/// @date Dec. 10, 2004
/// @brief implementation of the base class for all primeable grenades
///
/// All primeable grenades in the game inherit from this class. These grenades include:
/// - Frag
/// - Caltrops
/// - Concussion
/// - Nail
/// - MIRV
/// - Napalm
/// - Gas
/// - EMP
///
/// Revisions
/// ---------
/// Dec. 10, 2004	L0ki: Initial Creation
/// Mar. 20, 2005	Mirv: Updated some stuff, i don't like this html business
/// Apr. 23, 2005	L0ki: removed the html stuff, made some minor modifications to the code structure
/// Jan. 15, 2006   Mirv: Tidied this up a LOT!

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
#endif

extern short	g_sModelIndexFireball;		// (in combatweapon.cpp) holds the index for the fireball 
extern short	g_sModelIndexWExplosion;	// (in combatweapon.cpp) holds the index for the underwater explosion
extern short	g_sModelIndexSmoke;			// (in combatweapon.cpp) holds the index for the smoke cloud

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
#ifdef GAME_DLL
	ConVar gren_grav("ffdev_gren_grav", "0.8");
	ConVar gren_fric("ffdev_gren_fric", "0.6");
	ConVar gren_elas("ffdev_gren_elas", "0.5");
	ConVar gren_radius("ffdev_gren_radius", "180.0f", 0, "Radius of grenade explosions");
	ConVar gren_water_sink_rate("ffdev_gren_water_sink", "64.0");
	ConVar gren_water_vel_dec("ffdev_gren_water_vel_dec", "0.5");
	ConVar gren_water_reduce_think("ffdev_gren_water_reduce_think", "0.2");
#endif

//=============================================================================
// CFFGrenadeBase implementation
//=============================================================================
int CFFGrenadeBase::m_iShockwaveTexture = -1;
int CFFGrenadeBase::m_iRingTexture = -1;
int CFFGrenadeBase::m_iFlameSprite = -1;

#ifdef GAME_DLL

	void CFFGrenadeBase::Spawn()
	{
		BaseClass::Spawn();

		SetSolid(SOLID_BBOX);
		SetSolidFlags(FSOLID_NOT_STANDABLE);
		SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
		SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);

		SetSize(Vector(-3, -3, -3), Vector(3, 3, 3));

		// Rotate from the go
		SetLocalAngularVelocity(RandomAngle(-400, 400));

		// Grenade specific sets here
		SetGravity(GetGrenadeGravity());
		SetElasticity(GetGrenadeElasticity());
		SetFriction(GetGrenadeFriction());
		SetDamage(GetGrenadeDamage());
		m_DmgRadius = GetGrenadeRadius();

		// Flag for whether grenade has hit the water
		m_bHitwater = false;
		m_fIsHandheld = true;

		SetThink(&CFFGrenadeBase::GrenadeThink);
		SetNextThink(gpGlobals->curtime);
	}	

	void CFFGrenadeBase::SetDetonateTimerLength(float timer)
	{
		m_flDetonateTime = gpGlobals->curtime + timer;
	}

	void CFFGrenadeBase::PreExplode(trace_t *pTrace)
	{
		m_takedamage = DAMAGE_NO;

		// Pull out of the wall a bit
		if (pTrace->fraction != 1.0)
			SetLocalOrigin(pTrace->endpos + (pTrace->plane.normal * 0.6));
	}

	void CFFGrenadeBase::PreExplode(trace_t *pTrace, const char *pSound, const char *pEffect)
	{
		m_takedamage = DAMAGE_NO;

		// Pull out of the wall a bit
		if (pTrace->fraction != 1.0)
			SetLocalOrigin(pTrace->endpos + (pTrace->plane.normal * 0.6));

		// Bail here if in a no gren area
		if( !FFScriptRunPredicates( this, "onexplode", true ) )
			return;

		if (pSound)
			EmitSound(pSound);

		if (pEffect)
		{
			CEffectData data;
			data.m_vOrigin = GetAbsOrigin();
			data.m_flScale = 1.0f;
			DispatchEffect(pEffect, data);
		}
	}

	void CFFGrenadeBase::PostExplode()
	{
		SetModelName(NULL_STRING);

		AddSolidFlags(FSOLID_NOT_SOLID);
		AddEffects(EF_NODRAW);

		SetNextThink(gpGlobals->curtime);

		SetThink(&CBaseGrenade::SUB_Remove);
		SetTouch(NULL);

		SetAbsVelocity(vec3_origin);
	}

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

		CFFGrenadeBase::WaterThink();		
	}

	// Mulch: bug 0000273: make grens sink-ish in water
	void CFFGrenadeBase::WaterThink( void ) 
	{
		if (GetWaterLevel() != 0)
		{
			// 11/08/2005 - Defrag's content with this
			if (!m_bHitwater)
			{
				//DevMsg( "[greande] first hit the water: reducing velocity by %f\n", gren_water_vel_dec.GetFloat() );
				SetAbsVelocity(GetAbsVelocity() * gren_water_vel_dec.GetFloat());
				m_bHitwater = true;

				m_flHitwaterTimer = gpGlobals->curtime;
			}

			if ((m_flHitwaterTimer + gren_water_reduce_think.GetFloat()) < gpGlobals->curtime)
			{
				//DevMsg( "[greande] under water: reducing velocity by %f\n", gren_water_vel_dec.GetFloat() );
				SetAbsVelocity(GetAbsVelocity() * gren_water_vel_dec.GetFloat());
				m_flHitwaterTimer = gpGlobals->curtime;
			}
		}
	}

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

				////align to the ground so we're not standing on end
				//QAngle angle;
				//VectorAngles( trace.plane.normal, angle );

				//// rotate randomly in yaw
				//angle[1] = random->RandomFloat( 0, 360 );

				//// TODO: rotate around trace.plane.normal

				//SetAbsAngles( angle );
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
#endif

// Added so that grenades aren't using projectiles explode code.
// Grenades might need to look in more places than just below
// them to see if scorch marks can be drawn.
void CFFGrenadeBase::Explode( trace_t *pTrace, int bitsDamageType )
{
#ifdef GAME_DLL
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
				m_DmgRadius * .03, 
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
				m_DmgRadius * .03, 
				25, 
				TE_EXPLFLAG_NONE, 
				m_DmgRadius, 
				m_flDamage );

			// Trace hit nothing so do custom scorch mark finding
			FF_DecalTrace( this, FF_DECALTRACE_TRACE_DIST, "Scorch" );
		}

		CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );

		CBaseEntity *pThrower = GetThrower();
		// Use the thrower's position as the reported position
		Vector vecReported = pThrower ? pThrower->GetAbsOrigin() : vec3_origin;
		CTakeDamageInfo info( this, pThrower, GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, 0, &vecReported );
		RadiusDamage( info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );

		EmitSound( "BaseGrenade.Explode" );
	}

	SetThink( &CBaseGrenade::SUB_Remove );
	SetTouch( NULL );

	AddEffects( EF_NODRAW );
	SetAbsVelocity( vec3_origin );
	SetNextThink( gpGlobals->curtime );
#endif
}

void CFFGrenadeBase::Precache()
{
	m_iShockwaveTexture = PrecacheModel("sprites/spotlight.vmt");
	m_iRingTexture = PrecacheModel("sprites/smoke.vmt");
	m_iFlameSprite = PrecacheModel("sprites/fire_floor.vmt");

	//0000287: SV_StartSound: weapons/debris1.wav not precached (0)
	PrecacheScriptSound("BaseGrenade.Explode");

	BaseClass::Precache();
}
