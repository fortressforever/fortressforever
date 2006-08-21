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
#include "in_buttons.h"
#include "soundenvelope.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponAssaultCannon C_FFWeaponAssaultCannon
	#include "c_ff_player.h"
#else
	#include "ff_player.h"
#endif

#define AC_MAX_CHARGETIME	7.0f
#define AC_CHARGEUP_TIME	0.6f
#define AC_MAX_REV_SOUND	3.0f

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
	~CFFWeaponAssaultCannon();

	virtual void PrimaryAttack();
	//virtual void WeaponIdle();
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual bool Deploy();

	virtual void ItemPostFrame();

	virtual void Precache();

	virtual void Fire();

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_ASSAULTCANNON; }
	const char *GetTracerType() { return "ACTracer"; }

private:

	CFFWeaponAssaultCannon(const CFFWeaponAssaultCannon &);

public:	// temp while i expose m_flChargeTime to global function

	float m_flLastTick;
	CNetworkVar(float, m_flChargeTime);

	bool	m_bFiring;

#ifdef CLIENT_DLL
	CSoundPatch *m_pEngine;

	float		m_flRotationValue;
	float		m_flChargeTimeClient;

	int			m_iBarrelRotation;
#endif
};

//=============================================================================
// CFFWeaponAssaultCannon tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponAssaultCannon, DT_FFWeaponAssaultCannon) 

BEGIN_NETWORK_TABLE(CFFWeaponAssaultCannon, DT_FFWeaponAssaultCannon) 
#ifdef GAME_DLL
	SendPropFloat(SENDINFO(m_flChargeTime)),
#else
	RecvPropFloat(RECVINFO(m_flChargeTime)),
#endif
END_NETWORK_TABLE() 

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CFFWeaponAssaultCannon) 
	DEFINE_PRED_FIELD_TOL(m_flChargeTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, 1.0f),
END_PREDICTION_DATA() 
#endif

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
	m_flNextSecondaryAttack = gpGlobals->curtime;

#ifdef CLIENT_DLL
	m_pEngine = NULL;

	m_flRotationValue = 0;
	m_iBarrelRotation = -1;
#endif
}

//----------------------------------------------------------------------------
// Purpose: Deconstructor
//----------------------------------------------------------------------------
CFFWeaponAssaultCannon::~CFFWeaponAssaultCannon() 
{
	m_flNextSecondaryAttack = gpGlobals->curtime;

#ifdef CLIENT_DLL

	if(m_pEngine)
	{
		CSoundEnvelopeController::GetController().SoundDestroy(m_pEngine);
		m_pEngine = NULL;
	}

	m_flRotationValue = 0;
	m_iBarrelRotation = -1;
#endif
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

	if(pPlayer->IsSpeedEffectSet(SE_ASSAULTCANNON))
        pPlayer->AddSpeedEffect(SE_ASSAULTCANNON, 0.5f, 80.0f / 230.0f, SEM_BOOLEAN);
#endif

	// Also start the engine sound for the client
#ifdef CLIENT_DLL
	if (m_pEngine && GetPlayerOwner() == CBasePlayer::GetLocalPlayer())
	{
		CSoundEnvelopeController::GetController().SoundDestroy(m_pEngine);
		m_pEngine = NULL;
	}
#endif

	//if (!m_fFireState) 
	//	return BaseClass::Holster(pSwitchingTo);

	//StopSound("Assaultcannon.loop_shot");	// Possibly not needed
	//EmitSound("Assaultcannon.Winddown");

	// WindDown
#ifdef CLIENT_DLL
	if (GetPlayerOwner() == CBasePlayer::GetLocalPlayer())
#endif
	{
		WeaponSound(SPECIAL2);
	}

	//m_fFireState = 0;

	return BaseClass::Holster(pSwitchingTo);
}

