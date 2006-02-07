//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Dissolve entity to be attached to target entity. Serves two purposes:
//
//			1) An entity that can be placed by a level designer and triggered
//			   to ignite a target entity.
//
//			2) An entity that can be created at runtime to ignite a target entity.
//
//=============================================================================//

#include "cbase.h"
#include "EntityDissolve.h"
#include "baseanimating.h"
#include "physics_prop_ragdoll.h"
#include "ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static const char *s_pElectroThinkContext = "ElectroThinkContext";


//-----------------------------------------------------------------------------
// Lifetime 
//-----------------------------------------------------------------------------
#define DISSOLVE_FADE_IN_START_TIME			0.0f
#define DISSOLVE_FADE_IN_END_TIME			1.0f
#define DISSOLVE_FADE_OUT_MODEL_START_TIME	1.9f
#define DISSOLVE_FADE_OUT_MODEL_END_TIME	2.0f
#define DISSOLVE_FADE_OUT_START_TIME		2.0f
#define DISSOLVE_FADE_OUT_END_TIME			2.0f

//-----------------------------------------------------------------------------
// Model 
//-----------------------------------------------------------------------------
#define DISSOLVE_SPRITE_NAME	"sprites/blueglow1.vmt"	

//-----------------------------------------------------------------------------
// Save/load 
//-----------------------------------------------------------------------------
BEGIN_DATADESC( CEntityDissolve )

	DEFINE_FIELD( m_flStartTime, FIELD_TIME ),
	DEFINE_FIELD( m_flFadeInStart, FIELD_FLOAT ),
	DEFINE_FIELD( m_flFadeInLength, FIELD_FLOAT ),
	DEFINE_FIELD( m_flFadeOutModelStart, FIELD_FLOAT ),
	DEFINE_FIELD( m_flFadeOutModelLength, FIELD_FLOAT ),
	DEFINE_FIELD( m_flFadeOutStart, FIELD_FLOAT ),
	DEFINE_FIELD( m_flFadeOutLength, FIELD_FLOAT ),
	DEFINE_FIELD( m_nDissolveType, FIELD_INTEGER ),

	DEFINE_FUNCTION( DissolveThink ),
	DEFINE_FUNCTION( ElectrocuteThink ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Dissolve", InputDissolve ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Networking
//-----------------------------------------------------------------------------
IMPLEMENT_SERVERCLASS_ST( CEntityDissolve, DT_EntityDissolve )
	SendPropTime( SENDINFO( m_flStartTime ) ),
	SendPropFloat( SENDINFO( m_flFadeInStart ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_flFadeInLength ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_flFadeOutModelStart ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_flFadeOutModelLength ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_flFadeOutStart ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_flFadeOutLength ), 0, SPROP_NOSCALE ),
	SendPropInt( SENDINFO( m_nDissolveType ), ENTITY_DISSOLVE_BITS, SPROP_UNSIGNED ),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( env_entity_dissolver, CEntityDissolve );


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEntityDissolve::CEntityDissolve( void )
{
	m_flStartTime	= 0.0f;
}

CEntityDissolve::~CEntityDissolve( void )
{
}


//-----------------------------------------------------------------------------
// Precache
//-----------------------------------------------------------------------------
void CEntityDissolve::Precache()
{
	if ( NULL_STRING == GetModelName() )
	{
		PrecacheModel( DISSOLVE_SPRITE_NAME );
	}
	else
	{
		PrecacheModel( STRING( GetModelName() ) );
	}
}


//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CEntityDissolve::Spawn()
{
	BaseClass::Spawn();
	Precache();
	UTIL_SetModel( this, STRING( GetModelName() ) );

	if ( (m_nDissolveType == ENTITY_DISSOLVE_ELECTRICAL) || (m_nDissolveType == ENTITY_DISSOLVE_ELECTRICAL_LIGHT) )
	{
		if ( dynamic_cast< CRagdollProp* >( GetMoveParent() ) )
		{
			SetContextThink( &CEntityDissolve::ElectrocuteThink, gpGlobals->curtime + 0.01f, s_pElectroThinkContext );
		}
	}
	
	// Setup our times
	m_flFadeInStart = DISSOLVE_FADE_IN_START_TIME;
	m_flFadeInLength = DISSOLVE_FADE_IN_END_TIME - DISSOLVE_FADE_IN_START_TIME;
	
	m_flFadeOutModelStart = DISSOLVE_FADE_OUT_MODEL_START_TIME;
	m_flFadeOutModelLength = DISSOLVE_FADE_OUT_MODEL_END_TIME - DISSOLVE_FADE_OUT_MODEL_START_TIME;
	
	m_flFadeOutStart = DISSOLVE_FADE_OUT_START_TIME;
	m_flFadeOutLength = DISSOLVE_FADE_OUT_END_TIME - DISSOLVE_FADE_OUT_START_TIME;

	m_nRenderMode = kRenderTransColor;
	SetRenderColor( 255, 255, 255, 255 );
	m_nRenderFX = kRenderFxNone;

	// Turn them into debris
	CBaseAnimating *pTarget = ( GetMoveParent() ) ? GetMoveParent()->GetBaseAnimating() : NULL;
	if ( pTarget )
	{
		pTarget->SetCollisionGroup( COLLISION_GROUP_DISSOLVING );
	}
	
	SetThink( &CEntityDissolve::DissolveThink );
	if ( gpGlobals->curtime > m_flStartTime )
	{
		// Necessary for server-side ragdolls
		DissolveThink();
	}
	else
	{
		SetNextThink( gpGlobals->curtime + 0.01f );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : inputdata - 
//-----------------------------------------------------------------------------
void CEntityDissolve::InputDissolve( inputdata_t &inputdata )
{
	if (m_target == NULL_STRING)
		return;

	CBaseEntity *pTarget = NULL;
	while ((pTarget = gEntList.FindEntityGeneric(pTarget, STRING(m_target), this, inputdata.pActivator)) != NULL)
	{
		// Combat characters know how to catch themselves on fire.
		CBaseCombatCharacter *pBCC = pTarget->MyCombatCharacterPointer();
		if (pBCC)
		{
			pBCC->Dissolve( NULL, gpGlobals->curtime );
		}
	}
}



//-----------------------------------------------------------------------------
// Purpose: Creates a flame and attaches it to a target entity.
// Input  : pTarget - 
//-----------------------------------------------------------------------------
CEntityDissolve *CEntityDissolve::Create( CBaseEntity *pTarget, const char *pMaterialName, 
	float flStartTime, int nDissolveType, bool *pRagdollCreated )
{
	if ( pRagdollCreated )
	{
		*pRagdollCreated = false;
	}

	if ( !pMaterialName )
	{
		pMaterialName = DISSOLVE_SPRITE_NAME;
	}

	if ( pTarget->IsPlayer() )
	{
		// Simply immediately kill the player.
		CBasePlayer *pPlayer = assert_cast< CBasePlayer* >( pTarget );
		pPlayer->SetArmorValue( 0 );
		CTakeDamageInfo info( pPlayer, pPlayer, pPlayer->GetHealth(), DMG_GENERIC | DMG_REMOVENORAGDOLL | DMG_PREVENT_PHYSICS_FORCE );
		pPlayer->TakeDamage( info );
		return NULL;
	}

	CEntityDissolve *pDissolve = (CEntityDissolve *) CreateEntityByName( "env_entity_dissolver" );

	if ( pDissolve == NULL )
		return NULL;

	pDissolve->m_nDissolveType = nDissolveType;
	if ( (nDissolveType == ENTITY_DISSOLVE_ELECTRICAL) || (nDissolveType == ENTITY_DISSOLVE_ELECTRICAL_LIGHT) )
	{
		if ( pTarget->IsNPC() && pTarget->MyNPCPointer()->CanBecomeRagdoll() )
		{
			CTakeDamageInfo info;
			CBaseEntity *pRagdoll = CreateServerRagdoll( pTarget->MyNPCPointer(), 0, info, COLLISION_GROUP_DEBRIS, true );
			pRagdoll->SetCollisionBounds( pTarget->CollisionProp()->OBBMins(), pTarget->CollisionProp()->OBBMaxs() );

			// Necessary to cause it to do the appropriate death cleanup
			if ( pTarget->m_lifeState == LIFE_ALIVE )
			{
				CBasePlayer *pPlayer = UTIL_PlayerByIndex( 1 );
				CTakeDamageInfo ragdollInfo( pPlayer, pPlayer, 10000.0, DMG_SHOCK | DMG_REMOVENORAGDOLL | DMG_PREVENT_PHYSICS_FORCE );
				pTarget->TakeDamage( ragdollInfo );
			}

			if ( pRagdollCreated )
			{
				*pRagdollCreated = true;
			}

			UTIL_Remove( pTarget );
			pTarget = pRagdoll;
		}
	}

	pDissolve->SetModelName( MAKE_STRING(pMaterialName) );
	pDissolve->AttachToEntity( pTarget );
	pDissolve->SetStartTime( flStartTime );
	pDissolve->Spawn();

	// Send to the client even though we don't have a model
	pDissolve->AddEFlags( EFL_FORCE_CHECK_TRANSMIT );

	// Play any appropriate noises when we start to dissolve
	if ( (nDissolveType == ENTITY_DISSOLVE_ELECTRICAL) || (nDissolveType == ENTITY_DISSOLVE_ELECTRICAL_LIGHT) )
	{
		pTarget->DispatchResponse( "TLK_ELECTROCUTESCREAM" );
	}
	else
	{
		pTarget->DispatchResponse( "TLK_DISSOLVESCREAM" );
	}
	return pDissolve;
}


//-----------------------------------------------------------------------------
// What type of dissolve?
//-----------------------------------------------------------------------------
CEntityDissolve *CEntityDissolve::Create( CBaseEntity *pTarget, CBaseEntity *pSource )
{
	// Look for other boogies on the ragdoll + kill them
	for ( CBaseEntity *pChild = pSource->FirstMoveChild(); pChild; pChild = pChild->NextMovePeer() )
	{
		CEntityDissolve *pDissolve = dynamic_cast<CEntityDissolve*>(pChild);
		if ( !pDissolve )
			continue;

		return Create( pTarget, STRING( pDissolve->GetModelName() ), pDissolve->m_flStartTime, pDissolve->m_nDissolveType );
	}

	return NULL;
}

	
//-----------------------------------------------------------------------------
// Purpose: Attaches the flame to an entity and moves with it
// Input  : pTarget - target entity to attach to
//-----------------------------------------------------------------------------
void CEntityDissolve::AttachToEntity( CBaseEntity *pTarget )
{
	// So our dissolver follows the entity around on the server.
	SetParent( pTarget );
	SetLocalOrigin( vec3_origin );
	SetLocalAngles( vec3_angle );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : lifetime - 
//-----------------------------------------------------------------------------
void CEntityDissolve::SetStartTime( float flStartTime )
{
	m_flStartTime = flStartTime;
}

//-----------------------------------------------------------------------------
// Purpose: Burn targets around us
//-----------------------------------------------------------------------------
void CEntityDissolve::DissolveThink( void )
{
	CBaseAnimating *pTarget = ( GetMoveParent() ) ? GetMoveParent()->GetBaseAnimating() : NULL;
	
	if ( pTarget == NULL )
	{
		UTIL_Remove( this );
		return;
	}

	if ( pTarget && pTarget->GetFlags() & FL_TRANSRAGDOLL )
	{
		SetRenderColorA( 0 );
	}

	float dt = gpGlobals->curtime - m_flStartTime;

	if ( dt < m_flFadeInStart )
	{
		SetNextThink( m_flStartTime + m_flFadeInStart );
		return;
	}

	// If we're done fading, then kill our target entity and us
	if ( dt >= m_flFadeOutStart + m_flFadeOutLength )
	{
		// Necessary to cause it to do the appropriate death cleanup
		// Yeah, the player may have nothing to do with it, but
		// passing NULL to TakeDamage causes bad things to happen
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( 1 );
		CTakeDamageInfo info( pPlayer, pPlayer, 10000.0, DMG_GENERIC | DMG_REMOVENORAGDOLL | DMG_NO_PHYSICS_FORCE );
		pTarget->TakeDamage( info );

		if ( pTarget != pPlayer )
		{
			UTIL_Remove( pTarget );
		}
		
		UTIL_Remove( this );
		
		return;
	}

	SetNextThink( gpGlobals->curtime + TICK_INTERVAL );
}


//-----------------------------------------------------------------------------
// Purpose: Burn targets around us
//-----------------------------------------------------------------------------
void CEntityDissolve::ElectrocuteThink( void )
{
	CRagdollProp *pRagdoll = dynamic_cast< CRagdollProp* >( GetMoveParent() );
	if ( !pRagdoll )
		return;

	ragdoll_t *pRagdollPhys = pRagdoll->GetRagdoll( );
	for ( int j = 0; j < pRagdollPhys->listCount; ++j )
	{
		Vector vecForce;
		vecForce = RandomVector( -2400.0f, 2400.0f );
		pRagdollPhys->list[j].pObject->ApplyForceCenter( vecForce ); 
	}

	SetContextThink( &CEntityDissolve::ElectrocuteThink, gpGlobals->curtime + random->RandomFloat( 0.1, 0.2f ), 
		s_pElectroThinkContext );
}
