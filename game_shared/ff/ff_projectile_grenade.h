/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_grenade.h
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief Declarion of the class for grenade projectiles
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First created


#ifndef FF_PROJECTILE_GRENADE_H
#define FF_PROJECTILE_GRENADE_H
#ifdef _WIN32
#pragma once
#endif

#include "ff_projectile_base.h"
#include "SpriteTrail.h"

#ifdef CLIENT_DLL
	#define CFFProjectileGrenade C_FFProjectileGrenade
#endif

extern ConVar projectile_gren_friction;
extern ConVar projectile_gren_elasticity;
extern ConVar projectile_gren_gravity;

//=============================================================================
// CFFProjectileGrenade
//=============================================================================

class CFFProjectileGrenade : public CFFProjectileBase
{
public:
	DECLARE_CLASS(CFFProjectileGrenade, CFFProjectileBase);
	DECLARE_NETWORKCLASS(); 

public:

	virtual void Precache();
	void GrenadeThink();

	static inline float GetGrenadeGravity() { return /*0.7f */ projectile_gren_gravity.GetFloat(); }
	static inline const float GetGrenadeFriction() { return /*0.2f */ projectile_gren_friction.GetFloat(); }
	static inline const float GetGrenadeElasticity() { return /*0.45f */ projectile_gren_elasticity.GetFloat(); }

	virtual float GetShakeAmplitude() { return 2.5f; }

	static CFFProjectileGrenade *CreateGrenade(const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iDamageRadius, const int iSpeed);

	virtual const char *GetBounceSound() { return "GrenadeProjectile.Bounce"; }
	virtual Class_T Classify() { return CLASS_GLGRENADE; }

	virtual bool ExplodeOnHitPlayer() { return true; }

#ifdef CLIENT_DLL
	CFFProjectileGrenade() {}
	CFFProjectileGrenade(const CFFProjectileGrenade&) {}

#else

	DECLARE_DATADESC(); // Since we're adding new thinks etc

	virtual void Spawn();	
protected:

	// Creates the smoke trail
	virtual void CreateProjectileEffects();
	void SetDetonateTimerLength(float timer);

	CHandle<CSpriteTrail>	m_hGlowTrail;

	//Custom collision to allow for constant elasticity on hit surfaces
	virtual void ResolveFlyCollisionCustom(trace_t &trace, Vector &vecVelocity);
	
	float m_flDetonateTime;
	int m_nNumBounces;

#endif
};


#endif // FF_PROJECTILE_GRENADE_H
