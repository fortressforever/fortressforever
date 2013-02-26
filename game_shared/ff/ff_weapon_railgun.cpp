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
//
//	11/11/2006 Mulchman: Reverting back to bouncy rail

#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"
#include "ff_projectile_rail.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponRailgun C_FFWeaponRailgun

	#include "soundenvelope.h"
	#include "c_ff_player.h"
	//#include "c_te_effect_dispatch.h"
	
	#include "beamdraw.h"

	extern void FormatViewModelAttachment( Vector &vOrigin, bool bInverse );
	//extern void DrawHalo(IMaterial* pMaterial, const Vector &source, float scale, float const *color, float flHDRColorScale);
#else
	#include "omnibot_interface.h"
	#include "ff_player.h"
	#include "te_effect_dispatch.h"
#endif

extern unsigned char g_uchRailColors[3][3];

//ConVar ffdev_railgun_revsound_volume_high("ffdev_railgun_revsound_volume_high", "1.0", FCVAR_FF_FFDEV_REPLICATED, "Railgun Rev Sound High Volume");
#define RAILGUN_REVSOUND_VOLUME_HIGH 1.0f // ffdev_railgun_revsound_volume_high.GetFloat()
//ConVar ffdev_railgun_revsound_volume_low("ffdev_railgun_revsound_volume_low", "0.0", FCVAR_FF_FFDEV_REPLICATED, "Railgun Rev Sound Low Volume");
#define RAILGUN_REVSOUND_VOLUME_LOW 0.0f // ffdev_railgun_revsound_volume_low.GetFloat()
//ConVar ffdev_railgun_revsound_pitch_high("ffdev_railgun_revsound_pitch_high", "208", FCVAR_FF_FFDEV_REPLICATED, "Railgun Rev Sound High Pitch");
#define RAILGUN_REVSOUND_PITCH_HIGH 208 // ffdev_railgun_revsound_pitch_high.GetInt()
//ConVar ffdev_railgun_revsound_pitch_low("ffdev_railgun_revsound_pitch_low", "64", FCVAR_FF_FFDEV_REPLICATED, "Railgun Rev Sound Low Pitch");
#define RAILGUN_REVSOUND_PITCH_LOW 64 // ffdev_railgun_revsound_pitch_low.GetInt()


//ConVar ffdev_railgun_revsound_updateinterval("ffdev_railgun_revsound_updateinterval", "0.01", FCVAR_FF_FFDEV_REPLICATED, "How much time to wait before updating");
#define RAILGUN_REVSOUND_UPDATEINTERVAL 0.01f // ffdev_railgun_revsound_updateinterval.GetFloat()

//ConVar ffdev_railgun_ammoamount_halfcharge( "ffdev_railgun_ammoamount_halfcharge", "2", FCVAR_FF_FFDEV_REPLICATED, "How much total ammo is lost for a half charge." );
#define RAILGUN_AMMOAMOUNT_HALFCHARGE 2 // ffdev_railgun_ammoamount_halfcharge.GetInt()
//ConVar ffdev_railgun_ammoamount_fullcharge( "ffdev_railgun_ammoamount_fullcharge", "3", FCVAR_FF_FFDEV_REPLICATED, "How much total ammo is lost for a full charge." );
#define RAILGUN_AMMOAMOUNT_FULLCHARGE 3 // ffdev_railgun_ammoamount_fullcharge.GetInt()

//ConVar ffdev_railgun_overchargetime( "ffdev_railgun_overchargetime", "6.0", FCVAR_FF_FFDEV_REPLICATED, "Railgun overcharges at this time, stops charging, and damages player." );
#define RAILGUN_OVERCHARGETIME 6.0f // ffdev_railgun_overchargetime.GetFloat()
//ConVar ffdev_railgun_overchargedamage( "ffdev_railgun_overchargedamage", "0.0", FCVAR_FF_FFDEV_REPLICATED, "Amount of damage an overcharge gives to the player (doubled on full charge)." );
#define RAILGUN_OVERCHARGEDAMAGE 0.0f // ffdev_railgun_overchargedamage.GetFloat()

