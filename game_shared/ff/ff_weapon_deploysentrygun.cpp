//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_weapon_deploysentrygun.cpp
//	@author Patrick O'Leary(Mulchman) 
//	@date 05/12/05
//	@brief A sentrygun construction slot
//
//	REVISIONS
//	---------
//	05/12/05, Mulchman: First created - basically a copy/paste of the
//		DeployDispenser set up
//
//	05/14/05, Mulchman:
//		Optimized as per Mirv's forum suggestion
//
//	09/20/2005, 	Mulchman:
//		Added aiming ring.
//
//	09/24/2005, 	Mulchman:
//		Added a model for the aiming ring.
//
//	03/17/06, Mulchman:
//		Bug #0000333: Buildable Behavior (non build slot) while building fixes
//		Removing Aim Sphere

#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponDeploySentryGun C_FFWeaponDeploySentryGun
	#define CFFSentryGun C_FFSentryGun

	#include "c_ff_player.h"
	#include "ff_utils.h"
	#include "ff_hud_chat.h"
#else
	#include "ff_player.h"
#endif

#include "ff_buildableobjects_shared.h"

//=============================================================================
// CFFWeaponDeploySentryGun
//=============================================================================

class CFFWeaponDeploySentryGun : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponDeploySentryGun, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponDeploySentryGun( void );
#ifdef CLIENT_DLL 
	~CFFWeaponDeploySentryGun( void ) 
	{ 
		Cleanup(); 
	}
#endif

	virtual void PrimaryAttack( void );
	virtual void WeaponIdle( void );
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual bool CanBeSelected( void );
	virtual bool CanDeploy( void );
	virtual void ItemPostFrame( void );

	virtual FFWeaponID GetWeaponID( void ) const		{ return FF_WEAPON_DEPLOYSENTRYGUN; }

private:

	CFFWeaponDeploySentryGun(const CFFWeaponDeploySentryGun &);

#ifdef CLIENT_DLL
protected:
	CFFSentryGun *m_pBuildable;
#endif

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
// CFFWeaponDeploySentryGun tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponDeploySentryGun, DT_FFWeaponDeploySentryGun) 

BEGIN_NETWORK_TABLE(CFFWeaponDeploySentryGun, DT_FFWeaponDeploySentryGun) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponDeploySentryGun) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_deploysentrygun, CFFWeaponDeploySentryGun);
PRECACHE_WEAPON_REGISTER(ff_weapon_deploysentrygun);

//=============================================================================
// CFFWeaponDeploySentryGun implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponDeploySentryGun::CFFWeaponDeploySentryGun( void ) 
{
#ifdef CLIENT_DLL
	m_pBuildable = NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: A modified ItemPostFrame to allow for different cycledecrements
//-----------------------------------------------------------------------------
void CFFWeaponDeploySentryGun::ItemPostFrame()
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
void CFFWeaponDeploySentryGun::PrimaryAttack( void ) 
{
	if (m_flNextPrimaryAttack < gpGlobals->curtime) 
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

		Cleanup();
		
#ifdef GAME_DLL
		CFFPlayer *pPlayer = GetPlayerOwner();
		pPlayer->Command_BuildSentryGun();
#endif
	}
}

