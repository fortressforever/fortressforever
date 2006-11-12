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

//ConVar ffdev_railgun_push( "ffdev_railgun_pushmod", "30.0", FCVAR_REPLICATED, "Maximum push done by railgun" );
ConVar ffdev_railgun_maxcharge( "ffdev_railgun_maxcharge", "2.0", FCVAR_REPLICATED, "Maximum charge for railgun" );
ConVar ffdev_railgun_speed( "ffdev_railgun_speed", "1100.0", FCVAR_REPLICATED, "Rail speed" );

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

	CSoundPatch *m_pEngine;
#endif	

public:
	virtual FFWeaponID GetWeaponID( void ) const { return FF_WEAPON_RAILGUN; }

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
	m_flStartCharge = -1.0f;

#ifdef CLIENT_DLL
	m_pEngine = NULL;
	m_iAttachment1 = m_iAttachment2 = -1;
#endif
}

//----------------------------------------------------------------------------
// Purpose: Deploy
//----------------------------------------------------------------------------
bool CFFWeaponRailgun::Deploy( void )
{
	m_flStartCharge = -1.0f;

	return BaseClass::Deploy();
}

//----------------------------------------------------------------------------
// Purpose: Holster
//----------------------------------------------------------------------------
bool CFFWeaponRailgun::Holster( CBaseCombatWeapon *pSwitchingTo )
{
#ifdef CLIENT_DLL
	if( m_pEngine && ( GetPlayerOwner() == C_FFPlayer::GetLocalFFPlayer() ) )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pEngine );
		m_pEngine = NULL;
	}
#endif

	m_flStartCharge = -1.0f;

	return BaseClass::Holster( pSwitchingTo );
}

//----------------------------------------------------------------------------
// Purpose: Precache
//----------------------------------------------------------------------------
void CFFWeaponRailgun::Precache( void )
{
	PrecacheScriptSound( "railgun.chargeloop" );
	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Fire a rail
//----------------------------------------------------------------------------
void CFFWeaponRailgun::Fire( void )
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

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

	CFFProjectileRail::CreateRail( this, vecSrc, angAiming, pPlayer, pWeaponInfo.m_iDamage, ffdev_railgun_speed.GetFloat(), flChargeTime );	

	WeaponSound( SINGLE );

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

	if (pPlayer && pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0)
		HandleFireOnEmpty();

	// if we're currently firing, then check to see if we release

	if (pPlayer->m_nButtons & IN_ATTACK)
	{
		CANCEL_IF_BUILDING();

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

#ifdef CLIENT_DLL
			// Bring up the charging looping sound
			if( !m_pEngine )
			{
				// Play charge up sound
				CPASAttenuationFilter filter( this );

				m_pEngine = CSoundEnvelopeController::GetController().SoundCreate( filter, entindex(), "railgun.chargeloop" );
				CSoundEnvelopeController::GetController().Play( m_pEngine, 0.0, 50 );
				CSoundEnvelopeController::GetController().SoundChangeVolume( m_pEngine, 0.7, 2.0 );
			}
#endif
		}
		else
		{
#ifdef CLIENT_DLL
			if( m_pEngine )
			{
				float flPitch = 40 + 10 * min( ffdev_railgun_maxcharge.GetFloat(), clamp( gpGlobals->curtime - m_flStartCharge, 0.0f, ffdev_railgun_maxcharge.GetFloat() ) );
				CSoundEnvelopeController::GetController().SoundChangePitch( m_pEngine, min( 80, flPitch ), 0 );
			}
#endif
		}
	}
	else
	{
		if( m_flStartCharge > 0 )
		{
#ifdef CLIENT_DLL
			if( m_pEngine )
			{
				CSoundEnvelopeController::GetController().SoundDestroy( m_pEngine );
				m_pEngine = NULL;
			}
#endif
			// Fire!!
			Fire();
		}

		m_flStartCharge = -1.0f;
	}
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
// Purpose: This takes palce after the viewmodel is drawn. We use this to
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

	float flChargeAmount = gpGlobals->curtime - m_flStartCharge;
	flChargeAmount /= 5.0f;

	flChargeAmount = clamp(flChargeAmount, 0.0f, 1.0f);
	flChargeAmount = sqrtf(flChargeAmount);

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
