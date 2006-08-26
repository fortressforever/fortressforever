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
	
	CFFWeaponDeployDetpack();
#ifdef CLIENT_DLL 
	~CFFWeaponDeployDetpack( void ) { Cleanup( ); }
#endif

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void WeaponIdle();
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual bool CanBeSelected();

	virtual FFWeaponID GetWeaponID( void ) const		{ return FF_WEAPON_DEPLOYDETPACK; }

private:

	CFFWeaponDeployDetpack( const CFFWeaponDeployDetpack & );

protected:
#ifdef CLIENT_DLL
	C_FFDetpack *pDetpack;
#endif

	void Cleanup( void )
	{
#ifdef CLIENT_DLL
		if( pDetpack )
		{
			pDetpack->Remove( );
			pDetpack = NULL;
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
CFFWeaponDeployDetpack::CFFWeaponDeployDetpack()
{
#ifdef CLIENT_DLL
	pDetpack = NULL;
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

#ifdef GAME_DLL
		// Bug #0000378: Detpack slot sometimes cancels the deploy phase almost immediately
		engine->ClientCommand( GetPlayerOwner()->edict(), "detpack 5" );
#endif
	}
}

//----------------------------------------------------------------------------
// Purpose: Handles whatever should be done when they scondary fire
//----------------------------------------------------------------------------
void CFFWeaponDeployDetpack::SecondaryAttack( void )
{
	if( m_flNextSecondaryAttack < gpGlobals->curtime )
	{
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
	}
}

//----------------------------------------------------------------------------
// Purpose: Checks validity of ground at this point or whatever
//----------------------------------------------------------------------------
void CFFWeaponDeployDetpack::WeaponIdle()
{
	if( m_flTimeWeaponIdle < gpGlobals->curtime )
	{
#ifdef CLIENT_DLL 
		C_FFPlayer *pPlayer = GetPlayerOwner();

		// If we haven't built a detpack...
		if( !pPlayer->m_hDetpack.Get() )
		{
			float flRaiseVal = FF_BUILD_DET_RAISE_VAL;
			if( pPlayer->GetFlags() & FL_DUCKING )
				flRaiseVal /= 2;

			CFFBuildableInfo hBuildInfo(pPlayer, FF_BUILD_DETPACK, FF_BUILD_DET_BUILD_DIST, flRaiseVal);

			/*
			if( pDetpack )
			{
				// Update current fake Detpack
				pDetpack->SetAbsOrigin( hBuildInfo.GetBuildGroundOrigin() );
				pDetpack->SetAbsAngles( hBuildInfo.GetBuildGroundAngles() );

				pDetpack->SetBuildError( hBuildInfo.BuildResult() );
			}
			else
			{
				// Create fake Detpack
				pDetpack = CFFDetpack::CreateClientSideDetpack( hBuildInfo.GetBuildGroundOrigin(), hBuildInfo.GetBuildGroundAngles() );
			}

			if( hBuildInfo.BuildResult() != 0 )
				pDetpack->SetRenderColor( ( byte )255, ( byte )0, ( byte )0 );
			else
				pDetpack->SetRenderColor( ( byte )255, ( byte )255, ( byte )255 );
				*/

			//*
			if (hBuildInfo.BuildResult() == BUILD_ALLOWED)
			{
				if (pDetpack)
				{
					// Detpack is currently hidden
					if (pDetpack->GetEffects() & EF_NODRAW)
						pDetpack->RemoveEffects(EF_NODRAW);

					// These were calculated in TryBuild()
					pDetpack->SetAbsOrigin(hBuildInfo.GetBuildAirOrigin());
					pDetpack->SetAbsAngles(hBuildInfo.GetBuildAirAngles());
				}
				else
					pDetpack = C_FFDetpack::CreateClientSideDetpack(hBuildInfo.GetBuildAirOrigin(), hBuildInfo.GetBuildAirAngles());
			}
			// If we have built a detpack...
			// Unable to build, so hide buildable
			else if(pDetpack && !(pDetpack->GetEffects() & EF_NODRAW))
				pDetpack->SetEffects(EF_NODRAW);
				//*/
		}
		// Destroy if we already have one
		else
		{
			Cleanup();

			pPlayer->SwapToWeapon( FF_WEAPON_CROWBAR );
		}
#endif
	}
}

bool CFFWeaponDeployDetpack::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	Cleanup();

	return BaseClass::Holster( pSwitchingTo );
}

bool CFFWeaponDeployDetpack::CanBeSelected()
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	if( pPlayer && ( ( CFFDetpack * )pPlayer->m_hDetpack.Get() ) )
		return false;
	// Bug #0000333: Buildable Behavior (non build slot) while building
	else if( pPlayer->m_bBuilding )
		return false;
	// Bug #0000333: Buildable Behavior (non build slot) while building
	else if( pPlayer->GetAmmoCount( AMMO_DETPACK ) < 1 )
		return false;
	else
		return BaseClass::CanBeSelected();
}
