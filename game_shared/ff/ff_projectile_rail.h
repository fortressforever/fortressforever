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
	#include "c_ff_rail_effects.h"
	#define CFFProjectileRail C_FFProjectileRail
#endif

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
	virtual void UpdateOnRemove();
	static CFFProjectileRail *CreateRail( const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iSpeed, float flChargeTime );
	//virtual void CFFProjectileRail::Explode(trace_t *pTrace, int bitsDamageType);
	virtual Class_T Classify( void ) { return CLASS_RAIL_PROJECTILE; }

#ifdef CLIENT_DLL
	virtual void OnDataChanged(DataUpdateType_t type);
	virtual void ClientThink( void );

	CFFRailEffects *m_pRailEffects;
	bool m_bShouldInit;
	Vector m_vecStart;

	// networked variables...
	Vector m_vecEnd;
	bool m_bBounce1;
	Vector m_vecBounce1;
	bool m_bBounce2;
	Vector m_vecBounce2;
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

	// networked variables...
	CNetworkVector( m_vecEnd );
	CNetworkVar( bool, m_bBounce1 );
	CNetworkVector( m_vecBounce1 );
	CNetworkVar( bool, m_bBounce2 );
	CNetworkVector( m_vecBounce2 );
#endif

	int m_iNumBounces;
	int m_iMaxBounces;

private:
};

#endif // FF_PROJECTILE_RAIL_H
