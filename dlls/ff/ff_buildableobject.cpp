// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildableobject.cpp
// @author Patrick O'Leary (Mulchman)
// @date 12/15/2005
// @brief BuildableObject class
//
// REVISIONS
// ---------
// 12/15/2005, Mulchman: 
//		First created
//
// 12/23-25/2005, Mulchman: 
//		A bunch of modifications (explosions, gibs, fire, building checking)
//
// 12/28/2004, Mulchman:
//		Bunch of mods - shares network values correctly. Officially a base 
//		class for other buildables
//
// 01/20/2004, Mulchman: 
//		Having no sounds (build/explode) won't cause problems
//
// 05/09/2005, Mulchman: 
//		Tons of additions - better checking of build area, lots of 
//		cleanup... basically an overhaul

#include "cbase.h"
#include "ff_buildableobject.h"
#include "explode.h"
#include "gib.h"
#include "ff_player.h"
#include "EntityFlame.h"
#include "beam_flags.h"
#include "basecombatcharacter.h"
#include "gamevars_shared.h"

#include "omnibot_interface.h"

extern short	g_sModelIndexFireball;

LINK_ENTITY_TO_CLASS( FF_BuildableObject_entity, CFFBuildableObject );
PRECACHE_REGISTER( FF_BuildableObject_entity );

IMPLEMENT_SERVERCLASS_ST( CFFBuildableObject, DT_FFBuildableObject )
	SendPropEHandle( SENDINFO( m_hOwner ) ),
	SendPropInt( SENDINFO( m_iHealth ) ),
	SendPropInt( SENDINFO( m_iMaxHealth ) ),
	SendPropInt( SENDINFO( m_bBuilt ) ),
END_SEND_TABLE( )

// Start of our data description for the class
/*
BEGIN_DATADESC( CFFBuildableObject )
	DEFINE_THINKFUNC( OnObjectThink ),
END_DATADESC( )
*/

const char *g_pszFFModels[ ] =
{
	NULL
};

const char *g_pszFFGibModels[ ] =
{
	NULL
};

const char *g_pszFFSounds[ ] =
{
	// BUILD SOUND HAS TO COME FIRST,
	// EXPLODE SOUND HAS TO COME SECOND,
	// Other sounds go here,
	// NULL if no sounds
	NULL
};

// Generic gib models used for every buildable object, yay
const char *g_pszFFGenGibModels[ ] =
{
	FF_BUILDALBE_GENERIC_GIB_MODEL_01,
	FF_BUILDALBE_GENERIC_GIB_MODEL_02,
	FF_BUILDALBE_GENERIC_GIB_MODEL_03,
	FF_BUILDALBE_GENERIC_GIB_MODEL_04,
	FF_BUILDALBE_GENERIC_GIB_MODEL_05,
	NULL
};

/**
@fn CFFBuildableObject
@brief Constructor
@return N/A
*/
CFFBuildableObject::CFFBuildableObject( void )
{
	// Point these to stubs (super class re-sets these)
	m_ppszModels = g_pszFFModels;
	m_ppszGibModels = g_pszFFGibModels;
	m_ppszSounds = g_pszFFSounds;

	// Default values
	m_iExplosionMagnitude = 50;
	m_flExplosionMagnitude = 50.0f;
	m_flExplosionRadius = 3.5f * m_flExplosionMagnitude;
	m_iExplosionRadius = ( int )m_flExplosionRadius;	
	m_flExplosionForce = 100.0f;
	// TODO: for now - change this later? remember to update in dispenser.cpp as well
	m_flExplosionDamage = m_flExplosionForce;
	m_flExplosionDuration = 0.5f;
	m_iExplosionFireballScale = random->RandomInt( 10, 15 );

	// Default think time
	m_flThinkTime = 0.2f;

	// Defaults for derived classes
	m_iShockwaveExplosionTexture = -1;
	m_bShockWave = false;
	
	m_bBuilt = false;
	m_bTakesDamage = true;
	m_bHasSounds = false;
	m_bTranslucent = true; // by default
	m_bUsePhysics = false;
}

/**
@fn ~CFFBuildableObject
@brief Deconstructor
@return N/A
*/
CFFBuildableObject::~CFFBuildableObject( void )
{	
}

