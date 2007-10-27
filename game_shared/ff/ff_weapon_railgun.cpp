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

#define RAILGUN_AMMOAMOUNT_HALFCHARGE 3
#define RAILGUN_AMMOAMOUNT_FULLCHARGE 5

ConVar ffdev_railgun_maxchargetime( "ffdev_railgun_maxchargetime", "2.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Maximum charge for railgun" );

ConVar ffdev_railgun_overchargetime( "ffdev_railgun_overchargetime", "6.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Railgun overcharges at this time, stops charging, and damages player." );
ConVar ffdev_railgun_overchargedamage( "ffdev_railgun_overchargedamage", "10.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Amount of damage an overcharge gives to the player (doubled on full charge)." );

ConVar ffdev_railgun_cooldowntime_zerocharge( "ffdev_railgun_cooldowntime_zerocharge", "0.30", FCVAR_REPLICATED | FCVAR_CHEAT, "Cooldown time after firing a half-charged shot." );
ConVar ffdev_railgun_cooldowntime_halfcharge( "ffdev_railgun_cooldowntime_halfcharge", "0.80", FCVAR_REPLICATED | FCVAR_CHEAT, "Cooldown time after firing a half-charged shot." );
ConVar ffdev_railgun_cooldowntime_fullcharge( "ffdev_railgun_cooldowntime_fullcharge", "1.80", FCVAR_REPLICATED | FCVAR_CHEAT, "Cooldown time after firing a full-charged shot." );
ConVar ffdev_railgun_cooldowntime_overcharge( "ffdev_railgun_cooldowntime_overcharge", "2.80", FCVAR_REPLICATED | FCVAR_CHEAT, "Cooldown time after overcharging." );

ConVar ffdev_railgun_revsound_volume_high("ffdev_railgun_revsound_volume_high", "1.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Railgun Rev Sound High Volume");
ConVar ffdev_railgun_revsound_volume_low("ffdev_railgun_revsound_volume_low", "0.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Railgun Rev Sound Low Volume");
ConVar ffdev_railgun_revsound_pitch_high("ffdev_railgun_revsound_pitch_high", "208", FCVAR_REPLICATED | FCVAR_CHEAT, "Railgun Rev Sound High Pitch");
ConVar ffdev_railgun_revsound_pitch_low("ffdev_railgun_revsound_pitch_low", "64", FCVAR_REPLICATED | FCVAR_CHEAT, "Railgun Rev Sound Low Pitch");
ConVar ffdev_railgun_revsound_updateinterval("ffdev_railgun_revsound_updateinterval", "0.01", FCVAR_REPLICATED | FCVAR_CHEAT, "How much time to wait before updating");

ConVar ffdev_rail_speed_min( "ffdev_rail_speed_min", "1800.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Minimum speed of rail" );
ConVar ffdev_rail_speed_max( "ffdev_rail_speed_max", "3000.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Maximum speed of rail" );
ConVar ffdev_rail_damage_min( "ffdev_rail_damage_min", "35.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Minimum damage dealt by rail" );
ConVar ffdev_rail_damage_max( "ffdev_rail_damage_max", "60.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Maximum damage dealt by rail" );

ConVar ffdev_railgun_pushforce_min("ffdev_railgun_pushforce_min", "32.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Minimum force of backwards push (Like the HL Gauss Gun, WOOH YEAH!)");
ConVar ffdev_railgun_pushforce_max("ffdev_railgun_pushforce_max", "64.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Maximum force of backwards push (Like the HL Gauss Gun, WOOH YEAH!)");

ConVar ffdev_railgun_recoil_min("ffdev_railgun_recoil_min", "1", FCVAR_REPLICATED | FCVAR_CHEAT, "Minimum recoil");
ConVar ffdev_railgun_recoil_max("ffdev_railgun_recoil_max", "5", FCVAR_REPLICATED | FCVAR_CHEAT, "Minimum recoil");

