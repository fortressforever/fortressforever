// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_weapon_tommygun.cpp
// @author Patrick O'Leary (Mulchman
// @date July 21, 2006
// @brief The FF Tommy Gun
//
// REVISIONS
// ---------
//	7/21/2006	Mulchman:
//		First version


#include "cbase.h"
#include "ff_weapon_base.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponTommygun C_FFWeaponTommygun
	#include "c_ff_player.h"
#else
	#include "ff_player.h"
#endif

//=============================================================================
// CFFWeaponTommygun
//=============================================================================

class CFFWeaponTommygun : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponTommygun, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponTommygun();

	virtual void Fire();

	virtual FFWeaponID GetWeaponID() const { return FF_WEAPON_TOMMYGUN; }
	const char *GetTracerType() { return "ACTracer"; }

private:

	CFFWeaponTommygun(const CFFWeaponTommygun &);

};

//=============================================================================
// CFFWeaponTommygun tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponTommygun, DT_FFWeaponTommygun) 

BEGIN_NETWORK_TABLE(CFFWeaponTommygun, DT_FFWeaponTommygun) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponTommygun) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_tommygun, CFFWeaponTommygun);
PRECACHE_WEAPON_REGISTER(ff_weapon_tommygun);

//=============================================================================
// CFFWeaponTommygun implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponTommygun::CFFWeaponTommygun() 
{
}

//----------------------------------------------------------------------------
// Purpose: Fire a bullet
//----------------------------------------------------------------------------
void CFFWeaponTommygun::Fire() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	if( !pPlayer )
		return;

	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward);

	FireBulletsInfo_t info(pWeaponInfo.m_iBullets, pPlayer->Weapon_ShootPosition(), vecForward, Vector(pWeaponInfo.m_flBulletSpread, pWeaponInfo.m_flBulletSpread, pWeaponInfo.m_flBulletSpread), MAX_TRACE_LENGTH, m_iPrimaryAmmoType);
	info.m_pAttacker = pPlayer;
	info.m_iDamage = pWeaponInfo.m_iDamage;

	pPlayer->FireBullets(info);
}
