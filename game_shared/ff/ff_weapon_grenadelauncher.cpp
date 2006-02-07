/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_grenadelauncher.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF grenade launcher code & class declaration.
///
/// REVISIONS
/// ---------
/// Dec 24, 2004 Mirv: First created
/// Jan 16, 2005 Mirv: Moved all repeated code to base class


#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"
#include "ff_projectile_grenade.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponGrenadeLauncher C_FFWeaponGrenadeLauncher
	#include "c_ff_player.h"
#else
	#include "ff_player.h"
#endif

//=============================================================================
// CFFWeaponGrenadeLauncher
//=============================================================================

class CFFWeaponGrenadeLauncher : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponGrenadeLauncher, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponGrenadeLauncher();

	virtual void Fire();
	virtual bool Reload();
	virtual bool SendWeaponAnim(int iActivity);

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_GRENADELAUNCHER; }

private:

	CFFWeaponGrenadeLauncher(const CFFWeaponGrenadeLauncher &);
};

//=============================================================================
// CFFWeaponGrenadeLauncher tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponGrenadeLauncher, DT_FFWeaponGrenadeLauncher) 

BEGIN_NETWORK_TABLE(CFFWeaponGrenadeLauncher, DT_FFWeaponGrenadeLauncher) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponGrenadeLauncher) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_grenadelauncher, CFFWeaponGrenadeLauncher);
PRECACHE_WEAPON_REGISTER(ff_weapon_grenadelauncher);

//=============================================================================
// CFFWeaponGrenadeLauncher implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponGrenadeLauncher::CFFWeaponGrenadeLauncher() 
{
}

//----------------------------------------------------------------------------
// Purpose: Fires a grenade
//----------------------------------------------------------------------------
void CFFWeaponGrenadeLauncher::Fire() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	Vector	vForward, vRight, vUp;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);

	Vector	vecSrc = pPlayer->Weapon_ShootPosition() + vForward * 8.0f + vRight * 8.0f + vUp * -8.0f;

	QAngle angAiming;
	VectorAngles(pPlayer->GetAutoaimVector(0), angAiming);

	angAiming -= QAngle(12.0f, 0, 0);

	CFFProjectileGrenade::CreateGrenade(vecSrc, angAiming, pPlayer, pWeaponInfo.m_iDamage, pWeaponInfo.m_iSpeed);

	// We share ammo with the pipelauncher!
	// We could probably just do GetWeapon(2) 
	for (int i = 0; i < MAX_WEAPONS; i++) 
	{
		CFFWeaponBase *w = dynamic_cast<CFFWeaponBase *> (pPlayer->GetWeapon(i));

		if (w && w->GetWeaponID() == FF_WEAPON_PIPELAUNCHER) 
			w->m_iClip1 = m_iClip1;
	}
}

//----------------------------------------------------------------------------
// Purpose: Keep ammo acounts the same
//----------------------------------------------------------------------------
bool CFFWeaponGrenadeLauncher::Reload() 
{
	bool b = BaseClass::Reload();

	CFFPlayer *pPlayer = GetPlayerOwner();

	// We share ammo with the pipelauncher!
	// We could probably just do GetWeapon(2) 
	for (int i = 0; i < MAX_WEAPONS; i++) 
	{
		CFFWeaponBase *w = dynamic_cast<CFFWeaponBase *> (pPlayer->GetWeapon(i));

		if (w && w->GetWeaponID() == FF_WEAPON_PIPELAUNCHER) 
			w->m_iClip1 = m_iClip1;
	}

	return b;
}

//----------------------------------------------------------------------------
// Purpose: Override animations
//----------------------------------------------------------------------------
bool CFFWeaponGrenadeLauncher::SendWeaponAnim(int iActivity) 
{
	// If we have some unexpected clip amount, escape quick
	if (m_iClip1 < 0 || m_iClip1 > 6) 
		return BaseClass::SendWeaponAnim(iActivity);

	// Override the animation with a specific one
	switch (iActivity) 
	{
	case ACT_VM_DRAW:
		iActivity = ACT_VM_DRAW_WITH0 + m_iClip1;
		break;

	case ACT_VM_IDLE:
		iActivity = ACT_VM_IDLE_WITH0 + m_iClip1;
		break;

	case ACT_VM_PRIMARYATTACK:
		iActivity = ACT_VM_PRIMARYATTACK_1TO0 + (m_iClip1 - 1);
		break;

	case ACT_VM_RELOAD:
		iActivity = ACT_VM_RELOAD_0TO1 + m_iClip1;
		break;

	case ACT_SHOTGUN_RELOAD_START:
		iActivity = ACT_VM_STARTRELOAD_WITH0 + m_iClip1;
		break;

	case ACT_SHOTGUN_RELOAD_FINISH:
		iActivity = ACT_VM_FINISHRELOAD_WITH1 + (m_iClip1 - 1);
		break;
	}

	return BaseClass::SendWeaponAnim(iActivity);
}
