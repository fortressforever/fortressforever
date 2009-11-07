/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file ff_grenade_armorstrip.cpp
/// @author Shawn Smith (L0ki)
/// @date Apr. 23, 2005
/// @brief armorstrip grenade class
/// 
/// Implementation of the CFFGrenadeArmorstrip class. This is the secondary grenade type for engineer
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
	#include "ff_item_backpack.h"
	#include "ammodef.h"
	#include "beam_flags.h"
	#include "ff_entity_system.h"
	#include "te_effect_dispatch.h"
#endif

extern short g_sModelIndexFireball;
extern short g_sModelIndexWExplosion;

#ifdef CLIENT_DLL
int g_iArmorstripRingTexture = -1;
#endif

#ifdef CLIENT_DLL
	#define CFFGrenadeArmorstrip C_FFGrenadeArmorstrip
#endif

/*
#ifndef CLIENT_DLL
	ConVar armorstrip_framerate("ffdev_armorstrip_framerate","2",FCVAR_CHEAT,"Framerate of the armorstrip explosion");
	ConVar armorstrip_width("ffdev_armorstrip_width","8.0",FCVAR_CHEAT,"width of the armorstrip shockwave");
	ConVar armorstrip_life("ffdev_armorstrip_life","0.3",FCVAR_CHEAT,"life of the armorstrip shockwave");
	ConVar armorstrip_spread("ffdev_armorstrip_spread","0",FCVAR_CHEAT,"spread of the armorstrip shockwave");
	ConVar armorstrip_amplitude("ffdev_armorstrip_amplitude","1",FCVAR_CHEAT,"amplitude of the armorstrip shockwave");
	ConVar armorstrip_speed("ffdev_armorstrip_speed","0",FCVAR_CHEAT,"speed of the armorstrip shockwave");
#endif
*/
	
