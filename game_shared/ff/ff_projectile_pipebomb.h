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

#define PIPEBOMB_TIME_TILL_LIVE		0.75f

// Pretty much the same as CGrenade only we precache a different model and
// we set ourself as not being live so we don't explode on contact with players.
// Also define a TryDetonate function to detonate ourselves.

// Actually having this as another class could be considered a bit silly.

//=============================================================================
// CFFProjectilePipebomb
//=============================================================================

class CFFProjectilePipebomb : public CFFProjectileGrenade
{
public:
	DECLARE_CLASS(CFFProjectilePipebomb, CFFProjectileGrenade);
	DECLARE_NETWORKCLASS(); 

	// Override precache because we want a different model
	virtual void Spawn();
	// Needs its own explode func for custom scorch mark
	// drawing
	virtual void Explode( trace_t *pTrace, int bitsDamageType );

	//-- Added by L0ki ------------------------------
	virtual Class_T Classify( void ) { return CLASS_PIPEBOMB; }
	//-----------------------------------------------

#ifdef CLIENT_DLL
	virtual int DrawModel(int flags);
#endif

public:

	// We have our own create function
	static CFFProjectilePipebomb * CreatePipebomb(const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iSpeed);
	static void CFFProjectilePipebomb::DestroyAllPipes(CBaseEntity *pOwner, bool force = false);

#ifdef GAME_DLL	
	void DetonatePipe(bool force = false, CBaseEntity *pOther = NULL);

	// Override projectile_base so object isn't removed
	int TakeEmp( void ) { return m_flDamage; } 

	DECLARE_DATADESC(); // Added anyway

#else
	bool fAltSkin;
#endif

	CHandle<CSprite>		m_pArmedSprite;
};


#endif // FF_PROJECTILE_PIPEBOMB_H
