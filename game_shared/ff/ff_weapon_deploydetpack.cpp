//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_weapon_deploydetpack.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date May 12, 2005
//	@brief A detpack construction slot
//
//	REVISIONS
//	---------
//	05/12/05, Mulchman: 
//		First created - basically a copy/paste of the dispenser one!
//
//	05/14/05, Mulchman:
//		Optimized as per Mirv's forum suggestion

#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"
#include "in_buttons.h"

#if defined( CLIENT_DLL )
	#define CFFWeaponDeployDetpack C_FFWeaponDeployDetpack
	#define CFFDetpack C_FFDetpack
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
// CFFWeaponDeployDetpack
//=============================================================================

class CFFWeaponDeployDetpack : public CFFWeaponBase
{
public:
	DECLARE_CLASS( CFFWeaponDeployDetpack, CFFWeaponBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponDeployDetpack( void );
#ifdef CLIENT_DLL 
	~CFFWeaponDeployDetpack( void ) { Cleanup(); }
#endif

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual void WeaponIdle( void );
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual bool CanBeSelected( void );
	virtual bool CanDeploy( void );
	virtual bool Deploy( void );
	virtual void ItemPostFrame( void );

	virtual FFWeaponID GetWeaponID( void ) const		{ return FF_WEAPON_DEPLOYDETPACK; }

private:

	CFFWeaponDeployDetpack( const CFFWeaponDeployDetpack & );

protected:
#ifdef CLIENT_DLL
	C_FFDetpack *m_pBuildable;
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
// CFFWeaponDeployDetpack tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED( FFWeaponDeployDetpack, DT_FFWeaponDeployDetpack )

BEGIN_NETWORK_TABLE( CFFWeaponDeployDetpack, DT_FFWeaponDeployDetpack )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CFFWeaponDeployDetpack )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( ff_weapon_deploydetpack, CFFWeaponDeployDetpack );
PRECACHE_WEAPON_REGISTER( ff_weapon_deploydetpack );

//=============================================================================
// CFFWeaponDeployDetpack implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponDeployDetpack::CFFWeaponDeployDetpack( void )
{
#ifdef CLIENT_DLL
	m_pBuildable = NULL;
	m_bInSetTimerMenu = false;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: A modified ItemPostFrame to allow for different cycledecrements
//-----------------------------------------------------------------------------
void CFFWeaponDeployDetpack::ItemPostFrame()
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
void CFFWeaponDeployDetpack::PrimaryAttack( void )
{
	if( m_flNextPrimaryAttack < gpGlobals->curtime )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

		Cleanup();

//#ifdef GAME_DLL
//		// Bug #0000378: Detpack slot sometimes cancels the deploy phase almost immediately
//		engine->ClientCommand( GetPlayerOwner()->edict(), "detpack 5" );
//
//#endif

#ifdef CLIENT_DLL
		// By holding down the attack button, the player brings up a radial menu
		// to choose the timer length
		C_FFPlayer *pPlayer = GetPlayerOwner();
		if (!m_bInSetTimerMenu && pPlayer && pPlayer->IsLocalPlayer() )
		{
			m_bInSetTimerMenu = true;
			HudContextShow(true);
			
			FF_SendHint( DEMOMAN_SETDET, 2, PRIORITY_NORMAL, "#FF_HINT_DEMOMAN_SETDET" );
		}
#endif
	}
}

//----------------------------------------------------------------------------
// Purpose: Handles whatever should be done when they scondary fire
//----------------------------------------------------------------------------
void CFFWeaponDeployDetpack::SecondaryAttack( void )
{
	if( m_flNextSecondaryAttack < gpGlobals->curtime )
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
}

//----------------------------------------------------------------------------
// Purpose: Checks validity of ground at this point or whatever
//----------------------------------------------------------------------------
void CFFWeaponDeployDetpack::WeaponIdle( void )
{
	if( m_flTimeWeaponIdle < gpGlobals->curtime )
	{
#ifdef CLIENT_DLL 
		C_FFPlayer *pPlayer = GetPlayerOwner();

		if( !pPlayer->IsStaticBuilding() )
		{
			CFFBuildableInfo hBuildInfo( pPlayer, FF_BUILD_DETPACK );
			if( !m_pBuildable )
			{
				m_pBuildable = CFFDetpack::CreateClientSideDetpack( hBuildInfo.GetBuildOrigin(), hBuildInfo.GetBuildAngles() );
			}
			else
			{
				m_pBuildable->SetAbsOrigin( hBuildInfo.GetBuildOrigin() );
				m_pBuildable->SetAbsAngles( hBuildInfo.GetBuildAngles() );
			}
			m_pBuildable->SetBuildError( hBuildInfo.BuildResult() );
		}
		else
			Cleanup();

		// The player just released the attack button
		if( m_bInSetTimerMenu && pPlayer && pPlayer->IsLocalPlayer()/* pPlayer->m_afButtonReleased & IN_ATTACK */ )
		{
			HudContextShow(false);
			m_bInSetTimerMenu = false;
		}
#endif
	}
}

bool CFFWeaponDeployDetpack::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	Cleanup();
#ifdef CLIENT_DLL
	HudContextShow(false);
#endif
	return BaseClass::Holster( pSwitchingTo );
}

bool CFFWeaponDeployDetpack::CanDeploy( void )
{
	
	CFFPlayer *pPlayer = GetPlayerOwner();

	if( !pPlayer )
		return false;
	
	if( pPlayer->GetDetpack() )
	{
#ifdef CLIENT_DLL
		ClientPrintMsg( pPlayer, HUD_PRINTCENTER, "#FF_BUILDERROR_DETPACK_ALREADYSET" );
#endif
		return false;
	}

	return BaseClass::CanDeploy();
}

bool CFFWeaponDeployDetpack::CanBeSelected( void )
{
	return BaseClass::CanBeSelected();
}

//----------------------------------------------------------------------------
// Purpose: Send special hint on detpack deploy
//----------------------------------------------------------------------------
bool CFFWeaponDeployDetpack::Deploy() 
{

#ifdef CLIENT_DLL	
	FF_SendHint( DEMOMAN_DETPACK, 1, PRIORITY_LOW, "#FF_HINT_DEMOMAN_DETPACK" );
#endif
	
	return BaseClass::Deploy();
}
