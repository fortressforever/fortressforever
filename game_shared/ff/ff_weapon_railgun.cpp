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

//ConVar ffdev_railgun_revsound_volume_high("ffdev_railgun_revsound_volume_high", "1.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Railgun Rev Sound High Volume");
#define RAILGUN_REVSOUND_VOLUME_HIGH 1.0f // ffdev_railgun_revsound_volume_high.GetFloat()
//ConVar ffdev_railgun_revsound_volume_low("ffdev_railgun_revsound_volume_low", "0.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Railgun Rev Sound Low Volume");
#define RAILGUN_REVSOUND_VOLUME_LOW 0.0f // ffdev_railgun_revsound_volume_low.GetFloat()
//ConVar ffdev_railgun_revsound_pitch_high("ffdev_railgun_revsound_pitch_high", "208", FCVAR_REPLICATED | FCVAR_CHEAT, "Railgun Rev Sound High Pitch");
#define RAILGUN_REVSOUND_PITCH_HIGH 208 // ffdev_railgun_revsound_pitch_high.GetInt()
//ConVar ffdev_railgun_revsound_pitch_low("ffdev_railgun_revsound_pitch_low", "64", FCVAR_REPLICATED | FCVAR_CHEAT, "Railgun Rev Sound Low Pitch");
#define RAILGUN_REVSOUND_PITCH_LOW 64 // ffdev_railgun_revsound_pitch_low.GetInt()

#ifdef GAME_DLL

//ConVar ffdev_railgun_revsound_updateinterval("ffdev_railgun_revsound_updateinterval", "0.01", FCVAR_REPLICATED | FCVAR_CHEAT, "How much time to wait before updating");
#define RAILGUN_REVSOUND_UPDATEINTERVAL 0.01f // ffdev_railgun_revsound_updateinterval.GetFloat()

//ConVar ffdev_railgun_ammoamount_halfcharge( "ffdev_railgun_ammoamount_halfcharge", "2", FCVAR_REPLICATED | FCVAR_CHEAT, "How much total ammo is lost for a half charge." );
#define RAILGUN_AMMOAMOUNT_HALFCHARGE 2 // ffdev_railgun_ammoamount_halfcharge.GetInt()
//ConVar ffdev_railgun_ammoamount_fullcharge( "ffdev_railgun_ammoamount_fullcharge", "3", FCVAR_REPLICATED | FCVAR_CHEAT, "How much total ammo is lost for a full charge." );
#define RAILGUN_AMMOAMOUNT_FULLCHARGE 3 // ffdev_railgun_ammoamount_fullcharge.GetInt()

//ConVar ffdev_railgun_overchargetime( "ffdev_railgun_overchargetime", "6.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Railgun overcharges at this time, stops charging, and damages player." );
#define RAILGUN_OVERCHARGETIME 6.0f // ffdev_railgun_overchargetime.GetFloat()
//ConVar ffdev_railgun_overchargedamage( "ffdev_railgun_overchargedamage", "0.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Amount of damage an overcharge gives to the player (doubled on full charge)." );
#define RAILGUN_OVERCHARGEDAMAGE 0.0f // ffdev_railgun_overchargedamage.GetFloat()

//ConVar ffdev_railgun_cooldowntime_zerocharge( "ffdev_railgun_cooldowntime_zerocharge", "0.333", FCVAR_REPLICATED | FCVAR_CHEAT, "Cooldown time after firing a non-charged shot." );
#define RAILGUN_COOLDOWNTIME_ZEROCHARGE 0.333f // ffdev_railgun_cooldowntime_zerocharge.GetFloat()
//ConVar ffdev_railgun_cooldowntime_halfcharge( "ffdev_railgun_cooldowntime_halfcharge", "0.333", FCVAR_REPLICATED | FCVAR_CHEAT, "Cooldown time after firing a half-charged shot." );
#define RAILGUN_COOLDOWNTIME_HALFCHARGE 0.333f // ffdev_railgun_cooldowntime_halfcharge.GetFloat()
//ConVar ffdev_railgun_cooldowntime_fullcharge( "ffdev_railgun_cooldowntime_fullcharge", "0.333", FCVAR_REPLICATED | FCVAR_CHEAT, "Cooldown time after firing a full-charged shot." );
#define RAILGUN_COOLDOWNTIME_FULLCHARGE 0.333f // ffdev_railgun_cooldowntime_fullcharge.GetFloat()
//ConVar ffdev_railgun_cooldowntime_overcharge( "ffdev_railgun_cooldowntime_overcharge", "2.333", FCVAR_REPLICATED | FCVAR_CHEAT, "Cooldown time after overcharging." );
#define RAILGUN_COOLDOWNTIME_OVERCHARGE 2.333f // ffdev_railgun_cooldowntime_overcharge.GetFloat()

