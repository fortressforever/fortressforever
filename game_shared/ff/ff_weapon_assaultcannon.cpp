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

#ifdef CLIENT_DLL 
	#define CFFWeaponAssaultCannon C_FFWeaponAssaultCannon
	#include "c_ff_player.h"
	#include "ff_utils.h"
	#include "soundenvelope.h"
#else
	#include "omnibot_interface.h"
	#include "ff_player.h"
#endif


// please keep some values exposed to cvars so non programmers can tweak them, even if the code isn't final
ConVar ffdev_ac_maxchargetime("ffdev_ac_maxchargetime", "2.0", FCVAR_REPLICATED, "Assault Cannon Max Charge Time");
ConVar ffdev_ac_chargeuptime("ffdev_ac_chargeuptime", "0.0", FCVAR_REPLICATED, "Assault Cannon Chargeup Time");
ConVar ffdev_ac_overheatdelay("ffdev_ac_overheatdelay", "1.0", FCVAR_REPLICATED, "Assault Cannon Overheat delay");

ConVar ffdev_ac_spread_min("ffdev_ac_spread_min", "0.01", FCVAR_REPLICATED, "Assault Cannon Minimum spread");
ConVar ffdev_ac_spread_max("ffdev_ac_spread_max", "0.10", FCVAR_REPLICATED, "Assault Cannon Maximum spread");

ConVar ffdev_ac_rof_max("ffdev_ac_rof_max", "0.12", FCVAR_REPLICATED, "Assault Cannon maximum rate of fire");
ConVar ffdev_ac_rof_min("ffdev_ac_rof_min", "0.06", FCVAR_REPLICATED, "Assault Cannon minimum rate of fire");

ConVar ffdev_ac_rof_loopshotsound_max("ffdev_ac_rof_loopshotsound_max", "0.12", FCVAR_REPLICATED, "Assault Cannon loop shot sound's maximum rate of fire");
ConVar ffdev_ac_rof_loopshotsound_min("ffdev_ac_rof_loopshotsound_min", "0.06", FCVAR_REPLICATED, "Assault Cannon loop shot sound's minimum rate of fire");

ConVar ffdev_ac_bulletpush( "ffdev_ac_bulletpush", "1.0", FCVAR_REPLICATED, "Assault Cannon bullet push force");

#define FF_AC_LOOPSHOTSOUND_VOLUME_HIGH		1.0f
#define FF_AC_LOOPSHOTSOUND_VOLUME_LOW		0.4f
#define FF_AC_LOOPSHOTSOUND_PITCH_HIGH		160.0f
#define FF_AC_LOOPSHOTSOUND_PITCH_LOW		80.0f
#define FF_AC_LOOPSHOTSOUND_UPDATEINTERVAL	0.03f

#define FF_AC_SPEEDEFFECT_MAX 0.5f
#define FF_AC_SPEEDEFFECT_MIN 0.2f

#ifdef CLIENT_DLL

#define FF_AC_BARRELROTATIONSOUND_VOLUME_HIGH		1.0f
#define FF_AC_BARRELROTATIONSOUND_VOLUME_LOW		0.1f
#define FF_AC_BARRELROTATIONSOUND_PITCH_HIGH		100.0f
#define FF_AC_BARRELROTATIONSOUND_PITCH_LOW			50.0f

#define FF_AC_BARRELROTATION_SPEED_MIN 248.0f
#define FF_AC_BARRELROTATION_SPEED_MAX 448.0f

#define FF_AC_CHARGETIMEBUFFERED_UPDATEINTERVAL 0.01f

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
	~CFFWeaponAssaultCannon();

	virtual void Precache();
	virtual bool Deploy();
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual void ItemPostFrame();
	virtual void PrimaryAttack();
	virtual void Fire();

	void UpdateChargeTime();

	virtual void GetHeatLevel(int _firemode, float &_current, float &_max) 
	{
		_current = m_flChargeTime; 
		_max = ffdev_ac_maxchargetime.GetFloat(); 
	}
private:
	virtual float GetFireRate();
	Vector GetFireSpread();

#ifdef CLIENT_DLL

	void UpdateBarrelRotation();
	void PlayBarrelRotationSound();
	void StopBarrelRotationSound();
	int m_iBarrelRotation;
	float m_flBarrelRotationValue;
	float m_flBarrelRotationDelta;
	float m_flBarrelRotationStopTimer;
	CSoundPatch *m_sndBarrelRotation;

	float m_flChargeTimeBuffered;
	float m_flChargeTimeBufferedNextUpdate;

