/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_grenade_napalm.cpp
/// @author Shawn Smith (L0ki)
/// @date Jan. 31, 2005
/// @brief napalm grenade class
///
/// Implementation of the CFFGrenadeNapalm class.  This is the secondary grenade type for the pyro class.
/// 
/// Revisions
/// ---------
/// Jan. 30, 2005	L0ki: Initial Creation
/// Apr. 23, 2005	L0ki: removed header file, moved everything to a single cpp file
/// Apr. 24, 2005	L0ki: added GrenadeThink and FlameThink functions to control transition from
///					napalm grenade to a bunch of fires

//========================================================================
// Required includes
//========================================================================
#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"
#ifdef GAME_DLL
	#include "ff_player.h"
	#include "ff_buildableobjects_shared.h"
	#include "baseentity.h"
	#include "ff_entity_system.h"
#endif

//#define NAPALMGRENADE_MODEL "models/weapons/w_eq_fraggrenade_thrown.mdl"
#define NAPALMGRENADE_MODEL	"models/grenades/napalm/napalm.mdl"
#define NAPALM_EFFECT "NapalmBurst"
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

	CNetworkVector(m_vInitialVelocity);

	virtual void Precache();
	virtual const char *GetBounceSound() { return "NapalmGrenade.Bounce"; }
	virtual Class_T Classify( void ) { return CLASS_GREN_NAPALM; }

	float m_flLastBurnCheck;
#ifdef CLIENT_DLL
	CFFGrenadeNapalm() {}
	CFFGrenadeNapalm( const CFFGrenadeNapalm& ) {}
#else
	DECLARE_DATADESC()
	virtual void Spawn();
	virtual void Explode( trace_t *pTrace, int bitsDamageType );
	void GrenadeThink(void);
	void FlameThink(void);
#endif
};

#ifdef GAME_DLL
	BEGIN_DATADESC(CFFGrenadeNapalm)
		DEFINE_THINKFUNC( GrenadeThink ),
		DEFINE_THINKFUNC( FlameThink ),
	END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( napalmgrenade, CFFGrenadeNapalm );
PRECACHE_WEAPON_REGISTER( napalmgrenade );