//----------------------------------------------------------------------------
// Purpose: Reset state
//----------------------------------------------------------------------------
bool CFFWeaponAssaultCannon::Deploy()
{
	m_flChargeTime = 0;
	m_flLastTick = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime;

	m_bFiring = false;

#ifdef CLIENT_DLL
	// play an engine start sound!!
	CPASAttenuationFilter filter(this);

	// Bring up the engine looping sound.
	if (!m_pEngine)
	{
		m_pEngine = CSoundEnvelopeController::GetController().SoundCreate(filter, entindex(), "assaultcannon.rotate");
		CSoundEnvelopeController::GetController().Play(m_pEngine, 0.0, 50);
		CSoundEnvelopeController::GetController().SoundChangeVolume(m_pEngine, 0.7, 2.0);
	}

	m_flRotationValue = 0;
#endif

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

	Vector vecTest = info.m_vecSrc;

#ifdef CLIENT_DLL
	QAngle tmpAngle;
	pPlayer->GetViewModel()->GetAttachment(1, info.m_vecSrc, tmpAngle);
#endif

	pPlayer->FireBullets(info);

}

void CFFWeaponAssaultCannon::ItemPostFrame()
{
	CFFPlayer *pOwner = ToFFPlayer(GetOwner());

	if (!pOwner)
		return;

	float flTimeDelta = gpGlobals->curtime - m_flLastTick;
	m_flLastTick = gpGlobals->curtime;

#ifdef CLIENT_DLL
	
	// A buffered version of m_flChargeTime, if you will. This is to stop the
	// jerkiness that is being annoying
	if (pOwner->m_nButtons & IN_ATTACK)
	{
		m_flChargeTimeClient = max(m_flChargeTimeClient, m_flChargeTime);
	}
	else
	{
		m_flChargeTimeClient = min(m_flChargeTimeClient, m_flChargeTime);
	}
	

	CBaseViewModel *pVM = pOwner->GetViewModel();
	
	if (m_iBarrelRotation < 0)
	{
		m_iBarrelRotation = pVM->LookupPoseParameter("ac_rotate");
	}

	// Might need to separate m_flRotationValue from m_flChargeTime
	// Perhaps a separate client-side variable to track it
	m_flRotationValue += flTimeDelta * 100.0f * m_flChargeTimeClient * 5.0f;
	m_flRotationValue = pVM->SetPoseParameter(m_iBarrelRotation, m_flRotationValue);
#endif

	// Keep track of fire duration for anywhere else it may be needed
	m_fFireDuration = (pOwner->m_nButtons & IN_ATTACK) ? (m_fFireDuration + gpGlobals->frametime) : 0.0f;

	// Player is holding down fire. Don't allow it if we're still recovering from an overheat though
	if (pOwner->m_nButtons & IN_ATTACK && m_flNextSecondaryAttack <= gpGlobals->curtime)
	{
		m_flChargeTime += flTimeDelta;

		// Oh no...
		if (m_flChargeTime > AC_MAX_CHARGETIME)
		{
			// Freeze for 5s, reduce to max rev sound so it falls away instantly
			m_flNextSecondaryAttack = gpGlobals->curtime + 5.0;
			m_flChargeTime = AC_MAX_REV_SOUND;
			
			// Play the overheat sound
			WeaponSound(SPECIAL3);

#ifdef GAME_DLL
			// Remember to reset the speed soon
			pOwner->AddSpeedEffect(SE_ASSAULTCANNON, 0.5f, 80.0f / 230.0f, SEM_BOOLEAN);
#endif

			m_bFiring = false;
		}

		// Time for the next real fire think
		else if (m_flChargeTime > AC_CHARGEUP_TIME && m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
			// Out of ammo
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			{
				WeaponSound(STOP);
				HandleFireOnEmpty();
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;
			}
			// Weapon should be firing now
			else
			{
				// If the firing button was just pressed, reset the firing time
				if (pOwner && pOwner->m_afButtonPressed & IN_ATTACK)
					m_flNextPrimaryAttack = gpGlobals->curtime;

				m_flPlaybackRate = 1.0f + (m_flChargeTime);
				PrimaryAttack();
			}

			m_flNextPrimaryAttack = gpGlobals->curtime + 1.0f;

			float flT = (m_flChargeTime - AC_CHARGEUP_TIME) / (2.0f * AC_MAX_CHARGETIME);

			m_flNextPrimaryAttack = gpGlobals->curtime + (GetFFWpnData().m_flCycleTime * (flT > 0.0f ? 1.0f : 1 - flT));

			if (!m_bFiring && m_flChargeTime > AC_CHARGEUP_TIME)
			{
				WeaponSound(BURST);

#ifdef GAME_DLL
				CFFPlayer *pPlayer = GetPlayerOwner();
				pPlayer->AddSpeedEffect(SE_ASSAULTCANNON, 999, 80.0f / 230.0f, SEM_BOOLEAN);
#endif

				m_bFiring = true;
			}
		}
	}
	// No buttons down
	else
	{
		// Reduce speed at 3 times the rate
		if (m_flChargeTime > 0)
		{
			m_flChargeTime -= 3.0f * flTimeDelta;

			if (m_flChargeTime < 0)
				m_flChargeTime = 0;
		}
		
		if (m_bFiring)
		{
			WeaponSound(STOP);

#ifdef GAME_DLL
			CFFPlayer *pPlayer = GetPlayerOwner();
			pPlayer->RemoveSpeedEffect(SE_ASSAULTCANNON);
#endif

			m_bFiring = false;
		}

		WeaponIdle();
	}

#ifdef CLIENT_DLL
	if (m_pEngine)
	{
		float flPitch = 40 + 10 * min(AC_MAX_REV_SOUND, m_flChargeTime);
		CSoundEnvelopeController::GetController().SoundChangePitch(m_pEngine, min(80, flPitch), 0);
	}
#endif
}

