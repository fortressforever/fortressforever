/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_goop.h
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief Declarion of the class for goop projectiles
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First created


#ifndef FF_PROJECTILE_GOOP_H
#define FF_PROJECTILE_GOOP_H
#ifdef _WIN32
#pragma once
#endif

#include "ff_projectile_base.h"
#include "SpriteTrail.h"

#ifdef CLIENT_DLL
	#include "c_world.h"
	#include "c_te_effect_dispatch.h"
	#include "particles_simple.h"
	#include "dlight.h"
#else
	#include "te_effect_dispatch.h"
#endif

#ifdef CLIENT_DLL
	#define CFFProjectileGoop C_FFProjectileGoop
#endif

extern ConVar ffdev_goop_friction;
extern ConVar ffdev_goop_elasticity;
extern ConVar ffdev_goop_gravity;

enum GoopImpacts
{
	GOOP_IMPACT_HEAL = 0,
	GOOP_IMPACT_DAMAGE,
	GOOP_IMPACT_WORLD,

	GOOP_IMPACT_INVALID,
};

//=============================================================================
// CFFProjectileGoop
//=============================================================================

class CFFProjectileGoop : public CFFProjectileBase
{
public:
	DECLARE_CLASS(CFFProjectileGoop, CFFProjectileBase);
	DECLARE_NETWORKCLASS(); 

public:

	virtual void Precache();

	void GoopThink();
	virtual void ExplodeTouch(CBaseEntity *pOther);

	static inline float GetGoopGravity() { return /*0.7f */ ffdev_goop_gravity.GetFloat(); }
	static inline const float GetGoopFriction() { return /*0.2f */ ffdev_goop_friction.GetFloat(); }
	static inline const float GetGoopElasticity() { return /*0.45f */ ffdev_goop_elasticity.GetFloat(); }

	virtual float GetShakeAmplitude() { return 2.5f; }

	static CFFProjectileGoop *CreateGoop(const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iDamageRadius, const int iSpeed);

	virtual const char *GetBounceSound() { return "GoopProjectile.Bounce"; }
	virtual Class_T Classify() { return CLASS_GOOP; }
	
	int UpdateTransmitState() { return FL_EDICT_DONTSEND; }

#ifdef CLIENT_DLL
	CFFProjectileGoop() {}
	CFFProjectileGoop(const CFFProjectileGoop&) {}

#else

	DECLARE_DATADESC(); // Since we're adding new thinks etc

	virtual void Spawn();	
protected:
	
	virtual void CreateProjectileEffects();

	CHandle<CSpriteTrail>	m_hGlowTrail;

#endif
};


#endif // FF_PROJECTILE_GOOP_H
