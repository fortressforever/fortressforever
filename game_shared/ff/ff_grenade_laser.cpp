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
#include "ff_projectile_nail.h"
#include "effect_dispatch_data.h"
#include "IEffects.h"
#include "beam_shared.h"

#ifdef GAME_DLL
	#include "ff_entity_system.h"
	#include "te_effect_dispatch.h"
#else
	#include "c_te_effect_dispatch.h"
#endif

#define GRENADE_BEAM_SPRITE			"effects/bluelaser1.vmt"
#define NAILGRENADE_MODEL			"models/grenades/nailgren/nailgren.mdl"

#ifdef CLIENT_DLL
	#define CFFGrenadeLaser C_FFGrenadeLaser
#endif



#ifdef GAME_DLL

	ConVar laserdamage("ffdev_laserdamage", "10", FCVAR_CHEAT, "Damage tick of LG laser");
	ConVar lasertime("ffdev_lasertime", "3", FCVAR_CHEAT, "Laser activation time");
	ConVar laserangv("ffdev_laserangv", "7.5", FCVAR_CHEAT, "Laser angular velocity");
//	ConVar laserstreams( "ffdev_lasergren_streams", "2", FCVAR_CHEAT );

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

#ifdef CLIENT_DLL
	CFFGrenadeLaser() {}
	CFFGrenadeLaser(const CFFGrenadeLaser&) {}
#else
	DECLARE_DATADESC(); // Since we're adding new thinks etc
	virtual void Spawn();
	virtual void BeamEmit();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);
	virtual void DoDamage( CBaseEntity *pTarget );

protected:
	float	m_flAngleOffset;
	CBeam	*pBeamA, *pBeamB;

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
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(ff_grenade_laser, CFFGrenadeLaser);
PRECACHE_WEAPON_REGISTER(ff_grenade_laser);

#define NAIL_BBOX 2.0f

//-----------------------------------------------------------------------------
// Purpose: Various precache things
//-----------------------------------------------------------------------------
void CFFGrenadeLaser::Precache() 
{
	PrecacheModel(NAILGRENADE_MODEL);
	PrecacheModel(GRENADE_BEAM_SPRITE);
//	PrecacheScriptSound( "NailGrenade.shoot" );

	BaseClass::Precache();
}

#ifdef GAME_DLL

	//-----------------------------------------------------------------------------
	// Purpose: Various spawny flag things
	//-----------------------------------------------------------------------------
	void CFFGrenadeLaser::Spawn() 
	{
		SetModel(NAILGRENADE_MODEL);
		BaseClass::Spawn();

		m_flAngleOffset = 0.0f;
		SetLocalAngularVelocity(QAngle(0, 0, 0));
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

		SetDetonateTimerLength( lasertime.GetInt() );

		// Should this maybe be noclip?
		SetMoveType(MOVETYPE_FLY);

		pBeamA = CBeam::BeamCreate( GRENADE_BEAM_SPRITE, 0.5 );
		pBeamA->SetWidth( 1 );
		pBeamA->SetEndWidth( 0.05f );
		pBeamA->SetBrightness( 255 );
		pBeamA->SetColor( 255, 255, 255 );
//		pBeamA->RelinkBeam();
		pBeamA->LiveForTime( lasertime.GetFloat() );

		pBeamB = CBeam::BeamCreate( GRENADE_BEAM_SPRITE, 0.5 );
		pBeamB->SetWidth( 1 );
		pBeamB->SetEndWidth( 0.05f );
		pBeamB->SetBrightness( 255 );
		pBeamB->SetColor( 255, 255, 255 );
//		pBeamB->RelinkBeam();
		pBeamB->LiveForTime( lasertime.GetFloat() );

		// Go into nail mode
		SetThink(&CFFGrenadeLaser::BeamEmit);
		SetNextThink(gpGlobals->curtime);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Spin round emitting nails
	//-----------------------------------------------------------------------------
	void CFFGrenadeLaser::BeamEmit() 
	{
		// Blow up if we've reached the end of our fuse
		if (gpGlobals->curtime > m_flDetonateTime) 
		{
			Detonate();
			return;
		}

		float flRisingheight = 0;

		// Lasts for 3 seconds, rise for 0.3, but only if not handheld
		if (m_flDetonateTime - gpGlobals->curtime > 2.6 && !m_fIsHandheld)
			flRisingheight = 80;

		SetAbsVelocity(Vector(0, 0, flRisingheight + 20 * sin(DEG2RAD(GetAbsAngles().y))));
		SetAbsAngles(GetAbsAngles() + QAngle(0, laserangv.GetFloat(), 0));

		Vector vecOrigin = GetAbsOrigin();
		Vector vecDirection;
		AngleVectors(GetAbsAngles(), &vecDirection);
		VectorNormalizeFast(vecDirection);

		float flSize = 20.0f;

			//DrawBeam(vecOrigin + vecDirection * flSize, angRadial);
		trace_t trA, trB;
		UTIL_TraceLine(vecOrigin + vecDirection * flSize, vecOrigin + vecDirection * 4096.0f, MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER, &trA);
		UTIL_TraceLine(vecOrigin - vecDirection * flSize, vecOrigin - vecDirection * 4096.0f, MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER, &trB);

		pBeamA->PointsInit( vecOrigin, trA.endpos );
		pBeamB->PointsInit( vecOrigin, trB.endpos );

		if ( trA.m_pEnt )
			DoDamage( trA.m_pEnt );

		if ( trB.m_pEnt )
			DoDamage( trB.m_pEnt );

//		EmitSound("NailGrenade.shoot");
		
/*		CEffectData data;
		data.m_vOrigin = vecOrigin;
		data.m_vAngles = QAngle(0, 0, 0);

#ifdef GAME_DLL
			data.m_nEntIndex = entindex();
#else
			data.m_hEntity = this;
#endif
*/



		SetNextThink(gpGlobals->curtime);
	}

	void CFFGrenadeLaser::DoDamage( CBaseEntity *pTarget )
	{
		// only interested in players, dispensers & sentry guns
		if ( pTarget->IsPlayer() || pTarget->Classify() == CLASS_DISPENSER || pTarget->Classify() == CLASS_SENTRYGUN )
		{
			// If pTarget can take damage from nails...
			if ( g_pGameRules->FCanTakeDamage( pTarget, ToFFPlayer( GetOwnerEntity() ) ) )
			{
				if (pTarget->IsPlayer() )
				{
					CFFPlayer *pPlayerTarget = dynamic_cast< CFFPlayer* > ( pTarget );
					pPlayerTarget->TakeDamage( CTakeDamageInfo( this, ToFFPlayer( GetOwnerEntity() ), laserdamage.GetFloat(), DMG_ENERGYBEAM ) );
				}
				else if( FF_IsDispenser( pTarget ) )
				{
					CFFDispenser *pDispenser = FF_ToDispenser( pTarget );
					if( pDispenser )
						pDispenser->TakeDamage( CTakeDamageInfo( this, ToFFPlayer( GetOwnerEntity() ), laserdamage.GetFloat() + 2, DMG_ENERGYBEAM ) );
				}
				else /*if( FF_IsSentrygun( pTarget ) )*/
				{
					CFFSentryGun *pSentrygun = FF_ToSentrygun( pTarget );
					if( pSentrygun )
						pSentrygun->TakeDamage( CTakeDamageInfo( this, ToFFPlayer( GetOwnerEntity() ), laserdamage.GetFloat() + 2, DMG_ENERGYBEAM ) );
				}
			}
		}
	}


#endif


