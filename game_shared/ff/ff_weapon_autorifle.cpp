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
	#include "omnibot_interface.h"
	#include "ff_player.h"
#endif


// 1325: Dev variables for tweaking the autorifle
//ConVar ffdev_ar_recoil("ffdev_ar_recoil", "0.4", FCVAR_FF_FFDEV_REPLICATED, "Assault Rifle Recoil Amount");
//ConVar ffdev_ar_push("ffdev_ar_push", "1", FCVAR_FF_FFDEV_REPLICATED, "Assault Rifle Push Amount");
//ConVar ffdev_ar_damage("ffdev_ar_damage", "7", FCVAR_FF_FFDEV_REPLICATED, "Assault Rifle Damage");
//ConVar ffdev_ar_rof("ffdev_ar_rof", "0.1", FCVAR_FF_FFDEV_REPLICATED, "Assault Rifle Rate of Fire");
//ConVar ffdev_ar_bulletspread("ffdev_ar_bulletspread", "0.06", FCVAR_FF_FFDEV_REPLICATED, "Assault Rifle Bullet Spread");


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
	virtual void PrimaryAttack();

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


// 1325: Identical to the base class, except for the ConVars
void CFFWeaponAutoRifle::PrimaryAttack() 
{
	CANCEL_IF_BUILDING();
	CANCEL_IF_CLOAKED();

	// Only the player fires this way so we can cast
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	if (!pPlayer)
		return;

	// Undisguise
#ifdef GAME_DLL
	pPlayer->ResetDisguise();
#endif

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound(SINGLE);

	if (m_bMuzzleFlash)
		pPlayer->DoMuzzleFlash();

	SendWeaponAnim(GetPrimaryAttackActivity());

	// player "shoot" animation
	//pPlayer->SetAnimation(PLAYER_ATTACK1);
	pPlayer->DoAnimationEvent(PLAYERANIMEVENT_FIRE_GUN_PRIMARY);

#ifdef GAME_DLL
	int nShots = min(GetFFWpnData().m_iCycleDecrement, pPlayer->GetAmmoCount(m_iPrimaryAmmoType));
	pPlayer->RemoveAmmo(nShots, m_iPrimaryAmmoType);
#endif

	// Fire now
	Fire();

	pPlayer->m_flTrueAimTime = gpGlobals->curtime;

	// To use server .txt files uncomment this line
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFFWpnData().m_flCycleTime;
	// To use CVARs uncomment this line
	//m_flNextPrimaryAttack = gpGlobals->curtime + ffdev_ar_rof.GetFloat();

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}

	//Add our view kick in
	pPlayer->ViewPunch(QAngle(-GetFFWpnData().m_flRecoilAmount, 0, 0));
	//pPlayer->ViewPunch(QAngle(-ffdev_ar_recoil.GetFloat(), 0, 0));
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
	//info.m_iDamage = ffdev_ar_damage.GetInt();
	info.m_iDamage = pWeaponInfo.m_iDamage;
	//info.m_iTracerFreq = 0;

	// 1325: For tweaking autorifle
	//float flBulletSpread = ffdev_ar_bulletspread.GetFloat();
	//info.m_vecSpread = Vector( flBulletSpread, flBulletSpread, flBulletSpread );
	
	info.m_iTracerFreq = 1;
	//info.m_flDamageForceScale = ffdev_ar_push.GetFloat();

	pPlayer->FireBullets(info);

#ifdef GAME_DLL
	Omnibot::Notify_PlayerShoot(pPlayer, Omnibot::TF_WP_AUTORIFLE, 0);
#endif
}
