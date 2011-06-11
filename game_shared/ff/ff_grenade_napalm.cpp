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

	extern short g_sModelIndexFireball;
	extern short g_sModelIndexWExplosion;
#endif

#define NAPALMGRENADE_MODEL		"models/grenades/napalm/napalm.mdl"
//#define NAPALM_EFFECT			"NapalmBurst"

#ifdef CLIENT_DLL
	#define CFFGrenadeNapalm C_FFGrenadeNapalm
	#include "clienteffectprecachesystem.h"
	#include "particles_simple.h"
#else
	ConVar nap_flame_time("ffdev_nap_flame_time","5.0",FCVAR_FF_FFDEV,"How long the napalm grenade's fires burn");
	ConVar nap_burn_damage("ffdev_nap_burn_damage","15.0",FCVAR_FF_FFDEV,"How much damage being in the radius of a napalm grenade deals.");
	ConVar nap_burn_radius("ffdev_nap_burn_radius","98.0",FCVAR_FF_FFDEV,"Burn radius of a napalmlet.");
	#include "EntityFlame.h"
	#include "smoke_trail.h"
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
	void SmokeThink(void);
	SmokeTrail *m_pSmokeTrail;
#endif
};

#ifdef GAME_DLL
	BEGIN_DATADESC(CFFGrenadeNapalm)
		DEFINE_THINKFUNC( SmokeThink ),
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
	//EmitSound( "General.StopBurning" );

	BaseClass::UpdateOnRemove();
}

#ifdef GAME_DLL

	//-----------------------------------------------------------------------------
	// Purpose: Set the correct model
	//-----------------------------------------------------------------------------
	void CFFGrenadeNapalm::Spawn()
	{
		SetModel(NAPALMGRENADE_MODEL);
		m_pSmokeTrail=NULL;

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
		
		//BaseClass::Explode( pTrace, bitsDamageType );
		
		// Jiggles: This is a paste of BaseClass::Explode() code, modified to remove the standard grenade explosion
		//			 and damage / blastforce
		SetModelName( NULL_STRING );//invisible
		AddSolidFlags( FSOLID_NOT_SOLID );

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

			//CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );

			CBaseEntity *pThrower = GetThrower();
			// Use the grenade's position as the reported position
			Vector vecReported = pTrace->endpos;
			CTakeDamageInfo info( this, pThrower, GetBlastForce()/4, GetAbsOrigin(), 0.0f/*m_flDamage*/, bitsDamageType, 0, &vecReported );
			RadiusDamage( info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );

			//EmitSound( "BaseGrenade.Explode" );
		}

		SetThink( &CFFGrenadeNapalm::SmokeThink );
		SetTouch( NULL );

		AddEffects( EF_NODRAW );
		//SetAbsVelocity( vec3_origin );
	
		SetNextThink( gpGlobals->curtime );
		//	Jiggles: End paste

		CBaseEntity *pOwner = GetOwnerEntity();

		if(!pOwner)
		{
			Warning("BIG PROBLEM: NULL OWNER (%s)\n", __FUNCTION__);
			return;
		}

		// Bug #0000370: napalm explosion not playing
		EmitSound("Napalm.Explode");


		for ( int i = 0; i < 8; i++ )
		{
			Vector vecSrc = GetAbsOrigin();
			QAngle angSpawn;

			angSpawn.x = RandomFloat(45.0f,75.0f);
			angSpawn.y = RandomFloat(0.0f, 360.0f);
			angSpawn.z = 0.0f;

			Vector vecVelocity;
			AngleVectors(angSpawn,&vecVelocity);
			vecVelocity *= RandomFloat(250.0f,350.0f);

			// So they don't begin moving down, I guess
			if (vecVelocity.z < 0)
				vecVelocity.z *= -1;

			CFFGrenadeNapalmlet *pNaplet = (CFFGrenadeNapalmlet *)CreateEntityByName( "ff_grenade_napalmlet" );

			if (!pNaplet)
				continue;

			// Jiggles: Instead, each Naplet makes it's own flame
			//CEntityFlame *pFlame = CEntityFlame::Create( pNaplet, false );
			//if (pFlame)
			//{
			//	pFlame->SetLifetime( nap_flame_time.GetFloat() );
			//	AddFlag( FL_ONFIRE );
			//	SetEffectEntity( pFlame );
			//	pFlame->SetSize( 60.0f );
			//}

			

			QAngle angRotate;
			angRotate.x = RandomFloat(-360.0f, 360.0f);
			angRotate.y = RandomFloat(-360.0f, 360.0f);
			angRotate.z = 2.0*RandomFloat(-360.0f, 360.0f);

			UTIL_SetOrigin( pNaplet, vecSrc );
			pNaplet->SetAbsAngles(QAngle(0,0,0)); //make the model stand on end
			pNaplet->SetLocalAngularVelocity(angRotate);
			pNaplet->Spawn();

			pNaplet->SetBurnRadius( nap_burn_radius.GetFloat() );
			pNaplet->SetBurnTime( nap_flame_time.GetFloat() );
			pNaplet->SetBurnDamage( nap_burn_damage.GetFloat() );
			pNaplet->SetOwnerEntity( pOwner );


			pNaplet->SetAbsVelocity( vecVelocity );
			//pNaplet->SetupInitialTransmittedVelocity( vecVelocity );
			pNaplet->SetElasticity( 0.2f );

			pNaplet->ChangeTeam( pOwner->GetTeamNumber() );
			pNaplet->SetGravity( GetGrenadeGravity() + 0.1f );
			pNaplet->SetFriction( GetGrenadeFriction() );
		}


		//CEffectData data;
		//data.m_vOrigin = GetAbsOrigin();
		//data.m_flScale = 1.0f;

		//DispatchEffect(NAPALM_EFFECT, data);

		// Now do a napalm burning think
		SetDetonateTimerLength(nap_flame_time.GetFloat());
		m_bIsOn = true;

		// Should this maybe be noclip?
		SetMoveType( MOVETYPE_NONE );

		//m_flLastBurnCheck = 0;
		SetThink(&CFFGrenadeNapalm::SmokeThink);
		SetNextThink(gpGlobals->curtime);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Generate Smoke
	//			Jiggles: This used to be FlameThink, but I changed it so that each
	//					 Naplet does its own FlameThink
	//-----------------------------------------------------------------------------
	void CFFGrenadeNapalm::SmokeThink()
	{
		// Remove if we've reached the end of our fuse
		if( gpGlobals->curtime > m_flDetonateTime )
		{
			UTIL_Remove(this);
			return;
		}

		if (!m_pSmokeTrail)
		{
			m_pSmokeTrail =  SmokeTrail::CreateSmokeTrail();
			if (m_pSmokeTrail)
			{
				m_pSmokeTrail->FollowEntity( this );
				m_pSmokeTrail->m_SpawnRate = 4;
				m_pSmokeTrail->m_ParticleLifetime = 3.0f;
				m_pSmokeTrail->m_StartColor.Init( 0.7f, 0.7f, 0.7f );
				m_pSmokeTrail->m_EndColor.Init( 0.6, 0.6, 0.6 );
				m_pSmokeTrail->m_StartSize = 86;
				m_pSmokeTrail->m_EndSize = 196;
				m_pSmokeTrail->m_SpawnRadius = 4;
				m_pSmokeTrail->m_Opacity = 0.5f;
				m_pSmokeTrail->m_MinSpeed = 16;
				m_pSmokeTrail->m_MaxSpeed = 16;
				m_pSmokeTrail->m_MinDirectedSpeed	= 16.0f;
				m_pSmokeTrail->m_MaxDirectedSpeed	= 16.0f;
				m_pSmokeTrail->SetLifetime( 5.0f );
			}
		}
		SetNextThink( m_flDetonateTime );
	}
