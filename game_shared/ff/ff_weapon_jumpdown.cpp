/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_jumpdown.cpp
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
#include "in_buttons.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponJumpdown C_FFWeaponJumpdown

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

ConVar ffdev_jumpdown_chargeuptime("ffdev_jumpdown_chargeuptime", "6", FCVAR_REPLICATED | FCVAR_CHEAT);
#define JUMPDOWN_CHARGEUPTIME ffdev_jumpdown_chargeuptime.GetFloat()

#ifdef GAME_DLL

ConVar ffdev_jumpdown_allowunchargedshot("ffdev_jumpdown_allowunchargedshot", "0", FCVAR_REPLICATED | FCVAR_CHEAT);
#define JUMPDOWN_ALLOWUNCHARGEDSHOT ffdev_jumpdown_allowunchargedshot.GetBool()

ConVar ffdev_jumpdown_verticalpush("ffdev_jumpdown_verticalpush", "500", FCVAR_REPLICATED | FCVAR_CHEAT);
#define JUMPDOWN_VERTICALPUSH ffdev_jumpdown_verticalpush.GetFloat()

ConVar ffdev_jumpdown_horizontalpush("ffdev_jumpdown_horizontalpush", "450", FCVAR_REPLICATED | FCVAR_CHEAT);
#define JUMPDOWN_HORIZONTALPUSH ffdev_jumpdown_horizontalpush.GetFloat()

ConVar ffdev_jumpdown_horizontalsetvelocity("ffdev_jumpdown_horizontalsetvelocity", "1", FCVAR_REPLICATED | FCVAR_CHEAT);
#define JUMPDOWN_HORIZONTALSETVELOCITY ffdev_jumpdown_horizontalsetvelocity.GetBool()

ConVar ffdev_jumpdown_verticalsetvelocity("ffdev_jumpdown_verticalsetvelocity", "1", FCVAR_REPLICATED | FCVAR_CHEAT);
#define JUMPDOWN_VERTICALSETVELOCITY ffdev_jumpdown_verticalsetvelocity.GetBool()

#else

#define JUMPDOWN_CHARGETIMEBUFFERED_UPDATEINTERVAL 0.02f

#endif

//=============================================================================
// CFFWeaponJumpdown
//=============================================================================

