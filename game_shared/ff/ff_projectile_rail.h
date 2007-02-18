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
	static CFFProjectileRail *CreateRail( const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iSpeed, float flChargeTime );
	//virtual void CFFProjectileRail::Explode(trace_t *pTrace, int bitsDamageType);
	virtual Class_T Classify( void ) { return CLASS_RAIL_PROJECTILE; }

#ifdef CLIENT_DLL
	virtual void OnDataChanged(DataUpdateType_t type);
	virtual void ClientThink( void );

	bool m_bStart;
	Vector m_vecStart;
	bool m_bEnd;
	Vector m_vecEnd;
	bool m_bBounce1;
	Vector m_vecBounce1;
	bool m_bBounce2;
	Vector m_vecBounce2;

	CFFRailEffects *m_pRailEffects;
	float m_flLastDataChange;
#else
	DECLARE_DATADESC(); // Since we're adding new thinks etc
	virtual void Spawn( void );
	void DieThink( void );
	void RailThink( void );
	void RailTouch( CBaseEntity *pOther );

	// set transmit filter to transmit always
	int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

	Vector m_vecSameOriginCheck;
	float m_flSameOriginCheckTime;

	CNetworkVar( bool, m_bStart );
	CNetworkVector( m_vecStart );
	CNetworkVar( bool, m_bEnd );
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
