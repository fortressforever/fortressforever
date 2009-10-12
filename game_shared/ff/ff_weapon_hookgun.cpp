/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_hookgun.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF Hook launcher code & class declaration.
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation logged
/// Jan 16, 2005 Mirv: Moved all repeated code to base class


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

// ConVar rpg_damage_radius( "ffdev_rpg_damage_radius", "115", FCVAR_REPLICATED | FCVAR_CHEAT, "RPG explosion radius" );
//#define RPG_DAMAGERADIUS 115.0f // rpg_damage_radius.GetFloat()

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
	virtual bool		SendWeaponAnim(int iActivity);
	virtual FFWeaponID	GetWeaponID() const	{ return FF_WEAPON_HOOKGUN; }

	bool	m_fStartedReloading;

private:
	CFFWeaponHookGun(const CFFWeaponHookGun &);
};

//=============================================================================
// CFFWeaponHook tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponHookGun, DT_FFWeaponHookGun) 

BEGIN_NETWORK_TABLE(CFFWeaponHookGun, DT_FFWeaponHookGun) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponHookGun) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_hookgun, CFFWeaponHookGun);
PRECACHE_WEAPON_REGISTER(ff_weapon_hookgun);

//=============================================================================
// CFFWeaponHook implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponHookGun::CFFWeaponHookGun() 
{
	m_fStartedReloading = false;
}

//----------------------------------------------------------------------------
// Purpose: Fire a rocket
//----------------------------------------------------------------------------
void CFFWeaponHookGun::Fire() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	Vector	vForward, vRight, vUp;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);

	//Vector	vecSrc = pPlayer->Weapon_ShootPosition() + vForward * 8.0f + vRight * 8.0f + vUp * -8.0f;
	Vector vecSrc = pPlayer->GetLegacyAbsOrigin() + vForward * 16.0f + vRight * 8.0f + Vector(0, 1, (pPlayer->GetFlags() & FL_DUCKING) ? 5.0f : 23.0f);

	CFFProjectileHook *pHook = CFFProjectileHook::CreateHook(vecSrc, pPlayer->EyeAngles(), pPlayer);
	pHook;

#ifdef GAME_DLL
	Omnibot::Notify_PlayerShoot(pPlayer, Omnibot::TF_WP_HOOKGUN, pHook);
#endif
}

//----------------------------------------------------------------------------
// Purpose: Override animations
//----------------------------------------------------------------------------
bool CFFWeaponHookGun::SendWeaponAnim(int iActivity) 
{
	// If we have some unexpected clip amount, escape quick
	if (m_iClip1 < 0 || m_iClip1 > 6) 
		return BaseClass::SendWeaponAnim(iActivity);

	CFFPlayer *pPlayer = GetPlayerOwner();

	// Override the animation with a specific one
	switch (iActivity) 
	{
	case ACT_VM_DRAW:
		iActivity = ACT_VM_DRAW_WITH0 + m_iClip1;
		break;

	case ACT_VM_IDLE:
		if (pPlayer->m_flIdleTime + 10.0f < gpGlobals->curtime && random->RandomInt(0, 10) == 0)
			iActivity = ACT_VM_DEEPIDLE_WITH0 + m_iClip1;
		else
			iActivity = ACT_VM_IDLE_WITH0 + m_iClip1;
		break;

	case ACT_VM_PRIMARYATTACK:
		iActivity = ACT_VM_PRIMARYATTACK_1TO0 + (m_iClip1 - 1);
		break;

	case ACT_VM_RELOAD:
		if (m_fStartedReloading)
		{
			iActivity = ACT_VM_INITRELOAD_0TO1 + m_iClip1;
			m_fStartedReloading = false;
		}
		else
			iActivity = ACT_VM_RELOAD_0TO1 + m_iClip1;
		break;

	case ACT_SHOTGUN_RELOAD_START:
		iActivity = ACT_VM_STARTRELOAD_WITH0 + m_iClip1;
		m_fStartedReloading = true;
		break;

	case ACT_SHOTGUN_RELOAD_FINISH:
		iActivity = ACT_VM_FINISHRELOAD_WITH1 + (m_iClip1 - 1);
		break;
	}

	return BaseClass::SendWeaponAnim(iActivity);
}
