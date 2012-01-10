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
	#include "beamdraw.h"
	#include "cmodel.h"
#endif

//========================================================================
// CFFGrenadeBase tables
//========================================================================
#ifdef GAME_DLL
BEGIN_DATADESC(CFFGrenadeBase)
	DEFINE_THINKFUNC(GrenadeThink),
END_DATADESC()
#endif

#ifdef CLIENT_DLL
CLIENTEFFECT_REGISTER_BEGIN(PrecacheGrenadeSprite)
CLIENTEFFECT_MATERIAL("sprites/ff_target")
CLIENTEFFECT_MATERIAL("sprites/ff_target_blur")
CLIENTEFFECT_MATERIAL("sprites/ff_trail")
CLIENTEFFECT_REGISTER_END()
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeBase, DT_FFGrenadeBase)

BEGIN_NETWORK_TABLE(CFFGrenadeBase, DT_FFGrenadeBase)
#ifdef CLIENT_DLL
	RecvPropFloat(RECVINFO(m_flSpawnTime)),
	RecvPropFloat(RECVINFO(m_flDetonateTime)),
	RecvPropBool(RECVINFO(m_bIsOn)),
#else
	SendPropFloat(SENDINFO(m_flSpawnTime)),
	SendPropFloat(SENDINFO(m_flDetonateTime)),
	SendPropBool(SENDINFO(m_bIsOn)),
#endif
END_NETWORK_TABLE()

//BEGIN_PREDICTION_DATA(CFFGrenadeBase)
//END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(grenade_ff_base, CFFGrenadeBase);