class CFFWeaponJumpdown : public CFFWeaponBase
{
public:
	DECLARE_CLASS( CFFWeaponJumpdown, CFFWeaponBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponJumpdown( void );

	virtual void	PrimaryAttack() {}

	virtual bool	Deploy( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void	Precache( void );
	virtual void	Fire( void );
	virtual void	ItemPostFrame( void );
	virtual void	UpdateOnRemove( void );

	virtual FFWeaponID GetWeaponID( void ) const { return FF_WEAPON_JUMPDOWN; }

	float	GetClampedCharge( void );

	int m_nRevSound;

#ifdef GAME_DLL

	void JumpdownEmitSound( const char* szSoundName );

	bool m_bPlayRevSound;
	float m_flRevSoundNextUpdate;
	EmitSound_t m_paramsRevSound;

	float m_flStartTime;
	float m_flLastUpdate;

#else

private:
	int	m_iAttachment1;
	int m_iAttachment2;
	float m_flTotalChargeTimeBuffered;
	float m_flClampedChargeTimeBuffered;
	float m_flChargeTimeBufferedNextUpdate;

#endif	

private:
	CFFWeaponJumpdown( const CFFWeaponJumpdown & );
	CNetworkVar( float, m_flTotalChargeTime );
	CNetworkVar( float, m_flClampedChargeTime );
};

//=============================================================================
// CFFWeaponJumpdown tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED( FFWeaponJumpdown, DT_FFWeaponJumpdown )

BEGIN_NETWORK_TABLE(CFFWeaponJumpdown, DT_FFWeaponJumpdown )
#ifdef GAME_DLL
	SendPropTime( SENDINFO( m_flTotalChargeTime ) ), 
	SendPropTime( SENDINFO( m_flClampedChargeTime ) ), 
#else
	RecvPropTime( RECVINFO( m_flTotalChargeTime ) ), 
	RecvPropTime( RECVINFO( m_flClampedChargeTime ) ), 
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CFFWeaponJumpdown )
	DEFINE_PRED_FIELD_TOL( m_flTotalChargeTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ), 
	DEFINE_PRED_FIELD_TOL( m_flClampedChargeTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ), 
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( ff_weapon_jumpdown, CFFWeaponJumpdown );
PRECACHE_WEAPON_REGISTER( ff_weapon_jumpdown );

//=============================================================================
// CFFWeaponJumpdown implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponJumpdown::CFFWeaponJumpdown( void )
{
	m_nRevSound = SPECIAL1;

#ifdef GAME_DLL

	// -1 means we are not charging
	m_flStartTime = m_flLastUpdate = -1.0f;
	m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;

	m_flRevSoundNextUpdate = 0.0f;

#else

	m_iAttachment1 = m_iAttachment2 = -1;

	m_flTotalChargeTimeBuffered = m_flClampedChargeTimeBuffered = 0.0f;
	m_flChargeTimeBufferedNextUpdate = 0.0f;

	//m_colorMuzzleDLight.r = g_uchRailColors[0][0];
	//m_colorMuzzleDLight.g = g_uchRailColors[0][1];
	//m_colorMuzzleDLight.b = g_uchRailColors[0][2];

#endif
}

//----------------------------------------------------------------------------
// Purpose: 
//----------------------------------------------------------------------------
void CFFWeaponJumpdown::UpdateOnRemove( void )
{
#ifdef GAME_DLL

	m_flStartTime = m_flLastUpdate = -1.0f;
	m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;

	m_flNextPrimaryAttack = (JUMPDOWN_ALLOWUNCHARGEDSHOT) ? (gpGlobals->curtime + 0.2f) : (gpGlobals->curtime + JUMPDOWN_CHARGEUPTIME);

#else

	m_flTotalChargeTimeBuffered = m_flClampedChargeTimeBuffered = 0.0f;
	m_flChargeTimeBufferedNextUpdate = 0.0f;

#endif

	BaseClass::UpdateOnRemove();
}

//----------------------------------------------------------------------------
// Purpose: Deploy
//----------------------------------------------------------------------------
bool CFFWeaponJumpdown::Deploy( void )
{
#ifdef GAME_DLL

	m_flStartTime = m_flLastUpdate = -1.0f;
	m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;

	m_flNextPrimaryAttack = (JUMPDOWN_ALLOWUNCHARGEDSHOT) ? (gpGlobals->curtime + 0.2f) : (gpGlobals->curtime + JUMPDOWN_CHARGEUPTIME);

#else

	m_flTotalChargeTimeBuffered = m_flClampedChargeTimeBuffered = 0.0f;
	m_flChargeTimeBufferedNextUpdate = 0.0f;

#endif

	return BaseClass::Deploy();
}

//----------------------------------------------------------------------------
// Purpose: Holster
//----------------------------------------------------------------------------
bool CFFWeaponJumpdown::Holster( CBaseCombatWeapon *pSwitchingTo )
{
#ifdef GAME_DLL

	m_flStartTime = m_flLastUpdate = -1.0f;
	m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;

	m_flNextPrimaryAttack = (JUMPDOWN_ALLOWUNCHARGEDSHOT) ? (gpGlobals->curtime + 0.2f) : (gpGlobals->curtime + JUMPDOWN_CHARGEUPTIME);

#else

	m_flTotalChargeTimeBuffered = m_flClampedChargeTimeBuffered = 0.0f;
	m_flChargeTimeBufferedNextUpdate = 0.0f;

#endif

	return BaseClass::Holster( pSwitchingTo );
}

//----------------------------------------------------------------------------
// Purpose: Precache
//----------------------------------------------------------------------------
void CFFWeaponJumpdown::Precache( void )
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
// Purpose: Fire a jumpdown
//----------------------------------------------------------------------------
void CFFWeaponJumpdown::Fire( void )
{
#ifdef GAME_DLL

	CFFPlayer *pPlayer = GetPlayerOwner();
	//const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();  
	// Jiggles: Above line removed until we decide on a good base damage value

	if (!pPlayer)
		return;
	
	// in case a spy gives himself a jumpdown?
	pPlayer->ResetDisguise();

	Vector vecForward, vecRight, vecUp;
	pPlayer->EyeVectors( &vecForward, &vecRight, &vecUp);
	VectorNormalizeFast( vecForward );

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	//Vector vecSrc = pPlayer->GetLegacyAbsOrigin() + vecForward * 8.0f + vecRight * 5.0f + Vector(0, 1, (pPlayer->GetFlags() & FL_DUCKING) ? 5.0f : 22.0f);

	float flPercent = m_flClampedChargeTime / JUMPDOWN_CHARGEUPTIME;

	// Push them
	if (!JUMPDOWN_VERTICALSETVELOCITY && !JUMPDOWN_HORIZONTALSETVELOCITY)
		pPlayer->ApplyAbsVelocityImpulse(Vector(vecForward.x * JUMPDOWN_HORIZONTALPUSH, vecForward.y * JUMPDOWN_HORIZONTALPUSH, JUMPDOWN_VERTICALPUSH) * flPercent);
	else if (JUMPDOWN_VERTICALSETVELOCITY && JUMPDOWN_HORIZONTALSETVELOCITY)
	    pPlayer->SetAbsVelocity(Vector(vecForward.x * JUMPDOWN_HORIZONTALPUSH, vecForward.y * JUMPDOWN_HORIZONTALPUSH, JUMPDOWN_VERTICALPUSH) * flPercent);
	else
	{
		if (JUMPDOWN_VERTICALSETVELOCITY)
		{
			Vector vecVelocity = pPlayer->GetAbsVelocity();
			pPlayer->SetAbsVelocity(Vector(vecVelocity.x, vecVelocity.y, JUMPDOWN_VERTICALPUSH * flPercent));
		}
		else
		{
			pPlayer->ApplyAbsVelocityImpulse(Vector(0, 0, JUMPDOWN_VERTICALPUSH) * flPercent);
		}
		
		if (JUMPDOWN_HORIZONTALSETVELOCITY)
		{
			Vector vecVelocity = pPlayer->GetAbsVelocity();
			pPlayer->SetAbsVelocity(Vector(vecForward.x * JUMPDOWN_HORIZONTALPUSH * flPercent, vecForward.y * JUMPDOWN_HORIZONTALPUSH * flPercent, vecVelocity.z));
		}
		else
		{
			pPlayer->ApplyAbsVelocityImpulse(Vector(vecForward.x * JUMPDOWN_HORIZONTALPUSH, vecForward.y * JUMPDOWN_HORIZONTALPUSH, 0) * flPercent);
		}
	}

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound(SINGLE);

	if (m_bMuzzleFlash)
		pPlayer->DoMuzzleFlash();

	SendWeaponAnim( GetPrimaryAttackActivity() );

	// Player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
	
#ifdef GAME_DLL
	int nShots = min(GetFFWpnData().m_iCycleDecrement, pPlayer->GetAmmoCount(m_iPrimaryAmmoType));
	pPlayer->RemoveAmmo(nShots, m_iPrimaryAmmoType);

#endif

	//pPlayer->ViewPunch( -QAngle( RAILGUN_RECOIL_MIN + ((RAILGUN_RECOIL_MAX - RAILGUN_RECOIL_MIN) * flPercent), 0, 0 ) );

	m_flNextPrimaryAttack = (JUMPDOWN_ALLOWUNCHARGEDSHOT) ? (gpGlobals->curtime + 0.2f) : (gpGlobals->curtime + JUMPDOWN_CHARGEUPTIME);

	// reset these variables
	m_flStartTime = m_flLastUpdate = -1.0f;
	m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;

#endif
}

//----------------------------------------------------------------------------
// Purpose: Handle all the chargeup stuff here
//----------------------------------------------------------------------------
void CFFWeaponJumpdown::ItemPostFrame( void )
{
#ifdef GAME_DLL

	CFFPlayer *pPlayer = GetPlayerOwner();

	if (!pPlayer)
		return;

	// Not currently charging, but wanting to start it up
	if (m_flStartTime == -1.0f && pPlayer->GetAmmoCount(GetPrimaryAmmoType()) > 0)
	{
		m_flStartTime = m_flLastUpdate = gpGlobals->curtime;
		m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;

		// remove ammo immediately
		//pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );
	}

	else
	{
		m_flTotalChargeTime += gpGlobals->curtime - m_flLastUpdate;
		m_flLastUpdate = gpGlobals->curtime;

		// curtime - start time instead of total time, because start time can change if you don't have enough ammo to get to full charge
		m_flClampedChargeTime = clamp(gpGlobals->curtime - m_flStartTime, 0, JUMPDOWN_CHARGEUPTIME);

	}

    if ((pPlayer->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		CANCEL_IF_BUILDING();
		CANCEL_IF_CLOAKED();
		Fire();
	}

	// allow players to continue to charge if they've hit the halfway mark
	// and don't make it immediately switch, causing shot sounds to stop
	if (pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0 && m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		HandleFireOnEmpty();

		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}

#else // ^^ GAME_DLL ^^

	// create a little buffer so some client stuff can be more smooth
	if (m_flChargeTimeBufferedNextUpdate <= gpGlobals->curtime)
	{
		m_flChargeTimeBufferedNextUpdate = gpGlobals->curtime + JUMPDOWN_CHARGETIMEBUFFERED_UPDATEINTERVAL;
		m_flTotalChargeTimeBuffered = m_flTotalChargeTime;
		m_flClampedChargeTimeBuffered = m_flClampedChargeTime;
	}

#endif
}

//----------------------------------------------------------------------------
// Purpose: Get charge
//----------------------------------------------------------------------------
float CFFWeaponJumpdown::GetClampedCharge( void )
{
	return m_flClampedChargeTime;
}

#ifdef CLIENT_DLL

//----------------------------------------------------------------------------
// Purpose: Get charge
//----------------------------------------------------------------------------
float GetJumpdownCharge()
{
	C_FFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

	if (!pPlayer)
		return 0.0f;

	C_FFWeaponBase *pWeapon = pPlayer->GetActiveFFWeapon();

	if (!pWeapon || pWeapon->GetWeaponID() != FF_WEAPON_JUMPDOWN)
		return 0.0f;

	CFFWeaponJumpdown *pJump = (CFFWeaponJumpdown *) pWeapon;

	float fCharge = ( pJump->GetClampedCharge() ) / ( JUMPDOWN_CHARGEUPTIME );
	
	fCharge = clamp( fCharge, 0.01f, 1.0f );

	return fCharge;
}

#endif