//ConVar ffdev_railgun_cooldowntime_zerocharge( "ffdev_railgun_cooldowntime_zerocharge", "0.333", FCVAR_FF_FFDEV_REPLICATED, "Cooldown time after firing a non-charged shot." );
#define RAILGUN_COOLDOWNTIME_ZEROCHARGE 0.333f // ffdev_railgun_cooldowntime_zerocharge.GetFloat()
//ConVar ffdev_railgun_cooldowntime_halfcharge( "ffdev_railgun_cooldowntime_halfcharge", "0.333", FCVAR_FF_FFDEV_REPLICATED, "Cooldown time after firing a half-charged shot." );
#define RAILGUN_COOLDOWNTIME_HALFCHARGE 0.333f // ffdev_railgun_cooldowntime_halfcharge.GetFloat()
//ConVar ffdev_railgun_cooldowntime_fullcharge( "ffdev_railgun_cooldowntime_fullcharge", "0.333", FCVAR_FF_FFDEV_REPLICATED, "Cooldown time after firing a full-charged shot." );
#define RAILGUN_COOLDOWNTIME_FULLCHARGE 0.333f // ffdev_railgun_cooldowntime_fullcharge.GetFloat()
//ConVar ffdev_railgun_cooldowntime_overcharge( "ffdev_railgun_cooldowntime_overcharge", "2.333", FCVAR_FF_FFDEV_REPLICATED, "Cooldown time after overcharging." );
#define RAILGUN_COOLDOWNTIME_OVERCHARGE 2.333f // ffdev_railgun_cooldowntime_overcharge.GetFloat()

//ConVar ffdev_rail_speed_min( "ffdev_rail_speed_min", "1800.0", FCVAR_FF_FFDEV_REPLICATED, "Minimum speed of rail" );
#define RAIL_SPEED_MIN 1800.0f // ffdev_rail_speed_min.GetFloat()
//ConVar ffdev_rail_speed_max( "ffdev_rail_speed_max", "3000.0", FCVAR_FF_FFDEV_REPLICATED, "Maximum speed of rail" );
#define RAIL_SPEED_MAX 3000.0f // ffdev_rail_speed_max.GetFloat()
//ConVar ffdev_rail_damage_min( "ffdev_rail_damage_min", "35.0", FCVAR_FF_FFDEV_REPLICATED, "Minimum damage dealt by rail" );
#define RAIL_DAMAGE_MIN 35.0f // ffdev_rail_damage_min.GetFloat()
//ConVar ffdev_rail_damage_max( "ffdev_rail_damage_max", "60.0", FCVAR_FF_FFDEV_REPLICATED, "Maximum damage dealt by rail" );
#define RAIL_DAMAGE_MAX 60.0f // ffdev_rail_damage_max.GetFloat()

//ConVar ffdev_railgun_pushforce_min("ffdev_railgun_pushforce_min", "32.0", FCVAR_FF_FFDEV_REPLICATED, "Minimum force of backwards push (Like the HL Gauss Gun, WOOH YEAH!)");
#define RAILGUN_PUSHFORCE_MIN 32.0f // ffdev_railgun_pushforce_min.GetFloat()
//ConVar ffdev_railgun_pushforce_max("ffdev_railgun_pushforce_max", "64.0", FCVAR_FF_FFDEV_REPLICATED, "Maximum force of backwards push (Like the HL Gauss Gun, WOOH YEAH!)");
#define RAILGUN_PUSHFORCE_MAX 64.0f // ffdev_railgun_pushforce_max.GetFloat()

//ConVar ffdev_railgun_recoil_min("ffdev_railgun_recoil_min", "2", FCVAR_FF_FFDEV_REPLICATED, "Minimum recoil");
#define RAILGUN_RECOIL_MIN 2 // ffdev_railgun_recoil_min.GetInt()
//ConVar ffdev_railgun_recoil_max("ffdev_railgun_recoil_max", "10", FCVAR_FF_FFDEV_REPLICATED, "Minimum recoil");
#define RAILGUN_RECOIL_MAX 10 // ffdev_railgun_recoil_max.GetInt()

