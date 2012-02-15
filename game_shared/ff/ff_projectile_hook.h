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


#ifndef FF_PROJECTILE_HOOK_H
#define FF_PROJECTILE_HOOK_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "ff_projectile_base.h"

#ifdef GAME_DLL
	#include "rope.h"
#endif

#include "rope_shared.h"

//#include "ff_gamerules.h"

#ifdef CLIENT_DLL
	#define CFFProjectileHook C_FFProjectileHook
	//#define RocketTrail C_RocketTrail
#endif

//class RocketTrail;


//=============================================================================
// CFFProjectileRocket
//=============================================================================

class CFFProjectileHook : public CFFProjectileBase
{
public:
	DECLARE_CLASS(CFFProjectileHook, CFFProjectileBase);
	DECLARE_NETWORKCLASS();

public:

	virtual void Precache();
	void HookThink();
	void HookTouch(CBaseEntity *pOther);
#ifdef CLIENT_DLL
	virtual Class_T Classify() { return CLASS_HOOK; }
#endif
	static CFFProjectileHook *CreateHook(CBaseEntity *pOwnerGun, const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwner);
	void RemoveHook( void );
	virtual void Spawn();

	bool bHooked;
	bool bBeenNotJumping;
	float flLastSwingSpeed;
	Vector vecFirstPullDirXY;
	CBaseEntity * m_pOwnerGun;

#ifdef GAME_DLL
	CHandle<CRopeKeyframe>		m_hRope;
	float m_fNextSparkTime;
	CBaseEntity * m_pAttachedEntity;
#endif

	//void CreateSmokeTrail();
	CFFProjectileHook();
	CFFProjectileHook(const CFFProjectileHook&) { CFFProjectileHook(); }

#ifdef CLIENT_DLL

	virtual void OnDataChanged(DataUpdateType_t type);
	virtual void ClientThink( void );

	virtual const char *GetFlightSound() { return "rocket.fly"; }
	virtual void CleanUp();
	virtual bool ShouldPredict();

private:

#else
	DECLARE_DATADESC()
	
#endif

	//CNetworkHandle(RocketTrail, m_hRocketTrail);
};


#endif // FF_PROJECTILE_HOOK_H
