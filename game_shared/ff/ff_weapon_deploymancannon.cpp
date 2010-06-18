//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_weapon_deploymancannon.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 12/7/2007
//	@brief A man cannon construction slot
//
//	REVISIONS
//	---------
//	12/7/2007, Mulchman: 
//		First created

#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"
#include "in_buttons.h"

#if defined( CLIENT_DLL )
	#define CFFWeaponDeployManCannon C_FFWeaponDeployManCannon
	#define CFFManCannon C_FFManCannon
	#include "c_ff_player.h"	
	#include "ff_hud_chat.h"
	#include "ff_utils.h"
	extern void HudContextShow(bool visible);
	#include "in_buttons.h"
#else
	#include "ff_player.h"
#endif

#include "ff_buildableobjects_shared.h"

//=============================================================================
// CFFWeaponDeployManCannon
//=============================================================================

class CFFWeaponDeployManCannon : public CFFWeaponBase
{
public:
	DECLARE_CLASS( CFFWeaponDeployManCannon, CFFWeaponBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CFFWeaponDeployManCannon( void );
#ifdef CLIENT_DLL 
	~CFFWeaponDeployManCannon( void ) { Cleanup(); }
#endif

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual void WeaponIdle( void );
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual bool CanBeSelected( void );
	virtual bool CanDeploy( void );
	virtual bool Deploy( void );
	virtual void ItemPostFrame( void );

	virtual FFWeaponID GetWeaponID( void ) const		{ return FF_WEAPON_DEPLOYMANCANNON; }

private:

	CFFWeaponDeployManCannon( const CFFWeaponDeployManCannon & );

protected:
#ifdef CLIENT_DLL
	C_FFManCannon *m_pBuildable;
	bool m_bInSetTimerMenu;
#endif

	void Cleanup( void )
	{
#ifdef CLIENT_DLL
		if( m_pBuildable )
		{
			m_pBuildable->Remove();
			m_pBuildable = NULL;
		}
#endif
	}
};

//=============================================================================
// CFFWeaponDeployManCannon tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED( FFWeaponDeployManCannon, DT_FFWeaponDeployManCannon )

BEGIN_NETWORK_TABLE( CFFWeaponDeployManCannon, DT_FFWeaponDeployManCannon )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CFFWeaponDeployManCannon )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( ff_weapon_deploymancannon, CFFWeaponDeployManCannon );
PRECACHE_WEAPON_REGISTER( ff_weapon_deploymancannon );

//=============================================================================
// CFFWeaponDeployManCannon implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponDeployManCannon::CFFWeaponDeployManCannon( void )
{
#ifdef CLIENT_DLL
	m_pBuildable = NULL;
	m_bInSetTimerMenu = false;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: A modified ItemPostFrame to allow for different cycledecrements
//-----------------------------------------------------------------------------
void CFFWeaponDeployManCannon::ItemPostFrame()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	//Track the duration of the fire
	//FIXME: Check for IN_ATTACK2 as well?
	//FIXME: What if we're calling ItemBusyFrame?
	m_fFireDuration = (pOwner->m_nButtons & IN_ATTACK) ? (m_fFireDuration + gpGlobals->frametime) : 0.0f;

	// if just released the attack, then reset nextfiretime
	if (pOwner->m_afButtonReleased & IN_ATTACK)
		m_flNextPrimaryAttack = gpGlobals->curtime;

	if ((pOwner->m_nButtons & IN_ATTACK || pOwner->m_afButtonPressed & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
			PrimaryAttack();
	}

	// -----------------------
	//  Reload pressed / Clip Empty
	// -----------------------
	if (pOwner->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload)
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
		m_fFireDuration = 0.0f;
	}

	// -----------------------
	//  No buttons down
	// -----------------------
	if (! ((pOwner->m_nButtons & IN_ATTACK) || /* (pOwner->m_nButtons & IN_ATTACK2) ||*/ (pOwner->m_nButtons & IN_RELOAD))) // |-- Mirv: Removed attack2 so things can continue while in menu
	{
		// no fire buttons down or reloading
		if (!ReloadOrSwitchWeapons() && (m_bInReload == false))
		{
			WeaponIdle();
		}
	}
}