//ConVar ffdev_railgun_resupply_interval("ffdev_railgun_resupply_interval", "4.0", FCVAR_FF_FFDEV_REPLICATED, "Resupply every X seconds.");
#define RAILGUN_RESUPPLY_INTERVAL 4.0f // ffdev_railgun_resupply_interval.GetFloat()
//ConVar ffdev_railgun_resupply_rails("ffdev_railgun_resupply_rails", "1", FCVAR_FF_FFDEV_REPLICATED, "Resupply X rails");
#define RAILGUN_RESUPPLY_RAILS 1 // ffdev_railgun_resupply_rails.GetInt()
//ConVar ffdev_railgun_resupply_cells("ffdev_railgun_resupply_cells", "40", FCVAR_FF_FFDEV_REPLICATED, "Resupply X cells on overcharge");
#define RAILGUN_RESUPPLY_CELLS 40 // ffdev_railgun_resupply_cells.GetInt()

#ifdef GAME_DLL
#else

#define RAILGUN_CHARGETIMEBUFFERED_UPDATEINTERVAL 0.02f

CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectStunstick )
CLIENTEFFECT_MATERIAL( "effects/stunstick" )
CLIENTEFFECT_REGISTER_END()

#endif

//=============================================================================
// CFFWeaponRailgun
//=============================================================================

class CFFWeaponRailgun : public CFFWeaponBase
{
public:
	DECLARE_CLASS( CFFWeaponRailgun, CFFWeaponBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponRailgun( void );

	virtual void	PrimaryAttack() {}

	virtual bool	Deploy( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void	Precache( void );
	virtual void	Fire( void );
	virtual void	ItemPostFrame( void );
	virtual void	UpdateOnRemove( void );

	virtual FFWeaponID GetWeaponID( void ) const { return FF_WEAPON_RAILGUN; }

	void PlayRevSound();
	void StopRevSound();
	int m_nRevSound;

	int m_iAmmoUsed;
	float m_flStartTime;

#ifdef GAME_DLL

	void RailgunEmitSound( const char* szSoundName );

	bool m_bPlayRevSound;
	float m_flRevSoundNextUpdate;
	EmitSound_t m_paramsRevSound;

	float m_flNextResupply;

#else

	virtual void	ViewModelDrawn( C_BaseViewModel *pBaseViewModel );
	virtual RenderGroup_t GetRenderGroup( void ) { return RENDER_GROUP_TRANSLUCENT_ENTITY; }
	virtual bool IsTranslucent( void )			 { return true; }

private:
	CSoundPatch *m_sndRev;
	int	m_iAttachment1;
	int m_iAttachment2;
	float m_flTotalChargeTimeBuffered;
	float m_flClampedChargeTimeBuffered;
	float m_flChargeTimeBufferedNextUpdate;

#endif	

private:
	bool m_bInFire;
	float m_flFireStartTime;

	CFFWeaponRailgun( const CFFWeaponRailgun & );
};

//=============================================================================
// CFFWeaponRailgun tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED( FFWeaponRailgun, DT_FFWeaponRailgun )

BEGIN_NETWORK_TABLE(CFFWeaponRailgun, DT_FFWeaponRailgun )
#ifdef GAME_DLL
#else
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CFFWeaponRailgun )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( ff_weapon_railgun, CFFWeaponRailgun );
PRECACHE_WEAPON_REGISTER( ff_weapon_railgun );

//=============================================================================
// CFFWeaponRailgun implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponRailgun::CFFWeaponRailgun( void )
{
	m_nRevSound = SPECIAL1;
	m_iAmmoUsed = 0;
	m_flStartTime = 0.0f;

#ifdef GAME_DLL

	m_bPlayRevSound = false;
	m_flRevSoundNextUpdate = 0.0f;

	m_flNextResupply = 0.0f;

#else

	m_sndRev = NULL;

	m_iAttachment1 = m_iAttachment2 = -1;

	m_flTotalChargeTimeBuffered = m_flClampedChargeTimeBuffered = 0.0f;
	m_flChargeTimeBufferedNextUpdate = 0.0f;

	m_colorMuzzleDLight.r = g_uchRailColors[0][0];
	m_colorMuzzleDLight.g = g_uchRailColors[0][1];
	m_colorMuzzleDLight.b = g_uchRailColors[0][2];

#endif
}

//----------------------------------------------------------------------------
// Purpose: 
//----------------------------------------------------------------------------
void CFFWeaponRailgun::UpdateOnRemove( void )
{
	m_iAmmoUsed = 0;
	m_flStartTime = 0.0f;
	m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_ZEROCHARGE;

#ifdef GAME_DLL

	m_flRevSoundNextUpdate = 0.0f;

#else

	m_flTotalChargeTimeBuffered = m_flClampedChargeTimeBuffered = 0.0f;
	m_flChargeTimeBufferedNextUpdate = 0.0f;

#endif

	StopRevSound();

	BaseClass::UpdateOnRemove();
}

//----------------------------------------------------------------------------
// Purpose: Deploy
//----------------------------------------------------------------------------
bool CFFWeaponRailgun::Deploy( void )
{
	m_iAmmoUsed = 0;
	m_flStartTime = 0.0f;
	m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_ZEROCHARGE;

#ifdef GAME_DLL

	m_flRevSoundNextUpdate = 0.0f;

	// resupply every X seconds with the railgun out
	m_flNextResupply = gpGlobals->curtime + RAILGUN_RESUPPLY_INTERVAL;

#else

	m_flTotalChargeTimeBuffered = m_flClampedChargeTimeBuffered = 0.0f;
	m_flChargeTimeBufferedNextUpdate = 0.0f;

#endif

	StopRevSound();

	return BaseClass::Deploy();
}

//----------------------------------------------------------------------------
// Purpose: Holster
//----------------------------------------------------------------------------
bool CFFWeaponRailgun::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_iAmmoUsed = 0;
	m_flStartTime = 0.0f;
	m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_ZEROCHARGE;

#ifdef GAME_DLL

