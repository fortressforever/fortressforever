/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_base.h
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief All projectiles derived from here; takes advantage of base_grenade code
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation


#ifndef FF_PROJECTILE_BASE_H
#define FF_PROJECTILE_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "basegrenade_shared.h"

#ifdef CLIENT_DLL
	#define CFFProjectileBase C_FFProjectileBase
#endif

//=============================================================================
// CFFProjectileBase
//=============================================================================

class CFFProjectileBase : public CBaseGrenade
{
public:
	DECLARE_CLASS(CFFProjectileBase, CBaseGrenade);
	DECLARE_NETWORKCLASS(); 

public:

	// This gets sent to the client and placed in the client's interpolation history
	// so the projectile starts out moving right off the bat.
	CNetworkVector(m_vecInitialVelocity);

	virtual void Precache();
	virtual void Spawn();
	virtual void BounceSound();

	virtual const char *GetBounceSound() { return "BaseGrenade.BounceSound"; }
	virtual const char *GetFlightSound() { return NULL; }

	float m_flSpawnTime;
	float m_flNextBounceSoundTime;

#ifdef CLIENT_DLL

	CFFProjectileBase();
	
	// Add initial velocity into the interpolation history so that interp works okay
	virtual void PostDataUpdate(DataUpdateType_t type);
	virtual void OnDataChanged(DataUpdateType_t type);

	virtual int	 DrawModel(int flags);

	// Both to catch the end of this projectile's life
	virtual void SetDormant(bool bDormant);
	virtual void Release();

	virtual void CleanUp();

	// This entity is drawing its past position thanks to interpolation
	bool IsDrawingHistory();

	// No shadows for projectiles
	virtual ShadowType_t ShadowCastType() { return SHADOWS_NONE; }

private:
	// Flag to keep track of whether projectile needs a cleanup
	bool	m_bNeedsCleanup;
	bool	m_bInPresent;

	// Position that this projectile first started
	Vector	m_vecStartOrigin;

#else
	DECLARE_DATADESC();

	// Specify what velocity we want to have on the client immediately.
	// Without this, the entity wouldn't have an interpolation history initially, so it would
	// sit still until it had gotten a few updates from the server.
	void SetupInitialTransmittedVelocity(const Vector &velocity);
	int TakeEmp();

#endif

protected:

private:	
	
};

#endif // FF_PROJECTILE_BASE_H
