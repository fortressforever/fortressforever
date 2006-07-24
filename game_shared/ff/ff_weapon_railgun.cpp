/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_railgun.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF railgun code & class declaration
///
/// REVISIONS
/// ---------
/// Jan 19 2004 Mirv: First implementation


#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"
#include "ff_projectile_rail.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponRailgun C_FFWeaponRailgun

	#include "c_ff_player.h"
	#include "c_te_effect_dispatch.h"
#else

	#include "ff_player.h"
	#include "te_effect_dispatch.h"
#endif

//=============================================================================
// CFFWeaponRailgun
//=============================================================================

class CFFWeaponRailgun : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponRailgun, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponRailgun();

	virtual void	Fire();
	virtual void	ItemPostFrame();
	void			RailBeamEffect();

	CNetworkVar(float, m_flStartCharge);

	virtual FFWeaponID GetWeaponID() const { return FF_WEAPON_RAILGUN; }

private:

	CFFWeaponRailgun(const CFFWeaponRailgun &);
};

//=============================================================================
// CFFWeaponRailgun tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponRailgun, DT_FFWeaponRailgun)

BEGIN_NETWORK_TABLE(CFFWeaponRailgun, DT_FFWeaponRailgun)
#ifdef GAME_DLL
	SendPropTime(SENDINFO(m_flStartCharge)), 
#else
	RecvPropTime(RECVINFO(m_flStartCharge)), 
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CFFWeaponRailgun)
DEFINE_PRED_FIELD_TOL(m_flStartCharge, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE), 
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(ff_weapon_railgun, CFFWeaponRailgun);
PRECACHE_WEAPON_REGISTER(ff_weapon_railgun);

//=============================================================================
// CFFWeaponRailgun implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponRailgun::CFFWeaponRailgun()
{
	m_flStartCharge = -1.0f;
}

//----------------------------------------------------------------------------
// Purpose: Fire a rail
//----------------------------------------------------------------------------
void CFFWeaponRailgun::Fire()
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	Vector	vecForward;
	pPlayer->EyeVectors(&vecForward);

	Vector	vecSrc = pPlayer->Weapon_ShootPosition();

#ifdef CLIENT_DLL
	// For now, fake the bullet source on the client
	C_BaseAnimating *pWeapon;

	// Use the correct weapon model
	if (pPlayer->IsLocalPlayer())
		pWeapon = pPlayer->GetViewModel(0);
	else
		pWeapon = pPlayer->GetActiveWeapon();

	// Get the attachment(precache this number sometime)
	if (pWeapon)
	{
		QAngle angAiming;
		int iAttachment = pWeapon->LookupAttachment("1");
		pWeapon->GetAttachment(iAttachment, vecSrc, angAiming);

		AngleVectors(angAiming, &vecForward);
	}
	else
		AssertMsg(0, "Couldn't get weapon!");
#endif

	QAngle angAiming;
	VectorAngles(pPlayer->GetAutoaimVector(0), angAiming);

	RailBeamEffect();

	float flChargeTime = gpGlobals->curtime - m_flStartCharge;

	// Simulate this as a bullet for now
	FireBulletsInfo_t info(1, vecSrc, vecForward, vec3_origin, MAX_TRACE_LENGTH, m_iPrimaryAmmoType);
	info.m_pAttacker = pPlayer;
	info.m_iDamage = pWeaponInfo.m_iDamage + (flChargeTime * 3.0f);
	info.m_iTracerFreq = 0;
	info.m_flDamageForceScale = 1.0f + (flChargeTime * 100.0f);

	pPlayer->FireBullets(info);
}

//----------------------------------------------------------------------------
// Purpose: Handle all the chargeup stuff here
//----------------------------------------------------------------------------
void CFFWeaponRailgun::ItemPostFrame()
{
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	if (pPlayer && pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0)
		HandleFireOnEmpty();

	// if we're currently firing, then check to see if we release

	if (pPlayer->m_nButtons & IN_ATTACK)
	{
		// Not currently charging
		if (m_flStartCharge < 0)
		{
			// we shouldn't let them fire just yet
			if (m_flNextPrimaryAttack > gpGlobals->curtime)
				return;

			// make sure they have ammo
			if (pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0)
				return;

			m_flStartCharge = gpGlobals->curtime;
		}
	}
	else
	{
		if (m_flStartCharge > 0)
		{
			WeaponSound(SINGLE);

			pPlayer->DoMuzzleFlash();

			SendWeaponAnim(GetPrimaryAttackActivity());

			// player "shoot" animation
			pPlayer->SetAnimation(PLAYER_ATTACK1);

			float flPower = gpGlobals->curtime - m_flStartCharge;
			pPlayer->ViewPunch(-QAngle(1.0f + flPower, 0, 0));

			// Fire!!
			Fire();

			// remove the ammo
#ifdef GAME_DLL
			pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);
#endif

			m_flNextPrimaryAttack = gpGlobals->curtime + GetFFWpnData().m_flCycleTime;
		}

		m_flStartCharge = -1.0f;
	}
}

//----------------------------------------------------------------------------
// Purpose: Set up the actual beam effect
//----------------------------------------------------------------------------
void CFFWeaponRailgun::RailBeamEffect()
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	if (!pPlayer)
		return;

	CEffectData data;
	data.m_flScale = 1.0f;
	data.m_nEntIndex = pPlayer->entindex();

	DispatchEffect("RailBeam", data);
}