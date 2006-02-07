/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_dart.h
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief Declarion of the class for dart projectiles
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First created


#ifndef FF_PROJECTILE_DART_H
#define FF_PROJECTILE_DART_H
#ifdef _WIN32
#pragma once
#endif

#include "ff_projectile_base.h"

#ifdef CLIENT_DLL
	#define CFFProjectileDart C_FFProjectileDart
#endif

class DartTrail;

//=============================================================================
// CFFProjectileDart
//=============================================================================

class CFFProjectileDart : public CFFProjectileBase
{
public:
	DECLARE_CLASS(CFFProjectileDart, CFFProjectileBase);

public:

	virtual void Precache();
	void BubbleThink();
	void DartTouch(CBaseEntity *pOther);

	static CFFProjectileDart *CreateDart(const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iSpeed);

#ifdef CLIENT_DLL
	CFFProjectileDart() {}
	CFFProjectileDart(const CFFProjectileDart&) {}
#else
	DECLARE_DATADESC(); 
	virtual void Spawn();
#endif

protected:

private:	
	
};

#endif // FF_PROJECTILE_DART_H