#ifdef CLIENT_DLL
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

	virtual void	Fire( void );
	virtual void	ItemPostFrame( void );
	//void			RailBeamEffect( void );
	virtual bool	Deploy( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void	Precache( void );

#ifdef CLIENT_DLL
	virtual void	ViewModelDrawn( C_BaseViewModel *pBaseViewModel );
	virtual RenderGroup_t GetRenderGroup( void ) { return RENDER_GROUP_TRANSLUCENT_ENTITY; }
	virtual bool IsTranslucent( void )			 { return true; }

private:
	int	m_iAttachment1;
	int m_iAttachment2;
#endif	

public:
	virtual FFWeaponID GetWeaponID( void ) const { return FF_WEAPON_RAILGUN; }

	void PlayRevSound();
	void StopRevSound();
	int m_nRevSound;
	bool m_bPlayRevSound;
	float m_flRevSoundNextUpdate;

	float m_flStartTime;
	float m_flLastUpdate;
	int m_iAmmoUsed;

private:
	CFFWeaponRailgun( const CFFWeaponRailgun & );
	CNetworkVar( float, m_flTotalChargeTime );
	CNetworkVar( float, m_flClampedChargeTime );
};

//=============================================================================
// CFFWeaponRailgun tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED( FFWeaponRailgun, DT_FFWeaponRailgun )

BEGIN_NETWORK_TABLE(CFFWeaponRailgun, DT_FFWeaponRailgun )
#ifdef GAME_DLL
	SendPropTime( SENDINFO( m_flTotalChargeTime ) ), 
	SendPropTime( SENDINFO( m_flClampedChargeTime ) ), 
#else
	RecvPropTime( RECVINFO( m_flTotalChargeTime ) ), 
	RecvPropTime( RECVINFO( m_flClampedChargeTime ) ), 
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CFFWeaponRailgun )
	DEFINE_PRED_FIELD_TOL( m_flTotalChargeTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ), 
	DEFINE_PRED_FIELD_TOL( m_flClampedChargeTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ), 
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
	// -1 means we are not charging
	m_flStartTime = m_flLastUpdate = -1.0f;
	m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;
	m_iAmmoUsed = 0;

	m_nRevSound = SPECIAL1;
	m_bPlayRevSound = false;
	m_flRevSoundNextUpdate = 0.0f;

#ifdef CLIENT_DLL
	m_iAttachment1 = m_iAttachment2 = -1;
#endif
}

//----------------------------------------------------------------------------
// Purpose: Deploy
//----------------------------------------------------------------------------
bool CFFWeaponRailgun::Deploy( void )
{
	m_flStartTime = m_flLastUpdate = -1.0f;
	m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;
	m_iAmmoUsed = 0;

	m_flRevSoundNextUpdate = 0.0f;

	StopRevSound();

	return BaseClass::Deploy();
}

//----------------------------------------------------------------------------
// Purpose: Holster
//----------------------------------------------------------------------------
bool CFFWeaponRailgun::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_flStartTime = m_flLastUpdate = -1.0f;
	m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;
	m_iAmmoUsed = 0;

	m_flRevSoundNextUpdate = 0.0f;

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
	PrecacheScriptSound( "ambient.electrical_zap_9" );	// SPECIAL2 - half charge notification
	PrecacheScriptSound( "ambient.electrical_zap_9" );	// SPECIAL3 - full charge notification
	PrecacheScriptSound( "ambient.electrical_zap_5" );	// BURST - overcharge

	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Fire a rail
