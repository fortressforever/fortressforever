#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"
#include "in_buttons.h"
#include "beam_flags.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponJumpgun C_FFWeaponJumpgun

	#include "soundenvelope.h"
	#include "c_ff_player.h"
	#include "beamdraw.h"
	#include "c_te_effect_dispatch.h"

	extern void FormatViewModelAttachment( Vector &vOrigin, bool bInverse );
#else
	#include "omnibot_interface.h"
	#include "ff_player.h"
	#include "ff_entity_system.h"
	#include "te_effect_dispatch.h"
#endif

ConVar ffdev_jumpgun_chargeuptime("ffdev_jumpgun_chargeuptime", "6", FCVAR_REPLICATED | FCVAR_CHEAT);
#define JUMPGUN_CHARGEUPTIME ffdev_jumpgun_chargeuptime.GetFloat()

ConVar ffdev_jumpgun_allowunchargedshot("ffdev_jumpgun_allowunchargedshot", "0", FCVAR_REPLICATED | FCVAR_CHEAT);
#define JUMPGUN_ALLOWUNCHARGEDSHOT ffdev_jumpgun_allowunchargedshot.GetBool()

ConVar ffdev_jumpgun_verticalpush("ffdev_jumpgun_verticalpush", "500", FCVAR_REPLICATED | FCVAR_CHEAT);
#define JUMPGUN_VERTICALPUSH ffdev_jumpgun_verticalpush.GetFloat()

ConVar ffdev_jumpgun_horizontalpush("ffdev_jumpgun_horizontalpush", "450", FCVAR_REPLICATED | FCVAR_CHEAT);
#define JUMPGUN_HORIZONTALPUSH ffdev_jumpgun_horizontalpush.GetFloat()

ConVar ffdev_jumpgun_horizontalsetvelocity("ffdev_jumpgun_horizontalsetvelocity", "1", FCVAR_REPLICATED | FCVAR_CHEAT);
#define JUMPGUN_HORIZONTALSETVELOCITY ffdev_jumpgun_horizontalsetvelocity.GetBool()

ConVar ffdev_jumpgun_verticalsetvelocity("ffdev_jumpgun_verticalsetvelocity", "1", FCVAR_REPLICATED | FCVAR_CHEAT);
#define JUMPGUN_VERTICALSETVELOCITY ffdev_jumpgun_verticalsetvelocity.GetBool()

//effect vars

