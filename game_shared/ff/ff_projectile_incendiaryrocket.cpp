#include "cbase.h"
#include "ff_utils.h"
#include "ff_projectile_incendiaryrocket.h"

#define INCENDIARYROCKET_MODEL "models/gibs/AGIBS.mdl"

#ifdef GAME_DLL
	#include "smoke_trail.h"
	#include "ff_buildableobjects_shared.h"
	#include "soundent.h"
	#include "effect_dispatch_data.h"
	#include "te_effect_dispatch.h"
#else

#endif

ConVar ffdev_ic_bonusdamage_burn1("ffdev_ic_bonusdamage_burn1", "20", FCVAR_REPLICATED | FCVAR_CHEAT);
#define IC_BONUSDAMAGE_BURN1 ffdev_ic_bonusdamage_burn1.GetFloat()

ConVar ffdev_ic_bonusdamage_burn2("ffdev_ic_bonusdamage_burn2", "30", FCVAR_REPLICATED | FCVAR_CHEAT);
#define IC_BONUSDAMAGE_BURN2 ffdev_ic_bonusdamage_burn2.GetFloat()

ConVar ffdev_ic_bonusdamage_burn3("ffdev_ic_bonusdamage_burn3", "40", FCVAR_REPLICATED | FCVAR_CHEAT);
#define IC_BONUSDAMAGE_BURN3 ffdev_ic_bonusdamage_burn3.GetFloat()

//ConVar ffdev_ic_flarescale("ffdev_ic_flarescale", "0.8", FCVAR_REPLICATED | FCVAR_CHEAT);
#define FFDEV_IC_FLARESCALE 0.8f //ffdev_ic_flarescale.GetFloat()
//ConVar ffdev_ic_smoke_opacity("ffdev_ic_smoke_opacity", "0.1", FCVAR_REPLICATED | FCVAR_CHEAT);
#define FFDEV_IC_SMOKE_OPACITY 0.1f //ffdev_ic_smoke_opacity.GetFloat()
//ConVar ffdev_ic_smoke_startsize("ffdev_ic_smoke_startsize", "3", FCVAR_REPLICATED | FCVAR_CHEAT);
#define FFDEV_IC_SMOKE_STARTSIZE 3.0f //ffdev_ic_smoke_startsize.GetFloat()
//ConVar ffdev_ic_smoke_endsize("ffdev_ic_smoke_endsize", "5", FCVAR_REPLICATED | FCVAR_CHEAT);
#define FFDEV_IC_SMOKE_ENDSIZE 5.0f //ffdev_ic_smoke_endsize.GetFloat()
//ConVar ffdev_ic_smoke_spawnradius("ffdev_ic_smoke_spawnradius", "1", FCVAR_REPLICATED | FCVAR_CHEAT);
#define FFDEV_IC_SMOKE_SPAWNRADIUS 1.0f //ffdev_ic_smoke_spawnradius.GetFloat()
//ConVar ffdev_ic_smoke_minspeed("ffdev_ic_smoke_minspeed", "2", FCVAR_REPLICATED | FCVAR_CHEAT);
#define FFDEV_IC_SMOKE_MINSPEED 2.0f //ffdev_ic_smoke_minspeed.GetFloat()
//ConVar ffdev_ic_smoke_maxspeed("ffdev_ic_smoke_maxspeed", "10", FCVAR_REPLICATED | FCVAR_CHEAT);
#define FFDEV_IC_SMOKE_MAXSPEED 10.0f //ffdev_ic_smoke_maxspeed.GetFloat()



extern short	g_sModelIndexFireball;		// (in combatweapon.cpp) holds the index for the fireball 
extern short	g_sModelIndexWExplosion;	// (in combatweapon.cpp) holds the index for the underwater explosion
extern short	g_sModelIndexSmoke;			// (in combatweapon.cpp) holds the index for the smoke cloud

//=============================================================================
// CFFProjectileIncendiaryRocket tables
//=============================================================================

LINK_ENTITY_TO_CLASS(ff_projectile_ic, CFFProjectileIncendiaryRocket);
PRECACHE_WEAPON_REGISTER(ff_projectile_ic);

#ifdef GAME_DLL
BEGIN_DATADESC(CFFProjectileIncendiaryRocket)
DEFINE_THINKFUNC(ArcThink),
END_DATADESC()
#endif

//=============================================================================
// CFFProjectileIncendiaryRocket implementation
//=============================================================================