	m_flRevSoundNextUpdate = 0.0f;

#else

	m_flTotalChargeTimeBuffered = m_flClampedChargeTimeBuffered = 0.0f;
	m_flChargeTimeBufferedNextUpdate = 0.0f;

#endif

	StopRevSound();

	return BaseClass::Holster( pSwitchingTo );
}

//----------------------------------------------------------------------------
// Purpose: Precache
//----------------------------------------------------------------------------
void CFFWeaponRailgun::Precache( void )
{
	PrecacheScriptSound( "railgun.single_shot" );		// SINGLE
	PrecacheScriptSound( "railgun.charged_shot" );		// WPN_DOUBLE
	PrecacheScriptSound( "railgun.chargeloop" );		// SPECIAL1
	PrecacheScriptSound( "railgun.halfcharge" );		// SPECIAL2 - half charge notification
	PrecacheScriptSound( "railgun.fullcharge" );		// SPECIAL3 - full charge notification
	PrecacheScriptSound( "railgun.overcharge" );		// BURST - overcharge

	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Fire a rail
//----------------------------------------------------------------------------
void CFFWeaponRailgun::Fire( void )
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	if (!pPlayer)
		return;

	pPlayer->m_flTrueAimTime = gpGlobals->curtime;

	Vector vecForward, vecRight, vecUp;
	pPlayer->EyeVectors( &vecForward, &vecRight, &vecUp);
	VectorNormalizeFast( vecForward );

	Vector vecSrc = pPlayer->Weapon_ShootPosition();

	float flClampedChargeTime = clamp(gpGlobals->curtime - m_flStartTime, 0, RAILGUN_MAXCHARGETIME);
	float flPercent = flClampedChargeTime / RAILGUN_MAXCHARGETIME;

	// Push them backwards
	pPlayer->ApplyAbsVelocityImpulse(vecForward * -(RAILGUN_PUSHFORCE_MIN + ( (RAILGUN_PUSHFORCE_MAX - RAILGUN_PUSHFORCE_MIN) * flPercent )));
	
	if (m_bMuzzleFlash)
		pPlayer->DoMuzzleFlash();

	SendWeaponAnim( GetPrimaryAttackActivity() );

	//Player "shoot" animation
	pPlayer->DoAnimationEvent(PLAYERANIMEVENT_FIRE_GUN_PRIMARY);

	pPlayer->ViewPunch( -QAngle( RAILGUN_RECOIL_MIN + ((RAILGUN_RECOIL_MAX - RAILGUN_RECOIL_MIN) * flPercent), 0, 0 ) );

	// cycletime is based on charge level
	if (m_iAmmoUsed < RAILGUN_AMMOAMOUNT_HALFCHARGE || pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0)
		m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_ZEROCHARGE;
	else if (m_iAmmoUsed < RAILGUN_AMMOAMOUNT_FULLCHARGE)
		m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_HALFCHARGE;
	else
		m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_FULLCHARGE;

#ifdef GAME_DLL
	// Determine Speed of rail projectile by: railspeed = min + [ ( ( max - min ) * chargetime ) / maxchargetime ] 
	float flSpeed = RAIL_SPEED_MIN + ( (RAIL_SPEED_MAX - RAIL_SPEED_MIN) * flPercent );

	// Now determine damage the same way
	float flDamage = RAIL_DAMAGE_MIN + ( (RAIL_DAMAGE_MAX - RAIL_DAMAGE_MIN) * flPercent );

	const int iDamageRadius = 100;
	CFFProjectileRail *pRail = CFFProjectileRail::CreateRail( this, vecSrc, pPlayer->EyeAngles(), pPlayer, flDamage, iDamageRadius, flSpeed, flClampedChargeTime );	
	Omnibot::Notify_PlayerShoot(pPlayer, Omnibot::TF_WP_RAILGUN, pRail);

	// play a different sound for a fully charged shot
	if ( flClampedChargeTime < RAILGUN_MAXCHARGETIME )
		RailgunEmitSound(GetFFWpnData().aShootSounds[SINGLE]);
	else
		RailgunEmitSound(GetFFWpnData().aShootSounds[WPN_DOUBLE]);

	// stop the rev sound immediately
	StopRevSound();
	
#endif

}

