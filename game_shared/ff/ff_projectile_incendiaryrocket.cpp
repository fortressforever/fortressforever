/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_rocket.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date August 3, 2005
/// @brief The FF incendiary rocket projectile code.
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation logged


#include "cbase.h"
#include "ff_utils.h"
#include "ff_projectile_incendiaryrocket.h"

#define INCENDIARYROCKET_MODEL "models/gibs/AGIBS.mdl"

#ifdef GAME_DLL
	#include "smoke_trail.h"
	#include "ff_buildableobjects_shared.h"
	#include "soundent.h"
#else

#endif

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
			if (pPlayer->IsBurning())
			{
				pPlayer->TakeDamage(CTakeDamageInfo( this, pBurninator, 40.0f, DMG_BURN ) );
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
	// Purpose: Creata a trail of smoke for the rocket
	//----------------------------------------------------------------------------
	void CFFProjectileIncendiaryRocket::CreateSmokeTrail()
	{
		// Smoke trail.
		if ((m_hRocketTrail = RocketTrail::CreateRocketTrail()) != NULL)
		{
			m_hRocketTrail->m_Opacity = 0.2f;
			m_hRocketTrail->m_SpawnRate = 100;
			m_hRocketTrail->m_ParticleLifetime = 0.5f;
			m_hRocketTrail->m_StartColor.Init(1.0f, 0.3f, 0.0f);
			m_hRocketTrail->m_EndColor.Init(0.0f, 0.0f, 0.0f);
			m_hRocketTrail->m_StartSize = 8;
			m_hRocketTrail->m_EndSize = 32;
			m_hRocketTrail->m_SpawnRadius = 4;
			m_hRocketTrail->m_MinSpeed = 2;
			m_hRocketTrail->m_MaxSpeed = 16;
			
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

		// Creates the smoke trail
		CreateSmokeTrail();

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