/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_nail.h
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief Declarion of the class for nail projectiles
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First created


#ifndef FF_PROJECTILE_NAIL_H
#define FF_PROJECTILE_NAIL_H
#ifdef _WIN32
#pragma once
#endif

#include "ff_projectile_base.h"

#ifdef CLIENT_DLL
	#define CFFProjectileNail C_FFProjectileNail
#endif

class NailTrail;

//=============================================================================
// CFFProjectileNail
//=============================================================================

class CFFProjectileNail : public CFFProjectileBase
{
public:
	DECLARE_CLASS(CFFProjectileNail, CFFProjectileBase);

public:

	virtual void Precache();
	void BubbleThink();
	void NailTouch(CBaseEntity *pOther);
	//int ShouldTransmit(const CCheckTransmitInfo *pInfo) { return FL_EDICT_DONTSEND; }

	static CFFProjectileNail *CreateNail(const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iSpeed);

#ifdef CLIENT_DLL
	CFFProjectileNail() {}
	CFFProjectileNail(const CFFProjectileNail&) {}

#else

	DECLARE_DATADESC(); // Since we're adding new thinks etc
	virtual void Spawn();

protected:

private:	
	
#endif
};


#endif // FF_PROJECTILE_NAIL_H
