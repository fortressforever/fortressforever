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
//
//	06/30/2006, Mulchman:
//		This thing has been through tons of changes and additions.
//		The latest thing is the doorblockers
//
//	05/10/2006, Mulchman:
//		Messing w/ the explode function and dealing better damage

#include "cbase.h"
#include "ff_buildableobjects_shared.h"
#include "explode.h"
//#include "gib.h"
#include "ff_player.h"
//#include "EntityFlame.h"
#include "beam_flags.h"
#include "ff_gamerules.h"
#include "world.h"
#include "ff_luacontext.h"
#include "ff_scriptman.h"

#ifdef _DEBUG
#include "Color.h"
#endif

#include "ff_utils.h"

#include "omnibot_interface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern short	g_sModelIndexFireball;

//=============================================================================
//
//	class CFFBuildableFlickerer
//
//=============================================================================
LINK_ENTITY_TO_CLASS( ff_buildable_flickerer, CFFBuildableFlickerer );
PRECACHE_REGISTER( ff_buildable_flickerer );

BEGIN_DATADESC( CFFBuildableFlickerer )
	DEFINE_THINKFUNC( OnObjectThink ),
END_DATADESC()

static ConVar flicker_time( "ffdev_flicker_time", "0.1", FCVAR_NONE );

//-----------------------------------------------------------------------------
// Purpose: Spawn a flickerer
//-----------------------------------------------------------------------------
void CFFBuildableFlickerer::Spawn( void )
{
	VPROF_BUDGET( "CFFBuildableFlickerer::Spawn", VPROF_BUDGETGROUP_FF_BUILDABLE );

	m_flFlicker = gpGlobals->curtime;

	SetThink( &CFFBuildableFlickerer::OnObjectThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose: See if it's time to un-flicker
//-----------------------------------------------------------------------------
void CFFBuildableFlickerer::OnObjectThink( void )
{
	VPROF_BUDGET( "CFFBuildableFlickerer::OnObjectThink", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// If a certain time period has gone by
	// since we last flickered we need to unflicker

	if( m_pBuildable )
	{
		// See if it's time to un-flicker
		if( ( ( m_flFlicker + flicker_time.GetFloat() ) < gpGlobals->curtime ) /*&& ( m_pBuildable->GetRenderMode() != kRenderNormal )*/ )
		{
			//m_pBuildable->SetRenderMode( kRenderNormal );
			m_pBuildable->SetBodygroup( 1, 0 );
		}

		// Think again soon!
		SetThink( &CFFBuildableFlickerer::OnObjectThink );
		SetNextThink( gpGlobals->curtime + 0.1f );
	}
	else
	{
		UTIL_Remove( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Flicker a buildable to indicate it's taking damage
//-----------------------------------------------------------------------------
void CFFBuildableFlickerer::Flicker( void )
{
	VPROF_BUDGET( "CFFBuildableFlickerer::Flicker", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// When flicker is called the buildable is taking damage

	if( m_pBuildable )
	{
		// Put us in a flickered "state"
		//if( m_pBuildable->GetRenderMode() == kRenderNormal )
		//{
			//m_pBuildable->SetRenderMode( kRenderTransAlpha );
			//m_pBuildable->SetRenderColorA( ( byte )110 );
			m_pBuildable->SetBodygroup( 1, 1 );
		//}

		// Note the time we flickered
		m_flFlicker = gpGlobals->curtime;
	}
	else
		UTIL_Remove( this );
}

//=============================================================================
//
//	class CFFBuildableObject
//	
//=============================================================================

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
	m_iExplosionFireballScale = 1.1f;

	// Default think time
	m_flThinkTime = 0.2f;

	// Duration between sending hints for sentry/disp damage
	m_flOnTakeDamageHintTime = 0.0f;

	// Defaults for derived classes
	m_iShockwaveExplosionTexture = -1;
	m_bShockWave = false;
	
	m_bBuilt = false;
	m_bTakesDamage = true;
	m_bHasSounds = false;
	m_bTranslucent = true; // by default
	m_bUsePhysics = false;

	// Set to null
	m_pFlickerer = NULL;
}

/**
@fn ~CFFBuildableObject
@brief Deconstructor
@return N/A
*/
CFFBuildableObject::~CFFBuildableObject( void )
{
	// Remove the flickerer
	if( m_pFlickerer )
	{
		m_pFlickerer->SetBuildable( NULL );
		m_pFlickerer = NULL;
	}
}

/**
@fn void Spawn( )
@brief Do some generic stuff at spawn time (play sounds)
@return void
*/
void CFFBuildableObject::Spawn( void )
{
	VPROF_BUDGET( "CFFBuildableObject::Spawn", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Set a team
	if( GetOwnerPlayer() )
		ChangeTeam( GetOwnerPlayer()->GetTeamNumber() );

	if( m_bUsePhysics )
	{
		SetSolid( SOLID_VPHYSICS );
		SetMoveType( MOVETYPE_VPHYSICS );
	}
	else
	{
		// So that doors collide with it
		SetSolid( SOLID_VPHYSICS );		
		AddSolidFlags( FSOLID_FORCE_WORLD_ALIGNED );
		SetMoveType( MOVETYPE_FLY );

		VPhysicsInitStatic();
	}

	SetCollisionGroup( COLLISION_GROUP_PLAYER );
		
	// Make sure it has a model
	Assert( m_ppszModels[ 0 ] != NULL );

	// Give it a model
	SetModel( m_ppszModels[ 0 ] );

	SetBlocksLOS( false );
	m_takedamage = DAMAGE_EVENTS_ONLY;

	// Play the build sound (if there is one)
	if( m_bHasSounds )
	{
		// Detpack sound is local only
		if( Classify() == CLASS_DETPACK )
		{
			if( GetOwnerPlayer() )
			{
				CSingleUserRecipientFilter sndFilter( GetOwnerPlayer() );
				EmitSound( sndFilter, entindex(), m_ppszSounds[ 0 ] );
			}			
		}
		else
		{
			CPASAttenuationFilter sndFilter( this );
			//sndFilter.AddRecipientsByPAS( GetAbsOrigin() );
			EmitSound( sndFilter, entindex(), m_ppszSounds[ 0 ] );
		}		
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
@fn void GoLive()
@brief Object is built and ready to do it's thing
@return void
*/
void CFFBuildableObject::GoLive( void )
{
	VPROF_BUDGET( "CFFBuildableObject::GoLive", VPROF_BUDGETGROUP_FF_BUILDABLE );

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

	/*
	// React to physics!
	if( m_bUsePhysics )
	{
		IPhysicsObject *pPhysics = VPhysicsGetObject();
		if( pPhysics )
		{
			pPhysics->Wake();
			pPhysics->EnableCollisions( true );
			pPhysics->EnableMotion( true );
			pPhysics->EnableGravity( true );
			pPhysics->EnableDrag( true );			
		}
	}
	//*/

	//*
	IPhysicsObject *pPhysics = VPhysicsGetObject();
	if( pPhysics )
	{
		pPhysics->Wake();
		pPhysics->EnableCollisions( true );
		pPhysics->EnableMotion( m_bUsePhysics );
		pPhysics->EnableGravity( m_bUsePhysics );
		pPhysics->EnableDrag( m_bUsePhysics );

		if( Classify() == CLASS_DETPACK )
			pPhysics->SetMass( 500.0f );
	}
	//*/
}

/**
@fn void Precache( )
@brief Precache's the model
@return void
*/
void CFFBuildableObject::Precache( void )
{
	VPROF_BUDGET( "CFFBuildableObject::Precache", VPROF_BUDGETGROUP_FF_BUILDABLE );

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
	BaseClass::Precache();	
}

/**
@fn void Detonate( )
@brief The user wants to blow up their own object
@return void
*/
void CFFBuildableObject::Detonate( void )
{
	VPROF_BUDGET( "CFFBuildableObject::Detonate", VPROF_BUDGETGROUP_FF_BUILDABLE );

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
	VPROF_BUDGET( "CFFBuildableObject::RemoveQuietly", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// MUST DO THIS or CreateExplosion crashes HL2
	m_takedamage = DAMAGE_NO;

	// Remove bounding box
	SetSolid( SOLID_NONE );

	// Stop playing the build sound
	if( m_bHasSounds )
		StopSound( m_ppszSounds[ 0 ] );

	// Remove the flickerer
	if( m_pFlickerer )
	{
		m_pFlickerer->SetBuildable( NULL );
		m_pFlickerer = NULL;
	}

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
	VPROF_BUDGET( "CFFBuildableObject::OnObjectThink", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Check for "malfunctions"
	if( HasMalfunctioned() )
	{
		CFFPlayer *pOwner = GetOwnerPlayer();
		if( pOwner )
		{
			switch( Classify() )
			{
				case CLASS_DISPENSER:
					ClientPrint( pOwner, HUD_PRINTCENTER, "#FF_DISPENSER_MALFUNCTIONED" );
				break;

				case CLASS_SENTRYGUN:
					ClientPrint( pOwner , HUD_PRINTCENTER, "#FF_SENTRYGUN_MALFUNCTIONED" );
				break;
			}
		}

		Detonate();
	}
}

//-----------------------------------------------------------------------------
// Purpose: See if we've malfunctioned
//-----------------------------------------------------------------------------
bool CFFBuildableObject::HasMalfunctioned( void ) const
{
	VPROF_BUDGET( "CFFBuildableObject::HasMalfunctioned", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// This test can be flakey hence switching to distance and giving
	// it a little fluff. If part of the bbox is inside something the
	// origin will jump around (in really really small values though
	// but enough so that m_vecGroundOrigin != GetAbsOrigin()
	if( m_bBuilt && ( m_vecGroundOrigin.DistTo( GetAbsOrigin() ) > 3.0f ) )
		return true;

	return false;
}

/**
@fn void Event_Killed( const CTakeDamageInfo& info )
@brief Called automatically when an object dies
@param info - CTakeDamageInfo structure
@return void
*/
void CFFBuildableObject::Event_Killed( const CTakeDamageInfo& info )
{
	VPROF_BUDGET( "CFFBuildableObject::Event_Killed", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Remove the flickerer
	if( m_pFlickerer )
	{
		m_pFlickerer->SetBuildable( NULL );
		m_pFlickerer = NULL;
	}

	// TODO: Run through lua "buildable_onkilled"

	// Can't kill detpacks
	if( Classify() != CLASS_DETPACK )
	{
		// Send to game rules to then fire event and send out hud death notice
		FFGameRules()->BuildableKilled( this, info );
	}

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
	pObject->Spawn();

	return pObject;
}

//
// VPhysicsTakeDamage
//		So weapons like the railgun won't effect building
//
int CFFBuildableObject::VPhysicsTakeDamage( const CTakeDamageInfo &info )
{
	VPROF_BUDGET( "CFFBuildableObject::VPhysicsTakeDamage", VPROF_BUDGETGROUP_FF_BUILDABLE );

	return 0;
}

/**
@fn void Explode( )
@brief For when the object blows up
@return void
*/
void CFFBuildableObject::Explode( void )
{
	VPROF_BUDGET( "CFFBuildableObject::Explode", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// MUST DO THIS or CreateExplosion crashes HL2
	m_takedamage = DAMAGE_NO;

	// Remove bounding box (other models follow this pattern...)
	SetSolid( SOLID_NONE );

	// Do the explosion
	DoExplosion();

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
	VPROF_BUDGET( "CFFBuildableObject::SpawnGib", VPROF_BUDGETGROUP_FF_BUILDABLE );

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
	VPROF_BUDGET( "CFFBuildableObject::DoExplosion", VPROF_BUDGETGROUP_FF_BUILDABLE );

	//CFFPlayer *pOwner = static_cast< CFFPlayer * >( m_hOwner.Get() );

	// Explosion!
	Vector vecAbsOrigin = GetAbsOrigin() + Vector( 0, 0, 32.0f ); // Bring off the ground a little 
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
		// m_ppszSounds[1] is the explosion sound

		CPASAttenuationFilter sndFilter( this );
		EmitSound( sndFilter, entindex(), m_ppszSounds[ 1 ] );
	}
	else
		Warning( "CFFBuildableObject::DoExplosion - ERROR - NO EXPLOSION SOUND (might want to add one)!\n" );

	// Mirv: Moved explosion damage logic into the derived classes
	DoExplosionDamage();
}

int CFFBuildableObject::OnTakeDamage( const CTakeDamageInfo &info )
{
	VPROF_BUDGET( "CFFBuildableObject::OnTakeDamage", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Bug #0000333: Buildable Behavior (non build slot) while building
	// If we're not live yet don't take damage
	if( !m_bBuilt )
		return 0;

	// Sentry gun seems to take about 110% of damage, going to assume its the same
	// for all others for now -mirv
	CTakeDamageInfo adjustedDamage = info;
	adjustedDamage.SetDamage( adjustedDamage.GetDamage() * 1.1f );

	// Sorry trepids, not putting this one check in LUA
	// Bug #0000333: Buildable Behavior (non build slot) while building
	if(( adjustedDamage.GetAttacker() == m_hOwner.Get() ) && ( friendlyfire.GetInt() == 0 ))
		return 0;

	// Run through LUA!
	CFFLuaSC hContext;
	hContext.Push( this );
	hContext.Push( &adjustedDamage );
	_scriptman.RunPredicates_LUA( NULL, &hContext, "buildable_ondamage" );

	// Bug #0000333: Buildable Behavior (non build slot) while building
	// Depending on the teamplay value, take damage
	if( !FFGameRules()->FPlayerCanTakeDamage( ToFFPlayer( m_hOwner.Get() ), adjustedDamage.GetAttacker() ) )
		return 0;

	// DrEvil: The following is fucking wrong, don't add it back.
	// It's not as simple as checking player relationship. FPlayerCanTakeDamage compensates for sabotage and shit.
	/*if( ( FFGameRules()->PlayerRelationship( ToFFPlayer( m_hOwner.Get() ), adjustedDamage.GetAttacker() ) == GR_TEAMMATE ) && ( friendlyfire.GetInt() == 0 ) )
		return 0;*/

	// If we haven't taken any damage, no need to flicker or report to bots
	if( adjustedDamage.GetDamage() <= 0 )
		return 0;

	// Lets flicker since we're taking damage
	if( m_pFlickerer )
		m_pFlickerer->Flicker();
	
	// Just extending this to send events to the bots.
	CFFPlayer *pOwner = static_cast< CFFPlayer * >( m_hOwner.Get() );
	if( pOwner )
	{
		Omnibot::Notify_BuildableDamaged( pOwner, Classify(), this );
		SendStatsToBot();
	}

	// Jiggles: Added hint event for buildables taking damage
	//				There's a 10-second delay to avoid spamming the hint
	if( pOwner && ( gpGlobals->curtime > m_flOnTakeDamageHintTime ) )
	{
		switch( Classify() )
		{
			case CLASS_DISPENSER: 
				FF_SendHint( pOwner, ENGY_DISPDAMAGED, 3, PRIORITY_NORMAL, "#FF_HINT_ENGY_DISPDAMAGED" ); 
				m_flOnTakeDamageHintTime = gpGlobals->curtime + 10.0f; 
				break;
			case CLASS_SENTRYGUN: 
				FF_SendHint( pOwner, ENGY_SGDAMAGED, 3, PRIORITY_NORMAL, "#FF_HINT_ENGY_SGDAMAGED" ); 
				m_flOnTakeDamageHintTime = gpGlobals->curtime + 10.0f;
				break;
		}
	}
	// End hint code

	// Bug #0000772: Buildable hud information doesn't update... good
	// This will force an update of this variable for the client	
	NetworkStateChanged( ( int * )&m_iHealth );

	return CBaseEntity::OnTakeDamage( adjustedDamage );
}
