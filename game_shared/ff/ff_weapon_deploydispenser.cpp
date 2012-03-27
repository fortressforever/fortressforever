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
#include "in_buttons.h"


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
	virtual void ItemPostFrame( void );

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

//-----------------------------------------------------------------------------
// Purpose: A modified ItemPostFrame to allow for different cycledecrements
//-----------------------------------------------------------------------------
void CFFWeaponDeployDispenser::ItemPostFrame()
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
// Purpose: Handles whatever should be done when they fire(build, aim, etc) 
//----------------------------------------------------------------------------
void CFFWeaponDeployDispenser::PrimaryAttack( void ) 
{
	if (m_flNextPrimaryAttack < gpGlobals->curtime) 
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

		Cleanup();

#ifdef GAME_DLL
		CFFPlayer *pPlayer = GetPlayerOwner();
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

		if( !pPlayer->IsStaticBuilding() )
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

		if( ! pPlayer->IsAlive() )
		{
			ClientPrint( pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEWHENDEAD" );
			return;
		}

		// Bug #0000333: Buildable Behavior (non build slot) while building
		if( pPlayer->IsBuilding() && ( pPlayer->GetCurrentBuild() == FF_BUILD_DISPENSER ) )
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEMIDBUILD");			
			return;
		}

		CFFDispenser *pDispenser = pPlayer->GetDispenser();

		// can't dismantle what doesn't exist
		if (!pDispenser)
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_NODISPENSERTODISMANTLE");	
			return;
		}

		//Bug fix: dismantling a ghost dispenser 
		//if the dispenser is in transparent form, dont dismantle it -GreenMushy
		if( pDispenser->IsTransparent() )
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEMIDBUILD");			
			return;
		}

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
			// Fire an event.
			IGameEvent *pEvent = gameeventmanager->CreateEvent("dispenser_dismantled");		
			if(pEvent)
			{
				pEvent->SetInt("userid", pPlayer->GetUserID());
				gameeventmanager->FireEvent(pEvent, true);
			}
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

		if( ! pPlayer->IsAlive() )
		{
			ClientPrint( pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDETWHENDEAD" );
			return;
		}

		// Bug #0000333: Buildable Behavior (non build slot) while building
		if( pPlayer->IsBuilding() && ( pPlayer->GetCurrentBuild() == FF_BUILD_DISPENSER ) )
		{
			ClientPrint( pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDETMIDBUILD" );
			return;
		}

		CFFDispenser *pDispenser = pPlayer->GetDispenser();

		// can't detonate what we don't have
		if (!pDispenser)
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_NODISPENSERTODET");
			return;
		}			
		
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

		if( ! pPlayer->IsAlive() )
		{
			ClientPrint( pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEORDETWHENDEAD" );
			return;
		}

		// Bug #0000333: Buildable Behavior (non build slot) while building
		if( pPlayer->IsBuilding() && ( pPlayer->GetCurrentBuild() == FF_BUILD_DISPENSER ) )
		{
            ClientPrint( pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEMIDBUILD" );
			return;
		}

		CFFDispenser *pDispenser = pPlayer->GetDispenser();

		// can't do owt to it 'cause it doesn't exist!
		if (!pDispenser)
		{
            ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_NODISPENSER");
			return;
		}

		//Bug fix: dismantling a ghost dispenser 
		//if the dispenser is in transparent form, dont dismantle it -GreenMushy
		if( pDispenser->IsTransparent() )
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEMIDBUILD");			
			return;
		}

		if (pDispenser->IsSabotaged())
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_BUILDINGUNRESPONSIVE");
			return;
		}

		// Close enough to dismantle
		//The previous IsBuilt function didnt seem to work so i removed it -GreenMushy
		if ((pPlayer->GetAbsOrigin() - pDispenser->GetAbsOrigin()).LengthSqr() < 6400.0f )
		{
			// Changed 130 to 65 because:
			// Bug #0000333: Buildable Behavior (non build slot) while building
			pPlayer->GiveAmmo(65.0f, AMMO_CELLS, true);

			// Bug #0000426: Buildables Dismantle Sounds Missing
			CPASAttenuationFilter sndFilter( pDispenser );
			pDispenser->EmitSound( sndFilter, pDispenser->entindex(), "Dispenser.unbuild" );

			
			// Fire an event.
			IGameEvent *pEvent = gameeventmanager->CreateEvent("dispenser_dismantled");		
			if(pEvent)
			{
				pEvent->SetInt("userid", pPlayer->GetUserID());
				gameeventmanager->FireEvent(pEvent, true);
			}
			pDispenser->RemoveQuietly();

		}
		else
			pDispenser->Detonate();
	}
#endif