//----------------------------------------------------------------------------
void CFFWeaponRailgun::Fire( void )
{
	// stop the rev sound immediately
	StopRevSound();

	CFFPlayer *pPlayer = GetPlayerOwner();
	//const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();  
	// Jiggles: Above line removed until we decide on a good base damage value

	if (!pPlayer)
		return;

	Vector vecForward, vecRight, vecUp;
	pPlayer->EyeVectors( &vecForward, &vecRight, &vecUp);
	VectorNormalizeFast( vecForward );

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	//Vector vecSrc = pPlayer->GetLegacyAbsOrigin() + vecForward * 16.0f + vecRight * 5.0f + Vector(0, 1, (pPlayer->GetFlags() & FL_DUCKING) ? 5.0f : 23.0f);

//#ifdef CLIENT_DLL
//	// For now, fake the bullet source on the client
//	C_BaseAnimating *pWeapon = NULL;
//
//	// Use the correct weapon model
//	if( pPlayer->IsLocalPlayer() )
//		pWeapon = pPlayer->GetViewModel(0);
//	else
//		pWeapon = pPlayer->GetActiveWeapon();
//
//	// Get the attachment
//	// FF TODO: precache/define this attachment string sometime
//	if (pWeapon)
//		pWeapon->GetAttachment( pWeapon->LookupAttachment( "1" ), vecSrc, QAngle(0,0,0) ); // passing dummy angles, because they don't matter
//	else
//		AssertMsg( 0, "Couldn't get weapon railgun!" );
//#endif

	// Check that this isn't going through a wall
	//trace_t tr;
	//UTIL_TraceLine(pPlayer->EyePosition(), vecSrc /*+ ( vecForward * 4.0f )*/, MASK_SOLID_BRUSHONLY, pPlayer, COLLISION_GROUP_NONE, &tr);

	//// Yes, going through a wall
	//if (tr.fraction < 1.0f)
	//{
	//	// Drag backwards
	//	trace_t tr2;
	//	UTIL_TraceLine(tr.endpos - vecForward * 16.0f, tr.endpos, MASK_SOLID_BRUSHONLY, pPlayer, COLLISION_GROUP_NONE, &tr);
	//	vecSrc = tr2.endpos;
	//}

	float flPercent = m_flClampedChargeTime / ffdev_railgun_maxchargetime.GetFloat();

	// Push them backwards
	pPlayer->ApplyAbsVelocityImpulse(vecForward * -(ffdev_railgun_pushforce_min.GetFloat() + ( (ffdev_railgun_pushforce_max.GetFloat() - ffdev_railgun_pushforce_min.GetFloat()) * flPercent )));

	// Determine Speed of rail projectile by: railspeed = min + [ ( ( max - min ) * chargetime ) / maxchargetime ] 
	float flSpeed = ffdev_rail_speed_min.GetFloat() + ( (ffdev_rail_speed_max.GetFloat() - ffdev_rail_speed_min.GetFloat()) * flPercent );

	// Now determine damage the same way
	float flDamage = ffdev_rail_damage_min.GetFloat() + ( (ffdev_rail_damage_max.GetFloat() - ffdev_rail_damage_min.GetFloat()) * flPercent );

	const int iDamageRadius = 100;
	CFFProjectileRail *pRail = CFFProjectileRail::CreateRail( this, vecSrc, pPlayer->EyeAngles(), pPlayer, flDamage, iDamageRadius, flSpeed, m_flClampedChargeTime );	
	pRail;

#ifdef GAME_DLL
	Omnibot::Notify_PlayerShoot(pPlayer, Omnibot::TF_WP_RAILGUN, pRail);
#endif

	// play a different sound for a fully charged shot
	if ( m_flClampedChargeTime < ffdev_railgun_maxchargetime.GetFloat() )
		WeaponSound( SINGLE );
	else
		WeaponSound( WPN_DOUBLE );

	if (m_bMuzzleFlash)
		pPlayer->DoMuzzleFlash();

	SendWeaponAnim( GetPrimaryAttackActivity() );

	// Player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	pPlayer->ViewPunch( -QAngle( ffdev_railgun_recoil_min.GetFloat() + ((ffdev_railgun_recoil_max.GetFloat() - ffdev_railgun_recoil_min.GetFloat()) * m_flClampedChargeTime), 0, 0 ) );

	// cycletime is based on charge level
	if (m_iAmmoUsed < RAILGUN_AMMOAMOUNT_HALFCHARGE || pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0)
		m_flNextPrimaryAttack = gpGlobals->curtime + ffdev_railgun_cooldowntime_zerocharge.GetFloat();
	else if (m_iAmmoUsed < RAILGUN_AMMOAMOUNT_FULLCHARGE)
		m_flNextPrimaryAttack = gpGlobals->curtime + ffdev_railgun_cooldowntime_halfcharge.GetFloat();
	else
		m_flNextPrimaryAttack = gpGlobals->curtime + ffdev_railgun_cooldowntime_fullcharge.GetFloat();

	// reset these variables
	m_flStartTime = m_flLastUpdate = -1.0f;
	m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;
	m_iAmmoUsed = 0;
}

