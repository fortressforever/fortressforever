/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_spanner.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF Spanner code & class declaration.
///
/// REVISIONS
/// ---------
/// Jan 19, 2005 Mirv: Initial implementation
//
//	05/10/2006,	Mulchman:
//		Matched values from TFC (thanks Dospac)


#include "cbase.h"
#include "ff_weapon_basemelee.h"

#ifdef CLIENT_DLL
	#define CFFWeaponSpanner C_FFWeaponSpanner
#else
	#include "ff_buildableobjects_shared.h"
#endif

//=============================================================================
// CFFWeaponSpanner
//=============================================================================

class CFFWeaponSpanner : public CFFWeaponMeleeBase
{
public:
	DECLARE_CLASS(CFFWeaponSpanner, CFFWeaponMeleeBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CFFWeaponSpanner();

	virtual void PrimaryAttack( void );
	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_SPANNER; }

private:
	CFFWeaponSpanner(const CFFWeaponSpanner &);

	// BEG: Added by Mulchman
	virtual void	Hit(trace_t &traceHit, Activity nHitActivity);
	// END: Added by Mulchman

};

//=============================================================================
// CFFWeaponSpanner tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponSpanner, DT_FFWeaponSpanner) 

BEGIN_NETWORK_TABLE(CFFWeaponSpanner, DT_FFWeaponSpanner) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponSpanner) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_spanner, CFFWeaponSpanner);
PRECACHE_WEAPON_REGISTER(ff_weapon_spanner);

//=============================================================================
// CFFWeaponSpanner implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponSpanner::CFFWeaponSpanner() 
{
}

void CFFWeaponSpanner::PrimaryAttack( void )
{
	// Bug #0000333: Buildable Behavior (non build slot) while building
	CFFPlayer *pPlayer = GetPlayerOwner();
	if( pPlayer->m_bBuilding )
	{
		DevMsg( "[Spanner] Player is building - need to cancel build since they are hitting +attack\n" );

		switch( pPlayer->m_iCurBuild )
		{
#ifdef GAME_DLL
			case FF_BUILD_DISPENSER: pPlayer->Command_BuildDispenser(); break;
			case FF_BUILD_SENTRYGUN: pPlayer->Command_BuildSentryGun(); break;
#else
			case FF_BUILD_DISPENSER: pPlayer->SwapToWeapon( FF_WEAPON_DEPLOYDISPENSER ); break;
			case FF_BUILD_SENTRYGUN: pPlayer->SwapToWeapon( FF_WEAPON_DEPLOYSENTRYGUN ); break;
#endif
		}

		return;
	}

	BaseClass::PrimaryAttack();
}