#endif

	void PlayLoopShotSound();
	void StopLoopShotSound();
	int m_nLoopShotSound;
	EmitSound_t m_LoopShotSoundParams;
	bool m_bPlayLoopShotSound;
	float m_flLoopShotSoundNextUpdate;

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_ASSAULTCANNON; }
	//const char *GetTracerType() { return "ACTracer"; }

private:

	CFFWeaponAssaultCannon(const CFFWeaponAssaultCannon &);

public:	// temp while i expose m_flChargeTime to global function

	float m_flLastTick;
	float m_flDeployTick;
	float m_flChargeTime;
	CNetworkVar(float, m_flTriggerPressed);
	CNetworkVar(float, m_flTriggerReleased);

	bool m_bFiring;
};

//=============================================================================
// CFFWeaponAssaultCannon tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponAssaultCannon, DT_FFWeaponAssaultCannon) 

BEGIN_NETWORK_TABLE(CFFWeaponAssaultCannon, DT_FFWeaponAssaultCannon) 
#ifdef GAME_DLL
	SendPropFloat(SENDINFO(m_flTriggerPressed)),
	SendPropFloat(SENDINFO(m_flTriggerReleased)),
#else
	RecvPropFloat(RECVINFO(m_flTriggerPressed)),
	RecvPropFloat(RECVINFO(m_flTriggerReleased)),
#endif
END_NETWORK_TABLE() 

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CFFWeaponAssaultCannon) 
	DEFINE_PRED_FIELD_TOL(m_flTriggerPressed, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, 1.0f),
	DEFINE_PRED_FIELD_TOL(m_flTriggerReleased, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, 1.0f),
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

	m_flLastTick = 0.0f;
	m_flDeployTick = 0.0f;

	m_flTriggerReleased = 1.0f;
	m_flTriggerPressed = 0.0f;

	m_bFiring = false;

	m_nLoopShotSound = WPN_DOUBLE;
	m_bPlayLoopShotSound = false;
	m_flLoopShotSoundNextUpdate = 0.0f;

#ifdef CLIENT_DLL

	m_flChargeTimeBuffered = 0.0f;
	m_flChargeTimeBufferedNextUpdate = 0.0f;

	m_iBarrelRotation = -69;
	m_flBarrelRotationValue = 0.0f;
	m_flBarrelRotationDelta = 0.0f;
	m_flBarrelRotationStopTimer = 0.0f;
	m_sndBarrelRotation = NULL;

#endif
}

//----------------------------------------------------------------------------
// Purpose: Deconstructor
//----------------------------------------------------------------------------
CFFWeaponAssaultCannon::~CFFWeaponAssaultCannon() 
{
	m_flNextSecondaryAttack = gpGlobals->curtime;

#ifdef CLIENT_DLL

	m_flChargeTimeBuffered = 0.0f;
	m_flChargeTimeBufferedNextUpdate = 0.0f;

	m_iBarrelRotation = -69;
	m_flBarrelRotationValue = 0.0f;
	m_flBarrelRotationDelta = 0.0f;
	m_flBarrelRotationStopTimer = 0.0f;

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

	// Add a temp slowdown as we unwind
	//if(pPlayer->IsSpeedEffectSet(SE_ASSAULTCANNON))
	//	pPlayer->AddSpeedEffect( SE_ASSAULTCANNON, 0.5f, ffdev_ac_slowdownspeed.GetFloat(), SEM_BOOLEAN );

	// uh....why slow the player down when they're holstering it?
	pPlayer->RemoveSpeedEffect(SE_ASSAULTCANNON);
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
		if( (m_flLastTick - m_flDeployTick) > 0.5 )
			WeaponSoundLocal(SPECIAL2);
		//else
		//	WeaponSoundLocal(STOP);
	}

	StopLoopShotSound();

#ifdef CLIENT_DLL
	StopBarrelRotationSound();
#endif

	//m_fFireState = 0;

	CFFPlayer *pOwner = ToFFPlayer(GetOwner());
	if (pOwner)
		if ( pOwner->m_nButtons & IN_ATTACK || pOwner->m_afButtonPressed & IN_ATTACK )
			m_flTriggerReleased = gpGlobals->curtime; // we are essentially releasing trigger by holstering

	return BaseClass::Holster(pSwitchingTo);
}