// Jiggles: Old burn code below
//
//		//SetAbsVelocity( Vector( 0, 0, 10 + 20 * sin( DEG2RAD( GetAbsAngles().y ) ) ) );
//		//SetAbsAngles( GetAbsAngles() + QAngle( 0, 15, 0 ) );
//
//		//Vector vecForward;
//		//AngleVectors( GetAbsAngles(), &vecForward );
//
//		if((gpGlobals->curtime - m_flLastBurnCheck) >= 1.0f)
//		{
//			m_flLastBurnCheck = gpGlobals->curtime;
//
//			float	flRadius = GetGrenadeRadius();
//			Vector	vecSrc = GetAbsOrigin();
//			vecSrc.z += 1;
//
//			CBaseEntity *pEntity = NULL;
//
//			for( CEntitySphereQuery sphere( vecSrc, flRadius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
//			{
//				if( !pEntity )
//					continue;
//
//				// Bug #0000269: Napalm through walls.
//				// Mulch: if we hit water w/ the trace, abort too!
//				trace_t tr;
//				UTIL_TraceLine(GetAbsOrigin(), pEntity->GetAbsOrigin(), MASK_SOLID_BRUSHONLY | CONTENTS_WATER, this, COLLISION_GROUP_DEBRIS, &tr);
//
//				if (tr.fraction < 1.0f)
//					continue;
//
//				// Bug #0000270: Napalm grenade burn radius reaches unrealisticly high.
//				float height = tr.startpos.z - tr.endpos.z;
//				if (height < -40.0f || height > 40.0f)
//					continue;
//
//				// Don't damage if entity is more than feet deep in water
//				if( pEntity->GetWaterLevel() >= 2 )
//					continue;
//
//				switch( pEntity->Classify() )
//				{
//					case CLASS_PLAYER:
//					{
//						CFFPlayer *pPlayer = ToFFPlayer( pEntity );
//						if( !pPlayer )
//							continue;
//
//						if (g_pGameRules->FCanTakeDamage(pPlayer, GetOwnerEntity()))
//						{
//							pPlayer->TakeDamage( CTakeDamageInfo( this, GetOwnerEntity(), 10.0f, DMG_BURN ) );
//							pPlayer->ApplyBurning( ToFFPlayer( GetOwnerEntity() ), 5.0f, 10.0f, BURNTYPE_NALPALMGRENADE);
//						}
//					}
//					break;
//					case CLASS_SENTRYGUN:
//					case CLASS_DISPENSER:
//					{
//						CFFBuildableObject *pBuildable = dynamic_cast< CFFBuildableObject * >( pEntity );
//						if( !pBuildable )
//							continue;
//						
//						CFFPlayer *pPlayer = pBuildable->GetOwnerPlayer();
//						if( !pPlayer )
//							continue;
//
//						if (g_pGameRules->FCanTakeDamage(pPlayer, GetOwnerEntity()))
//							pBuildable->TakeDamage( CTakeDamageInfo( this, GetOwnerEntity(), 8.0f, DMG_BURN ) );
//					}
//					
//					default:
//						break;
//				}
//			}
//		}
//
//		SetNextThink( gpGlobals->curtime );
//	}
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
