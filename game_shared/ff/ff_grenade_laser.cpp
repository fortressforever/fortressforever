/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_grenade_laser.cpp
/// @author David "Mervaka" Cook
/// @date March 18, 2010
/// @brief The FF laser grenade code.
///
/// REVISIONS
/// ---------
/// Mar 29, 2005	Mervaka: First created, based on ff_grenade_nail.cpp by Mirvin_Monkey

//========================================================================
// Required includes
//========================================================================
#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"
#include "effect_dispatch_data.h"
#include "IEffects.h"
#include "beam_shared.h"

#ifdef GAME_DLL
	#include "ff_entity_system.h"
	#include "te_effect_dispatch.h"
#else
	#include "c_te_effect_dispatch.h"
#endif

#define GRENADE_BEAM_SPRITE			"sprites/plasma.spr"
#define NAILGRENADE_MODEL			"models/grenades/nailgren/nailgren.mdl"

#ifdef CLIENT_DLL
	#define CFFGrenadeLaser C_FFGrenadeLaser
#endif

#define MAX_BEAMS 16

//ConVar laserbeams( "ffdev_lasergren_beams", "3", FCVAR_FF_FFDEV_REPLICATED, "Number of laser beams", true, 1, true, MAX_BEAMS);
#define LASERGREN_BEAMS 3
//ConVar laserdistance( "ffdev_lasergren_distance", "256", FCVAR_FF_FFDEV_REPLICATED, "Laser beam max radius",true, 0, true, 4096 );
#define LASERGREN_DISTANCE 256
//ConVar growTime( "ffdev_lasergren_growTime", "0.7", FCVAR_FF_FFDEV_REPLICATED, "Time taken to grow to full length" );
#define LASERGREN_GROWTIME 0.7f
//ConVar shrinkTime( "ffdev_lasergren_shrinkTime", "1", FCVAR_FF_FFDEV_REPLICATED, "Time taken to shrink to nothing" );
#define LASERGREN_SHRINKTIME 1
//ConVar lasertime("ffdev_lasergren_time", "10", FCVAR_FF_FFDEV_REPLICATED, "Laser active time");
#define LASERGREN_TIME 10

#ifdef CLIENT_DLL
	ConVar hud_lasergren_customColor_enable( "hud_lasergren_customColor_enable", "0", FCVAR_ARCHIVE, "Use custom laser colors (1 = use custom colour)");
	ConVar hud_lasergren_customColor_r( "hud_lasergren_customColor_r", "255", FCVAR_ARCHIVE, "Custom laser color - Red Component (0-255)");
	ConVar hud_lasergren_customColor_g( "hud_lasergren_customColor_g", "128", FCVAR_ARCHIVE, "Custom laser color - Green Component(0-255)");
	ConVar hud_lasergren_customColor_b( "hud_lasergren_customColor_b", "255", FCVAR_ARCHIVE, "Custom laser color - Blue Component(0-255)");
	ConVar ffdev_lasergren_widthcreate("ffdev_lasergren_widthcreate", "0.5", FCVAR_FF_FFDEV_REPLICATED, "Width given in the constructor; not used");
	ConVar ffdev_lasergren_widthstart("ffdev_lasergren_widthstart", "8", FCVAR_FF_FFDEV_REPLICATED, "Width at the start of the beam");
	ConVar ffdev_lasergren_widthend("ffdev_lasergren_widthend", "7", FCVAR_FF_FFDEV_REPLICATED, "Width at the end of the beam");
	#define LASERGREN_WIDTHCREATE ffdev_lasergren_widthcreate.GetFloat()
	#define LASERGREN_WIDTHSTART ffdev_lasergren_widthstart.GetFloat()
	#define LASERGREN_WIDTHEND ffdev_lasergren_widthend.GetFloat()
#endif