//----------------------------------------------------------------------------
// Purpose: Reset state
//----------------------------------------------------------------------------
bool CFFWeaponAssaultCannon::Deploy()
{
	m_flChargeTime = 0;
	m_flLastTick = gpGlobals->curtime;
	m_flDeployTick = gpGlobals->curtime;

	m_bFiring = false;

	m_bPlayLoopShotSound = false;
	m_flLoopShotSoundNextUpdate = 0.0f;

	CFFPlayer *pOwner = ToFFPlayer(GetOwner());
	if (pOwner)
		if ( pOwner->m_nButtons & IN_ATTACK || pOwner->m_afButtonPressed & IN_ATTACK )
			m_flTriggerPressed = gpGlobals->curtime; // set this if we come into this while +attacking
	
#ifdef CLIENT_DLL

	m_flChargeTimeBuffered = 0.0f;
	m_flChargeTimeBufferedNextUpdate = 0.0f;

	m_iBarrelRotation = -69;
	m_flBarrelRotationValue = 0.0f;
	m_flBarrelRotationDelta = 0.0f;
	m_flBarrelRotationStopTimer = 0.0f;

#endif

	return BaseClass::Deploy();
}

//----------------------------------------------------------------------------
// Purpose: Fires bullets
//----------------------------------------------------------------------------
void CFFWeaponAssaultCannon::Fire() 
{
	Assert(0);
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward);

	FireBulletsInfo_t info(pWeaponInfo.m_iBullets, pPlayer->Weapon_ShootPosition(), vecForward, GetFireSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType);
	info.m_pAttacker = pPlayer;
	info.m_iDamage = pWeaponInfo.m_iDamage;

	// AC Push.  May not function because the CFFPlayer::Fire() function (line 944 ff_player_shared) is scaling push by 0.01f -> Defrag
	info.m_flDamageForceScale = ffdev_ac_bulletpush.GetFloat();

	Vector vecTest = info.m_vecSrc;

#ifdef CLIENT_DLL
	QAngle tmpAngle;
	pPlayer->GetViewModel()->GetAttachment(1, info.m_vecSrc, tmpAngle);
#endif

	pPlayer->FireBullets(info);

#ifdef GAME_DLL
	Omnibot::Notify_PlayerShoot(pPlayer, Omnibot::TF_WP_MINIGUN, 0);
#endif
}

void CFFWeaponAssaultCannon::UpdateChargeTime()
{
	CFFPlayer *pOwner = ToFFPlayer(GetOwner());

	if (!pOwner)
		return;

	m_fFireDuration = (pOwner->m_nButtons & IN_ATTACK) ? (m_fFireDuration + gpGlobals->frametime) : 0.0f;

	// Firing has started
	if (pOwner->m_afButtonPressed & IN_ATTACK && m_flTriggerPressed < gpGlobals->curtime)
	{
		// We've logged the trigger having being released recently.
		// If the trigger was held longer than the time since the release then there
		// must be charge left on the AC.
		// We will subtract this remaining charge from the curtime for the new charge.
		if (m_flTriggerReleased > m_flTriggerPressed)
		{
			float flTimeSinceRelease = gpGlobals->curtime - m_flTriggerReleased;
			float flTimeHeld = m_flTriggerReleased - m_flTriggerPressed;
			float flTimeLeft = flTimeHeld - flTimeSinceRelease;

			if (flTimeLeft > 0 && flTimeLeft < ffdev_ac_maxchargetime.GetFloat())
			{
				m_flTriggerPressed = gpGlobals->curtime - flTimeLeft;
			}
			else
				m_flTriggerPressed = gpGlobals->curtime;
		}

		// Sometimes m_afButtonPressed seems to be set for 2 frames in a row.
		// Therefore only allow 
		else if (m_flTriggerReleased > 0)
			m_flTriggerPressed = gpGlobals->curtime;

		// Reset the trigger released
		m_flTriggerReleased = 0;
	}

	// If they have released the fire button, catch the release trigger time
	if (pOwner->m_afButtonReleased & IN_ATTACK)
	{
		m_flTriggerReleased = gpGlobals->curtime;
	}

	// If we're currently firing then the charge time is simply the time since the
	// trigger was pressed (assuming trigger actually was pressed).
	if (pOwner->m_nButtons & IN_ATTACK && m_flTriggerPressed)
	{
		m_flChargeTime = gpGlobals->curtime - m_flTriggerPressed;
	}
	// Otherwise the charge time is the amount it was held minus the amount of time
	// that has elapsed since it was released.
	else
	{
		float flTimeSinceRelease = gpGlobals->curtime - m_flTriggerReleased;
		float flTimeHeld = m_flTriggerReleased - m_flTriggerPressed;

		m_flChargeTime = flTimeHeld - flTimeSinceRelease;

		if (m_flChargeTime < 0)
			m_flChargeTime = 0;
	}

	// They might have overheated.
	// Manufacture a smooth chargetime reduction
	if (m_flNextSecondaryAttack > gpGlobals->curtime)
	{
		m_flChargeTime = ffdev_ac_maxchargetime.GetFloat() * ((m_flNextSecondaryAttack - gpGlobals->curtime) / ffdev_ac_overheatdelay.GetFloat());
	}

#ifdef CLIENT_DLL

	// create a little buffer so some client stuff can be more smooth
	if (m_flChargeTimeBufferedNextUpdate <= gpGlobals->curtime)
	{
		m_flChargeTimeBufferedNextUpdate = gpGlobals->curtime + FF_AC_CHARGETIMEBUFFERED_UPDATEINTERVAL;
		m_flChargeTimeBuffered = m_flChargeTime;
	}

#endif
}

