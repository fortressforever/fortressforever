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
	#include "ff_player.h"
	#include "te_effect_dispatch.h"
#endif

ConVar ffdev_railgun_maxcharge( "ffdev_railgun_maxcharge", "2.0", FCVAR_REPLICATED, "Maximum charge for railgun" );

ConVar ffdev_railgun_revsound_volume_high("ffdev_railgun_revsound_volume_high", "1.0", FCVAR_REPLICATED, "Railgun Rev Sound High Volume");
ConVar ffdev_railgun_revsound_volume_low("ffdev_railgun_revsound_volume_low", "0.0", FCVAR_REPLICATED, "Railgun Rev Sound Low Volume");
ConVar ffdev_railgun_revsound_pitch_high("ffdev_railgun_revsound_pitch_high", "200", FCVAR_REPLICATED, "Railgun Rev Sound High Pitch");
ConVar ffdev_railgun_revsound_pitch_low("ffdev_railgun_revsound_pitch_low", "50", FCVAR_REPLICATED, "Railgun Rev Sound Low Pitch");

ConVar ffdev_rail_speed_min( "ffdev_rail_speed_min", "1100.0", FCVAR_REPLICATED, "Minimum speed of rail" );
ConVar ffdev_rail_speed_max( "ffdev_rail_speed_max", "3300.0", FCVAR_REPLICATED, "Maximum speed of rail" );
ConVar ffdev_rail_damage_min( "ffdev_rail_damage_min", "25.0", FCVAR_REPLICATED, "Minimum damage dealt by rail" );
ConVar ffdev_rail_damage_max( "ffdev_rail_damage_max", "75.0", FCVAR_REPLICATED, "Maximum damage dealt by rail" );

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

	virtual float GetRecoilMultiplier( void );

	void PlayRevSound();
	void StopRevSound();
	int m_nRevSound;
	bool m_bPlayRevSound;

private:
	CFFWeaponRailgun( const CFFWeaponRailgun & );
	CNetworkVar( float, m_flStartCharge );
};

//=============================================================================
// CFFWeaponRailgun tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED( FFWeaponRailgun, DT_FFWeaponRailgun )

BEGIN_NETWORK_TABLE(CFFWeaponRailgun, DT_FFWeaponRailgun )
#ifdef GAME_DLL
	SendPropTime( SENDINFO( m_flStartCharge ) ), 
#else
	RecvPropTime( RECVINFO( m_flStartCharge ) ), 
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CFFWeaponRailgun )
	DEFINE_PRED_FIELD_TOL( m_flStartCharge, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ), 
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
	m_flStartCharge = -1.0f;

	m_nRevSound = SPECIAL1;
	m_bPlayRevSound = false;

#ifdef CLIENT_DLL
	m_iAttachment1 = m_iAttachment2 = -1;
#endif
}

//----------------------------------------------------------------------------
// Purpose: Deploy
//----------------------------------------------------------------------------
bool CFFWeaponRailgun::Deploy( void )
{
	m_flStartCharge = -1.0f;

	StopRevSound();

	return BaseClass::Deploy();
}

//----------------------------------------------------------------------------
// Purpose: Holster
//----------------------------------------------------------------------------
bool CFFWeaponRailgun::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	StopRevSound();

	m_flStartCharge = -1.0f;

	return BaseClass::Holster( pSwitchingTo );
}

//----------------------------------------------------------------------------
// Purpose: Precache
//----------------------------------------------------------------------------
void CFFWeaponRailgun::Precache( void )
{
	PrecacheScriptSound( "railgun.single_shot" );
	PrecacheScriptSound( "railgun.charged_shot" );
	PrecacheScriptSound( "railgun.chargeloop" );
	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Fire a rail
//----------------------------------------------------------------------------
void CFFWeaponRailgun::Fire( void )
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	//const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();  
	// Jiggles: Above line removed until we decide on a good base damage value

	Vector	vecForward;
	pPlayer->EyeVectors( &vecForward );

	Vector	vecSrc = pPlayer->Weapon_ShootPosition();

#ifdef CLIENT_DLL
	// For now, fake the bullet source on the client
	C_BaseAnimating *pWeapon = NULL;

	// Use the correct weapon model
	if( pPlayer->IsLocalPlayer() )
		pWeapon = pPlayer->GetViewModel(0);
	else
		pWeapon = pPlayer->GetActiveWeapon();

	// Get the attachment(precache this number sometime)
	if (pWeapon)
	{
		QAngle angAiming;
		int iAttachment = pWeapon->LookupAttachment( "1" );
		pWeapon->GetAttachment( iAttachment, vecSrc, angAiming );

		AngleVectors( angAiming, &vecForward );
	}
	else
		AssertMsg( 0, "Couldn't get weapon railgun!" );
#endif

	VectorNormalizeFast( vecForward );

	QAngle angAiming;
	VectorAngles( pPlayer->GetAutoaimVector(0), angAiming) ;

	float flChargeTime = clamp( gpGlobals->curtime - m_flStartCharge, 0.0f, ffdev_railgun_maxcharge.GetFloat() );
	float flPercent = flChargeTime / ffdev_railgun_maxcharge.GetFloat();

	// Determine Speed of rail projectile by: railspeed = min + [ ( ( max - min ) * chargetime ) / maxchargetime ] 
	float flSpeed = ffdev_rail_speed_min.GetFloat() + ( (ffdev_rail_speed_max.GetFloat() - ffdev_rail_speed_min.GetFloat()) * flPercent );

	// Now determine damage the same way
	float flDamage = ffdev_rail_damage_min.GetFloat() + ( (ffdev_rail_damage_max.GetFloat() - ffdev_rail_damage_min.GetFloat()) * flPercent );

	//CFFProjectileRail::CreateRail( this, vecSrc, angAiming, pPlayer, pWeaponInfo.m_iDamage, ffdev_rail_minspeed.GetFloat(), flChargeTime );	
	CFFProjectileRail::CreateRail( this, vecSrc, angAiming, pPlayer, flDamage, flSpeed, flChargeTime );	

	// play a different sound for a fully charged shot
	if ( flChargeTime < ffdev_railgun_maxcharge.GetFloat() )
		WeaponSound( SINGLE );
	else
		WeaponSound( WPN_DOUBLE );

	if (m_bMuzzleFlash)
		pPlayer->DoMuzzleFlash();

	SendWeaponAnim( GetPrimaryAttackActivity() );

	// Player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	pPlayer->ViewPunch( -QAngle( 1.0f + flChargeTime, 0, 0 ) );

	// remove the ammo
#ifdef GAME_DLL
	pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );
