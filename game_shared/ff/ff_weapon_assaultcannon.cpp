/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_assaultcannon.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF Assault Cannon code.
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation logged


#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponAssaultCannon C_FFWeaponAssaultCannon
	#include "c_ff_player.h"
#else
	#include "ff_player.h"
#endif

//=============================================================================
// CFFWeaponAssaultCannon
//=============================================================================

class CFFWeaponAssaultCannon : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponAssaultCannon, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponAssaultCannon();

	virtual void PrimaryAttack();
	virtual void WeaponIdle();
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual bool Deploy();

	virtual void Precache();

	virtual void Fire();

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_ASSAULTCANNON; }
	const char *GetTracerType() { return "ACTracer"; }

private:

	CFFWeaponAssaultCannon(const CFFWeaponAssaultCannon &);
	CNetworkVar(int, m_fFireState);
};

//=============================================================================
// CFFWeaponAssaultCannon tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponAssaultCannon, DT_FFWeaponAssaultCannon) 

BEGIN_NETWORK_TABLE(CFFWeaponAssaultCannon, DT_FFWeaponAssaultCannon) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponAssaultCannon) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_assaultcannon, CFFWeaponAssaultCannon);
PRECACHE_WEAPON_REGISTER(ff_weapon_assaultcannon);

//=============================================================================
// CFFWeaponAssaultCannon implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponAssaultCannon::CFFWeaponAssaultCannon() 
{
}

//----------------------------------------------------------------------------
// Purpose: When holstered we need to stop any sounds + remove speed effects
//----------------------------------------------------------------------------
bool CFFWeaponAssaultCannon::Holster(CBaseCombatWeapon *pSwitchingTo) 
{
	// Bug #0000499: Oddity with assault cannon
	// Moved this up to here so it gets called and remove it if its set
#ifdef GAME_DLL
	CFFPlayer *pPlayer = GetPlayerOwner();

	if( pPlayer->IsSpeedEffectSet( SE_ASSAULTCANNON ) )
        pPlayer->AddSpeedEffect( SE_ASSAULTCANNON, 0.5f, 80.0f / 230.0f, SEM_BOOLEAN );
#endif

	if (!m_fFireState) 
		return BaseClass::Holster(pSwitchingTo);

	//StopSound("Assaultcannon.loop_shot");	// Possibly not needed
	//EmitSound("Assaultcannon.Winddown");

	// WindDown
	WeaponSound(SPECIAL2);

	m_fFireState = 0;

	return BaseClass::Holster(pSwitchingTo);
}

//----------------------------------------------------------------------------
// Purpose: Reset state
//----------------------------------------------------------------------------
bool CFFWeaponAssaultCannon::Deploy()
{
	m_fFireState = 0;

	return BaseClass::Deploy();
}

//----------------------------------------------------------------------------
// Purpose: Fires bullets
//----------------------------------------------------------------------------
void CFFWeaponAssaultCannon::Fire() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward);

	FireBulletsInfo_t info(pWeaponInfo.m_iBullets, pPlayer->Weapon_ShootPosition(), vecForward, Vector(pWeaponInfo.m_flBulletSpread, pWeaponInfo.m_flBulletSpread, pWeaponInfo.m_flBulletSpread), MAX_TRACE_LENGTH, m_iPrimaryAmmoType);
	info.m_pAttacker = pPlayer;
	info.m_iDamage = pWeaponInfo.m_iDamage;

	pPlayer->FireBullets(info);

}