void CFFWeaponAssaultCannon::ItemPostFrame()
{
	CFFPlayer *pOwner = ToFFPlayer(GetOwner());

	if (!pOwner)
		return;

	// weapon was JUST deployed
	if (m_flLastTick == m_flDeployTick)
		m_flNextSecondaryAttack = m_flDeployTick;

	// The time since we last thought
	float flTimeDelta = gpGlobals->curtime - m_flLastTick;

	// Keep track of fire duration for anywhere else it may be needed
	UpdateChargeTime();

#ifdef CLIENT_DLL
	UpdateBarrelRotation();
#endif

	// update pitch and volume of the loop shot sound (might make it only update the volume)
	PlayLoopShotSound();

	float flTimeSinceRelease = gpGlobals->curtime - m_flTriggerReleased;

	// Player is holding down fire. Don't allow it if we're still recovering from an overheat though
	if ((flTimeSinceRelease <= 0.5f || pOwner->m_nButtons & IN_ATTACK) && m_flNextSecondaryAttack <= gpGlobals->curtime)
	{
		// Oh no...
		if (m_flChargeTime > ffdev_ac_maxchargetime.GetFloat())
		{
			// Freeze for 5s, reduce to max rev sound so it falls away instantly
			m_flNextSecondaryAttack = gpGlobals->curtime + ffdev_ac_overheatdelay.GetFloat();
			m_flTriggerPressed = gpGlobals->curtime + ffdev_ac_overheatdelay.GetFloat();
			m_flTriggerReleased = 0; //gpGlobals->curtime;
			
			// Play the overheat sound
			WeaponSound(SPECIAL3);

			StopLoopShotSound();

#ifdef CLIENT_DLL
			StopBarrelRotationSound();
#endif

#ifdef CLIENT_DLL
			FF_SendHint( HWGUY_OVERHEAT, 5, PRIORITY_NORMAL, "#FF_HINT_HWGUY_OVERHEAT" );
#endif

#ifdef GAME_DLL
			// Remember to reset the speed soon
			pOwner->AddSpeedEffect(SE_ASSAULTCANNON, 0.5f, FF_AC_SPEEDEFFECT_MIN, SEM_BOOLEAN);
#endif

			m_bFiring = false;
		}

		// Time for the next real fire think
		else if (m_flChargeTime >= ffdev_ac_chargeuptime.GetFloat() && m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
			// Out of ammo
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			{
				//WeaponSound(STOP);
				StopLoopShotSound();

#ifdef CLIENT_DLL
				StopBarrelRotationSound();
#endif

				HandleFireOnEmpty();
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;

#ifdef GAME_DLL
				pOwner->RemoveSpeedEffect(SE_ASSAULTCANNON);
#endif

				m_bFiring = false;
			}
			// Weapon should be firing now
			else if(m_flDeployTick + 0.5f <= gpGlobals->curtime)
			{
				// If the firing button was just pressed, reset the firing time
				if (pOwner && pOwner->m_afButtonPressed & IN_ATTACK)
					m_flNextPrimaryAttack = gpGlobals->curtime;

#ifdef GAME_DLL
				// base the speed effect on how charged the ac is
				float flSpeed = FF_AC_SPEEDEFFECT_MAX - ( (FF_AC_SPEEDEFFECT_MAX - FF_AC_SPEEDEFFECT_MIN) * (m_flChargeTime / ffdev_ac_maxchargetime.GetFloat()) );
				CFFPlayer *pPlayer = GetPlayerOwner();
				pPlayer->AddSpeedEffect(SE_ASSAULTCANNON, 0.5f, flSpeed, SEM_BOOLEAN);
#endif

				m_flPlaybackRate = 1.0f + m_flChargeTime;
				PrimaryAttack();

				m_bFiring = true;
			}

			//m_flNextPrimaryAttack = gpGlobals->curtime + 1.0f;

			//float flT = (m_flChargeTime - ffdev_ac_chargeuptime.GetFloat()) / (2.0f * ffdev_ac_maxchargetime.GetFloat());

			//m_flNextPrimaryAttack = gpGlobals->curtime + (GetFFWpnData().m_flCycleTime * (flT > 0.0f ? 1.0f : 1 - flT));

			if (!m_bFiring && m_flChargeTime > ffdev_ac_chargeuptime.GetFloat())
			{
				//WeaponSound(SINGLE);

#ifdef GAME_DLL
				CFFPlayer *pPlayer = GetPlayerOwner();
				pPlayer->AddSpeedEffect(SE_ASSAULTCANNON, 0.5f, FF_AC_SPEEDEFFECT_MAX, SEM_BOOLEAN);
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
			m_flChargeTime -= flTimeDelta;

			if (m_flChargeTime < 0)
				m_flChargeTime = 0;
		}
		
		if (m_bFiring)
		{
			//WeaponSound(STOP);
			StopLoopShotSound();

#ifdef CLIENT_DLL
			StopBarrelRotationSound();
#endif

#ifdef GAME_DLL
			CFFPlayer *pPlayer = GetPlayerOwner();
			pPlayer->RemoveSpeedEffect(SE_ASSAULTCANNON);
#endif

			m_bFiring = false;
		}

		WeaponIdle();
	}

	m_flLastTick = gpGlobals->curtime;
}

//----------------------------------------------------------------------------
// Purpose: Precache some extra sounds
//----------------------------------------------------------------------------
void CFFWeaponAssaultCannon::Precache() 
{
	PrecacheScriptSound("Assaultcannon.single_shot");
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

	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	float fireRate = GetFireRate();

	// MUST call sound before removing a round from the clip of a CMachineGun

	if ( fireRate > ffdev_ac_rof_loopshotsound_max.GetFloat() )
	{
		// only play the single sound while shooting slow
		WeaponSound(SINGLE);

		// so stop playing the loop sound
		StopLoopShotSound();

#ifdef CLIENT_DLL
		StopBarrelRotationSound();
#endif
	}
	else if ( fireRate > ffdev_ac_rof_loopshotsound_min.GetFloat() )
	{
		// Play both sounds while between slow and fast
		WeaponSound(SINGLE);
		m_bPlayLoopShotSound = true;
	}
	else
	{
		// only play the loop sound while firing fast
		m_bPlayLoopShotSound = true;
	}

	if (m_bMuzzleFlash)
		pPlayer->DoMuzzleFlash();

	SendWeaponAnim(GetPrimaryAttackActivity());

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	int iBulletsToFire = 0;

	// moving up above to test whether or not to play the loop shot sound
	//float fireRate = GetFireRate();

	while ( m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		iBulletsToFire++;
	}

	if (iBulletsToFire == 0)
		return;

	// Make sure we can't fire more bullets that we have
	iBulletsToFire = min(iBulletsToFire, pPlayer->GetAmmoCount(m_iPrimaryAmmoType));

#ifdef GAME_DLL
	pPlayer->RemoveAmmo(iBulletsToFire, m_iPrimaryAmmoType);
#endif

	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward);

	FireBulletsInfo_t info(iBulletsToFire * pWeaponInfo.m_iBullets, 
		pPlayer->Weapon_ShootPosition(), 
		vecForward, 
		Vector(pWeaponInfo.m_flBulletSpread, pWeaponInfo.m_flBulletSpread, pWeaponInfo.m_flBulletSpread), 
		MAX_TRACE_LENGTH, 
		m_iPrimaryAmmoType);
	info.m_pAttacker = pPlayer;
	info.m_iDamage = (iBulletsToFire * pWeaponInfo.m_iBullets) * pWeaponInfo.m_iDamage;

	Vector vecTest = info.m_vecSrc;

#ifdef CLIENT_DLL
	QAngle tmpAngle;
	pPlayer->GetViewModel()->GetAttachment(1, info.m_vecSrc, tmpAngle);
#endif

	pPlayer->FireBullets(info);

	//m_flNextPrimaryAttack = gpGlobals->curtime + GetFFWpnData().m_flCycleTime;

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}

	//Add our view kick in
	pPlayer->ViewPunchReset();
	pPlayer->ViewPunch(QAngle(-GetFFWpnData().m_flRecoilAmount, 0, 0));
}

