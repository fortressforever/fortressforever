/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_rocket.h
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief Declaration of the class for rocket projectiles
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First created


#ifndef FF_PROJECTILE_ROCKET_H
#define FF_PROJECTILE_ROCKET_H
#ifdef _WIN32
#pragma once
#endif

#include "ff_projectile_base.h"

#ifdef CLIENT_DLL
	#define CFFProjectileRocket C_FFProjectileRocket
#endif

class RocketTrail;

//=============================================================================
// CFFProjectileRocket
//=============================================================================

class CFFProjectileRocket : public CFFProjectileBase
{
public:
	DECLARE_CLASS(CFFProjectileRocket, CFFProjectileBase);
	DECLARE_NETWORKCLASS();

public:

	virtual void Precache();
	static CFFProjectileRocket * CreateRocket(const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwner, const int iDamage, const int iSpeed);

#ifdef CLIENT_DLL
	CFFProjectileRocket() {}
	CFFProjectileRocket(const CFFProjectileRocket&) {}

	virtual const char *GetFlightSound() { return "rocket.fly"; }
	virtual void Simulate( void );

#else
	DECLARE_DATADESC()

	virtual void Spawn();

protected:

	// Creates the smoke trail
	void CreateSmokeTrail();

	CHandle<RocketTrail>	m_hRocketTrail;

private:	
	
#endif
};


#endif // FF_PROJECTILE_ROCKET_H
