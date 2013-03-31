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
	ConVar ffdev_shockemp_damage("ffdev_shockemp_damage","90.0",FCVAR_CHEAT,"Amount of damage dealt to anyone right in the center of the blast");
	#define FFDEV_SHOCKEMP_DAMAGE ffdev_shockemp_damage.GetFloat()
	ConVar ffdev_shockemp_removepipesquietly("ffdev_shockemp_removepipesquietly","1",FCVAR_CHEAT,"1 = shockEMPing pipes removes them without detonation");
	#define FFDEV_SHOCKEMP_REMOVEPIPESQUIETLY ffdev_shockemp_removepipesquietly.GetBool()

	ConVar ffdev_disable_duration_max("ffdev_disable_duration_max", "6", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
	#define FFDEV_DISABLE_DURATION_MAX ffdev_disable_duration_max.GetFloat()
	ConVar ffdev_disable_duration_min("ffdev_disable_duration_min", "4", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
	#define FFDEV_DISABLE_DURATION_MIN ffdev_disable_duration_min.GetFloat()
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
		CTakeDamageInfo info( this, GetThrower(), GetBlastForce(), GetAbsOrigin(), FFDEV_SHOCKEMP_DAMAGE, bitsDamageType, 0, &vecReported );
		RadiusDamage( info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );

		float radius = GetGrenadeRadius();

		CBaseEntity *pEntity = NULL;
		for ( CEntitySphereQuery sphere( GetAbsOrigin(), radius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			if( pEntity == this )
				continue;

			switch( pEntity->Classify() )
			{
				case CLASS_PIPEBOMB:
					{
						CFFProjectilePipebomb *pPipebomb = (CFFProjectilePipebomb *) pEntity;
						bool bFriendly = ( g_pGameRules->PlayerRelationship( GetThrower(), ToFFPlayer( pPipebomb->GetOwnerEntity() ) ) == GR_TEAMMATE );
						if (bFriendly == true)
							continue;

						pPipebomb->DecrementHUDCount();

						if ( FFDEV_SHOCKEMP_REMOVEPIPESQUIETLY == 1)
						{
							Vector vecUp(0, 0, 1.0f);
							g_pEffects->Sparks( pPipebomb->GetAbsOrigin(), 2, 5, &vecUp );
							EmitSound( "DoSpark" );
			
							UTIL_Remove(pPipebomb);
						}
						else
						{
							pPipebomb->DetonatePipe(true, GetOwnerEntity());
						}
					}
					break;

				case CLASS_SENTRYGUN:
					{
						CFFSentryGun *pSentryGun = (CFFSentryGun *) pEntity;
						bool bFriendly = ( g_pGameRules->PlayerRelationship( GetThrower(), ToFFPlayer( pSentryGun->m_hOwner.Get() ) ) == GR_TEAMMATE );
						if (bFriendly == true)
							continue;

						Vector distanceFromGrenade = GetAbsOrigin() - pSentryGun->GetAbsOrigin();
						float linearDistanceFromGrenade = distanceFromGrenade.Length();
						float flPercentPower = (radius - linearDistanceFromGrenade)/radius;
						float disableTime = FFDEV_DISABLE_DURATION_MIN + (FFDEV_DISABLE_DURATION_MAX - FFDEV_DISABLE_DURATION_MIN) * flPercentPower;
						pSentryGun->Disable(disableTime);
					}
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