float CFFWeaponAssaultCannon::GetFireRate()
{
	float t = m_flChargeTime / ffdev_ac_maxchargetime.GetFloat();
	t = clamp(t, 0.0f, 1.0f);
	t = SimpleSpline(t);

	return ffdev_ac_rof_max.GetFloat() * (1.0f - t) + ffdev_ac_rof_min.GetFloat() * t;
}

Vector CFFWeaponAssaultCannon::GetFireSpread()
{
	float t = m_flChargeTime / ffdev_ac_maxchargetime.GetFloat();
	t = clamp(t, 0.0f, 1.0f);
	t = SimpleSpline(t);

	float flSpread = ffdev_ac_spread_min.GetFloat() * (1.0f - t) + ffdev_ac_spread_max.GetFloat() * t;

	return Vector(flSpread, flSpread, flSpread);
}

//-----------------------------------------------------------------------------
// Purpose: plays the looping sound that's played while firing really fast
//-----------------------------------------------------------------------------
void CFFWeaponAssaultCannon::PlayLoopShotSound()
{
	// this function is currently called every frame and this bool is set during certain fire situations
	if (!m_bPlayLoopShotSound)
	{
		StopLoopShotSound();
		return;
	}

	if (!m_LoopShotSoundParams.m_pSoundName || !m_LoopShotSoundParams.m_pSoundName[0])
	{
		const char *shootsound = GetShootSound( m_nLoopShotSound );
		if (!shootsound || !shootsound[0])
			return;

		// setup parameters (Only one time!)
		m_LoopShotSoundParams.m_pSoundName = shootsound;
		m_LoopShotSoundParams.m_flSoundTime = 0.0f;
		m_LoopShotSoundParams.m_pOrigin = NULL;
		m_LoopShotSoundParams.m_pflSoundDuration = NULL;
		m_LoopShotSoundParams.m_bWarnOnDirectWaveReference = true;
		m_LoopShotSoundParams.m_SoundLevel = SNDLVL_NORM;
		m_LoopShotSoundParams.m_nFlags = SND_CHANGE_PITCH | SND_CHANGE_VOL;
	}

	// wait a little while so we're not constantly calling EmitSound
	if (gpGlobals->curtime < m_flLoopShotSoundNextUpdate)
		return;
	m_flLoopShotSoundNextUpdate = gpGlobals->curtime + FF_AC_LOOPSHOTSOUND_UPDATEINTERVAL;

	float flPercentForVolume = 0.0f;
	float flPercentForPitch = 0.0f;

	float flFireRate = GetFireRate();
	float flMax = ffdev_ac_rof_loopshotsound_max.GetFloat();
	float flMin = ffdev_ac_rof_loopshotsound_min.GetFloat();
	if (flMax - flMin != 0)
		flPercentForVolume = clamp((flMax - clamp(flFireRate, flMin, flMax)) / (flMax - flMin), 0.0f, 1.0f);
	SimpleSpline(flPercentForVolume);

	flMax = ffdev_ac_rof_max.GetFloat();
	flMin = ffdev_ac_rof_min.GetFloat();
	if (flMax - flMin != 0)
		flPercentForPitch = clamp((flMax - clamp(flFireRate, flMin, flMax)) / (flMax - flMin), 0.0f, 1.0f);
	SimpleSpline(flPercentForPitch);

	// setup pitch and volume parameters
	m_LoopShotSoundParams.m_flVolume = FF_AC_LOOPSHOTSOUND_VOLUME_LOW + ((FF_AC_LOOPSHOTSOUND_VOLUME_HIGH - FF_AC_LOOPSHOTSOUND_VOLUME_LOW) * flPercentForVolume);
	m_LoopShotSoundParams.m_nPitch = FF_AC_LOOPSHOTSOUND_PITCH_LOW + ((FF_AC_LOOPSHOTSOUND_PITCH_HIGH - FF_AC_LOOPSHOTSOUND_PITCH_LOW) * flPercentForPitch);

	// have no idea, but this is done elsewhere in Source when emitting a sound
	CPASAttenuationFilter filter( GetOwner(), m_LoopShotSoundParams.m_SoundLevel );
	if ( IsPredicted() )
	{
		filter.UsePredictionRules();
	}

	// play the sound
	EmitSound( filter, entindex(), m_LoopShotSoundParams, m_LoopShotSoundParams.m_hSoundScriptHandle );

	// we're playing now...in case nobody knows for some reason
	m_bPlayLoopShotSound = true;
}

