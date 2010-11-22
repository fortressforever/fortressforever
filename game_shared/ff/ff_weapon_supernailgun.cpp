/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_supernailgun.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 23, 2004
/// @brief The FF supernailgun code & class declaration
///
/// REVISIONS
/// ---------
/// Dec 23 2004 Mirv: First creation
/// Jan 16, 2005 Mirv: Moved all repeated code to base class


#include "cbase.h"
#include "ff_weapon_baseclip.h"
#include "ff_fx_shared.h"
#include "ff_projectile_nail.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponSuperNailgun C_FFWeaponSuperNailgun
	#include "c_ff_player.h"
#else
	#include "omnibot_interface.h"
	#include "ff_player.h"
#endif

//=============================================================================
// CFFWeaponSuperNailgun
//=============================================================================

class CFFWeaponSuperNailgun : public CFFWeaponBaseClip
{
public:
	DECLARE_CLASS(CFFWeaponSuperNailgun, CFFWeaponBaseClip);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponSuperNailgun();

	virtual void Fire();

	virtual FFWeaponID GetWeaponID() const { return FF_WEAPON_SUPERNAILGUN; }

private:

	CFFWeaponSuperNailgun(const CFFWeaponSuperNailgun &);

};

//=============================================================================
// CFFWeaponSuperNailgun tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponSuperNailgun, DT_FFWeaponSuperNailgun) 

BEGIN_NETWORK_TABLE(CFFWeaponSuperNailgun, DT_FFWeaponSuperNailgun) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponSuperNailgun) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_supernailgun, CFFWeaponSuperNailgun);
PRECACHE_WEAPON_REGISTER(ff_weapon_supernailgun);

//=============================================================================
// CFFWeaponSuperNailgun implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponSuperNailgun::CFFWeaponSuperNailgun() 
{
}

//----------------------------------------------------------------------------
// Purpose: Fire a nail
//----------------------------------------------------------------------------
void CFFWeaponSuperNailgun::Fire() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	Vector	vForward, vRight, vUp;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);

	Vector	vecSrc = pPlayer->Weapon_ShootPosition() + vForward * 8.0f + vRight * 4.0f + vUp * -5.0f;

	CFFProjectileNail *pNail = CFFProjectileNail::CreateNail(this, vecSrc, pPlayer->EyeAngles(), pPlayer, pWeaponInfo.m_iDamage, pWeaponInfo.m_iSpeed);
	pNail;

#ifdef GAME_DLL
	Omnibot::Notify_PlayerShoot(pPlayer, Omnibot::TF_WP_SUPERNAILGUN, pNail);
#endif

}
