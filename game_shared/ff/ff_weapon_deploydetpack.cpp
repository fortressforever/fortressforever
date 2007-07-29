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
	~CFFWeaponDeployDetpack( void ) { Cleanup( ); }
#endif

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual void WeaponIdle( void );
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual bool CanBeSelected( void );
	virtual bool CanDeploy( void );
	virtual bool Deploy( void );

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
			m_pBuildable->Remove( );
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
		if (!m_bInSetTimerMenu)
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

		if( !pPlayer->IsBuilding() )
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
		if( m_bInSetTimerMenu /* pPlayer->m_afButtonReleased & IN_ATTACK */ )
		{
			HudContextShow(false);
			m_bInSetTimerMenu = false;
		}


		//if( ( pPlayer->GetDetpack() && !pPlayer->IsBuilding() ) || ( pPlayer->GetAmmoCount( AMMO_DETPACK ) < 1 ) )
		//	pPlayer->SwapToWeapon( FF_WEAPON_GRENADELAUNCHER );

		//// If we haven't built a detpack...
		//if( !pPlayer->GetDetpack() )
		//{
		//	CFFBuildableInfo hBuildInfo(pPlayer, FF_BUILD_DETPACK );

		//	if( m_pBuildable )
		//	{
		//		// Update current fake detpack
		//		m_pBuildable->SetAbsOrigin( hBuildInfo.GetBuildOrigin() );
		//		m_pBuildable->SetAbsAngles( hBuildInfo.GetBuildAngles() );
		//		m_pBuildable->SetBuildError( hBuildInfo.BuildResult() );
		//	}
		//	else
		//	{
		//		// Create fake detpack
		//		m_pBuildable = CFFDetpack::CreateClientSideDetpack( hBuildInfo.GetBuildOrigin(), hBuildInfo.GetBuildAngles() );
		//	}
		//}
		//else
		//	Cleanup();
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
	/*
	else if( pPlayer->IsBuilding() )
	{
#ifdef CLIENT_DLL
		ClientPrintMsg( pPlayer, HUD_PRINTCENTER, "#FF_BUILDERROR_MULTIPLEBUILDS" );
#endif
		return false;
	}
	else if( pPlayer->GetAmmoCount( AMMO_DETPACK ) < 1 )
	{
#ifdef CLIENT_DLL
		ClientPrintMsg( pPlayer, HUD_PRINTCENTER, "#FF_BUILDERROR_DETPACK_NOTENOUGHAMMO" );
#endif
		return false;
	}
	*/

	return BaseClass::CanDeploy();
}

bool CFFWeaponDeployDetpack::CanBeSelected( void )
{
	/*CFFPlayer *pPlayer = GetPlayerOwner();

	if( !pPlayer )
		return false;

	if( pPlayer->GetDetpack() )
		return false;
	else if( pPlayer->IsBuilding() )
		return false;
	else if( pPlayer->GetAmmoCount( AMMO_DETPACK ) < 1 )
		return false;*/

	return BaseClass::CanBeSelected();
}

//----------------------------------------------------------------------------
// Purpose: Send special hint on detpack deploy
//----------------------------------------------------------------------------
bool CFFWeaponDeployDetpack::Deploy() 
{

#ifdef CLIENT_DLL	
	FF_SendHint( DEMOMAN_DETPACK, 5, PRIORITY_LOW, "#FF_HINT_DEMOMAN_DETPACK" );
#endif
	
	return BaseClass::Deploy();
}