//-----------------------------------------------------------------------------
// Purpose: stops the looping sound that's played while firing really fast
//-----------------------------------------------------------------------------
void CFFWeaponAssaultCannon::StopLoopShotSound()
{
	if (!m_LoopShotSoundParams.m_pSoundName || !m_LoopShotSoundParams.m_pSoundName[0])
		return;

	StopSound( entindex(), m_LoopShotSoundParams.m_pSoundName );
	m_bPlayLoopShotSound = false;
	m_flLoopShotSoundNextUpdate = 0.0f;
}

#ifdef CLIENT_DLL

// old revsound junk is gone - barrel rotation stuff now handles the spinning sound
/*
//-----------------------------------------------------------------------------
// Purpose: plays the rev sound (barrel spin)
//-----------------------------------------------------------------------------
void CFFWeaponAssaultCannon::PlayRevSound()
{
	if (!m_RevSoundParams.m_pSoundName || !m_RevSoundParams.m_pSoundName[0])
	{
		const char *shootsound = GetShootSound( m_nRevSound );
		if (!shootsound || !shootsound[0])
			return;

		// setup parameters (Only one time!)
		m_RevSoundParams.m_pSoundName = shootsound;
		m_RevSoundParams.m_flSoundTime = 0.0f;
		m_RevSoundParams.m_pOrigin = NULL;
		m_RevSoundParams.m_pflSoundDuration = NULL;
		m_RevSoundParams.m_bWarnOnDirectWaveReference = true;
		m_RevSoundParams.m_SoundLevel = SNDLVL_NORM;
		m_RevSoundParams.m_nFlags = SND_CHANGE_PITCH | SND_CHANGE_VOL;
	}

	float flPercent = 0.0f;

	float flFireRate = GetFireRate();
	float flMax = ffdev_ac_rof_max.GetFloat();
	float flMin = ffdev_ac_rof_min.GetFloat();
	if (flMax - flMin != 0)
		flPercent = clamp((flMax - clamp(flFireRate, flMin, flMax)) / (flMax - flMin), 0.0f, 1.0f);
	SimpleSpline(flPercent);

	// setup pitch and volume parameters
	// volume and pitch go up and down based on certain variables and whatnot
	// ac is charging/firing
	if (m_flRevSoundStopTick == 0.0f)
	{
		m_RevSoundParams.m_flVolume = FF_AC_REVSOUND_VOLUME_LOW + ((FF_AC_REVSOUND_VOLUME_HIGH - FF_AC_REVSOUND_VOLUME_LOW) * flPercent);
		m_RevSoundParams.m_nPitch = FF_AC_REVSOUND_PITCH_LOW + ((FF_AC_REVSOUND_PITCH_HIGH - FF_AC_REVSOUND_PITCH_LOW) * flPercent);
	}
	// ac is done charging/firing
	else
	{
		m_RevSoundParams.m_flVolume = FF_AC_REVSOUND_VOLUME_LOW * ( (m_flRevSoundStopTick + 1.0f) - gpGlobals->curtime);
		m_RevSoundParams.m_nPitch = (FF_AC_REVSOUND_PITCH_LOW * 0.75) + ( ( FF_AC_REVSOUND_PITCH_LOW - (FF_AC_REVSOUND_PITCH_LOW * 0.75) ) * ( (m_flRevSoundStopTick + 1.0f) - gpGlobals->curtime) ); // min + ( (max - min) * percent )
	}

	// have no idea, but this is done elsewhere in Source when emitting a sound
	if ( IsPredicted() )
	{
		filter.UsePredictionRules();
	}

	// play the sound
	EmitSound( filter, entindex(), m_RevSoundParams, m_RevSoundParams.m_hSoundScriptHandle );

	// just cause
	m_bPlayRevSound = true;
}

//-----------------------------------------------------------------------------
// Purpose: stops the rev sound (barrel spin)
//-----------------------------------------------------------------------------
void CFFWeaponAssaultCannon::StopRevSound()
{
	if (!m_RevSoundParams.m_pSoundName || !m_RevSoundParams.m_pSoundName[0])
		return;

	StopSound( entindex(), m_RevSoundParams.m_pSoundName );

	// reset some variables
	m_bPlayRevSound = false;
	m_flRevSoundNextUpdate = 0.0f;
	m_flRevSoundStopTick = 0.0f;
}
*/

