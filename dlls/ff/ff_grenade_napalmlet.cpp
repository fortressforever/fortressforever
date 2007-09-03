

#include "cbase.h"
#include "ff_grenade_base.h"

#include "ff_grenade_napalmlet.h"
#include "ff_utils.h"

#ifndef CLIENT_DLL
	#include "ff_player.h"
#endif

ConVar burn_standon_ng("ffdev_burn_standon_ng", "7.0", 0, "Damage you take when standing on a burning napalmlet");


BEGIN_DATADESC( CFFGrenadeNapalmlet )
	DEFINE_THINKFUNC( FlameThink ),
END_DATADESC()


LINK_ENTITY_TO_CLASS( ff_grenade_napalmlet, CFFGrenadeNapalmlet );
PRECACHE_WEAPON_REGISTER( ff_grenade_napalmlet );


void CFFGrenadeNapalmlet::UpdateOnRemove( void )
{
	StopSound( "General.BurningFlesh" );
	StopSound( "General.BurningObject" );
	//EmitSound( "General.StopBurning" );

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
//CFFGrenadeNapalmlet::CFFGrenadeNapalmlet( void ){}

//-----------------------------------------------------------------------------
// Purpose: Precache assets
//-----------------------------------------------------------------------------
void CFFGrenadeNapalmlet::Precache ( void )
{
	PrecacheModel( NAPALMLET_MODEL );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Spawn
//-----------------------------------------------------------------------------
void CFFGrenadeNapalmlet::Spawn( void )
{
	//m_flSpawnTime = gpGlobals->curtime;
	BaseClass::Spawn();

	SetModel ( NAPALMLET_MODEL );
	SetAbsAngles( QAngle( 0, 0, 0 ) );
	SetSolidFlags( FSOLID_TRIGGER | FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
	SetSolid( SOLID_BBOX );
	SetCollisionGroup( COLLISION_GROUP_DEBRIS );
	SetSize( Vector ( -5, -5, -5 ), Vector ( 5, 5, 5 ) );
	SetThink( &CFFGrenadeNapalmlet::FlameThink );
	SetNextThink( gpGlobals->curtime );
	SetEffects(EF_NOSHADOW);

	m_pFlame = CEntityFlame::Create( this, false );
	if (m_pFlame)
	{
		m_pFlame->SetLifetime( m_flBurnTime );
		AddFlag( FL_ONFIRE );
		SetEffectEntity( m_pFlame );
		m_pFlame->SetSize( 50.0f );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFGrenadeNapalmlet::ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity )
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

//-----------------------------------------------------------------------------
// Purpose: Burninate the players
//-----------------------------------------------------------------------------
void CFFGrenadeNapalmlet::FlameThink()
	{
		// Remove if we've reached the end of our fuse
		if( gpGlobals->curtime > m_flBurnTime )
		{
			UTIL_Remove(this);
			return;
		}

		// Bug #0001664: Pyro napalm flames in water shouldnt exist
		// Ideally add some fancy smouldering effect for when they are extinguished, but this will do for now |---> Defrag		
		if( GetWaterLevel() != 0  )
		{
			UTIL_Remove(this);
			return;
		}

		// Jiggles: We stopped moving; let's switch to a standing flame
		// Nevermind, this doesn't work.
		//if( m_bFlameSwitch && GetAbsVelocity() == vec3_origin )
		//{
		//	if (m_pFlame)
		//		m_pFlame->SetUseHitboxes(true);
		//	m_bFlameSwitch = false;
		//}


		//SetAbsVelocity( Vector( 0, 0, 10 + 20 * sin( DEG2RAD( GetAbsAngles().y ) ) ) );
		//SetAbsAngles( GetAbsAngles() + QAngle( 0, 15, 0 ) );

		//Vector vecForward;
		//AngleVectors( GetAbsAngles(), &vecForward );

		if((gpGlobals->curtime - m_flLastBurnCheck) >= 1.0f)
		{
			m_flLastBurnCheck = gpGlobals->curtime;

			Vector	vecSrc = GetAbsOrigin();
			vecSrc.z += 1;

			CBaseEntity *pEntity = NULL;

			for( CEntitySphereQuery sphere( vecSrc, m_flBurnRadius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
			{
				if( !pEntity )
					continue;

				// Bug #0000269: Napalm through walls.
				// Mulch: if we hit water w/ the trace, abort too!
				trace_t tr;
				UTIL_TraceLine(GetAbsOrigin(), pEntity->GetAbsOrigin(), MASK_SOLID_BRUSHONLY | CONTENTS_WATER, this, COLLISION_GROUP_DEBRIS, &tr);

				if (tr.fraction < 1.0f)
					continue;

				// Bug #0000270: Napalm grenade burn radius reaches unrealisticly high.
				float height = tr.startpos.z - tr.endpos.z;
				if (height < -40.0f || height > 40.0f)
					continue;

				// Don't damage if entity is more than feet deep in water
				if( pEntity->GetWaterLevel() >= 2 )
					continue;

				switch( pEntity->Classify() )
				{
					case CLASS_PLAYER:
					{
						CFFPlayer *pPlayer = ToFFPlayer( pEntity );
						if( !pPlayer )
							continue;

						if (g_pGameRules->FCanTakeDamage(pPlayer, GetOwnerEntity()))
						{
							pPlayer->TakeDamage( CTakeDamageInfo( this, GetOwnerEntity(), burn_standon_ng.GetInt(), DMG_BURN ) );
							pPlayer->ApplyBurning( ToFFPlayer( GetOwnerEntity() ), 1.0f, 10.0f, BURNTYPE_NALPALMGRENADE);
						}
					}
					break;
					case CLASS_SENTRYGUN:
					case CLASS_DISPENSER:
					{
						// don't have to bother casting this here anymore, just pass the buildable and the FCanTakeDamage function will sort it

						/*
						CFFBuildableObject *pBuildable = dynamic_cast< CFFBuildableObject * >( pEntity );
						if( !pBuildable )
							continue;
						*/
						
						//CFFPlayer *pPlayer = pBuildable->GetOwnerPlayer();
						//if( !pPlayer )
						//	continue;

						if (g_pGameRules->FCanTakeDamage( pEntity, GetOwnerEntity()))
							pEntity->TakeDamage( CTakeDamageInfo( this, GetOwnerEntity(), 8.0f, DMG_BURN ) );
					}
					
					default:
						break;
				}
			}
		}

		SetNextThink( gpGlobals->curtime + 1.0f );
	}

