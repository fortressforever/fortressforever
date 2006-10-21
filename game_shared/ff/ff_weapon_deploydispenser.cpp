//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_weapon_deploydispenser.cpp
//	@author Gavin "Mirvin_Monkey" Bramhill
//	@date May 10, 2005
//	@brief A test dispenser construction slot
//
//	REVISIONS
//	---------
//	May 10, 2005, Mirv: First created
//
//	05/10/05, Mulchman:
//		Took over this class, thanks Mirv. Dispenser now sees if it can be built
//		during weapon idle times. If it can it draws a box. Pressing the attack
//		button will cause the dispenser to be built(if it can be).
//
//	05/11/05, Mulchman:
//		Fixed a bug where the faux dispenser would still be valid even though
//		we had built the thing already
//
//	05/14/05, Mulchman:
//		Optimized as per Mirv's forum suggestion
//
//	03/14/06, Mulchman:
//		Add some stuff for bug fixing

#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"


#ifdef CLIENT_DLL 
	#define CFFWeaponDeployDispenser C_FFWeaponDeployDispenser
	#define CFFDispenser C_FFDispenser

	#include "c_ff_player.h"
	#include "ff_hud_chat.h"
#else
	#include "ff_player.h"
#endif

#include "ff_buildableobjects_shared.h"

//=============================================================================
// CFFWeaponDeployDispenser
//=============================================================================

class CFFWeaponDeployDispenser : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponDeployDispenser, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponDeployDispenser( void );
#ifdef CLIENT_DLL 
	~CFFWeaponDeployDispenser( void ) { Cleanup(); }
#endif

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );
	virtual void WeaponIdle( void );
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual bool CanBeSelected( void );
	virtual bool CanDeploy( void );

	virtual FFWeaponID GetWeaponID( void ) const		{ return FF_WEAPON_DEPLOYDISPENSER; }

private:

	CFFWeaponDeployDispenser(const CFFWeaponDeployDispenser &);

protected:
#ifdef CLIENT_DLL
	CFFDispenser *m_pBuildable;
#endif

	// I've stuck the client dll check in here so it doesnt have to be everywhere else
	void Cleanup() 
	{
#ifdef CLIENT_DLL
		if (m_pBuildable) 
		{
			m_pBuildable->Remove();
			m_pBuildable = NULL;
		}
#endif
	}
};

//=============================================================================
// CFFWeaponDeployDispenser tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponDeployDispenser, DT_FFWeaponDeployDispenser) 

BEGIN_NETWORK_TABLE(CFFWeaponDeployDispenser, DT_FFWeaponDeployDispenser) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponDeployDispenser) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_deploydispenser, CFFWeaponDeployDispenser);
PRECACHE_WEAPON_REGISTER(ff_weapon_deploydispenser);

//=============================================================================
// CFFWeaponDeployDispenser implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponDeployDispenser::CFFWeaponDeployDispenser( void ) 
{
#ifdef CLIENT_DLL
	m_pBuildable = NULL;
#endif
}

//----------------------------------------------------------------------------
// Purpose: Handles whatever should be done when they fire(build, aim, etc) 
//----------------------------------------------------------------------------
void CFFWeaponDeployDispenser::PrimaryAttack( void ) 
{
	if (m_flNextPrimaryAttack < gpGlobals->curtime) 
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

		Cleanup();

		ENGY_SPECIAL_AIMSENTRYGUN();

#ifdef GAME_DLL
		CFFPlayer *pPlayer = GetPlayerOwner();		
		if( pPlayer->IsBuilding() )
		{
			switch( pPlayer->GetCurBuild() )
			{
				case FF_BUILD_DISPENSER: pPlayer->Command_BuildDispenser(); break;
				case FF_BUILD_SENTRYGUN: pPlayer->Command_BuildSentryGun(); break;
			}
		}
		else
			pPlayer->Command_BuildDispenser();
#endif
	}
}