#ifdef GAME_DLL
	//ConVar laserdamage("ffdev_lasergren_damage", "660", FCVAR_FF_FFDEV, "Damage of laser");
	#define LASERGREN_DAMAGE_PER_TICK 660.0f*gpGlobals->interval_per_tick
	//ConVar laserdamage_buildablemult("ffdev_lasergren_damage_buildablemult", "0.53", FCVAR_FF_FFDEV, "Damage multiplier of laser against buildables");
	#define LASERGREN_DAMAGE_BUILDABLEMULT 0.53f
	//ConVar laserangv("ffdev_lasergren_angv", "120", FCVAR_FF_FFDEV, "Laser angular increment");
	#define LASERGREN_ROTATION_PER_TICK 120.0f*gpGlobals->interval_per_tick
	//ConVar laserjump( "ffdev_lasergren_jump", "80", FCVAR_FF_FFDEV, "Laser grenade jump distance" );
	#define LASERGREN_JUMP 80
	//ConVar laserbob( "ffdev_lasergren_bob", "10", FCVAR_FF_FFDEV, "Laser grenade bob factor" );
	#define LASERGREN_BOB 10
	//ConVar laserbeamtime( "ffdev_lasergren_beamtime", "0.0", FCVAR_FF_FFDEV, "Laser grenade update time" );
	#define LASERGREN_BEAMTIME 0.0f
	//ConVar laserradius( "ffdev_lasergren_laserradius", "4.0", FCVAR_CHEAT, "Laser grenade laser radius" );
	#define LASERGREN_LASERRADIUS 4.0f
	//ConVar usenails( "ffdev_lasergren_usenails", "0", FCVAR_FF_FFDEV, "Use nails instead of lasers" );
	#define LASERGREN_USENAILS 0
	//ConVar bobfrequency( "ffdev_lasergren_bobfreq", "0.5", FCVAR_FF_FFDEV, "Bob Frequency");
	#define LASERGREN_BOBFREQ 0.5f
	//ConVar laserexplode("ffdev_lasergren_explode", "0", FCVAR_FF_FFDEV, "Explosion at end of active time");
	#define LASERGREN_EXPLODE 0
	//ConVar explosiondamage("ffdev_lasergren_explosiondamage", "90", FCVAR_FF_FFDEV, "Explosion damage at end of active period" );
	#define LASERGREN_EXPLOSIONDAMAGE 90.0f
	//ConVar explosionradius("ffdev_lasergren_explosionradius", "180", FCVAR_FF_FFDEV, "Explosion radius at end of active period" );
	#define LASERGREN_EXPLOSIONRADIUS 180.0f
#endif

class CFFGrenadeLaser : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeLaser, CFFGrenadeBase) 
	DECLARE_NETWORKCLASS(); 

	virtual void Precache();
	virtual const char *GetBounceSound() { return "NailGrenade.Bounce"; }

	virtual Class_T Classify( void ) { return CLASS_GREN_LASER; } 

	virtual color32 GetColour() { color32 col = { 128, 225, 255, GREN_ALPHA_DEFAULT }; return col; }

	float getLengthPercent();
	bool m_bPlayingDeploySound;

#ifdef CLIENT_DLL
	CFFGrenadeLaser() {}
	CFFGrenadeLaser(const CFFGrenadeLaser&) {}
	virtual void ClientThink();
	virtual void OnDataChanged(DataUpdateType_t updateType);
	virtual void UpdateOnRemove( void );
protected:
	CBeam		*pBeam[MAX_BEAMS];
	IMaterial	*m_pMaterial;

public:
#else
	DECLARE_DATADESC(); // Since we're adding new thinks etc
	virtual void Spawn();
	virtual void BeamEmit();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);
	virtual void DoDamage( CBaseEntity *pTarget );

	virtual float GetGrenadeDamage()		{ return LASERGREN_EXPLOSIONDAMAGE; }
	virtual float GetGrenadeRadius()		{ return LASERGREN_EXPLOSIONRADIUS; }

protected:
	float	m_flBeams;
	float	m_flNailSpit;
	float	m_flAngleOffset;
	int		m_iOffset;
	float	m_flLastThinkTime;

