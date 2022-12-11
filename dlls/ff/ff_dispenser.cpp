// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_dispenser.cpp
// @author Patrick O'Leary (Mulchman)
// @date 12/28/2005
// @brief Dispenser class
//
// REVISIONS
// ---------
//	12/28/2005, Mulchman: 
//		First created
//
//	05/09/2005, Mulchman:
//		Tons of little updates here and there
//
//	05/10/2005, Mulchman:
//		Added some stuff for regeneration of items inside
//
//	05/11/2005, Mulchman
//		Added some OnTouch stuff to give the player touching
//		us some stuff
//
//	05/13/2005, Mulchman
//		Fixed the OnTouch stuff to work correctly now, heh
//
//	05/17/2005, Mulchman
//		Added some code so the disp is ready
//		to calculate a new explosion magnitude
//		based on how full it is
//
//	08/25/2005, Mulchman
//		Removed physics orienting the weapon and am now doing
//		it manually
//
// 22/01/2006, Mirv:
//		Dispenser now has ground pos & angles pre-stored for when it goes live
//
//	05/04/2006,	Mulchman
//		AddAmmo function. Minor tweaks here and there.
//
//	05/10/2006,	Mulchman:
//		Matched values from TFC (thanks Dospac)

#include "cbase.h"
#include "ff_buildableobjects_shared.h"
#include "const.h"
#include "ff_weapon_base.h"
#include "ff_gamerules.h"
#include "ff_utils.h"
#include "ff_item_backpack.h"
#include "te_effect_dispatch.h"

#include "omnibot_interface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
//
//	class CFFDispenser
//
//=============================================================================
LINK_ENTITY_TO_CLASS( FF_Dispenser, CFFDispenser );
PRECACHE_REGISTER( FF_Dispenser );

IMPLEMENT_SERVERCLASS_ST( CFFDispenser, DT_FFDispenser )
	SendPropInt( SENDINFO( m_iAmmoPercent ), 8, SPROP_UNSIGNED ), 
	SendPropInt( SENDINFO( m_iCells ), 9, SPROP_UNSIGNED ), // max is 400 cells
	SendPropInt( SENDINFO( m_iShells ), 9, SPROP_UNSIGNED ), // max is 400 shells
	SendPropInt( SENDINFO( m_iNails ), 9, SPROP_UNSIGNED ), // max is 500 nails
	SendPropInt( SENDINFO( m_iRockets ), 8, SPROP_UNSIGNED ), // max is 250 rockets
	SendPropInt( SENDINFO( m_iArmor ), 9, SPROP_UNSIGNED ), // max is 500 armor
END_SEND_TABLE()

// Start of our data description for the class
BEGIN_DATADESC( CFFDispenser )
	DEFINE_ENTITYFUNC( OnObjectTouch ),
	DEFINE_THINKFUNC( OnObjectThink ),
END_DATADESC( )

extern const char *g_pszFFDispenserModels[];
extern const char *g_pszFFDispenserGibModels[];
extern const char *g_pszFFDispenserSounds[];

extern const char *g_pszFFGenGibModels[];

inline void DispenseHelper( CFFPlayer *pPlayer, int& iAmmo, int iGiveAmmo, const char *pszAmmoType )
{
	// Amount to give must be less than or equal to the amount we have,
	// and the amount we want to give
	int iToGive = min(iGiveAmmo, iAmmo);

	// If there's any to give then, give it
	if( iToGive )
	{
		int iGave = pPlayer->GiveAmmo( iToGive, pszAmmoType );

		// Decrement ammo store by the amount
		// we actually gave
		iAmmo -= iGave;
	}
}