//ConVar ffdev_rail_speed_min( "ffdev_rail_speed_min", "1800.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Minimum speed of rail" );
#define RAIL_SPEED_MIN 1800.0f // ffdev_rail_speed_min.GetFloat()
//ConVar ffdev_rail_speed_max( "ffdev_rail_speed_max", "3000.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Maximum speed of rail" );
#define RAIL_SPEED_MAX 3000.0f // ffdev_rail_speed_max.GetFloat()
//ConVar ffdev_rail_damage_min( "ffdev_rail_damage_min", "35.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Minimum damage dealt by rail" );
#define RAIL_DAMAGE_MIN 35.0f // ffdev_rail_damage_min.GetFloat()
//ConVar ffdev_rail_damage_max( "ffdev_rail_damage_max", "60.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Maximum damage dealt by rail" );
#define RAIL_DAMAGE_MAX 60.0f // ffdev_rail_damage_max.GetFloat()

//ConVar ffdev_railgun_pushforce_min("ffdev_railgun_pushforce_min", "32.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Minimum force of backwards push (Like the HL Gauss Gun, WOOH YEAH!)");
#define RAILGUN_PUSHFORCE_MIN 32.0f // ffdev_railgun_pushforce_min.GetFloat()
//ConVar ffdev_railgun_pushforce_max("ffdev_railgun_pushforce_max", "64.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Maximum force of backwards push (Like the HL Gauss Gun, WOOH YEAH!)");
#define RAILGUN_PUSHFORCE_MAX 64.0f // ffdev_railgun_pushforce_max.GetFloat()

//ConVar ffdev_railgun_recoil_min("ffdev_railgun_recoil_min", "2", FCVAR_REPLICATED | FCVAR_CHEAT, "Minimum recoil");
#define RAILGUN_RECOIL_MIN 2 // ffdev_railgun_recoil_min.GetInt()
//ConVar ffdev_railgun_recoil_max("ffdev_railgun_recoil_max", "10", FCVAR_REPLICATED | FCVAR_CHEAT, "Minimum recoil");
#define RAILGUN_RECOIL_MAX 10 // ffdev_railgun_recoil_max.GetInt()

//ConVar ffdev_railgun_resupply_interval("ffdev_railgun_resupply_interval", "4.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Resupply every X seconds.");
#define RAILGUN_RESUPPLY_INTERVAL 4.0f // ffdev_railgun_resupply_interval.GetFloat()
//ConVar ffdev_railgun_resupply_rails("ffdev_railgun_resupply_rails", "1", FCVAR_REPLICATED | FCVAR_CHEAT, "Resupply X rails");
#define RAILGUN_RESUPPLY_RAILS 1 // ffdev_railgun_resupply_rails.GetInt()
//ConVar ffdev_railgun_resupply_cells("ffdev_railgun_resupply_cells", "40", FCVAR_REPLICATED | FCVAR_CHEAT, "Resupply X cells on overcharge");
#define RAILGUN_RESUPPLY_CELLS 40 // ffdev_railgun_resupply_cells.GetInt()

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

#ifdef GAME_DLL

	void RailgunEmitSound( const char* szSoundName );

	bool m_bPlayRevSound;
	float m_flRevSoundNextUpdate;
	EmitSound_t m_paramsRevSound;

	float m_flStartTime;
	float m_flLastUpdate;
	int m_iAmmoUsed;

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
	m_nRevSound = SPECIAL1;

