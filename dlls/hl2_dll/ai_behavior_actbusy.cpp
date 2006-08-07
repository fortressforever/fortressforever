//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"

#include "ai_behavior_actbusy.h"

#include "ai_navigator.h"
#include "ai_hint.h"
#include "ai_behavior_follow.h"
#include "KeyValues.h"
#include "filesystem.h"
#include "eventqueue.h"
#include "ai_playerally.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "entityblocker.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ACTBUSY_SEE_ENTITY_TIMEOUT	1.0f

ConVar	ai_actbusy_search_time( "ai_actbusy_search_time","10.0" );
ConVar  ai_debug_actbusy( "ai_debug_actbusy", "0", FCVAR_CHEAT, "Used to debug actbusy behavior. Usage:\n\
1: Constantly draw lines from NPCs to the actbusy nodes they've chosen to actbusy at.\n\
2: Whenever an NPC makes a decision to use an actbusy, show which actbusy they've chosen.\n\
3: Selected NPCs (with npc_select) will report why they're not choosing actbusy nodes.\n\
4: Display debug output of actbusy logic.\n\
");

BEGIN_DATADESC( CAI_ActBusyBehavior )
	DEFINE_FIELD( m_bEnabled, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bForceActBusy, FIELD_BOOLEAN ),
	DEFINE_CUSTOM_FIELD( m_ForcedActivity, ActivityDataOps() ),
	DEFINE_FIELD( m_bTeleportToBusy, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bUseNearestBusy, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bLeaving, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bVisibleOnly, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bUseRenderBoundsForCollision, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flForcedMaxTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_bBusy, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bMovingToBusy, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bNeedsToPlayExitAnim, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flNextBusySearchTime, FIELD_TIME ),
	DEFINE_FIELD( m_flEndBusyAt, FIELD_TIME ),
	DEFINE_FIELD( m_flBusySearchRange, FIELD_FLOAT ),
	DEFINE_FIELD( m_bInQueue, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iCurrentBusyAnim, FIELD_INTEGER ),
	DEFINE_FIELD( m_hActBusyGoal, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bNeedToSetBounds, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hSeeEntity, FIELD_EHANDLE ),
	DEFINE_FIELD( m_fTimeLastSawSeeEntity, FIELD_TIME ),
	DEFINE_FIELD( m_bExitedBusyToDueLostSeeEntity, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bExitedBusyToDueSeeEnemy, FIELD_BOOLEAN ),
END_DATADESC();


//=============================================================================
//-----------------------------------------------------------------------------
// Purpose: Gamesystem that parses the act busy anim file
//-----------------------------------------------------------------------------
class CActBusyAnimData : public CAutoGameSystem
{
public:
	CActBusyAnimData( void ) : CAutoGameSystem( "CActBusyAnimData" )
	{
	}

	// Inherited from IAutoServerSystem
	virtual void LevelInitPostEntity( void );
	virtual void LevelShutdownPostEntity( void );

	// Read in the data from the anim data file
	void ParseAnimDataFile( void );

	// Parse a keyvalues section into an act busy anim
	bool ParseActBusyFromKV( busyanim_t *pAnim, KeyValues *pSection );

	// Purpose: Returns the index of the busyanim data for the specified activity or sequence
	int FindBusyAnim( Activity iActivity, const char *pSequence );

	busyanim_t *GetBusyAnim( int iIndex ) { return &m_ActBusyAnims[iIndex]; }

protected:
	CUtlVector<busyanim_t>	m_ActBusyAnims;
};

CActBusyAnimData g_ActBusyAnimDataSystem;

