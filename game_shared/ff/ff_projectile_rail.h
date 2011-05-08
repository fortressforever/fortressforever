/// =============== Fortress Forever ==============
/// ======== A modifcation for Half-Life 2 ========
///
/// @file ff_projectile_rail.h
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief Declaration of the class for rail projectiles
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First created
//
//	11/11/2006 Mulchman: Tweaking a bit

#ifndef FF_PROJECTILE_RAIL_H
#define FF_PROJECTILE_RAIL_H
#ifdef _WIN32
#pragma once
#endif

#include "ff_projectile_base.h"
#include "Sprite.h"
#include "SpriteTrail.h"

#ifdef CLIENT_DLL
	#define CFFProjectileRail C_FFProjectileRail
	#include "c_world.h"
	#include "c_te_effect_dispatch.h"
	#include "particles_simple.h"
	#include "dlight.h"
#else
	#include "te_effect_dispatch.h"
#endif

//ConVar ffdev_railgun_maxchargetime( "ffdev_railgun_maxchargetime", "2.0", FCVAR_FF_FFDEV_REPLICATED, "Maximum charge for railgun" );
#define RAILGUN_MAXCHARGETIME 2.0f // ffdev_railgun_maxchargetime.GetFloat()

//=============================================================================
// CFFProjectileRail
//=============================================================================
class CFFProjectileRail : public CFFProjectileBase
{
public:
	DECLARE_CLASS( CFFProjectileRail, CFFProjectileBase );
	DECLARE_NETWORKCLASS();

	CFFProjectileRail();

	virtual void Precache( void );
	static CFFProjectileRail *CreateRail( const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iDamageRadius, const int iSpeed, float flChargeTime );
	//virtual void CFFProjectileRail::Explode(trace_t *pTrace, int bitsDamageType);
	virtual Class_T Classify( void ) { return CLASS_RAIL_PROJECTILE; }

#ifdef CLIENT_DLL

	virtual void OnDataChanged(DataUpdateType_t type);
	virtual void ClientThink( void );

	//virtual const char *GetFlightSound() { return "Rail.Fly"; }

	CSmartPtr<CSimpleEmitter> m_hEmitter; // particle emitter
	PMaterialHandle m_hMaterial; // material handle for the particles
	TimedEvent m_tParticleTimer; // Timer used to control particle emission rate

	int m_iNumBounces;

	dlight_t *m_pDLight; // dynamic light attached to the rail

#else

	DECLARE_DATADESC(); // Since we're adding new thinks etc
	virtual void Spawn( void );
	void SetupEnd( Vector end );
	void DieThink( void );
	void RailThink( void );
	void RailTouch( CBaseEntity *pOther );

	// set transmit filter to always transmit
	int UpdateTransmitState() { return SetTransmitState( FL_EDICT_PVSCHECK ); }

	Vector m_vecSameOriginCheck;
	float m_flSameOriginCheckTimer;

	CNetworkVar( int, m_iNumBounces );

	CHandle<CSprite>	m_pGlow;
	CHandle<CSpriteTrail>	m_pTrail;

#endif

	int m_iMaxBounces;

private:
};

#endif // FF_PROJECTILE_RAIL_H
