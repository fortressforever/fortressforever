/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========

#include "cbase.h"
#include "ff_grenade_shockemp.h"
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
	#define CFFGrenadeShockEmp C_FFGrenadeShockEmp
#endif

#ifndef CLIENT_DLL
	ConVar shockemp_framerate("ffdev_eshockmp_framerate","2",FCVAR_CHEAT,"Framerate of the shockemp explosion");
	ConVar shockemp_width("ffdev_shockemp_width","8.0",FCVAR_CHEAT,"width of the shockemp shockwave");
	ConVar shockemp_life("ffdev_shockemp_life","0.3",FCVAR_CHEAT,"life of the shockemp shockwave");
	ConVar shockemp_spread("ffdev_shockemp_spread","0",FCVAR_CHEAT,"spread of the shockemp shockwave");
	ConVar shockemp_amplitude("ffdev_shockemp_amplitude","1",FCVAR_CHEAT,"amplitude of the shockemp shockwave");
	ConVar shockemp_speed("ffdev_shockemp_speed","0",FCVAR_CHEAT,"speed of the shockemp shockwave");
	ConVar shockemp_damage("ffdev_shockemp_damage","90.0",FCVAR_CHEAT,"Amount of damage dealt to anyone right in the center of the blast");

	ConVar ffdev_shockemp_removepipesquietly("ffdev_shockemp_removepipesquietly","1",FCVAR_CHEAT,"1 = shockEMPing pipes removes them without detonation");
	#define FFDEV_SHOCKEMP_REMOVEPIPESQUIETLY ffdev_shockemp_removepipesquietly.GetBool()

#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeShockEmp, DT_FFGrenadeShockEmp)

BEGIN_NETWORK_TABLE(CFFGrenadeShockEmp, DT_FFGrenadeShockEmp)
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( ff_grenade_shockemp, CFFGrenadeShockEmp );
PRECACHE_WEAPON_REGISTER( ff_grenade_shockemp );

#ifdef GAME_DLL

	//-----------------------------------------------------------------------------
	// Purpose: Various flags, models
	//-----------------------------------------------------------------------------
	void CFFGrenadeShockEmp::Spawn( void )
	{
		SetModel( SHOCKEMPGRENADE_MODEL );
		m_bWarned = false;
		BaseClass::Spawn();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Emit a ring that blows things up
	//-----------------------------------------------------------------------------
	void CFFGrenadeShockEmp::Explode(trace_t *pTrace, int bitsDamageType)
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

		DispatchEffect(SHOCKEMP_EFFECT, data);
		g_pEffects->Sparks(GetAbsOrigin(), 20, 10, &vecUp);

		// Do small bonus damage in center
		Vector vecReported = pTrace->endpos;
		CTakeDamageInfo info( this, GetThrower(), GetBlastForce(), GetAbsOrigin(), shockemp_damage.GetFloat(), bitsDamageType, 0, &vecReported );
		RadiusDamage( info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );

		float radius = GetGrenadeRadius();

		CBaseEntity *pEntity = NULL;
		for ( CEntitySphereQuery sphere( GetAbsOrigin(), radius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			if( pEntity == this )
				continue;

			// It's a bit weird that we've split the code so that some EMP effects are done in the target class and some are done right here. 
			// We should probably agree to either have the effects in teh target code or in the source code (here) and stick with it.. - AfterShock

			switch( pEntity->Classify() )
			{
				case CLASS_PIPEBOMB:

					((CFFProjectilePipebomb *)pEntity)->DecrementHUDCount();

					if ( FFDEV_SHOCKEMP_REMOVEPIPESQUIETLY == 1)
					{
						Vector vecUp(0, 0, 1.0f);
						g_pEffects->Sparks( pEntity->GetAbsOrigin(), 2, 5, &vecUp );
						EmitSound( "DoSpark" );
		
						UTIL_Remove(pEntity);
					}
					else
					{
						((CFFProjectilePipebomb *)pEntity)->DetonatePipe(true, GetOwnerEntity());
					}
					break;

				case CLASS_SENTRYGUN:
					// disable
					// Damage?
					break;
			}
		}

		UTIL_Remove(this);
	}

	//----------------------------------------------------------------------------
	// Purpose: Fire explosion sound early
	//----------------------------------------------------------------------------
	void CFFGrenadeShockEmp::GrenadeThink( void )
	{
		BaseClass::GrenadeThink();

		if (!m_bWarned && gpGlobals->curtime > m_flDetonateTime - 0.685f)
		{
			m_bWarned = true;

			// If the grenade is in a no gren area don't do explode sound
			if( FFScriptRunPredicates( this, "onexplode", true ) )
			{
				EmitSound(SHOCKEMP_SOUND);
			}
		}
	}
#endif

//----------------------------------------------------------------------------
// Purpose: Precache the model ready for spawn
//----------------------------------------------------------------------------
void CFFGrenadeShockEmp::Precache()
{
	PrecacheModel( SHOCKEMPGRENADE_MODEL );
	PrecacheScriptSound( SHOCKEMP_SOUND );

	BaseClass::Precache();
}