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
#include "ff_grenade_base.h"
#include "ff_utils.h"
#include "engine/IEngineSound.h"
#ifdef GAME_DLL
	#include "ff_buildableobjects_shared.h"
	#include "ff_projectile_pipebomb.h"
	#include "baseentity.h"
	#include "beam_flags.h"
	#include "ff_entity_system.h"
#endif

#define EMPGRENADE_MODEL "models/grenades/emp/emp.mdl"
#define EMP_SOUND "EmpGrenade.Explode"
#define EMP_EFFECT "EmpExplosion"

extern short g_sModelIndexFireball;
extern short g_sModelIndexWExplosion;

#ifdef CLIENT_DLL
	#define CFFGrenadeEmp C_FFGrenadeEmp
#endif

#ifndef CLIENT_DLL
	ConVar emp_framerate("ffdev_emp_framerate","2",0,"Framerate of the emp explosion");
	ConVar emp_width("ffdev_emp_width","8.0",0,"width of the emp shockwave");
	ConVar emp_life("ffdev_emp_life","0.3",0,"life of the emp shockwave");
	ConVar emp_spread("ffdev_emp_spread","0",0,"spread of the emp shockwave");
	ConVar emp_amplitude("ffdev_emp_amplitude","1",0,"amplitude of the emp shockwave");
	ConVar emp_speed("ffdev_emp_speed","0",0,"speed of the emp shockwave");
	ConVar emp_buildable_damage("ffdev_emp_buildable_damage","10.0",0,"Amount of damage to deal to sentryguns and dispensers in the emp radius.");
#endif

class CFFGrenadeEmp : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeEmp,CFFGrenadeBase)

	CNetworkVector(m_vInitialVelocity);

	virtual void Precache();
	virtual Class_T Classify( void ) { return CLASS_GREN_EMP; }

	virtual float GetShakeAmplitude( void ) { return 0.0f; }	// remove the shake
	virtual float GetGrenadeRadius() { return 240.0f; }
	virtual float GetGrenadeDamage() { return 0.0f; }
	virtual const char *GetBounceSound() { return "EmpGrenade.Bounce"; }

#ifdef CLIENT_DLL
	CFFGrenadeEmp() {}
	CFFGrenadeEmp( const CFFGrenadeEmp& ) {}
#else
	virtual void Spawn();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);

	void GrenadeThink( void );
	bool m_bWarned;
#endif
};

LINK_ENTITY_TO_CLASS( empgrenade, CFFGrenadeEmp );
PRECACHE_WEAPON_REGISTER( empgrenade );

#ifdef GAME_DLL

	void CFFGrenadeEmp::Spawn( void )
	{
		DevMsg("[Grenade Debug] CFFGrenadeEmp::Spawn\n");
		SetModel( EMPGRENADE_MODEL );
		m_bWarned = false;
		BaseClass::Spawn();
	}

	void CFFGrenadeEmp::Explode(trace_t *pTrace, int bitsDamageType)
	{
		DevMsg("[Grenade Debug] CFFGrenadeEmp::Explode\n");
		//CFFGrenadeBase::PreExplode( pTrace );//, EMP_SOUND, EMP_EFFECT );
		//CFFGrenadeBase::PreExplode( pTrace, NULL, "FF_RingEffect" );
		CFFGrenadeBase::PreExplode(pTrace, NULL, "FF_EmpZap");
		
		// If the grenade is in a no gren area don't explode
		if( FFScriptRunPredicates( this, "onexplode", true ) )
		{		
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
							( ( CFFProjectilePipebomb * )pEntity )->DetonatePipe( true, GetOwnerEntity() );
						break;

						default:
							// For all other projectiles or objects that return
							// something from TakeEmp we gotta add the explosions
							// ourselves

							DevMsg( "%s exploding with force of %d\n", pEntity->GetClassname(), explode );

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

							// Sound
							CSoundEnt::InsertSound ( SOUND_COMBAT, pEntity->GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );

							CTakeDamageInfo info( this, GetOwnerEntity(), GetBlastForce(), pEntity->GetAbsOrigin(), explode, DMG_SHOCK, 0, &vecOrigin );
							RadiusDamage( info, pEntity->GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );
						
							EmitSound( "BaseGrenade.Explode" );

							UTIL_ScreenShake( pEntity->GetAbsOrigin(), explode, 150.0, 1.0, explode * 30, SHAKE_START );
						break;

					}
				}
			}			
		}

		// EXPLODE THE ACTUAL EMP GRENADE NOW

		SetModelName( NULL_STRING );
		AddSolidFlags( FSOLID_NOT_SOLID );

		// According to PHISH & Dospac the TFC grenade
		// doesn't do any of this stuff (other than
		// blowing up ammo stored on players/objects) &
		// the stuff we're having it do in FF.

		// If the grenade is in a no gren area don't explode
		if( FFScriptRunPredicates( this, "onexplode", true ) )
			CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );

		SetThink( &CBaseGrenade::SUB_Remove );
		SetTouch( NULL );

		AddEffects( EF_NODRAW );
		SetAbsVelocity( vec3_origin );
		SetNextThink( gpGlobals->curtime );
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

			// If the grenade is in a no gren area don't explode
			if( FFScriptRunPredicates( this, "onexplode", true ) )
				EmitSound(EMP_SOUND);
		}
	}
#endif

//----------------------------------------------------------------------------
// Purpose: Precache the model ready for spawn
//----------------------------------------------------------------------------
void CFFGrenadeEmp::Precache()
{
	DevMsg("[Grenade Debug] CFFGrenadeEmp::Precache\n");
	PrecacheModel( EMPGRENADE_MODEL );
	PrecacheScriptSound( EMP_SOUND );
	BaseClass::Precache();
}