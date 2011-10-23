/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_pipebomb.h
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 24, 2004
/// @brief Declarion of the class for pipebombs
///
/// REVISIONS
/// ---------
/// Dec 24, 2004 Mirv: First created


#ifndef FF_PROJECTILE_PIPEBOMB_H
#define FF_PROJECTILE_PIPEBOMB_H
#ifdef _WIN32
#pragma once
#endif

#include "ff_projectile_grenade.h"
#include "Sprite.h"

#ifdef GAME_DLL
	#include "baseentity.h"
#endif

#ifdef CLIENT_DLL
	#define CFFProjectilePipebomb C_FFProjectilePipebomb
#endif

extern ConVar ffdev_pipe_friction;

//0001279: Commented out because of the ConVar created for the pipe det delay.
//#define PIPEBOMB_TIME_TILL_LIVE		0.75f

// Pretty much the same as CGrenade only we precache a different model and
// we set ourself as not being live so we don't explode on contact with players.
// Also define a TryDetonate function to detonate ourselves.

// Actually having this as another class could be considered a bit silly.

//=============================================================================
// CFFProjectilePipebomb
//=============================================================================

class CFFProjectilePipebomb : public CFFProjectileGrenade
{
private:
	//Control the time before the explosion is armed
	bool m_bArmed;

	//Control the time before the magnetism
	bool m_bMagnetArmed;

	//Bool to know whether this is magnetically active
	bool m_bMagnetActive;

	//Pipe's target when magnetized ( declaring as a base entity, but this will only be a player )
	CBaseEntity* m_pMagnetTarget;

	//Bool to tell this pipe it is detonated and waiting to blow up
	bool m_bShouldDetonate;

	//Pipe's detonate time in the future
	float m_flDetonateTime;

public:
	DECLARE_CLASS(CFFProjectilePipebomb, CFFProjectileGrenade);
	DECLARE_NETWORKCLASS(); 

	void PipebombThink();

	//Function that moves the pipe towards the target
	void Magnetize(CBaseEntity* _pTarget);

	//Function to start glowing a sprite on the pipe
	void StartGlowEffect();

	//Accessors and Mutators
	CBaseEntity* GetMagnetTarget(){ return m_pMagnetTarget; }
	void SetMagnetTarget( CBaseEntity* _pTarget){ m_pMagnetTarget = _pTarget; }
	
	bool GetShouldDetonate(){ return m_bShouldDetonate; }
	void SetShouldDetonate( bool _bShouldDet ){ m_bShouldDetonate = _bShouldDet; }
	
	float GetDetonateTime(){ return m_flDetonateTime; }
	void SetDetonateTime( float _flDetTime ){ m_flDetonateTime = _flDetTime; }

	bool GetArmed(){return m_bArmed; }
	void SetArmed(bool _bArmed){ m_bArmed = _bArmed; }

	bool GetMagnetArmed(){ return m_bMagnetArmed; }
	void SetMagnetArmed(bool _bMagnetArmed){ m_bMagnetArmed = _bMagnetArmed; }

	// Override precache because we want a different model
	virtual void Spawn();
	// Needs its own explode func for custom scorch mark
	// drawing
	virtual void Explode( trace_t *pTrace, int bitsDamageType );
	
	static inline const float GetGrenadeFriction() { return /*0.2f */ ffdev_pipe_friction.GetFloat(); }

	//-- Added by L0ki ------------------------------
	virtual Class_T Classify( void ) { return CLASS_PIPEBOMB; }
	//-----------------------------------------------

	void Precache( void );

	virtual bool ExplodeOnHitPlayer() { return false; }

	void DecrementHUDCount();

#ifdef CLIENT_DLL
	virtual int DrawModel(int flags);
#endif

public:

	// We have our own create function
	static CFFProjectilePipebomb *CreatePipebomb(const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iDamageRadius, const int iSpeed);
	static void CFFProjectilePipebomb::DestroyAllPipes(CBaseEntity *pOwner, bool force = false);

	//Custom collision to allow for constant elasticity on hit surfaces
	virtual void ResolveFlyCollisionCustom(trace_t &trace, Vector &vecVelocity);

	//Randomize the pipe velocities to not clump into a ball
	void RandomizePipeVelocity();

#ifdef GAME_DLL	
	void DetonatePipe(bool force = false, CBaseEntity *pOther = NULL);

	bool IsDetonated() { return m_bIsDetonated; }
	void SetDetonated( bool bIsDetonated ) { m_bIsDetonated = bIsDetonated; }
	bool m_bIsDetonated;

	// Override projectile_base so object isn't removed
	int TakeEmp( void ) { return m_flDamage; } 

	void CreateProjectileEffects();



	DECLARE_DATADESC(); // Added anyway

#else
	bool fAltSkin;
#endif

	CHandle<CSprite>		m_hMainGlow;
};


#endif // FF_PROJECTILE_PIPEBOMB_H