//----------------------------------------------------------------------------
// Purpose: Spin up or fire weapon, depending on state
//----------------------------------------------------------------------------
void CFFWeaponAssaultCannon::PrimaryAttack() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	// check to see if we're ready to spin up
	if (m_fFireState == 0) 
	{
		pPlayer->SetAnimation(PLAYER_ATTACK1);

		SendWeaponAnim(ACT_FIRE_START);
		m_fFireState = 1;

		//CPASAttenuationFilter filter(this);
		//filter.UsePredictionRules();
		//EmitSound(filter, entindex(), "Assaultcannon.Windup");

		// Windup
		WeaponSound(SPECIAL1);
		
		pPlayer->m_flNextAttack = gpGlobals->curtime + pWeaponInfo.m_flSpinTime;

		m_flNextPrimaryAttack = gpGlobals->curtime + pWeaponInfo.m_flSpinTime;
		m_flNextSecondaryAttack = gpGlobals->curtime + pWeaponInfo.m_flSpinTime;
 
		SetWeaponIdleTime(gpGlobals->curtime + pWeaponInfo.m_flSpinTime);

#ifdef GAME_DLL
		pPlayer->AddSpeedEffect(SE_ASSAULTCANNON, 999, 80.0/230.0, SEM_BOOLEAN);
#endif
	}
	// Spinning
	else if (m_fFireState > 0) 
	{
		pPlayer->m_iShotsFired++;

		// Out of ammo?
		if (pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0) 
		{
			if (m_bFireOnEmpty) 
			{
				PlayEmptySound();
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			}
			//return;	// return?
		}
		else if (m_fFireState == 1) 
		{
			CPASAttenuationFilter filter(this);
			filter.UsePredictionRules();

			//EmitSound(filter, entindex(), "Assaultcannon.loop_shot");

			WeaponSound(BURST);

			m_fFireState++;
		}


		SendWeaponAnim(ACT_VM_PRIMARYATTACK);
	
#ifdef GAME_DLL
		pPlayer->RemoveAmmo(pWeaponInfo.m_iCycleDecrement, m_iPrimaryAmmoType);
#endif

		Fire();

		pPlayer->DoMuzzleFlash();
	
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + pWeaponInfo.m_flCycleTime;

		if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0) 
		{
			//StopSound("Assaultcannon.loop_shot");
			WeaponSound(EMPTY);

			// HEV suit - indicate out of ammo condition
			pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
		}

		// so we can catch for spindown immediately
		SetWeaponIdleTime(gpGlobals->curtime + 0.1);
	}
}

//----------------------------------------------------------------------------
// Purpose: Spin down or play idle animation, depending on state
//----------------------------------------------------------------------------
void CFFWeaponAssaultCannon::WeaponIdle() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	if (m_flTimeWeaponIdle > gpGlobals->curtime) 
		return;

	// We are either spinning up or down
	if (m_fFireState != 0) 
	{
		pPlayer->SetAnimation(PLAYER_ATTACK1);

		SendWeaponAnim(ACT_FIRE_END);
		m_fFireState = 0;

		//StopSound("Assaultcannon.loop_shot");

		//CPASAttenuationFilter filter(this);
		//filter.UsePredictionRules();
		//EmitSound(filter, entindex(), "Assaultcannon.Winddown");

		// Wind down
		WeaponSound(SPECIAL2);

		pPlayer->m_flNextAttack = gpGlobals->curtime + pWeaponInfo.m_flSpinTime;

		m_flNextPrimaryAttack = gpGlobals->curtime + pWeaponInfo.m_flSpinTime;
		m_flNextSecondaryAttack = gpGlobals->curtime + pWeaponInfo.m_flSpinTime;
 
		SetWeaponIdleTime(gpGlobals->curtime + pWeaponInfo.m_flSpinTime);
	}
	else
	{
		SetWeaponIdleTime(gpGlobals->curtime + 5.0f);
		SendWeaponAnim(ACT_VM_IDLE);

#ifdef GAME_DLL
		pPlayer->RemoveSpeedEffect(SE_ASSAULTCANNON);
#endif
	}
}

//----------------------------------------------------------------------------
// Purpose: Precache some extra sounds
//----------------------------------------------------------------------------
void CFFWeaponAssaultCannon::Precache() 
{
	PrecacheScriptSound("Assaultcannon.loop_shot");
	PrecacheScriptSound("Assaultcannon.Windup");
	PrecacheScriptSound("Assaultcannon.Winddown");
	BaseClass::Precache();
}
