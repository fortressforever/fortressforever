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
	#include "ff_utils.h"
#else
	#include "ff_buildableobjects_shared.h"
	#include "ff_item_backpack.h"
	#include "omnibot_interface.h"
#endif

//=============================================================================
// CFFWeaponSpanner
//=============================================================================


ConVar ffdev_spanner_bagcells("ffdev_spanner_bagcells", "10", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Spanner: How many base cells in created bags (excluding armor steal)");
#define FFDEV_SPANNER_BAGCELLS ffdev_spanner_bagcells.GetFloat()

ConVar ffdev_spanner_bagthrowforceup("ffdev_spanner_bagthrowforceup", "1", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Spanner: vertical throw force on bags ");
#define FFDEV_SPANNER_BAGTHROWFORCEUP ffdev_spanner_bagthrowforceup.GetFloat()

ConVar ffdev_spanner_bagthrowforce("ffdev_spanner_bagthrowforce", "200", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Spanner: horizontal throw force on bags ");
#define FFDEV_SPANNER_BAGTHROWFORCE ffdev_spanner_bagthrowforce.GetFloat()

ConVar ffdev_spanner_bagsonlyonsteal("ffdev_spanner_bagsonlyonsteal", "1", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Spanner: only spawn bags when you steal armor with spanner");
#define FFDEV_SPANNER_BAGSONLYONSTEAL ffdev_spanner_bagsonlyonsteal.GetBool()

class CFFWeaponSpanner : public CFFWeaponMeleeBase
{
public:
	DECLARE_CLASS(CFFWeaponSpanner, CFFWeaponMeleeBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CFFWeaponSpanner();

	virtual bool CanBeSelected();
	virtual void Precache();
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

//----------------------------------------------------------------------------
// Purpose: Precache
//----------------------------------------------------------------------------
void CFFWeaponSpanner::Precache( void )
{
	PrecacheScriptSound( "Spanner.armorbreak" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Allow to be selected even when no ammo
//-----------------------------------------------------------------------------
bool CFFWeaponSpanner::CanBeSelected()
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	if( !pPlayer )
		return false;
	if( pPlayer->IsStaticBuilding() )
		return false;
	if (!VisibleInWeaponSelection())
		return false;

	return true;
}

//----------------------------------------------------------------------------
// Purpose: Implement impact function
//----------------------------------------------------------------------------
void CFFWeaponSpanner::Hit(trace_t &traceHit, Activity nHitActivity) 
{
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
			return;

		// Can the guy we hit take damage from us? If he can't, he's
		// on our team or our ally so give him some armor!
		// Bug #0000521: Engineer's spanner shouldn't inflict damage even with mp_friendlyfire 1
		if( g_pGameRules->PlayerRelationship( pPlayer, pHitPlayer ) == GR_TEAMMATE )		
		{
			// See how much the player needs...
			int iArmorGiven = min( min( pHitPlayer->NeedsArmor(), 50 ), 5 * pPlayer->GetAmmoCount( AMMO_CELLS ) );
			if( iArmorGiven > 0 )
			{
				WeaponSoundLocal( SPECIAL3 );

#ifdef GAME_DLL
				const int iBeforeArmor = pHitPlayer->GetArmor();

				pHitPlayer->AddArmor( iArmorGiven );
				pPlayer->RemoveAmmo( iArmorGiven / 5, AMMO_CELLS );
				// AfterShock - scoring system: Repair x armor +.5*armor_given (only if last damage from enemy) 
				// Leaving the 'last damage from enemy' part out until discussion has finished about it.
				pPlayer->AddFortPoints( ( iArmorGiven*0.5 ), "#FF_FORTPOINTS_GIVEARMOR");

				const int iAfterArmor = pHitPlayer->GetArmor();
				Omnibot::Notify_GotSpannerArmor(pHitPlayer,pPlayer,iBeforeArmor,iAfterArmor);
				Omnibot::Notify_GaveSpannerArmor(pPlayer,pHitPlayer,iBeforeArmor,iAfterArmor);
#endif
			}

			// Don't want to call the baseclass hit func cause
			// we'll apply damage - and we shouldn't apply damage -
			// we're just helping peeps get armor over here
			return;
		}
		else
		{
			// AfterShock: Create bag when hitting enemies with spanner!
#ifdef GAME_DLL

			int iArmorStolen = (pHitPlayer->GetArmor() / 2);
			if ( iArmorStolen > 0)
			{
				pHitPlayer->m_iArmor.GetForModify() = clamp( pHitPlayer->GetArmor() - iArmorStolen, 0, pHitPlayer->GetMaxArmor() );
				EmitSound("Spanner.armorbreak");
			}

			if ( !FFDEV_SPANNER_BAGSONLYONSTEAL || (iArmorStolen > 0))
			{
				Vector vecForward;
				AngleVectors( pPlayer->GetAbsAngles(), &vecForward );

				CFFItemBackpack *pBackpack = (CFFItemBackpack *) CBaseEntity::Create( "ff_item_backpack", ( (pHitPlayer->GetAbsOrigin() + vecForward *0.1) + Vector(0.0f, 0.0f, -1) ), QAngle(0, random->RandomFloat(0.0f, 359.0f), 0) );
				if( pBackpack )
				{
					pBackpack->SetAmmoCount( GetAmmoDef()->Index( AMMO_CELLS ), FFDEV_SPANNER_BAGCELLS + (iArmorStolen/2) );
					//pBackpack->SetAbsVelocity( Vector( random->RandomFloat(-100.0f, 0.0f), random->RandomFloat(-100.0f, 100.0f), random->RandomFloat(20.0f, 100.0f) ) );
					pBackpack->SetAbsVelocity( Vector(vecForward.x, vecForward.y, FFDEV_SPANNER_BAGTHROWFORCEUP) * FFDEV_SPANNER_BAGTHROWFORCE );
					pBackpack->SetOwnerEntity( GetOwner() );
				}
			}
#endif
		}
	}
	else
	{
		// Did not hit a player but we might have hit a buildable object

		// If what we hit was a dispenser
		if (pHitEntity->Classify() == CLASS_DISPENSER) 
		{
			CFFDispenser *pDispenser = (CFFDispenser *) pHitEntity;

			WeaponSound( SPECIAL2 );

			// Is the dispenser mine(is pPlayer the owner?) 
			bool bMine = ( pPlayer == ToFFPlayer( pDispenser->m_hOwner.Get() ) );

			// Is the dispenser a teammates or an allies? (changes depending on friendlyfire value) 
			bool bFriendly = ( g_pGameRules->PlayerRelationship( pPlayer, ToFFPlayer( pDispenser->m_hOwner.Get() ) ) == GR_TEAMMATE );

			// If the dispenser is mine, a team mates, or an allies, don't hurt it, ever
			if( bMine || bFriendly ) 
			{
				// DrEvil: Added IsBuilt check to fix upgrading when build in progress
				if(!pDispenser->IsBuilt())
					return;

				// If it's damaged, restore it's health on the first clang
				if( pDispenser->NeedsHealth() ) 
				{
					// We get 5 health for each cell
					int iHealthGiven = min( pDispenser->NeedsHealth(), 5 * pPlayer->GetAmmoCount( AMMO_CELLS ) );

					// If we give health, play a special sound. Pun intended.
					if( iHealthGiven > 0 )
						WeaponSoundLocal( SPECIAL3 );
					
#ifdef GAME_DLL
					pDispenser->SetHealth( pDispenser->GetHealth() + iHealthGiven );
					// AfterShock - scoring system: Added this for if we later want to give points for repairing friendly dispensers
					if ( bFriendly && !bMine )
						pPlayer->AddFortPoints(iHealthGiven*0.1, "#FF_FORTPOINTS_REPAIRTEAMDISPENSER");
					pPlayer->RemoveAmmo( iHealthGiven / 5, AMMO_CELLS );
#endif
				}
				else
				{
					// On subsequent clangs, we gotta give it ammo and stuff!
					int iCells = min( min( 40, pPlayer->GetAmmoCount( AMMO_CELLS ) ), pDispenser->NeedsCells() );
					int iShells = min( min( 40, pPlayer->GetAmmoCount( AMMO_SHELLS ) ), pDispenser->NeedsShells() );
					int iNails = min( min( 30, pPlayer->GetAmmoCount( AMMO_NAILS ) ), pDispenser->NeedsNails() );
					int iRockets = min( min( 10, pPlayer->GetAmmoCount( AMMO_ROCKETS ) ), pDispenser->NeedsRockets() );

					// If we give it anything, play a special sound. Pun intended.
					if( ( iCells > 0 ) || ( iShells > 0 ) || ( iNails > 0 ) || ( iRockets > 0 ) )
						WeaponSoundLocal( SPECIAL3 );

#ifdef GAME_DLL
					pDispenser->AddAmmo( 0, iCells, iShells, iNails, iRockets );

					pPlayer->RemoveAmmo( iCells, AMMO_CELLS );
					pPlayer->RemoveAmmo( iShells, AMMO_SHELLS );
					pPlayer->RemoveAmmo( iNails, AMMO_NAILS );
					pPlayer->RemoveAmmo( iRockets, AMMO_ROCKETS );
#endif
				}

				// Get out now so we don't call the baseclass and do damage
				return;
			}
		}
		else if (pHitEntity->Classify() == CLASS_SENTRYGUN) 
		{
			CFFSentryGun *pSentryGun = (CFFSentryGun *) pHitEntity;
			WeaponSound( SPECIAL1 );

			// Is the sentrygun mine(is pPlayer the owner?) 
			bool bMine = ( pPlayer == ToFFPlayer( pSentryGun->m_hOwner.Get() ) );

			// Is the sentrygun a teammates or an allies? (changes depending on friendlyfire value) 
			bool bFriendly = ( g_pGameRules->PlayerRelationship( pPlayer, ToFFPlayer( pSentryGun->m_hOwner.Get() ) ) == GR_TEAMMATE );

			// If the sentrygun is mine, a team mates, or an allies, don't hurt it, ever
			if( bMine || bFriendly ) 
			{
				// DrEvil: Added IsBuilt check to fix upgrading when build in progress
				if(!pSentryGun->IsBuilt())
					return;

				// Try to upgrade first
				if ((pSentryGun->GetLevel() < 3) && (pPlayer->GetAmmoCount(AMMO_CELLS) >= FF_BUILDCOST_UPGRADE_SENTRYGUN )) 
				{
					// If we upgrade, play a special sound. Pun intended.
					if( pSentryGun->Upgrade(true) )
					{
						WeaponSoundLocal( SPECIAL3 );
#ifdef GAME_DLL
						// AfterShock - scoring system: If we upgrade teammates SG, +100 points
						// P.S. Make a special upgrade sound 5 lines above this !! :D
						if ( bFriendly && !bMine )
							pPlayer->AddFortPoints(100, "#FF_FORTPOINTS_UPGRADETEAMMATESG");

						IGameEvent *pEvent = gameeventmanager->CreateEvent( "sentrygun_upgraded" );
						if( pEvent )
						{
							//CFFPlayer *pOwner = static_cast<CFFPlayer*>( pSentryGun->m_hOwner.Get() );
							pEvent->SetInt( "userid", pPlayer->GetUserID() );
							pEvent->SetInt( "sgownerid", ((CFFPlayer*)pSentryGun->m_hOwner.Get())->GetUserID() );
							pEvent->SetInt( "level", pSentryGun->GetLevel() );
							gameeventmanager->FireEvent( pEvent, true );
						}
#endif
					}
#ifdef GAME_DLL
					pPlayer->RemoveAmmo(FF_BUILDCOST_UPGRADE_SENTRYGUN, AMMO_CELLS);
#endif
				}
				else
				{

#ifdef CLIENT_DLL
					if ((pSentryGun->GetLevel() < 3) && (pPlayer->GetAmmoCount(AMMO_CELLS) < FF_BUILDCOST_UPGRADE_SENTRYGUN))
						FF_SendHint( ENGY_NOUPGRADE, 1, PRIORITY_NORMAL, "#FF_HINT_ENGY_NOUPGRADE" );
#endif

					// Calculate if it needs anything...
					int cells = min(ceil(pSentryGun->NeedsHealth() / 5.0f), pPlayer->GetAmmoCount(AMMO_CELLS));
/* AfterShock - SGs have infinite ammo now
					int shells = min(pSentryGun->NeedsShells(), pPlayer->GetAmmoCount(AMMO_SHELLS));
					int rockets = 0; 

					if( pSentryGun->GetLevel() > 2 )
						rockets = min(pSentryGun->NeedsRockets(), pPlayer->GetAmmoCount(AMMO_ROCKETS));
*/
					// If it needs anything, play a special sound. Pun intended.
					if( cells > 0 )
						WeaponSoundLocal( SPECIAL3 );

#ifdef GAME_DLL
					// AfterShock - scoring system: Save teammate sg +.5*amount repaired (only if last damage from enemy)
					// last enemy damage bit ignored for now.
					if ( cells > 0 ) 
						pPlayer->AddFortPoints(cells*0.3, "#FF_FORTPOINTS_REPAIRTEAMMATESG");


					pSentryGun->Upgrade(false, cells, 0, 0);
					pPlayer->RemoveAmmo(cells, AMMO_CELLS);
					//pPlayer->RemoveAmmo(shells, AMMO_SHELLS);
					//pPlayer->RemoveAmmo(rockets, AMMO_ROCKETS);
#endif
				}

				// Get out now so we don't call the baseclass and do damage
				return;
			}
		}
	}

	BaseClass::Hit(traceHit, nHitActivity);
}