//----------------------------------------------------------------------------
// Purpose: Handles whatever should be done when they fire (build, aim, etc)
//----------------------------------------------------------------------------
void CFFWeaponDeployManCannon::PrimaryAttack( void )
{
	if( m_flNextPrimaryAttack < gpGlobals->curtime )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

		Cleanup();

#ifdef GAME_DLL
		CFFPlayer *pPlayer = GetPlayerOwner();
		pPlayer->Command_BuildManCannon();
#endif
	}
}

//----------------------------------------------------------------------------
// Purpose: Handles whatever should be done when they scondary fire
//----------------------------------------------------------------------------
void CFFWeaponDeployManCannon::SecondaryAttack( void )
{
	if( m_flNextSecondaryAttack < gpGlobals->curtime )
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
}


//----------------------------------------------------------------------------
// Purpose: Checks validity of ground at this point or whatever
//----------------------------------------------------------------------------
void CFFWeaponDeployManCannon::WeaponIdle( void )
{
	if( m_flTimeWeaponIdle < gpGlobals->curtime )
	{
#ifdef CLIENT_DLL 
		C_FFPlayer *pPlayer = GetPlayerOwner();

		if( !pPlayer->IsStaticBuilding() )
		{
			CFFBuildableInfo hBuildInfo( pPlayer, FF_BUILD_MANCANNON );
			if( !m_pBuildable )
			{
				m_pBuildable = CFFManCannon::CreateClientSideManCannon( hBuildInfo.GetBuildOrigin(), hBuildInfo.GetBuildAngles() );
			}
			else
			{
				m_pBuildable->SetAbsOrigin( hBuildInfo.GetBuildOrigin() );
				m_pBuildable->SetAbsAngles( hBuildInfo.GetBuildAngles() );
			}
			m_pBuildable->SetBuildError( hBuildInfo.BuildResult() );
		}
		else
		{
			Cleanup();
		}

		// The player just released the attack button
		if( m_bInSetTimerMenu /* pPlayer->m_afButtonReleased & IN_ATTACK */ )
		{
			HudContextShow(false);
			m_bInSetTimerMenu = false;
		}
#endif
	}
}

//----------------------------------------------------------------------------
// Purpose: 
//----------------------------------------------------------------------------
bool CFFWeaponDeployManCannon::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	Cleanup();

#ifdef CLIENT_DLL
	HudContextShow(false);
#endif

	return BaseClass::Holster( pSwitchingTo );
}

//----------------------------------------------------------------------------
// Purpose: 
//----------------------------------------------------------------------------
bool CFFWeaponDeployManCannon::CanDeploy( void )
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	if( !pPlayer )
		return false;

	if( pPlayer->GetManCannon() )
	{
#ifdef CLIENT_DLL
		ClientPrintMsg( pPlayer, HUD_PRINTCENTER, "#FF_BUILDERROR_MANCANNON_ALREADYBUILT" );
#endif

		return false;
	}

	return BaseClass::CanDeploy();
}

//----------------------------------------------------------------------------
// Purpose: 
//----------------------------------------------------------------------------
bool CFFWeaponDeployManCannon::CanBeSelected( void )
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	if( !pPlayer )
		return false;

	if( pPlayer->GetManCannon() )
	{
		return false;
	}

	return BaseClass::CanBeSelected();
}

//----------------------------------------------------------------------------
// Purpose: Send special hint on man cannon deploy
//----------------------------------------------------------------------------
bool CFFWeaponDeployManCannon::Deploy() 
{
#ifdef CLIENT_DLL	
	FF_SendHint( DEMOMAN_DETPACK, 1, PRIORITY_LOW, "#FF_HINT_SCOUT_MANCANNON" );
#endif

	return BaseClass::Deploy();
}