void CFFProjectileIncendiaryRocket::Explode(trace_t *pTrace, int bitsDamageType)
{
	// Make sure grenade explosion is 32.0f away from hit point
	if (pTrace->fraction != 1.0)
		SetLocalOrigin(pTrace->endpos + (pTrace->plane.normal * 32));

#ifdef GAME_DLL

	Vector vecSrc = GetAbsOrigin();
	vecSrc.z += 1;

	CFFPlayer *pBurninator = ToFFPlayer( GetOwnerEntity() );
	if ( !pBurninator )
		return;
	// Do normal radius damage then do a trace sphere to set things alight
	Vector vecReported = pTrace->endpos; //m_hThrower ? m_hThrower->GetAbsOrigin() : vec3_origin;
	CTakeDamageInfo info(this, pBurninator, GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, m_iKillType, &vecReported);
	RadiusDamage(info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL);
	
	// Sorry, not fond of the BEGIN_ENTITY_SPHERE_QUERY macro
	CBaseEntity *pEntity = NULL;
	for( CEntitySphereQuery sphere( vecSrc, m_DmgRadius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if( !pEntity || !pEntity->IsPlayer() )
			continue;

		CFFPlayer *pPlayer = ToFFPlayer( pEntity );
		if( g_pGameRules->FCanTakeDamage( pPlayer, pBurninator ) )
		{
			if (pPlayer->GetBurnLevel() > 0)
			{
				float damage = CalculateBonusIcBurnDamage(pPlayer->GetBurnLevel());
				pPlayer->TakeDamage(CTakeDamageInfo( this, pBurninator, damage /*IC_BONUSDAMAGE*/, DMG_BURN ) );
				pPlayer->IncreaseBurnLevel(100);

				CEffectData data;
				data.m_vOrigin = GetAbsOrigin();
				data.m_flScale = damage;
				data.m_nEntIndex = pPlayer->entindex();
				DispatchEffect("BonusFire", data);
			}
		}
	}

	// We don't use this and it also caused a NULL pointer assert from
	// GetOwnerEntity()
	//Vector vecDisp = GetOwnerEntity()->GetAbsOrigin() - GetAbsOrigin();

#endif
	// 0000936: go through the explode code but don't apply damage! from basegrenade.cpp
	
#if !defined( CLIENT_DLL )
	
	SetModelName( NULL_STRING );//invisible
	AddSolidFlags( FSOLID_NOT_SOLID );

	m_takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if ( pTrace->fraction != 1.0 )
	{
		SetLocalOrigin( pTrace->endpos + (pTrace->plane.normal * 32.0f) );	// |-- Mirv: 32 units used in TFC
	}

	Vector vecAbsOrigin = GetAbsOrigin();
	int contents = UTIL_PointContents ( vecAbsOrigin );

#if defined( TF_DLL )
	// Since this code only runs on the server, make sure it shows the tempents it creates.
	// This solves a problem with remote detonating the pipebombs (client wasn't seeing the explosion effect)
	CDisablePredictionFiltering disabler;
#endif

	if ( pTrace->fraction != 1.0 )
	{
		Vector vecNormal = pTrace->plane.normal;
		surfacedata_t *pdata = physprops->GetSurfaceData( pTrace->surface.surfaceProps );	
		CPASFilter filter( vecAbsOrigin );

		te->Explosion( filter, -1.0, // don't apply cl_interp delay
			&vecAbsOrigin,
			!( contents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
			/*m_DmgRadius * .03*/ m_flDamage / 128.0f, 
			25,
			TE_EXPLFLAG_NONE,
			m_DmgRadius,
			m_flDamage,
			&vecNormal,
			(char) pdata->game.material );
	}
	else
	{
		CPASFilter filter( vecAbsOrigin );
		te->Explosion( filter, -1.0, // don't apply cl_interp delay
			&vecAbsOrigin, 
			!( contents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
			/*m_DmgRadius * .03*/ m_flDamage / 128.0f, 
			25,
			TE_EXPLFLAG_NONE,
			m_DmgRadius,
			m_flDamage );
	}

#if !defined( CLIENT_DLL )
	CSoundEnt::InsertSound ( SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );
#endif

	// We need to report where the explosion took place
	vecReported = pTrace->endpos; //m_hThrower ? m_hThrower->GetAbsOrigin() : vec3_origin;
	
	// Took out the damage info here, since we don't want to apply damage 2x

	UTIL_DecalTrace( pTrace, "Scorch" );

	EmitSound( "BaseGrenade.Explode" );

	SetThink( &CBaseGrenade::SUB_Remove );
	SetTouch( NULL );
	
	AddEffects( EF_NODRAW );
	SetAbsVelocity( vec3_origin );
	SetNextThink( gpGlobals->curtime );
#endif
}

#ifdef GAME_DLL

	//----------------------------------------------------------------------------
	// Purpose: Calculate the bonus damage for the IC based on the players current burn level
	//----------------------------------------------------------------------------
	float CFFProjectileIncendiaryRocket::CalculateBonusIcBurnDamage(int burnLevel)
	{
		if (burnLevel <100)
		{
			return IC_BONUSDAMAGE_BURN1;
		}
		if (burnLevel <200)
		{
			return IC_BONUSDAMAGE_BURN2;
		}

		return IC_BONUSDAMAGE_BURN3;
	}

	//----------------------------------------------------------------------------
	// Purpose: Creata a trail of smoke for the rocket
	//----------------------------------------------------------------------------
	void CFFProjectileIncendiaryRocket::CreateRocketTrail()
	{
		// Smoke trail.
		if ((m_hRocketTrail = RocketTrail::CreateRocketTrail()) != NULL)
		{
			m_hRocketTrail->m_Opacity = FFDEV_IC_SMOKE_OPACITY;
			m_hRocketTrail->m_SpawnRate = 100;
			m_hRocketTrail->m_ParticleLifetime = 0.3f;
			m_hRocketTrail->m_StartColor.Init(1.0f, 0.3f, 0.0f);
			m_hRocketTrail->m_EndColor.Init(0.0f, 0.0f, 0.0f);
			m_hRocketTrail->m_StartSize = FFDEV_IC_SMOKE_STARTSIZE;
			m_hRocketTrail->m_EndSize = FFDEV_IC_SMOKE_ENDSIZE;
			m_hRocketTrail->m_SpawnRadius = FFDEV_IC_SMOKE_SPAWNRADIUS;
			m_hRocketTrail->m_MinSpeed = FFDEV_IC_SMOKE_MINSPEED;
			m_hRocketTrail->m_MaxSpeed = FFDEV_IC_SMOKE_MAXSPEED;
			m_hRocketTrail->m_flFlareScale = FFDEV_IC_FLARESCALE;
			
			m_hRocketTrail->SetLifetime(999);
			m_hRocketTrail->FollowEntity(this, "0");
		}
	}

	//----------------------------------------------------------------------------
	// Purpose: Spawn a rocket, set up model, size, etc
	//----------------------------------------------------------------------------
	void CFFProjectileIncendiaryRocket::Spawn()
	{
		// Setup
		SetModel(INCENDIARYROCKET_MODEL);
		SetMoveType(MOVETYPE_FLYGRAVITY);
		SetSize(Vector(-2, -2, -2), Vector(2, 2, 2)); // smaller, cube bounding box so we rest on the ground
		SetSolid(SOLID_BBOX);	// So it will collide with physics props!
		SetSolidFlags(FSOLID_NOT_STANDABLE);
		m_iDamageType = DMG_BURN;

		// Set the correct think & touch for the nail
		SetTouch(&CFFProjectileIncendiaryRocket::ExplodeTouch); // No we're going to explode when we touch something
		SetThink(&CFFProjectileIncendiaryRocket::ArcThink);

		// Next think
		SetNextThink(gpGlobals->curtime);

		// Creates the rocket trail
		CreateRocketTrail();

		BaseClass::Spawn();
	}

#endif

//----------------------------------------------------------------------------
// Purpose: Precache the rocket model
//----------------------------------------------------------------------------
void CFFProjectileIncendiaryRocket::Precache()
{
	PrecacheModel(INCENDIARYROCKET_MODEL);
	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Create a new rocket
//----------------------------------------------------------------------------
CFFProjectileIncendiaryRocket * CFFProjectileIncendiaryRocket::CreateRocket(const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iDamageRadius, const int iSpeed)
{
	CFFProjectileIncendiaryRocket *pRocket = (CFFProjectileIncendiaryRocket *) CreateEntityByName("ff_projectile_ic");

	UTIL_SetOrigin(pRocket, vecOrigin);
	pRocket->SetAbsAngles(angAngles);
	pRocket->Spawn();
	pRocket->SetOwnerEntity(pentOwner);
	pRocket->SetGravity(0.6f);
	pRocket->m_iSourceClassname = (pSource ? pSource->m_iClassname : NULL_STRING);

	Vector vecForward;
	AngleVectors(angAngles, &vecForward);

	// Set the speed and the initial transmitted velocity
	pRocket->SetAbsVelocity(vecForward * iSpeed);

#ifdef GAME_DLL
	pRocket->SetupInitialTransmittedVelocity(vecForward * iSpeed);
#endif

	pRocket->m_flDamage = iDamage;
	pRocket->m_DmgRadius = iDamageRadius;

	return pRocket; 
}

//-----------------------------------------------------------------------------
// Purpose: Orient the model to follow the arc
//-----------------------------------------------------------------------------
void CFFProjectileIncendiaryRocket::ArcThink()
{
	QAngle angDir;
	VectorAngles(GetAbsVelocity(), angDir);

	SetAbsAngles(angDir);

	SetNextThink(gpGlobals->curtime + 0.1f);
}