/**
@fn void Spawn( )
@brief Spawns a model - called from Create
@return void
*/ 
void CFFDispenser::Spawn( void )
{
	VPROF_BUDGET( "CFFDispenser::Spawn", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Yeah, you can guess what this does!
	Precache();

	// In shared code now
	// Health value based on TFC
	//m_iMaxHealth = m_iHealth = 150;

	//SetSolid( SOLID_VPHYSICS );

	// Call baseclass spawn stuff
	CFFBuildableObject::Spawn();
	
	// set skin
	CFFPlayer *pOwner = static_cast< CFFPlayer * >( m_hOwner.Get() );
	if( pOwner ) 
		m_nSkin = clamp( pOwner->GetTeamNumber() + 1 - TEAM_BLUE, 0, 4 );	// |-- Mirv: BUG #0000118: SGs are always red	

	UpdateAmmoPercentage();
}

/**
@fn void GoLive( )
@brief Object is built and ready to do it's thing
@return void
*/
void CFFDispenser::GoLive( void )
{
	VPROF_BUDGET( "CFFDispenser::GoLive", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Call base class
	CFFBuildableObject::GoLive();

	// Object is now built
	m_bBuilt = true;

	// Mirv: Now use our stored ground location + orientation
	SetAbsOrigin(m_vecGroundOrigin);
	SetAbsAngles(m_angGroundAngles);

	//OrientBuildableToGround( this, FF_BUILD_DISPENSER );
	
	// Set up our touch function
	SetTouch( &CFFDispenser::OnObjectTouch );	// |-- Mirv: Account for GCC strictness

	// Set up a think function
	SetThink( &CFFDispenser::OnObjectThink );	// |-- Mirv: Account for GCC strictness
	SetNextThink( gpGlobals->curtime + m_flThinkTime );

	CFFPlayer *pOwner = static_cast<CFFPlayer*>(m_hOwner.Get());

	// Bug #0000244: Building dispenser doesn't take away cells
	// Temp value of 100 for now
	if (pOwner)
	{
		// Bug #0001558: exploit to get instant lvl2 SG (applies to dispenser, too -- you can discard and get a 'free' buildable)
		// due to the cells being removed at the end of the build cycle.  Moved RemoveAmmo call to CFFPlayer::PreBuildGenericThink()

		//pOwner->RemoveAmmo(100, AMMO_CELLS);

		int iArmor = min( min( 40, pOwner->GetArmor() ), NeedsArmor() );
		AddAmmo( iArmor, 0, 0, 0, 0 );
		pOwner->RemoveArmor( iArmor );
	}

	// Create our flickerer
	m_pFlickerer = ( CFFBuildableFlickerer * )CreateEntityByName( "ff_buildable_flickerer" );
	if( !m_pFlickerer )
	{
		Warning( "[Dispenser] Failed to create flickerer!\n" );
		m_pFlickerer = NULL;
	}
	else
	{		
		m_pFlickerer->SetBuildable( this );
		m_pFlickerer->Spawn();
	}

	m_flSabotageTime = 0;
	m_hSaboteur = NULL;
	m_bMaliciouslySabotaged = false;
	m_iSaboteurTeamNumber = TEAM_UNASSIGNED;

	// Figure out if we're under water or not
	if( UTIL_PointContents( GetAbsOrigin() + Vector( 0, 0, 48 ) ) & CONTENTS_WATER )
		SetWaterLevel( WL_Eyes );
	else if( UTIL_PointContents( GetAbsOrigin() + Vector( 0, 0, 24 ) ) & CONTENTS_WATER )
		SetWaterLevel( WL_Waist );
	else if( UTIL_PointContents( GetAbsOrigin() ) & CONTENTS_WATER )
		SetWaterLevel( WL_Feet );
}

/**
@fn void OnTouch( )
@brief For when an entity touches our object
@param pOther - pointer to the entity that touched our object
@return void
*/
void CFFDispenser::OnObjectTouch( CBaseEntity *pOther )
{
	VPROF_BUDGET( "CFFDispenser::OnObjectTouch", VPROF_BUDGETGROUP_FF_BUILDABLE );

	CheckForOwner();

	if( pOther )
	{
		if( pOther->IsPlayer() )
		{
			CFFPlayer *pPlayer = ToFFPlayer( pOther );
			if( !pPlayer->IsObserver() && pPlayer->IsAlive() )
			{
				bool bDispense = false;

				// If we haven't been touched...
				if( !m_pLastTouch )
				{
					bDispense = true;
				}
				// We've been touched before...
				else
				{
					// If the last person to touch us touches us again...
					if( pPlayer == m_pLastTouch )
					{
						// If the player hasn't touched us within some time...
						if( ( m_flLastTouch + 0.5f ) < gpGlobals->curtime )
						{
							bDispense = true;
						}
					}
					// Someone new touched us...
					else
					{
						bDispense = true;
					}
				}

				// Give the player junk if we need to
				if( bDispense )
				{
					m_pLastTouch = pPlayer;
					m_flLastTouch = gpGlobals->curtime;

					// Give the player stuff
					Dispense( pPlayer );

					// Update ammo percentage
					UpdateAmmoPercentage();

					// Modify explosion values
					CalcAdjExplosionVal();
				}

				// Now, since someone touched us, see if we need to alert our owner
				// that enemies are using the dispenser and also do our new
				// thing where we send a hud message to the guy that touched us with
				// the custom dispenser text message :evil:
				CFFPlayer *pOwner = ToFFPlayer( m_hOwner.Get() );

				// Quit if I'm touching my own dispenser
				if( pOwner == pPlayer )
					return;

				// If a spy who's disguised as our team (or an ally team) touches us ignore
				if( pPlayer->IsDisguised() && ( FFGameRules()->IsTeam1AlliedToTeam2( pOwner->GetTeamNumber(), pPlayer->GetDisguisedTeam() ) == GR_TEAMMATE ) )
				{
					// Send him the custom message
					// Bug #0001176: dispenser text only showing up for enemy team
					if( Q_strlen( m_szCustomText ) > 1 )
						ClientPrint( pPlayer, HUD_PRINTCENTER, m_szCustomText );

					return;
				}

				// Bug #0000551: Dispenser treats teammates as enemies if mp_friendlyfire is enabled
				//if( g_pGameRules->FCanTakeDamage( pOwner, pPlayer ) )				
				if( FFGameRules()->PlayerRelationship( pOwner, pPlayer ) == GR_NOTTEAMMATE )
				{
					// Touched by non-teammate/non-ally, send custom message
					if( Q_strlen( m_szCustomText ) > 1 )
						ClientPrint( pPlayer, HUD_PRINTCENTER, m_szCustomText );

					// Mirv: Don't do this while sabotaged
					
					//if (!IsSabotaged())
					//{
						// Message owner (wiki says spies don't trigger the "enemies are using..." message)
						if( !pPlayer->IsDisguised() )
						{
							ClientPrint( pOwner, HUD_PRINTCENTER, "#FF_DISPENSER_ENEMIESUSING" );
							
							FF_SendHint( pOwner, ENGY_DISPENEMY, -1, PRIORITY_NORMAL, "#FF_HINT_ENGY_DISPENEMY" );
						}
						// Fire an event.
						IGameEvent *pEvent = gameeventmanager->CreateEvent("dispenser_enemyused");						
						if(pEvent)
						{
							pEvent->SetInt("userid", pOwner->GetUserID());
							pEvent->SetInt("enemyid", pPlayer->GetUserID());
							gameeventmanager->FireEvent(pEvent, true);
						}
					//}
				}
				else
				{
					// Touched by teammate/ally, send him the custom message
					// Bug #0001176: dispenser text only showing up for enemy team
					if( Q_strlen( m_szCustomText ) > 1 )
						ClientPrint( pPlayer, HUD_PRINTCENTER, m_szCustomText );
				}
			}
		}
		else if( pOther->Classify() == CLASS_BACKPACK )
		{
			CFFItemBackpack *pBackpack = dynamic_cast< CFFItemBackpack* > (pOther);

			if ( pBackpack && pBackpack->GetSpawnFlags() & SF_NORESPAWN )
			{
				int iCells = clamp( m_iCells + ( pBackpack->GetAmmoCount( GetAmmoDef()->Index( AMMO_CELLS ) ) ) , 0, m_iMaxCells );
				int iNails = clamp( m_iNails + ( pBackpack->GetAmmoCount( GetAmmoDef()->Index( AMMO_NAILS ) ) ), 0, m_iMaxNails );
				int iShells = clamp( m_iShells + ( pBackpack->GetAmmoCount( GetAmmoDef()->Index( AMMO_SHELLS ) ) ), 0, m_iMaxShells );
				int iRockets = clamp( m_iRockets + ( pBackpack->GetAmmoCount( GetAmmoDef()->Index( AMMO_ROCKETS ) ) ), 0, m_iMaxRockets );

				if (iCells > m_iCells || iNails > m_iNails || iShells > m_iShells || iRockets > m_iRockets)
				{
					UTIL_Remove( pBackpack );

					// Update ammo percentage
					UpdateAmmoPercentage();	

					SendStatsToBot();

					EmitSound( "Dispenser.omnomnom" );
				}
			}
		}
	}
}

/**
@fn void OnThink
@brief Think function (modifies our network health var for now)
@return void
*/
void CFFDispenser::OnObjectThink( void )
{
	VPROF_BUDGET( "CFFDispenser::OnObjectThink", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Do any thinking that needs to be done for _this_ class
	CheckForOwner();

			int iCells = 20;
			int iNails = 20;
			int iShells = 20;
			int iRockets = 10;

	// Generate stock
	m_iCells = clamp( m_iCells + iCells, 0, m_iMaxCells );
	m_iNails = clamp( m_iNails + iNails, 0, m_iMaxNails );
	m_iShells = clamp( m_iShells + iShells, 0, m_iMaxShells );
	m_iRockets = clamp( m_iRockets + iRockets, 0, m_iMaxRockets );
	m_iArmor = clamp( m_iArmor + 50, 0, m_iMaxArmor );

	// Update ammo percentage
	UpdateAmmoPercentage();	

	SendStatsToBot();

	// Set the next time to call this function
	SetNextThink( gpGlobals->curtime + m_flThinkTime );

	// Call base class think func
	CFFBuildableObject::OnObjectThink();
}

void CFFDispenser::Event_Killed( const CTakeDamageInfo &info )
{
	VPROF_BUDGET( "CFFDispenser::Event_Killed", VPROF_BUDGETGROUP_FF_BUILDABLE );

	if( m_hOwner.Get() )
		ClientPrint( ToFFPlayer( m_hOwner.Get() ), HUD_PRINTCENTER, "#FF_DISPENSER_DESTROYED" );

	BaseClass::Event_Killed( info );
}

/**
@fn CFFDispenser *Create( const Vector& vecOrigin, const QAngle& vecAngles, edict_t *pentOwner )
@brief Creates a new CFFDispenser at a particular location
@param vecOrigin - origin of the object to be created
@param vecAngles - view angles of the object to be created
@param pentOwner - edict_t of the owner creating the object (usually keep NULL)
@return a _new_ CFFDispenser
*/ 
CFFDispenser *CFFDispenser::Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner )
{
	// Create the object
	CFFDispenser *pObject = ( CFFDispenser * )CBaseEntity::Create( "FF_Dispenser", vecOrigin, vecAngles, NULL );

	// Set our faux owner - see CFFBuildable::Create for the reason why
	pObject->m_hOwner.GetForModify() = pentOwner;

	//pObject->VPhysicsInitNormal( SOLID_VPHYSICS, pObject->GetSolidFlags(), true );

	// Spawn the object
	//pObject->Spawn();
	// commented out for better nobuild functions, will be spawned ONLY IF it's allowed to, from ff_player.cpp

	return pObject;
}

//
// Dispense
//		Actually give a player some stuff
//
void CFFDispenser::Dispense( CFFPlayer *pPlayer )
{
	VPROF_BUDGET( "CFFDispenser::Dispense", VPROF_BUDGETGROUP_FF_BUILDABLE );

	if( pPlayer->GetClassSlot() == CLASS_ENGINEER )
		DispenseHelper( pPlayer, m_iCells.GetForModify(), 75, AMMO_CELLS ); // Engies get 75 cells
	else
    	DispenseHelper( pPlayer, m_iCells.GetForModify(), m_iGiveCells, AMMO_CELLS ); // Everyone else gets 10 cells
	DispenseHelper( pPlayer, m_iNails.GetForModify(), m_iGiveNails, AMMO_NAILS );
	DispenseHelper( pPlayer, m_iShells.GetForModify(), m_iGiveShells, AMMO_SHELLS );
	DispenseHelper( pPlayer, m_iRockets.GetForModify(), m_iGiveRockets, AMMO_ROCKETS );

	// Give armor if we can
	if( m_iArmor > 0 )
	{
		int iToGive = min( m_iArmor, m_iGiveArmor );
		if( iToGive )
		{
			int iGave = pPlayer->AddArmor( min( pPlayer->NeedsArmor(), iToGive ) );
			m_iArmor.GetForModify() -= iGave;
		}
	}

	// Mirv: sabotage
	// We can call this over and over, it will only ever reduce one level
	//if (IsSabotaged())
	//	pPlayer->ReduceArmorClass();

	Omnibot::Notify_GotDispenserAmmo(pPlayer);
}

//
// AddAmmo
//		Put stuff into the dispenser
//
void CFFDispenser::AddAmmo( int iArmor, int iCells, int iShells, int iNails, int iRockets )
{
	VPROF_BUDGET( "CFFDispenser::AddAmmo", VPROF_BUDGETGROUP_FF_BUILDABLE );

	m_iArmor = min( m_iArmor + iArmor, m_iMaxArmor );
	m_iCells = min( m_iCells + iCells, m_iMaxCells );
	m_iShells = min( m_iShells + iShells, m_iMaxShells );
	m_iNails = min( m_iNails + iNails, m_iMaxNails );
	m_iRockets = min( m_iRockets + iRockets, m_iMaxRockets );

	UpdateAmmoPercentage();
}

//
// CalcAdjExplosionVal
//		Calculates an adjustment to be made to the explosion
//		based on how much stuff is in the dispenser
void CFFDispenser::CalcAdjExplosionVal( void )
{
	VPROF_BUDGET( "CFFDispenser::CalcAdjExplosionVal", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// TODO: Calculate some percentage full and modify the 
	// explosion magnitude value by taking this percentage
	// times the original explosion magnitude value

	// From constructor
	//// Default values
	//m_iExplosionMagnitude = 50;
	//m_flExplosionMagnitude = 50.0f;
	//m_flExplosionRadius = 3.5f * m_flExplosionMagnitude;
	//m_iExplosionRadius = ( int )m_flExplosionRadius;	
	//m_flExplosionForce = 100.0f;
	//// TODO: for now - change this later? remember to update in dispenser.cpp as well
	//m_flExplosionDamage = m_flExplosionForce;

	float flPercent = ( float )( m_iAmmoPercent / 100.0f );

	m_iExplosionMagnitude = 50 * ( 1 + flPercent );
	m_flExplosionMagnitude = 50.0f * ( 1 + flPercent );

	// Update values for when we explode
	m_flExplosionRadius = 3.5f * m_flExplosionMagnitude;
	m_iExplosionRadius = ( int )m_flExplosionRadius;
	// TODO: for now - remember to change it to what it
	// is for the base class or whatever
	m_flExplosionForce = 100.0f * ( 1 + flPercent );
	m_flExplosionDamage = m_flExplosionForce;
}

//
// UpdateAmmoPercentage
//		Calculates the percent ammo the dispenser has
//
void CFFDispenser::UpdateAmmoPercentage( void )
{
	VPROF_BUDGET( "CFFDispenser::UpdateAmmoPercentage", VPROF_BUDGETGROUP_FF_BUILDABLE );

	float flAmmo = m_iCells + m_iNails + m_iShells + m_iRockets + m_iArmor;
	float flMaxAmmo = m_iMaxCells + m_iMaxNails + m_iMaxShells + m_iMaxRockets + m_iMaxArmor;

	m_iAmmoPercent = ( ( flAmmo / flMaxAmmo ) * 100 );
}

//-----------------------------------------------------------------------------
// Purpose: Flags the dispenser as sabotaged.
//			This results in:
//				· Reducing armour types
//				· No more enemy warning messages
//-----------------------------------------------------------------------------
void CFFDispenser::Sabotage(CFFPlayer *pSaboteur)
{
	VPROF_BUDGET( "CFFDispenser::Sabotage", VPROF_BUDGETGROUP_FF_BUILDABLE );

	if ( !pSaboteur )
		return;

	m_flSabotageTime = gpGlobals->curtime + FF_BUILD_SABOTAGE_TIMEOUT;
	m_hSaboteur = pSaboteur;
	m_iSaboteurTeamNumber = m_hSaboteur->GetTeamNumber();

	// AfterShock - scoring system: 25 points for sabotage dispenser
	pSaboteur->AddFortPoints(25, "#FF_FORTPOINTS_SABOTAGEDISPENSER");

	//Warning("Dispenser sabotaged\n");
}

//-----------------------------------------------------------------------------
// Purpose: This blows up the sentry a la detdispenser (nice and simple)
//-----------------------------------------------------------------------------
void CFFDispenser::MaliciouslySabotage(CFFPlayer *pSaboteur)
{
	VPROF_BUDGET( "CFFDispenser::MaliciousSabotage", VPROF_BUDGETGROUP_FF_BUILDABLE );

	BaseClass::MaliciouslySabotage( pSaboteur );

	if ( !pSaboteur )
		return;

	// Explode SG on sabotage finish
	// TODO: create custom death message for it
	if ( pSaboteur )
		TakeDamage( CTakeDamageInfo( this, pSaboteur, 10000, DMG_GENERIC ) );
	else	// Jiggles: I'm not sure what would happen if the Saboteur leaves the server before this
	{
		CFFPlayer *pOwner = ToFFPlayer( m_hOwner.Get() );
		if ( pOwner )
			TakeDamage( CTakeDamageInfo( this, pOwner, 10000, DMG_GENERIC ) );
	}

	//Warning("Dispenser maliciously sabotaged\n");
}

//-----------------------------------------------------------------------------
// Purpose: Overridden just to fire the appropriate event.
//-----------------------------------------------------------------------------
void CFFDispenser::Detonate()
{
	VPROF_BUDGET( "CFFDispenser::Detonate", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Fire an event.
	IGameEvent *pEvent = gameeventmanager->CreateEvent("dispenser_detonated");						
	if(pEvent)
	{
		if (m_hOwner.Get())
		{
			CFFPlayer *pOwner = static_cast<CFFPlayer*>(m_hOwner.Get());
			pEvent->SetInt("userid", pOwner->GetUserID());
			gameeventmanager->FireEvent(pEvent, true);
		}		
	}

	CFFBuildableObject::Detonate();
}

//-----------------------------------------------------------------------------
// Purpose: Carry out the radius damage for this buildable
//-----------------------------------------------------------------------------
void CFFDispenser::DoExplosionDamage()
{
	VPROF_BUDGET( "CFFDispenser::DoExplosionDamage", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Using the values posted in the bugtracker, capping at 200
	float flDamage = 1.4f * m_iRockets + 1.0f * m_iCells;
	flDamage = min(200, flDamage);

	// In TFC the damage comes from the base of the dispenser
	if (m_hOwner.Get())
	{
		CTakeDamageInfo info(this, m_hOwner, vec3_origin, GetAbsOrigin(), flDamage, DMG_BLAST);
		RadiusDamage(info, GetAbsOrigin(), 625, CLASS_NONE, NULL);
		
		UTIL_ScreenShake(GetAbsOrigin(), flDamage * 0.0125f, 150.0f, m_flExplosionDuration, 620.0f, SHAKE_START);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Spawn dispenser specific gibs
//-----------------------------------------------------------------------------
void CFFDispenser::SpawnGibs()
{
	CFFPlayer *pOwner = static_cast< CFFPlayer * >( m_hOwner.Get() );
	
	if( !pOwner ) 
		return;

	CEffectData data;
		data.m_nEntIndex = entindex();
		data.m_vOrigin = GetAbsOrigin();
		data.m_nMaterial = clamp( pOwner->GetTeamNumber() + 1 - TEAM_BLUE, 0, 4 ); // using this for skin, not sure what it's meant to be used for
	DispatchEffect("DispenserGib", data);
}

//-----------------------------------------------------------------------------
// Purpose: Carry out the radius damage for this buildable
//-----------------------------------------------------------------------------
void CFFDispenser::PhysicsSimulate()
{
	BaseClass::PhysicsSimulate();

	// Update the client every 0.2 seconds
	if (gpGlobals->curtime > m_flLastClientUpdate + 0.2f)
	{
		m_flLastClientUpdate = gpGlobals->curtime;

		CFFPlayer *pPlayer = ToFFPlayer( m_hOwner.Get() );

		if (!pPlayer)
			return;

		int iHealth = (int) (100.0f * GetHealth() / GetMaxHealth());
		int iAmmo = m_iAmmoPercent;

		// If things haven't changed then do nothing more
		int iState = iHealth + (iAmmo << 8);
		if (m_iLastState == iState)
			return;

		CSingleUserRecipientFilter user(pPlayer);
		user.MakeReliable();

		UserMessageBegin(user, "DispenserMsg");
		WRITE_BYTE(iHealth);
		WRITE_BYTE(iAmmo);
		MessageEnd();

		m_iLastState = iState;
	}
}
    
bool CFFDispenser::CloseEnoughToDismantle( CFFPlayer *pPlayer)
{
    return (pPlayer->GetAbsOrigin() - GetAbsOrigin()).LengthSqr() < 6400.0f;
}

void CFFDispenser::Dismantle( CFFPlayer *pPlayer)
{
	// Bug #0000333: Buildable Behavior (non build slot) while building
	pPlayer->GiveAmmo( (FF_BUILDCOST_DISPENSER/2) , AMMO_CELLS, true);

	// Bug #0000426: Buildables Dismantle Sounds Missing
	CPASAttenuationFilter sndFilter( this );
	EmitSound( sndFilter, entindex(), "Dispenser.unbuild" );

	// Fire an event.
	IGameEvent *pEvent = gameeventmanager->CreateEvent("dispenser_dismantled");		
	if(pEvent)
	{
		pEvent->SetInt("userid", pPlayer->GetUserID());
		gameeventmanager->FireEvent(pEvent, true);
	}

	RemoveQuietly();
}