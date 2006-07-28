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
//#include "ff_sevtest.h"

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
	SendPropInt( SENDINFO( m_iCells ) ),
	SendPropInt( SENDINFO( m_iShells ) ),
	SendPropInt( SENDINFO( m_iNails ) ),
	SendPropInt( SENDINFO( m_iRockets ) ),
	SendPropInt( SENDINFO( m_iArmor ) ),
	SendPropInt( SENDINFO( m_iRadioTags ) ),
END_SEND_TABLE()

// Start of our data description for the class
BEGIN_DATADESC( CFFDispenser )
	DEFINE_ENTITYFUNC( OnObjectTouch ),
	DEFINE_THINKFUNC( OnObjectThink ),
END_DATADESC( )

// Array of char *'s to dispenser models
const char *g_pszFFDispenserModels[ ] =
{
	FF_DISPENSER_MODEL,
	NULL
};

// Array of char *'s to gib models
const char *g_pszFFDispenserGibModels[ ] =
{
	FF_DISPENSER_GIB01_MODEL,
	FF_DISPENSER_GIB02_MODEL,
	FF_DISPENSER_GIB03_MODEL,
	FF_DISPENSER_GIB04_MODEL,
	NULL
};

// Array of char *'s to sounds
const char *g_pszFFDispenserSounds[ ] =
{
	FF_DISPENSER_BUILD_SOUND,
	FF_DISPENSER_EXPLODE_SOUND,
	FF_DISPENSER_UNBUILD_SOUND,
	NULL
};

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
	// Yeah, you can guess what this does!
	Precache();

	// In shared code now
	// Health value based on TFC
	//m_iMaxHealth = m_iHealth = 150;

	//SetSolid( SOLID_VPHYSICS );

	// Call baseclass spawn stuff
	CFFBuildableObject::Spawn();

	UpdateAmmoPercentage();
}

/**
@fn void GoLive( )
@brief Object is built and ready to do it's thing
@return void
*/
void CFFDispenser::GoLive( void )
{
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
		pOwner->RemoveAmmo(100, AMMO_CELLS);

		int iArmor = min( min( 40, pOwner->GetArmor() ), NeedsArmor() );
		AddAmmo( iArmor, 0, 0, 0, 0, 0 );
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
}

