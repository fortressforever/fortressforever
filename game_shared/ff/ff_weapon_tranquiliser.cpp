/// =============== Fortress Forever ==============
/// ======== A modifcation for Half-Life 2 ========
///
/// @file ff_weapon_tranquiliser.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF tranquiliser code & class declaration
///
/// REVISIONS
/// ---------
/// Dec 21 2004 Mirv: First logged
/// Jan 16, 2005 Mirv: Moved all repeated code to base class


#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"
#include "ff_projectile_dart.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponTranquiliser C_FFWeaponTranquiliser
	#include "c_ff_player.h"
#else
	#include "ff_player.h"
#endif

//=============================================================================
// CFFWeaponTranquiliser
//=============================================================================

class CFFWeaponTranquiliser : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponTranquiliser, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponTranquiliser();

	virtual void Fire();

	virtual FFWeaponID GetWeaponID() const { return FF_WEAPON_TRANQUILISER; }

private:

	CFFWeaponTranquiliser(const CFFWeaponTranquiliser &);

};

//=============================================================================
// CFFWeaponTranquiliser tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponTranquiliser, DT_FFWeaponTranquiliser) 

BEGIN_NETWORK_TABLE(CFFWeaponTranquiliser, DT_FFWeaponTranquiliser) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponTranquiliser) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_tranq, CFFWeaponTranquiliser);
PRECACHE_WEAPON_REGISTER(ff_weapon_tranq);

//=============================================================================
// CFFWeaponTranquiliser implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponTranquiliser::CFFWeaponTranquiliser() 
{
}

//----------------------------------------------------------------------------
// Purpose: Fire a dart
//----------------------------------------------------------------------------
void CFFWeaponTranquiliser::Fire() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();	

	Vector	vForward, vRight, vUp;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);

	Vector	vecSrc = pPlayer->Weapon_ShootPosition() + vForward * 8.0f + vRight * 8.0f + vUp * -8.0f;

	QAngle angAiming;
	VectorAngles(pPlayer->GetAutoaimVector(0), angAiming);

	CFFProjectileDart::CreateDart(this, vecSrc, angAiming, pPlayer, pWeaponInfo.m_iDamage, pWeaponInfo.m_iSpeed);
}
