/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file f:\Program Files\Steam\SteamApps\SourceMods\FortressForeverCode\game_shared\ff\ff_caltrop.cpp
/// @author Shawn Smith (L0ki)
/// @date Jun. 12, 2005
/// @brief Caltrop class
/// 
/// Declares and implements the caltrop class
/// 
/// Revisions
/// ---------
/// Jun. 12, 2005	L0ki: Initial Creation

#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_player.h"
#include "ff_caltrop.h"

ConVar caltrop_life("ffdev_caltrop_life","5.0",0,"How long caltrops are alive.");
ConVar caltrop_dmg("ffdev_caltrop_dmg","9.0",0,"How much damage a single caltrop deals.");
ConVar caltrop_mode("ffdev_caltrop_mode","0",0,"0 = 'sticky' mode, 1 = 'normal' mode");

LINK_ENTITY_TO_CLASS( caltropgib, CFFCaltropGib );
PRECACHE_WEAPON_REGISTER( caltropgib );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFCaltropGib::CFFCaltropGib( void )
{
	m_flSpawnTime = 0.0f;
	m_iGibModel = 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFCaltropGib::Spawn( void )
{
	m_flSpawnTime = gpGlobals->curtime;
	BaseClass::Spawn();

	if( m_iGibModel == 1 )
		SetModel( CALTROPGRENADE_MODEL_GIB1 );
	else
		SetModel( CALTROPGRENADE_MODEL_GIB2 );

	SetAbsAngles( QAngle( 0, 0, 0 ) );
	SetSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
	SetSolid( SOLID_BBOX );
	SetCollisionGroup( COLLISION_GROUP_DEBRIS );
	SetSize( Vector ( -3, -3, -3 ), Vector ( 3, 3, 3 ) );

	SetThink( &CBaseAnimating::SUB_Remove );
	SetNextThink( gpGlobals->curtime + caltrop_life.GetFloat() );
}

//-----------------------------------------------------------------------------
// Purpose: Precache assets
//-----------------------------------------------------------------------------
void CFFCaltropGib::Precache( void )
{
	if( m_iGibModel == 1 )
		PrecacheModel( CALTROPGRENADE_MODEL_GIB1 );
	else
		PrecacheModel( CALTROPGRENADE_MODEL_GIB2 );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Tell it what to do when it's moving
//-----------------------------------------------------------------------------
void CFFCaltropGib::ResolveFlyCollisionCustom( trace_t& trace, Vector& vecVelocity )
{
	//Assume all surfaces have the same elasticity
	float flSurfaceElasticity = 1.0;

	//Don't bounce off of players with perfect elasticity
	if( trace.m_pEnt && trace.m_pEnt->IsPlayer() )
	{
		flSurfaceElasticity = 0.3;
	}

	// if its breakable glass and we kill it, don't bounce.
	// give some damage to the glass, and if it breaks, pass 
	// through it.
	bool breakthrough = false;

	if( trace.m_pEnt && FClassnameIs( trace.m_pEnt, "func_breakable" ) )
	{
		breakthrough = true;
	}

	if( trace.m_pEnt && FClassnameIs( trace.m_pEnt, "func_breakable_surf" ) )
	{
		breakthrough = true;
	}

	if (breakthrough)
	{
		CTakeDamageInfo info( this, this, 10, DMG_CLUB );
		trace.m_pEnt->DispatchTraceAttack( info, GetAbsVelocity(), &trace );

		ApplyMultiDamage();

		if( trace.m_pEnt->m_iHealth <= 0 )
		{
			// slow our flight a little bit
			Vector vel = GetAbsVelocity();

			vel *= 0.4;

			SetAbsVelocity( vel );
			return;
		}
	}

	float flTotalElasticity = GetElasticity() * flSurfaceElasticity;
	flTotalElasticity = clamp( flTotalElasticity, 0.0f, 0.9f );

	// NOTE: A backoff of 2.0f is a reflection
	Vector vecAbsVelocity;
	PhysicsClipVelocity( GetAbsVelocity(), trace.plane.normal, vecAbsVelocity, 2.0f );
	vecAbsVelocity *= flTotalElasticity;

	// Get the total velocity (player + conveyors, etc.)
	VectorAdd( vecAbsVelocity, GetBaseVelocity(), vecVelocity );
	float flSpeedSqr = DotProduct( vecVelocity, vecVelocity );

	// Stop if on ground.
	if ( trace.plane.normal.z > 0.7f )			// Floor
	{
		// Verify that we have an entity.
		CBaseEntity *pEntity = trace.m_pEnt;
		Assert( pEntity );

		SetAbsVelocity( vecAbsVelocity );

		if ( flSpeedSqr < ( 30 * 30 ) )
		{
			if ( pEntity->IsStandable() )
			{
				SetGroundEntity( pEntity );
			}

			// Reset velocities.
			SetAbsVelocity( vec3_origin );
			SetLocalAngularVelocity( vec3_angle );

			/*//align to the ground so we're not standing on end
			QAngle angle;
			VectorAngles( trace.plane.normal, angle );

			// rotate randomly in yaw
			angle[1] = random->RandomFloat( 0, 360 );

			// TODO: rotate around trace.plane.normal

			SetAbsAngles( angle );	*/		
		}
		else
		{
			Vector vecDelta = GetBaseVelocity() - vecAbsVelocity;	
			Vector vecBaseDir = GetBaseVelocity();
			VectorNormalize( vecBaseDir );
			float flScale = vecDelta.Dot( vecBaseDir );

			VectorScale( vecAbsVelocity, ( 1.0f - trace.fraction ) * gpGlobals->frametime, vecVelocity ); 
			VectorMA( vecVelocity, ( 1.0f - trace.fraction ) * gpGlobals->frametime, GetBaseVelocity() * flScale, vecVelocity );
			PhysicsPushEntity( vecVelocity, &trace );
		}
	}
	else
	{
		// If we get *too* slow, we'll stick without ever coming to rest because
		// we'll get pushed down by gravity faster than we can escape from the wall.
		if ( flSpeedSqr < ( 30 * 30 ) )
		{
			// Reset velocities.
			SetAbsVelocity( vec3_origin );
			SetLocalAngularVelocity( vec3_angle );
		}
		else
		{
			SetAbsVelocity( vecAbsVelocity );
		}
	}
}

BEGIN_DATADESC( CFFCaltrop )
	DEFINE_FUNCTION( CaltropTouch ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( caltrop, CFFCaltrop );
PRECACHE_WEAPON_REGISTER( caltrop );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFCaltrop::CFFCaltrop( void )
{
	m_flSpawnTime = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Precache assets
//-----------------------------------------------------------------------------
void CFFCaltrop::Precache ( void )
{
	DevMsg("[Grenade Debug] CFFCaltrop::Precache\n");
	PrecacheModel( CALTROPGRENADE_MODEL_CALTROP );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Spawn
//-----------------------------------------------------------------------------
void CFFCaltrop::Spawn( void )
{
	DevMsg("[Grenade Debug] CFFCaltrop::Spawn\n");
	m_flSpawnTime = gpGlobals->curtime;
	BaseClass::Spawn();

	SetModel ( CALTROPGRENADE_MODEL_CALTROP );
	SetAbsAngles( QAngle( 0, 0, 0 ) );
	SetSolidFlags( FSOLID_TRIGGER | FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
	SetSolid( SOLID_BBOX );
	SetCollisionGroup( COLLISION_GROUP_DEBRIS );
	SetSize( Vector ( -3, -3, -3 ), Vector ( 3, 3, 3 ) );
	SetTouch( &CFFCaltrop::CaltropTouch );
	SetThink( &CBaseAnimating::SUB_Remove );
	SetNextThink( gpGlobals->curtime + caltrop_life.GetFloat() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFCaltrop::CaltropTouch ( CBaseEntity *pOther )
{
	//DevMsg("[Grenade Debug] CFFCaltrop::CaltropTouch\n");
	/*if(caltrop_mode.GetInt() == 0)
		SetAbsVelocity(Vector(0,0,0));
	else
		SetMoveType( MOVETYPE_FLYGRAVITY );*/
	if ( !pOther->IsPlayer() )
	{
		//DevMsg("[Grenade Debug] entity is not a player, ignoring\n");
		return;
	}

	// #0000623: Priming caltrop grenade and not ever throwing it causes all caltrops to hit yourself
	if (GetGroundEntity() == NULL)
		return;

	CFFPlayer *pPlayer = ToFFPlayer(pOther);
	if( pPlayer && !pPlayer->IsObserver() )
	{
		if( g_pGameRules->FPlayerCanTakeDamage( pPlayer, GetOwnerEntity() ) )
		{
			DevMsg("[Grenade Debug] Damaging player and removing\n");
			CTakeDamageInfo info(this,GetOwnerEntity(),Vector(0,0,0),GetAbsOrigin(),caltrop_dmg.GetFloat(), DMG_GENERIC);
			pPlayer->TakeDamage( info );
			m_takedamage = DAMAGE_NO;
			AddSolidFlags( FSOLID_NOT_SOLID );
			SetMoveType( MOVETYPE_NONE );
			AddEffects( EF_NODRAW );
			SetTouch ( NULL );
			SetThink ( &CBaseAnimating::SUB_Remove );
			SetNextThink( gpGlobals->curtime );

			// make the player walk slow
			pPlayer->AddSpeedEffect(SE_CALTROP, 15.0, 0.8f, SEM_ACCUMULATIVE|SEM_HEALABLE);

			// send them the status icon
			DevMsg("[Grenade Debug] Sending status icon\n");
			CSingleUserRecipientFilter user( (CBasePlayer *)pPlayer );
			user.MakeReliable();
			UserMessageBegin(user, "StatusIconUpdate");
			WRITE_BYTE(FF_ICON_CALTROP);
			WRITE_FLOAT(15.0);
			MessageEnd();
		}		
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFCaltrop::ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity )
{
	//Assume all surfaces have the same elasticity
	float flSurfaceElasticity = 1.0;

	//Don't bounce off of players with perfect elasticity
	if( trace.m_pEnt && trace.m_pEnt->IsPlayer() )
	{
		flSurfaceElasticity = 0.3;
	}

	// if its breakable glass and we kill it, don't bounce.
	// give some damage to the glass, and if it breaks, pass 
	// through it.
	bool breakthrough = false;

	if( trace.m_pEnt && FClassnameIs( trace.m_pEnt, "func_breakable" ) )
	{
		breakthrough = true;
	}

	if( trace.m_pEnt && FClassnameIs( trace.m_pEnt, "func_breakable_surf" ) )
	{
		breakthrough = true;
	}

	if (breakthrough)
	{
		CTakeDamageInfo info( this, this, 10, DMG_CLUB );
		trace.m_pEnt->DispatchTraceAttack( info, GetAbsVelocity(), &trace );

		ApplyMultiDamage();

		if( trace.m_pEnt->m_iHealth <= 0 )
		{
			// slow our flight a little bit
			Vector vel = GetAbsVelocity();

			vel *= 0.4;

			SetAbsVelocity( vel );
			return;
		}
	}

	float flTotalElasticity = GetElasticity() * flSurfaceElasticity;
	flTotalElasticity = clamp( flTotalElasticity, 0.0f, 0.9f );

	// NOTE: A backoff of 2.0f is a reflection
	Vector vecAbsVelocity;
	PhysicsClipVelocity( GetAbsVelocity(), trace.plane.normal, vecAbsVelocity, 2.0f );
	vecAbsVelocity *= flTotalElasticity;

	// Get the total velocity (player + conveyors, etc.)
	VectorAdd( vecAbsVelocity, GetBaseVelocity(), vecVelocity );
	float flSpeedSqr = DotProduct( vecVelocity, vecVelocity );

	// Stop if on ground.
	if ( trace.plane.normal.z > 0.7f )			// Floor
	{
		// Verify that we have an entity.
		CBaseEntity *pEntity = trace.m_pEnt;
		Assert( pEntity );

		SetAbsVelocity( vecAbsVelocity );

		if ( flSpeedSqr < ( 30 * 30 ) )
		{
			if ( pEntity->IsStandable() )
			{
				SetGroundEntity( pEntity );
			}

			// Reset velocities.
			SetAbsVelocity( vec3_origin );
			SetLocalAngularVelocity( vec3_angle );

			/*//align to the ground so we're not standing on end
			QAngle angle;
			VectorAngles( trace.plane.normal, angle );

			// rotate randomly in yaw
			angle[1] = random->RandomFloat( 0, 360 );

			// TODO: rotate around trace.plane.normal

			SetAbsAngles( angle );	*/		
		}
		else
		{
			Vector vecDelta = GetBaseVelocity() - vecAbsVelocity;	
			Vector vecBaseDir = GetBaseVelocity();
			VectorNormalize( vecBaseDir );
			float flScale = vecDelta.Dot( vecBaseDir );

			VectorScale( vecAbsVelocity, ( 1.0f - trace.fraction ) * gpGlobals->frametime, vecVelocity ); 
			VectorMA( vecVelocity, ( 1.0f - trace.fraction ) * gpGlobals->frametime, GetBaseVelocity() * flScale, vecVelocity );
			PhysicsPushEntity( vecVelocity, &trace );
		}
	}
	else
	{
		// If we get *too* slow, we'll stick without ever coming to rest because
		// we'll get pushed down by gravity faster than we can escape from the wall.
		if ( flSpeedSqr < ( 30 * 30 ) )
		{
			// Reset velocities.
			SetAbsVelocity( vec3_origin );
			SetLocalAngularVelocity( vec3_angle );
		}
		else
		{
			SetAbsVelocity( vecAbsVelocity );
		}
	}
}