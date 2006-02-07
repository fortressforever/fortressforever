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

#include "cbase.h"
#include "ff_dispenser.h"
#include "const.h"
#include "ff_weapon_base.h"
#include "ff_sevtest.h"

#include "omnibot_interface.h"

LINK_ENTITY_TO_CLASS( FF_Dispenser_entity, CFFDispenser );
PRECACHE_REGISTER( FF_Dispenser_entity );

IMPLEMENT_SERVERCLASS_ST( CFFDispenser, DT_FFDispenser )
	SendPropInt( SENDINFO( m_iCells ) ),
	SendPropInt( SENDINFO( m_iShells ) ),
	SendPropInt( SENDINFO( m_iNails ) ),
	SendPropInt( SENDINFO( m_iRockets ) ),
	SendPropInt( SENDINFO( m_iArmor ) ),
END_SEND_TABLE( )

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
@fn CFFDispenser
@brief Constructor
@return N/A
*/
CFFDispenser::CFFDispenser( void )
{
	// Overwrite the base class stubs
	m_ppszModels = g_pszFFDispenserModels;
	m_ppszGibModels = g_pszFFDispenserGibModels;
	m_ppszSounds = g_pszFFDispenserSounds;

	// Max values
	m_iMaxCells		= 400;
	m_iMaxShells	= 400;
	m_iMaxNails		= 600;
	m_iMaxRockets	= 300;
	m_iMaxArmor		= 500;

	// Give values - values to give a player when they touch us
	m_iGiveCells	= 20;
	m_iGiveShells	= 20;
	m_iGiveNails	= 20;
	m_iGiveRockets	= 20;
	m_iGiveArmor	= 10;

	// Time in seconds between generating shiz
	m_flThinkTime = 10.0f;

	// Initialize
	m_pLastTouch = NULL;
	m_flLastTouch = 0.0f;

	// Store a value from the base class
	m_flOrigExplosionMagnitude = m_flExplosionMagnitude;
}

