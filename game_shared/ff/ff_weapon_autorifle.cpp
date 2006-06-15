/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_autorifle.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF autorifle code.
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation logged
/// Jan 16, 2005 Mirv: Moved all repeated code to base class


#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponAutoRifle C_FFWeaponAutoRifle
	#include "c_ff_player.h"
#else
	#include "ff_player.h"
#endif

//=============================================================================
// CFFWeaponAutoRifle
//=============================================================================

class CFFWeaponAutoRifle : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponAutoRifle, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponAutoRifle();

	virtual void Fire();

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_AUTORIFLE; }

private:

	CFFWeaponAutoRifle(const CFFWeaponAutoRifle &);
};

//=============================================================================
// CFFWeaponAutoRifle tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponAutoRifle, DT_FFWeaponAutoRifle) 

BEGIN_NETWORK_TABLE(CFFWeaponAutoRifle, DT_FFWeaponAutoRifle) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponAutoRifle) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_autorifle, CFFWeaponAutoRifle);
PRECACHE_WEAPON_REGISTER(ff_weapon_autorifle);

//=============================================================================
// CFFWeaponAutoRifle implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponAutoRifle::CFFWeaponAutoRifle() 
{
}

//----------------------------------------------------------------------------
// Purpose: Fires a single bullet
//----------------------------------------------------------------------------
void CFFWeaponAutoRifle::Fire() 
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