//----------------------------------------------------------------------------
// Purpose: Handle all the chargeup stuff here
//----------------------------------------------------------------------------
void CFFWeaponRailgun::ItemPostFrame( void )
{
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	if (pPlayer->m_nButtons & IN_ATTACK)
	{	
		CANCEL_IF_BUILDING();
	}

	float flTotalChargeTime = gpGlobals->curtime - m_flStartTime;
	float flClampedChargeTime = clamp(gpGlobals->curtime - m_flStartTime, 0, RAILGUN_MAXCHARGETIME);

	if (m_iAmmoUsed <= 0)
	{
		flTotalChargeTime = flClampedChargeTime = 0.0f;
	}
	
	// if we're currently firing, then check to see if we release
	if (m_iAmmoUsed > 0) 
	{
		// Using m_afButtonReleased to catch the button being released rather than
		// just testing for IN_ATTACK not being pressed. This way we don't think we've
		// fird multiple times due to the latency of m_bInFire being changed by server
		//if (! (pPlayer->m_nButtons & IN_ATTACK)) 
		if (pPlayer->m_afButtonReleased & IN_ATTACK)
		{
			// reset the data
			m_iAmmoUsed = 0;

			Fire();
		}
		else
		{

#ifdef GAME_DLL
			if (!m_bPlayRevSound)
				m_bPlayRevSound = true;

			PlayRevSound();
#endif

			if ( pPlayer->GetAmmoCount(GetPrimaryAmmoType()) > 0 )
			{
				if ( (flClampedChargeTime >= RAILGUN_MAXCHARGETIME * 0.5f && m_iAmmoUsed < RAILGUN_AMMOAMOUNT_HALFCHARGE) || (flClampedChargeTime >= RAILGUN_MAXCHARGETIME && m_iAmmoUsed < RAILGUN_AMMOAMOUNT_FULLCHARGE) )
				{
#ifdef GAME_DLL
					// play a charge sound
					// it's very important that half-charge is SPECIAL2 and full-charge is SPECIAL3 ( as in right after SPECIAL1 [as in 1, 2, 3] )
					RailgunEmitSound(GetFFWpnData().aShootSounds[WeaponSound_t(SPECIAL1 + int(flClampedChargeTime))]);

					// remove additional ammo at each charge level
					pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );
#endif

					// client needs to know, too
					m_iAmmoUsed++;
				}
			}
			// autofire if we have no ammo to charge with and aren't already halfway charged (so players can still try to get a good charged shot)
			else if (m_iAmmoUsed < RAILGUN_AMMOAMOUNT_HALFCHARGE)
			{
				m_iAmmoUsed = 0;
				Fire();
			}
			else if (m_iAmmoUsed < RAILGUN_AMMOAMOUNT_FULLCHARGE)
			{
				// doing this so you can have 2 ammo and get halfway charged, but won't instantly become fully charged when you get more ammo
				flClampedChargeTime = RAILGUN_MAXCHARGETIME * 0.5f;
				m_flStartTime = gpGlobals->curtime - flClampedChargeTime;
			}

			// check if it's been overcharged
			if (RAILGUN_OVERCHARGETIME <= flTotalChargeTime )
			{
				m_flStartTime = 0.0f;
				m_iAmmoUsed = 0;
				m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_OVERCHARGE;
				
				StopRevSound();

#ifdef GAME_DLL
				// deal damage
				//pPlayer->TakeDamage( CTakeDamageInfo( this, pPlayer, RAILGUN_OVERCHARGEDAMAGE * int(m_flClampedChargeTime), DMG_SHOCK ) );

				// remove one more rail on overcharge
				pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );

				// give cells on overcharge
				pPlayer->GiveAmmo( RAILGUN_RESUPPLY_CELLS, AMMO_CELLS, true );

				// play an overcharge sound
				RailgunEmitSound(GetFFWpnData().aShootSounds[BURST]);
#endif
			}
		}
	}
	else
	{
		if (pPlayer->m_nButtons & IN_ATTACK && m_flNextPrimaryAttack <= gpGlobals->curtime && pPlayer->GetAmmoCount(GetPrimaryAmmoType()) > 0) 
		{
			// set up us the variables!
			m_iAmmoUsed = 1;
			m_flStartTime = gpGlobals->curtime;

#ifdef GAME_DLL
			// remove ammo immediately
			pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );
#endif
		}
	}

	if (!(pPlayer->m_nButtons & IN_ATTACK))
		WeaponIdle();


