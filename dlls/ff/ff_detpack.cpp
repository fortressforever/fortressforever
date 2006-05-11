// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_detpack.cpp
// @author Patrick O'Leary (Mulchman)
// @date 12/28/2005
// @brief Detpack class
//
// REVISIONS
// ---------
//	12/28/2005, Mulchman: 
//		First created
//
//	05/09/2005, Mulchman:
//		Tons of little updates here and there
//
//	09/28/2005,	Mulchman:
//		Played with the think time and adding
//		support for whine sound when hit by
//		emp or 5 seconds left to go until
//		explode time

#include "cbase.h"
#include "ff_buildableobjects_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( FF_Detpack, CFFDetpack );
PRECACHE_REGISTER( FF_Detpack );

IMPLEMENT_SERVERCLASS_ST( CFFDetpack, DT_FFDetpack )
END_SEND_TABLE( )

// Start of our data description for the class
BEGIN_DATADESC( CFFDetpack )
	DEFINE_ENTITYFUNC( OnObjectTouch ),
	DEFINE_THINKFUNC( OnObjectThink ),
END_DATADESC( )

// Array of char *'s to dispenser models
const char *g_pszFFDetpackModels[ ] =
{
	FF_DETPACK_MODEL,
	NULL
};

// Array of char *'s to gib models
const char *g_pszFFDetpackGibModels[ ] =
{
	NULL
};

// Array of char *'s to sounds
const char *g_pszFFDetpackSounds[ ] =
{
	FF_DETPACK_BUILD_SOUND,
	FF_DETPACK_EXPLODE_SOUND,
	"Detpack.FiveSeconds",
	"Detpack.Defuse",
	NULL
};

/**
@fn CFFDetpack
@brief Constructor
@return N/A
*/
CFFDetpack::CFFDetpack( void )
{
	// Overwrite the base class stubs
	m_ppszModels = g_pszFFDetpackModels;
	m_ppszGibModels = g_pszFFDetpackGibModels;
	m_ppszSounds = g_pszFFDetpackSounds;

	// Override some values
	m_iExplosionMagnitude = 200;
	m_flExplosionMagnitude = 200.0f;
	m_flExplosionRadius = 3.5f * m_flExplosionMagnitude;
	m_iExplosionRadius = ( int )m_flExplosionRadius;	
	m_flExplosionForce = 300.0f;
	m_flExplosionDamage = m_flExplosionForce; // for now
	m_flExplosionDuration = 2.0f;
	m_iExplosionFireballScale = random->RandomInt( 20, 35 );

	m_bTakesDamage = false;

	// Use the shockwave
	m_bShockWave = true;

	// Override
	m_bTranslucent = false;
	m_bUsePhysics = true;
	
	//m_bLive = false;

	// Default
	m_iFuseTime = 5;
	m_bFiveSeconds = false;
}

/**
@fn ~CFFDetpack
@brief Deconstructor
@return N/A
*/
CFFDetpack::~CFFDetpack( void )
{
}

/**
@fn void Spawn( )
@brief Spawns a model - called from Create
@return void
*/ 
void CFFDetpack::Spawn( void )
{
	// Yeah, you can guess what this does!
	Precache( );
	
	m_iHealth = 1;

	// Call baseclass spawn stuff
	CFFBuildableObject::Spawn();

	// Done in baseclass
	//SetSolid( SOLID_VPHYSICS );
}

/**
@fn void GoLive( )
@brief Object is built and ready to do it's thing
@return void
*/
void CFFDetpack::GoLive( void )
{
	// Call base class
	CFFBuildableObject::GoLive();

	// Object is now built
	m_bBuilt = true;

	// Tell client to start the timer
	SendStartTimerMessage();

	// Set up when we're supposed to blow up
	float flCurTime = gpGlobals->curtime;
	m_flDetonateTime = flCurTime + ( float )m_iFuseTime;

	// Set up our touch function
	SetTouch( &CFFDetpack::OnObjectTouch );		// |-- Mirv: Account for GCC strictness

	// Set up a think function
	SetThink( &CFFDetpack::OnObjectThink );		// |-- Mirv: Account for GCC strictness

	// Set next think time to be our explode time - 5 seconds
	// so we can start up our 5 second count down
	// If it's a 5 second timer, then we need to start the countdown now
	if( m_iFuseTime <= 5 )
		m_flThinkTime = 0.001f; // immediate
	else
		m_flThinkTime = m_flDetonateTime - flCurTime - 5.0f;

	SetNextThink( flCurTime + m_flThinkTime );

	DevMsg( "[Detpack] Next think in: %f seconds\n", flCurTime + m_flThinkTime );
}