/**
@fn void Spawn( )
@brief Do some generic stuff at spawn time (play sounds)
@return void
*/
void CFFBuildableObject::Spawn( void )
{
	// Set the team number to the owner team number.
	// Hope this is ok, bots use this for ally checks atm.
	CFFPlayer *pOwner = static_cast<CFFPlayer*>(m_hOwner.Get());
	if(pOwner)
	{
		ChangeTeam(pOwner->GetTeamNumber());
	}

	// Give it a bounding box (use the .phy file provided)
	//SetSolid( SOLID_VPHYSICS );
	SetSolid( SOLID_BBOX );

	SetBlocksLOS( false );

	// Make sure it has a model
	Assert( m_ppszModels[ 0 ] != NULL );

	// Give it a model
	SetModel( m_ppszModels[ 0 ] );

	m_takedamage = DAMAGE_EVENTS_ONLY;
	
	AddFlag( FL_OBJECT );

	// Play the build sound (if there is one)
	if( m_bHasSounds )
	{
		CPASAttenuationFilter sndFilter( this );
		EmitSound( sndFilter, entindex(), m_ppszSounds[ 0 ] );
	}

	// Start making it drop and/or flash (if applicable)
	if( m_bTranslucent )
	{
		SetRenderMode( kRenderTransAlpha );
		SetRenderColorA( ( byte )110 );
	}

	// Don't let the model react to physics just yet
	IPhysicsObject *pPhysics = VPhysicsGetObject();
	if( pPhysics )
	{
		pPhysics->EnableCollisions( false );
		pPhysics->EnableMotion( false );
		pPhysics->EnableGravity( false );
		pPhysics->EnableDrag( false );
	}

	m_bBuilt = false;	// |-- Mirv: Make sure we're in a state of not built
}

/**
@fn void GoLive( )
@brief Object is built and ready to do it's thing
@return void
*/
void CFFBuildableObject::GoLive( void )
{
	// Object is now built
	m_bBuilt = true;

	// Object is built and can take damage if it is supposed to
	if( m_bTakesDamage )
		m_takedamage = DAMAGE_YES;

	// Make opaque
	if( m_bTranslucent )
	{
		// Make sure the model is drawn normally now (if "flashing")
		SetRenderMode( kRenderNormal );
	}

	// React to physics!
	if( m_bUsePhysics )
	{
		IPhysicsObject *pPhysics = VPhysicsGetObject( );
		if( pPhysics )
		{
			pPhysics->Wake();
			pPhysics->EnableCollisions( true );
			pPhysics->EnableMotion( true );
			pPhysics->EnableGravity( true );
			pPhysics->EnableDrag( true );
		}
	}
}

/**
@fn void Precache( )
@brief Precache's the model
@return void
*/
void CFFBuildableObject::Precache( void )
{
	// Precache normal models
	int iCount = 0;
	while( m_ppszModels[ iCount ] != NULL )
	{
		PrecacheModel( m_ppszModels[ iCount ] );
		iCount++;
	}

	// Precache gib models
	iCount = 0;
	while( m_ppszGibModels[ iCount ] != NULL )
	{
		PrecacheModel( m_ppszGibModels[ iCount ] );
		iCount++;
	}

	// Precache the random always used gib models
	iCount = 0;
	while( g_pszFFGenGibModels[ iCount ] != NULL )
	{
		PrecacheModel( g_pszFFGenGibModels[ iCount ] );
		iCount++;
	}

	// See if we've got any sounds to precache
	if( m_ppszSounds[ 0 ] != NULL )
		if( m_ppszSounds[ 1 ] != NULL )
			m_bHasSounds = true;

	// Precache sound files
	if( m_bHasSounds )
	{
		iCount = 0;
		while( m_ppszSounds[ iCount ] != NULL )
		{		
			PrecacheScriptSound( m_ppszSounds[ iCount ] );
			iCount++;
		}
	}

	// Precache the shockwave
	if( m_bShockWave )
		m_iShockwaveExplosionTexture = PrecacheModel( "sprites/lgtning.vmt" );	

	// Call base class
	BaseClass::Precache( );	
}

/**
@fn void Detonate( )
@brief The user wants to blow up their own object
@return void
*/
void CFFBuildableObject::Detonate( void )
{
	// Do the explosion and radius damage
	Explode();
}

