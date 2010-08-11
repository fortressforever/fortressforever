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
#include "ff_scriptman.h"
#include "ff_luacontext.h"
#include "beam_flags.h"
#include "ff_gamerules.h"

#ifdef _DEBUG
#include "Color.h"
#endif
#include "ff_utils.h"

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

//static ConVar detpack_radius( "ffdev_detpack_radius", "700" );
//static ConVar detpack_falloff( "ffdev_detpack_falloff", "1" );
#define DETPACK_FALLOFF 1.0f


extern const char *g_pszFFDetpackModels[];
extern const char *g_pszFFDetpackGibModels[];
extern const char *g_pszFFDetpackSounds[];

extern const char *g_pszFFGenGibModels[];

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
	m_flExplosionRadius = 700.0f;
	m_iExplosionRadius = 700;
	m_flExplosionForce = 3000.0f;
	m_flExplosionDamage = 1270.f;
	m_flExplosionDuration = 2.0f;
	m_iExplosionFireballScale = 2.0f;

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
	VPROF_BUDGET( "CFFDetpack::Spawn", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Yeah, you can guess what this does!
	Precache();
	
	m_iHealth = 1;

	// Call baseclass spawn stuff
	CFFBuildableObject::Spawn();
}

/**
@fn void GoLive( )
@brief Object is built and ready to do it's thing
@return void
*/
void CFFDetpack::GoLive( void )
{
	VPROF_BUDGET( "CFFDetpack::GoLive", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Call base class
	CFFBuildableObject::GoLive();

	// Object is now built
	m_bBuilt = true;

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

	// Take away what it cost to build
	CFFPlayer *pOwner = GetOwnerPlayer();
	if( pOwner )
		pOwner->RemoveAmmo( 1, AMMO_DETPACK );
	
	// tell the client when we're supposed to blow up
	CSingleUserRecipientFilter user(pOwner);
	user.MakeReliable();

	UserMessageBegin(user, "DetpackMsg");
		WRITE_FLOAT(m_flDetonateTime);
	MessageEnd();

	//DevMsg( "[Detpack] Next think in: %f seconds\n", flCurTime + m_flThinkTime );
}

/**
@fn void OnTouch( )
@brief For when an entity touches our object
@param pOther - pointer to the entity that touched our object
@return void
*/
void CFFDetpack::OnObjectTouch( CBaseEntity *pOther )
{
	VPROF_BUDGET( "CFFDetpack::OnObjectTouch", VPROF_BUDGETGROUP_FF_BUILDABLE );

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

	// Skip if on the same team
	//if( !g_pGameRules->FCanTakeDamage( ( ( CFFPlayer * )m_hOwner.Get() ), pPlayer ) )
	if( FFGameRules()->IsTeam1AlliedToTeam2( pPlayer->GetTeamNumber(), ( ( CFFPlayer * )m_hOwner.Get() )->GetTeamNumber() ) == GR_TEAMMATE )
		return;

	// Play defuse sound
	CPASAttenuationFilter sndFilter( this );
	EmitSound( sndFilter, entindex(), "Detpack.Defuse" );
	
	// AfterShock - Scoring System: 100 points for defusing detpack
	pPlayer->AddFortPoints(100,"#FF_FORTPOINTS_DEFUSEDETPACK");

	// Send a message to the owner telling them what happened
	ClientPrint( ( CFFPlayer * )m_hOwner.Get(), HUD_PRINTCENTER, "#FF_DETPACK_DEFUSED" );

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
	VPROF_BUDGET( "CFFDetpack::OnObjectThink", VPROF_BUDGETGROUP_FF_BUILDABLE );

	CheckForOwner();

	// First time we come here it will be 5 seconds before we need 
	// to blow up
	if( !m_bFiveSeconds )
	{
		// Play the 5 second to go sound (whine up) whatever.
		EmitSound( m_ppszSounds[ 2 ] );		

		m_bFiveSeconds = true;

		SetNextThink( gpGlobals->curtime + 5.0f );
	}
	else
	{
		// Second time calling the think func, so time to blow up!
		Detonate();
	}
}

int CFFDetpack::TakeEmp( void )
{
	VPROF_BUDGET( "CFFDetpack::TakeEmp", VPROF_BUDGETGROUP_FF_BUILDABLE );

	if( ( m_flDetonateTime - gpGlobals->curtime ) >= 5.0f )
	{
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


//-----------------------------------------------------------------------------
// Purpose: Overridden just to fire the appropriate event.
//-----------------------------------------------------------------------------
void CFFDetpack::Detonate()
{
	VPROF_BUDGET( "CFFDetpack::Detonate", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Fire an event.
	IGameEvent *pEvent = gameeventmanager->CreateEvent("detpack_detonated");						
	if(pEvent)
	{
		CFFPlayer *pOwner = static_cast<CFFPlayer*>(m_hOwner.Get());
		if (pOwner)
		{
			pEvent->SetInt("userid", pOwner->GetUserID());
			gameeventmanager->FireEvent(pEvent, true);
		}
	}

	CFFBuildableObject::Detonate();
}

//-----------------------------------------------------------------------------
// Purpose: Carry out the radius damage for this buildable
//			Mulchman's custom detpack radius damage has been moved into here
//-----------------------------------------------------------------------------
void CFFDetpack::DoExplosionDamage( void )
{
	VPROF_BUDGET( "CFFDetpack::DoExplosionDamage", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Cause damage and some effects to things around our origin
	// Raise up a little, don't want to be right on the ground
	Vector vecOrigin = GetAbsOrigin() + Vector( 0, 0, 16 );

	// Better damage given out
	CBaseEntity *pEntity = NULL;
	for( CEntitySphereQuery sphere( GetAbsOrigin(), m_flExplosionRadius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if( !pEntity )
			continue;

		// Bail if the object doesn't take damage
		if( pEntity->m_takedamage == DAMAGE_NO )
			continue;

		// The player (or buildable's owner) that is inside our sphere
		CFFPlayer *pPlayer = NULL;
		bool bSpecial = false;
		Vector vecTarget;

		if( pEntity->IsPlayer() )
		{
			pPlayer = ToFFPlayer( pEntity );
			vecTarget = pPlayer->GetLegacyAbsOrigin();
		}
		else if( pEntity->Classify() == CLASS_DISPENSER )
		{
			CFFDispenser *pDispenser = static_cast< CFFDispenser * >( pEntity );
			pPlayer = ToFFPlayer( pDispenser->m_hOwner.Get() );
			vecTarget = pDispenser->GetAbsOrigin() + Vector( 0, 0, 16 );
		}
		else if( pEntity->Classify() == CLASS_SENTRYGUN )
		{
			CFFSentryGun *pSentryGun = static_cast< CFFSentryGun * >( pEntity );
			pPlayer = ToFFPlayer( pSentryGun->m_hOwner.Get() );
			vecTarget = pSentryGun->GetAbsOrigin() + Vector( 0, 0, 16 );
		}
		else if( pEntity->Classify() == CLASS_MANCANNON )
		{
			CFFManCannon *pManCannon = static_cast<CFFManCannon *>( pEntity );
			pPlayer = ToFFPlayer( pManCannon->m_hOwner.Get() );
			vecTarget = pManCannon->GetAbsOrigin() + Vector( 0, 0, 16 );
		}
		else
		{
			// Not a player, dispenser, or SG. Still need to give out damage
			// in case the object needs to respond to damage (buttons?)
			bSpecial = true;
			vecTarget = pEntity->GetAbsOrigin();
		}

		// Only want to do the next couple of checks on
		// players, dispenser, and sentryguns
		if( !bSpecial )
		{
			if( !pPlayer->IsAlive() || pPlayer->IsObserver() )
				continue;

			//			Bug: #0000666: detpack doesn't push teammates
			//			Effect everybody...
			// If the player can't take damage from us (our owner), bail
			//			if( !FFGameRules()->FCanTakeDamage( pPlayer, pOwner ) )
			//				continue;

#ifdef _DEBUG
			/* VOOGRU: I debug with dedicated server, and I don't want srcds to throw 
			util.cpp (552) : Assertion Failed: !"UTIL_GetListenServerHost" */
			if( !engine->IsDedicatedServer() )
			{
				Color cColor;
				SetColorByTeam( pPlayer->GetTeamNumber(), cColor );

				NDebugOverlay::Line( vecOrigin, vecTarget, cColor.r(), cColor.g(), cColor.b(), false, 10.0f );
			}
#endif
		}

		bool bDoDamage = true, bBail = false;

		// Need to trace until we hit the entity or a door/wall as someone/something
		// can block the trace from getting to us but there is still a door/wall in the
		// way that should have stopped the blast in the first place.

		Vector vecBeg = vecOrigin;
		CBaseEntity *pIgnore = this;
		int iCount = 0;

		while( bDoDamage && ( iCount < 256 ) && !bBail )
		{
			// Now, Trace! 
			trace_t tr;
			UTIL_TraceLine( vecBeg, vecTarget, MASK_SOLID, pIgnore, COLLISION_GROUP_NONE, &tr );

			// If we hit something...
			if( tr.DidHit() )
			{
#ifdef _DEBUG
				if( pEntity->IsPlayer() )
					Warning( "[%s Explosion] Iterating on: %s (%s)", GetClassname(), pEntity->GetClassname(), ToFFPlayer( pEntity )->GetPlayerName() );
				else
					Warning( "[%s Explosion] Iterating on: %s", GetClassname(), pEntity->GetClassname() );

				if( tr.m_pEnt->IsPlayer() )
					Warning( ", TraceLine hit something: %s (%s)\n", tr.m_pEnt->GetClassname(), ToFFPlayer( tr.m_pEnt )->GetPlayerName() );
				else
					Warning( ", TraceLine hit something: %s\n", tr.m_pEnt->GetClassname() );
#endif

				// Don't do damage if the trace hit:
				/*
				if( tr.DidHitWorld() ||
					FClassnameIs( tr.m_pEnt, "func_door" ) ||
					FClassnameIs( tr.m_pEnt, "worldspawn" ) ||
					FClassnameIs( tr.m_pEnt, "func_door_rotating" ) ||
					FClassnameIs( tr.m_pEnt, "prop_door_rotating" ) )
					*/
				if( FF_TraceHitWorld( &tr ) )
					bDoDamage = false;	// Get out of loop

				// Traced until we hit ourselves
				if( tr.m_pEnt == pEntity )
					bBail = true;
			}

			// Haven't hit a wall or pEntity so keep tracing
			// Update start & ignore entity
			vecBeg = tr.endpos;
			pIgnore = tr.m_pEnt;

			iCount++; // In case we get stuck tracing this will bail us out after so many traces

			if( vecBeg == vecTarget )
				bBail = true;
		}

#ifdef _DEBUG
		Warning( "[Buildable Object] bDoDamage: %s, iCount: %i, bBail: %s\n", bDoDamage ? "true" : "false", iCount, bBail ? "true" : "false" );
#endif

		// Basically, if we don't hit a couple of objects deal out [absolute] damage		
		// Do damage
		if( bDoDamage )
		{
			Vector vecDir = pEntity->GetAbsOrigin() - vecOrigin;
			VectorNormalize( vecDir );

			// Linear falloff * detpack_falloff%
			float flDamage = ( ( m_flExplosionRadius - (float)vecOrigin.DistTo( GetAbsOrigin() ) ) / m_flExplosionRadius ) * m_flExplosionDamage * DETPACK_FALLOFF;
			pEntity->TakeDamage( CTakeDamageInfo( this, m_hOwner.Get(), vecDir * m_flExplosionForce, pEntity->GetAbsOrigin(), flDamage, DMG_SHOCK | DMG_BLAST ) );
		}
	}

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
			filter2, 0, GetAbsOrigin(),		//origin
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

	// Hit detpack triggers (and do this after potentially gibbing players so there's less to trace through [possibly])
	if( Classify() == CLASS_DETPACK )
	{
		CBaseEntity *pEntity = NULL;
		for( CEntitySphereQuery sphere( GetAbsOrigin(), 1500.0f ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			if( !pEntity )
				continue;

			// See if the world is not blocking this object from us
			bool bDoDamage = true, bBail = false;
			Vector vecBeg = vecOrigin, vecTarget = pEntity->GetAbsOrigin();
			CBaseEntity *pIgnore = this;
			int iCount = 0;			

			while( bDoDamage && ( iCount < 256 ) && !bBail )
			{
				// Now, Trace! 
				trace_t tr;
				UTIL_TraceLine( vecBeg, vecTarget, MASK_SOLID, pIgnore, COLLISION_GROUP_NONE, &tr );

				// If we hit something...
				if( tr.DidHit() )
				{
					// Skip object if we hit one of these while tracing to it
					/*
					if( tr.DidHitWorld() ||
						FClassnameIs( tr.m_pEnt, "func_door" ) ||
						FClassnameIs( tr.m_pEnt, "worldspawn" ) ||
						FClassnameIs( tr.m_pEnt, "func_door_rotating" ) ||
						FClassnameIs( tr.m_pEnt, "prop_door_rotating" ) )
						*/
					if( FF_TraceHitWorld( &tr ) )
						bDoDamage = false;	// Get out of loop

					// Traced until we hit ourselves
					if( tr.m_pEnt == pEntity )
						bBail = true;
				}

				// Haven't hit a wall or pEntity so keep tracing
				// Update start & ignore entity
				vecBeg = tr.endpos;
				pIgnore = tr.m_pEnt;

				iCount++; // In case we get stuck tracing this will bail us out after so many traces

				if( vecBeg == vecTarget )
					bBail = true;
			}

			// If we traced to the object w/o hitting world or something else
			if( bDoDamage )
			{
				// If it's affected by detpack explosions do something
				CFFLuaSC hOnExplode( 1, ( CFFDetpack * )this );
				_scriptman.RunPredicates_LUA( pEntity, &hOnExplode, "onexplode" );
			}
		}
	}
}