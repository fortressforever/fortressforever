/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file ff_grenade_emp.cpp
/// @author Shawn Smith (L0ki)
/// @date Apr. 23, 2005
/// @brief emp grenade class
/// 
/// Implementation of the CFFGrenadeEmp class. This is the secondary grenade type for engineer
/// 
/// Revisions
/// ---------
/// Apr. 23, 2005	L0ki: Initial creation

#include "cbase.h"
#include "ff_grenade_emp.h"
#include "ff_utils.h"
#include "engine/IEngineSound.h"
#include "IEffects.h"

#ifdef GAME_DLL
	#include "ff_buildableobjects_shared.h"
	#include "ff_projectile_pipebomb.h"
	#include "baseentity.h"
	#include "beam_flags.h"
	#include "ff_entity_system.h"
	#include "te_effect_dispatch.h"
#endif

extern short g_sModelIndexFireball;
extern short g_sModelIndexWExplosion;

#ifdef CLIENT_DLL
int g_iEmpRingTexture = -1;
#endif

#ifdef CLIENT_DLL
	#define CFFGrenadeEmp C_FFGrenadeEmp
#endif

#ifndef CLIENT_DLL
	ConVar emp_framerate("ffdev_emp_framerate","2",FCVAR_FF_FFDEV_CLIENT,"Framerate of the emp explosion");
	ConVar emp_width("ffdev_emp_width","8.0",FCVAR_FF_FFDEV_CLIENT,"width of the emp shockwave");
	ConVar emp_life("ffdev_emp_life","0.3",FCVAR_FF_FFDEV_CLIENT,"life of the emp shockwave");
	ConVar emp_spread("ffdev_emp_spread","0",FCVAR_FF_FFDEV_CLIENT,"spread of the emp shockwave");
	ConVar emp_amplitude("ffdev_emp_amplitude","1",FCVAR_FF_FFDEV_CLIENT,"amplitude of the emp shockwave");
	ConVar emp_speed("ffdev_emp_speed","0",FCVAR_FF_FFDEV_CLIENT,"speed of the emp shockwave");
	ConVar emp_buildable_damage("ffdev_emp_buildable_damage","10.0",FCVAR_FF_FFDEV_CLIENT,"Amount of damage to deal to sentryguns and dispensers in the emp radius.");
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeEmp, DT_FFGrenadeEmp)

BEGIN_NETWORK_TABLE(CFFGrenadeEmp, DT_FFGrenadeEmp)
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( ff_grenade_emp, CFFGrenadeEmp );
PRECACHE_WEAPON_REGISTER( ff_grenade_emp );

#ifdef GAME_DLL

	//-----------------------------------------------------------------------------
	// Purpose: Various flags, models
	//-----------------------------------------------------------------------------
	void CFFGrenadeEmp::Spawn( void )
	{
		SetModel( EMPGRENADE_MODEL );
		m_bWarned = false;
		BaseClass::Spawn();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Emit a ring that blows things up
	//-----------------------------------------------------------------------------
	void CFFGrenadeEmp::Explode(trace_t *pTrace, int bitsDamageType)
	{
		// Don't explode if in no gren area
		if( !FFScriptRunPredicates( this, "onexplode", true ) )
		{
			UTIL_Remove( this );
			return;
		}

		CEffectData data;
		data.m_vOrigin = GetAbsOrigin();
		data.m_flScale = 1.0f;

		Vector vecUp(0, 0, 1.0f);

		DispatchEffect(EMP_EFFECT, data);
		g_pEffects->Sparks(GetAbsOrigin(), 20, 10, &vecUp);

		float radius = GetGrenadeRadius();

		CBaseEntity *pEntity = NULL;
		for ( CEntitySphereQuery sphere( GetAbsOrigin(), radius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			// Don't care about ourselves
			if( pEntity == this )
				continue;

			if( int explode = pEntity->TakeEmp() )
			{
				switch( pEntity->Classify() )
				{
				case CLASS_PIPEBOMB:

					// This will handle the pipes blowing up and setting
					// the correct owner
					((CFFProjectilePipebomb *)pEntity)->DecrementHUDCount();
					((CFFProjectilePipebomb *)pEntity)->DetonatePipe(true, GetOwnerEntity());
					break;

				default:
					// For all other projectiles or objects that return
					// something from TakeEmp we gotta add the explosions
					// ourselves

					trace_t		tr;						
					Vector		vecOrigin = pEntity->GetAbsOrigin();

					// Traceline to check if we should do scorch marks on the floor						
					UTIL_TraceLine( vecOrigin + Vector( 0, 0, 2.0f ), vecOrigin - Vector( 0, 0, FF_DECALTRACE_TRACE_DIST ), MASK_SHOT_HULL, pEntity, COLLISION_GROUP_NONE, &tr);

					// Explode now
					if( tr.fraction != 1.0 )
					{
						Vector vecNormal = tr.plane.normal;
						surfacedata_t *pdata = physprops->GetSurfaceData( tr.surface.surfaceProps );	
						CPASFilter filter( vecOrigin );

						te->Explosion( filter, -1.0, // don't apply cl_interp delay
							&vecOrigin,
							!pEntity->GetWaterLevel() ? g_sModelIndexFireball : g_sModelIndexWExplosion,
							m_DmgRadius * .03, 
							25,
							TE_EXPLFLAG_NONE,
							m_DmgRadius,
							m_flDamage,
							&vecNormal,
							( char )pdata->game.material );

						// Normal decals since trace hit something
						UTIL_DecalTrace( &tr, "Scorch" );
					}
					else
					{
						CPASFilter filter( vecOrigin );

						te->Explosion( filter, -1.0, // don't apply cl_interp delay
							&vecOrigin, 
							!pEntity->GetWaterLevel() != 0 ? g_sModelIndexFireball : g_sModelIndexWExplosion,
							m_DmgRadius * .03, 
							25,
							TE_EXPLFLAG_NONE,
							m_DmgRadius,
							m_flDamage );

						// Trace hit nothing so do custom scorch mark finding
						FF_DecalTrace( pEntity, FF_DECALTRACE_TRACE_DIST, "Scorch" );
					}

					CTakeDamageInfo info( this, GetOwnerEntity(), GetBlastForce(), pEntity->GetAbsOrigin(), explode, DMG_SHOCK, 0, &vecOrigin );
					RadiusDamage( info, pEntity->GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );
						
					EmitSound( "BaseGrenade.Explode" );

					UTIL_ScreenShake( pEntity->GetAbsOrigin(), explode, 150.0, 1.0, radius, SHAKE_START );

					break;
				}
			}
		}

		UTIL_Remove(this);
	}

	//----------------------------------------------------------------------------
	// Purpose: Fire explosion sound early
	//----------------------------------------------------------------------------
	void CFFGrenadeEmp::GrenadeThink( void )
	{
		BaseClass::GrenadeThink();

		if (!m_bWarned && gpGlobals->curtime > m_flDetonateTime - 0.685f)
		{
			m_bWarned = true;

			// If the grenade is in a no gren area don't do explode sound
			if( FFScriptRunPredicates( this, "onexplode", true ) )
			{
				EmitSound(EMP_SOUND);
			}
		}
	}
#endif

//----------------------------------------------------------------------------
// Purpose: Precache the model ready for spawn
//----------------------------------------------------------------------------
void CFFGrenadeEmp::Precache()
{
	PrecacheModel( EMPGRENADE_MODEL );
	PrecacheScriptSound( EMP_SOUND );

#ifdef CLIENT_DLL
	g_iEmpRingTexture = PrecacheModel("sprites/lgtning.vmt");
#endif

	BaseClass::Precache();
}