/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_supershotgun.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF supershotgun code & class declaration.
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation logged
/// Jan 16, 2005 Mirv: Moved all repeated code to base class


#include "cbase.h"
#include "ff_weapon_baseclip.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponSuperShotgun C_FFWeaponSuperShotgun
	#include "c_ff_player.h"
#else
	#include "ff_player.h"
#endif

//=============================================================================
// CFFWeaponSuperShotgun
//=============================================================================

class CFFWeaponSuperShotgun : public CFFWeaponBaseClip
{
public:
	DECLARE_CLASS(CFFWeaponSuperShotgun, CFFWeaponBaseClip);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponSuperShotgun();

	virtual void		Fire();
	virtual FFWeaponID	GetWeaponID() const	{ return FF_WEAPON_SUPERSHOTGUN; }

private:
	CFFWeaponSuperShotgun(const CFFWeaponSuperShotgun &);
};

//=============================================================================
// CFFWeaponSuperShotgun tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponSuperShotgun, DT_FFWeaponSuperShotgun) 

BEGIN_NETWORK_TABLE(CFFWeaponSuperShotgun, DT_FFWeaponSuperShotgun) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponSuperShotgun) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_supershotgun, CFFWeaponSuperShotgun);
PRECACHE_WEAPON_REGISTER(ff_weapon_supershotgun);

//=============================================================================
// CFFWeaponSuperShotgun implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponSuperShotgun::CFFWeaponSuperShotgun() 
{
}

//----------------------------------------------------------------------------
// Purpose: Fire shotgun pellets
//----------------------------------------------------------------------------
void CFFWeaponSuperShotgun::Fire() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward);

	FireBulletsInfo_t info(pWeaponInfo.m_iBullets, pPlayer->Weapon_ShootPosition(), vecForward, Vector(pWeaponInfo.m_flBulletSpread, pWeaponInfo.m_flBulletSpread, pWeaponInfo.m_flBulletSpread), MAX_TRACE_LENGTH, m_iPrimaryAmmoType);
	info.m_pAttacker = pPlayer;
	info.m_iDamage = pWeaponInfo.m_iDamage;
	info.m_iTracerFreq = 0;

	pPlayer->FireBullets(info);
}
