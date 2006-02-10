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
	CNetworkVector(m_vInitialVelocity);

	virtual void Precache() 
	{
		PrecacheScriptSound("BaseGrenade.BounceSound");
	}

	// Bug #0000275: Grenade bounce sounds missing
	virtual void BounceSound()
	{
		if (gpGlobals->curtime > m_flNextBounceSoundTime)
		{
			EmitSound("BaseGrenade.BounceSound");

			m_flNextBounceSoundTime = gpGlobals->curtime + 0.1;
		}	
	}

	void Explode(trace_t *pTrace, int bitsDamageType);

#ifdef CLIENT_DLL
	CFFProjectileBase() {}
	CFFProjectileBase(const CFFProjectileBase&) {}
	
	// Add initial velocity into the interpolation history so that interp works okay
	virtual void PostDataUpdate(DataUpdateType_t type);
	virtual int DrawModel(int flags);

#else
	DECLARE_DATADESC();

	// Specify what velocity we want to have on the client immediately.
	// Without this, the entity wouldn't have an interpolation history initially, so it would
	// sit still until it had gotten a few updates from the server.
	void SetupInitialTransmittedVelocity(const Vector &velocity);
	int TakeEmp();

#endif

	virtual void Spawn();
    float m_flSpawnTime;
	float m_flNextBounceSoundTime;

protected:

private:	
	
};

#endif // FF_PROJECTILE_BASE_H
