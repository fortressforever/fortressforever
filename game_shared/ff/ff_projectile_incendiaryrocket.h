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
	#define CFFProjectileIncendiaryRocket C_FFProjectileIncendiaryRocket
	#include "dlight.h"
#endif

class RocketTrail;

//=============================================================================
// CFFProjectileIncendiaryRocket
//=============================================================================

class CFFProjectileIncendiaryRocket : public CFFProjectileBase
{
public:
	DECLARE_CLASS(CFFProjectileIncendiaryRocket, CFFProjectileBase);

#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

public:

	virtual void Precache();
	static CFFProjectileIncendiaryRocket *CreateRocket(const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iDamageRadius, const int iSpeed);
	virtual void CFFProjectileIncendiaryRocket::Explode(trace_t *pTrace, int bitsDamageType);
	virtual Class_T Classify( void ) { return CLASS_IC_ROCKET; }

	void ArcThink();
	virtual bool			CanClipOwnerEntity() const { return gpGlobals->curtime - m_flSpawnTime > 0.5; }

#ifdef CLIENT_DLL

#else

	virtual void Spawn();

protected:

	// Creates the smoke trail
	void CreateSmokeTrail();

	CHandle<RocketTrail>	m_hRocketTrail;

private:	
	
#endif
};


#endif // FF_PROJECTILE_ROCKET_H
