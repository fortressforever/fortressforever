/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_nailgun.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF nailgun code & class declaration
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First logged
/// Jan 16, 2005 Mirv: Moved all repeated code to base class


#include "cbase.h"
#include "ff_weapon_baseclip.h"
#include "ff_fx_shared.h"
#include "ff_projectile_nail.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponNailgun C_FFWeaponNailgun
	#include "c_ff_player.h"
#else
	#include "omnibot_interface.h"
	#include "ff_player.h"
#endif

//=============================================================================
// CFFWeaponNailgun
//=============================================================================

class CFFWeaponNailgun : public CFFWeaponBaseClip
{
public:
	DECLARE_CLASS(CFFWeaponNailgun, CFFWeaponBaseClip);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponNailgun();

	virtual void Fire();

	virtual FFWeaponID GetWeaponID() const { return FF_WEAPON_NAILGUN; }

private:

	CFFWeaponNailgun(const CFFWeaponNailgun &);

};

//=============================================================================
// CFFWeaponNailgun tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponNailgun, DT_FFWeaponNailgun) 

BEGIN_NETWORK_TABLE(CFFWeaponNailgun, DT_FFWeaponNailgun) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponNailgun) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_nailgun, CFFWeaponNailgun);
PRECACHE_WEAPON_REGISTER(ff_weapon_nailgun);

//=============================================================================
// CFFWeaponNailgun implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponNailgun::CFFWeaponNailgun() 
{
}

//----------------------------------------------------------------------------
// Purpose: Fire a nail
//----------------------------------------------------------------------------
void CFFWeaponNailgun::Fire() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	Vector	vForward, vRight, vUp;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);

	Vector	vecSrc = pPlayer->Weapon_ShootPosition() + vForward * 8.0f + vRight * 8.0f + vUp * -8.0f;

	CFFProjectileNail *pNail = CFFProjectileNail::CreateNail(this, vecSrc, pPlayer->EyeAngles(), pPlayer, pWeaponInfo.m_iDamage, pWeaponInfo.m_iSpeed);
	pNail;

#ifdef GAME_DLL
	Omnibot::Notify_PlayerShoot(pPlayer, Omnibot::TF_WP_NAILGUN, pNail);
#endif

}