//-----------------------------------------------------------------------------
// Inherited from IAutoServerSystem
//-----------------------------------------------------------------------------
void CActBusyAnimData::LevelInitPostEntity( void )
{
	ParseAnimDataFile();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CActBusyAnimData::LevelShutdownPostEntity( void )
{
	m_ActBusyAnims.Purge();
}

//-----------------------------------------------------------------------------
// Clear out the stats + their history
//-----------------------------------------------------------------------------
void CActBusyAnimData::ParseAnimDataFile( void )
{
	KeyValues *pKVAnimData = new KeyValues( "ActBusyAnimDatafile" );
	if ( pKVAnimData->LoadFromFile( filesystem, "scripts/actbusy.txt" ) )
	{
		// Now try and parse out each act busy anim
		KeyValues *pKVAnim = pKVAnimData->GetFirstSubKey();
		while ( pKVAnim )
		{
			// Create a new anim and add it to our list
			int index = m_ActBusyAnims.AddToTail();
			busyanim_t *pAnim = &m_ActBusyAnims[index];
			if ( !ParseActBusyFromKV( pAnim, pKVAnim ) )
			{
				m_ActBusyAnims.Remove( index );
			}

			pKVAnim = pKVAnim->GetNextKey();
		}
	}
	pKVAnimData->deleteThis();
}	

//-----------------------------------------------------------------------------
// Purpose: Parse a keyvalues section into the prop
//-----------------------------------------------------------------------------
bool CActBusyAnimData::ParseActBusyFromKV( busyanim_t *pAnim, KeyValues *pSection )
{
	pAnim->iszName = AllocPooledString( pSection->GetName() );

	// Activities
	pAnim->iActivities[BA_BUSY] = (Activity)CAI_BaseNPC::GetActivityID( pSection->GetString( "busy_anim", "ACT_INVALID" ) );
	pAnim->iActivities[BA_ENTRY] = (Activity)CAI_BaseNPC::GetActivityID( pSection->GetString( "entry_anim", "ACT_INVALID" ) );
	pAnim->iActivities[BA_EXIT] = (Activity)CAI_BaseNPC::GetActivityID( pSection->GetString( "exit_anim", "ACT_INVALID" ) );

	// Sequences
	pAnim->iszSequences[BA_BUSY] = AllocPooledString( pSection->GetString( "busy_sequence", NULL ) );
	pAnim->iszSequences[BA_ENTRY] = AllocPooledString( pSection->GetString( "entry_sequence", NULL ) );
	pAnim->iszSequences[BA_EXIT] = AllocPooledString( pSection->GetString( "exit_sequence", NULL ) );

	// Sounds
	pAnim->iszSounds[BA_BUSY] = AllocPooledString( pSection->GetString( "busy_sound", NULL ) );
	pAnim->iszSounds[BA_ENTRY] = AllocPooledString( pSection->GetString( "entry_sound", NULL ) );
	pAnim->iszSounds[BA_EXIT] = AllocPooledString( pSection->GetString( "exit_sound", NULL ) );
	
	// Times
	pAnim->flMinTime = pSection->GetFloat( "min_time", 10.0 );
	pAnim->flMaxTime = pSection->GetFloat( "max_time", 20.0 );

	pAnim->bUseAutomovement = pSection->GetInt( "use_automovement", 0 ) != 0;

	const char *sInterrupt = pSection->GetString( "interrupts", "BA_INT_DANGER" );
	if ( !strcmp( sInterrupt, "BA_INT_PLAYER" ) )
	{
		pAnim->iBusyInterruptType = BA_INT_PLAYER;
	}
	else if ( !strcmp( sInterrupt, "BA_INT_DANGER" ) )
	{
		pAnim->iBusyInterruptType = BA_INT_DANGER;
	}
	else if ( !strcmp( sInterrupt, "BA_INT_AMBUSH" ) )
	{
		pAnim->iBusyInterruptType = BA_INT_AMBUSH;
	}
	else if ( !strcmp( sInterrupt, "BA_INT_COMBAT" ) )
	{
		pAnim->iBusyInterruptType = BA_INT_COMBAT;
	}
	else if ( !strcmp( sInterrupt, "BA_INT_ZOMBIESLUMP" ))
	{
		pAnim->iBusyInterruptType = BA_INT_ZOMBIESLUMP;
	}
	else
	{
		pAnim->iBusyInterruptType = BA_INT_NONE;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the busyanim data for the specified activity
//-----------------------------------------------------------------------------
int CActBusyAnimData::FindBusyAnim( Activity iActivity, const char *pSequence )
{
	int iCount = m_ActBusyAnims.Count();
	for ( int i = 0; i < iCount; i++ )
	{
		busyanim_t *pBusyAnim = &m_ActBusyAnims[i];
		Assert( pBusyAnim );

		if ( pSequence && pBusyAnim->iszName != NULL_STRING && !Q_stricmp( STRING(pBusyAnim->iszName), pSequence ) )
			return i;

		if ( iActivity != ACT_INVALID && pBusyAnim->iActivities[BA_BUSY] == iActivity )
			return i;
	}

	if ( pSequence )
	{
		Warning("Specified '%s' as a busy anim name, and it's not in the act busy anim list.\n", pSequence );
	}
	else if ( iActivity != ACT_INVALID )
	{
		Warning("Tried to use Activity %d as a busy anim, and it's not in the act busy anim list.\n", iActivity );
	}

	return -1;
}

//=============================================================================================================
//=============================================================================================================

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CAI_ActBusyBehavior::CAI_ActBusyBehavior()
{
	// Disabled by default. Enabled by map entity.
	m_bEnabled = false;
	m_bUseRenderBoundsForCollision = false;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyBehavior::Enable( CAI_ActBusyGoal *pGoal, float flRange, bool bVisibleOnly )
{
	NotifyBusyEnding();

	if ( pGoal )
	{
		m_hActBusyGoal = pGoal;
	}
	m_bEnabled = true;
	m_bBusy = false;
	m_bMovingToBusy = false;
	m_bNeedsToPlayExitAnim = false;
	m_bLeaving = false;
	m_flNextBusySearchTime = gpGlobals->curtime + ai_actbusy_search_time.GetFloat();
	m_flEndBusyAt = 0;
	m_bVisibleOnly = bVisibleOnly;
	m_bInQueue = dynamic_cast<CAI_ActBusyQueueGoal*>(m_hActBusyGoal.Get()) != NULL;
	m_ForcedActivity = ACT_INVALID;
	m_hSeeEntity = NULL;
	m_bExitedBusyToDueLostSeeEntity = false;
	m_bExitedBusyToDueSeeEnemy = false;

	SetBusySearchRange( flRange );

	if ( ai_debug_actbusy.GetInt() == 4 )
	{
		Msg("ACTBUSY: behavior enabled on NPC %s (%s)\n", GetOuter()->GetClassname(), GetOuter()->GetDebugName() );
	}

	// Robin: Due to ai goal entities delaying their EnableGoal call on each
	// of their target Actors, NPCs that are spawned with active actbusies
	// will have their SelectSchedule() called before their behavior has been
	// enabled. To fix this, if we're enabled while in a schedule that can be
	// overridden, immediately act busy.
	if ( IsCurScheduleOverridable() )
	{
		// Force search time to be now.
		m_flNextBusySearchTime = gpGlobals->curtime;
		GetOuter()->ClearSchedule();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyBehavior::SetBusySearchRange( float flRange )
{
	m_flBusySearchRange = flRange;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyBehavior::Disable( void )
{
	if ( ai_debug_actbusy.GetInt() == 4 )
	{
		Msg("ACTBUSY: behavior disabled on NPC %s (%s)\n", GetOuter()->GetClassname(), GetOuter()->GetDebugName() );
	}

	if ( m_bEnabled )
	{
		SetCondition( COND_PROVOKED );
	}

	StopBusying();
	m_bEnabled = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyBehavior::ForceActBusy( CAI_ActBusyGoal *pGoal, CAI_Hint *pHintNode, float flMaxTime, bool bVisibleOnly, bool bTeleportToBusy, bool bUseNearestBusy, CBaseEntity *pSeeEntity, Activity activity )
{
	Assert( !m_bLeaving );

	if ( m_bNeedsToPlayExitAnim )
	{
		// If we hit this, the mapmaker's told this NPC to actbusy somewhere while it's still in an actbusy.
		// Right now, we don't support this. We could support it with a bit of work.
		if ( HasAnimForActBusy( m_iCurrentBusyAnim, BA_EXIT ) )
		{
			Warning("ACTBUSY: %s(%s) was told to actbusy while inside an actbusy that needs to exit first. IGNORING.\n", GetOuter()->GetDebugName(), GetOuter()->GetClassname() );
			return;
		}
	}

	if ( ai_debug_actbusy.GetInt() == 4 )
	{
		Msg("ACTBUSY: ForceActBusy on NPC %s (%s): ", GetOuter()->GetClassname(), GetOuter()->GetDebugName() );
		if ( pHintNode )
		{
			Msg("Hintnode %s", pHintNode->GetDebugName());
		}
		else
		{
			Msg("No Hintnode specified");
		}
		Msg("\n");
	}

	Enable( pGoal, m_flBusySearchRange, bVisibleOnly );
	m_bForceActBusy = true;
	m_flForcedMaxTime = flMaxTime;
	m_bTeleportToBusy = bTeleportToBusy;
	m_bUseNearestBusy = bUseNearestBusy;
	m_ForcedActivity = activity;
	m_hSeeEntity = pSeeEntity;

	if ( pHintNode )
	{
		if ( GetHintNode() && GetHintNode() != pHintNode )
		{
			GetHintNode()->Unlock();
		}

		if ( pHintNode->Lock( GetOuter() ) )
		{
			SetHintNode( pHintNode );
		}
	}

	SetCondition( COND_PROVOKED );
}

//-----------------------------------------------------------------------------
// Purpose: Force the NPC to find an exit node and leave
//-----------------------------------------------------------------------------
void CAI_ActBusyBehavior::ForceActBusyLeave( bool bVisibleOnly )
{
	if ( ai_debug_actbusy.GetInt() == 4 )
	{
		Msg("ACTBUSY: ForceActBusyLeave on NPC %s (%s)\n", GetOuter()->GetClassname(), GetOuter()->GetDebugName() );
	}

	Enable( NULL, m_flBusySearchRange, bVisibleOnly );
	m_bForceActBusy = true;
	m_bLeaving = true;
	m_ForcedActivity = ACT_INVALID;
	m_hSeeEntity = NULL;

	SetCondition( COND_PROVOKED );
}

//-----------------------------------------------------------------------------
// Purpose: Break the NPC out of the current busy state, but don't disable busying
//-----------------------------------------------------------------------------
void CAI_ActBusyBehavior::StopBusying( void )
{
	if ( !GetOuter() )
		return;

	if ( ai_debug_actbusy.GetInt() == 4 )
	{
		Msg("ACTBUSY: StopBusying on NPC %s (%s)\n", GetOuter()->GetClassname(), GetOuter()->GetDebugName() );
	}

	if ( m_bBusy || m_bMovingToBusy )
	{
		SetCondition( COND_PROVOKED );
	}

	m_flEndBusyAt = gpGlobals->curtime;
	m_bForceActBusy = false;
	m_bTeleportToBusy = false;
	m_bUseNearestBusy = false;
	m_bLeaving = false;
	m_bMovingToBusy = false;
	m_ForcedActivity = ACT_INVALID;
	m_hSeeEntity = NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CAI_ActBusyBehavior::IsStopBusying()
{
	return IsCurSchedule(SCHED_ACTBUSY_STOP_BUSYING);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAI_ActBusyBehavior::FValidateHintType( CAI_Hint *pHint )
{
	if ( pHint->HintType() != HINT_WORLD_WORK_POSITION && pHint->HintType() != HINT_NPC_EXIT_POINT )
		return false;

	// If the node doesn't want to be teleported to, we need to check for clear
	const char *pSequenceOrActivity = STRING(pHint->HintActivityName());
	const char *cSpace = strchr( pSequenceOrActivity, ' ' );
	if ( cSpace )
	{
		if ( !Q_strncmp( cSpace+1, "teleport", 8 ) )
		{
			// Node is a teleport node, so it's good
			return true;
		}
	}

	// Check for clearance
	trace_t tr;
	AI_TraceHull( pHint->GetAbsOrigin(), pHint->GetAbsOrigin(), GetOuter()->WorldAlignMins(), GetOuter()->WorldAlignMaxs(), MASK_SOLID, GetOuter(), COLLISION_GROUP_NONE, &tr );
	if ( tr.fraction == 1.0 )
		return true;

	// Report failures
	if ( ai_debug_actbusy.GetInt() == 3 && GetOuter()->m_debugOverlays & OVERLAY_NPC_SELECTED_BIT )
	{
		NDebugOverlay::Text( pHint->GetAbsOrigin(), "Node isn't clear.", false, 60 );
		NDebugOverlay::Box( pHint->GetAbsOrigin(), GetOuter()->WorldAlignMins(), GetOuter()->WorldAlignMaxs(), 255,0,0, 8, 2.0 );
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAI_ActBusyBehavior::CanSelectSchedule( void )
{
	// Always active when we're busy
	if ( m_bBusy || m_bForceActBusy || m_bNeedsToPlayExitAnim )
		return true;

	if ( !m_bEnabled )
		return false;

	if ( !IsCurScheduleOverridable() )
		return false;

	// Don't select actbusy if we're not going to search for a node anyway
	return ( m_flNextBusySearchTime < gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose: Return true if the current schedule is one that ActBusy is allowed to override
//-----------------------------------------------------------------------------
bool CAI_ActBusyBehavior::IsCurScheduleOverridable( void )
{
	// Only if we're about to idle (or SCHED_NONE to catch newly spawned guys)		
	return ( IsCurSchedule( SCHED_IDLE_STAND ) || IsCurSchedule( SCHED_NONE ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSound - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CAI_ActBusyBehavior::ShouldIgnoreSound( CSound *pSound )
{
	// If we're busy in an actbusy anim that's an ambush, stay mum as long as we can
	if ( m_bBusy )
	{
		busyanim_t *pBusyAnim = g_ActBusyAnimDataSystem.GetBusyAnim( m_iCurrentBusyAnim );

		if( pBusyAnim && pBusyAnim->iBusyInterruptType == BA_INT_ZOMBIESLUMP )
		{
			// Slumped zombies are deaf.
			return true;
		}

		if ( pBusyAnim && ( pBusyAnim->iBusyInterruptType == BA_INT_AMBUSH ) || ( pBusyAnim->iBusyInterruptType == BA_INT_COMBAT ) )
		{
			/*
			// Robin: First version ignored sounds in front of the NPC.
			Vector vecToSound = (pSound->GetSoundReactOrigin() - GetAbsOrigin());
			vecToSound.z = 0;
			VectorNormalize( vecToSound );
			Vector facingDir = GetOuter()->EyeDirection2D();
			if ( DotProduct( vecToSound, facingDir ) > 0 )
				return true;
			*/

			// Ignore sounds that aren't visible
			if ( !GetOuter()->FVisible( pSound->GetSoundReactOrigin() ) )
				return true;
		}
	}

	return BaseClass::ShouldIgnoreSound( pSound );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyBehavior::GatherConditions( void )
{
	BaseClass::GatherConditions();

	// If we have a see entity, make sure we can still see it
	if ( m_hSeeEntity )
	{
		if ( GetOuter()->FInViewCone(m_hSeeEntity) && GetOuter()->QuerySeeEntity(m_hSeeEntity) && GetOuter()->FVisible(m_hSeeEntity) )
		{
			m_fTimeLastSawSeeEntity = gpGlobals->curtime;
			ClearCondition( COND_ACTBUSY_LOST_SEE_ENTITY );
		}
		else
		{
			float fDelta = gpGlobals->curtime - m_fTimeLastSawSeeEntity;
			if( fDelta >= ACTBUSY_SEE_ENTITY_TIMEOUT )
			{
				SetCondition( COND_ACTBUSY_LOST_SEE_ENTITY );
				if ( m_hActBusyGoal )
				{
					m_hActBusyGoal->NPCLostSeeEntity( GetOuter() );
				}
			}
		}
	}
	else
	{
		ClearCondition( COND_ACTBUSY_LOST_SEE_ENTITY );
	}

	// If we're busy, ignore sounds depending on our actbusy break rules
	if ( m_bBusy )
	{
		busyanim_t *pBusyAnim = g_ActBusyAnimDataSystem.GetBusyAnim( m_iCurrentBusyAnim );
		if ( pBusyAnim )
		{
			switch( pBusyAnim->iBusyInterruptType )
			{
			case BA_INT_DANGER:
				break;

			case BA_INT_AMBUSH:
				break;

			case BA_INT_ZOMBIESLUMP:
				{
					ClearCondition( COND_HEAR_PLAYER );
					ClearCondition( COND_SEE_ENEMY );
					ClearCondition( COND_NEW_ENEMY );

					CBasePlayer *pPlayer = UTIL_PlayerByIndex(1);

					if( pPlayer )
					{
						float flDist = pPlayer->GetAbsOrigin().DistTo( GetAbsOrigin() );

						if( flDist <= 60 )
						{
							StopBusying();
						}
					}
				}
				break;

			case BA_INT_COMBAT:
				// Ignore the player unless he shoots at us
				ClearCondition( COND_HEAR_PLAYER );
				ClearCondition( COND_SEE_ENEMY );
				ClearCondition( COND_NEW_ENEMY );
				break;

			case BA_INT_PLAYER:
				// Clear all but player.
				ClearCondition( COND_HEAR_DANGER );
				ClearCondition( COND_HEAR_COMBAT );
				ClearCondition( COND_HEAR_WORLD  );
				ClearCondition( COND_HEAR_BULLET_IMPACT );
				break;

			case BA_INT_NONE:
				// Clear all
				ClearCondition( COND_HEAR_DANGER );
				ClearCondition( COND_HEAR_COMBAT );
				ClearCondition( COND_HEAR_WORLD  );
				ClearCondition( COND_HEAR_BULLET_IMPACT );
				ClearCondition( COND_HEAR_PLAYER );
				break;

			default:
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyBehavior::EndScheduleSelection( void )
{
	NotifyBusyEnding();

	CheckAndCleanupOnExit();
}

//-----------------------------------------------------------------------------
// Purpose: Actbusy's ending, ensure we haven't left NPC in broken state.
//-----------------------------------------------------------------------------
void CAI_ActBusyBehavior::CheckAndCleanupOnExit( void )
{
	if ( m_bNeedsToPlayExitAnim && !GetOuter()->IsMarkedForDeletion() && GetOuter()->IsAlive() )
	{
		Warning("NPC %s(%s) left actbusy without playing exit anim.\n", GetOuter()->GetDebugName(), GetOuter()->GetClassname() );
		m_bNeedsToPlayExitAnim = false;
	}

	GetOuter()->RemoveFlag( FL_FLY );

	// If we're supposed to use render bounds while inside the busy anim, restore normal now
	if ( m_bUseRenderBoundsForCollision )
	{
		GetOuter()->SetHullSizeNormal( true );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyBehavior::BuildScheduleTestBits( void )
{
	BaseClass::BuildScheduleTestBits();

	// When going to an actbusy, we can't be interrupted during the entry anim
	if ( IsCurSchedule(SCHED_ACTBUSY_START_BUSYING) )
	{
		if ( GetOuter()->GetTask()->iTask == TASK_ACTBUSY_PLAY_ENTRY )
			return;

		GetOuter()->SetCustomInterruptCondition( COND_PROVOKED );
	}

	// If we're in a queue, or leaving, we have no extra conditions
	if ( m_bInQueue || IsCurSchedule( SCHED_ACTBUSY_LEAVE ) )
		return;

	// If we're not busy, or we're exiting a busy, we have no extra conditions
	if ( !m_bBusy || IsCurSchedule( SCHED_ACTBUSY_STOP_BUSYING ) )
		return;

	busyanim_t *pBusyAnim = g_ActBusyAnimDataSystem.GetBusyAnim( m_iCurrentBusyAnim );
	if ( pBusyAnim )
	{
		switch( pBusyAnim->iBusyInterruptType )
		{
			case BA_INT_ZOMBIESLUMP:
			{
				GetOuter()->SetCustomInterruptCondition( COND_LIGHT_DAMAGE );
				GetOuter()->SetCustomInterruptCondition( COND_HEAVY_DAMAGE );
			}
			break;

			case BA_INT_AMBUSH:
			case BA_INT_DANGER:
			{
				GetOuter()->SetCustomInterruptCondition( COND_LIGHT_DAMAGE );
				GetOuter()->SetCustomInterruptCondition( COND_HEAVY_DAMAGE );
				GetOuter()->SetCustomInterruptCondition( COND_HEAR_DANGER );
				GetOuter()->SetCustomInterruptCondition( COND_HEAR_COMBAT );
				GetOuter()->SetCustomInterruptCondition( COND_HEAR_BULLET_IMPACT );
				GetOuter()->SetCustomInterruptCondition( COND_NEW_ENEMY );
				GetOuter()->SetCustomInterruptCondition( COND_SEE_ENEMY );
				GetOuter()->SetCustomInterruptCondition( COND_PLAYER_ADDED_TO_SQUAD );
				GetOuter()->SetCustomInterruptCondition( COND_RECEIVED_ORDERS );
				break;
			}

			case BA_INT_PLAYER:
			{
				GetOuter()->SetCustomInterruptCondition( COND_LIGHT_DAMAGE );
				GetOuter()->SetCustomInterruptCondition( COND_HEAVY_DAMAGE );
				GetOuter()->SetCustomInterruptCondition( COND_HEAR_DANGER );
				GetOuter()->SetCustomInterruptCondition( COND_HEAR_COMBAT );
				GetOuter()->SetCustomInterruptCondition( COND_HEAR_BULLET_IMPACT );
				GetOuter()->SetCustomInterruptCondition( COND_NEW_ENEMY );
				GetOuter()->SetCustomInterruptCondition( COND_PLAYER_ADDED_TO_SQUAD );
				GetOuter()->SetCustomInterruptCondition( COND_RECEIVED_ORDERS );

				// The player can interrupt us
				GetOuter()->SetCustomInterruptCondition( COND_SEE_PLAYER );
				break;
			}

			case BA_INT_COMBAT:
			{
				GetOuter()->SetCustomInterruptCondition( COND_LIGHT_DAMAGE );
				GetOuter()->SetCustomInterruptCondition( COND_HEAVY_DAMAGE );
				GetOuter()->SetCustomInterruptCondition( COND_HEAR_DANGER );
				GetOuter()->SetCustomInterruptCondition( COND_HEAR_COMBAT );
				GetOuter()->SetCustomInterruptCondition( COND_HEAR_BULLET_IMPACT );
				break;
			}

			case BA_INT_NONE:
				break;

			default:
				break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CAI_ActBusyBehavior::SelectScheduleForLeaving( void )
{
	// Are we already near an exit node?
	if ( GetHintNode() )
	{
		if ( GetHintNode()->HintType() == HINT_NPC_EXIT_POINT )
		{
			// Are we near it? If so, we're done. If not, move to it.
			if ( UTIL_DistApprox( GetHintNode()->GetAbsOrigin(), GetAbsOrigin() ) < 64 )
			{
				if ( !GetOuter()->IsMarkedForDeletion() )
				{
					CBaseEntity *pOwner = GetOuter()->GetOwnerEntity();
					if ( pOwner )
					{
						pOwner->DeathNotice( GetOuter() );
						GetOuter()->SetOwnerEntity( NULL );
					}
					GetOuter()->SetThink( &CBaseEntity::SUB_Remove); //SUB_Remove) ; //GetOuter()->SUB_Remove );
					GetOuter()->SetNextThink( gpGlobals->curtime + 0.1 );

					if ( m_hActBusyGoal )
					{
						m_hActBusyGoal->NPCLeft( GetOuter() );
					}
				}

				return SCHED_IDLE_STAND;
			}

			return SCHED_ACTBUSY_LEAVE;
		}
		else
		{
			// Clear the node, it's no use to us
			GetHintNode()->NPCStoppedUsing( GetOuter() );
			GetHintNode()->Unlock();
			SetHintNode( NULL );
		}
	}

	// Find an exit node
	CHintCriteria	hintCriteria;
	hintCriteria.SetHintType( HINT_NPC_EXIT_POINT );
	hintCriteria.SetFlag( bits_HINT_NODE_RANDOM | bits_HINT_NODE_CLEAR | bits_HINT_NODE_USE_GROUP );
	CAI_Hint *pNode = CAI_HintManager::FindHintRandom( GetOuter(), GetOuter()->GetAbsOrigin(), hintCriteria );
	if ( pNode )
	{
		SetHintNode( pNode );
		return SCHED_ACTBUSY_LEAVE;
	}

	// We've been told to leave, but we can't find an exit node. What to do?
	return SCHED_IDLE_STAND;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CAI_ActBusyBehavior::SelectScheduleWhileNotBusy( int iBase )
{
	// Randomly act busy (unless we're being forced, in which case we should search immediately)
	if ( m_bForceActBusy || m_flNextBusySearchTime < gpGlobals->curtime )
	{
		// If we're being forced, think again quickly
		if ( m_bForceActBusy )
		{
			m_flNextBusySearchTime = gpGlobals->curtime + 2.0;
		}
		else
		{
			m_flNextBusySearchTime = gpGlobals->curtime + RandomFloat(ai_actbusy_search_time.GetFloat(), ai_actbusy_search_time.GetFloat()*2);
		}

		// We may already have a node
		CAI_Hint *pNode = GetHintNode();
		if ( !pNode )
		{
			int iBits = bits_HINT_NODE_USE_GROUP;
			if ( m_bVisibleOnly )
			{
				iBits |= bits_HINT_NODE_VISIBLE;
			}

			if ( ai_debug_actbusy.GetInt() == 3 && GetOuter()->m_debugOverlays & OVERLAY_NPC_SELECTED_BIT )
			{
				iBits |= bits_HINT_NODE_REPORT_FAILURES;
			}

			if ( m_bUseNearestBusy )
			{
				iBits |= bits_HINT_NODE_NEAREST;
			}
			else
			{
				iBits |= bits_HINT_NODE_RANDOM;
			}

			pNode = CAI_HintManager::FindHint( GetOuter(), HINT_WORLD_WORK_POSITION, iBits, m_flBusySearchRange );
		}
		if ( pNode )
		{
			// Ensure we've got a sequence for the node
			const char *pSequenceOrActivity = STRING(pNode->HintActivityName());
			Activity iNodeActivity;
			int iBusyAnim;

			// See if the node specifies that we should teleport to it
			const char *cSpace = strchr( pSequenceOrActivity, ' ' );
			if ( cSpace )
			{
				if ( !Q_strncmp( cSpace+1, "teleport", 8 ) )
				{
					m_bTeleportToBusy = true;
				}

				char sActOrSeqName[512];
				Q_strncpy( sActOrSeqName, pSequenceOrActivity, (cSpace-pSequenceOrActivity)+1 );
				iNodeActivity = (Activity)CAI_BaseNPC::GetActivityID( sActOrSeqName ); 
				iBusyAnim = g_ActBusyAnimDataSystem.FindBusyAnim( iNodeActivity, sActOrSeqName );
			}
			else
			{
				iNodeActivity = (Activity)CAI_BaseNPC::GetActivityID( pSequenceOrActivity ); 
				iBusyAnim = g_ActBusyAnimDataSystem.FindBusyAnim( iNodeActivity, pSequenceOrActivity );
			}

			// Does this NPC have the activity or sequence for this node?
			if ( HasAnimForActBusy( iBusyAnim, BA_BUSY ) )
			{
				if ( HasCondition(COND_ACTBUSY_LOST_SEE_ENTITY) )
				{
					// We've lost our see entity, which means we can't continue.
					if ( m_bForceActBusy )
					{
						// We were being told to act busy, which we can't do now that we've lost the see entity.
						// Abort, and assume that the mapmaker will make us retry.
						StopBusying();
					}
					return iBase;
				}

				m_iCurrentBusyAnim = iBusyAnim;
				if ( m_iCurrentBusyAnim == -1 )
					return iBase;

				if ( ai_debug_actbusy.GetInt() == 4 )
				{
					Msg("ACTBUSY: NPC %s (%s) found Actbusy node %s \n", GetOuter()->GetClassname(), GetOuter()->GetDebugName(), pNode->GetDebugName() );
				}

				if ( GetHintNode() )
				{
					GetHintNode()->Unlock();
				}

				SetHintNode( pNode );
				if ( GetHintNode() && GetHintNode()->Lock( GetOuter() ) )
				{
					if ( ai_debug_actbusy.GetInt() == 2 )
					{
						// Show which actbusy we're moving towards
						NDebugOverlay::Line( GetOuter()->WorldSpaceCenter(), pNode->GetAbsOrigin(), 0, 255, 0, true, 5.0 );
						NDebugOverlay::Box( pNode->GetAbsOrigin(), GetOuter()->WorldAlignMins(), GetOuter()->WorldAlignMaxs(), 0, 255, 0, 64, 5.0 );
					}

					// Let our act busy know we're moving to a node
					if ( m_hActBusyGoal )
					{
						m_hActBusyGoal->NPCMovingToBusy( GetOuter() );
					}

					m_bMovingToBusy = true;

					// If we're supposed to teleport, do that instead
					if ( m_bTeleportToBusy )
						return SCHED_ACTBUSY_TELEPORT_TO_BUSY;

					return SCHED_ACTBUSY_START_BUSYING;
				}
			}
		}
	}

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CAI_ActBusyBehavior::SelectScheduleWhileBusy( void )
{
	// Are we supposed to stop on our current actbusy, but stay in the actbusy state?
	if ( !ActBusyNodeStillActive() || (m_flEndBusyAt && gpGlobals->curtime >= m_flEndBusyAt) )
	{
		if ( ai_debug_actbusy.GetInt() == 4 )
		{
			Msg("ACTBUSY: NPC %s (%s) ending actbusy.\n", GetOuter()->GetClassname(), GetOuter()->GetDebugName() );
		}

		StopBusying();
		return SCHED_ACTBUSY_STOP_BUSYING;
	}

	return SCHED_ACTBUSY_BUSY;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CAI_ActBusyBehavior::SelectSchedule()
{
	int iBase = BaseClass::SelectSchedule();

	// Only do something if the base ai doesn't want to do anything
	if ( !m_bForceActBusy && iBase != SCHED_IDLE_STAND )
	{
		// If we're busy, we need to get out of it first
		if ( m_bBusy )
			return SCHED_ACTBUSY_STOP_BUSYING;

		CheckAndCleanupOnExit();
		return iBase;
	}

	// If we're supposed to be leaving, find a leave node and exit
	if ( m_bLeaving )
		return SelectScheduleForLeaving();

	// NPCs should not be busy if the actbusy behaviour has been disabled, or if they've received player squad commands
	bool bShouldNotBeBusy = (!m_bEnabled || HasCondition( COND_PLAYER_ADDED_TO_SQUAD ) || HasCondition( COND_RECEIVED_ORDERS ));
	if ( bShouldNotBeBusy )
	{
		if ( !GetOuter()->IsMarkedForDeletion() && GetOuter()->IsAlive() )
			return SCHED_ACTBUSY_STOP_BUSYING;
	}
	else
	{
		if ( m_bBusy )
			return SelectScheduleWhileBusy();

		// I'm not busy, and I'm supposed to be
		int schedule = SelectScheduleWhileNotBusy( iBase );
		if ( schedule != SCHED_NONE )
			return schedule;
	}

	CheckAndCleanupOnExit();
	return iBase;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAI_ActBusyBehavior::ActBusyNodeStillActive( void )
{
	if ( !GetHintNode() )
		return false;
	
	return ( GetHintNode()->IsDisabled() == false );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CAI_ActBusyBehavior::IsInterruptable( void )
{
	if ( IsActive() )
		return false;

	return BaseClass::IsInterruptable();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CAI_ActBusyBehavior::CanFlinch( void )
{
	if ( m_bNeedsToPlayExitAnim )
		return false;

	return BaseClass::CanFlinch();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAI_ActBusyBehavior::CanRunAScriptedNPCInteraction( bool bForced )
{
	// Prevent interactions during actbusy modes
	if ( IsActive() )
		return false;

	return BaseClass::CanRunAScriptedNPCInteraction( bForced );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyBehavior::OnScheduleChange()
{
	if( IsCurSchedule(SCHED_ACTBUSY_BUSY, false) )
	{
		if( HasCondition(COND_SEE_ENEMY) )
		{
			m_bExitedBusyToDueSeeEnemy = true;
		}

		if( HasCondition(COND_ACTBUSY_LOST_SEE_ENTITY) )
		{
			m_bExitedBusyToDueLostSeeEntity = true;
		}
	}

	BaseClass::OnScheduleChange();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAI_ActBusyBehavior::QueryHearSound( CSound *pSound )
{
	// Ignore friendly created combat sounds while in an actbusy.
	// Fixes friendly NPCs going in & out of actbusies when the 
	// player fires shots at their feet.
	if ( pSound->IsSoundType( SOUND_COMBAT ) || pSound->IsSoundType( SOUND_BULLET_IMPACT ) )
	{
		if ( GetOuter()->IRelationType( pSound->m_hOwner ) == D_LI )
			return false;
	}

	return BaseClass::QueryHearSound( pSound );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CAI_ActBusyBehavior::ShouldPlayerAvoid( void )
{
	if ( IsCurSchedule ( SCHED_ACTBUSY_START_BUSYING ) )
	{
		if ( GetCurTask() && GetCurTask()->iTask == TASK_WAIT_FOR_MOVEMENT )
			 return true;
	}

	return BaseClass::ShouldPlayerAvoid();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_ActBusyBehavior::ComputeAndSetRenderBounds()
{
	Vector mins, maxs;
	if ( GetOuter()->ComputeHitboxSurroundingBox( &mins, &maxs ) )
	{
		UTIL_SetSize( GetOuter(), mins - GetAbsOrigin(), maxs - GetAbsOrigin());
		if ( GetOuter()->VPhysicsGetObject() )
		{
			GetOuter()->SetupVPhysicsHull();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the current NPC is acting busy, or moving to an actbusy
//-----------------------------------------------------------------------------
bool CAI_ActBusyBehavior::IsActive( void )
{
	return ( m_bBusy || m_bForceActBusy || m_bNeedsToPlayExitAnim || m_bLeaving );
}

//-----------------------------------------------------------------------------
// Purpose: Return true if this NPC has the anims required to use the specified actbusy hint
//-----------------------------------------------------------------------------
bool CAI_ActBusyBehavior::HasAnimForActBusy( int iActBusy, busyanimparts_t AnimPart )
{
	if ( iActBusy == -1 )
		return false;

	busyanim_t *pBusyAnim = g_ActBusyAnimDataSystem.GetBusyAnim( iActBusy );
	if ( !pBusyAnim )
		return false;

	// Try and play the sequence first
	if ( pBusyAnim->iszSequences[AnimPart] != NULL_STRING )
		return (GetOuter()->LookupSequence( (char*)STRING(pBusyAnim->iszSequences[AnimPart]) ) != ACTIVITY_NOT_AVAILABLE);

	// Try and play the activity second
	if ( pBusyAnim->iActivities[AnimPart] != ACT_INVALID )
		return GetOuter()->HaveSequenceForActivity( pBusyAnim->iActivities[AnimPart] );

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Play the sound associated with the specified part of the current actbusy, if any
//-----------------------------------------------------------------------------
void CAI_ActBusyBehavior::PlaySoundForActBusy( busyanimparts_t AnimPart )
{
	busyanim_t *pBusyAnim = g_ActBusyAnimDataSystem.GetBusyAnim( m_iCurrentBusyAnim );
	if ( !pBusyAnim )
		return;

	// Play the sound
	if ( pBusyAnim->iszSounds[AnimPart] != NULL_STRING )
	{
		// See if we can treat it as a game sound name
		CSoundParameters params;
		if ( GetOuter()->GetParametersForSound( STRING(pBusyAnim->iszSounds[AnimPart]), params, STRING(GetOuter()->GetModelName()) ) )
		{
			CPASAttenuationFilter filter( GetOuter() );
			GetOuter()->EmitSound( filter, GetOuter()->entindex(), params );
		}
		else
		{
			// Assume it's a response concept, and try to speak it
			CAI_Expresser *pExpresser = GetOuter()->GetExpresser();
			if ( pExpresser )
			{
				const char *concept = STRING(pBusyAnim->iszSounds[AnimPart]);

				// Must be able to speak the concept
				if ( !pExpresser->IsSpeaking() && pExpresser->CanSpeakConcept( concept ) )
				{
					pExpresser->Speak( concept );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Play a sequence or activity for the current actbusy
//-----------------------------------------------------------------------------
bool CAI_ActBusyBehavior::PlayAnimForActBusy( busyanimparts_t AnimPart )
{
	busyanim_t *pBusyAnim = g_ActBusyAnimDataSystem.GetBusyAnim( m_iCurrentBusyAnim );
	if ( !pBusyAnim )
		return false;

	// Try and play the sequence first
	if ( pBusyAnim->iszSequences[AnimPart] != NULL_STRING )
	{
		GetOuter()->SetSequenceByName( (char*)STRING(pBusyAnim->iszSequences[AnimPart]) );
		GetOuter()->SetIdealActivity( ACT_DO_NOT_DISTURB );
		return true;
	}

	// Try and play the activity second
	if ( pBusyAnim->iActivities[AnimPart] != ACT_INVALID )
	{
		GetOuter()->SetIdealActivity( pBusyAnim->iActivities[AnimPart] );
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyBehavior::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_ACTBUSY_PLAY_BUSY_ANIM:
		{
			// If we're not enabled here, it's due to the actbusy being deactivated during
			// the NPC's entry animation. We can't abort in the middle of the entry, so we
			// arrive here with a disabled actbusy behaviour. Exit gracefully.
			if ( !m_bEnabled )
			{
				TaskComplete();
				return;
			}

			// Set the flag to remind the code to recompute the NPC's box from render bounds.
			// This is used to delay the process so that we don't get a box built from render bounds
			// when a character is still interpolating to their busy pose.
			m_bNeedToSetBounds = true;

			// Get the busyanim for the specified activity
			busyanim_t *pBusyAnim = g_ActBusyAnimDataSystem.GetBusyAnim( m_iCurrentBusyAnim );

			// We start "flying" so we don't collide with the world, in case the level
			// designer has us sitting on a chair, etc.
			if( !pBusyAnim || !pBusyAnim->bUseAutomovement )
			{
				GetOuter()->AddFlag( FL_FLY );
			}

			GetOuter()->SetGroundEntity( NULL );

			// Fail if we're not on the node & facing the correct way
			// We only do this check if we're still moving to the busy. This will only
			// be true if there was no entry animation for this busy. We do it this way
			// because the entry code contains this same check, and so we assume we're
			// valid even if we're off now, because some entry animations move the 
			// character off the node.
			if ( m_bMovingToBusy )
			{
				if ( UTIL_DistApprox( GetHintNode()->GetAbsOrigin(), GetAbsOrigin() ) > 16 || !GetOuter()->FacingIdeal() )
				{
					TaskFail( "Not correctly on hintnode" );
					m_flEndBusyAt = gpGlobals->curtime;
					return;
				}
			}

			m_bMovingToBusy = false;

			if ( !ActBusyNodeStillActive() )
			{
				TaskFail( FAIL_NO_HINT_NODE );
				return;
			}

			// Have we just started using this node?
			if ( !m_bBusy )
			{
				m_bBusy = true;

				GetHintNode()->NPCStartedUsing( GetOuter() );
				if ( m_hActBusyGoal )
				{
					m_hActBusyGoal->NPCStartedBusy( GetOuter() );
				}

				if ( pBusyAnim )
				{
					float flMaxTime = pBusyAnim->flMaxTime;
					float flMinTime = pBusyAnim->flMinTime;

					// Mapmaker input may have specified it's own max time
					if ( m_bForceActBusy && m_flForcedMaxTime != NO_MAX_TIME )
					{
						flMaxTime = m_flForcedMaxTime;

						// Don't let non-unlimited time amounts be less than the mintime
						if ( flMaxTime && flMaxTime < flMinTime )
						{
							flMinTime = flMaxTime;
						}
					}

					// If we have no max time, or we're in a queue, we loop forever.
					if ( !flMaxTime || m_bInQueue )
					{
						m_flEndBusyAt = 0;
						GetOuter()->SetWait( 99999 );
					}
					else
					{
						float flTime = RandomFloat(flMinTime, flMaxTime);
						m_flEndBusyAt = gpGlobals->curtime + flTime;
						GetOuter()->SetWait( flTime );
					}
				}
			}

			// Start playing the act busy
			PlayAnimForActBusy( BA_BUSY );
			PlaySoundForActBusy( BA_BUSY );

			// Now that we're busy, we don't need to be forced anymore
			m_bForceActBusy = false;
			m_bTeleportToBusy = false;
			m_bUseNearestBusy = false;
			m_ForcedActivity = ACT_INVALID;

			// If we're supposed to use render bounds while inside the busy anim, do so
			if ( m_bUseRenderBoundsForCollision )
			{
				ComputeAndSetRenderBounds();
			}
		}
		break;

	case TASK_ACTBUSY_PLAY_ENTRY:
		{
			// We start "flying" so we don't collide with the world, in case the level
			// designer has us sitting on a chair, etc.

			// Get the busyanim for the specified activity
			busyanim_t *pBusyAnim = g_ActBusyAnimDataSystem.GetBusyAnim( m_iCurrentBusyAnim );

			// We start "flying" so we don't collide with the world, in case the level
			// designer has us sitting on a chair, etc.
			if( !pBusyAnim || !pBusyAnim->bUseAutomovement )
			{
				GetOuter()->AddFlag( FL_FLY );
			}

			GetOuter()->SetGroundEntity( NULL );

			m_bMovingToBusy = false;
			m_bNeedsToPlayExitAnim = HasAnimForActBusy( m_iCurrentBusyAnim, BA_EXIT );

			if ( !ActBusyNodeStillActive() )
			{
				TaskFail( FAIL_NO_HINT_NODE );
				return;
			}

			// Fail if we're not on the node & facing the correct way
			if ( UTIL_DistApprox( GetHintNode()->GetAbsOrigin(), GetAbsOrigin() ) > 16 || !GetOuter()->FacingIdeal() )
			{
				m_bBusy = false;
				TaskFail( "Not correctly on hintnode" );
				return;
			}

			PlaySoundForActBusy( BA_ENTRY );

			// Play the entry animation. If it fails, we don't have an entry anim, so complete immediately.
			if ( !PlayAnimForActBusy( BA_ENTRY ) )
			{
				TaskComplete();
			}
		}
		break;

	case TASK_ACTBUSY_VERIFY_EXIT:
		{
			// NPC's that changed their bounding box must ensure that they can restore their regular box
			// before they exit their actbusy. This task is designed to delay until that time if necessary.
			if( !m_bUseRenderBoundsForCollision )
			{
				// Don't bother if we didn't alter our BBox. 
				TaskComplete();
				break;
			}

			// Set up a timer to check immediately.
			GetOuter()->SetWait( 0 );			
		}
		break;

	case TASK_ACTBUSY_PLAY_EXIT:
		{
			// If we're supposed to use render bounds while inside the busy anim, restore normal now
			if ( m_bUseRenderBoundsForCollision )
			{
				GetOuter()->SetHullSizeNormal( true );
			}

			if ( m_hActBusyGoal )
			{
				m_hActBusyGoal->NPCStartedLeavingBusy( GetOuter() );
			}

			PlaySoundForActBusy( BA_EXIT );

			// Play the exit animation. If it fails, we don't have an entry anim, so complete immediately.
			if ( !PlayAnimForActBusy( BA_EXIT ) )
			{
				m_bNeedsToPlayExitAnim = false;
				GetOuter()->RemoveFlag( FL_FLY );
				NotifyBusyEnding();
				TaskComplete();
			}
		}
		break;

	case TASK_ACTBUSY_TELEPORT_TO_BUSY:
		{
			if ( !ActBusyNodeStillActive() )
			{
				TaskFail( FAIL_NO_HINT_NODE );
				return;
			}

			Vector vecAbsOrigin = GetHintNode()->GetAbsOrigin();
			QAngle vecAbsAngles = GetHintNode()->GetAbsAngles();
			GetOuter()->Teleport( &vecAbsOrigin, &vecAbsAngles, NULL );
			GetOuter()->GetMotor()->SetIdealYaw( vecAbsAngles.y );

			TaskComplete();
		}
		break;

	case TASK_ACTBUSY_WALK_PATH_TO_BUSY:
		{
			// If we have a forced activity, use that. Otherwise, walk.
			if ( m_ForcedActivity != ACT_INVALID && m_ForcedActivity != ACT_RESET )
			{
				GetNavigator()->SetMovementActivity( m_ForcedActivity );

				// Cover is void once I move
				Forget( bits_MEMORY_INCOVER );

				TaskComplete();
			}
			else
			{
				ChainStartTask( TASK_WALK_PATH );
			}
			break;
		}

	case TASK_ACTBUSY_GET_PATH_TO_ACTBUSY:
		{
			ChainStartTask( TASK_GET_PATH_TO_HINTNODE );

			if ( !HasCondition(COND_TASK_FAILED) )
			{
				// Set the arrival sequence for the actbusy to be the busy sequence, if we don't have an entry animation
				busyanim_t *pBusyAnim = g_ActBusyAnimDataSystem.GetBusyAnim( m_iCurrentBusyAnim );
				if ( pBusyAnim && pBusyAnim->iszSequences[BA_ENTRY] == NULL_STRING && pBusyAnim->iActivities[BA_ENTRY] == ACT_INVALID )
				{
					// Try and play the sequence first
					if ( pBusyAnim->iszSequences[BA_BUSY] != NULL_STRING )
					{
						GetNavigator()->SetArrivalSequence( GetOuter()->LookupSequence( STRING(pBusyAnim->iszSequences[BA_BUSY]) ) );
					}
					else if ( pBusyAnim->iActivities[BA_BUSY] != ACT_INVALID )
					{
						// Try and play the activity second
						GetNavigator()->SetArrivalActivity( pBusyAnim->iActivities[BA_BUSY] );
					}
				}
				else
				{
					// Robin: Set the arrival sequence / activity to be the entry animation.
					if ( pBusyAnim->iszSequences[BA_ENTRY] != NULL_STRING )
					{
						GetNavigator()->SetArrivalSequence( GetOuter()->LookupSequence( STRING(pBusyAnim->iszSequences[BA_ENTRY]) ) );
					}
					else if ( pBusyAnim->iActivities[BA_ENTRY] != ACT_INVALID )
					{
						// Try and play the activity second
						GetNavigator()->SetArrivalActivity( pBusyAnim->iActivities[BA_ENTRY] );
					}
				}
			}
			else
			{
				if ( ai_debug_actbusy.GetInt() == 1 )
				{
					if ( GetHintNode() )
					{
						// Show which actbusy we're moving towards
						NDebugOverlay::Line( GetOuter()->WorldSpaceCenter(), GetHintNode()->GetAbsOrigin(), 255, 0, 0, true, 1.0 );
					}
				}
			}
			break;
		}

	default:
		BaseClass::StartTask( pTask);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyBehavior::RunTask( const Task_t *pTask )		
{ 
	switch ( pTask->iTask )
	{
	case TASK_WAIT_FOR_MOVEMENT:
		{
			// Ensure the hint node hasn't been disabled
			if ( IsCurSchedule( SCHED_ACTBUSY_START_BUSYING ) )
			{
				if ( !ActBusyNodeStillActive() )
				{
					TaskFail(FAIL_NO_HINT_NODE);
					return;
				}
			}

			if ( ai_debug_actbusy.GetInt() == 1 )
			{
				if ( GetHintNode() )
				{
					// Show which actbusy we're moving towards
					NDebugOverlay::Line( GetOuter()->WorldSpaceCenter(), GetHintNode()->GetAbsOrigin(), 0, 255, 0, true, 0.2 );
				}
			}

			BaseClass::RunTask( pTask );
			break;
		}

	case TASK_ACTBUSY_PLAY_BUSY_ANIM:
		{
			if( m_bUseRenderBoundsForCollision )
			{
				if( GetOuter()->IsSequenceFinished() && m_bNeedToSetBounds )
				{
					ComputeAndSetRenderBounds();
					m_bNeedToSetBounds = false;
				}
			}

			GetOuter()->AutoMovement();
			// Stop if the node's been disabled
			if ( !ActBusyNodeStillActive() || GetOuter()->IsWaitFinished() )
			{
				TaskComplete();
			}
			else
			{
				CAI_PlayerAlly *pAlly = dynamic_cast<CAI_PlayerAlly*>(GetOuter());
				if ( pAlly )
				{
					pAlly->SelectInterjection();
				}

				if( HasCondition(COND_ACTBUSY_LOST_SEE_ENTITY) )
				{
					StopBusying();
					TaskComplete();
				}
			}
			break;
		}

	case TASK_ACTBUSY_PLAY_ENTRY:
		{
			GetOuter()->AutoMovement();
			if ( !ActBusyNodeStillActive() || GetOuter()->IsSequenceFinished() )
			{
				TaskComplete();
			}
		}
		break;

	case TASK_ACTBUSY_VERIFY_EXIT:
		{
			if( GetOuter()->IsWaitFinished() )
			{
				// Trace my normal hull over this spot to see if I'm able to stand up right now.
				trace_t tr;
				CTraceFilterOnlyNPCsAndPlayer filter( GetOuter(), COLLISION_GROUP_NONE );
				UTIL_TraceHull( GetOuter()->GetAbsOrigin(), GetOuter()->GetAbsOrigin(), NAI_Hull::Mins( HULL_HUMAN ), NAI_Hull::Maxs( HULL_HUMAN ), MASK_NPCSOLID, &filter, &tr );

				if( tr.startsolid )
				{
					// Blocked. Try again later.
					GetOuter()->SetWait( 1.0f );
				}
				else
				{
					// Put an entity blocker here for a moment until I get into my bounding box.
					CBaseEntity *pBlocker = CEntityBlocker::Create( GetOuter()->GetAbsOrigin(), NAI_Hull::Mins( HULL_HUMAN ), NAI_Hull::Maxs( HULL_HUMAN ), GetOuter(), true );
					g_EventQueue.AddEvent( pBlocker, "Kill", 1.0, GetOuter(), GetOuter() );
					TaskComplete();
				}
			}
		}
		break;

	case TASK_ACTBUSY_PLAY_EXIT:
		{
			GetOuter()->AutoMovement();
			if ( GetOuter()->IsSequenceFinished() )
			{
				m_bNeedsToPlayExitAnim = false;
				GetOuter()->RemoveFlag( FL_FLY );
				NotifyBusyEnding();
				TaskComplete();
			}
		}
		break;

	default:
		BaseClass::RunTask( pTask);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyBehavior::NotifyBusyEnding( void )
{
	// Clear the hintnode if we're done with it
	if ( GetHintNode() )
	{
		if ( m_bBusy || m_bMovingToBusy )
		{
			GetHintNode()->NPCStoppedUsing( GetOuter() );
		}

		GetHintNode()->Unlock();
		SetHintNode( NULL );
	}

	// Then, if we were busy, stop being busy
 	if ( m_bBusy )
	{
		m_bBusy = false;

		if ( m_hActBusyGoal )
		{
			m_hActBusyGoal->NPCFinishedBusy( GetOuter() );

			if ( m_bExitedBusyToDueLostSeeEntity )
			{
				m_hActBusyGoal->NPCLostSeeEntity( GetOuter() );
				m_bExitedBusyToDueLostSeeEntity = false;
			}

			if ( m_bExitedBusyToDueSeeEnemy )
			{
				m_hActBusyGoal->NPCSeeEnemy( GetOuter() );
				m_bExitedBusyToDueSeeEnemy = false;
			}
		}
	}
	else if ( m_bMovingToBusy && m_hActBusyGoal )
	{
		// Or if we were just on our way to be busy, let the goal know
		m_hActBusyGoal->NPCAbortedMoveTo( GetOuter() );
	}

	// Don't busy again for a while
	m_flEndBusyAt = 0;
	m_flNextBusySearchTime = gpGlobals->curtime + (RandomFloat(ai_actbusy_search_time.GetFloat(), ai_actbusy_search_time.GetFloat()*2));
}

//-------------------------------------

AI_BEGIN_CUSTOM_SCHEDULE_PROVIDER( CAI_ActBusyBehavior )

	DECLARE_CONDITION( COND_ACTBUSY_LOST_SEE_ENTITY )

	DECLARE_TASK( TASK_ACTBUSY_PLAY_BUSY_ANIM )
	DECLARE_TASK( TASK_ACTBUSY_PLAY_ENTRY )
	DECLARE_TASK( TASK_ACTBUSY_PLAY_EXIT )
	DECLARE_TASK( TASK_ACTBUSY_TELEPORT_TO_BUSY )
	DECLARE_TASK( TASK_ACTBUSY_WALK_PATH_TO_BUSY )
	DECLARE_TASK( TASK_ACTBUSY_GET_PATH_TO_ACTBUSY )
	DECLARE_TASK( TASK_ACTBUSY_VERIFY_EXIT )

	//---------------------------------

	DEFINE_SCHEDULE
	( 
		SCHED_ACTBUSY_START_BUSYING,

		"	Tasks"
		"		TASK_SET_TOLERANCE_DISTANCE			4"
		"		TASK_ACTBUSY_GET_PATH_TO_ACTBUSY	0"
		"		TASK_ACTBUSY_WALK_PATH_TO_BUSY		0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_STOP_MOVING					0"
		"		TASK_FACE_HINTNODE					0"
		"		TASK_ACTBUSY_PLAY_ENTRY				0"
		"		TASK_SET_SCHEDULE					SCHEDULE:SCHED_ACTBUSY_BUSY"
		""
		"	Interrupts"
		"		COND_ACTBUSY_LOST_SEE_ENTITY"
	)

	DEFINE_SCHEDULE
	( 
		SCHED_ACTBUSY_BUSY,

		"	Tasks"
		"		TASK_ACTBUSY_PLAY_BUSY_ANIM		0"
		""
		"	Interrupts"
		"		COND_PROVOKED"
	)

	DEFINE_SCHEDULE
	( 
		SCHED_ACTBUSY_STOP_BUSYING,

		"	Tasks"
		"		TASK_ACTBUSY_VERIFY_EXIT		0"
		"		TASK_ACTBUSY_PLAY_EXIT			0"
		""
		"	Interrupts"
	)

	DEFINE_SCHEDULE
	( 
		SCHED_ACTBUSY_LEAVE,

		"	Tasks"
		"		TASK_SET_TOLERANCE_DISTANCE			4"
		"		TASK_ACTBUSY_GET_PATH_TO_ACTBUSY	0"
		"		TASK_ACTBUSY_WALK_PATH_TO_BUSY		0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		""
		"	Interrupts"
		"		COND_PROVOKED"
	)

	DEFINE_SCHEDULE
	( 
		SCHED_ACTBUSY_TELEPORT_TO_BUSY,

		"	Tasks"
		"		TASK_ACTBUSY_TELEPORT_TO_BUSY	0"
		"		TASK_ACTBUSY_PLAY_ENTRY			0"
		"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_ACTBUSY_BUSY"
		""
		"	Interrupts"
		"		COND_PROVOKED"
	)

AI_END_CUSTOM_SCHEDULE_PROVIDER()


//==========================================================================================================
// ACT BUSY GOALS
//==========================================================================================================
//-----------------------------------------------------------------------------
// Purpose: A level tool to control the actbusy behavior.
//-----------------------------------------------------------------------------
LINK_ENTITY_TO_CLASS( ai_goal_actbusy, CAI_ActBusyGoal );

BEGIN_DATADESC( CAI_ActBusyGoal )
	DEFINE_KEYFIELD( m_flBusySearchRange, FIELD_FLOAT, "busysearchrange" ),
	DEFINE_KEYFIELD( m_bVisibleOnly, FIELD_BOOLEAN, "visibleonly" ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetBusySearchRange", InputSetBusySearchRange ),
	DEFINE_INPUTFUNC( FIELD_STRING, "ForceNPCToActBusy", InputForceNPCToActBusy ),
	DEFINE_INPUTFUNC( FIELD_EHANDLE, "ForceThisNPCToActBusy", InputForceThisNPCToActBusy ),
	DEFINE_INPUTFUNC( FIELD_EHANDLE, "ForceThisNPCToLeave", InputForceThisNPCToLeave ),

	// Outputs
	DEFINE_OUTPUT( m_OnNPCStartedBusy, "OnNPCStartedBusy" ),
	DEFINE_OUTPUT( m_OnNPCFinishedBusy, "OnNPCFinishedBusy" ),
	DEFINE_OUTPUT( m_OnNPCLeft, "OnNPCLeft" ),
	DEFINE_OUTPUT( m_OnNPCLostSeeEntity, "OnNPCLostSeeEntity" ),
	DEFINE_OUTPUT( m_OnNPCSeeEnemy, "OnNPCSeeEnemy" ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CAI_ActBusyBehavior *CAI_ActBusyGoal::GetBusyBehaviorForNPC( CBaseEntity *pEntity, const char *sInputName )
{
	CAI_BaseNPC *pActor = dynamic_cast<CAI_BaseNPC*>(pEntity);
	if ( !pActor )
	{
		Msg("ai_goal_actbusy input %s fired targeting an entity that isn't an NPC.\n", sInputName);
		return NULL;
	}

	// Get the NPC's behavior
	CAI_ActBusyBehavior *pBehavior;
	if ( !pActor->GetBehavior( &pBehavior ) )
	{
		Msg("ai_goal_actbusy input %s fired on an NPC that doesn't support ActBusy behavior.\n", sInputName );
		return NULL;
	}

	return pBehavior;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CAI_ActBusyBehavior *CAI_ActBusyGoal::GetBusyBehaviorForNPC( const char *pszActorName, CBaseEntity *pActivator, CBaseEntity *pCaller, const char *sInputName )
{
	CBaseEntity *pEntity = gEntList.FindEntityByName( NULL, MAKE_STRING(pszActorName), NULL, pActivator, pCaller );
	if ( !pEntity )
	{
		Msg("ai_goal_actbusy input %s fired targeting a non-existant entity (%s).\n", sInputName, pszActorName );
		return NULL;
	}

	return GetBusyBehaviorForNPC( pEntity, sInputName );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAI_ActBusyGoal::EnableGoal( CAI_BaseNPC *pAI )
{
	BaseClass::EnableGoal( pAI );

	// Now use this actor to lookup the Behavior
	CAI_ActBusyBehavior *pBehavior;
	if ( pAI->GetBehavior( &pBehavior ) )
	{
		// Some NPCs may already be active due to a ForceActBusy input.
		if ( !pBehavior->IsEnabled() )
		{
			pBehavior->Enable( this, m_flBusySearchRange, m_bVisibleOnly );
		}
	}
	else
	{
		DevMsg( "ActBusy goal entity activated for an NPC (%s) that doesn't have the ActBusy behavior\n", pAI->GetDebugName() );
		return;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAI_ActBusyGoal::InputActivate( inputdata_t &inputdata )
{
	if ( ai_debug_actbusy.GetInt() == 4 )
	{
		Msg("ACTBUSY: Actbusy goal %s (%s) activated.\n", GetClassname(), GetDebugName() );
	}

	BaseClass::InputActivate( inputdata );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAI_ActBusyGoal::InputDeactivate( inputdata_t &inputdata )
{
	if ( ai_debug_actbusy.GetInt() == 4 )
	{
		Msg("ACTBUSY: Actbusy goal %s (%s) disabled.\n", GetClassname(), GetDebugName() );
	}

	BaseClass::InputDeactivate( inputdata );

	for( int i = 0 ; i < NumActors() ; i++ )
	{
		CAI_BaseNPC *pActor = GetActor( i );

		if ( pActor )
		{
			// Now use this actor to lookup the Behavior
			CAI_ActBusyBehavior *pBehavior;
			if ( pActor->GetBehavior( &pBehavior ) )
			{
				pBehavior->Disable();
			}
			else
			{
				DevMsg( "ActBusy goal entity deactivated for an NPC that doesn't have the ActBusy behavior\n" );
				return;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyGoal::InputSetBusySearchRange( inputdata_t &inputdata )
{
	m_flBusySearchRange = inputdata.value.Float();

	for( int i = 0 ; i < NumActors() ; i++ )
	{
		CAI_BaseNPC *pActor = GetActor( i );

		if ( pActor )
		{
			// Now use this actor to lookup the Behavior
			CAI_ActBusyBehavior *pBehavior;
			if ( pActor->GetBehavior( &pBehavior ) )
			{
				pBehavior->SetBusySearchRange( m_flBusySearchRange );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyGoal::InputForceNPCToActBusy( inputdata_t &inputdata )
{
	char parseString[255];
	Q_strncpy(parseString, inputdata.value.String(), sizeof(parseString));

	CAI_Hint *pHintNode = NULL;
	float flMaxTime = NO_MAX_TIME;
	bool bTeleport = false;
	bool bUseNearestBusy = false;
	CBaseEntity *pSeeEntity = NULL;

	// Get NPC name
 	char *pszParam = strtok(parseString," ");
	CAI_ActBusyBehavior *pBehavior = GetBusyBehaviorForNPC( pszParam, inputdata.pActivator, inputdata.pCaller, "InputForceNPCToActBusy" );
	if ( !pBehavior )
		return;

	// Wrapped this bugfix so that it doesn't break HL2.
	bool bEpisodicBugFix = hl2_episodic.GetBool();

	// Do we have a specified node too?
	pszParam = strtok(NULL," ");
	if ( pszParam )
	{	
		// Find the specified hintnode
		CBaseEntity *pEntity = gEntList.FindEntityByName( NULL, pszParam, NULL, inputdata.pActivator, inputdata.pCaller );
		if ( pEntity )
		{
			pHintNode = dynamic_cast<CAI_Hint*>(pEntity);
			if ( !pHintNode )
			{
				Msg("ai_goal_actbusy input ForceNPCToActBusy fired targeting an entity that isn't a hintnode.\n", pszParam);
				return;
			}

			if ( bEpisodicBugFix )
			{
				pszParam = strtok(NULL," ");
			}
		}
	}

	Activity activity = ACT_INVALID;

	if ( !bEpisodicBugFix )
	{
 		pszParam = strtok(NULL," ");
	}

	while ( pszParam )
	{
		// Teleport?
 		if ( !Q_strncmp( pszParam, "teleport", 8 ) )
		{
			bTeleport = true;
		}
		else if ( !Q_strncmp( pszParam, "nearest", 8 ) )
		{
			bUseNearestBusy = true;
		}
		else if ( !Q_strncmp( pszParam, "see:", 4 ) )
		{
			pSeeEntity = gEntList.FindEntityByName( NULL, pszParam+4 );
		}
		else if ( pszParam[0] == '$' )
		{
			// $ signs prepend custom movement sequences / activities
			const char *pAnimName = pszParam+1;
			// Try and resolve it as an activity name
			activity = (Activity)ActivityList_IndexForName( pAnimName );
			if ( activity == ACT_INVALID )
			{
				// Try it as sequence name
				pBehavior->GetOuter()->m_iszSceneCustomMoveSeq = AllocPooledString( pAnimName );
				activity = ACT_SCRIPT_CUSTOM_MOVE;
			}
		}
		else 
		{
			// Do we have a specified time?
			flMaxTime = atof( pszParam );
		}

		pszParam = strtok(NULL," ");
	}

	if ( ai_debug_actbusy.GetInt() == 4 )
	{
		Msg("ACTBUSY: Actbusy goal %s (%s) ForceNPCToActBusy input with data: %s.\n", GetClassname(), GetDebugName(), parseString );
	}

	// Tell the NPC to immediately act busy
	pBehavior->SetBusySearchRange( m_flBusySearchRange );
	pBehavior->ForceActBusy( this, pHintNode, flMaxTime, m_bVisibleOnly, bTeleport, bUseNearestBusy, pSeeEntity, activity );
}

//-----------------------------------------------------------------------------
// Purpose: Force the passed in NPC to actbusy
//-----------------------------------------------------------------------------
void CAI_ActBusyGoal::InputForceThisNPCToActBusy( inputdata_t &inputdata )
{
	CAI_ActBusyBehavior *pBehavior = GetBusyBehaviorForNPC( inputdata.value.Entity(), "InputForceThisNPCToActBusy" );
	if ( !pBehavior )
		return;

	// Tell the NPC to immediately act busy
	pBehavior->SetBusySearchRange( m_flBusySearchRange );
	pBehavior->ForceActBusy( this );
}

//-----------------------------------------------------------------------------
// Purpose: Force the passed in NPC to walk to a point and vanish
//-----------------------------------------------------------------------------
void CAI_ActBusyGoal::InputForceThisNPCToLeave( inputdata_t &inputdata )
{
	CAI_ActBusyBehavior *pBehavior = GetBusyBehaviorForNPC( inputdata.value.Entity(), "InputForceThisNPCToLeave" );
	if ( !pBehavior )
		return;

	// Tell the NPC to find a leave point and move to it
	pBehavior->SetBusySearchRange( m_flBusySearchRange );
	pBehavior->ForceActBusyLeave();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pNPC - 
//-----------------------------------------------------------------------------
void CAI_ActBusyGoal::NPCMovingToBusy( CAI_BaseNPC *pNPC )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pNPC - 
//-----------------------------------------------------------------------------
void CAI_ActBusyGoal::NPCStartedBusy( CAI_BaseNPC *pNPC )
{
	m_OnNPCStartedBusy.Set( pNPC, pNPC, this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyGoal::NPCStartedLeavingBusy( CAI_BaseNPC *pNPC )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pNPC - 
//-----------------------------------------------------------------------------
void CAI_ActBusyGoal::NPCAbortedMoveTo( CAI_BaseNPC *pNPC )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pNPC - 
//-----------------------------------------------------------------------------
void CAI_ActBusyGoal::NPCFinishedBusy( CAI_BaseNPC *pNPC )
{
	m_OnNPCFinishedBusy.Set( pNPC, pNPC, this );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pNPC - 
//-----------------------------------------------------------------------------
void CAI_ActBusyGoal::NPCLeft( CAI_BaseNPC *pNPC )
{
	m_OnNPCLeft.Set( pNPC, pNPC, this );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_ActBusyGoal::NPCLostSeeEntity( CAI_BaseNPC *pNPC )
{
	m_OnNPCLostSeeEntity.Set( pNPC, pNPC, this );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_ActBusyGoal::NPCSeeEnemy( CAI_BaseNPC *pNPC )
{
	m_OnNPCSeeEnemy.Set( pNPC, pNPC, this );
}

//==========================================================================================================
// ACT BUSY QUEUE
//==========================================================================================================
//-----------------------------------------------------------------------------
// Purpose: A level tool to control the actbusy behavior to create NPC queues 
//-----------------------------------------------------------------------------
LINK_ENTITY_TO_CLASS( ai_goal_actbusy_queue, CAI_ActBusyQueueGoal );

BEGIN_DATADESC( CAI_ActBusyQueueGoal )
	// Keys
	DEFINE_FIELD( m_iCurrentQueueCount, FIELD_INTEGER ),
	DEFINE_ARRAY( m_hNodes, FIELD_EHANDLE, MAX_QUEUE_NODES ),
	DEFINE_ARRAY( m_bPlayerBlockedNodes, FIELD_BOOLEAN, MAX_QUEUE_NODES ),
	DEFINE_FIELD( m_hExitNode, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hExitingNPC, FIELD_EHANDLE ),
	DEFINE_KEYFIELD( m_bForceReachFront, FIELD_BOOLEAN, "mustreachfront" ),
	// DEFINE_ARRAY( m_iszNodes, FIELD_STRING, MAX_QUEUE_NODES ), // Silence Classcheck!
	DEFINE_KEYFIELD( m_iszNodes[0], FIELD_STRING, "node01"),
	DEFINE_KEYFIELD( m_iszNodes[1], FIELD_STRING, "node02"),
	DEFINE_KEYFIELD( m_iszNodes[2], FIELD_STRING, "node03"),
	DEFINE_KEYFIELD( m_iszNodes[3], FIELD_STRING, "node04"),
	DEFINE_KEYFIELD( m_iszNodes[4], FIELD_STRING, "node05"),
	DEFINE_KEYFIELD( m_iszNodes[5], FIELD_STRING, "node06"),
	DEFINE_KEYFIELD( m_iszNodes[6], FIELD_STRING, "node07"),
	DEFINE_KEYFIELD( m_iszNodes[7], FIELD_STRING, "node08"),
	DEFINE_KEYFIELD( m_iszNodes[8], FIELD_STRING, "node09"),
	DEFINE_KEYFIELD( m_iszNodes[9], FIELD_STRING, "node10"),
	DEFINE_KEYFIELD( m_iszNodes[10], FIELD_STRING, "node11"),
	DEFINE_KEYFIELD( m_iszNodes[11], FIELD_STRING, "node12"),
	DEFINE_KEYFIELD( m_iszNodes[12], FIELD_STRING, "node13"),
	DEFINE_KEYFIELD( m_iszNodes[13], FIELD_STRING, "node14"),
	DEFINE_KEYFIELD( m_iszNodes[14], FIELD_STRING, "node15"),
	DEFINE_KEYFIELD( m_iszNodes[15], FIELD_STRING, "node16"),
	DEFINE_KEYFIELD( m_iszNodes[16], FIELD_STRING, "node17"),
	DEFINE_KEYFIELD( m_iszNodes[17], FIELD_STRING, "node18"),
	DEFINE_KEYFIELD( m_iszNodes[18], FIELD_STRING, "node19"),
	DEFINE_KEYFIELD( m_iszNodes[19], FIELD_STRING, "node20"),
	DEFINE_KEYFIELD( m_iszExitNode, FIELD_STRING, "node_exit"),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_INTEGER, "PlayerStartedBlocking", InputPlayerStartedBlocking ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "PlayerStoppedBlocking", InputPlayerStoppedBlocking ),
	DEFINE_INPUTFUNC( FIELD_VOID, "MoveQueueUp", InputMoveQueueUp ),

	// Outputs
	DEFINE_OUTPUT( m_OnQueueMoved, "OnQueueMoved" ),
	DEFINE_OUTPUT( m_OnNPCLeftQueue, "OnNPCLeftQueue" ),
	DEFINE_OUTPUT( m_OnNPCStartedLeavingQueue, "OnNPCStartedLeavingQueue" ),

	DEFINE_THINKFUNC( QueueThink ),
	DEFINE_THINKFUNC( MoveQueueUpThink ),

END_DATADESC()

#define QUEUE_THINK_CONTEXT			"ActBusyQueueThinkContext"
#define QUEUE_MOVEUP_THINK_CONTEXT	"ActBusyQueueMoveUpThinkContext"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyQueueGoal::Spawn( void )
{
	BaseClass::Spawn();

	RegisterThinkContext( QUEUE_MOVEUP_THINK_CONTEXT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyQueueGoal::DrawDebugGeometryOverlays( void )
{
	BaseClass::DrawDebugGeometryOverlays();

	// Debug for reservers
	for ( int i = 0; i < MAX_QUEUE_NODES; i++ )
	{
		if ( !m_hNodes[i] )
			continue;
		if ( m_bPlayerBlockedNodes[i] )
		{
			NDebugOverlay::Box( m_hNodes[i]->GetAbsOrigin(), -Vector(5,5,5), Vector(5,5,5), 255, 0, 0, 0, 0.1 );
		}
		else
		{
			NDebugOverlay::Box( m_hNodes[i]->GetAbsOrigin(), -Vector(5,5,5), Vector(5,5,5), 255, 255, 255, 0, 0.1 );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyQueueGoal::InputActivate( inputdata_t &inputdata )
{
	if ( !IsActive() )
	{
		// Find all our nodes
		for ( int i = 0; i < MAX_QUEUE_NODES; i++ )
		{
			if ( m_iszNodes[i] == NULL_STRING )
			{
				m_hNodes[i] = NULL;
				continue;
			}

			CBaseEntity *pEntity = gEntList.FindEntityByName( NULL, m_iszNodes[i] );
			if ( !pEntity )
			{
				Warning( "Unable to find ai_goal_actbusy_queue %s's node %d: %s\n", STRING(GetEntityName()), i, STRING(m_iszNodes[i]) );
				UTIL_Remove( this );
				return;
			}
			m_hNodes[i] = dynamic_cast<CAI_Hint*>(pEntity);
			if ( !m_hNodes[i] )
			{
				Warning( "ai_goal_actbusy_queue %s's node %d: '%s' is not an ai_hint.\n", STRING(GetEntityName()), i, STRING(m_iszNodes[i]) );
				UTIL_Remove( this );
				return;
			}

			// Disable all but the first node
			if ( i == 0 )
			{
				m_hNodes[i]->SetDisabled( false );
			}
			else
			{
				m_hNodes[i]->SetDisabled( true );
			}
		}

		// Find the exit node
		m_hExitNode = gEntList.FindEntityByName( NULL, m_iszExitNode );
		if ( !m_hExitNode )
		{
			Warning( "Unable to find ai_goal_actbusy_queue %s's exit node: %s\n", STRING(GetEntityName()), STRING(m_iszExitNode) );
			UTIL_Remove( this );
			return;
		}

		RecalculateQueueCount();

		SetContextThink( &CAI_ActBusyQueueGoal::QueueThink, gpGlobals->curtime + 5, QUEUE_THINK_CONTEXT );
	}

	BaseClass::InputActivate( inputdata );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iCount - 
//-----------------------------------------------------------------------------
void CAI_ActBusyQueueGoal::RecalculateQueueCount( void )
{
	// First, find the highest unused node in the queue
	int iCount = 0;
	for ( int i = 0; i < MAX_QUEUE_NODES; i++ )
	{
		if ( NodeIsOccupied(i) || m_bPlayerBlockedNodes[i] )
		{
			iCount = i+1;
		}
	}

	//Msg("Count: %d (OLD %d)\n", iCount, m_iCurrentQueueCount );

	// Queue hasn't changed?
	if ( iCount == m_iCurrentQueueCount )
		return;

	for ( int i = 0; i < MAX_QUEUE_NODES; i++ )
	{
		if ( m_hNodes[i] )
		{
			// Disable nodes beyond 1 past the end of the queue
			if ( i > iCount )
			{
				m_hNodes[i]->SetDisabled( true );
			}
			else
			{
				m_hNodes[i]->SetDisabled( false );

				// To prevent NPCs outside the queue moving directly to nodes within the queue, only
				// have the entry node be a valid actbusy node.
				if ( i == iCount )
				{
					m_hNodes[i]->SetHintType( HINT_WORLD_WORK_POSITION );
				}
				else
				{
					m_hNodes[i]->SetHintType( HINT_NONE );
				}
			}
		}
	}

	m_iCurrentQueueCount = iCount;
	m_OnQueueMoved.Set( m_iCurrentQueueCount, this, this);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAI_ActBusyQueueGoal::InputPlayerStartedBlocking( inputdata_t &inputdata )
{
	int iNode = inputdata.value.Int() - 1;
	Assert( iNode >= 0 && iNode < MAX_QUEUE_NODES );

	m_bPlayerBlockedNodes[iNode] = true;

	/*
	// First, find all NPCs heading to points in front of the player's blocked spot
	for ( int i = 0; i < iNode; i++ )
	{
		CAI_BaseNPC *pNPC = GetNPCOnNode(i);
		if ( !pNPC )
			continue;

		CAI_ActBusyBehavior *pBehavior = GetQueueBehaviorForNPC( pNPC );
		if ( pBehavior->IsMovingToBusy() )
		{
			// We may be ahead of the player in the queue, which means we can safely 
			// be left alone to reach the node. Make sure we're not closer to it than the player is
			float flPlayerDistToNode = (inputdata.pActivator->GetAbsOrigin() - m_hNodes[i]->GetAbsOrigin()).LengthSqr();
			if ( (pNPC->GetAbsOrigin() - m_hNodes[i]->GetAbsOrigin()).LengthSqr() < flPlayerDistToNode )
				continue;

			// We're an NPC heading to a node past the player, and yet the player's in our way.
			pBehavior->StopBusying();
		}
	}
	*/

	// If an NPC was heading towards this node, tell him to go elsewhere
	CAI_BaseNPC *pNPC = GetNPCOnNode(iNode);
	PushNPCBackInQueue( pNPC, iNode );

	RecalculateQueueCount();
}

//-----------------------------------------------------------------------------
// Purpose: Find a node back in the queue to move to, and push all NPCs beyond that backwards
//-----------------------------------------------------------------------------
void CAI_ActBusyQueueGoal::PushNPCBackInQueue( CAI_BaseNPC *pNPC, int iStartingNode )
{
	// Push this guy back, and tell everyone behind him to move back too, until we find a gap
	while ( pNPC )
	{
		CAI_ActBusyBehavior *pBehavior = GetQueueBehaviorForNPC( pNPC );
		pBehavior->StopBusying();

		// Find any node farther back in the queue that isn't player blocked
		for ( int iNext = iStartingNode+1; iNext < MAX_QUEUE_NODES; iNext++ )
		{
			if ( !m_bPlayerBlockedNodes[iNext] )
			{
				// Kick off any NPCs on the node we're about to steal
				CAI_BaseNPC *pTargetNPC = GetNPCOnNode(iNext);
				if ( pTargetNPC )
				{
					CAI_ActBusyBehavior *pTargetBehavior = GetQueueBehaviorForNPC( pTargetNPC );
					pTargetBehavior->StopBusying();
				}

				// Force the NPC to move up to the empty slot
				pBehavior->ForceActBusy( this, m_hNodes[iNext] );

				// Now look for a spot for the npc who's spot we've just stolen
				pNPC = pTargetNPC;
				iStartingNode = iNext;
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAI_ActBusyQueueGoal::InputPlayerStoppedBlocking( inputdata_t &inputdata )
{
	int iNode = inputdata.value.Int() - 1;
	Assert( iNode >= 0 && iNode < MAX_QUEUE_NODES );

	m_bPlayerBlockedNodes[iNode] = false;

	RecalculateQueueCount();
	MoveQueueUp();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAI_ActBusyQueueGoal::InputMoveQueueUp( inputdata_t &inputdata )
{
	// Find the first NPC in the queue
	CAI_BaseNPC *pNPC = NULL;
	for ( int i = 0; i < MAX_QUEUE_NODES; i++ )
	{
		pNPC = GetNPCOnNode(i);
		if ( pNPC )
		{
			CAI_ActBusyBehavior *pBehavior = GetQueueBehaviorForNPC( pNPC );
			// If we're still en-route, we're only allowed to leave if the queue
			// is allowed to send NPCs away that haven't reached the front.
			if ( !pBehavior->IsMovingToBusy() || !m_bForceReachFront )
				break;

			pNPC = NULL;
		}

		// If queue members have to reach the front of the queue,
		// break after trying the first node.
		if ( m_bForceReachFront )
			break;
	}

	// Did we find an NPC?
	if ( pNPC )
	{
		// Make them leave the actbusy
		CAI_ActBusyBehavior *pBehavior = GetQueueBehaviorForNPC( pNPC );
		pBehavior->Disable();
		m_hExitingNPC = pNPC;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pNPC - 
//-----------------------------------------------------------------------------
void CAI_ActBusyQueueGoal::NPCMovingToBusy( CAI_BaseNPC *pNPC )
{
	BaseClass::NPCMovingToBusy( pNPC );
	RecalculateQueueCount();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pNPC - 
//-----------------------------------------------------------------------------
void CAI_ActBusyQueueGoal::NPCStartedBusy( CAI_BaseNPC *pNPC )
{
	BaseClass::NPCStartedBusy( pNPC );
	MoveQueueUp();
}

//-----------------------------------------------------------------------------
// Purpose: Start a short timer that'll clean up holes in the queue
//-----------------------------------------------------------------------------
void CAI_ActBusyQueueGoal::MoveQueueUp( void )
{
	// Find the node the NPC has arrived at, and tell the guy behind him to move forward
	if ( GetNextThink( QUEUE_MOVEUP_THINK_CONTEXT ) < gpGlobals->curtime )
	{
		float flTime = gpGlobals->curtime + RandomFloat( 0.3, 0.5 );
		SetContextThink( &CAI_ActBusyQueueGoal::MoveQueueUpThink, flTime, QUEUE_MOVEUP_THINK_CONTEXT );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyQueueGoal::MoveQueueUpThink( void )
{
	// Find empty holes in the queue, and move NPCs past them forward
	for ( int iEmptyNode = 0; iEmptyNode < (MAX_QUEUE_NODES-1); iEmptyNode++ )
	{
		if ( !NodeIsOccupied(iEmptyNode) && !m_bPlayerBlockedNodes[iEmptyNode] )
		{
			// Look for NPCs farther down the queue, but not on the other side of a player
			for ( int iNext = iEmptyNode+1; iNext < MAX_QUEUE_NODES; iNext++ )
			{
				// Is the player blocking this node? If so, we're done
				if ( m_bPlayerBlockedNodes[iNext] )
					break;

				CAI_BaseNPC *pNPC = GetNPCOnNode(iNext);
				if ( pNPC )
				{
					CAI_ActBusyBehavior *pBehavior = GetQueueBehaviorForNPC( pNPC );

					// Force the NPC to move up to the empty slot
					pBehavior->ForceActBusy( this, m_hNodes[iEmptyNode] );
					break;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pNPC - 
//-----------------------------------------------------------------------------
void CAI_ActBusyQueueGoal::NPCAbortedMoveTo( CAI_BaseNPC *pNPC )
{
	BaseClass::NPCAbortedMoveTo( pNPC );

	RemoveNPCFromQueue( pNPC );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pNPC - 
//-----------------------------------------------------------------------------
void CAI_ActBusyQueueGoal::NPCFinishedBusy( CAI_BaseNPC *pNPC )
{
	BaseClass::NPCFinishedBusy( pNPC );

	// If this NPC was at the head of the line, move him to the exit node
	if ( m_hExitingNPC == pNPC )
	{
		pNPC->ScheduledMoveToGoalEntity( SCHED_IDLE_WALK, m_hExitNode, ACT_WALK );
		m_OnNPCLeftQueue.Set( pNPC, pNPC, this );
		m_hExitingNPC = NULL;
	}

	RemoveNPCFromQueue( pNPC );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pNPC - 
//-----------------------------------------------------------------------------
void CAI_ActBusyQueueGoal::NPCStartedLeavingBusy( CAI_BaseNPC *pNPC )
{
	BaseClass::NPCStartedLeavingBusy( pNPC );

	// If this NPC it at the head of the line, fire the output
	if ( m_hExitingNPC == pNPC )
	{
		m_OnNPCStartedLeavingQueue.Set( pNPC, pNPC, this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyQueueGoal::RemoveNPCFromQueue( CAI_BaseNPC *pNPC )
{
	RecalculateQueueCount();

	// Find the node the NPC was heading to, and tell the guy behind him to move forward
	MoveQueueUp();
}

//-----------------------------------------------------------------------------
// Purpose: Move the first NPC out of the queue
//-----------------------------------------------------------------------------
void CAI_ActBusyQueueGoal::QueueThink( void )
{
	if ( !GetNPCOnNode(0) )
	{
		MoveQueueUp();
	}

	SetContextThink( &CAI_ActBusyQueueGoal::QueueThink, gpGlobals->curtime + 5, QUEUE_THINK_CONTEXT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
inline bool	CAI_ActBusyQueueGoal::NodeIsOccupied( int i ) 
{ 
	return ( m_hNodes[i] && !m_hNodes[i]->IsDisabled() && m_hNodes[i]->IsLocked() ); 
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iNode - 
// Output : CAI_BaseNPC
//-----------------------------------------------------------------------------
CAI_BaseNPC *CAI_ActBusyQueueGoal::GetNPCOnNode( int iNode )
{
	if ( !m_hNodes[iNode] )
		return NULL;

	return dynamic_cast<CAI_BaseNPC *>(m_hNodes[iNode]->User());
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iNode - 
// Output : CAI_ActBusyBehavior
//-----------------------------------------------------------------------------
CAI_ActBusyBehavior *CAI_ActBusyQueueGoal::GetQueueBehaviorForNPC( CAI_BaseNPC *pNPC )
{
	CAI_ActBusyBehavior *pBehavior;
	pNPC->GetBehavior( &pBehavior );
	Assert( pBehavior );
	return pBehavior;
}