//----------------------------------------------------------------------------
// Purpose: Handle all the chargeup stuff here
//----------------------------------------------------------------------------
void CFFWeaponRailgun::ItemPostFrame( void )
{
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	if (!pPlayer)
		return;

	// if we're currently firing, then check to see if we release

	if (pPlayer->m_nButtons & IN_ATTACK)
	{
		CANCEL_IF_BUILDING();

		// Not currently charging, but wanting to start it up
		if (m_iAmmoUsed < 1 && m_flNextPrimaryAttack < gpGlobals->curtime && pPlayer->GetAmmoCount(GetPrimaryAmmoType()) > 0)
		{
			m_flStartTime = m_flLastUpdate = gpGlobals->curtime;
			m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;
			m_bPlayRevSound = true;

#ifdef GAME_DLL
			// remove ammo immediately
			pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );
#endif
			// client needs to know, too
			m_iAmmoUsed++;
		}

		if (m_iAmmoUsed > 0)
		{
			PlayRevSound();

			m_flTotalChargeTime += gpGlobals->curtime - m_flLastUpdate;
			m_flLastUpdate = gpGlobals->curtime;

			float flMaxChargeTime = ffdev_railgun_maxchargetime.GetFloat();
			m_flClampedChargeTime = clamp(gpGlobals->curtime - m_flStartTime, 0, flMaxChargeTime);

			if ( pPlayer->GetAmmoCount(GetPrimaryAmmoType()) > 0 )
			{

				if ( (m_flClampedChargeTime >= flMaxChargeTime / 2 && m_iAmmoUsed < RAILGUN_AMMOAMOUNT_HALFCHARGE ) || (m_flClampedChargeTime >= flMaxChargeTime && m_iAmmoUsed < RAILGUN_AMMOAMOUNT_FULLCHARGE ) )
				{
					// play a charge sound
					// it's very important that half-charge is SPECIAL2 and full-charge is SPECIAL3
					WeaponSound(WeaponSound_t(SPECIAL1 + int(m_flClampedChargeTime)));
#ifdef GAME_DLL
					// remove additional ammo at each charge level
					pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );
#endif
					// client needs to know, too
					m_iAmmoUsed++;
				}
			}
			// autofire if we have no ammo to charge with and aren't already halfway charged (so players can still try to get a good charged shot)
			else if (m_iAmmoUsed < RAILGUN_AMMOAMOUNT_HALFCHARGE)
				Fire();
			else if (m_iAmmoUsed < RAILGUN_AMMOAMOUNT_FULLCHARGE)
			{
				// doing this so you can have 2 ammo and get halfway charged, but won't instantly become fully charged when you get more ammo
				m_flClampedChargeTime = flMaxChargeTime / 2;
				m_flStartTime = gpGlobals->curtime - m_flClampedChargeTime;
			}

			// check if it's been overcharged
			if (ffdev_railgun_overchargetime.GetFloat() < m_flTotalChargeTime )
			{
#ifdef GAME_DLL
				// deal damage
				pPlayer->TakeDamage( CTakeDamageInfo( this, pPlayer, ffdev_railgun_overchargedamage.GetFloat() * int(m_flClampedChargeTime), DMG_SHOCK ) );
#endif

				StopRevSound();

				// play an overcharge sound
				WeaponSound(BURST); // this might have to stay as EmitSound so it plays even if you die from overcharging
				//EmitSound(GetFFWpnData().aShootSounds[BURST]);

				m_flStartTime = m_flLastUpdate = -1.0f;
				m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;
				m_iAmmoUsed = 0;

				m_flNextPrimaryAttack = gpGlobals->curtime + ffdev_railgun_cooldowntime_overcharge.GetFloat();
			}
		}
		// just a little extra fail-safe shit
		else
			StopRevSound();
	}
	else if( m_iAmmoUsed > 0 )
		Fire();

	// allow players to continue to charge if they've hit the halfway mark
	// and don't make it immediately switch, causing shot sounds to stop
	if (pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0 && m_flNextPrimaryAttack < gpGlobals->curtime && m_iAmmoUsed < RAILGUN_AMMOAMOUNT_HALFCHARGE)
		HandleFireOnEmpty();
}

