/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_shotgun.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF single barrelled shotgun code & class declaration.
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation logged
/// Jan 16, 2005 Mirv: Moved all repeated code to base class


#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponShotgun C_FFWeaponShotgun
	#include "c_ff_player.h"
#else
	#include "ff_player.h"
	#include "te_firebullets.h"
#endif

//=============================================================================
// CFFWeaponShotgun
//=============================================================================

class CFFWeaponShotgun : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponShotgun, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponShotgun();

	virtual void Fire();

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_SHOTGUN; }

private:

	CFFWeaponShotgun(const CFFWeaponShotgun &);

};

//=============================================================================
// CFFWeaponShotgun tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponShotgun, DT_FFWeaponShotgun) 

BEGIN_NETWORK_TABLE(CFFWeaponShotgun, DT_FFWeaponShotgun) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponShotgun) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_shotgun, CFFWeaponShotgun);
PRECACHE_WEAPON_REGISTER(ff_weapon_shotgun);

//=============================================================================
// CFFWeaponShotgun implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponShotgun::CFFWeaponShotgun() 
{
}

//----------------------------------------------------------------------------
// Purpose: Fire shotgun pellets
//----------------------------------------------------------------------------
void CFFWeaponShotgun::Fire() 
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
