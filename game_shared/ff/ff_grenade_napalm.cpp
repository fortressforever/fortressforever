/********************************************************************
	created:	2006/08/14
	created:	14:8:2006   10:53
	filename: 	f:\ff-svn\code\trunk\game_shared\ff\ff_grenade_napalm.cpp
	file path:	f:\ff-svn\code\trunk\game_shared\ff
	file base:	ff_grenade_napalm
	file ext:	cpp
	author:		Various
	
	purpose:	Napalm grenade
*********************************************************************/

#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"

#ifdef GAME_DLL
	#include "ff_player.h"
	#include "ff_buildableobjects_shared.h"
	#include "baseentity.h"
	#include "ff_entity_system.h"
	#include "te_effect_dispatch.h"
#endif

#define NAPALMGRENADE_MODEL		"models/grenades/napalm/napalm.mdl"
#define NAPALM_EFFECT			"NapalmBurst"

#ifdef CLIENT_DLL
	#define CFFGrenadeNapalm C_FFGrenadeNapalm
#endif

#ifndef CLIENT_DLL
	ConVar nap_flame_time("ffdev_nap_flame_time","5.0",0,"How long the napalm grenade's fires burn");
	ConVar nap_burn_damage("ffdev_nap_burn_damage","1.0",0,"How much damage being in the radius of a napalm grenade deals.");
#endif

class CFFGrenadeNapalm : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeNapalm,CFFGrenadeBase)
	DECLARE_NETWORKCLASS();

	virtual void		Precache();
	virtual const char	*GetBounceSound() { return "NapalmGrenade.Bounce"; }
	virtual Class_T		Classify() { return CLASS_GREN_NAPALM; }

	float m_flLastBurnCheck;

#ifdef CLIENT_DLL
	CFFGrenadeNapalm() {}
	CFFGrenadeNapalm( const CFFGrenadeNapalm& ) {}

	virtual RenderGroup_t GetRenderGroup()  { return RENDER_GROUP_TWOPASS; }

#else
	DECLARE_DATADESC()
	virtual void Spawn();
	virtual void Explode( trace_t *pTrace, int bitsDamageType );
	void FlameThink(void);
#endif
};

#ifdef GAME_DLL
	BEGIN_DATADESC(CFFGrenadeNapalm)
		DEFINE_THINKFUNC( FlameThink ),
	END_DATADESC()
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeNapalm, DT_FFGrenadeNapalm)

BEGIN_NETWORK_TABLE(CFFGrenadeNapalm, DT_FFGrenadeNapalm)
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( ff_grenade_napalm, CFFGrenadeNapalm );
PRECACHE_WEAPON_REGISTER( ff_grenade_napalm );

#ifdef GAME_DLL

	//-----------------------------------------------------------------------------
	// Purpose: Set the correct model
	//-----------------------------------------------------------------------------
	void CFFGrenadeNapalm::Spawn()
	{
		SetModel(NAPALMGRENADE_MODEL);

		BaseClass::Spawn();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Instead of exploding, change to the napalm think
	//-----------------------------------------------------------------------------
	void CFFGrenadeNapalm::Explode(trace_t *pTrace, int bitsDamageType)
	{
		// Don't explode underwater
		if (UTIL_PointContents(GetAbsOrigin()) & (CONTENTS_SLIME|CONTENTS_WATER))
		{
			UTIL_Bubbles(GetAbsOrigin() - Vector(5, 5, 5), GetAbsOrigin() + Vector(5, 5, 5), 30);
			UTIL_Remove(this);
			return;
		}
		
		// Bug #0000370: napalm explosion not playing
		EmitSound("Napalm.Explode");

		CEffectData data;
		data.m_vOrigin = GetAbsOrigin();
		data.m_flScale = 1.0f;

		DispatchEffect(NAPALM_EFFECT, data);

		// Now do a napalm burning think
		SetDetonateTimerLength(nap_flame_time.GetFloat());

		// Should this maybe be noclip?
		SetMoveType( MOVETYPE_NONE );

		m_flLastBurnCheck = 0;
		SetThink(&CFFGrenadeNapalm::FlameThink);
		SetNextThink(gpGlobals->curtime);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Hurt nearby people and things
	//-----------------------------------------------------------------------------
	void CFFGrenadeNapalm::FlameThink()
	{
		// Remove if we've reached the end of our fuse
		if( gpGlobals->curtime > m_flDetonateTime )
		{
			UTIL_Remove(this);
			return;
		}

		SetAbsVelocity( Vector( 0, 0, 10 + 20 * sin( DEG2RAD( GetAbsAngles().y ) ) ) );
		SetAbsAngles( GetAbsAngles() + QAngle( 0, 15, 0 ) );

		Vector vecForward;
		AngleVectors( GetAbsAngles(), &vecForward );

		if((gpGlobals->curtime - m_flLastBurnCheck) >= 1.0f)
		{
			m_flLastBurnCheck = gpGlobals->curtime;

			float	flRadius = GetGrenadeRadius();
			Vector	vecSrc = GetAbsOrigin();
			vecSrc.z += 1;

			CBaseEntity *pEntity = NULL;

			for( CEntitySphereQuery sphere( vecSrc, flRadius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
			{
				if( !pEntity )
					continue;

				// Bug #0000269: Napalm through walls.
				trace_t tr;
				UTIL_TraceLine(GetAbsOrigin(), pEntity->GetAbsOrigin(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_DEBRIS, &tr);

				if (tr.fraction < 1.0f)
					continue;

				// Bug #0000270: Napalm grenade burn radius reaches unrealisticly high.
				float height = tr.startpos.z - tr.endpos.z;
				if (height < -40.0f || height > 40.0f)
					continue;

				Class_T cls = pEntity->Classify();

				switch(cls)
				{
				case CLASS_PLAYER:
					{
						CFFPlayer *pPlayer = ToFFPlayer( pEntity );

						if (g_pGameRules->FPlayerCanTakeDamage(pPlayer, GetOwnerEntity()))
						{
							pPlayer->TakeDamage( CTakeDamageInfo( this, GetOwnerEntity(), 10.0f, DMG_BURN ) );
							pPlayer->ApplyBurning( ToFFPlayer( GetOwnerEntity() ), 5.0f, 10.0f, BURNTYPE_NALPALMGRENADE);
						}
					}
					break;
				case CLASS_SENTRYGUN:
				case CLASS_DISPENSER:
					{
						CFFPlayer *pPlayer = ToFFPlayer(((CFFBuildableObject *) pEntity)->GetOwnerPlayer());

						if (g_pGameRules->FPlayerCanTakeDamage(pPlayer, GetOwnerEntity()))
						{
							pEntity->TakeDamage( CTakeDamageInfo( this, GetOwnerEntity(), 8.0f, DMG_BURN ) );
						}
					}
					
					default:
						break;
				}
			}
		}

		SetNextThink( gpGlobals->curtime );
	}
#endif

//-----------------------------------------------------------------------------
// Purpose: Precache
//-----------------------------------------------------------------------------
void CFFGrenadeNapalm::Precache()
{
	PrecacheScriptSound("Napalm.Explode");
	PrecacheModel( NAPALMGRENADE_MODEL );
	BaseClass::Precache();
}