#endif
	
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFFWpnData().m_flCycleTime;
	m_flStartCharge = -1.0f;
}

//----------------------------------------------------------------------------
// Purpose: Handle all the chargeup stuff here
//----------------------------------------------------------------------------
void CFFWeaponRailgun::ItemPostFrame( void )
{
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	if (!pPlayer)
		return;

	if (pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0)
		HandleFireOnEmpty();

	// if we're currently firing, then check to see if we release

	if (pPlayer->m_nButtons & IN_ATTACK && pPlayer->GetAmmoCount(GetPrimaryAmmoType()) > 0)
	{
		CANCEL_IF_BUILDING();

		// Not currently charging, but wanting to start it up
		if (m_flStartCharge < 0 && m_flNextPrimaryAttack < gpGlobals->curtime)
		{
			m_flStartCharge = gpGlobals->curtime;
			m_bPlayRevSound = true;
		}

		if (m_flStartCharge != -1.0f)
			PlayRevSound();
		else
			StopRevSound();
	}
	else
	{
		if( m_flStartCharge > 0 )
			Fire();

		StopRevSound();

		m_flStartCharge = -1.0f;
	}
}

void CFFWeaponRailgun::PlayRevSound()
{
	if (!m_bPlayRevSound || m_flStartCharge == -1.0f )
	{
		StopRevSound();
		return;
	}

	const char *shootsound = GetShootSound( m_nRevSound );
	if (!shootsound || !shootsound[0])
		return;

	float flPercent = clamp( gpGlobals->curtime - m_flStartCharge, 0.0f, ffdev_railgun_maxcharge.GetFloat() ) / ffdev_railgun_maxcharge.GetFloat();

	EmitSound_t params;
	params.m_pSoundName = shootsound;
	params.m_flSoundTime = 0.0f;
	params.m_pOrigin = NULL;
	params.m_pflSoundDuration = NULL;
	params.m_bWarnOnDirectWaveReference = true;
	params.m_SoundLevel = SNDLVL_NORM;
	params.m_flVolume = ffdev_railgun_revsound_volume_low.GetInt() + ((ffdev_railgun_revsound_volume_high.GetInt() - ffdev_railgun_revsound_volume_low.GetInt()) * flPercent);
	params.m_nPitch = ffdev_railgun_revsound_pitch_low.GetInt() + ((ffdev_railgun_revsound_pitch_high.GetInt() - ffdev_railgun_revsound_pitch_low.GetInt()) * flPercent);
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
}

float CFFWeaponRailgun::GetRecoilMultiplier()
{
	float flChargeTime = gpGlobals->curtime - m_flStartCharge;

	if (flChargeTime < 1.0f)
		return 1.0f;
	else if (flChargeTime < 2.0f)
		return 2.0f;
	else
		return 3.0f;

	// going to do exect values instead
	// return 1 + ( 2 * ( clamp( gpGlobals->curtime - m_flStartCharge, 0.0f, ffdev_railgun_maxcharge.GetFloat() ) / ffdev_railgun_maxcharge.GetFloat() ) );
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
	// We'll get these done and out of the way
	if (m_iAttachment1 == -1 || m_iAttachment2 == -1)
	{
		m_iAttachment1 = pBaseViewModel->LookupAttachment("railgunFX1");
		m_iAttachment2 = pBaseViewModel->LookupAttachment("railgunFX2");
	}

	// Not charging at all, no need for a glow thing
	if (m_flStartCharge < 0.0f)
	{
		return;
	}

	Vector vecStart, vecEnd, vecMuzzle;
	QAngle tmpAngle;

	pBaseViewModel->GetAttachment(m_iAttachment1, vecStart, tmpAngle);
	pBaseViewModel->GetAttachment(m_iAttachment2, vecEnd, tmpAngle);
	pBaseViewModel->GetAttachment(1, vecMuzzle, tmpAngle);

	::FormatViewModelAttachment(vecStart, true);
	::FormatViewModelAttachment(vecEnd, true);
	::FormatViewModelAttachment(vecMuzzle, true);

	float flChargeAmount = clamp( gpGlobals->curtime - m_flStartCharge, 0.0f, ffdev_railgun_maxcharge.GetFloat() );
	flChargeAmount = clamp( flChargeAmount / ffdev_railgun_maxcharge.GetFloat(), 0.0f, 1.0f);
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
