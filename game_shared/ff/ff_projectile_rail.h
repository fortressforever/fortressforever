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
#endif

//class RailTrail;

//=============================================================================
// CFFProjectileRail
//=============================================================================
class CFFProjectileRail : public CFFProjectileBase
{
public:
	DECLARE_CLASS( CFFProjectileRail, CFFProjectileBase );
	DECLARE_NETWORKCLASS();

public:
	virtual void Precache( void );
	void BubbleThink( void );
	void RailTouch( CBaseEntity *pOther );

	static CFFProjectileRail *CreateRail( const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iSpeed, float flChargeTime );

#ifdef CLIENT_DLL
	CFFProjectileRail( void ) { m_iNumBounces = 0; }
	CFFProjectileRail( const CFFProjectileRail& ) {}
#else
	DECLARE_DATADESC(); // Since we're adding new thinks etc
	virtual void Spawn( void );

private:
	CHandle< CSprite >		m_pMainGlow;
	CHandle<CSpriteTrail >	m_pGlowTrail;	
#endif

	int				m_iNumBounces;
	int				m_iMaxBounces;
};

#endif // FF_PROJECTILE_RAIL_H