#ifdef GAME_DLL

	// -1 means we are not charging
	m_flStartTime = m_flLastUpdate = -1.0f;
	m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;
	m_iAmmoUsed = 0;

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
#ifdef GAME_DLL

	m_flStartTime = m_flLastUpdate = -1.0f;
	m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;
	m_iAmmoUsed = 0;

	m_flRevSoundNextUpdate = 0.0f;

	m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_ZEROCHARGE;

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
#ifdef GAME_DLL

	m_flStartTime = m_flLastUpdate = -1.0f;
	m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;
	m_iAmmoUsed = 0;

	m_flRevSoundNextUpdate = 0.0f;

	// resupply every X seconds with the railgun out
	m_flNextResupply = gpGlobals->curtime + RAILGUN_RESUPPLY_INTERVAL;

	m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_ZEROCHARGE;

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
#ifdef GAME_DLL

	m_flStartTime = m_flLastUpdate = -1.0f;
	m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;
	m_iAmmoUsed = 0;

	m_flRevSoundNextUpdate = 0.0f;

	m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_ZEROCHARGE;

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
#ifdef GAME_DLL

	CFFPlayer *pPlayer = GetPlayerOwner();
	//const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();  
	// Jiggles: Above line removed until we decide on a good base damage value

	if (!pPlayer)
		return;

	Vector vecForward, vecRight, vecUp;
	pPlayer->EyeVectors( &vecForward, &vecRight, &vecUp);
	VectorNormalizeFast( vecForward );

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	//Vector vecSrc = pPlayer->GetLegacyAbsOrigin() + vecForward * 8.0f + vecRight * 5.0f + Vector(0, 1, (pPlayer->GetFlags() & FL_DUCKING) ? 5.0f : 22.0f);

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

	float flPercent = m_flClampedChargeTime / RAILGUN_MAXCHARGETIME;

	// Push them backwards
	pPlayer->ApplyAbsVelocityImpulse(vecForward * -(RAILGUN_PUSHFORCE_MIN + ( (RAILGUN_PUSHFORCE_MAX - RAILGUN_PUSHFORCE_MIN) * flPercent )));

	// Determine Speed of rail projectile by: railspeed = min + [ ( ( max - min ) * chargetime ) / maxchargetime ] 
	float flSpeed = RAIL_SPEED_MIN + ( (RAIL_SPEED_MAX - RAIL_SPEED_MIN) * flPercent );

	// Now determine damage the same way
	float flDamage = RAIL_DAMAGE_MIN + ( (RAIL_DAMAGE_MAX - RAIL_DAMAGE_MIN) * flPercent );

	const int iDamageRadius = 100;
	CFFProjectileRail *pRail = CFFProjectileRail::CreateRail( this, vecSrc, pPlayer->EyeAngles(), pPlayer, flDamage, iDamageRadius, flSpeed, m_flClampedChargeTime );	
	Omnibot::Notify_PlayerShoot(pPlayer, Omnibot::TF_WP_RAILGUN, pRail);

	// play a different sound for a fully charged shot
	if ( m_flClampedChargeTime < RAILGUN_MAXCHARGETIME )
		RailgunEmitSound(GetFFWpnData().aShootSounds[SINGLE]);
	else
		RailgunEmitSound(GetFFWpnData().aShootSounds[WPN_DOUBLE]);

	if (m_bMuzzleFlash)
		pPlayer->DoMuzzleFlash();

	SendWeaponAnim( GetPrimaryAttackActivity() );

	// Player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	pPlayer->ViewPunch( -QAngle( RAILGUN_RECOIL_MIN + ((RAILGUN_RECOIL_MAX - RAILGUN_RECOIL_MIN) * flPercent), 0, 0 ) );

	// cycletime is based on charge level
	if (m_iAmmoUsed < RAILGUN_AMMOAMOUNT_HALFCHARGE || pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0)
		m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_ZEROCHARGE;
	else if (m_iAmmoUsed < RAILGUN_AMMOAMOUNT_FULLCHARGE)
		m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_HALFCHARGE;
	else
		m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_FULLCHARGE;

	// stop the rev sound immediately
	StopRevSound();

	// reset these variables
	m_flStartTime = m_flLastUpdate = -1.0f;
	m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;
	m_iAmmoUsed = 0;