void CFFWeaponRailgun::PlayRevSound()
{
	if (!m_bPlayRevSound || m_iAmmoUsed < 1 )
	{
		StopRevSound();
		return;
	}

	// wait a little while so we're not constantly calling EmitSound
	if (gpGlobals->curtime < m_flRevSoundNextUpdate)
		return;
	m_flRevSoundNextUpdate = gpGlobals->curtime + ffdev_railgun_revsound_updateinterval.GetFloat();

	const char *shootsound = GetShootSound( m_nRevSound );
	if (!shootsound || !shootsound[0])
		return;

	float flPercent = m_flClampedChargeTime / ffdev_railgun_maxchargetime.GetFloat();

	EmitSound_t params;
	params.m_pSoundName = shootsound;
	params.m_flSoundTime = 0.0f;
	params.m_pOrigin = NULL;
	params.m_pflSoundDuration = NULL;
	params.m_bWarnOnDirectWaveReference = true;
	params.m_SoundLevel = SNDLVL_NORM;
	params.m_flVolume = ffdev_railgun_revsound_volume_low.GetFloat() + ((ffdev_railgun_revsound_volume_high.GetFloat() - ffdev_railgun_revsound_volume_low.GetFloat()) * flPercent);
	params.m_nPitch = ffdev_railgun_revsound_pitch_low.GetFloat() + ((ffdev_railgun_revsound_pitch_high.GetFloat() - ffdev_railgun_revsound_pitch_low.GetFloat()) * flPercent);
	params.m_nFlags = SND_CHANGE_PITCH | SND_CHANGE_VOL;

	CPASAttenuationFilter filter( GetOwner(), params.m_SoundLevel );
	if ( IsPredicted() )
	{
		filter.UsePredictionRules();
	}
	EmitSound( filter, entindex(), params, params.m_hSoundScriptHandle );

	m_bPlayRevSound = true;
}

void CFFWeaponRailgun::StopRevSound()
{
	const char *shootsound = GetShootSound( m_nRevSound );
	if ( !shootsound || !shootsound[0] )
		return;

	StopSound( entindex(), shootsound );
	m_bPlayRevSound = false;
	m_flRevSoundNextUpdate = 0.0f;
}

/*
//----------------------------------------------------------------------------
// Purpose: Set up the actual beam effect
//----------------------------------------------------------------------------
void CFFWeaponRailgun::RailBeamEffect( void )
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	if (!pPlayer)
		return;

	CEffectData data;
	data.m_flScale = 1.0f;

#ifdef GAME_DLL
	data.m_nEntIndex = pPlayer->entindex();
#else
	data.m_hEntity = pPlayer;
#endif

	DispatchEffect("RailBeam", data);
}
*/

#ifdef CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose: This takes place after the viewmodel is drawn. We use this to
//			create the glowing glob of stuff inside the railgun and the faint
//			glow at the barrel.
//-----------------------------------------------------------------------------
void CFFWeaponRailgun::ViewModelDrawn( C_BaseViewModel *pBaseViewModel )
{
	// Not charging at all or even much, so no need to draw shit
	if (m_iAmmoUsed < 1 && m_flClampedChargeTime < 0.1)
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

	float flChargeAmount = clamp( m_flClampedChargeTime / ffdev_railgun_maxchargetime.GetFloat(), 0.0f, 1.0f);
	flChargeAmount = sqrtf( flChargeAmount );

#define FLUTTER_AMOUNT	(1.0f * flChargeAmount)

	// Haha, clean this up pronto!
	Vector vecControl = (vecStart + vecEnd) * 0.5f + Vector(random->RandomFloat(-FLUTTER_AMOUNT, FLUTTER_AMOUNT), random->RandomFloat(-FLUTTER_AMOUNT, FLUTTER_AMOUNT), random->RandomFloat(-FLUTTER_AMOUNT, FLUTTER_AMOUNT));

	float flScrollOffset = gpGlobals->curtime - (int) gpGlobals->curtime;

	IMaterial *pMat = materials->FindMaterial("sprites/physbeam", TEXTURE_GROUP_CLIENT_EFFECTS);
	materials->Bind(pMat);

	DrawBeamQuadratic(vecStart, vecControl, vecEnd, 2.0f * flChargeAmount, Vector(0.51f, 0.89f, 0.95f), flScrollOffset);

	float colour[3] = { 0.51f, 0.89f, 0.95f };

	pMat = materials->FindMaterial("effects/stunstick", TEXTURE_GROUP_CLIENT_EFFECTS);
	materials->Bind(pMat);
	
	DrawHalo(pMat, vecMuzzle, 1.9f * flChargeAmount, colour);
}
#endif