#endif
};

#ifdef GAME_DLL
	BEGIN_DATADESC(CFFGrenadeLaser) 
		DEFINE_THINKFUNC(BeamEmit),
	END_DATADESC() 
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeLaser, DT_FFGrenadeLaser)

BEGIN_NETWORK_TABLE(CFFGrenadeLaser, DT_FFGrenadeLaser)
#ifdef CLIENT_DLL
	RecvPropFloat(RECVINFO(m_flDetonateTime)), 
#else
	SendPropFloat(SENDINFO(m_flDetonateTime)),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(ff_grenade_laser, CFFGrenadeLaser);
PRECACHE_WEAPON_REGISTER(ff_grenade_laser);

//-----------------------------------------------------------------------------
// Purpose: Various precache things
//-----------------------------------------------------------------------------
void CFFGrenadeLaser::Precache() 
{
	PrecacheModel(NAILGRENADE_MODEL);
	PrecacheModel(GRENADE_BEAM_SPRITE);
	PrecacheScriptSound( "NailGrenade.shoot" );
	PrecacheScriptSound( "NailGrenade.LaserLoop" );
	PrecacheScriptSound( "NailGrenade.LaserDeploy" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: To calc % length for laser grow/shrink
//-----------------------------------------------------------------------------
float CFFGrenadeLaser::getLengthPercent()
{
	float spawnTime = m_flDetonateTime - LASERGREN_TIME;
	float growTillTime = spawnTime + LASERGREN_GROWTIME;
	float startShrinkTime = m_flDetonateTime - LASERGREN_SHRINKTIME;

	if(gpGlobals->curtime > growTillTime && gpGlobals->curtime < startShrinkTime && m_flDetonateTime > 0)
	{
		if ( m_bPlayingDeploySound )
		{
			StopSound("NailGrenade.LaserDeploy");
			m_bPlayingDeploySound = 0;
		}
		return 1.0f;
	}
	else if(gpGlobals->curtime >= startShrinkTime && m_flDetonateTime > 0)
	{
		if ( !m_bPlayingDeploySound )
		{
			EmitSound("NailGrenade.LaserDeploy");
			m_bPlayingDeploySound = 1;
		}
		return 1 - (gpGlobals->curtime - startShrinkTime) / (m_flDetonateTime - startShrinkTime);
	}
	else if(gpGlobals->curtime < growTillTime && m_flDetonateTime > 0)
	{
		if ( !m_bPlayingDeploySound )
		{
			EmitSound("NailGrenade.LaserDeploy");
			m_bPlayingDeploySound = 1;
		}
		return (gpGlobals->curtime - spawnTime) / (growTillTime - spawnTime);
	}
	else
		return 0.0f;
}

#ifdef GAME_DLL

	//-----------------------------------------------------------------------------
	// Purpose: Various spawny flag things
	//-----------------------------------------------------------------------------
	void CFFGrenadeLaser::Spawn() 
	{
		SetModel(NAILGRENADE_MODEL);
		BaseClass::Spawn();
		m_bIsOn = false;
		m_flAngleOffset = 0.0f;
		m_iOffset = 0;
		SetLocalAngularVelocity(QAngle(0, 0, 0));

		m_bPlayingDeploySound = 0;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Instead of exploding, change to 
	//-----------------------------------------------------------------------------
	void CFFGrenadeLaser::Explode(trace_t *pTrace, int bitsDamageType)
	{
		// Clumsy, will do for now
		if (GetMoveType() == MOVETYPE_FLY)
		{
			BaseClass::Explode(pTrace, bitsDamageType);
			return;
		}

		SetDetonateTimerLength( LASERGREN_TIME );

		// Should this maybe be noclip?
		SetMoveType(MOVETYPE_FLY);
	
		m_bIsOn = true;
		SetThink(&CFFGrenadeLaser::BeamEmit);
		
		SetNextThink(gpGlobals->curtime);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Spin round emitting lasers
	//-----------------------------------------------------------------------------
	void CFFGrenadeLaser::BeamEmit() 
	{
		// Blow up if we've reached the end of our fuse
		if (gpGlobals->curtime > m_flDetonateTime) 
		{
			if( LASERGREN_EXPLODE )
				Detonate();
			else
			{
				g_pEffects->EnergySplash(GetAbsOrigin(), Vector(0, 0, 1.0f), true);
				UTIL_Remove( this );
			}
			return;
		}

		float flRisingheight = 0;

		// Lasts for 3 seconds, rise for 0.3, but only if not handheld
		if (m_flDetonateTime - gpGlobals->curtime > LASERGREN_TIME - 0.3 && !m_fIsHandheld)
			flRisingheight = LASERGREN_JUMP;

		SetAbsVelocity(Vector(0, 0, flRisingheight + LASERGREN_BOB * sin(DEG2RAD(gpGlobals->curtime * 360 * LASERGREN_BOBFREQ))));
		SetAbsAngles(GetAbsAngles() + QAngle(0, LASERGREN_ROTATION_PER_TICK, 0));

		Vector vecOrigin = GetAbsOrigin();

		//float flSize = 20.0f;
		trace_t tr;
		char i;

		float flDeltaAngle = 360.0f / LASERGREN_BEAMS;

		CBaseEntity *pEntity = NULL;
		for (CEntitySphereQuery sphere(vecOrigin, LASERGREN_DISTANCE * getLengthPercent() + LASERGREN_LASERRADIUS); (pEntity = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity()) 
		{
			if (!pEntity)
				continue;

			if (pEntity->m_takedamage == DAMAGE_NO) 
				continue;

			// we don't care about weapons, rockets, or projectiles
			if (pEntity->GetCollisionGroup() == COLLISION_GROUP_WEAPON
				|| pEntity->GetCollisionGroup() == COLLISION_GROUP_ROCKET
				|| pEntity->GetCollisionGroup() == COLLISION_GROUP_PROJECTILE)
				continue;

			if (pEntity->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer(pEntity);

				if( !pPlayer->IsAlive() || pPlayer->IsObserver() )
					continue;
			}
			if (FF_IsBuildableObject(pEntity))
			{
				// Is this a buildable of some sort
				CFFBuildableObject *pBuildable = FF_ToBuildableObject(pEntity);

				// Skip objects that are building
				if(pBuildable && !pBuildable->IsBuilt())
					continue;
			}

			Vector vecMin = pEntity->WorldAlignMins();
			Vector vecMax = pEntity->WorldAlignMaxs();

			// check if the entity is below all the lasers
			if (pEntity->GetAbsOrigin().z + vecMax.z < vecOrigin.z - LASERGREN_LASERRADIUS)
				continue;

			// check if the entity is above all the lasers
			if (pEntity->GetAbsOrigin().z + vecMin.z > vecOrigin.z + LASERGREN_LASERRADIUS)
				continue;

			Vector vecDirection;
			QAngle angRadial = GetAbsAngles();
			// check each laser
			for( i = 0; i < LASERGREN_BEAMS; i++ )
			{
				AngleVectors(angRadial, &vecDirection);
				VectorNormalizeFast(vecDirection);

				Vector vecToEnt = pEntity->GetAbsOrigin() - vecOrigin;

				float dot = DotProduct( vecDirection, vecToEnt );

				// player is behind the laser
				if (dot < 0)
				{
					angRadial.y += flDeltaAngle;
					continue;
				}

				// don't allow dividing by zero
				if (getLengthPercent() == 0.0f)
				{
					angRadial.y += flDeltaAngle;
					continue;
				}
				Vector vecLaser = vecDirection * LASERGREN_DISTANCE * getLengthPercent();
				float ratio = DotProduct( vecToEnt, vecLaser ) / DotProduct( vecLaser, vecLaser );
				Vector vecLaserClosestPoint = vecOrigin + (ratio * vecLaser);
				
				Vector point;
				pEntity->CollisionProp()->CalcNearestPoint( vecLaserClosestPoint, &point );
				Vector vecDistFromLaser = point - vecLaserClosestPoint;
				vecDistFromLaser.z = 0;

				//NDebugOverlay::Box(vecLaserClosestPoint, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 0, 255, 127, 4);
				//NDebugOverlay::Box(point, Vector(-2, -2, -2), Vector(2, 2, 2), 255, 0, 0, 127, 4);

				float dist = vecDistFromLaser.Length();

				// outside of the laser radius
				if (dist > LASERGREN_LASERRADIUS)
				{
					angRadial.y += flDeltaAngle;
					continue;
				}

				//make sure theres nothing in the way
				trace_t tr;
				UTIL_TraceLine( vecOrigin, 
								point, 
								MASK_SHOT, this, COLLISION_GROUP_PLAYER, &tr );
				
				if (!tr.m_pEnt || tr.m_pEnt == pEntity)
					DoDamage( pEntity );

				angRadial.y += flDeltaAngle;
			}
		}

		SetNextThink( gpGlobals->curtime );
	}

	void CFFGrenadeLaser::DoDamage( CBaseEntity *pTarget )
	{
		if (!pTarget)
			return;

		// only interested in players, dispensers & sentry guns
		if ( pTarget->IsPlayer() || pTarget->Classify() == CLASS_DISPENSER || pTarget->Classify() == CLASS_SENTRYGUN || pTarget->Classify() == CLASS_MANCANNON )
		{
			// If pTarget can take damage from nails...
			if ( g_pGameRules->FCanTakeDamage( pTarget, ToFFPlayer( GetOwnerEntity() ) ) )
			{
				if (pTarget->IsPlayer() )
				{
					CFFPlayer *pPlayerTarget = ToFFPlayer( pTarget );
					if (!pPlayerTarget)
						return;
					
					pPlayerTarget->TakeDamage( CTakeDamageInfo( this, ToFFPlayer( GetOwnerEntity() ), LASERGREN_DAMAGE_PER_TICK, DMG_ENERGYBEAM ) );
				}
				else if( FF_IsDispenser( pTarget ) )
				{
					CFFDispenser *pDispenser = FF_ToDispenser( pTarget );
					if( pDispenser )
						pDispenser->TakeDamage( CTakeDamageInfo( this, ToFFPlayer( GetOwnerEntity() ), LASERGREN_DAMAGE_PER_TICK * LASERGREN_DAMAGE_BUILDABLEMULT, DMG_ENERGYBEAM ) );
				}
				else if( FF_IsSentrygun( pTarget ) )
				{
					CFFSentryGun *pSentrygun = FF_ToSentrygun( pTarget );
					if( pSentrygun )
						pSentrygun->TakeDamage( CTakeDamageInfo( this, ToFFPlayer( GetOwnerEntity() ), LASERGREN_DAMAGE_PER_TICK * LASERGREN_DAMAGE_BUILDABLEMULT, DMG_ENERGYBEAM ) );
				}
				else /*if( FF_IsManCannon( pTarget ) )*/
				{
					CFFManCannon *pManCannon = FF_ToManCannon( pTarget );
					if( pManCannon )
						pManCannon->TakeDamage( CTakeDamageInfo( this, ToFFPlayer( GetOwnerEntity() ), LASERGREN_DAMAGE_PER_TICK * LASERGREN_DAMAGE_BUILDABLEMULT, DMG_ENERGYBEAM ) );
				}
			}
		}
	}

#else
	void CFFGrenadeLaser::UpdateOnRemove( void )
	{
		StopSound("NailGrenade.LaserLoop");
		StopSound("NailGrenade.LaserDeploy");

		int i;
		for( i = 0; i < LASERGREN_BEAMS; i++ )
		{
			if( pBeam[i] )
			{
				delete pBeam[i];
			}
		}

		BaseClass::UpdateOnRemove();
	}
	//-----------------------------------------------------------------------------
	// Purpose: Emit gas.
	//-----------------------------------------------------------------------------
	void CFFGrenadeLaser::ClientThink()
	{
		if ( m_bIsOn )
		{
			EmitSound("NailGrenade.LaserLoop");

			Vector vecDirection;
			Vector vecOrigin = GetAbsOrigin();
			QAngle angRadial = GetAbsAngles();

			float flSize = 20.0f;
			trace_t tr;
			char i;

			CFFPlayer *pgrenOwner = ToFFPlayer( this->GetOwnerEntity() );

			if (!pgrenOwner)
				return;

			float flDeltaAngle = 360.0f / LASERGREN_BEAMS;
		
			for( i = 0; i < LASERGREN_BEAMS; i++ )
			{
				AngleVectors(angRadial, &vecDirection);
				VectorNormalizeFast(vecDirection);

				UTIL_TraceLine( vecOrigin + vecDirection * flSize, 
								vecOrigin + vecDirection * LASERGREN_DISTANCE * getLengthPercent(), 
								MASK_SHOT, this, COLLISION_GROUP_PLAYER, &tr );
				
				if( !pBeam[i] )
				{
					pBeam[i] = CBeam::BeamCreate( GRENADE_BEAM_SPRITE, LASERGREN_WIDTHCREATE );
					if (!pBeam[i])
						continue;

					pBeam[i]->SetWidth( LASERGREN_WIDTHSTART );
					pBeam[i]->SetEndWidth( LASERGREN_WIDTHEND );
					pBeam[i]->LiveForTime( 1  );
					pBeam[i]->SetBrightness( 255 );
					if(hud_lasergren_customColor_enable.GetBool() == true)
						pBeam[i]->SetColor( hud_lasergren_customColor_r.GetInt(), hud_lasergren_customColor_g.GetInt(), hud_lasergren_customColor_b.GetInt() );
					else if(pgrenOwner->GetTeamNumber() == TEAM_RED)
						pBeam[i]->SetColor( 255, 64, 64 );
					else if(pgrenOwner->GetTeamNumber() == TEAM_BLUE)
						pBeam[i]->SetColor( 64, 128, 255 );
					else if(pgrenOwner->GetTeamNumber() == TEAM_GREEN)
						pBeam[i]->SetColor( 153, 255, 153 );
					else if(pgrenOwner->GetTeamNumber() == TEAM_YELLOW)
						pBeam[i]->SetColor( 255, 178, 0 );
					else // just in case
						pBeam[i]->SetColor( 204, 204, 204 );
				}
				pBeam[i]->PointsInit( vecOrigin, tr.endpos );

				angRadial.y += flDeltaAngle;

				/*
				g_pEffects->Sparks(tr.endpos);

				//UTIL_DecalTrace( &tr, "LaserBurn" );

				if(tr.fraction != 1)
					g_pEffects->Smoke(tr.endpos, -1, 6, -1);
				*/

				if ( tr.fraction == 1.0f )
					g_pEffects->MetalSparks( tr.endpos, vecDirection );
				else
					g_pEffects->MetalSparks( tr.endpos, -vecDirection );

			}
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Called when data changes on the server
	//-----------------------------------------------------------------------------
	void CFFGrenadeLaser::OnDataChanged(DataUpdateType_t updateType)
	{
		// NOTE: We MUST call the base classes' implementation of this function
//		BaseClass::OnDataChanged(updateType);

		// Setup our entity's particle system on creation
		if (updateType == DATA_UPDATE_CREATED)
		{
			m_pMaterial = materials->FindMaterial("effects/blueblacklargebeam", TEXTURE_GROUP_CLIENT_EFFECTS);
			m_pMaterial->IncrementReferenceCount();

			// Call our ClientThink() function once every client frame
			SetNextClientThink(CLIENT_THINK_ALWAYS);
		}
	}

#endif