#ifdef GAME_DLL
	// allow players to continue to charge if they've hit the halfway mark
	// and don't make it immediately switch, causing shot sounds to stop
	if (pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0 && m_flNextPrimaryAttack < gpGlobals->curtime && m_iAmmoUsed < RAILGUN_AMMOAMOUNT_HALFCHARGE)
	{
		HandleFireOnEmpty();

		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}

	// time to resupply?
	if (m_flNextResupply <= gpGlobals->curtime)
	{
		// give ammo already, GOSH!
		pPlayer->GiveAmmo( RAILGUN_RESUPPLY_RAILS, m_iPrimaryAmmoType, true );

		// resupply every X seconds with the railgun out
		m_flNextResupply = gpGlobals->curtime + RAILGUN_RESUPPLY_INTERVAL;
	}

#else // ^^ GAME_DLL ^^

	// create a little buffer so some client stuff can be more smooth
	if (m_flChargeTimeBufferedNextUpdate <= gpGlobals->curtime)
	{
		m_flChargeTimeBufferedNextUpdate = gpGlobals->curtime + RAILGUN_CHARGETIMEBUFFERED_UPDATEINTERVAL;
		m_flTotalChargeTimeBuffered = flTotalChargeTime;
		m_flClampedChargeTimeBuffered = flClampedChargeTime;
	}

	PlayRevSound();