//----------------------------------------------------------------------------
// Purpose: Handles whatever should be done when they scondary fire
//----------------------------------------------------------------------------
void CFFWeaponDeployDispenser::SecondaryAttack( void ) 
{
	m_flNextSecondaryAttack = gpGlobals->curtime;
}

//----------------------------------------------------------------------------
// Purpose: Checks validity of ground at this point or whatever
//----------------------------------------------------------------------------
void CFFWeaponDeployDispenser::WeaponIdle( void ) 
{
	if (m_flTimeWeaponIdle < gpGlobals->curtime) 
	{
		//SetWeaponIdleTime(gpGlobals->curtime + 0.1f);

#ifdef CLIENT_DLL 
		C_FFPlayer *pPlayer = GetPlayerOwner();

		// If we've built and we're not building pop out wrench
		/*if( ( pPlayer->GetDispenser() && !pPlayer->IsBuilding() ) || ( pPlayer->GetAmmoCount( AMMO_CELLS ) < 100 ) )
			pPlayer->SwapToWeapon( FF_WEAPON_SPANNER );*/

		if( !pPlayer->IsBuilding() )
		{
			CFFBuildableInfo hBuildInfo( pPlayer, FF_BUILD_DISPENSER );
			if( !m_pBuildable )
			{
				m_pBuildable = CFFDispenser::CreateClientSideDispenser( hBuildInfo.GetBuildOrigin(), hBuildInfo.GetBuildAngles() );
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

		// If we haven't built a dispenser...
		//if( !pPlayer->GetDispenser() ) 
		//{
		//	CFFBuildableInfo hBuildInfo( pPlayer, FF_BUILD_DISPENSER );

		//	if( m_pBuildable )
		//	{
		//		// Update current fake dispenser
		//		m_pBuildable->SetAbsOrigin( hBuildInfo.GetBuildOrigin() );
		//		m_pBuildable->SetAbsAngles( hBuildInfo.GetBuildAngles() );
		//		m_pBuildable->SetBuildError( hBuildInfo.BuildResult() );
		//	}
		//	else
		//	{
		//		// Create fake dispenser
		//		m_pBuildable = CFFDispenser::CreateClientSideDispenser( hBuildInfo.GetBuildOrigin(), hBuildInfo.GetBuildAngles() );
		//	}
		//}
		//else
		//	Cleanup();

		//// If we're building something else, make sure to clean up
		//// this thing
		//if( pPlayer->IsBuilding() )
		//	Cleanup();
#endif
	}
}

bool CFFWeaponDeployDispenser::Holster(CBaseCombatWeapon *pSwitchingTo) 
{
	Cleanup();

	return BaseClass::Holster( pSwitchingTo );
}

bool CFFWeaponDeployDispenser::CanDeploy( void )
{
	/*
	CFFPlayer *pPlayer = GetPlayerOwner();

	if( !pPlayer )
		return false;

	if( pPlayer->GetDispenser() )
	{
#ifdef CLIENT_DLL
		ClientPrintMsg( pPlayer, HUD_PRINTCENTER, "#FF_BUILDERROR_DISPENSER_ALREADYBUILT" );
#endif
		return false;
	}
	else if( pPlayer->IsBuilding() )
	{
#ifdef CLIENT_DLL
		ClientPrintMsg( pPlayer, HUD_PRINTCENTER, "#FF_BUILDERROR_MULTIPLEBUILDS" );
#endif
		return false;
	}
	else if( pPlayer->GetAmmoCount( AMMO_CELLS ) < 100 )
	{
#ifdef CLIENT_DLL
		ClientPrintMsg( pPlayer, HUD_PRINTCENTER, "#FF_BUILDERROR_DISPENSER_NOTENOUGHAMMO" );
#endif
		return false;
	}
	*/

	return BaseClass::CanDeploy();
}

bool CFFWeaponDeployDispenser::CanBeSelected( void )
{
	/*CFFPlayer *pPlayer = GetPlayerOwner();

	if( !pPlayer )
		return false;

	if( pPlayer->GetDispenser() )
		return false;
	else if( pPlayer->IsBuilding() )
		return false;
	else if( pPlayer->GetAmmoCount( AMMO_CELLS ) < 100 )
		return false;*/

	return BaseClass::CanBeSelected();
}

#ifdef GAME_DLL
	//=============================================================================
	// Commands
	//=============================================================================
	CON_COMMAND(dismantledispenser, "Dismantle dispenser")
	{
		CFFPlayer *pPlayer = ToFFPlayer(UTIL_GetCommandClient());

		if (!pPlayer)
			return;

		// Bug #0000333: Buildable Behavior (non build slot) while building
		if( pPlayer->IsBuilding() && ( pPlayer->GetCurBuild() == FF_BUILD_DISPENSER ) )
			return;

		CFFDispenser *pDispenser = pPlayer->GetDispenser();

		if (!pDispenser)
			return;

		if (pDispenser->IsSabotaged())
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_BUILDINGUNRESPONSIVE");
			return;
		}

		// Close enough to dismantle
		if ((pPlayer->GetAbsOrigin() - pDispenser->GetAbsOrigin()).LengthSqr() < 6400.0f)
		{
			// Changed 130 to 65 because:
			// Bug #0000333: Buildable Behavior (non build slot) while building
			pPlayer->GiveAmmo(65.0f, AMMO_CELLS, true);

			// Bug #0000426: Buildables Dismantle Sounds Missing
			CPASAttenuationFilter sndFilter( pDispenser );
			pDispenser->EmitSound( sndFilter, pDispenser->entindex(), "Dispenser.unbuild" );

			pDispenser->RemoveQuietly();
		}
		else
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_TOOFARAWAY");
	}

	CON_COMMAND(detdispenser, "Detonates dispenser")
	{
		CFFPlayer *pPlayer = ToFFPlayer(UTIL_GetCommandClient());

		if (!pPlayer)
			return;

		// Bug #0000333: Buildable Behavior (non build slot) while building
		if( pPlayer->IsBuilding() && ( pPlayer->GetCurBuild() == FF_BUILD_DISPENSER ) )
			return;

		CFFDispenser *pDispenser = pPlayer->GetDispenser();

		if (!pDispenser)
			return;
		
		if (pDispenser->IsSabotaged())
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_BUILDINGUNRESPONSIVE");
			return;
		}

		pDispenser->Detonate();
	}

	CON_COMMAND(detdismantledispenser, "Dismantles or detonate dispenser depending on distance")
	{
		CFFPlayer *pPlayer = ToFFPlayer(UTIL_GetCommandClient());

		if (!pPlayer)
			return;

		// Bug #0000333: Buildable Behavior (non build slot) while building
		if( pPlayer->IsBuilding() && ( pPlayer->GetCurBuild() == FF_BUILD_DISPENSER ) )
			return;

		CFFDispenser *pDispenser = pPlayer->GetDispenser();

		if (!pDispenser)
			return;

		if (pDispenser->IsSabotaged())
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_BUILDINGUNRESPONSIVE");
			return;
		}

		// Close enough to dismantle
		if ((pPlayer->GetAbsOrigin() - pDispenser->GetAbsOrigin()).LengthSqr() < 6400.0f)
		{
			// Changed 130 to 65 because:
			// Bug #0000333: Buildable Behavior (non build slot) while building
			pPlayer->GiveAmmo(65.0f, AMMO_CELLS, true);

			// Bug #0000426: Buildables Dismantle Sounds Missing
			CPASAttenuationFilter sndFilter( pDispenser );
			pDispenser->EmitSound( sndFilter, pDispenser->entindex(), "Dispenser.unbuild" );

			pDispenser->RemoveQuietly();

			// Fire an event.
			IGameEvent *pEvent = gameeventmanager->CreateEvent("dispenser_dismantled");						
			if(pEvent)
			{
				pEvent->SetInt("userid", pPlayer->GetUserID());
				gameeventmanager->FireEvent(pEvent, true);
			}
		}
		else
			pDispenser->Detonate();
	}
#endif