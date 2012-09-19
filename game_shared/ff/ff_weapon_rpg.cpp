/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_rpg.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF Rocket launcher code & class declaration.
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation logged
/// Jan 16, 2005 Mirv: Moved all repeated code to base class


#include "cbase.h"
#include "ff_weapon_baseclip.h"
#include "ff_projectile_rocket.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponRPG C_FFWeaponRPG
	#include "c_ff_player.h"
#else
	#include "omnibot_interface.h"
	#include "ff_player.h"
#endif

ConVar rpg_damage_radius( "ffdev_rpg_damage_radius", "108", FCVAR_REPLICATED, "RPG explosion radius" );
#define RPG_DAMAGERADIUS rpg_damage_radius.GetFloat() //115.0f

ConVar rpg_speed( "ffdev_rpg_speed", "1000", FCVAR_REPLICATED, "RPG explosion radius" );
#define RPG_SPEED rpg_speed.GetFloat() //1000

ConVar rpg_spawnpos_forward( "ffdev_rpg_spawnpos_forward", "16", FCVAR_REPLICATED, "RPG spawn position in the forward direction" );
#define RPG_SPAWNPOS_FORWARD rpg_spawnpos_forward.GetFloat() //16.0f

//=============================================================================
// CFFWeaponRPG
//=============================================================================

class CFFWeaponRPG : public CFFWeaponBaseClip
{
public:
	DECLARE_CLASS(CFFWeaponRPG, CFFWeaponBaseClip);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponRPG();

	virtual void		Fire();
	virtual bool		SendWeaponAnim(int iActivity);
	virtual FFWeaponID	GetWeaponID() const	{ return FF_WEAPON_RPG; }

	bool	m_fStartedReloading;

private:
	CFFWeaponRPG(const CFFWeaponRPG &);
};

//=============================================================================
// CFFWeaponRPG tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponRPG, DT_FFWeaponRPG) 

BEGIN_NETWORK_TABLE(CFFWeaponRPG, DT_FFWeaponRPG) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponRPG) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_rpg, CFFWeaponRPG);
PRECACHE_WEAPON_REGISTER(ff_weapon_rpg);

//=============================================================================
// CFFWeaponRPG implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponRPG::CFFWeaponRPG() 
{
	m_fStartedReloading = false;
}

//----------------------------------------------------------------------------
// Purpose: Fire a rocket
//----------------------------------------------------------------------------
void CFFWeaponRPG::Fire() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	Vector	vForward, vRight, vUp;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);

	//Vector	vecSrc = pPlayer->Weapon_ShootPosition() + vForward * 8.0f + vRight * 8.0f + vUp * -8.0f;
	Vector vecSrc = pPlayer->GetAbsOrigin() + vForward * RPG_SPAWNPOS_FORWARD + vRight * 8.0f + Vector(0, 1, (pPlayer->GetFlags() & FL_DUCKING) ? 5.0f : 23.0f);

	CFFProjectileRocket *pRocket = CFFProjectileRocket::CreateRocket(this, vecSrc, pPlayer->EyeAngles(), pPlayer, pWeaponInfo.m_iDamage, RPG_DAMAGERADIUS/*pWeaponInfo.m_iDamageRadius*/, RPG_SPEED);
	pRocket;

#ifdef GAME_DLL
	Omnibot::Notify_PlayerShoot(pPlayer, Omnibot::TF_WP_ROCKET_LAUNCHER, pRocket);
#endif
}

//----------------------------------------------------------------------------
// Purpose: Override animations
//----------------------------------------------------------------------------
bool CFFWeaponRPG::SendWeaponAnim(int iActivity) 
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