#endif
}

#ifdef GAME_DLL
void CFFWeaponRailgun::RailgunEmitSound( const char *szSoundName )
{
	CSoundParameters params;
	if ( !CBaseEntity::GetParametersForSound( szSoundName, params, NULL ) || !GetOwner() )
		return;

	CPASAttenuationFilter filter( GetOwner(), params.soundlevel );
	EmitSound_t ep;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = params.volume;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nPitch = params.pitch;
	GetOwner()->EmitSound( filter, GetOwner()->entindex(), ep ); // This needs to be client predicted, can probably just use EmitSoundShared instead of this.. - AfterShock
}
#endif

void CFFWeaponRailgun::PlayRevSound()
{
	// if not charging
	if (m_iAmmoUsed <= 0)
	{
		StopRevSound();
		return;
	}

	float flClampedChargeTime = clamp(gpGlobals->curtime - m_flStartTime, 0, RAILGUN_MAXCHARGETIME);

	float flTempClampedChargeTime = flClampedChargeTime;
#ifdef CLIENT_DLL
	flTempClampedChargeTime = m_flClampedChargeTimeBuffered;
#endif

	float flPercent = clamp( flTempClampedChargeTime / RAILGUN_MAXCHARGETIME, 0.0f, 1.0f);
	float flVolume = RAILGUN_REVSOUND_VOLUME_LOW + ((RAILGUN_REVSOUND_VOLUME_HIGH - RAILGUN_REVSOUND_VOLUME_LOW) * flPercent);
	float flPitch = RAILGUN_REVSOUND_PITCH_LOW + ((RAILGUN_REVSOUND_PITCH_HIGH - RAILGUN_REVSOUND_PITCH_LOW) * flPercent);

#ifdef GAME_DLL

	if (!m_bPlayRevSound || m_iAmmoUsed < 1 )
	{
		StopRevSound();
		return;
	}

	// wait a little while so we're not constantly calling EmitSound
	if (gpGlobals->curtime < m_flRevSoundNextUpdate)
		return;
	m_flRevSoundNextUpdate = gpGlobals->curtime + RAILGUN_REVSOUND_UPDATEINTERVAL;

	const char *shootsound = GetShootSound( m_nRevSound );
	if (!shootsound || !shootsound[0])
		return;

	if (!m_paramsRevSound.m_pSoundName)
	{
		m_paramsRevSound.m_pSoundName = shootsound;
		m_paramsRevSound.m_flSoundTime = 0.0f;
		m_paramsRevSound.m_pOrigin = NULL;
		m_paramsRevSound.m_pflSoundDuration = NULL;
		m_paramsRevSound.m_bWarnOnDirectWaveReference = true;
		m_paramsRevSound.m_SoundLevel = SNDLVL_NORM;
		m_paramsRevSound.m_nFlags = SND_CHANGE_PITCH | SND_CHANGE_VOL;
	}

	m_paramsRevSound.m_flVolume = flVolume;
	m_paramsRevSound.m_nPitch = flPitch;

	CPASAttenuationFilter filter( this, m_paramsRevSound.m_SoundLevel );
	if ( IsPredicted() )
	{
		filter.UsePredictionRules();
	}
	EmitSound( filter, entindex(), m_paramsRevSound, m_paramsRevSound.m_hSoundScriptHandle );

	m_bPlayRevSound = true;

#else

	if ( flPercent == 0.0f )
	{
		StopRevSound();
		return;
	}

	// setup and play the rev sound if it's not setup yet
	if (!m_sndRev)
	{
		const char *shootsound = GetShootSound( m_nRevSound );
		if (!shootsound || !shootsound[0])
			return;

		CPASAttenuationFilter filter( this );
		m_sndRev = (CSoundEnvelopeController::GetController()).SoundCreate( filter, entindex(), shootsound );
		CSoundEnvelopeController::GetController().Play( m_sndRev, flVolume, flPitch );
	}
	// modify the rev sound if it exists
	else
	{
		CSoundEnvelopeController::GetController().SoundChangeVolume( m_sndRev, flVolume, 0.05f );
		CSoundEnvelopeController::GetController().SoundChangePitch( m_sndRev, flPitch, 0.05f );
	}

#endif
}

