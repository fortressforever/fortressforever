/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life2 ========
///
/// @file ff_weapon_baseclip.h
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date June 18, 2006
/// @brief Base for clip-based weapons

#ifndef FF_WEAPON_BASECLIP_H
#define FF_WEAPON_BASECLIP_H

#include "cbase.h"
#include "ff_weapon_base.h"

#ifdef CLIENT_DLL
	#define CFFWeaponBaseClip	C_FFWeaponBaseClip
#endif

class CFFWeaponBaseClip : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponBaseClip, CFFWeaponBase);

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

private:

#ifdef CLIENT_DLL
	float		m_flNextAutoReload;
#endif

public:
	virtual	bool StartReload();
	virtual bool Reload();
	virtual void FillClip();
	virtual void FinishReload();
	virtual void ItemPostFrame();
	virtual void PrimaryAttack();
	virtual void DryFire();
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);

	CFFWeaponBaseClip();

	float m_flReloadTime;

	// This is here for the pipe/gren launcher really
	bool	m_fIsSwitching;

private:
	CFFWeaponBaseClip(const CFFWeaponBaseClip &);
};

#endif