/**
@fn void RemoveQuietly( )
@brief The user died during build process
@return void
*/
void CFFBuildableObject::RemoveQuietly( void )
{
	// MUST DO THIS or CreateExplosion crashes HL2
	m_takedamage = DAMAGE_NO;

	// Remove bounding box
	SetSolid( SOLID_NONE );

	// Stop playing the build sound
	if( m_bHasSounds )
		StopSound( m_ppszSounds[ 0 ] );

	// Notify player to tell them they can build
	// again and remove current owner
	m_hOwner = NULL;

	// Remove entity from game
	UTIL_Remove( this );
}

/**
@fn void OnThink
@brief Think function (modifies our network health var for now)
@return void
*/
void CFFBuildableObject::OnObjectThink( void )
{
	if( m_bUsePhysics )
	{
		// See if we've stopped moving from physics stuff
		IPhysicsObject *pPhysics = VPhysicsGetObject();
		if( pPhysics )
		{
			Vector vecVelocity;
			AngularImpulse angVelocity;

			pPhysics->GetVelocity( &vecVelocity, &angVelocity );

			// If we stopped moving, don't react to physics!
			if( vecVelocity.IsZero() )
			{
				pPhysics->EnableCollisions( false );
				pPhysics->EnableMotion( false );
				pPhysics->EnableGravity( false );
				pPhysics->EnableDrag( false );

				m_bUsePhysics = false;
			}
		}
	}
}

/**
@fn void Event_Killed( const CTakeDamageInfo& info )
@brief Called automatically when an object dies
@param info - CTakeDamageInfo structure
@return void
*/
void CFFBuildableObject::Event_Killed( const CTakeDamageInfo& info )
{
	//////////////////////////////////////////////////////////////////////////
	const char *pEventName = 0;
	switch(Classify())
	{
	case CLASS_DISPENSER:
		pEventName = "dispenser_killed";
		break;
	case CLASS_SENTRYGUN:
		pEventName = "sentry_killed";
		break;
	}
	if(pEventName)
	{
		CFFPlayer *pOwner = static_cast<CFFPlayer*>(m_hOwner.Get());
		CBasePlayer *pAttacker = info.GetAttacker() ? info.GetAttacker()->MyCharacterPointer() : 0;
		IGameEvent *pEvent = gameeventmanager->CreateEvent( pEventName );
		Assert(pOwner);
		if(pEvent)
		{			
			pEvent->SetInt("ownerid", pOwner->GetUserID() );
			pEvent->SetInt("attackerid", pAttacker ? pAttacker->GetUserID() : 0 );
			gameeventmanager->FireEvent( pEvent );
		}
	}
	//////////////////////////////////////////////////////////////////////////

	// Do the explosion and radius damage last, because it clears the owner.
	Explode();
}

/**
@fn CFFBuildableObject *Create( const Vector& vecOrigin, const QAngle& vecAngles, edict_t *pentOwner )
@brief Creates a new CFFBuildableObject at a particular location
@param vecOrigin - origin of the object to be created
@param vecAngles - view angles of the object to be created
@param pentOwner - edict_t of the owner creating the object (usually keep NULL)
@return a _new_ CFFBuildableObject object
*/ 
CFFBuildableObject *CFFBuildableObject::Create( const Vector& vecOrigin, const QAngle& vecAngles, CBaseEntity *pentOwner )
{
	// Create the object
	CFFBuildableObject *pObject = ( CFFBuildableObject * )CBaseEntity::Create( "FF_BuildableObject_entity", vecOrigin, vecAngles, pentOwner );

	// Set the real owner. We're not telling CBaseEntity::Create that we have an owner so that
	// touch functions [and possibly other items] will work properly when activated by us
	pObject->m_hOwner = pentOwner;

	// Spawn the object
	pObject->Spawn( );

	return pObject;
}

//
// VPhysicsTakeDamage
//		So weapons like the railgun won't effect building
//
int CFFBuildableObject::VPhysicsTakeDamage( const CTakeDamageInfo &info )
{
	return 1;
}

/**
@fn void Explode( )
@brief For when the object blows up
@return void
*/
void CFFBuildableObject::Explode( void )
{
	// MUST DO THIS or CreateExplosion crashes HL2
	m_takedamage = DAMAGE_NO;

	// Remove bounding box (other models follow this pattern...)
	SetSolid( SOLID_NONE );

	// Do the explosion
	DoExplosion( );

	// Notify player to tell them they can build
	// again and remove current owner
	m_hOwner = NULL;

	// Remove entity from game 
	UTIL_Remove( this );
}