//========================================================================
// Developer ConVars
//========================================================================
ConVar gren_grav("ffdev_gren_grav", "0.8", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar gren_fric("ffdev_gren_fric", "0.25", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar gren_elas("ffdev_gren_elas", "0.4", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar gren_fric_conc("ffdev_gren_fric_conc", "0.3", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar gren_elas_conc("ffdev_gren_elas_conc", "0.7", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar gren_radius("ffdev_gren_radius", "180.0f", FCVAR_REPLICATED | FCVAR_CHEAT, "Radius of grenade explosions");
ConVar gren_water_sink_rate("ffdev_gren_water_sink", "64.0", FCVAR_REPLICATED | FCVAR_CHEAT);
//ConVar gren_water_vel_dec("ffdev_gren_water_vel_dec", "0.5", FCVAR_REPLICATED | FCVAR_CHEAT);
#define GREN_WATER_VEL_DEC 0.5f
//ConVar gren_water_reduce_think("ffdev_gren_water_reduce_think", "0.2", FCVAR_REPLICATED | FCVAR_CHEAT);
#define GREN_WATER_REDUCE_THINK 0.2f
ConVar gren_teamcolored_trails("ffdev_gren_teamcolored_trails", "0", FCVAR_REPLICATED | FCVAR_CHEAT);

//=============================================================================
// CFFGrenadeBase implementation
//=============================================================================

#ifdef GAME_DLL
	extern short g_sModelIndexFireball;
	extern short g_sModelIndexWExplosion;
#endif

	//-----------------------------------------------------------------------------
	// Purpose: Set all the spawn stuff
	//-----------------------------------------------------------------------------
	void CFFGrenadeBase::Spawn()
	{
		BaseClass::Spawn();

		m_fIsHandheld = true;

#ifdef GAME_DLL

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
		m_bIsOn = false;

		CreateTrail();

		SetThink(&CFFGrenadeBase::GrenadeThink);
		SetNextThink(gpGlobals->curtime);
#endif

#ifdef CLIENT_DLL
		SetNextClientThink(gpGlobals->curtime);
#endif
	}	

#ifdef GAME_DLL
	void CFFGrenadeBase::CreateTrail()
	{
		m_pTrail = CSpriteTrail::SpriteTrailCreate("sprites/ff_trail.vmt", GetLocalOrigin(), false);

		if (!m_pTrail)
			return;

		color32 col = GetColour();

		m_pTrail->FollowEntity(this);
		//m_pTrail->SetAttachment(this, nAttachment);
		m_pTrail->SetTransparency(kRenderTransAdd, col.r, col.g, col.b, col.a, kRenderFxNone);
		m_pTrail->SetStartWidth(10.0f);
		m_pTrail->SetEndWidth(5.0f);
		m_pTrail->SetLifeTime(0.5f);
	}

	//-----------------------------------------------------------------------------
	// Purpose: This'll be called once the grenade is actually thrown
	//-----------------------------------------------------------------------------
	void CFFGrenadeBase::SetDetonateTimerLength(float timer)
	{
		m_flDetonateTime = gpGlobals->curtime + timer;
	}
#endif

	//-----------------------------------------------------------------------------
	// Purpose: If we're trying to detonate, run through Lua first to check allowed
	//-----------------------------------------------------------------------------
	void CFFGrenadeBase::Detonate()
	{
#ifdef GAME_DLL
		// Remove if not allowed by Lua 
 		if (FFScriptRunPredicates(this, "onexplode", true) == false)
		{
			UTIL_Remove(this);
			return;
		}
#endif

		BaseClass::Detonate();
	}

#ifdef GAME_DLL
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
#endif
	
#ifdef CLIENT_DLL
	//-----------------------------------------------------------------------------
	// Purpose: Check for end of fuse, stuff like that
	//-----------------------------------------------------------------------------
	void CFFGrenadeBase::ClientThink()
	{
		if (!IsInWorld())
		{
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

		BaseClass::ClientThink();

		// Next think straight away
		SetNextClientThink(gpGlobals->curtime);
	}
#endif

#ifdef GAME_DLL
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
			CTakeDamageInfo info(this, GetThrower(), 10, DMG_CLUB);
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

		// HL1: float backoff = 2.0 - pmove->friction;
		float backoff = 1.0 + GetElasticity();

		Vector vecAbsVelocity;

		//DevMsg("Pre: %f\n", GetAbsVelocity().z);

		// HL1: PM_ClipVelocity (pmove->velocity, trace.plane.normal, pmove->velocity, backoff);
		PhysicsClipVelocity(GetAbsVelocity(), trace.plane.normal, vecAbsVelocity, backoff);

		if (trace.plane.normal.z > 0.7f)
		{
			vecAbsVelocity *= 1.0f - GetFriction();
		}

		//DevMsg("Post: %f\n", vecAbsVelocity.z);

		/*float flTotalElasticity = GetElasticity() * flSurfaceElasticity;
		flTotalElasticity = clamp(flTotalElasticity, 0.0f, 0.9f);

		// NOTE: A backoff of 2.0f is a reflection
		Vector vecAbsVelocity;
		PhysicsClipVelocity(GetAbsVelocity(), trace.plane.normal, vecAbsVelocity, 2.0f);

		// 0 when hitting walls, 1 when hitting floor
		float flDot = DotProduct(trace.plane.normal, Vector(0.0f, 0.0f, 1.0f));

		const float flWallVerticalElasticity = 1.0f;
		const float flWallHorizontalElasticity = 0.6f;

		// Let's try a smooth scale from from max at vertical (dot:0) -> normal on flat (dot:1)
		float flV = flDot * flTotalElasticity + (1.0f - flDot) * flWallVerticalElasticity;
		float flH = flDot * flTotalElasticity + (1.0f - flDot) * flWallHorizontalElasticity;

		// We'll only change the 
		vecAbsVelocity.x *= flH;
		vecAbsVelocity.y *= flH;
		vecAbsVelocity.z *= flV;*/

		// Get the total velocity (player + conveyors, etc.)
		VectorAdd(vecAbsVelocity, GetBaseVelocity(), vecVelocity);


		// stop if on ground
		if (trace.plane.normal[2] > 0.7)
		{		
			//if (vecVelocity[2] < 800 * gpGlobals->frametime && trace.m_pEnt->IsStandable())
			//{
			//	// we're rolling on the ground, add static friction.
			//	//SetGroundEntity(trace.m_pEnt);	//pmove->onground = trace.ent;
			//	vecVelocity[2] = 0;
			//	SetLocalAngularVelocity(vec3_angle);
			//}

			float speed = DotProduct(vecVelocity, vecVelocity);

			if (speed < (30 * 30))
			{
				SetGroundEntity(trace.m_pEnt);	// pmove->onground = trace.ent;
				SetAbsVelocity(vec3_origin);	// VectorCopy(vec3_origin, pmove->velocity);
				SetLocalAngularVelocity(vec3_angle);
				
			}
			else
			{
				//VectorScale (pmove->velocity, (1.0 - trace.fraction) * pmove->frametime * 0.9, move);
				//trace = PM_PushEntity (move);
				SetAbsVelocity(vecVelocity);
			}
			//VectorSubtract( pmove->velocity, base, pmove->velocity )
		}
		else
		{
			SetAbsVelocity(vecVelocity);
		}

		BounceSound();



/*		float flSpeedSqr = DotProduct(vecVelocity, vecVelocity);

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
		BounceSound();*/
	}
#endif

	//-----------------------------------------------------------------------------
	// Purpose: Added so that grenades aren't using projectiles explode code.
	//			Grenades might need to look in more places than just below
	//			 them to see if scorch marks can be drawn.
	//-----------------------------------------------------------------------------
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
#endif
	}
#ifdef GAME_DLL
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

	void DrawSpriteRotated( const Vector &vecOrigin, float flWidth, float flHeight, color32 color, float rotation );

	ConVar target_clamp_min("ffdev_target_clamp_min", "5.0", FCVAR_CHEAT);
	ConVar target_clamp_max("ffdev_target_clamp_max", "60.0", FCVAR_CHEAT); // 30
	ConVar target_size_base("ffdev_target_size_base", "1.0", FCVAR_CHEAT);	// 15
	ConVar target_size_multiplier("ffdev_target_size_multiplier", "6.0", FCVAR_CHEAT); // 15
	ConVar target_time_remaining("ffdev_target_time_remaining", "3.0", FCVAR_CHEAT);

	ConVar target_speed_max("ffdev_target_speed_max", "100", FCVAR_CHEAT);
	ConVar target_speed_min("ffdev_target_speed_min", "20", FCVAR_CHEAT);

	ConVar target_rotation("ffdev_target_rotation", "-118.2", FCVAR_CHEAT); // -100

	ConVar grenadetargets("cl_grenadetargets", "1", FCVAR_ARCHIVE);
	ConVar grenadedurationtargets("cl_grenadedurationtargets", "0", FCVAR_ARCHIVE | FCVAR_CHEAT);

	int CFFGrenadeBase::DrawModel(int flags)
	{
		int ret = BaseClass::DrawModel(flags);

		if (ret == 0)
			return 0;

		if (!m_bIsOn && grenadetargets.GetBool() == false)
			return ret;

		if (m_bIsOn && grenadedurationtargets.GetBool() == false)
			return ret;

		float flSpeed = GetAbsVelocity().Length();
		
		float speed_max = target_speed_max.GetFloat();
		float speed_min = target_speed_min.GetFloat();

		// Safety check...
		if (speed_max == speed_min)
			speed_max += 0.1f;

		if (flSpeed > speed_max)
			return ret;

		color32 col = GetColour();

		if (m_flModelSize == 0.0f)
		{
			const model_t *pModel = GetModel();

			if (pModel)
			{
				studiohdr_t *pStudio = modelinfo->GetStudiomodel(pModel);

				if (pStudio)
				{
					Vector vecDimensions = pStudio->hull_max - pStudio->hull_min;

					// We could be cunning and project these with our projection matrix
					// in order to be more accurate, but lets try like this first
					m_flModelSize = vecDimensions.Length();
				}
			}
		}

		// Need to scale somewhere between speed_min and speed_max...
		if (flSpeed > speed_min)
		{
			float flScale = (flSpeed - speed_min) / (speed_max - speed_min);
			col.a *= 1.0f - flScale;
		}

		float flRemaining = m_flDetonateTime - gpGlobals->curtime; // DetonateTime is only sent for grenades with non-standard timers e.g. mirvlets
//DevMsg("flRemaining is %4.2f\n", flRemaining);
		if (flRemaining < -5.f) // for grenades that have standard timers, flRemaining will be large and negative
			flRemaining = target_time_remaining.GetFloat() - (gpGlobals->curtime - m_flSpawnTime);
		if (flRemaining < 0.f) // for grenades that have just gone off, but remain in game and need a persistent halo, e.g. hover turret / slow field
			return ret; //flRemaining = 0.01f;

		//if (( Classify() == CLASS_GREN_LASER ) && ( m_flDetonateTime - m_flSpawnTime > 5.f))
		//	return ret;
			
		float flSize = m_flModelSize * target_size_base.GetFloat() + target_size_multiplier.GetFloat() * flRemaining;
		flSize = clamp(flSize, target_clamp_min.GetFloat(), target_clamp_max.GetFloat());

		// The blur graphic now has everything all in one
		// TODO: Stop doing this every frame.
		//IMaterial *pMaterial = materials->FindMaterial("sprites/ff_target", TEXTURE_GROUP_CLIENT_EFFECTS);
		IMaterial *pMaterialBlur = materials->FindMaterial("sprites/ff_target_blur", TEXTURE_GROUP_CLIENT_EFFECTS);

//		float flRotation = gpGlobals->curtime * target_rotation.GetFloat() - anglemod(m_flSpawnTime);
		//float flRotation = anglemod(gpGlobals->curtime  - m_flSpawnTime) * target_rotation.GetFloat();
		float flRotation = anglemod(flRemaining) * target_rotation.GetFloat();

		/*if (pMaterialBlur)
		{
			color32 colblur = col;
			colblur.r *= 0.5f;
			colblur.g *= 0.5f;
			colblur.b *= 0.5f;
			colblur.a *= 0.6f;
			materials->Bind(pMaterialBlur);
			DrawSpriteRotated(GetAbsOrigin(), flSize, flSize, colblur, flRotation);
		}*/

		// Just display the blur material as that has all the stuff in one
		if (pMaterialBlur)
		{
			materials->Bind(pMaterialBlur);
			DrawSpriteRotated(GetAbsOrigin(), flSize, flSize, col, flRotation);
		}

		return ret;
	}
#endif

void CFFGrenadeBase::Precache()
{
	//0000287: SV_StartSound: weapons/debris1.wav not precached (0)
	PrecacheScriptSound("BaseGrenade.Explode");
	PrecacheModel("sprites/ff_trail.vmt");

	BaseClass::Precache();
}

color32 CFFGrenadeBase::GetColour()
{
	if(gren_teamcolored_trails.GetFloat())
	{
		//dexter - dont do this, grenades have a team id or should
		/* 
		CBaseEntity *pEntity = GetThrower();
		if(pEntity && pEntity->IsPlayer())
		{
			CFFPlayer *pPlayer = ToFFPlayer(pEntity);
			int teamID = pPlayer->GetTeamNumber();
		*/
		switch(GetTeamNumber())
		{
		case FF_TEAM_BLUE:
			return GREN_COLOR_BLUE;
			break;
		case FF_TEAM_RED:
			return GREN_COLOR_RED;
			break;
		case FF_TEAM_GREEN:
			return GREN_COLOR_GREEN;
			break;
		case FF_TEAM_YELLOW:
			return GREN_COLOR_YELLOW;
			break;
		default:
			return GREN_COLOR_DEFAULT;
			break;
		}
	}
	else
		return GREN_COLOR_DEFAULT;		
}