/**
@fn void OnTouch( )
@brief For when an entity touches our object
@param pOther - pointer to the entity that touched our object
@return void
*/
void CFFDetpack::OnObjectTouch( CBaseEntity *pOther )
{
	CheckForOwner();

	if( !m_bBuilt )
		return;

	if( !pOther )
		return;

	if( !pOther->IsPlayer() )
		return;

	CFFPlayer *pPlayer = ToFFPlayer( pOther );

	// Skip the owner
	if( ( ( CFFPlayer * )m_hOwner.Get() ) == pPlayer )
		return;
			
	// Skip if spectator
	if( pPlayer->IsObserver() ) // make sure spectators aren't touching the object
		return;

	// Skip if not scout
    if( pPlayer->GetClassSlot() != 1 )
        return;

	// Skip if person touching can't damage us
	if( !g_pGameRules->FPlayerCanTakeDamage( ( ( CFFPlayer * )m_hOwner.Get() ), pPlayer ) )
		return;

	// Tell client to stop the timer
	SendStopTimerMessage();

	// Play defuse sound
	CPASAttenuationFilter sndFilter( this );
	EmitSound( sndFilter, entindex(), "Detpack.Defuse" );

	// Finally remove
	RemoveQuietly();	
}

/**
@fn void OnThink
@brief Think function
@return void
*/
void CFFDetpack::OnObjectThink( void )
{
	CheckForOwner();

	// First time we come here it will be 5 seconds before we need 
	// to blow up
	DevMsg( "[Detpack] In think function!\n" );

	if( !m_bFiveSeconds )
	{
		DevMsg( "[Detpack] Setting 5 second timer\n" );
		
		// Play the 5 second to go sound (whine up) whatever.
		EmitSound( m_ppszSounds[ 2 ] );		

		m_bFiveSeconds = true;

		SetNextThink( gpGlobals->curtime + 5.0f );
	}
	else
	{
		DevMsg( "[Detpack] Detonating\n" );

		// Tell player to stop timer (in case the detpack took emp damage)
		SendStopTimerMessage();

		// Second time calling the think func, so time to blow up!
		Detonate();		
	}
}

void CFFDetpack::SendStartTimerMessage( void )
{
	// Only send this message to the owner player	
	CSingleUserRecipientFilter user( ToFFPlayer( m_hOwner.Get() ) );
	user.MakeReliable( );

	// Start the message block
	UserMessageBegin( user, "DetpackStartTimer" );

	// Message to send
	WRITE_SHORT( m_iFuseTime );

	// End the message block
	MessageEnd( );
}

void CFFDetpack::SendStopTimerMessage( void )
{
	// Only send this message to the owner player	
	CSingleUserRecipientFilter user( ToFFPlayer( m_hOwner.Get() ) );
	user.MakeReliable();

	// Start the message block
	UserMessageBegin( user, "DetpackStopTimer" );

	// Message to send
	WRITE_SHORT( 1 );

	// End the message block
	MessageEnd();
}

int CFFDetpack::TakeEmp( void )
{
	DevMsg( "[Detpack] Emp gren attacked me!\n" );

	if( ( m_flDetonateTime - gpGlobals->curtime ) >= 5.0f )
	{
		DevMsg( "[Detpack] Calling think function early to start timer - hit by emp!\n" );
		m_flThinkTime = 0.001f;	// immediate
		SetNextThink( gpGlobals->curtime + m_flThinkTime );

		// Return something so an explosion goes off from the emp
		return 1;
	}

	return 0;
}

/**
@fn CFFDetpack *Create( const Vector& vecOrigin, const QAngle& vecAngles, edict_t *pentOwner )
@brief Creates a new CFFDetpack at a particular location
@param vecOrigin - origin of the object to be created
@param vecAngles - view angles of the object to be created
@param pentOwner - edict_t of the owner creating the object (usually keep NULL)
@return a _new_ CFFDetpack
*/ 
CFFDetpack *CFFDetpack::Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner )
{
	// Create the object
	CFFDetpack *pObject = ( CFFDetpack * )CBaseEntity::Create( "FF_Detpack", vecOrigin, vecAngles, NULL );

	// Set our faux owner - see CFFBuildable::Create for the reason why
	pObject->m_hOwner.GetForModify() = pentOwner;

	pObject->VPhysicsInitNormal( SOLID_VPHYSICS, pObject->GetSolidFlags(), true );

	// Spawn the object
	pObject->Spawn( );

	return pObject;
}