//----------------------------------------------------------------------------
// Purpose: Precache some extra sounds
//----------------------------------------------------------------------------
void CFFWeaponAssaultCannon::Precache() 
{
	PrecacheScriptSound("Assaultcannon.loop_shot");
	PrecacheScriptSound("Assaultcannon.Windup");
	PrecacheScriptSound("Assaultcannon.Winddown");
	PrecacheScriptSound("assaultcannon.rotate");
	PrecacheScriptSound("assaultcannon.overheat");
	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Precache some extra sounds
//----------------------------------------------------------------------------
void CFFWeaponAssaultCannon::PrimaryAttack()
{
	// Only the player fires this way so we can cast
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	if (!pPlayer)
		return;

	// Undisguise
#ifdef GAME_DLL
	pPlayer->ResetDisguise();
#endif

	// MUST call sound before removing a round from the clip of a CMachineGun
	//WeaponSound(SINGLE);

	pPlayer->DoMuzzleFlash();

	SendWeaponAnim(GetPrimaryAttackActivity());

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifdef GAME_DLL
	int nShots = min(GetFFWpnData().m_iCycleDecrement, pPlayer->GetAmmoCount(m_iPrimaryAmmoType));
	pPlayer->RemoveAmmo(nShots, m_iPrimaryAmmoType);
#endif

	// Fire now
	Fire();

	m_flNextPrimaryAttack = gpGlobals->curtime + GetFFWpnData().m_flCycleTime;

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}

	//Add our view kick in
	pPlayer->ViewPunch(QAngle(-GetFFWpnData().m_flRecoilAmount, 0, 0));
}

#ifdef CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose: This is a awful function to quickly get the AC charge. It will be
//			replaced tomorrow.
//-----------------------------------------------------------------------------
float GetAssaultCannonCharge()
{
	C_FFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

	if (!pPlayer)
		return 0.0f;

	C_FFWeaponBase *pWeapon = pPlayer->GetActiveFFWeapon();
	Assert(pWeapon);

	if (!pWeapon || pWeapon->GetWeaponID() != FF_WEAPON_ASSAULTCANNON)
		return 0.0f;

	CFFWeaponAssaultCannon *pAC = (CFFWeaponAssaultCannon *) pWeapon;

	return (100.0f * pAC->m_flChargeTime / AC_MAX_CHARGETIME);
}
#endif