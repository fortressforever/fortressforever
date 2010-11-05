/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life2 ========
///
/// @file ff_weapon_baseclip.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date June 18, 2006
/// @brief Base for clip-based weapons

#include "cbase.h"
#include "ff_weapon_baseclip.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
	#include "c_ff_player.h"

	ConVar auto_reload("cl_autoreload", "1", FCVAR_ARCHIVE, "Automatic weapon reload");
#else
	#include "ff_player.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponBaseClip, DT_FFWeaponBaseClip)

BEGIN_NETWORK_TABLE(CFFWeaponBaseClip, DT_FFWeaponBaseClip)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CFFWeaponBaseClip)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_ff_baseclip, CFFWeaponBaseClip);

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CFFWeaponBaseClip::StartReload()
{
	CBaseCombatCharacter *pOwner  = GetOwner();

#ifdef CLIENT_DLL	
	engine->ClientCmd("-reload");
#endif

	if (pOwner == NULL)
		return false;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) < GetFFWpnData().m_iCycleDecrement)
		return false;

	if (m_iClip1 >= GetMaxClip1())
		return false;

	// Don't allow reload instantly after fire
	if (m_flNextPrimaryAttack > gpGlobals->curtime)
		return false;

	SendWeaponAnim(ACT_SHOTGUN_RELOAD_START);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flTimeWeaponIdle = gpGlobals->curtime + GetFFWpnData().m_flPreReloadTime;

	m_bInReload = true;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CFFWeaponBaseClip::Reload()
{
	// Check that StartReload was called first
	if (!m_bInReload)
	{
		Warning("ERROR: Shotgun Reload called incorrectly!\n");
	}

	CBaseCombatCharacter *pOwner  = GetOwner();

	if (pOwner == NULL)
		return false;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) < GetFFWpnData().m_iCycleDecrement)
		return false;

	if (m_iClip1 >= GetMaxClip1())
		return false;

	// Defer clip filling until later
	//FillClip();

	// Play reload on different channel as otherwise steals channel away from fire sound
	if( ( GetWeaponID() == FF_WEAPON_SHOTGUN ) || ( GetWeaponID() == FF_WEAPON_SUPERSHOTGUN ) )
		WeaponSound( RELOAD );
	else
		WeaponSoundLocal( RELOAD );
	SendWeaponAnim(ACT_VM_RELOAD);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flTimeWeaponIdle = gpGlobals->curtime + GetFFWpnData().m_flReloadTime;

	// Using secondary attack for now in order to defer actual addition of ammo voogru: NOOOOOO!!!!!!!!!!!!!!11111111111oneone
	//m_flNextSecondaryAttack = gpGlobals->curtime + GetFFWpnData().m_flReloadTime;
	m_flReloadTime = gpGlobals->curtime + GetFFWpnData().m_flReloadTime;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CFFWeaponBaseClip::FinishReload()
{
	CBaseCombatCharacter *pOwner  = GetOwner();

	if (pOwner == NULL)
		return;

	m_bInReload = false;

	// Finish reload animation
	SendWeaponAnim(ACT_SHOTGUN_RELOAD_FINISH);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flTimeWeaponIdle = gpGlobals->curtime + GetFFWpnData().m_flPostReloadTime * 0.7f;
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CFFWeaponBaseClip::FillClip()
{
	CBaseCombatCharacter *pOwner  = GetOwner();

	if (pOwner == NULL)
		return;

	int nAdd = GetFFWpnData().m_iCycleDecrement;

	// Add them to the clip
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) >= nAdd)
	{
		if (Clip1() < GetMaxClip1())
		{
			m_iClip1 += nAdd;
#ifdef GAME_DLL
			pOwner->RemoveAmmo(nAdd, m_iPrimaryAmmoType);
#endif
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CFFWeaponBaseClip::DryFire()
{
	WeaponSound(EMPTY);
	SendWeaponAnim(ACT_VM_DRYFIRE);

	m_flTimeWeaponIdle = gpGlobals->curtime + 0.2f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// To do stuff
//
//-----------------------------------------------------------------------------
bool CFFWeaponBaseClip::Holster(CBaseCombatWeapon *pSwitchingTo) 
{
	m_flReloadTime = -1.0;
	return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CFFWeaponBaseClip::PrimaryAttack()
{
	CANCEL_IF_BUILDING();
	CANCEL_IF_CLOAKED();

	// Only the player fires this way so we can cast
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	if (!pPlayer)
		return;

#ifdef GAME_DLL
	pPlayer->ResetDisguise();
#endif

	// No longer reloading (cancel any deferred ammo too)
	m_bInReload = false;
	m_flReloadTime = -1.0f;

#ifdef CLIENT_DLL
	m_flNextAutoReload = gpGlobals->curtime;
#endif

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound(SINGLE);

	if (m_bMuzzleFlash)
		pPlayer->DoMuzzleFlash();

	SendWeaponAnim(ACT_VM_PRIMARYATTACK);

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFFWpnData().m_flCycleTime;

	m_iClip1 -= GetFFWpnData().m_iCycleDecrement;

#ifdef GAME_DLL

	IGameEvent *pEvent = gameeventmanager->CreateEvent("player_shoot");
	if(pEvent)
	{
		pEvent->SetInt("userid", pPlayer->GetUserID());
		pEvent->SetInt("weapon", GetWeaponID());
		pEvent->SetInt("mode", 0);
		gameeventmanager->FireEvent(pEvent, true);
	}
#endif

	// player "shoot" animation
	//pPlayer->SetAnimation(PLAYER_ATTACK1);
	pPlayer->DoAnimationEvent(PLAYERANIMEVENT_FIRE_GUN_PRIMARY);

	// Fire bullets
	Fire();
	
#ifdef CLIENT_DLL
	pPlayer->m_flTrueAimTime = gpGlobals->curtime;
#endif

	// Do view punch from recoil
	pPlayer->ViewPunch(QAngle(-GetFFWpnData().m_flRecoilAmount, 0, 0));

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}
}

//-----------------------------------------------------------------------------
// Purpose: Override so shotgun can do mulitple reloads in a row
//-----------------------------------------------------------------------------
void CFFWeaponBaseClip::ItemPostFrame()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
	{
		return;
	}

	if (m_bInReload)
	{
		// Add the ammo now
		if (m_flReloadTime > 0 && m_flReloadTime <= gpGlobals->curtime)
		{
			FillClip();
			m_flReloadTime = -1.0f;
		}

		// If time for the next reload tick, then do it
		if (m_flTimeWeaponIdle <= gpGlobals->curtime)
		{
			// If out of ammo, end reload
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) < GetFFWpnData().m_iCycleDecrement)
			{
				FinishReload();
				return;
			}
			// If clip not full reload again
			if (m_iClip1 < GetMaxClip1())
			{
				Reload();
				return;
			}
			// Clip full, stop reloading
			else
			{
				FinishReload();
				return;
			}
		}
	}

	if ((pOwner->m_nButtons & IN_ATTACK) && m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		if ((m_iClip1 <= 0 && UsesClipsForAmmo1()) || (!UsesClipsForAmmo1() && !pOwner->GetAmmoCount(m_iPrimaryAmmoType)))
		{
			if (!pOwner->GetAmmoCount(m_iPrimaryAmmoType))
			{
				DryFire();
			}
			else if (!m_bInReload)
			{
				StartReload();
			}
		}
		else
		{
			// If the firing button was just pressed, reset the firing time
			CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
			if (pPlayer && pPlayer->m_afButtonPressed & IN_ATTACK)
			{
				m_flNextPrimaryAttack = gpGlobals->curtime;
			}
			PrimaryAttack();
		}
	}

	if (pOwner->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload)
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		StartReload();
	}
	else 
	{
		// no fire buttons down
		m_bFireOnEmpty = false;

		if (!HasAnyAmmo() && m_flNextPrimaryAttack < gpGlobals->curtime)
		{
			// weapon isn't useable, switch.
			if (! (GetWeaponFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) && pOwner->SwitchToNextBestWeapon(this))
			{
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
				return;
			}
		}
		else
		{
			// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
			if (m_iClip1 <= 0 && m_flTimeWeaponIdle < gpGlobals->curtime && m_flNextPrimaryAttack < gpGlobals->curtime)
			{
				if (StartReload())
				{
					// if we've successfully started to reload, we're done
					return;
				}
			}

			// Autoreload
			// This would be better done reading a client off the client
			// Added: Don't do it if they are holding down fire while there is still ammo in clip
#ifdef CLIENT_DLL
			if (!m_bInReload && !(pOwner->m_nButtons & IN_ATTACK && m_iClip1 > 0) 
				&& m_flNextAutoReload <= gpGlobals->curtime 
				&& m_iClip1 < GetMaxClip1() 
				&& auto_reload.GetBool())
			{
				if(pOwner->IsAlive())
				{
					engine->ClientCmd("+reload");
					m_flNextAutoReload = gpGlobals->curtime + 0.2f;
				}
			}
#endif
		}

		WeaponIdle();
		return;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFWeaponBaseClip::CFFWeaponBaseClip()
{
	m_bReloadsSingly = true;

#ifdef CLIENT_DLL
	m_flNextAutoReload = 0;
#endif
}