//----------------------------------------------------------------------------
// Purpose: Implement impact function
//----------------------------------------------------------------------------
void CFFWeaponSpanner::Hit(trace_t &traceHit, Activity nHitActivity) 
{
#ifdef GAME_DLL
	// Get the player who is swinging us...
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());
	if (!pPlayer) 
	{
		Warning("[CFFWeaponSpanner] [serverside] Failed to get owner!\n");
		return;
	}

	// Get what we hit - if anything
	CBaseEntity *pHitEntity = traceHit.m_pEnt;
	if (!pHitEntity) 
		return;

	// Hit a player
	if (pHitEntity->IsPlayer()) 
	{
		// If it's dead, who cares
		if (!pHitEntity->IsAlive()) 
			return;

		CFFPlayer *pHitPlayer = ToFFPlayer(pHitEntity);
		if (!pHitPlayer) 
		{
			Warning("[CFFWeaponSpanner] [serverside] Failed to get the hit player\n");
			return;
		}

		DevMsg("[CFFWeaponSpanner] Hit a player\n");

		// Can the guy we hit take damage from us? If he can't, he's
		// on our team or our ally so give him some armor!
		// Bug #0000521: Engineer's spanner shouldn't inflict damage even with mp_friendlyfire 1
		if( g_pGameRules->PlayerRelationship( pPlayer, pHitPlayer ) == GR_TEAMMATE )		
		{
			DevMsg("[CFFWeaponSpanner] Player is on my team or an ally - so giving armor!\n");

			// See how much the player needs...
			int iNeedsArmor = pHitPlayer->NeedsArmor();

			if (iNeedsArmor > 0) 
			{
				DevMsg("[CFFWeaponSpanner] Player NEEDS armor!\n");

				// If we've got 10 cells...
				if (pPlayer->GetAmmoCount(AMMO_CELLS) >= 10) 
				{
					DevMsg("[CFFWeaponSpanner] Giving armor and reducing my cells\n");

					int armour_needed = max(pHitPlayer->NeedsArmor(), 50);
					int armour_given = min(armour_needed, 5 * pPlayer->GetAmmoCount(AMMO_CELLS));

					pHitPlayer->AddArmor(armour_given);
					pPlayer->RemoveAmmo(armour_given / 5, AMMO_CELLS);
				}				
			}
			else
			{
				DevMsg("[CFFWeaponSpanner] Player DOES NOT NEED armor!\n");
			}

			// Don't want to call the baseclass hit func cause
			// we'll apply damage - and we shouldn't apply damage -
			// we're just helping peeps get armor over here
			return;
		}
		else
		{
			DevMsg("[CFFWeaponSpanner] Player is not on my team and not an ally - so sending to baseclass to hurt!\n");
		}		
	}
	else
	{
		// Did not hit a player but we might have hit a buildable object

		// If what we hit was a dispenser
		if (pHitEntity->Classify() == CLASS_DISPENSER) 
		{
			CFFDispenser *pDispenser = (CFFDispenser *) pHitEntity;

			WeaponSound(SPECIAL2);

			// Is the dispenser mine(is pPlayer the owner?) 
			bool bMine = ( pPlayer == ToFFPlayer( pDispenser->m_hOwner.Get() ) );

			// Is the dispenser a teammates or an allies? (changes depending on friendlyfire value) 
			bool bFriendly = ( g_pGameRules->PlayerRelationship( pPlayer, ToFFPlayer( pDispenser->m_hOwner.Get() ) ) == GR_TEAMMATE );

			// If the dispenser is mine, a team mates, or an allies, don't hurt it, ever
			if( bMine || bFriendly ) 
			{
				DevMsg("[CFFWeaponSpanner] [serverside] Dispenser is mine, a team mates, or an allies, don't hurt it!\n");

				// If it's damaged, restore it's health on the first clang
				if( pDispenser->NeedsHealth() ) 
				{
					// We get 5 health for each cell
					int iHealthGiven = min( pDispenser->NeedsHealth(), 5 * pPlayer->GetAmmoCount( AMMO_CELLS ) );
					
					pDispenser->SetHealth( pDispenser->GetHealth() + iHealthGiven );

					pPlayer->RemoveAmmo( iHealthGiven / 5, AMMO_CELLS );
				}
				else
				{
					// On subsequent clangs, we gotta give it ammo and stuff!
					int iCells = min( min( 40, pPlayer->GetAmmoCount( AMMO_CELLS ) ), pDispenser->NeedsCells() );
					int iShells = min( min( 40, pPlayer->GetAmmoCount( AMMO_SHELLS ) ), pDispenser->NeedsShells() );
					int iNails = min( min( 30, pPlayer->GetAmmoCount( AMMO_NAILS ) ), pDispenser->NeedsNails() );
					int iRockets = min( min( 10, pPlayer->GetAmmoCount( AMMO_ROCKETS ) ), pDispenser->NeedsRockets() );
					int iRadioTags = min( min( 10, pPlayer->GetAmmoCount( AMMO_RADIOTAG ) ), pDispenser->NeedsRadioTags() );
					// Only add armor when building
					//int iArmor = min( min( 40, pPlayer->GetArmor() ), pDispenser->NeedsArmor() );

					pDispenser->AddAmmo( 0, iCells, iShells, iNails, iRockets, iRadioTags );

					//pPlayer->RemoveArmor( iArmor );
					pPlayer->RemoveAmmo( iCells, AMMO_CELLS );
					pPlayer->RemoveAmmo( iShells, AMMO_SHELLS );
					pPlayer->RemoveAmmo( iNails, AMMO_NAILS );
					pPlayer->RemoveAmmo( iRockets, AMMO_ROCKETS );
					pPlayer->RemoveAmmo( iRadioTags, AMMO_RADIOTAG );
				}

				// Get out now so we don't call the baseclass and do damage
				return;
			}

			DevMsg("[CFFWeaponSpanner] [serverside] Dispenser is not mine, a team mates, or an allies. HURT THE THING!\n");
		}
		else if (pHitEntity->Classify() == CLASS_SENTRYGUN) 
		{
			CFFSentryGun *pSentryGun = (CFFSentryGun *) pHitEntity;

			WeaponSound(SPECIAL1);

			// Is the sentrygun mine(is pPlayer the owner?) 
			bool bMine = ( pPlayer == ToFFPlayer( pSentryGun->m_hOwner.Get() ) );

			// Is the sentrygun a teammates or an allies? (changes depending on friendlyfire value) 
			bool bFriendly = ( g_pGameRules->PlayerRelationship( pPlayer, ToFFPlayer( pSentryGun->m_hOwner.Get() ) ) == GR_TEAMMATE );

			// If the sentrygun is mine, a team mates, or an allies, don't hurt it, ever
			if( bMine || bFriendly ) 
			{
				// Try to upgrade first
				if ((pSentryGun->GetLevel() < 3) && (pPlayer->GetAmmoCount(AMMO_CELLS) >= 130)) 
				{
					pSentryGun->Upgrade(true);
					pPlayer->RemoveAmmo(130, AMMO_CELLS);
				}
				else
				{
					int cells = min(ceil(pSentryGun->NeedsHealth() / 3.5f), pPlayer->GetAmmoCount(AMMO_CELLS));
					int shells = min(pSentryGun->NeedsShells(), pPlayer->GetAmmoCount(AMMO_SHELLS));
					int rockets = 0; 
					
					if( pSentryGun->GetLevel() > 2 )
						rockets = min(pSentryGun->NeedsRockets(), pPlayer->GetAmmoCount(AMMO_ROCKETS));

					pSentryGun->Upgrade(false, cells, shells, rockets);
					pPlayer->RemoveAmmo(cells, AMMO_CELLS);
					pPlayer->RemoveAmmo(shells, AMMO_SHELLS);
					pPlayer->RemoveAmmo(rockets, AMMO_ROCKETS);					
				}

				// Get out now so we don't call the baseclass and do damage
				return;
			}
		}
		// See if we hit some type of entity that needs to do something
		else if (0) 
		{
			// TODO: Add else if for hitting an entity
			// that is supposed to fire some game event -
			// like the generator in oppose or something...

			// enysys.RunPredicates( NULL, this, "engy hit the shit w/ spanner, bitch" );
		}
	}
#else

	CBaseEntity *pHitEntity = traceHit.m_pEnt;

	// If we're a client we don't want to draw decals and stuff
	// Also since weapon sounds are predicted, play on client too
	if (pHitEntity->Classify() == CLASS_SENTRYGUN)
	{
		WeaponSound(SPECIAL1);
		return;
	}
	else if (pHitEntity->Classify() ==CLASS_DISPENSER)
	{
		WeaponSound(SPECIAL2);
		return;
	}
	else if (pHitEntity->IsPlayer() && pHitEntity->IsAlive())
	{
		CFFPlayer *pPlayer = ToFFPlayer(GetOwner());
		CFFPlayer *pHitPlayer = ToFFPlayer(pHitEntity);

		// Don't do rest of swing if teammate so that blood won't be shown
		if (g_pGameRules->PlayerRelationship( pPlayer, pHitPlayer ) == GR_TEAMMATE)
			return;
	}

#endif

	BaseClass::Hit(traceHit, nHitActivity);
}