//----------------------------------------------------------------------------
// Purpose: Checks validity of ground at this point or whatever
//----------------------------------------------------------------------------
void CFFWeaponDeploySentryGun::WeaponIdle( void ) 
{
	if (m_flTimeWeaponIdle < gpGlobals->curtime) 
	{
#ifdef CLIENT_DLL 
		C_FFPlayer *pPlayer = GetPlayerOwner();

		// If we've built and we're not building pop out wrench
		/*if( ( pPlayer->GetSentryGun() && !pPlayer->IsBuilding() ) || ( pPlayer->GetAmmoCount( AMMO_CELLS ) < 130 ) )
			pPlayer->SwapToWeapon( FF_WEAPON_SPANNER );*/

		if( !pPlayer->IsStaticBuilding() )
		{
			CFFBuildableInfo hBuildInfo( pPlayer, FF_BUILD_SENTRYGUN );
			if( !m_pBuildable )
			{
				m_pBuildable = CFFSentryGun::CreateClientSideSentryGun( hBuildInfo.GetBuildOrigin(), hBuildInfo.GetBuildAngles() );
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

bool CFFWeaponDeploySentryGun::Holster(CBaseCombatWeapon *pSwitchingTo) 
{
	Cleanup();

	return BaseClass::Holster( pSwitchingTo );
}

bool CFFWeaponDeploySentryGun::CanDeploy( void )
{
	return BaseClass::CanDeploy();
}

bool CFFWeaponDeploySentryGun::CanBeSelected( void )
{
	return BaseClass::CanBeSelected();
}

#ifdef GAME_DLL
	//=============================================================================
	// Commands
	//=============================================================================
	CON_COMMAND(aimsentry, "Aim sentrygun")
	{
		CFFPlayer *pPlayer = ToFFPlayer(UTIL_GetCommandClient());

		if (!pPlayer) 
			return;

		if( ! pPlayer->IsAlive() )
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTAIMSENTRYWHENDEAD" );
			return;
		}
		// Bug #0000333: Buildable Behavior (non build slot) while building
		if( pPlayer->IsBuilding() && ( pPlayer->GetCurrentBuild() == FF_BUILD_SENTRYGUN ) )
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTAIMSENTRYWHENBUILDINGIT");
			return;
		}

		CFFSentryGun *pSentry = pPlayer->GetSentryGun();

		if (!pSentry) 
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_NOSENTRY");
			return;
		}

		if (pSentry->IsSabotaged())
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_BUILDINGUNRESPONSIVE");
			return;
		}

		Vector vecForward;
		AngleVectors(pPlayer->EyeAngles(), &vecForward);

		trace_t tr;
		CTraceFilterWorldOnly traceFilter;
		UTIL_TraceLine(pPlayer->Weapon_ShootPosition(), pPlayer->Weapon_ShootPosition() + vecForward * MAX_TRACE_LENGTH, MASK_SHOT, &traceFilter, &tr);

		pSentry->SetFocusPoint(tr.endpos);
	}

	CON_COMMAND(dismantlesentry, "Dismantle sentrygun") 
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
		if( pPlayer->IsBuilding() && ( pPlayer->GetCurrentBuild() == FF_BUILD_SENTRYGUN ) )
		{
			ClientPrint( pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEMIDBUILD" );
			return;
		}

		CFFSentryGun *pSentry = pPlayer->GetSentryGun();

		if (!pSentry) 
		{
			ClientPrint( pPlayer, HUD_PRINTCENTER, "#FF_ENGY_NOSENTRYTODISMANTLE" );
			return;
		}

		if (pSentry->IsSabotaged())
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_BUILDINGUNRESPONSIVE");
			return;
		}

		// Close enough to dismantle
		if ((pPlayer->GetAbsOrigin() - pSentry->GetAbsOrigin()).LengthSqr() < 6400.0f) 
		{
			pPlayer->GiveAmmo(pSentry->GetLevel() * 65, AMMO_CELLS, true);

			// Bug #0000426: Buildables Dismantle Sounds Missing
			CPASAttenuationFilter sndFilter( pSentry );
			pSentry->EmitSound( sndFilter, pSentry->entindex(), FF_SENTRYGUN_UNBUILD_SOUND );

			// Fire an event.
			IGameEvent *pEvent = gameeventmanager->CreateEvent("sentry_dismantled");						
			if(pEvent)
			{
				pEvent->SetInt("userid", pPlayer->GetUserID());
				pEvent->SetInt("level", pSentry->GetLevel());
				gameeventmanager->FireEvent(pEvent, true);
			}
			pSentry->RemoveQuietly();
		}
		else
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_TOOFARAWAY");
	}

	CON_COMMAND(detsentry, "Detonates sentrygun") 
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
		if( pPlayer->IsBuilding() && ( pPlayer->GetCurrentBuild() == FF_BUILD_SENTRYGUN ) )
		{
			ClientPrint( pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDETMIDBUILD" );
			return;
		}

		CFFSentryGun *pSentry = pPlayer->GetSentryGun();

		if (!pSentry) 
		{
            ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_NOSENTRYTODET");
			return;
		}

		if (pSentry->IsSabotaged())
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_BUILDINGUNRESPONSIVE");
			return;
		}

		pSentry->Detonate();
	}

	CON_COMMAND(detdismantlesentry, "Dismantles or detonate sentrygun depending on distance") 
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
		if( pPlayer->IsBuilding() && ( pPlayer->GetCurrentBuild() == FF_BUILD_SENTRYGUN ) )
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDETMIDBUILD" );
			return;
		}

		CFFSentryGun *pSentry = pPlayer->GetSentryGun();

		if (!pSentry) 
		{
            ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_NOSENTRY" );
			return;
		}

		if (pSentry->IsSabotaged())
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_BUILDINGUNRESPONSIVE");
			return;
		}

		// Close enough to dismantle
		if ((pPlayer->GetAbsOrigin() - pSentry->GetAbsOrigin()).LengthSqr() < 6400.0f) 
		{
			pPlayer->GiveAmmo(pSentry->GetLevel() * 65, AMMO_CELLS, true);

			// Bug #0000426: Buildables Dismantle Sounds Missing
			CPASAttenuationFilter sndFilter( pSentry );
			pSentry->EmitSound( sndFilter, pSentry->entindex(), FF_SENTRYGUN_UNBUILD_SOUND );

			pSentry->RemoveQuietly();
		}
		else
			pSentry->Detonate();
	}
#endif