#endif
}

//----------------------------------------------------------------------------
// Purpose: Handle all the chargeup stuff here
//----------------------------------------------------------------------------
void CFFWeaponRailgun::ItemPostFrame( void )
{
#ifdef GAME_DLL

	CFFPlayer *pPlayer = GetPlayerOwner();

	if (!pPlayer)
		return;

	// if we're currently firing, then check to see if we release

	if (pPlayer->m_nButtons & IN_ATTACK)
	{
		CANCEL_IF_BUILDING();

		// in case a spy gives himself a railgun?
		pPlayer->ResetDisguise();

		// Not currently charging, but wanting to start it up
		if (m_iAmmoUsed < 1 && m_flNextPrimaryAttack <= gpGlobals->curtime && pPlayer->GetAmmoCount(GetPrimaryAmmoType()) > 0)
		{
			m_flStartTime = m_flLastUpdate = gpGlobals->curtime;
			m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;

			// remove ammo immediately
			pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );

			// client needs to know, too
			m_iAmmoUsed++;
		}

		if (m_iAmmoUsed > 0)
		{
			m_flTotalChargeTime += gpGlobals->curtime - m_flLastUpdate;
			m_flLastUpdate = gpGlobals->curtime;

			// curtime - start time instead of total time, because start time can change if you don't have enough ammo to get to full charge
			m_flClampedChargeTime = clamp(gpGlobals->curtime - m_flStartTime, 0, RAILGUN_MAXCHARGETIME);

			if (m_flClampedChargeTime > 0.0f && !m_bPlayRevSound)
				m_bPlayRevSound = true;

			PlayRevSound();

			if ( pPlayer->GetAmmoCount(GetPrimaryAmmoType()) > 0 )
			{
				if ( (m_flClampedChargeTime >= RAILGUN_MAXCHARGETIME * 0.5f && m_iAmmoUsed < RAILGUN_AMMOAMOUNT_HALFCHARGE) || (m_flClampedChargeTime >= RAILGUN_MAXCHARGETIME && m_iAmmoUsed < RAILGUN_AMMOAMOUNT_FULLCHARGE) )
				{
					// play a charge sound
					// it's very important that half-charge is SPECIAL2 and full-charge is SPECIAL3 ( as in right after SPECIAL1 [as in 1, 2, 3] )
					RailgunEmitSound(GetFFWpnData().aShootSounds[WeaponSound_t(SPECIAL1 + int(m_flClampedChargeTime))]);

					// remove additional ammo at each charge level
					pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );

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
				m_flClampedChargeTime = RAILGUN_MAXCHARGETIME * 0.5f;
				m_flStartTime = gpGlobals->curtime - m_flClampedChargeTime;
			}

			// check if it's been overcharged
			if (RAILGUN_OVERCHARGETIME <= m_flTotalChargeTime )
			{
				// deal damage
				//pPlayer->TakeDamage( CTakeDamageInfo( this, pPlayer, RAILGUN_OVERCHARGEDAMAGE * int(m_flClampedChargeTime), DMG_SHOCK ) );

				// remove one more rail on overcharge
				pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );

				// give cells on overcharge
				pPlayer->GiveAmmo( RAILGUN_RESUPPLY_CELLS, AMMO_CELLS, true );

				m_flStartTime = m_flLastUpdate = -1.0f;
				m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;

				m_iAmmoUsed = 0;

				m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_OVERCHARGE;

				StopRevSound();

				// play an overcharge sound
				RailgunEmitSound(GetFFWpnData().aShootSounds[BURST]);
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
		m_flTotalChargeTimeBuffered = m_flTotalChargeTime;
		m_flClampedChargeTimeBuffered = m_flClampedChargeTime;
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
	float flTempClampedChargeTime = m_flClampedChargeTime;
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