#ifdef GAME_DLL
	void CFFGrenadeNapalm::Spawn( void )
	{
//		DevMsg("[Grenade Debug] CFFGrenadeNapalm::\n");
		SetModel( NAPALMGRENADE_MODEL );
		BaseClass::Spawn();
	}
	void CFFGrenadeNapalm::Explode( trace_t *pTrace, int bitsDamageType )
	{
		
//		DevMsg("[Grenade Debug] CFFGrenadeNapalm::Explode\n");
		CFFGrenadeBase::PreExplode( pTrace, NULL, NAPALM_EFFECT );
		
		// If the grenade is not in a no gren area
		if( FFScriptRunPredicates( this, "onexplode", true ) )
		{
			// Bug #0000370: napalm explosion not playing
			EmitSound("Napalm.Explode");
		}

		CFFGrenadeBase::PostExplode(); 
	}

	void CFFGrenadeNapalm::GrenadeThink( void )
	{
		// Remove if we're nolonger in the world
		if (!IsInWorld())
		{
			Remove( );
			return;
		}

		// Blow up if we've reached the end of our fuse
		if( gpGlobals->curtime > m_flDetonateTime )
		{
//			DevMsg("[Grenade Debug] CFFGrenadeNapalm:: Detonate stuffs\n");
			//napalm grenade explodes and spews "napalm" over an area, then starts some fires
			Detonate();

			if( FFScriptRunPredicates( this, "onexplode", true ) )
			{
				// Reset the detonation time
				SetDetonateTimerLength( nap_flame_time.GetFloat() );

				// Should this maybe be noclip?
				SetMoveType( MOVETYPE_NONE );

				//start burning napalm
				m_flLastBurnCheck = 0;
				SetThink( &CFFGrenadeNapalm::FlameThink );		// |-- Mirv: Account for GCC strictness
				SetNextThink( gpGlobals->curtime );
			}

			return;
		}

		// Bug #0000501: Doors can be blocked by shit that shouldn't block them.
		if( GetGroundEntity() && ( GetAbsVelocity() == vec3_origin ) )
		{
			if( GetGroundEntity()->GetMoveType() != MOVETYPE_PUSH )
				SetMoveType( MOVETYPE_NONE );
		}

		// Next think straight away
		SetNextThink( gpGlobals->curtime );

		CFFGrenadeBase::WaterThink(); // Mulch: bug 0000273: make grens sink-ish in water
	}

	void CFFGrenadeNapalm::FlameThink( void )
	{
		/*static int iFlameThinks = 0;
		static float flLastCalcTime = 0;
		iFlameThinks++;
		if((gpGlobals->curtime - flLastCalcTime) >= 1.0f)
		{
		DevMsg("number of FlameThinks per second: %f\n",iFlameThinks / (gpGlobals->curtime - flLastCalcTime));
		flLastCalcTime = gpGlobals->curtime;
		iFlameThinks = 0;
		}*/
		// Blow up if we've reached the end of our fuse
		if( gpGlobals->curtime > m_flDetonateTime )
		{
//			DevMsg("[Grenade Debug] CFFGrenadeNapalm -- We're done!\n");
			// we dont need to detonate, just remove the grenade
			SetModelName( NULL_STRING );//invisible
			AddSolidFlags( FSOLID_NOT_SOLID );
			SetThink( &CBaseGrenade::SUB_Remove );
			SetTouch( NULL );

			AddEffects( EF_NODRAW );
			SetAbsVelocity( vec3_origin );
			SetNextThink( gpGlobals->curtime );
			return;
		}

		SetAbsVelocity( Vector( 0, 0, 10 + 20 * sin( DEG2RAD( GetAbsAngles().y ) ) ) );

		SetAbsAngles( GetAbsAngles() + QAngle( 0, 15, 0 ) );

		Vector vecForward;

		AngleVectors( GetAbsAngles(), &vecForward );

//		DevMsg("[Grenade Debug] CFFGrenadeNapalm::FlameThink\n");
		//we want to deal 2 burn damage per second
		if((gpGlobals->curtime - m_flLastBurnCheck) >= 1.0f)
		{
//			DevMsg("[Grenade Debug] CFFGrenadeNapalm::FlameThink\n[Grenade Debug] Looping through entities\n");
			m_flLastBurnCheck = gpGlobals->curtime;

			float flRadius = CFFGrenadeBase::GetGrenadeRadius();
			Vector vecSrc = GetAbsOrigin();
			vecSrc.z += 1;// in case grenade is lying on the ground
			//BEGIN_ENTITY_SPHERE_QUERY(vecSrc, flRadius)
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
						if( pPlayer && !pPlayer->IsObserver() && pPlayer->IsAlive() )
						{
							// damage enemies and self
							if( g_pGameRules->FPlayerCanTakeDamage( pPlayer, GetOwnerEntity() ) )
							{
//								DevMsg("[Grenade Debug] damaging enemy or self\n");
								//CTakeDamageInfo info( this, pPlayer, 1.0f, DMG_BURN );
								//TakeDamage( info );
								// Why do we do damage to the grenade itself?

								pPlayer->TakeDamage( CTakeDamageInfo( this, GetOwnerEntity(), 1.0f, DMG_BURN ) );
								pPlayer->ApplyBurning( ToFFPlayer( GetOwnerEntity() ), 1.0f );
							}
						}
					}				
					break;

					case CLASS_SENTRYGUN:
					{
						CFFSentryGun *pSentryGun = dynamic_cast< CFFSentryGun * >( pEntity );
						if( g_pGameRules->FPlayerCanTakeDamage( ToFFPlayer( pSentryGun->m_hOwner.Get() ), GetOwnerEntity() ) )
							pSentryGun->TakeDamage( CTakeDamageInfo( this, GetOwnerEntity(), 8.0f, DMG_BURN ) );
					}
					break;
				
					case CLASS_DISPENSER:
					{
						CFFDispenser *pDispenser = dynamic_cast< CFFDispenser * >( pEntity );
						if( g_pGameRules->FPlayerCanTakeDamage( ToFFPlayer( pDispenser->m_hOwner.Get() ), GetOwnerEntity() ) )
							pDispenser->TakeDamage( CTakeDamageInfo( this, GetOwnerEntity(), 8.0f, DMG_BURN ) );
					}
					break;
					
					default:
						break;
				}
//				DevMsg("[Grenade Debug] Checking next entity\n");
			//END_ENTITY_SPHERE_QUERY();
			}
		}
		SetNextThink( gpGlobals->curtime );
	}
#endif

void CFFGrenadeNapalm::Precache()
{
//	DevMsg("[Grenade Debug] CFFGrenadeNapalm::Precache\n");
	PrecacheScriptSound("Napalm.Explode");
	PrecacheModel( NAPALMGRENADE_MODEL );
	BaseClass::Precache();
}