void CFFWeaponRailgun::StopRevSound()
{
#ifdef GAME_DLL

	const char *shootsound = GetShootSound( m_nRevSound );
	if ( !shootsound || !shootsound[0] )
		return;

	StopSound( entindex(), shootsound );
	m_bPlayRevSound = false;
	m_flRevSoundNextUpdate = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_ZEROCHARGE - RAILGUN_REVSOUND_UPDATEINTERVAL;

#else

	if (m_sndRev)
	{
		CSoundEnvelopeController::GetController().SoundDestroy(m_sndRev);
		m_sndRev = NULL;
	}

#endif
}

#ifdef CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose: This takes place after the viewmodel is drawn. We use this to
//			create the glowing glob of stuff inside the railgun and the faint
//			glow at the barrel.
//-----------------------------------------------------------------------------
void CFFWeaponRailgun::ViewModelDrawn( C_BaseViewModel *pBaseViewModel )
{
	// Not charging at all or even much, so no need to draw shit
	if (m_flClampedChargeTimeBuffered <= 0.0)
		return;

	// We'll get these done and out of the way
	if (m_iAttachment1 == -1 || m_iAttachment2 == -1)
	{
		m_iAttachment1 = pBaseViewModel->LookupAttachment("railgunFX1");
		m_iAttachment2 = pBaseViewModel->LookupAttachment("railgunFX2");
	}

	Vector vecStart, vecEnd, vecMuzzle;
	QAngle tmpAngle;

	pBaseViewModel->GetAttachment(m_iAttachment1, vecStart, tmpAngle);
	pBaseViewModel->GetAttachment(m_iAttachment2, vecEnd, tmpAngle);
	pBaseViewModel->GetAttachment(1, vecMuzzle, tmpAngle);

	::FormatViewModelAttachment(vecStart, true);
	::FormatViewModelAttachment(vecEnd, true);
	::FormatViewModelAttachment(vecMuzzle, true);

	float flPercent = clamp( m_flClampedChargeTimeBuffered / RAILGUN_MAXCHARGETIME, 0.0f, 1.0f);
	flPercent = sqrtf( flPercent );

	// Haha, clean this up pronto!
	Vector vecControl = (vecStart + vecEnd) * 0.5f + Vector(random->RandomFloat(-flPercent, flPercent), random->RandomFloat(-flPercent, flPercent), random->RandomFloat(-flPercent, flPercent));

	float flScrollOffset = gpGlobals->curtime - (int) gpGlobals->curtime;

	IMaterial *pMat = materials->FindMaterial("sprites/physbeam", TEXTURE_GROUP_CLIENT_EFFECTS);
	materials->Bind(pMat);

	DrawBeamQuadratic(vecStart, vecControl, vecEnd, 2.0f * flPercent, Vector(0.51f, 0.89f, 0.95f), flScrollOffset);

	float colour[3] = { 0.51f, 0.89f, 0.95f };

	pMat = materials->FindMaterial("effects/stunstick", TEXTURE_GROUP_CLIENT_EFFECTS);
	materials->Bind(pMat);
	
	DrawHalo(pMat, vecMuzzle, 1.9f * flPercent, colour);
}
#endif
