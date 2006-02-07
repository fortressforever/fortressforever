//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "npc_antlion.h"
#include "antlion_maker.h"
#include "saverestore_utlvector.h"
#include "ai_hint.h"
#include "mapentities.h"
#include "decals.h"
#include "iservervehicle.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CAntlionMakerManager g_AntlionMakerManager;

static const char *s_pPoolThinkContext = "PoolThinkContext";

ConVar g_debug_antlionmaker( "g_debug_antlionmaker", "0", FCVAR_CHEAT );

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &vFightGoal - 
//-----------------------------------------------------------------------------
void CAntlionMakerManager::BroadcastFightGoal( const Vector &vFightGoal )
{
	CAntlionTemplateMaker *pMaker;

	for ( int i=0; i < m_Makers.Size(); i++ )
	{
		pMaker = m_Makers[i];

		if ( pMaker && pMaker->ShouldHearBugbait() )
		{
			pMaker->SetFightTarget( vFightGoal );
			pMaker->SetChildMoveState( ANTLION_MOVE_FIGHT_TO_GOAL );
			pMaker->UpdateChildren();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pFightGoal - 
//-----------------------------------------------------------------------------
void CAntlionMakerManager::BroadcastFightGoal( CBaseEntity *pFightGoal )
{
	CAntlionTemplateMaker *pMaker;

	for ( int i=0; i < m_Makers.Size(); i++ )
	{
		pMaker = m_Makers[i];

		if ( pMaker && pMaker->ShouldHearBugbait() )
		{
			pMaker->SetFightTarget( pFightGoal );
			pMaker->SetChildMoveState( ANTLION_MOVE_FIGHT_TO_GOAL );
			pMaker->UpdateChildren();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pFightGoal - 
//-----------------------------------------------------------------------------
void CAntlionMakerManager::BroadcastFollowGoal( CBaseEntity *pFollowGoal )
{
	CAntlionTemplateMaker *pMaker;

	for ( int i=0; i < m_Makers.Size(); i++ )
	{
		pMaker = m_Makers[i];

		if ( pMaker && pMaker->ShouldHearBugbait() )
		{
			//pMaker->SetFightTarget( NULL );
			pMaker->SetFollowTarget( pFollowGoal );
			pMaker->SetChildMoveState( ANTLION_MOVE_FOLLOW );
			pMaker->UpdateChildren();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAntlionMakerManager::GatherMakers( void )
{
	CBaseEntity				*pSearch = NULL;
	CAntlionTemplateMaker	*pMaker;

	m_Makers.Purge();

	// Find these all once
	while ( ( pSearch = gEntList.FindEntityByClassname( pSearch, "npc_antlion_template_maker" ) ) != NULL )
	{
		pMaker = static_cast<CAntlionTemplateMaker *>(pSearch);

		m_Makers.AddToTail( pMaker );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAntlionMakerManager::LevelInitPostEntity( void )
{
	//Find all antlion makers
	GatherMakers();
}

//-----------------------------------------------------------------------------
// Antlion template maker
//-----------------------------------------------------------------------------

LINK_ENTITY_TO_CLASS( npc_antlion_template_maker, CAntlionTemplateMaker );

//DT Definition
BEGIN_DATADESC( CAntlionTemplateMaker )

	DEFINE_KEYFIELD( m_strSpawnGroup,	FIELD_STRING,	"spawngroup" ),
	DEFINE_KEYFIELD( m_strSpawnTarget,	FIELD_STRING,	"spawntarget" ),
	DEFINE_KEYFIELD( m_flSpawnRadius,	FIELD_FLOAT,	"spawnradius" ),
	DEFINE_KEYFIELD( m_strFightTarget,	FIELD_STRING,	"fighttarget" ),
	DEFINE_KEYFIELD( m_strFollowTarget,	FIELD_STRING,	"followtarget" ),
	DEFINE_KEYFIELD( m_bIgnoreBugbait,	FIELD_BOOLEAN,	"ignorebugbait" ),
	DEFINE_KEYFIELD( m_flVehicleSpawnDistance,	FIELD_FLOAT,	"vehicledistance" ),

	DEFINE_FIELD( m_nChildMoveState,	FIELD_INTEGER ),
	DEFINE_FIELD( m_hFightTarget,		FIELD_EHANDLE ),
	DEFINE_FIELD( m_hProxyTarget,		FIELD_EHANDLE ),
	DEFINE_FIELD( m_hFollowTarget,		FIELD_EHANDLE ),
	DEFINE_FIELD( m_iSkinCount,			FIELD_INTEGER ),

	DEFINE_UTLVECTOR( m_Children,		FIELD_EHANDLE ),

	DEFINE_KEYFIELD( m_iPool,			FIELD_INTEGER,	"pool_start" ),
	DEFINE_KEYFIELD( m_iMaxPool,		FIELD_INTEGER,	"pool_max" ),
	DEFINE_KEYFIELD( m_iPoolRegenAmount,FIELD_INTEGER,	"pool_regen_amount" ),
	DEFINE_KEYFIELD( m_flPoolRegenTime,	FIELD_FLOAT,	"pool_regen_time" ),

	DEFINE_INPUTFUNC( FIELD_STRING, "SetFightTarget",		InputSetFightTarget ),
	DEFINE_INPUTFUNC( FIELD_STRING, "SetFollowTarget",		InputSetFollowTarget ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"ClearFollowTarget",	InputClearFollowTarget ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"ClearFightTarget",		InputClearFightTarget ),
	DEFINE_INPUTFUNC( FIELD_FLOAT,	"SetSpawnRadius",		InputSetSpawnRadius ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "AddToPool",			InputAddToPool ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetMaxPool",			InputSetMaxPool ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetPoolRegenAmount",	InputSetPoolRegenAmount ),
	DEFINE_INPUTFUNC( FIELD_FLOAT,	 "SetPoolRegenTime",	InputSetPoolRegenTime ),

	DEFINE_THINKFUNC( PoolRegenThink ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CAntlionTemplateMaker::CAntlionTemplateMaker( void )
{
	m_hFightTarget = NULL;
	m_hProxyTarget = NULL;
	m_hFollowTarget = NULL;
	m_nChildMoveState = ANTLION_MOVE_FREE;
	m_iSkinCount = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CAntlionTemplateMaker::~CAntlionTemplateMaker( void )
{
	DestroyProxyTarget();
	m_Children.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pAnt - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::AddChild( CNPC_Antlion *pAnt )
{
	m_Children.AddToTail( pAnt );
	m_nLiveChildren = m_Children.Count();

	pAnt->SetOwnerEntity( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pAnt - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::RemoveChild( CNPC_Antlion *pAnt )
{
	m_Children.FindAndRemove( pAnt );
	m_nLiveChildren = m_Children.Count();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::FixupOrphans( void )
{
	CBaseEntity		*pSearch = NULL;
	CNPC_Antlion	*pAntlion = NULL;

	// Iterate through all antlions and see if there are any orphans
	while ( ( pSearch = gEntList.FindEntityByClassname( pSearch, "npc_antlion" ) ) != NULL )
	{
		pAntlion = dynamic_cast<CNPC_Antlion *>(pSearch);

		// See if it's a live orphan
		if ( pAntlion && pAntlion->GetOwnerEntity() == NULL && pAntlion->IsAlive() )
		{
			// See if its parent was named the same as we are
			if ( stricmp( pAntlion->GetParentSpawnerName(), STRING( GetEntityName() ) ) == 0 )
			{
				// Relink us to this antlion, he's come through a transition and was orphaned
				AddChild( pAntlion );
			}
		}
	}	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::Activate( void )
{
	FixupOrphans();

	BaseClass::Activate();

	// Are we using the pool behavior for coast?
	if ( m_iMaxPool )
	{
		if ( !m_flPoolRegenTime )
		{
			Warning("%s using pool behavior without a specified pool regen time.\n", GetClassname() );
			m_flPoolRegenTime = 0.1;
		}

		SetContextThink( &CAntlionTemplateMaker::PoolRegenThink, gpGlobals->curtime + m_flPoolRegenTime, s_pPoolThinkContext );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : CBaseEntity
//-----------------------------------------------------------------------------
CBaseEntity *CAntlionTemplateMaker::GetFightTarget( void )
{
	if ( m_hFightTarget != NULL )
		return m_hFightTarget;

	return m_hProxyTarget;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : CBaseEntity
//-----------------------------------------------------------------------------
CBaseEntity *CAntlionTemplateMaker::GetFollowTarget( void )
{
	return m_hFollowTarget;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::UpdateChildren( void )
{
	//Update all children
	CNPC_Antlion *pAntlion = NULL;

	// Move through our child list
	for ( int i=0; i < m_Children.Size(); i++ )
	{
		pAntlion = m_Children[i];
		
		//HACKHACK
		//Let's just fix this up.
		//This guy might have been killed in another level and we just came back.
		if ( pAntlion == NULL )
		{
			m_Children.Remove( i );
			i--;
			continue;
		}
		
		if ( pAntlion->m_lifeState != LIFE_ALIVE )
			 continue;

		pAntlion->SetFightTarget( GetFightTarget() );
		pAntlion->SetFollowTarget( GetFollowTarget() );
		pAntlion->SetMoveState( m_nChildMoveState );
	}

	m_nLiveChildren = i;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : strTarget - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::SetFightTarget( string_t strTarget )
{
	CBaseEntity *pSearch = gEntList.FindEntityByName( NULL, strTarget, this );

	if ( pSearch != NULL )
	{
		SetFightTarget( pSearch );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEntity - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::SetFightTarget( CBaseEntity *pEntity )
{
	m_hFightTarget = pEntity;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &position - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::SetFightTarget( const Vector &position )
{
	CreateProxyTarget( position );
	
	m_hFightTarget = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTarget - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::SetFollowTarget( CBaseEntity *pTarget )
{
	m_hFollowTarget = pTarget;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTarget - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::SetFollowTarget( string_t strTarget )
{
	CBaseEntity *pSearch = gEntList.FindEntityByName( NULL, strTarget, this );

	if ( pSearch != NULL )
	{
		SetFollowTarget( pSearch );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : state - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::SetChildMoveState( AntlionMoveState_e state )
{
	m_nChildMoveState = state;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &position - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::CreateProxyTarget( const Vector &position )
{
	// Create if we don't have one
	if ( m_hProxyTarget == NULL )
	{
		m_hProxyTarget = CreateEntityByName( "info_target" );
	}

	// Update if we do
	if ( m_hProxyTarget != NULL )
	{
		m_hProxyTarget->SetAbsOrigin( position );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::DestroyProxyTarget( void )
{
	if ( m_hProxyTarget )
	{
		UTIL_Remove( m_hProxyTarget );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bIgnoreSolidEntities - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CAntlionTemplateMaker::CanMakeNPC( bool bIgnoreSolidEntities )
{
	if ( m_nMaxLiveChildren == 0 )
		 return false;

	if ( !HasSpawnFlags( SF_ANTLIONMAKER_SPAWN_CLOSE_TO_TARGET ) )
	{
		if ( m_strSpawnGroup == NULL_STRING )
			 return BaseClass::CanMakeNPC( bIgnoreSolidEntities );
	}

	if ( m_nMaxLiveChildren > 0 && m_nLiveChildren >= m_nMaxLiveChildren )
		return false;

	// If we're spawning from a pool, ensure the pool has an antlion in it
	if ( m_iMaxPool && !m_iPool )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::MakeNPC( void )
{
	// If we're not restricting to hint groups, spawn as normal
	if ( !HasSpawnFlags( SF_ANTLIONMAKER_SPAWN_CLOSE_TO_TARGET ) )
	{
		if ( m_strSpawnGroup == NULL_STRING )
		{
			BaseClass::MakeNPC();
			return;
		}
	}

	if ( CanMakeNPC( true ) == false )
		return;

	// Set our defaults
	Vector	targetOrigin = GetAbsOrigin();
	QAngle	targetAngles = GetAbsAngles();

	// Look for our target entity
	CBaseEntity *pTarget = gEntList.FindEntityByName( NULL, m_strSpawnTarget, this );

	// Take its position if it exists
	if ( pTarget != NULL )
	{
		UTIL_PredictedPosition( pTarget, 1.5f, &targetOrigin );
	}

	Vector	spawnOrigin = vec3_origin;

	CAI_Hint *pNode = NULL;

	bool bRandom = HasSpawnFlags( SF_ANTLIONMAKER_RANDOM_SPAWN_NODE );

	if ( HasSpawnFlags( SF_ANTLIONMAKER_SPAWN_CLOSE_TO_TARGET ) )
	{
		if ( FindNearTargetSpawnPosition( spawnOrigin, m_flSpawnRadius, pTarget ) == false )
			return;
	}
	else
	{
		// If we can't find a spawn position, then we can't spawn this time
		if ( FindHintSpawnPosition( targetOrigin, m_flSpawnRadius, m_strSpawnGroup, &pNode, bRandom ) == false )
			return;

		 pNode->GetPosition( HULL_MEDIUM, &spawnOrigin );

		trace_t	tr;
		UTIL_TraceHull( spawnOrigin, spawnOrigin + Vector(0,0,1), NAI_Hull::Mins( HULL_MEDIUM ), NAI_Hull::Maxs( HULL_MEDIUM ), MASK_NPCSOLID, NULL, COLLISION_GROUP_NONE, &tr );

		if ( tr.startsolid || tr.allsolid )
			return;

		CBaseEntity*	pList[100];

		Vector vMins = NAI_Hull::Mins( HULL_MEDIUM );
		Vector vMaxs = NAI_Hull::Maxs( HULL_MEDIUM );
		vMins.z = -96;
		UTIL_TraceHull( spawnOrigin, spawnOrigin + Vector(0,0,1), vMins, vMaxs, MASK_NPCSOLID, NULL, COLLISION_GROUP_NONE, &tr );

		int count = UTIL_EntitiesInBox( pList, 100, spawnOrigin + vMins, spawnOrigin + vMaxs, 0 );
	
		//Iterate over all the possible targets
		for ( int i = 0; i < count; i++ )
		{
			CBaseEntity *pObstruction = pList[i];

			if ( pObstruction )
			{
				if ( pObstruction->GetMoveType() == MOVETYPE_VPHYSICS && pObstruction->GetCollisionGroup() != COLLISION_GROUP_DEBRIS && pObstruction->GetCollisionGroup() != COLLISION_GROUP_INTERACTIVE_DEBRIS  )
				{
					IPhysicsObject *pObject = pObstruction->VPhysicsGetObject();
					
					if ( pObject )
					{
						Vector v;
						Vector physicsCenter = pObstruction->WorldSpaceCenter();
						v = spawnOrigin + Vector( 0, 0, 128 ) - physicsCenter;

						VectorNormalize( v );

						v = v * 800;
		
						AngularImpulse angVelocity( random->RandomFloat(-180, 180), random->RandomFloat(-180, 180), random->RandomFloat(-360, 360) );

						pObject->AddVelocity( &v, &angVelocity );
					}
				}
			}
		}
	}
	
	// Point at the current position of the enemy
	if ( pTarget != NULL )
	{
		targetOrigin = pTarget->GetAbsOrigin();
	}	
 	
	// Create the entity via a template
	CAI_BaseNPC	*pent = NULL;
	CBaseEntity *pEntity = NULL;
	MapEntity_ParseEntity( pEntity, STRING(m_iszTemplateData), NULL );
	
	if ( pEntity != NULL )
	{
		pent = (CAI_BaseNPC *) pEntity;
	}

	if ( pent == NULL )
	{
		Warning("NULL Ent in NPCMaker!\n" );
		return;
	}
	
	if ( !HasSpawnFlags( SF_ANTLIONMAKER_SPAWN_CLOSE_TO_TARGET ) )
	{
		// Lock this hint node
		pNode->Lock( pEntity );
		
		// Unlock it in two seconds, this forces subsequent antlions to 
		// reject this point as a spawn point to spread them out a bit
		pNode->Unlock( 2.0f );
	}

	m_OnSpawnNPC.Set( pEntity, pEntity, this );

	pent->AddSpawnFlags( SF_NPC_FALL_TO_GROUND );

	ChildPreSpawn( pent );

	DispatchSpawn( pent );
	
	// Put us at the desired location
	pent->SetLocalOrigin( spawnOrigin );
	
	// Face our spawning direction
	Vector	spawnDir = ( targetOrigin - spawnOrigin );
	VectorNormalize( spawnDir );

	QAngle	spawnAngles;
	VectorAngles( spawnDir, spawnAngles );
	spawnAngles[PITCH] = 0.0f;
	spawnAngles[ROLL] = 0.0f;

	pent->SetLocalAngles( spawnAngles );	
	pent->Activate();

	m_iSkinCount = ( m_iSkinCount + 1 ) % ANTLION_SKIN_COUNT;
	pent->m_nSkin = m_iSkinCount; 

	ChildPostSpawn( pent );

	// Hold onto the child
	AddChild( static_cast<CNPC_Antlion *>(pent) );

	if (!(m_spawnflags & SF_NPCMAKER_INF_CHILD))
	{
		if ( m_iMaxPool )
		{
			m_iPool--;

			if ( g_debug_antlionmaker.GetInt() == 2 )
			{
				Msg("SPAWNED: Pool: %d (max %d) (Regenerating %d every %f)\n", m_iPool, m_iMaxPool, m_iPoolRegenAmount, m_flPoolRegenTime );
			}
		}
		else
		{
			m_nMaxNumNPCs--;
		}

		if ( IsDepleted() )
		{
			m_OnAllSpawned.FireOutput( this, this );

			// Disable this forever.  Don't kill it because it still gets death notices
			SetThink( NULL );
			SetUse( NULL );
		}
	}
}

bool CAntlionTemplateMaker::FindPositionOnFoot( Vector &origin, float radius, CBaseEntity *pTarget )
{
	int iMaxTries = 10;
	Vector vSpawnOrigin = pTarget->GetAbsOrigin();

	while ( iMaxTries > 0 )
	{
		vSpawnOrigin.x += random->RandomFloat( -radius, radius );
		vSpawnOrigin.y += random->RandomFloat( -radius, radius );
		vSpawnOrigin.z += 96;

		if ( ValidateSpawnPosition( vSpawnOrigin, pTarget ) == false )
		{
			iMaxTries--;
			continue;
		}

		origin = vSpawnOrigin;
		return true;
	}

	return false;
}

bool CAntlionTemplateMaker::FindPositionOnVehicle( Vector &origin, float radius, CBaseEntity *pTarget )
{
	int iMaxTries = 10;
	Vector vSpawnOrigin = pTarget->GetAbsOrigin();
	vSpawnOrigin.z += 96;

	if ( pTarget == NULL )
		 return false;

	while ( iMaxTries > 0 )
	{
		Vector vForward, vRight;
		
		pTarget->GetVectors( &vForward, &vRight, NULL );

		float flSpeed = (pTarget->GetSmoothedVelocity().Length() * m_flVehicleSpawnDistance) * random->RandomFloat( 1.0f, 1.5f );
	
		vSpawnOrigin = vSpawnOrigin + (vForward * flSpeed) + vRight * random->RandomFloat( -radius, radius );

		if ( ValidateSpawnPosition( vSpawnOrigin, pTarget ) == false )
		{
			iMaxTries--;
			continue;
		}

		origin = vSpawnOrigin;
		return true;
	}

	return false;
}

bool CAntlionTemplateMaker::ValidateSpawnPosition( Vector &vOrigin, CBaseEntity *pTarget )
{
	trace_t	tr;
	UTIL_TraceLine( vOrigin, vOrigin - Vector( 0, 0, 1024 ), MASK_OPAQUE | CONTENTS_WATER, NULL, COLLISION_GROUP_NONE, &tr );

	if ( g_debug_antlionmaker.GetInt() == 1 )
		 NDebugOverlay::Line( vOrigin, tr.endpos, 0, 255, 0, false, 5 );
		
	// Make sure this point is clear 
	if ( tr.fraction != 1.0 )
	{
		if ( tr.contents & ( CONTENTS_WATER ) )
			 return false;

		surfacedata_t *psurf = physprops->GetSurfaceData( tr.surface.surfaceProps );

		if ( psurf )
		{
			if ( g_debug_antlionmaker.GetInt() == 1 )
			{
				char szText[16];

				Q_snprintf( szText, 16, "Material %c", psurf->game.material );
				NDebugOverlay::Text( vOrigin, szText, true, 5 );
			}

			if ( psurf->game.material != CHAR_TEX_SAND )
				return false;
		}

		if ( CAntlionRepellant::IsPositionRepellantFree( tr.endpos ) == false )
			 return false;
	
		trace_t trCheck;
		UTIL_TraceHull( tr.endpos, tr.endpos + Vector(0,0,5), NAI_Hull::Mins( HULL_MEDIUM ), NAI_Hull::Maxs( HULL_MEDIUM ), MASK_NPCSOLID, NULL, COLLISION_GROUP_NONE, &trCheck );

		if ( trCheck.DidHit() == false )
		{
			if ( g_debug_antlionmaker.GetInt() == 1 )
				 NDebugOverlay::Box( tr.endpos + Vector(0,0,5), NAI_Hull::Mins( HULL_MEDIUM ), NAI_Hull::Maxs( HULL_MEDIUM ), 0, 255, 0, 128, 5 );
		
			if ( pTarget )
			{
				if ( pTarget->IsPlayer() )
				{
					CBaseEntity *pVehicle = NULL;
					CBasePlayer *pPlayer = dynamic_cast < CBasePlayer *> ( pTarget );

					if ( pPlayer && pPlayer->GetVehicle() )
						 pVehicle = ((CBasePlayer *)pTarget)->GetVehicle()->GetVehicleEnt();

					CTraceFilterSkipTwoEntities traceFilter( pPlayer, pVehicle, COLLISION_GROUP_NONE );

					trace_t trVerify;
					
					Vector vVerifyOrigin = pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset();
					float flZOffset = NAI_Hull::Maxs( HULL_MEDIUM ).z;
					UTIL_TraceLine( vVerifyOrigin, tr.endpos + Vector( 0, 0, flZOffset ), MASK_OPAQUE | CONTENTS_WATER, &traceFilter, &trVerify );

					if ( trVerify.fraction != 1.0f )
					{
						surfacedata_t *psurf = physprops->GetSurfaceData( trVerify.surface.surfaceProps );

						if ( psurf )
						{
							if ( psurf->game.material == CHAR_TEX_DIRT )
							{
								if ( g_debug_antlionmaker.GetInt() == 1 )
								{
									NDebugOverlay::Line( vVerifyOrigin, trVerify.endpos, 255, 0, 0, false, 5 );
								}

								return false;
							}
						}
					}

					if ( g_debug_antlionmaker.GetInt() == 1 )
					{
						NDebugOverlay::Line( vVerifyOrigin, trVerify.endpos, 0, 255, 0, false, 5 );
					}
				}
			}

	
			vOrigin = trCheck.endpos + Vector(0,0,5);
			return true;
		}
		else
		{
			if ( g_debug_antlionmaker.GetInt() == 1 )
				 NDebugOverlay::Box( tr.endpos + Vector(0,0,5), NAI_Hull::Mins( HULL_MEDIUM ), NAI_Hull::Maxs( HULL_MEDIUM ), 255, 0, 0, 128, 5 );

			return false;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Find a position near the player to spawn the new antlion at
// Input  : &origin - search origin
//			radius - search radius
//			*retOrigin - found origin (if any)
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CAntlionTemplateMaker::FindNearTargetSpawnPosition( Vector &origin, float radius, CBaseEntity *pTarget )
{
	if ( pTarget )
	{
		CBaseEntity *pVehicle = NULL;

		if ( pTarget->IsPlayer() )
		{
			CBasePlayer *pPlayer = ((CBasePlayer *)pTarget);

			if ( pPlayer->GetVehicle() )
				 pVehicle = ((CBasePlayer *)pTarget)->GetVehicle()->GetVehicleEnt();
		}

		if ( pVehicle )
		     return FindPositionOnVehicle( origin, radius, pVehicle );
		else 
			 return FindPositionOnFoot( origin, radius, pTarget );
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Find a hint position to spawn the new antlion at
// Input  : &origin - search origin
//			radius - search radius
//			hintGroupName - search hint group name
//			*retOrigin - found origin (if any)
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CAntlionTemplateMaker::FindHintSpawnPosition( const Vector &origin, float radius, string_t hintGroupName, CAI_Hint **pHint, bool bRandom )
{
	CHintCriteria	hintCriteria;

	hintCriteria.SetGroup( hintGroupName );
	hintCriteria.SetHintType( HINT_ANTLION_BURROW_POINT );
	
	if ( bRandom )
	{
		hintCriteria.SetFlag( bits_HINT_NODE_RANDOM );
	}
	else
	{
		hintCriteria.SetFlag( bits_HINT_NODE_NEAREST );
	}
	
	// If requested, deny nodes that can be seen by the player
	if ( m_spawnflags & SF_NPCMAKER_HIDEFROMPLAYER )
	{
		hintCriteria.SetFlag( bits_HINT_NODE_NOT_VISIBLE_TO_PLAYER );
	}

	hintCriteria.AddIncludePosition( origin, radius );

	if ( bRandom == true )
	{
		*pHint = CAI_HintManager::FindHintRandom( NULL, origin, hintCriteria );
	}
	else
	{
		*pHint = CAI_HintManager::FindHint( origin, hintCriteria );
	}

	if ( *pHint != NULL )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Makes the antlion immediatley unburrow if it started burrowed
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::ChildPostSpawn( CAI_BaseNPC *pChild )
{
	CNPC_Antlion *pAntlion = static_cast<CNPC_Antlion*>(pChild);

	if ( pAntlion == NULL )
		return;

	// Unburrow the spawned antlion immediately
	if ( pAntlion->m_bStartBurrowed )
	{
		pAntlion->BurrowUse( this, this, USE_ON, 0.0f );
	}

	// Set us to a follow target, if we have one
	if ( GetFollowTarget() )
	{
		pAntlion->SetFollowTarget( GetFollowTarget() );
	}
	else if ( ( m_strFollowTarget != NULL_STRING ) )
	{
		// If we don't already have a fight target, set it up
		SetFollowTarget( m_strFollowTarget );

		if ( GetFightTarget() == NULL )
		{
			SetChildMoveState( ANTLION_MOVE_FOLLOW );

			// If it's valid, fight there
			if ( GetFollowTarget() != NULL )
			{
				pAntlion->SetFollowTarget( GetFollowTarget() );
			}
		}
	}
	// See if we need to send them on their way to a fight goal
	if ( GetFightTarget() )
	{
		pAntlion->SetFightTarget( GetFightTarget() );
	}
	else if ( m_strFightTarget != NULL_STRING )
	{
		// If we don't already have a fight target, set it up
		SetFightTarget( m_strFightTarget );	
		SetChildMoveState( ANTLION_MOVE_FIGHT_TO_GOAL );

		// If it's valid, fight there
		if ( GetFightTarget() != NULL )
		{
			pAntlion->SetFightTarget( GetFightTarget() );
		}
	}

	// Set us to the desired movement state
	pAntlion->SetMoveState( m_nChildMoveState );

	// Save our name for level transitions
	pAntlion->SetParentSpawnerName( STRING( GetEntityName() ) );

	BaseClass::ChildPostSpawn( pChild );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::InputSetFightTarget( inputdata_t &inputdata )
{
	// Set our new goal
	m_strFightTarget = MAKE_STRING( inputdata.value.String() );

	SetFightTarget( m_strFightTarget );
	SetChildMoveState( ANTLION_MOVE_FIGHT_TO_GOAL );
	
	UpdateChildren();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::InputSetFollowTarget( inputdata_t &inputdata )
{
	// Set our new goal
	m_strFollowTarget = MAKE_STRING( inputdata.value.String() );

	SetFollowTarget( m_strFollowTarget );
	SetChildMoveState( ANTLION_MOVE_FOLLOW );
	
	UpdateChildren();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::InputClearFightTarget( inputdata_t &inputdata )
{
	SetFightTarget( NULL );
	SetChildMoveState( ANTLION_MOVE_FOLLOW );

	UpdateChildren();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::InputClearFollowTarget( inputdata_t &inputdata )
{
	SetFollowTarget( NULL );
	SetChildMoveState( ANTLION_MOVE_FIGHT_TO_GOAL );

	UpdateChildren();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::InputSetSpawnRadius( inputdata_t &inputdata )
{
	m_flSpawnRadius = inputdata.value.Float();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::InputAddToPool( inputdata_t &inputdata )
{
	PoolAdd( inputdata.value.Int() );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::InputSetMaxPool( inputdata_t &inputdata )
{
	m_iMaxPool = inputdata.value.Int();
	if ( m_iPool > m_iMaxPool )
	{
		m_iPool = m_iMaxPool;
	}

	// Stop regenerating if we're supposed to stop using the pool
	if ( !m_iMaxPool )
	{
		SetContextThink( NULL, gpGlobals->curtime, s_pPoolThinkContext );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::InputSetPoolRegenAmount( inputdata_t &inputdata )
{
	m_iPoolRegenAmount = inputdata.value.Int();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::InputSetPoolRegenTime( inputdata_t &inputdata )
{
	m_flPoolRegenTime = inputdata.value.Float();
}

//-----------------------------------------------------------------------------
// Purpose: Pool behavior for coast
// Input  : iNumToAdd - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::PoolAdd( int iNumToAdd )
{
	m_iPool = clamp( m_iPool + iNumToAdd, 0, m_iMaxPool );
}

//-----------------------------------------------------------------------------
// Purpose: Regenerate the pool
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::PoolRegenThink( void )
{
	if ( m_iPool < m_iMaxPool )
	{
		m_iPool = clamp( m_iPool + m_iPoolRegenAmount, 0, m_iMaxPool );

		if ( g_debug_antlionmaker.GetInt() == 2 )
		{
			Msg("REGENERATED: Pool: %d (max %d) (Regenerating %d every %f)\n", m_iPool, m_iMaxPool, m_iPoolRegenAmount, m_flPoolRegenTime );
		}
	}

	SetContextThink( &CAntlionTemplateMaker::PoolRegenThink, gpGlobals->curtime + m_flPoolRegenTime, s_pPoolThinkContext );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pVictim - 
//-----------------------------------------------------------------------------
void CAntlionTemplateMaker::DeathNotice( CBaseEntity *pVictim )
{
	CNPC_Antlion *pAnt = dynamic_cast<CNPC_Antlion *>(pVictim);

	if ( pAnt == NULL )
		return;

	// Take it out of our list
	RemoveChild( pAnt );

	// See if we've exhausted our supply of NPCs
	if ( ( HasSpawnFlags(SF_NPCMAKER_INF_CHILD) == false ) && IsDepleted() || m_iMaxPool && !m_iPool )
	{
		// Signal that all our children have been spawned and are now dead
		m_OnAllSpawnedDead.FireOutput( this, this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: If this had a finite number of children, return true if they've all
//			been created.
//-----------------------------------------------------------------------------
bool CAntlionTemplateMaker::IsDepleted( void )
{
	// If we're running pool behavior, we're never depleted
	if ( m_iMaxPool )
		return false;

	return BaseClass::IsDepleted();
}
