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

#include "ff_weapon_hookgun.h"
#include "in_buttons.h"

extern ConVar auto_reload;

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
	m_pHook = NULL;
}

//----------------------------------------------------------------------------
// Purpose: Fire a rocket
//----------------------------------------------------------------------------
void CFFWeaponHookGun::Fire() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;
	//const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	Vector	vForward, vRight, vUp;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);

	//Vector	vecSrc = pPlayer->Weapon_ShootPosition() + vForward * 8.0f + vRight * 8.0f + vUp * -8.0f;
	Vector vecSrc = pPlayer->GetLegacyAbsOrigin() + vForward * 16.0f + vRight * 8.0f + Vector(0, 1, (pPlayer->GetFlags() & FL_DUCKING) ? 5.0f : 23.0f);

	CFFProjectileHook *pHook = CFFProjectileHook::CreateHook(this, vecSrc, pPlayer->EyeAngles(), pPlayer);
	pHook->SetLocalAngularVelocity(QAngle(0, 0, -500)); // spin!
	// TODO: if we want, we should delete any old hook so player can only have 1 active.
	// to do this we'd just have a pointer to the current hook, then RemoveHook that one and set it to pHook here

	m_pHook = pHook;

#ifdef GAME_DLL
	Omnibot::Notify_PlayerShoot(pPlayer, Omnibot::TF_WP_HOOKGUN, pHook);
#endif
}

//-----------------------------------------------------------------------------
// Purpose: This is copied + tweaked from baseclip
//-----------------------------------------------------------------------------
void CFFWeaponHookGun::ItemPostFrame()
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
		if ( m_pHook ) // if hook exists just delete it and dont allow firing until they release the attack key
		{
			m_pHook->RemoveHook();
			m_pHook = NULL;
			//m_flNextPrimaryAttack = gpGlobals->curtime + 999.0f; //enable refiring
		}
		/*else*/ if ((m_iClip1 <= 0 && UsesClipsForAmmo1()) || (!UsesClipsForAmmo1() && !pOwner->GetAmmoCount(m_iPrimaryAmmoType)))
		{
			if (!pOwner->GetAmmoCount(m_iPrimaryAmmoType))
			{
				DryFire();
			}
			else if (!m_bInReload)
			{
				if (!m_pHook)
				{
					StartReload();
				}
			}
		}
		else
		{
			 // create a hook and dont allow firing until they release the attack key
			// If the firing button was just pressed, reset the firing time
			CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
			if (pPlayer && pPlayer->m_afButtonPressed & IN_ATTACK)
			{
				m_flNextPrimaryAttack = gpGlobals->curtime;
			}
			PrimaryAttack();
			//m_flNextPrimaryAttack = gpGlobals->curtime + 999.0f;
		}
	}
	/* Commenting this because it makes u fire when you spawn - shok
	else if ( !(pOwner->m_nButtons & IN_ATTACK) )
	{
		m_flNextPrimaryAttack = 0.0f; // enable firing again if they release the attack key
	}*/

	if (pOwner->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload)
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		if (!m_pHook)
		{
			StartReload();
		}
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
				if (!m_pHook)
				{
					if (StartReload())
					{
						// if we've successfully started to reload, we're done
						return;
					}
				}
			}

			// Autoreload
			// This would be better done reading a client off the client
			// Added: Don't do it if they are holding down fire while there is still ammo in clip
#ifdef CLIENT_DLL
			if( auto_reload.GetBool() )
#else
			if( (Q_atoi(engine->GetClientConVarValue( pOwner->entindex(), "cl_autoreload" ) ) ) )
#endif
			{
				if (!m_bInReload && !(pOwner->m_nButtons & IN_ATTACK && m_iClip1 > 0) 
					&& m_flNextAutoReload <= gpGlobals->curtime 
					&& m_iClip1 < GetMaxClip1())
				{
					if(pOwner->IsAlive())
					{
						if (!m_pHook)
						{
							StartReload();
						}
					}
				}
			}
		}

		WeaponIdle();
		return;
	}
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

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CFFWeaponHookGun::Holster(CBaseCombatWeapon *pSwitchingTo) 
{
	if ( m_pHook ) // Remove any hooks if you change weapon
	{
		m_pHook->RemoveHook();
		m_pHook = NULL;
	}

	return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose: Precache
//-----------------------------------------------------------------------------
void CFFWeaponHookGun::Precache()
{
	PrecacheScriptSound("hookgun.rope_snap");
	PrecacheScriptSound("hookgun.winch");
	BaseClass::Precache();
}