// bools
ConVar ffdev_armorstrip_damagetype_percent_currarmor("ffdev_armorstrip_damagetype_percent_currarmor", "0", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
ConVar ffdev_armorstrip_damagetype_percent_maxarmor("ffdev_armorstrip_damagetype_percent_maxarmor", "0", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
ConVar ffdev_armorstrip_damagetype_flat("ffdev_armorstrip_damagetype_flat", "1", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
ConVar ffdev_armorstrip_damage_falloff("ffdev_armorstrip_damage_falloff", "1", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
ConVar ffdev_armorstrip_damage_armortype("ffdev_armorstrip_damage_armortype", "1", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define ARMORSTRIP_DAMAGETYPE_PERCENT_CURRARMOR ffdev_armorstrip_damagetype_percent_currarmor.GetBool()
#define ARMORSTRIP_DAMAGETYPE_PERCENT_MAXARMOR ffdev_armorstrip_damagetype_percent_maxarmor.GetBool()
#define ARMORSTRIP_DAMAGETYPE_FLAT ffdev_armorstrip_damagetype_flat.GetBool()
#define ARMORSTRIP_DAMAGE_FALLOFF ffdev_armorstrip_damage_falloff.GetBool()
#define ARMORSTRIP_DAMAGE_ARMORTYPE ffdev_armorstrip_damage_armortype.GetBool()

// values
ConVar ffdev_armorstrip_damage("ffdev_armorstrip_damage", "75", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
ConVar ffdev_armorstrip_damage_falloff_min("ffdev_armorstrip_damage_falloff_min", "25", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
ConVar ffdev_armorstrip_damage_percent("ffdev_armorstrip_damage_percent", ".75", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
ConVar ffdev_armorstrip_damage_falloff_min_percent("ffdev_armorstrip_damage_falloff_min_percent", ".25", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
ConVar ffdev_armorstrip_armortocells_percent("ffdev_armorstrip_armortocells_percent", "1", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
ConVar ffdev_armorstrip_radius("ffdev_armorstrip_radius", "240", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
ConVar ffdev_armorstrip_damage_armortype_percent("ffdev_armorstrip_damage_armortype_percent", ".5", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
ConVar ffdev_armorstrip_takebag_cellpercent("ffdev_armorstrip_takebag_cellpercent", "1", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define ARMORSTRIP_DAMAGE ffdev_armorstrip_damage.GetFloat()
#define ARMORSTRIP_DAMAGE_FALLOFF_MIN ffdev_armorstrip_damage_falloff_min.GetFloat()
#define ARMORSTRIP_DAMAGE_PERCENT ffdev_armorstrip_damage_percent.GetFloat()
#define ARMORSTRIP_DAMAGE_FALLOFF_MIN_PERCENT ffdev_armorstrip_damage_falloff_min_percent.GetFloat()
#define ARMORSTRIP_ARMORTOCELLS_PERCENT ffdev_armorstrip_armortocells_percent.GetFloat()
#define ARMORSTRIP_RADIUS ffdev_armorstrip_radius.GetFloat()
#define ARMORSTRIP_DAMAGE_ARMORTYPE_PERCENT ffdev_armorstrip_damage_armortype_percent.GetFloat()
#define ARMORSTRIP_TAKEBAG_CELLPERCENT ffdev_armorstrip_takebag_cellpercent.GetFloat()

#define ARMORSTRIPGRENADE_MODEL		"models/grenades/emp/emp.mdl"
#define ARMORSTRIP_SOUND			"EmpGrenade.Explode"
#define ARMORSTRIP_EFFECT			"FF_EmpZap"

#ifdef CLIENT_DLL
#define CFFGrenadeArmorstrip C_FFGrenadeArmorstrip
#endif

class CFFGrenadeArmorstrip : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeArmorstrip,CFFGrenadeBase);
	DECLARE_NETWORKCLASS();

	virtual void Precache();
	virtual Class_T Classify( void ) { return CLASS_GREN_ARMORSTRIP; }

	virtual float GetGrenadeRadius() { return ARMORSTRIP_RADIUS; }
	virtual float GetGrenadeDamage() { return 0.0f; }
	virtual const char *GetBounceSound() { return "EmpGrenade.Bounce"; }

	virtual color32 GetColour() { color32 col = { 225, 225, 0, GREN_ALPHA_DEFAULT }; return col; }

#ifdef CLIENT_DLL
	CFFGrenadeArmorstrip() {}
	CFFGrenadeArmorstrip( const CFFGrenadeArmorstrip& ) {}
#else
	virtual void Spawn();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);
	void SetWarned( void ) { m_bWarned = true; }

	void GrenadeThink( void );
	bool m_bWarned;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeArmorstrip, DT_FFGrenadeArmorstrip)

BEGIN_NETWORK_TABLE(CFFGrenadeArmorstrip, DT_FFGrenadeArmorstrip)
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( ff_grenade_armorstrip, CFFGrenadeArmorstrip );
PRECACHE_WEAPON_REGISTER( ff_grenade_armorstrip );

#ifdef GAME_DLL

	//-----------------------------------------------------------------------------
	// Purpose: Various flags, models
	//-----------------------------------------------------------------------------
	void CFFGrenadeArmorstrip::Spawn( void )
	{
		SetModel( ARMORSTRIPGRENADE_MODEL );
		m_bWarned = false;
		BaseClass::Spawn();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Emit a ring that blows things up
	//-----------------------------------------------------------------------------
	void CFFGrenadeArmorstrip::Explode(trace_t *pTrace, int bitsDamageType)
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

		//DispatchEffect(ARMORSTRIP_EFFECT, data);
		g_pEffects->Sparks(GetAbsOrigin(), 20, 10, &vecUp);

		float radius = GetGrenadeRadius();

		CFFPlayer *pOwner = ToFFPlayer( GetOwnerEntity() );

		CBaseEntity *pEntity = NULL;
		for ( CEntitySphereQuery sphere( GetAbsOrigin(), radius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			if (!pEntity)
				continue;

			// strip armor from players
			if( pEntity->IsPlayer() )
			{
				CFFPlayer *pPlayer = ToFFPlayer(pEntity);

				if( !pPlayer->IsAlive() || pPlayer->IsObserver() )
					continue;

				if( g_pGameRules->FCanTakeDamage( pPlayer, pOwner ) )
				{
					float armordamage = 0;

					if (ARMORSTRIP_DAMAGETYPE_PERCENT_MAXARMOR)
					{
						armordamage = pPlayer->GetMaxArmor() * ARMORSTRIP_DAMAGE_PERCENT;
					}
					else if (ARMORSTRIP_DAMAGETYPE_PERCENT_CURRARMOR)
					{
						armordamage = pPlayer->GetArmor() * ARMORSTRIP_DAMAGE_PERCENT;
					}
					else if (ARMORSTRIP_DAMAGETYPE_FLAT)
					{
						armordamage = ARMORSTRIP_DAMAGE;
					}
					
					if (ARMORSTRIP_DAMAGE_ARMORTYPE)
					{
						armordamage *= (1.0f - ARMORSTRIP_DAMAGE_ARMORTYPE_PERCENT * pPlayer->GetArmorType());
					}
					if (ARMORSTRIP_DAMAGE_FALLOFF)
					{
						// Some useful things to know
						Vector vecDisplacement = pPlayer->GetLegacyAbsOrigin() - GetAbsOrigin();
						float flDistance = vecDisplacement.Length();
						Vector vecDir = vecDisplacement / flDistance;

						armordamage *= 1.0f - (flDistance > 16.0f ? flDistance : 0.0f) / radius;
						if (ARMORSTRIP_DAMAGETYPE_PERCENT_MAXARMOR || ARMORSTRIP_DAMAGETYPE_PERCENT_CURRARMOR)
							armordamage = max( armordamage, ARMORSTRIP_DAMAGE_FALLOFF_MIN_PERCENT );
						else if (ARMORSTRIP_DAMAGETYPE_FLAT)
							armordamage = max( armordamage, ARMORSTRIP_DAMAGE_FALLOFF_MIN );
					}

					int armortaken = pPlayer->RemoveArmor( (int)armordamage );

					if (ARMORSTRIP_ARMORTOCELLS_PERCENT && armortaken)
					{
						pOwner->GiveAmmo( (int)(ARMORSTRIP_ARMORTOCELLS_PERCENT * armortaken), AMMO_CELLS );
					}
					
					if (armortaken)
					{
						g_pEffects->Sparks(pPlayer->GetLegacyAbsOrigin(), 10, 5, &vecUp);
					}
				}
			}
			// get cells from bags
			else
			{
				switch( pEntity->Classify() )
				{
				case CLASS_BACKPACK:
					CFFItemBackpack *pBackpack = dynamic_cast< CFFItemBackpack* > (pEntity);

					if ( pBackpack && pBackpack->GetSpawnFlags() & SF_NORESPAWN )
					{
						if (ARMORSTRIP_TAKEBAG_CELLPERCENT > 0)
						{
							int givecells = (int)(ARMORSTRIP_TAKEBAG_CELLPERCENT * pBackpack->GetAmmoCount( GetAmmoDef()->Index( AMMO_CELLS ) ));
							pOwner->GiveAmmo( givecells, AMMO_CELLS );

							EmitSound( "Armorstrip.Takebag" );
							g_pEffects->Sparks(pBackpack->GetAbsOrigin(), 5, 1, &vecUp);
							
							UTIL_Remove( pBackpack );
						}
					}
					break;
				}
			}
		}

		UTIL_Remove(this);
	}

	//----------------------------------------------------------------------------
	// Purpose: Fire explosion sound early
	//----------------------------------------------------------------------------
	void CFFGrenadeArmorstrip::GrenadeThink( void )
	{
		BaseClass::GrenadeThink();

		if (!m_bWarned && gpGlobals->curtime > m_flDetonateTime - 0.685f)
		{
			m_bWarned = true;

			// If the grenade is in a no gren area don't do explode sound
			if( FFScriptRunPredicates( this, "onexplode", true ) )
			{
				EmitSound(ARMORSTRIP_SOUND);
			}
		}
	}
#endif

//----------------------------------------------------------------------------
// Purpose: Precache the model ready for spawn
//----------------------------------------------------------------------------
void CFFGrenadeArmorstrip::Precache()
{
	PrecacheModel( ARMORSTRIPGRENADE_MODEL );
	PrecacheScriptSound( ARMORSTRIP_SOUND );

#ifdef CLIENT_DLL
	g_iArmorstripRingTexture = PrecacheModel("sprites/lgtning.vmt");
#endif

	BaseClass::Precache();
}