// stop the spinning sound
void CFFWeaponAssaultCannon::StopBarrelRotationSound()
{
	if (m_sndBarrelRotation)
	{
		// fade and die
		(CSoundEnvelopeController::GetController()).SoundFadeOut( m_sndBarrelRotation, 0.05f, true );
		m_sndBarrelRotation = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Keep the barrels spinning
//-----------------------------------------------------------------------------
void CFFWeaponAssaultCannon::UpdateBarrelRotation()
{
	CFFPlayer *pOwner = ToFFPlayer(GetOwner());
	if (!pOwner)
		return;

	CBaseViewModel *pVM = pOwner->GetViewModel();
	if (!pVM)
		return;

	// init when still set as the default value
	if (m_iBarrelRotation == -69)
		m_iBarrelRotation = pVM->LookupPoseParameter("ac_rotate");

	// rotate when not the default value
	if (m_iBarrelRotation != -69)
	{
		// weapon was JUST deployed
		if (m_flLastTick == m_flDeployTick)
			// so get the current rotation
			m_flBarrelRotationValue = pVM->GetPoseParameter(m_iBarrelRotation);

		// the barrel is spinning
		if (m_flChargeTimeBuffered > 0 || m_bFiring)
		{
			m_flBarrelRotationDelta = gpGlobals->frametime * FLerp( FF_AC_BARRELROTATION_SPEED_MIN, FF_AC_BARRELROTATION_SPEED_MAX, m_flChargeTimeBuffered / ffdev_ac_maxchargetime.GetFloat() );
			m_flBarrelRotationStopTimer = 0.0f;
		}
		// barrel needs to stop spinning
		else if (m_flBarrelRotationDelta > 0)
		{
			// take X seconds to stop
			m_flBarrelRotationStopTimer = clamp(m_flBarrelRotationStopTimer + gpGlobals->frametime, 0, 2.5f);

			// smooth transition (FLerp rules)
			m_flBarrelRotationDelta = gpGlobals->frametime * FLerp( FF_AC_BARRELROTATION_SPEED_MIN, 0, m_flBarrelRotationStopTimer / 2.5f );
		}
		else
			// reset the timer
			m_flBarrelRotationStopTimer = 0.0f;

		// don't bother rotating ALL the time
		if (m_flBarrelRotationDelta > 0)
		{
			// smoothness...good
			SimpleSpline(m_flBarrelRotationDelta);

			m_flBarrelRotationValue += m_flBarrelRotationDelta;
			m_flBarrelRotationValue = pVM->SetPoseParameter(m_iBarrelRotation, m_flBarrelRotationValue);

			float flPercent = (m_flBarrelRotationDelta / gpGlobals->frametime) / FF_AC_BARRELROTATION_SPEED_MAX;
			float flVolume = FLerp( FF_AC_BARRELROTATIONSOUND_VOLUME_LOW, FF_AC_BARRELROTATIONSOUND_VOLUME_HIGH, flPercent );
			float flPitch = FLerp( FF_AC_BARRELROTATIONSOUND_PITCH_LOW, FF_AC_BARRELROTATIONSOUND_PITCH_HIGH, flPercent );

			// setup and play the spinning sound if it's not setup yet
			if (!m_sndBarrelRotation)
			{
				CPASAttenuationFilter filter( this );
				m_sndBarrelRotation = (CSoundEnvelopeController::GetController()).SoundCreate( filter, entindex(), "assaultcannon.rotate" );
				(CSoundEnvelopeController::GetController()).Play( m_sndBarrelRotation, flVolume, flPitch );
			}
			// modify the spinning sound if it exists
			else
			{
				(CSoundEnvelopeController::GetController()).SoundChangeVolume( m_sndBarrelRotation, flVolume, 0.05f );
				(CSoundEnvelopeController::GetController()).SoundChangePitch( m_sndBarrelRotation, flPitch, 0.05f );
			}
		}
		else
		{
			StopBarrelRotationSound();
		}
	}
}

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

	if (!pWeapon || pWeapon->GetWeaponID() != FF_WEAPON_ASSAULTCANNON)
		return 0.0f;

	CFFWeaponAssaultCannon *pAC = (CFFWeaponAssaultCannon *) pWeapon;

	return (100.0f * pAC->m_flChargeTime / ffdev_ac_maxchargetime.GetFloat());
}

#endif
