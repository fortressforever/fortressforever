// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_weapon_sniperrifle.h
// @author Patrick O'Leary (Mulchman)
// @date May 2nd, 2007
// @brief The FF sniperrifle code .h file
//
// REVISIONS
// ---------
// May 02 2007:	Mulchman
//				Just adding the .h file

#ifndef FF_WEAPON_HOOKGUN_H
#define FF_WEAPON_HOOKGUN_H

#include "cbase.h"
#include "ff_weapon_baseclip.h"
#include "ff_projectile_hook.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponHookGun C_FFWeaponHookGun
	#include "c_ff_player.h"
#else
	#include "omnibot_interface.h"
	#include "ff_player.h"
#endif

//=============================================================================
// CFFWeaponHook
//=============================================================================

class CFFWeaponHookGun : public CFFWeaponBaseClip
{
public:
	DECLARE_CLASS(CFFWeaponHookGun, CFFWeaponBaseClip);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponHookGun();

	virtual void		Fire();
	virtual void		ItemPostFrame();
	virtual bool		SendWeaponAnim(int iActivity);
	virtual FFWeaponID	GetWeaponID() const	{ return FF_WEAPON_HOOKGUN; }

	bool	m_fStartedReloading;
	CFFProjectileHook *m_pHook;

private:
	CFFWeaponHookGun(const CFFWeaponHookGun &);
};

#endif // FF_WEAPON_SNIPERRIFLE_H