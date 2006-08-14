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

class RailTrail;

//=============================================================================
// CFFProjectileRail
//=============================================================================

class CFFProjectileRail : public CFFProjectileBase
{
public:
	DECLARE_CLASS(CFFProjectileRail, CFFProjectileBase);
	DECLARE_NETWORKCLASS();

public:

	virtual void Precache();
	void BubbleThink();
	void RailTouch(CBaseEntity *pOther);

	static CFFProjectileRail *CreateRail(const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iSpeed);

#ifdef CLIENT_DLL
	CFFProjectileRail() {}
	CFFProjectileRail(const CFFProjectileRail&) {}

#else

	DECLARE_DATADESC(); // Since we're adding new thinks etc
	virtual void Spawn();

protected:

private:	

	CHandle<CSprite>		m_pMainGlow;
	CHandle<CSpriteTrail>	m_pGlowTrail;
	
#endif

	int				m_iNumBounces;

};


#endif // FF_PROJECTILE_RAIL_H