/**
@fn void SpawnGib( const char *szGibModel )
@brief Creates a gib and tosses it randomly
@param szGibModel - path to a model for the gib to toss
@return void
*/
void CFFBuildableObject::SpawnGib( const char *szGibModel, bool bFlame, bool bDieGroundTouch )
{
	/*
	// Create some gibs! MMMMM CHUNKY
	CGib *pChunk = CREATE_ENTITY( CGib, "gib" );
	pChunk->Spawn( szGibModel );
	pChunk->SetBloodColor( DONT_BLEED );

	QAngle vecSpawnAngles;
	vecSpawnAngles.Random( -90, 90 );
	pChunk->SetAbsOrigin( GetAbsOrigin( ) );
	pChunk->SetAbsAngles( vecSpawnAngles );

	pChunk->SetOwnerEntity( this );
	if( bDieGroundTouch )
		pChunk->m_lifeTime = 0.0f;
	else
		pChunk->m_lifeTime = random->RandomFloat( 1.0f, 3.0f );
	pChunk->SetCollisionGroup( COLLISION_GROUP_DEBRIS );

	IPhysicsObject *pPhysicsObject = pChunk->VPhysicsInitNormal( SOLID_VPHYSICS, pChunk->GetSolidFlags(), false );
	if( pPhysicsObject )
	{
		pPhysicsObject->EnableMotion( true );
		Vector vecVelocity;

		QAngle angles;
		angles.x = random->RandomFloat( -40, 0 );
		angles.y = random->RandomFloat( 0, 360 );
		angles.z = 0.0f;
		AngleVectors( angles, &vecVelocity );

		vecVelocity *= random->RandomFloat( 300, 600 );
		vecVelocity += GetAbsVelocity( );

		AngularImpulse angImpulse;
		angImpulse = RandomAngularImpulse( -180, 180 );

		pChunk->SetAbsVelocity( vecVelocity );
		pPhysicsObject->SetVelocity( &vecVelocity, &angImpulse );
	}

	if( bFlame )
	{
		// Add a flame to the gib
		CEntityFlame *pFlame = CEntityFlame::Create( pChunk, false );
		if( pFlame != NULL )
		{
			pFlame->SetLifetime( pChunk->m_lifeTime );
		}
	}

	// Make the next think function the die one (to get rid of the damn things)
	if( bDieGroundTouch )
		pChunk->SetThink( &CGib::SUB_FadeOut );
	else
		pChunk->SetThink( &CGib::DieThink );
		*/
}