ConVar ffdev_jumpgun_fx_radius("ffdev_jumpgun_fx_radius", "128", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define JUMPGUN_EFFECT_RADIUS ffdev_jumpgun_fx_radius.GetFloat()

ConVar ffdev_jumpgun_fx_lifetime("ffdev_jumpgun_fx_lifetime", ".5", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define JUMPGUN_EFFECT_LIFETIME ffdev_jumpgun_fx_lifetime.GetFloat()

ConVar ffdev_jumpgun_fx_width("ffdev_jumpgun_fx_width", "16", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define JUMPGUN_EFFECT_WIDTH ffdev_jumpgun_fx_width.GetFloat()

ConVar ffdev_jumpgun_fx_spread("ffdev_jumpgun_fx_spread", "0", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define JUMPGUN_EFFECT_SPREAD ffdev_jumpgun_fx_spread.GetFloat()

ConVar ffdev_jumpgun_fx_amplitude("ffdev_jumpgun_fx_amplitude", "10", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define JUMPGUN_EFFECT_AMPLITUDE ffdev_jumpgun_fx_amplitude.GetFloat()

ConVar ffdev_jumpgun_fx_r("ffdev_jumpgun_fx_r", "255", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define JUMPGUN_EFFECT_R ffdev_jumpgun_fx_r.GetInt()

ConVar ffdev_jumpgun_fx_g("ffdev_jumpgun_fx_g", "255", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define JUMPGUN_EFFECT_G ffdev_jumpgun_fx_g.GetInt()

ConVar ffdev_jumpgun_fx_b("ffdev_jumpgun_fx_b", "255", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define JUMPGUN_EFFECT_B ffdev_jumpgun_fx_b.GetInt()

ConVar ffdev_jumpgun_fx_alpha("ffdev_jumpgun_fx_alpha", "100", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define JUMPGUN_EFFECT_ALPHA ffdev_jumpgun_fx_alpha.GetInt()

ConVar ffdev_jumpgun_fx_speed("ffdev_jumpgun_fx_speed", "0", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define JUMPGUN_EFFECT_SPEED ffdev_jumpgun_fx_speed.GetFloat()

ConVar ffdev_jumpgun_fx_offset_x("ffdev_jumpgun_fx_offset_x", "0", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define JUMPGUN_EFFECT_X_OFFSET ffdev_jumpgun_fx_offset_x.GetFloat()

ConVar ffdev_jumpgun_fx_offset_y("ffdev_jumpgun_fx_offset_y", "0", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define JUMPGUN_EFFECT_Y_OFFSET ffdev_jumpgun_fx_offset_y.GetFloat()

ConVar ffdev_jumpgun_fx_offset_z("ffdev_jumpgun_fx_offset_z", "-32", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define JUMPGUN_EFFECT_Z_OFFSET ffdev_jumpgun_fx_offset_z.GetFloat()

#ifdef CLIENT_DLL

#define JUMPGUN_CHARGETIMEBUFFERED_UPDATEINTERVAL 0.02f

#endif

//=============================================================================
// CFFWeaponJumpgun
//=============================================================================

class CFFWeaponJumpgun : public CFFWeaponBase
{
public:
	DECLARE_CLASS( CFFWeaponJumpgun, CFFWeaponBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponJumpgun( void );

	virtual void	PrimaryAttack() {}

	virtual bool	Deploy( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void	Precache( void );
	virtual void	Fire( void );
	virtual void	ItemPostFrame( void );
	virtual void	UpdateOnRemove( void );

	virtual FFWeaponID GetWeaponID( void ) const { return FF_WEAPON_JUMPGUN; }

	float	GetClampedCharge( void );

	int m_nRevSound;
	int m_iShockwaveTexture;

#ifdef GAME_DLL

	void JumpgunEmitSound( const char* szSoundName );

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
	CFFWeaponJumpgun( const CFFWeaponJumpgun & );
	CNetworkVar( float, m_flTotalChargeTime );
	CNetworkVar( float, m_flClampedChargeTime );
};

//=============================================================================
// CFFWeaponJumpgun tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED( FFWeaponJumpgun, DT_FFWeaponJumpgun )

BEGIN_NETWORK_TABLE(CFFWeaponJumpgun, DT_FFWeaponJumpgun )
#ifdef GAME_DLL
	SendPropTime( SENDINFO( m_flTotalChargeTime ) ), 
	SendPropTime( SENDINFO( m_flClampedChargeTime ) ), 
#else
	RecvPropTime( RECVINFO( m_flTotalChargeTime ) ), 
	RecvPropTime( RECVINFO( m_flClampedChargeTime ) ), 
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CFFWeaponJumpgun )
	DEFINE_PRED_FIELD_TOL( m_flTotalChargeTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ), 
	DEFINE_PRED_FIELD_TOL( m_flClampedChargeTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ), 
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( ff_weapon_jumpgun, CFFWeaponJumpgun );
PRECACHE_WEAPON_REGISTER( ff_weapon_jumpgun );

//=============================================================================
// CFFWeaponJumpgun implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponJumpgun::CFFWeaponJumpgun( void )
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
void CFFWeaponJumpgun::UpdateOnRemove( void )
{
#ifdef GAME_DLL

	m_flStartTime = m_flLastUpdate = -1.0f;
	m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;

	m_flNextPrimaryAttack = (JUMPGUN_ALLOWUNCHARGEDSHOT) ? (gpGlobals->curtime + 0.2f) : (gpGlobals->curtime + JUMPGUN_CHARGEUPTIME);

#else

	m_flTotalChargeTimeBuffered = m_flClampedChargeTimeBuffered = 0.0f;
	m_flChargeTimeBufferedNextUpdate = 0.0f;

#endif

	BaseClass::UpdateOnRemove();
}

//----------------------------------------------------------------------------
// Purpose: Deploy
//----------------------------------------------------------------------------
bool CFFWeaponJumpgun::Deploy( void )
{
#ifdef GAME_DLL

	//Get this player's last weapon
	CFFWeaponBase* pLastWeapon = ToFFPlayer(GetOwnerEntity())->GetLastFFWeapon();

	//If the weapon is valid
	if( pLastWeapon != NULL )
	{
		//If the last weapon was a jumppad, dont reset the jumpgun values
		if( pLastWeapon->GetWeaponID() == FF_WEAPON_DEPLOYMANCANNON )
		{
			//The last weapon was a jumpad
			DevMsg("Last weapon was a jumpad.  Do nothing\n");
			//Dont reset anything?
		}
		//If this was the players last weapon
		else if( pLastWeapon == this )
		{
			//Means this was the weapon deployed when the player built a jumppad with right click
			DevMsg("Jumpgun was the last weapon.  Do nothing\n");
			//Dont reset anything?
		}
		//The last weapon was not a jumppad, so reset normally
		else
		{
			DevMsg("Last weapon was valid but not a jumppad.  Deploy jumpgun normally\n");

			m_flStartTime = -1.0f;
			m_flLastUpdate = -1.0f;
			m_flTotalChargeTime = 0.0f;
			m_flClampedChargeTime = 0.0f;

			m_flNextPrimaryAttack = (JUMPGUN_ALLOWUNCHARGEDSHOT) ? (gpGlobals->curtime + 0.2f) : (gpGlobals->curtime + JUMPGUN_CHARGEUPTIME);
		}
	}
	//This is the first time the player pulls out a weapon
	else
	{
		DevMsg("Last weapon was NULL.  Deploy charged jumpgun\n");

		m_flStartTime = gpGlobals->curtime - JUMPGUN_CHARGEUPTIME;
		m_flLastUpdate = gpGlobals->curtime - JUMPGUN_CHARGEUPTIME;
		m_flTotalChargeTime = JUMPGUN_CHARGEUPTIME;
		m_flClampedChargeTime = JUMPGUN_CHARGEUPTIME;

		m_flNextPrimaryAttack = gpGlobals->curtime;
	}

	//Reset the last weapon to this jumpgun
	ToFFPlayer(GetOwnerEntity())->SetLastFFWeapon(this);

#endif

	return BaseClass::Deploy();
}

//----------------------------------------------------------------------------
// Purpose: Holster
//----------------------------------------------------------------------------
bool CFFWeaponJumpgun::Holster( CBaseCombatWeapon *pSwitchingTo )
{
#ifdef GAME_DLL
	//Only set this weapon to the last if something is calling Holster(NULL) on the jumpgun
	if( pSwitchingTo == NULL )
	{
		DevMsg("Holstering jumpgun - Setting last weapon to this jumpgun\n");

		//Reset the last weapon to this jumpgun
		ToFFPlayer(GetOwnerEntity())->SetLastFFWeapon(this);
	}
	//this means the player is switching to a valid weapon other then this
	else
	{
		DevMsg("Holstering jumpgun - Setting new last weapon\n");

		//Reset the last weapon to this jumpgun
		ToFFPlayer(GetOwnerEntity())->SetLastFFWeapon((CFFWeaponBase*)pSwitchingTo);

		//Only reset the values if the weapon being switched to is NOT a mancannon
		if( ((CFFWeaponBase*)pSwitchingTo)->GetWeaponID() != FF_WEAPON_DEPLOYMANCANNON )
		{
			m_flStartTime = -1.0f;
			m_flLastUpdate = -1.0f;
			m_flTotalChargeTime = 0.0f;
			m_flClampedChargeTime = 0.0f;

			m_flNextPrimaryAttack = (JUMPGUN_ALLOWUNCHARGEDSHOT) ? (gpGlobals->curtime + 0.2f) : (gpGlobals->curtime + JUMPGUN_CHARGEUPTIME);
		}
	}
#endif

	return BaseClass::Holster( pSwitchingTo );
}

//----------------------------------------------------------------------------
// Purpose: Precache
//----------------------------------------------------------------------------
void CFFWeaponJumpgun::Precache( void )
{
	PrecacheScriptSound( "railgun.single_shot" );		// SINGLE
	m_iShockwaveTexture = PrecacheModel( "sprites/lgtning.vmt" );	

	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Fire a jumpgun
//----------------------------------------------------------------------------
void CFFWeaponJumpgun::Fire( void )
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	if (!pPlayer)
		return;

	pPlayer->m_flTrueAimTime = gpGlobals->curtime;

	Vector vecForward, vecRight, vecUp;
	pPlayer->EyeVectors( &vecForward, &vecRight, &vecUp);
	VectorNormalizeFast( vecForward );
	VectorNormalizeFast( vecRight );
	
	// get only the direction the player is looking (ignore any z)
	Vector horizPush = CrossProduct(Vector( 0.0f, 0.0f, 1.0f ), vecRight);
	horizPush *= JUMPGUN_HORIZONTALPUSH;

	Vector vecSrc = pPlayer->Weapon_ShootPosition();

	float flPercent = m_flClampedChargeTime / JUMPGUN_CHARGEUPTIME;

	// Push them
	if (!JUMPGUN_VERTICALSETVELOCITY && !JUMPGUN_HORIZONTALSETVELOCITY)
		pPlayer->ApplyAbsVelocityImpulse(Vector(horizPush.x, horizPush.y, JUMPGUN_VERTICALPUSH) * flPercent);
	else if (JUMPGUN_VERTICALSETVELOCITY && JUMPGUN_HORIZONTALSETVELOCITY)
	    pPlayer->SetAbsVelocity(Vector(horizPush.x, horizPush.y, JUMPGUN_VERTICALPUSH) * flPercent);
	else
	{
		if (JUMPGUN_VERTICALSETVELOCITY)
		{
			Vector vecVelocity = pPlayer->GetAbsVelocity();
			pPlayer->SetAbsVelocity(Vector(vecVelocity.x, vecVelocity.y, JUMPGUN_VERTICALPUSH * flPercent));
		}
		else
		{
			pPlayer->ApplyAbsVelocityImpulse(Vector(0, 0, JUMPGUN_VERTICALPUSH) * flPercent);
		}
		
		if (JUMPGUN_HORIZONTALSETVELOCITY)
		{
			Vector vecVelocity = pPlayer->GetAbsVelocity();
			pPlayer->SetAbsVelocity(Vector(horizPush.x * flPercent, horizPush.y * flPercent, vecVelocity.z));
		}
		else
		{
			pPlayer->ApplyAbsVelocityImpulse(Vector(horizPush.x, horizPush.y, 0) * flPercent);
		}
	}

	if (m_bMuzzleFlash)
		pPlayer->DoMuzzleFlash();

	SendWeaponAnim( GetPrimaryAttackActivity() );

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound(SINGLE);

	// Player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
	
	int nShots = min(GetFFWpnData().m_iCycleDecrement, pPlayer->GetAmmoCount(m_iPrimaryAmmoType));
	pPlayer->RemoveAmmo(nShots, m_iPrimaryAmmoType);

	m_flNextPrimaryAttack = (JUMPGUN_ALLOWUNCHARGEDSHOT) ? (gpGlobals->curtime + 0.2f) : (gpGlobals->curtime + JUMPGUN_CHARGEUPTIME);

	// reset these variables
#ifdef GAME_DLL
	m_flStartTime = m_flLastUpdate = -1.0f;
#endif
	m_flTotalChargeTime = m_flClampedChargeTime = 0.0f;

	// effect
	CBroadcastRecipientFilter filter;
	te->BeamRingPoint( 
		filter, 
		0,										//delay
		pPlayer->GetAbsOrigin() + Vector(JUMPGUN_EFFECT_X_OFFSET, JUMPGUN_EFFECT_Y_OFFSET, JUMPGUN_EFFECT_Z_OFFSET + ((pPlayer->GetFlags() & FL_DUCKING) ? 16.0f : 0.0f)),					//origin
		1.0f,									//start radius
		flPercent * JUMPGUN_EFFECT_RADIUS,		//end radius
		m_iShockwaveTexture,					//texture
		0,										//halo index
		0,										//start frame
		2,										//framerate
		JUMPGUN_EFFECT_LIFETIME,				//life
		JUMPGUN_EFFECT_WIDTH,					//width
		JUMPGUN_EFFECT_SPREAD,					//spread
		JUMPGUN_EFFECT_AMPLITUDE,				//amplitude
		JUMPGUN_EFFECT_R,						//r
		JUMPGUN_EFFECT_G,						//g
		JUMPGUN_EFFECT_B,						//b
		JUMPGUN_EFFECT_ALPHA,					//a
		JUMPGUN_EFFECT_SPEED,					//speed
		FBEAM_FADEOUT
	);
}

//----------------------------------------------------------------------------
// Purpose: Handle all the chargeup stuff here
//----------------------------------------------------------------------------
void CFFWeaponJumpgun::ItemPostFrame( void )
{

	CFFPlayer *pPlayer = GetPlayerOwner();

	if (!pPlayer)
		return;

#ifdef GAME_DLL
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
		m_flClampedChargeTime = clamp(gpGlobals->curtime - m_flStartTime, 0, JUMPGUN_CHARGEUPTIME);

	}
#endif

    if ((pPlayer->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime) && (pPlayer->GetAmmoCount(GetPrimaryAmmoType()) > 0))
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

#ifdef CLIENT_DLL
	// create a little buffer so some client stuff can be more smooth
	if (m_flChargeTimeBufferedNextUpdate <= gpGlobals->curtime)
	{
		m_flChargeTimeBufferedNextUpdate = gpGlobals->curtime + JUMPGUN_CHARGETIMEBUFFERED_UPDATEINTERVAL;
		m_flTotalChargeTimeBuffered = m_flTotalChargeTime;
		m_flClampedChargeTimeBuffered = m_flClampedChargeTime;
	}
#endif
}

//----------------------------------------------------------------------------
// Purpose: Get charge
//----------------------------------------------------------------------------
float CFFWeaponJumpgun::GetClampedCharge( void )
{
	return m_flClampedChargeTime;
}

#ifdef CLIENT_DLL

//----------------------------------------------------------------------------
// Purpose: Get charge
//----------------------------------------------------------------------------
float GetJumpgunCharge()
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayerOrObserverTarget();

	if (!pPlayer)
		return 0.0f;

	C_FFWeaponBase *pWeapon = pPlayer->GetActiveFFWeapon();

	if (!pWeapon || pWeapon->GetWeaponID() != FF_WEAPON_JUMPGUN)
		return 0.0f;

	CFFWeaponJumpgun *pJump = (CFFWeaponJumpgun *) pWeapon;

	float fCharge = ( pJump->GetClampedCharge() ) / ( JUMPGUN_CHARGEUPTIME );
	
	fCharge = clamp( fCharge, 0.01f, 1.0f );

	return fCharge;
}

#endif