/**
@fn ~CFFDispenser
@brief Deconstructor
@return N/A
*/
CFFDispenser::~CFFDispenser( void )
{
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

	// Health value based on TFC
	m_iMaxHealth = m_iHealth = 150;

	// TODO: Give these _real_ values
	// Initialize dispenser items
	m_iCells = 20;
	m_iShells = 20;
	m_iNails = 20;
	m_iRockets = 20;
	m_iArmor = 20;	

	// Call baseclass spawn stuff
	CFFBuildableObject::Spawn();
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

	// Mirv: Now use our stored ground location + orientation
	SetAbsOrigin(m_vecGroundOrigin);
	SetAbsAngles(m_angGroundAngles);

	//OrientBuildableToGround( this, FF_BUILD_DISPENSER );
	
	// Set up our touch function
	SetTouch( &CFFDispenser::OnObjectTouch );	// |-- Mirv: Account for GCC strictness

	// Set up a think function
	SetThink( &CFFDispenser::OnObjectThink );	// |-- Mirv: Account for GCC strictness
	SetNextThink( gpGlobals->curtime + m_flThinkTime );

	// Omni-bot: Tell the bot about his dispenser.
	CFFPlayer *pOwner = static_cast<CFFPlayer*>(m_hOwner.Get());
	if(pOwner && pOwner->IsBot())
	{
		int iGameId = pOwner->entindex()-1;

		Omnibot::BotUserData bud(edict());
		Omnibot::omnibot_interface::Bot_Interface_SendEvent(
			Omnibot::TF_MESSAGE_DISPENSER_BUILT,
			iGameId, 0, 0, &bud);
		SendStatsToBot();
	}

	// Bug #0000244: Building dispenser doesn't take away cells
	// Temp value of 100 for now
	if (pOwner)
		pOwner->RemoveAmmo(100, AMMO_CELLS);
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
			if( !pPlayer->IsObserver() ) // make sure spectators aren't touching the object
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

				if( g_pGameRules->FPlayerCanTakeDamage( pOwner, pPlayer ) )
				{
					// TODO: Hud message to owner					
					SendMessageToPlayer( pOwner, "Dispenser_EnemiesUsing" );

					// TODO: Hud message to person who touched us
					SendMessageToPlayer( pPlayer, "Dispenser_TouchEnemy", true );

					// Omni-bot: Tell the bot about his dispenser.
					if(pOwner->IsBot())
					{
						int iGameId = pOwner->entindex()-1;

						Omnibot::BotUserData bud(pPlayer->edict());
						Omnibot::omnibot_interface::Bot_Interface_SendEvent(
							Omnibot::TF_MESSAGE_DISPENSER_ENEMYUSED,
							iGameId, 0, 0, &bud);
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
	m_iNails = clamp( m_iNails + 20, 0, m_iMaxNails );
	m_iCells = clamp( m_iCells + 20, 0, m_iMaxCells );
	m_iRockets = clamp( m_iRockets + 10, 0, m_iMaxRockets );
	m_iArmor = clamp( m_iArmor + 20, 0, m_iMaxArmor );

	// Call base class think func
	CFFBuildableObject::OnObjectThink();

	SendStatsToBot();

	// Set the next time to call this function - right away
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
	CFFDispenser *pObject = ( CFFDispenser * )CBaseEntity::Create( "FF_Dispenser_entity", vecOrigin, vecAngles, NULL );

	// Set our faux owner - see CFFBuildable::Create for the reason why
	pObject->m_hOwner = pentOwner;

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
	DispenseHelper( pPlayer, m_iCells.GetForModify(), m_iGiveCells, AMMO_CELLS );
	DispenseHelper( pPlayer, m_iNails.GetForModify(), m_iGiveNails, AMMO_NAILS );
	DispenseHelper( pPlayer, m_iShells.GetForModify(), m_iGiveShells, AMMO_SHELLS );
	DispenseHelper( pPlayer, m_iRockets.GetForModify(), m_iGiveRockets, AMMO_ROCKETS );

	// Give armor if we can
	if( m_iArmor > 0 )
	{
		int iToGive = m_iGiveArmor;

		if( m_iArmor < m_iGiveArmor )
			iToGive = m_iArmor;

		if( iToGive )
		{
			int iAccept = pPlayer->GetMaxArmor() - pPlayer->GetArmor();

			if( iToGive < iAccept )
				iToGive = iAccept;

			if( iToGive )
			{
				pPlayer->AddArmor( iToGive );

				m_iArmor.GetForModify() -= iToGive;
			}
		}
	}
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

	// Update values for when we explode
	m_iExplosionMagnitude = ( int )m_flExplosionMagnitude;
	m_flExplosionRadius = 3.5f * m_flExplosionMagnitude;
	m_iExplosionRadius = ( int )m_flExplosionRadius;
	// TODO: for now - remember to change it to what it
	// is for the base class or whatever
	m_flExplosionDamage = m_flExplosionForce;
}

void CFFDispenser::SendStatsToBot()
{
	CFFPlayer *pOwner = static_cast<CFFPlayer*>(m_hOwner.Get());
	if(pOwner && pOwner->IsBot())
	{
		Omnibot::BotUserData bud;
		bud.m_DataType = Omnibot::BotUserData::dt6_2byteFlags;
		bud.udata.m_2ByteFlags[0] = m_iHealth;
		bud.udata.m_2ByteFlags[1] = m_iShells;
		bud.udata.m_2ByteFlags[2] = m_iNails;
		bud.udata.m_2ByteFlags[3] = m_iRockets;
		bud.udata.m_2ByteFlags[4] = m_iCells;
		bud.udata.m_2ByteFlags[5] = m_iArmor;

		int iGameId = pOwner->entindex()-1;
		Omnibot::omnibot_interface::Bot_Interface_SendEvent(
			Omnibot::TF_MESSAGE_DISPENSER_STATS,
			iGameId, 0, 0, &bud);
	}
}
