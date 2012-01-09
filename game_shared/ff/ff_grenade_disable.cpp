/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file ff_grenade_disable.cpp
/// @author Shawn Smith (L0ki)
/// @date Apr. 23, 2005
/// @brief disable grenade class
/// 
/// Implementation of the CFFGrenadeDisable class. This is the secondary grenade type for engineer
/// 
/// Revisions
/// ---------
/// Apr. 23, 2005	L0ki: Initial creation

#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"
#include "engine/IEngineSound.h"
#include "IEffects.h"

#ifdef GAME_DLL
	#include "ff_buildableobjects_shared.h"
	#include "ff_projectile_pipebomb.h"
	#include "baseentity.h"
	#include "ff_player.h"
	#include "ammodef.h"
	#include "beam_flags.h"
	#include "ff_entity_system.h"
	#include "te_effect_dispatch.h"
#endif

extern short g_sModelIndexFireball;
extern short g_sModelIndexWExplosion;

#ifdef CLIENT_DLL
int g_iDisableRingTexture = -1;
#endif

#ifdef CLIENT_DLL
	#define CFFGrenadeDisable C_FFGrenadeDisable
#endif

/*
#ifndef CLIENT_DLL
	ConVar disable_framerate("ffdev_disable_framerate","2",FCVAR_CHEAT,"Framerate of the disable explosion");
	ConVar disable_width("ffdev_disable_width","8.0",FCVAR_CHEAT,"width of the disable shockwave");
	ConVar disable_life("ffdev_disable_life","0.3",FCVAR_CHEAT,"life of the disable shockwave");
	ConVar disable_spread("ffdev_disable_spread","0",FCVAR_CHEAT,"spread of the disable shockwave");
	ConVar disable_amplitude("ffdev_disable_amplitude","1",FCVAR_CHEAT,"amplitude of the disable shockwave");
	ConVar disable_speed("ffdev_disable_speed","0",FCVAR_CHEAT,"speed of the disable shockwave");
#endif
*/

// values
ConVar ffdev_disable_duration("ffdev_disable_duration", "4", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define DISABLE_DURATION ffdev_disable_duration.GetFloat()

ConVar ffdev_disable_radius("ffdev_disable_radius", "240", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define DISABLE_RADIUS ffdev_disable_radius.GetFloat()

#define DISABLEGRENADE_MODEL		"models/grenades/gas/gas.mdl"
#define DISABLE_SOUND			"EmpGrenade.Explode"
#define DISABLE_EFFECT			"FF_EmpZap"

#ifdef CLIENT_DLL
#define CFFGrenadeDisable C_FFGrenadeDisable
#endif

class CFFGrenadeDisable : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeDisable,CFFGrenadeBase);
	DECLARE_NETWORKCLASS();

	virtual void Precache();
	virtual Class_T Classify( void ) { return CLASS_GREN_DISABLE; }

	virtual float GetGrenadeRadius() { return DISABLE_RADIUS; }
	virtual float GetGrenadeDamage() { return 0.0f; }
	virtual const char *GetBounceSound() { return "GasGrenade.Bounce"; }

	virtual color32 GetColour() { color32 col = { 20, 168, 20, GREN_ALPHA_DEFAULT }; return col; }

#ifdef CLIENT_DLL
	CFFGrenadeDisable() {}
	CFFGrenadeDisable( const CFFGrenadeDisable& ) {}
#else
	virtual void Spawn();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);
	void SetWarned( void ) { m_bWarned = true; }

	void GrenadeThink( void );
	bool m_bWarned;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeDisable, DT_FFGrenadeDisable)

BEGIN_NETWORK_TABLE(CFFGrenadeDisable, DT_FFGrenadeDisable)
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( ff_grenade_disable, CFFGrenadeDisable );
PRECACHE_WEAPON_REGISTER( ff_grenade_disable );

#ifdef GAME_DLL

	//-----------------------------------------------------------------------------
	// Purpose: Various flags, models
	//-----------------------------------------------------------------------------
	void CFFGrenadeDisable::Spawn( void )
	{
		SetModel( DISABLEGRENADE_MODEL );
		m_bWarned = false;
		BaseClass::Spawn();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Emit a ring that blows things up
	//-----------------------------------------------------------------------------
	void CFFGrenadeDisable::Explode(trace_t *pTrace, int bitsDamageType)
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

		DispatchEffect(DISABLE_EFFECT, data);
		//g_pEffects->Sparks(GetAbsOrigin(), 20, 10, &vecUp);

		float radius = GetGrenadeRadius();

		CFFPlayer *pOwner = ToFFPlayer( GetOwnerEntity() );

		CBaseEntity *pEntity = NULL;
		for ( CEntitySphereQuery sphere( GetAbsOrigin(), radius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			if (!pEntity)
				continue;

			// disable buildables that can be disabled
			if( FF_IsBuildableObject( pEntity ) )
			{
				CFFBuildableObject *pBuildable = FF_ToBuildableObject( pEntity );

				if( !pBuildable->CanDisable() )
					continue;

				if( g_pGameRules->FCanTakeDamage( pBuildable, pOwner ) )
				{
					pBuildable->Disable( DISABLE_DURATION );
				}
			}
		}

		UTIL_Remove(this);
	}

	//----------------------------------------------------------------------------
	// Purpose: Fire explosion sound early
	//----------------------------------------------------------------------------
	void CFFGrenadeDisable::GrenadeThink( void )
	{
		BaseClass::GrenadeThink();

		if (!m_bWarned && gpGlobals->curtime > m_flDetonateTime - 0.685f)
		{
			m_bWarned = true;

			// If the grenade is in a no gren area don't do explode sound
			if( FFScriptRunPredicates( this, "onexplode", true ) )
			{
				EmitSound(DISABLE_SOUND);
			}
		}
	}
#endif

//----------------------------------------------------------------------------
// Purpose: Precache the model ready for spawn
//----------------------------------------------------------------------------
void CFFGrenadeDisable::Precache()
{
	PrecacheModel( DISABLEGRENADE_MODEL );
	PrecacheScriptSound( DISABLE_SOUND );

#ifdef CLIENT_DLL
	g_iDisableRingTexture = PrecacheModel("sprites/lgtning.vmt");
#endif

	BaseClass::Precache();
}