/**
@fn void DoExplosion( )
@brief For when the object blows up (the actual explosion)
@return void
*/
void CFFBuildableObject::DoExplosion( void )
{
	CFFPlayer *pOwner = static_cast<CFFPlayer*>(m_hOwner.Get());

	// Explosion!
	Vector vecAbsOrigin = GetAbsOrigin();
	CPASFilter filter( vecAbsOrigin );	
	te->Explosion( filter,			// Filter
		0.0f,						// Delay
		&vecAbsOrigin,				// Origin (position)
		g_sModelIndexFireball,		// Model index
		m_iExplosionFireballScale,	// scale
		random->RandomInt( 8, 15 ),	// framerate
		TE_EXPLFLAG_NONE,			// flags
		m_iExplosionRadius,			// radius
		m_iExplosionMagnitude		// magnitude
	);

	// Play the explosion sound
	if( m_bHasSounds )
	{
		// m_ppszSounds[ 1 ] is the explosion sound
		CPASAttenuationFilter sndFilter( this );
		EmitSound( sndFilter, entindex(), m_ppszSounds[ 1 ] );	
	}
	else
		DevMsg( "CFFBuildableObject::DoExplosion - ERROR - NO EXPLOSION SOUND (might want to add one)!\n" );

	// Cause damage and some effects to things around our origin
	// Mirv: Changed pAttacker from this to pOwner, to fix
	// Bug #0000279: Detpack not causing damage.
	// Bug #0000247: Dispenser explosion does not hurt you
	RadiusDamage( CTakeDamageInfo( this, pOwner, m_flExplosionDamage, DMG_SHOCK | DMG_BLAST ), GetAbsOrigin(), m_flExplosionRadius, CLASS_NONE, NULL );

	// Shake the screen if you're close enough
	UTIL_ScreenShake( GetAbsOrigin(), 25.0f, 150.0f, m_flExplosionDuration, 5.0f * m_flExplosionRadius, SHAKE_START );

	// If we need a shockwave for this explosion
	if( m_bShockWave )
	{
		//Shock waves
		CBroadcastRecipientFilter filter2;
		te->BeamRingPoint( 
			filter2, 0, GetAbsOrigin(),	//origin
			1.0f,							//start radius
			m_flExplosionRadius * 2.0f,		//end radius
			m_iShockwaveExplosionTexture,	//texture
			0,								//halo index
			0,								//start frame
			2,								//framerate
			0.2f,							//life
			64,								//width
			0,								//spread
			0,								//amplitude
			255,							//r
			255,							//g
			255,							//b
			100,							//a
			0,								//speed
			FBEAM_FADEOUT
		);

		//Shock ring
		te->BeamRingPoint( 
			filter2, 0, GetAbsOrigin( ),	//origin
			1.0f,							//start radius
			m_flExplosionRadius * 2.0f,		//end radius
			m_iShockwaveExplosionTexture,	//texture
			0,								//halo index
			0,								//start frame
			2,								//framerate
			0.5f,							//life
			64,								//width
			0,								//spread
			0,								//amplitude
			255,							//r
			255,							//g
			255,							//b
			255,							//a
			0,								//speed
			FBEAM_FADEOUT
		);
	}

	/*
	// TODO: Get w/ sev about new gibs and shiz
	// Throw some gibs out
	int iCount = 0;	
	while( m_ppszGibModels[ iCount ] != NULL )
	{
		SpawnGib( m_ppszGibModels[ iCount ] );
		iCount++;
	}

	// Throw some generic gibs out
	for( int i = 0; i < 3; i++ )
	{
		iCount = 0;
		while( g_pszFFGenGibModels[ iCount ] != NULL )
		{
			SpawnGib( g_pszFFGenGibModels[ iCount ], false, true );
			iCount++;
		}
	}
	*/
}

int CFFBuildableObject::OnTakeDamage( const CTakeDamageInfo &info )
{
	// Bug #0000333: Buildable Behavior (non build slot) while building
	// If we're not live yet don't take damage
	if( !m_bBuilt )
		return 0;

	// Bug #0000333: Buildable Behavior (non build slot) while building
	// Depending on the teamplay value, take damage
	if( !g_pGameRules->FPlayerCanTakeDamage( ToFFPlayer( m_hOwner.Get() ), info.GetAttacker() ) )
	{
		DevMsg( "[Buildable] Teammate or ally is attacking me so don't take damage!\n" );
		return 0;
	}

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if(( info.GetAttacker() == m_hOwner.Get() ) && ( friendlyfire.GetInt() == 0 ))
	{
		DevMsg( "[Buildable] My owner is attacking me & friendly fire is off so don't take damage!\n" );
		return 0;
	}


	// Sentry gun seems to take about 110% of damage, going to assume its the same
	// for all others for now -mirv
	CTakeDamageInfo adjustedDamage = info;
	adjustedDamage.SetDamage(adjustedDamage.GetDamage() * 1.1f);
	
	// Just extending this to send events to the bots.
	CFFPlayer *pOwner = static_cast<CFFPlayer*>(m_hOwner.Get());
	if(pOwner && pOwner->IsBot())
	{
		int iMsg = 0;
		switch(Classify())
		{
		case CLASS_DISPENSER:
			iMsg = Omnibot::TF_MESSAGE_DISPENSER_DAMAGED;
			break;
		case CLASS_SENTRYGUN:
			iMsg = Omnibot::TF_MESSAGE_SENTRY_DAMAGED;
			break;
		}			

		if(iMsg != 0)
		{
			int iGameId = pOwner->entindex()-1;

			Omnibot::BotUserData bud(edict());
			Omnibot::omnibot_interface::Bot_Interface_SendEvent(
				iMsg,
				iGameId, 0, 0, &bud);
		}
		SendStatsToBot();
	}

	return BaseClass::OnTakeDamage(adjustedDamage);
}
