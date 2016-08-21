#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"

#ifdef GAME_DLL
	#include "ff_player.h"
	#include "ff_buildableobjects_shared.h"
	#include "baseentity.h"
	#include "ff_entity_system.h"
	#include "te_effect_dispatch.h"

	extern short g_sModelIndexFireball;
	extern short g_sModelIndexWExplosion;
#endif

#define NAPALMGRENADE_MODEL		"models/grenades/napalm/napalm.mdl"

#ifdef CLIENT_DLL
	#define CFFGrenadeNapalm C_FFGrenadeNapalm
	#include "clienteffectprecachesystem.h"
	#include "particles_simple.h"
#else
	//ConVar nap_flame_time("ffdev_nap_flame_time","5.0",FCVAR_FF_FFDEV,"How long the napalm grenade's fires burn");
	#define NAP_FLAME_TIME 5.0f
	//ConVar nap_burn_damage("ffdev_nap_burn_damage","15.0",FCVAR_FF_FFDEV,"How much damage being in the radius of a napalm grenade deals.");
	#define NAP_BURN_DAMAGE 15.0f
	//ConVar nap_burn_radius("ffdev_nap_burn_radius","98.0",FCVAR_FF_FFDEV,"Burn radius of a napalmlet.");
	#define NAP_BURN_RADIUS 98.0f

	ConVar ffdev_nap_distance_min("ffdev_nap_distance_min","250.0",FCVAR_FF_FFDEV,"Min launch velocity of a napalmlet.");
	#define FFDEV_NAP_DISTANCE_MIN ffdev_nap_distance_min.GetFloat() //250.0f
	ConVar ffdev_nap_distance_max("ffdev_nap_distance_max","350.0",FCVAR_FF_FFDEV,"Max launch velocity of a napalmlet.");
	#define FFDEV_NAP_DISTANCE_MAX ffdev_nap_distance_max.GetFloat() // 350.0f

	#include "EntityFlame.h"
	#include "ff_grenade_napalmlet.h"
#endif

class CFFGrenadeNapalm : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeNapalm,CFFGrenadeBase)
	DECLARE_NETWORKCLASS();

	virtual void		Precache();
	virtual const char	*GetBounceSound() { return "NapalmGrenade.Bounce"; }
	virtual Class_T		Classify() { return CLASS_GREN_NAPALM; }
	virtual void UpdateOnRemove( void );

	float m_flLastBurnCheck;

	virtual color32 GetColour() { color32 col = { 255, 128, 0, GREN_ALPHA_DEFAULT }; return col; }

#ifdef CLIENT_DLL
	CFFGrenadeNapalm() {}
	CFFGrenadeNapalm( const CFFGrenadeNapalm& ) {}

	virtual RenderGroup_t GetRenderGroup()  { return RENDER_GROUP_TWOPASS; }
	static PMaterialHandle m_hHeatwaveMaterial;

#else
	DECLARE_DATADESC()
	virtual void Spawn();
	virtual void Explode( trace_t *pTrace, int bitsDamageType );
#endif
};

#ifdef GAME_DLL
	BEGIN_DATADESC(CFFGrenadeNapalm)
	END_DATADESC()
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeNapalm, DT_FFGrenadeNapalm)

BEGIN_NETWORK_TABLE(CFFGrenadeNapalm, DT_FFGrenadeNapalm)
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( ff_grenade_napalm, CFFGrenadeNapalm );
PRECACHE_WEAPON_REGISTER( ff_grenade_napalm );

void CFFGrenadeNapalm::UpdateOnRemove( void )
{
	StopSound( "General.BurningFlesh" );
	StopSound( "General.BurningObject" );

	BaseClass::UpdateOnRemove();
}

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

			CBaseEntity *pThrower = GetThrower();
			// Use the grenade's position as the reported position
			Vector vecReported = pTrace->endpos;
			CTakeDamageInfo info( this, pThrower, GetBlastForce()/4, GetAbsOrigin(), 0.0f/*m_flDamage*/, bitsDamageType, 0, &vecReported );
			RadiusDamage( info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );
		}

		CBaseEntity *pOwner = GetOwnerEntity();

		if(!pOwner)
		{
			Warning("BIG PROBLEM: NULL OWNER (%s)\n", __FUNCTION__);
			return;
		}

		EmitSound("Napalm.Explode");

		for ( int i = 0; i < 9; i++ )
		{
			Vector vecSrc = GetAbsOrigin();
			QAngle angSpawn;

			angSpawn.x = RandomFloat(45.0f,75.0f);
			if (i == 0)
			{
				angSpawn.x = 90;
			}

			angSpawn.y = RandomFloat(0.0f, 360.0f);
			angSpawn.z = 0.0f;

			Vector vecVelocity;
			AngleVectors(angSpawn,&vecVelocity);
			vecVelocity *= RandomFloat(FFDEV_NAP_DISTANCE_MIN,FFDEV_NAP_DISTANCE_MAX);

			// So they don't begin moving down, I guess
			if (vecVelocity.z < 0)
				vecVelocity.z *= -1;

			CFFGrenadeNapalmlet *pNaplet = (CFFGrenadeNapalmlet *)CreateEntityByName( "ff_grenade_napalmlet" );

			if (!pNaplet)
				continue;

			QAngle angRotate;
			angRotate.x = RandomFloat(-360.0f, 360.0f);
			angRotate.y = RandomFloat(-360.0f, 360.0f);
			angRotate.z = 2.0*RandomFloat(-360.0f, 360.0f);

			UTIL_SetOrigin( pNaplet, vecSrc );
			pNaplet->SetAbsAngles(QAngle(0,0,0)); //make the model stand on end
			pNaplet->SetLocalAngularVelocity(angRotate);
			pNaplet->Spawn();

			pNaplet->SetBurnRadius( NAP_BURN_RADIUS );
			pNaplet->SetBurnTime( NAP_FLAME_TIME );
			pNaplet->SetBurnDamage( NAP_BURN_DAMAGE );
			pNaplet->SetOwnerEntity( pOwner );


			pNaplet->SetAbsVelocity( vecVelocity );
			pNaplet->SetElasticity( 0.2f );

			pNaplet->ChangeTeam( pOwner->GetTeamNumber() );
			pNaplet->SetGravity( GetGrenadeGravity() + 0.1f );
			pNaplet->SetFriction( GetGrenadeFriction() );
		}

		UTIL_Remove(this);
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