/**
@fn void OnTouch( )
@brief For when an entity touches our object
@param pOther - pointer to the entity that touched our object
@return void
*/
void CFFDispenser::OnObjectTouch( CBaseEntity *pOther )
{
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
					//DevMsg( "[Dispenser] Have not been touched before!\n" );

					bDispense = true;
				}
				// We've been touched before...
				else
				{
					//DevMsg( "[Dispenser] Has been touched before!\n" );

					// If the last person to touch us touches us again...
					if( pPlayer == m_pLastTouch )
					{
						//DevMsg( "[Dispenser] Last person to touch us was this guy!\n" );

						// If the player hasn't touched us within some time...
						if( ( m_flLastTouch + 2.0f ) < gpGlobals->curtime )
						{
							bDispense = true;
						}
					}
					// Someone new touched us...
					else
					{
						//DevMsg( "[Dispenser] Someone new touched us!\n" );

						bDispense = true;
					}
				}

				// Give the player junk if we need to
				if( bDispense )
				{
					//DevMsg( "[Dispenser] Dispensing @ time: %f\n", gpGlobals->curtime );

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
					return;

				// Bug #0000551: Dispenser treats teammates as enemies if mp_friendlyfire is enabled
				//if( g_pGameRules->FPlayerCanTakeDamage( pOwner, pPlayer ) )				
				if( FFGameRules()->PlayerRelationship( pOwner, pPlayer ) == GR_NOTTEAMMATE )
				{
					// Mirv: Don't do this while sabotaged
					if (!IsSabotaged())
					{
						// TODO: Hud message to owner					
						SendMessageToPlayer( pOwner, "Dispenser_EnemiesUsing" );

						// TODO: Hud message to person who touched us
						SendMessageToPlayer( pPlayer, "Dispenser_TouchEnemy", true );

						// Fire an event.
						IGameEvent *pEvent = gameeventmanager->CreateEvent("dispenser_enemyused");						
						if(pEvent)
						{
							pEvent->SetInt("userid", pOwner->GetUserID());
							pEvent->SetInt("enemyid", pPlayer->GetUserID());
							gameeventmanager->FireEvent(pEvent, true);
						}
					}
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
	// Do any thinking that needs to be done for _this_ class
	CheckForOwner();

	// Generate stock
	m_iShells = clamp( m_iShells + 20, 0, m_iMaxShells );
	m_iNails = clamp( m_iNails + 30, 0, m_iMaxNails );
	m_iCells = clamp( m_iCells + 20, 0, m_iMaxCells );
	m_iRockets = clamp( m_iRockets + 10, 0, m_iMaxRockets );
	m_iArmor = clamp( m_iArmor + 50, 0, m_iMaxArmor );
	m_iRadioTags = clamp( m_iRadioTags + 10, 0, m_iMaxRadioTags );

	// Update ammo percentage
	UpdateAmmoPercentage();

	// Call base class think func
	CFFBuildableObject::OnObjectThink();

	SendStatsToBot();

	// Set the next time to call this function
	SetNextThink( gpGlobals->curtime + m_flThinkTime );
}

void CFFDispenser::Event_Killed( const CTakeDamageInfo &info )
{
	if( m_hOwner.Get() )
		SendMessageToPlayer( ToFFPlayer( m_hOwner.Get() ), "Dispenser_Destroyed" );

	BaseClass::Event_Killed( info );
}

void CFFDispenser::SendMessageToPlayer( CFFPlayer *pPlayer, const char *pszMessage, bool bDispenserText )
{
	if( !pPlayer )
		return;

	CSingleUserRecipientFilter user( pPlayer );
	user.MakeReliable();

	// Begin message block
	UserMessageBegin( user, pszMessage );

	// Write something
	WRITE_SHORT( 1 );

	// If we're sending the custom text...
	if( bDispenserText )
	{
		// TODO: Make sure it's not null... want to send at least one character
		if( ( int )strlen( m_szCustomText ) < 1 )
			WRITE_STRING( "@1" );
		else
			WRITE_STRING( m_szCustomText );
	}

	// End of message block
	MessageEnd();
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
	pObject->Spawn();

	return pObject;
}

//
// Dispense
//		Actually give a player some stuff
//
void CFFDispenser::Dispense( CFFPlayer *pPlayer )
{
	if( pPlayer->GetClassSlot() == CLASS_ENGINEER )
		DispenseHelper( pPlayer, m_iCells.GetForModify(), 75, AMMO_CELLS ); // Engies get 75 cells
	else
    	DispenseHelper( pPlayer, m_iCells.GetForModify(), m_iGiveCells, AMMO_CELLS ); // Everyone else gets 10 cells
	DispenseHelper( pPlayer, m_iNails.GetForModify(), m_iGiveNails, AMMO_NAILS );
	DispenseHelper( pPlayer, m_iShells.GetForModify(), m_iGiveShells, AMMO_SHELLS );
	DispenseHelper( pPlayer, m_iRockets.GetForModify(), m_iGiveRockets, AMMO_ROCKETS );
	DispenseHelper( pPlayer, m_iRadioTags.GetForModify(), m_iGiveRadioTags, AMMO_RADIOTAG );

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
	if (IsSabotaged())
		pPlayer->ReduceArmorClass();
}

//
// AddAmmo
//		Put stuff into the dispenser
//
void CFFDispenser::AddAmmo( int iArmor, int iCells, int iShells, int iNails, int iRockets, int iRadioTags )
{
	m_iArmor = min( m_iArmor + iArmor, m_iMaxArmor );
	m_iCells = min( m_iCells + iCells, m_iMaxCells );
	m_iShells = min( m_iShells + iShells, m_iMaxShells );
	m_iNails = min( m_iNails + iNails, m_iMaxNails );
	m_iRockets = min( m_iRockets + iRockets, m_iMaxRockets );
	m_iRadioTags = min( m_iRadioTags + iRadioTags, m_iMaxRadioTags );

	UpdateAmmoPercentage();
}

//
// CalcAdjExplosionVal
//		Calculates an adjustment to be made to the explosion
//		based on how much stuff is in the dispenser
void CFFDispenser::CalcAdjExplosionVal( void )
{
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
	float flAmmo = m_iCells + m_iNails + m_iShells + m_iRockets + m_iRadioTags + m_iArmor;
	float flMaxAmmo = m_iMaxCells + m_iMaxNails + m_iMaxShells + m_iMaxRockets + m_iMaxRadioTags + m_iMaxArmor;

	m_iAmmoPercent = ( ( flAmmo / flMaxAmmo ) * 100 );
}

void CFFDispenser::SendStatsToBot()
{
	CFFPlayer *pOwner = static_cast<CFFPlayer*>(m_hOwner.Get());
	if(pOwner && pOwner->IsBot())
	{
		Omnibot::BotUserData bud;
		bud.DataType = Omnibot::BotUserData::dt6_2byteFlags;
		bud.udata.m_2ByteFlags[0] = m_iHealth;
		bud.udata.m_2ByteFlags[1] = m_iShells;
		bud.udata.m_2ByteFlags[2] = m_iNails;
		bud.udata.m_2ByteFlags[3] = m_iRockets;
		bud.udata.m_2ByteFlags[4] = m_iCells;
		bud.udata.m_2ByteFlags[5] = m_iArmor;

		// TODO: Add in radio tag?

		int iGameId = pOwner->entindex()-1;
		Omnibot::omnibot_interface::Bot_Interface_SendEvent(
			Omnibot::TF_MSG_DISPENSER_STATS,
			iGameId, 0, 0, &bud);
	}
}

//-----------------------------------------------------------------------------
// Purpose: If already sabotaged then don't try and sabotage again
//-----------------------------------------------------------------------------
bool CFFDispenser::CanSabotage()
{
	if (!m_bBuilt)
		return false;

	return !IsSabotaged();
}

//-----------------------------------------------------------------------------
// Purpose: Is this building in level 1 sabotage
//-----------------------------------------------------------------------------
bool CFFDispenser::IsSabotaged()
{
	return (m_hSaboteur && m_flSabotageTime > gpGlobals->curtime);
}

//-----------------------------------------------------------------------------
// Purpose: Flags the dispenser as sabotaged.
//			This results in:
//				· Reducing armour types
//				· No more enemy warning messages
//-----------------------------------------------------------------------------
void CFFDispenser::Sabotage(CFFPlayer *pSaboteur)
{
	m_flSabotageTime = gpGlobals->curtime + 120.0f;
	m_hSaboteur = pSaboteur;

	Warning("Dispenser sabotaged\n");
}

//-----------------------------------------------------------------------------
// Purpose: This blows up the sentry a la detdispenser (nice and simple)
//-----------------------------------------------------------------------------
void CFFDispenser::MaliciousSabotage(CFFPlayer *pSaboteur)
{
	Detonate();

	Warning("Dispenser maliciously sabotaged\n");
}

//-----------------------------------------------------------------------------
// Purpose: Overridden just to fire the appropriate event.
//-----------------------------------------------------------------------------
void CFFDispenser::Detonate()
{
	// Fire an event.
	IGameEvent *pEvent = gameeventmanager->CreateEvent("dispenser_detonated");						
	if(pEvent)
	{
		CFFPlayer *pOwner = static_cast<CFFPlayer*>(m_hOwner.Get());
		pEvent->SetInt("userid", pOwner->GetUserID());
		gameeventmanager->FireEvent(pEvent, true);
	}

	CFFBuildableObject::Detonate();
}

//-----------------------------------------------------------------------------
// Purpose: Carry out the radius damage for this buildable
//-----------------------------------------------------------------------------
void CFFDispenser::DoExplosionDamage()
{
	// Using the values posted in the bugtracker, capping at 200
	float flDamage = 1.4f * m_iRockets + 1.0f * m_iCells;
	flDamage = min(200, flDamage);

	// In TFC the damage comes from the base of the dispenser
	CTakeDamageInfo info(this, m_hOwner, vec3_origin, GetAbsOrigin(), flDamage, DMG_BLAST);
	RadiusDamage(info, GetAbsOrigin(), 625, CLASS_NONE, NULL);

	UTIL_ScreenShake(GetAbsOrigin(), flDamage * 0.0125f, 150.0f, m_flExplosionDuration, 620.0f, SHAKE_START);
}