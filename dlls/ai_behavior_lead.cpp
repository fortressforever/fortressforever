//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"

#include "ai_behavior_lead.h"

#include "ai_goalentity.h"
#include "ai_navigator.h"
#include "ai_speech.h"
#include "ai_senses.h"
#include "ai_playerally.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Minimum time between leader nags
#define LEAD_NAG_TIME		3.0

//-----------------------------------------------------------------------------
// class CAI_LeadBehavior
//
// Purpose:
//
//-----------------------------------------------------------------------------

BEGIN_SIMPLE_DATADESC( AI_LeadArgs_t )
	// Only the flags needs saving
	DEFINE_FIELD(		flags,			FIELD_INTEGER ),

	//DEFINE_FIELD(		pszGoal,		FIELD_STRING ),
	//DEFINE_FIELD(		pszWaitPoint,	FIELD_STRING ),
	//DEFINE_FIELD(		flWaitDistance,	FIELD_FLOAT ),
	//DEFINE_FIELD(		flLeadDistance,	FIELD_FLOAT ),
	//DEFINE_FIELD(		bRun,			FIELD_BOOLEAN ),
END_DATADESC();


BEGIN_DATADESC( CAI_LeadBehavior )
	DEFINE_EMBEDDED(	m_args ),
	//					m_pSink		(reconnected on load)
	DEFINE_FIELD(		m_hSinkImplementor, FIELD_EHANDLE ),
	DEFINE_FIELD(		m_goal,			FIELD_POSITION_VECTOR ),
	DEFINE_FIELD(		m_goalyaw, 		FIELD_FLOAT ),
	DEFINE_FIELD(		m_waitpoint, 	FIELD_POSITION_VECTOR ),
	DEFINE_FIELD(		m_waitdistance, FIELD_FLOAT ),
	DEFINE_FIELD(		m_leaddistance, FIELD_FLOAT ),
	DEFINE_FIELD(		m_weaponname,	FIELD_STRING ),
	DEFINE_FIELD(		m_run,			FIELD_BOOLEAN ),
	DEFINE_FIELD(		m_hasspokenstart, FIELD_BOOLEAN ),
	DEFINE_FIELD(		m_hasspokenarrival, FIELD_BOOLEAN ),
	DEFINE_FIELD(		m_flSpeakNextNagTime, FIELD_TIME ),
	DEFINE_FIELD(		m_flWeaponSafetyTimeOut, FIELD_TIME ),
	DEFINE_EMBEDDED(	m_MoveMonitor ),
	DEFINE_EMBEDDED(	m_LostTimer ),
	DEFINE_EMBEDDED(	m_LostLOSTimer ),
END_DATADESC();

//-----------------------------------------------------------------------------


void CAI_LeadBehavior::OnRestore()
{
	CBaseEntity *pSinkImplementor = m_hSinkImplementor;
	if ( pSinkImplementor )
	{
		m_pSink = dynamic_cast<CAI_LeadBehaviorHandler *>(pSinkImplementor);
		if ( !m_pSink )
		{
			DevMsg( "Failed to reconnect to CAI_LeadBehaviorHandler\n" );
			m_hSinkImplementor = NULL;
		}
	}
}

//-------------------------------------

void CAI_LeadBehavior::LeadPlayer( const AI_LeadArgs_t &leadArgs, CAI_LeadBehaviorHandler *pSink )
{
#ifndef CSTRIKE_DLL
	CAI_PlayerAlly *pOuter = dynamic_cast<CAI_PlayerAlly*>(GetOuter());
	if ( pOuter && AI_IsSinglePlayer() )
	{
		pOuter->SetSpeechTarget( UTIL_GetLocalPlayer() );
	}
#endif

	if( SetGoal( leadArgs ) )
	{
		SetCondition( COND_PROVOKED );
		Connect( pSink );
		NotifyChangeBehaviorStatus();
	}
	else
	{
		DevMsg( "*** Warning! LeadPlayer() has a NULL Goal Ent\n" );
	}
}

//-------------------------------------

void CAI_LeadBehavior::StopLeading( void )
{
	ClearGoal();
	m_pSink = NULL;
	NotifyChangeBehaviorStatus();
}

//-------------------------------------

bool CAI_LeadBehavior::CanSelectSchedule()
{
	if ( !AI_GetSinglePlayer() || AI_GetSinglePlayer()->IsDead() )
		return false;

	bool fAttacked = ( HasCondition( COND_LIGHT_DAMAGE ) || HasCondition( COND_HEAVY_DAMAGE ) );
	bool fNonCombat = ( GetNpcState() == NPC_STATE_IDLE || GetNpcState() == NPC_STATE_ALERT );

	return ( !fAttacked && fNonCombat && HasGoal() );
}

//-------------------------------------

void CAI_LeadBehavior::BeginScheduleSelection()
{
	SetTarget( AI_GetSinglePlayer() );
	CAI_Expresser *pExpresser = GetOuter()->GetExpresser();
	if ( pExpresser )
		pExpresser->ClearSpokeConcept( CONCEPT_LEAD_ARRIVAL );
}

//-------------------------------------

bool CAI_LeadBehavior::SetGoal( const AI_LeadArgs_t &args )
{
	CBaseEntity *pGoalEnt;
	pGoalEnt = gEntList.FindEntityByName( NULL, args.pszGoal, NULL );
	
	if ( !pGoalEnt )
		return false;

	m_args 		= args;	// @Q (toml 08-13-02): need to copy string?
	m_goal 		= pGoalEnt->GetLocalOrigin();
	m_goalyaw 	= (args.flags & AILF_USE_GOAL_FACING) ? pGoalEnt->GetLocalAngles().y : -1;
	m_waitpoint = vec3_origin;
	m_waitdistance = args.flWaitDistance;
	m_leaddistance = args.flLeadDistance ? args.flLeadDistance : 64;
	m_run = args.bRun;
	m_hasspokenstart = false;
	m_hasspokenarrival = false;
	m_flSpeakNextNagTime = 0;
	m_flWeaponSafetyTimeOut = 0;

	if ( args.pszWaitPoint && args.pszWaitPoint[0] )
	{
		CBaseEntity *pWaitPoint = gEntList.FindEntityByName( NULL, args.pszWaitPoint, NULL );
		if ( pWaitPoint )
		{
			m_waitpoint = pWaitPoint->GetLocalOrigin();
		}
	}

	return true;
}

//-------------------------------------

void CAI_LeadBehavior::GatherConditions( void )
{
	BaseClass::GatherConditions();

	if ( HasGoal() )
	{
		// Fix for bad transition case (to investigate)
		if ( ( WorldSpaceCenter() - m_goal ).LengthSqr() > (64*64) && IsCurSchedule( SCHED_LEAD_AWAIT_SUCCESS, false)  )
		{
			GetOuter()->ClearSchedule();
		}

		// We have to collect data about the person we're leading around.
		CBaseEntity *pFollower = AI_GetSinglePlayer();

		if( pFollower )
		{
			ClearCondition( COND_LEAD_FOLLOWER_VERY_CLOSE );

			// Check distance to the follower
			float flFollowerDist = ( WorldSpaceCenter() - pFollower->WorldSpaceCenter() ).Length();
			// If he's outside our lag range, consider him lagging
			if ( flFollowerDist > (m_leaddistance*4) )
			{
				SetCondition( COND_LEAD_FOLLOWER_LAGGING );
				ClearCondition( COND_LEAD_FOLLOWER_NOT_LAGGING );
			}
			else
			{
				ClearCondition( COND_LEAD_FOLLOWER_LAGGING );
				SetCondition( COND_LEAD_FOLLOWER_NOT_LAGGING );

				// If he's really close, note that
				if ( flFollowerDist < m_leaddistance )
				{
					SetCondition( COND_LEAD_FOLLOWER_VERY_CLOSE );
				}
			}

			// To be considered not lagging, the follower must be visible, and within the lead distance
			if ( GetOuter()->FVisible( pFollower ) && GetOuter()->GetSenses()->ShouldSeeEntity( pFollower ) )
			{
				SetCondition( COND_LEAD_HAVE_FOLLOWER_LOS );
				m_LostLOSTimer.Stop();
			}
			else
			{
				ClearCondition( COND_LEAD_HAVE_FOLLOWER_LOS );

				// We don't have a LOS. But if we did have LOS, don't clear it until the timer is up.
				if ( m_LostLOSTimer.IsRunning() )
				{
					if ( m_LostLOSTimer.Expired() )
					{
						SetCondition( COND_LEAD_FOLLOWER_LAGGING );
						ClearCondition( COND_LEAD_FOLLOWER_NOT_LAGGING );
					}
				}
				else
				{
					m_LostLOSTimer.Start();
				}
			}

			// Now we want to see if the follower is lost. Being lost means being (far away || out of LOS ) 
			// && some time has passed. Also, lagging players are considered lost if the NPC's never delivered
			// the start speech, because it means the NPC should run to the player to start the lead.
			if( HasCondition( COND_LEAD_FOLLOWER_LAGGING ) )
			{
				if ( !m_hasspokenstart )
				{
					SetCondition( COND_LEAD_FOLLOWER_LOST );
				}
				else
				{
					if( m_LostTimer.IsRunning() )
					{
						if( m_LostTimer.Expired() )
						{
							SetCondition( COND_LEAD_FOLLOWER_LOST );
						}
					}
					else
					{
						m_LostTimer.Start();
					}
				}
			}
			else
			{
				m_LostTimer.Stop();
				ClearCondition( COND_LEAD_FOLLOWER_LOST );
			}

			// Evaluate for success
			// Success right now means being stationary, close to the goal, and having the player close by
			if ( !( m_args.flags & AILF_NO_DEF_SUCCESS ) )
			{
				// Check Z first, and only check 2d if we're within that
				bool bWithinZ = fabs(GetLocalOrigin().z - m_goal.z) < 64;
				if ( bWithinZ && HasCondition( COND_LEAD_FOLLOWER_VERY_CLOSE ) && 
					(GetLocalOrigin() - m_goal).Length2D() <= 64 )
				{

					SetCondition( COND_LEAD_SUCCESS );
				}
				else
				{
					ClearCondition( COND_LEAD_SUCCESS );
				}
			}
			if ( m_MoveMonitor.IsMarkSet() && m_MoveMonitor.TargetMoved( pFollower ) )
				SetCondition( COND_LEAD_FOLLOWER_MOVED_FROM_MARK );
			else
				ClearCondition( COND_LEAD_FOLLOWER_MOVED_FROM_MARK );
		}
	}
}


//-------------------------------------

int CAI_LeadBehavior::SelectSchedule()
{
	if ( HasGoal() )
	{
		if( HasCondition(COND_LEAD_SUCCESS) )
		{
			return SCHED_LEAD_SUCCEED;
		}

		// Player's here, but does he have the weapon we want him to have?
		if ( m_weaponname != NULL_STRING )
		{
			CBasePlayer *pFollower = AI_GetSinglePlayer();
			if ( pFollower && !pFollower->Weapon_OwnsThisType( STRING(m_weaponname) ) )
			{
				// If the safety timeout has run out, just give the player the weapon
				if ( !m_flWeaponSafetyTimeOut || (m_flWeaponSafetyTimeOut > gpGlobals->curtime) )
					return SCHED_LEAD_PLAYERNEEDSWEAPON;

				string_t iszItem = AllocPooledString( "weapon_bugbait" );
				pFollower->GiveNamedItem( STRING(iszItem) );
			}
		}

		// If we have a waitpoint, we want to wait at it for the player.
		if( HasWaitPoint() )
		{
			bool bKeepWaiting = true;

			// If we have no wait distance, trigger as soon as the player comes in view
			if ( !m_waitdistance )
			{
				if ( HasCondition( COND_SEE_PLAYER ) )
				{
					// We've spotted the player, so stop waiting
					bKeepWaiting = false;
				}
			}
			else
			{
				// We have to collect data about the person we're leading around.
				CBaseEntity *pFollower = AI_GetSinglePlayer();
				if( pFollower )
				{
					float flFollowerDist = ( WorldSpaceCenter() - pFollower->WorldSpaceCenter() ).Length();
					if ( flFollowerDist < m_waitdistance )
					{
						bKeepWaiting = false;
					}
				}
			}

			// Player still not here?
			if ( bKeepWaiting )
				return SCHED_LEAD_WAITFORPLAYER;

			// We're finished waiting
			m_waitpoint = vec3_origin;
			Speak( CONCEPT_LEAD_WAITOVER );

			// Don't speak the start line, because we've said 
			m_hasspokenstart = true;
			return SCHED_WAIT_FOR_SPEAK_FINISH;
		}

		// If we haven't spoken our start speech, do that first
		if ( !m_hasspokenstart )
		{
			if ( HasCondition(COND_LEAD_HAVE_FOLLOWER_LOS) && HasCondition(COND_LEAD_FOLLOWER_VERY_CLOSE) )
				return SCHED_LEAD_SPEAK_START;

			// We haven't spoken to him, and we still need to. Go get him.
			return SCHED_LEAD_RETRIEVE;
		}

		if( HasCondition( COND_LEAD_FOLLOWER_LOST ) )
		{
			// If not, we want to go get the player.
			DevMsg( GetOuter(), "Follower lost. Spoke COMING_BACK.\n");

			Speak( CONCEPT_LEAD_COMING_BACK );
			m_MoveMonitor.ClearMark();
			return SCHED_LEAD_RETRIEVE;
		}

		if( HasCondition( COND_LEAD_FOLLOWER_LAGGING ) )
		{
			DevMsg( GetOuter(), "Follower lagging. Spoke CATCHUP.\n");

			Speak( CONCEPT_LEAD_CATCHUP );
			return SCHED_LEAD_PAUSE;
		}
		else
		{
			// If we're at the goal, wait for the player to get here
			if ( ( WorldSpaceCenter() - m_goal ).LengthSqr() < (64*64) )
				return SCHED_LEAD_AWAIT_SUCCESS;

			// If we were retrieving the player, speak the resume
			if ( IsCurSchedule( SCHED_LEAD_RETRIEVE ) )
			{
				Speak( CONCEPT_LEAD_RETRIEVE );
			}

			DevMsg( GetOuter(), "Leading Follower.\n");
			return SCHED_LEAD_PLAYER;
		}
	}
	return BaseClass::SelectSchedule();
}

//-------------------------------------

bool CAI_LeadBehavior::IsCurTaskContinuousMove()
{
	const Task_t *pCurTask = GetCurTask();
	if ( pCurTask && pCurTask->iTask == TASK_LEAD_MOVE_TO_RANGE )
		return true;
	return BaseClass::IsCurTaskContinuousMove();
}

//-------------------------------------

void CAI_LeadBehavior::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
		case TASK_LEAD_FACE_GOAL:
		{
			if ( m_goalyaw != -1 )
			{
				GetMotor()->SetIdealYaw( m_goalyaw ); 
			}

			TaskComplete();
			break;
		}

		case TASK_LEAD_SUCCEED:
		{
			Speak( CONCEPT_LEAD_SUCCESS );
			NotifyEvent( LBE_SUCCESS );

			break;
		}

		case TASK_LEAD_ARRIVE:
		{
			// Only speak the first time we arrive
			if ( !m_hasspokenarrival )
			{
				Speak( CONCEPT_LEAD_ARRIVAL );
				NotifyEvent( LBE_ARRIVAL );

				m_hasspokenarrival = true;
			}
			else
			{
				TaskComplete();
			}
			
			break;
		}
		
		case TASK_STOP_LEADING:
		{
			ClearGoal();
			TaskComplete();
			break;
		}

		case TASK_GET_PATH_TO_LEAD_GOAL:
		{
			if ( GetNavigator()->SetGoal( m_goal ) )
			{
				TaskComplete();
			}
			else
			{
				TaskFail("NO PATH");
			}
			break;
		}
		
		case TASK_LEAD_GET_PATH_TO_WAITPOINT:
		{
			if ( GetNavigator()->SetGoal( m_waitpoint ) )
			{
				TaskComplete();
			}
			else
			{
				TaskFail("NO PATH");
			}
			break;
		}

		case TASK_LEAD_WAVE_TO_PLAYER:
		{
			// Wave to the player if we can see him. Otherwise, just idle.
			if ( HasCondition( COND_SEE_PLAYER ) )
			{
				Speak( CONCEPT_LEAD_ATTRACTPLAYER );
				if ( HaveSequenceForActivity(ACT_SIGNAL1) )
				{
					SetActivity(ACT_SIGNAL1);
				}
			}
			else
			{
				SetActivity(ACT_IDLE);
			}

			TaskComplete();
			break;
		}

		case TASK_LEAD_PLAYER_NEEDS_WEAPON:
		{
			float flAvaliableTime = GetOuter()->GetExpresser()->GetSemaphoreAvailableTime( GetOuter() );

			// if someone else is talking, don't speak
			if ( flAvaliableTime <= gpGlobals->curtime )
			{
				Speak( CONCEPT_LEAD_MISSINGWEAPON );
			}

			SetActivity(ACT_IDLE);
			TaskComplete();
			break;
		}

		case TASK_LEAD_SPEAK_START:
		{
			m_hasspokenstart = true;

			Speak( CONCEPT_LEAD_START );
			SetActivity(ACT_IDLE);
			TaskComplete();
			break;
		}

		case TASK_LEAD_MOVE_TO_RANGE:
		{
			// If we haven't spoken our start speech, move closer
			if ( !m_hasspokenstart)
			{
				ChainStartTask( TASK_MOVE_TO_TARGET_RANGE, m_leaddistance - 24 );
			}
			else
			{
				ChainStartTask( TASK_MOVE_TO_TARGET_RANGE, m_leaddistance + 24 );
			}
			break;
		}

		case TASK_LEAD_RETRIEVE_WAIT:
		{
			m_MoveMonitor.SetMark( AI_GetSinglePlayer(), 24 );
			ChainStartTask( TASK_WAIT_INDEFINITE );
			break;
		}

		case TASK_STOP_MOVING:
		{
			BaseClass::StartTask( pTask);

			if ( IsCurSchedule( SCHED_LEAD_PAUSE ) && pTask->flTaskData == 1 )
			{
				GetNavigator()->SetArrivalDirection( GetTarget() );
			}
			break;
		}

		default:
			BaseClass::StartTask( pTask);
	}
}

//-------------------------------------

void CAI_LeadBehavior::RunTask( const Task_t *pTask )		
{ 
	switch ( pTask->iTask )
	{
		case TASK_LEAD_SUCCEED:
		{
			if ( !IsSpeaking() )
			{
				TaskComplete();
				NotifyEvent( LBE_DONE );
			}
			break;
		}
		case TASK_LEAD_ARRIVE:
		{
			if ( !IsSpeaking() )
			{
				TaskComplete();
				NotifyEvent( LBE_ARRIVAL_DONE );
			}
			break;
		}

		case TASK_LEAD_MOVE_TO_RANGE:
		{
			// If we haven't spoken our start speech, move closer
			if ( !m_hasspokenstart)
			{
				ChainRunTask( TASK_MOVE_TO_TARGET_RANGE, m_leaddistance - 24 );
			}
			else
			{
				ChainRunTask( TASK_MOVE_TO_TARGET_RANGE, m_leaddistance + 24 );
			}
			break;
		}

		case TASK_LEAD_RETRIEVE_WAIT:
		{
			ChainRunTask( TASK_WAIT_INDEFINITE );
			break;
		}

		default:
			BaseClass::RunTask( pTask);
	}
}

//-------------------------------------

Activity CAI_LeadBehavior::NPC_TranslateActivity( Activity activity )
{
	// If we're leading, and we're supposed to run, run instead of walking
	if ( activity == ACT_WALK && m_run && (IsCurSchedule( SCHED_LEAD_WAITFORPLAYER ) 
								 		|| IsCurSchedule( SCHED_LEAD_PLAYER ) 
										|| IsCurSchedule( SCHED_LEAD_RETRIEVE )) )
		return ACT_RUN;

	return BaseClass::NPC_TranslateActivity( activity );
}

//-------------------------------------

bool CAI_LeadBehavior::Speak( AIConcept_t concept )
{
	CAI_Expresser *pExpresser = GetOuter()->GetExpresser();
	if ( !pExpresser )
		return false;

	// If we haven't said the start speech, don't nag
	bool bNag = ( FStrEq(concept,CONCEPT_LEAD_COMING_BACK) || FStrEq(concept, CONCEPT_LEAD_CATCHUP) || FStrEq(concept, CONCEPT_LEAD_RETRIEVE) );
	if ( !m_hasspokenstart && bNag )
		return false;

	// Don't spam Nags
	if ( bNag )
	{
		if ( m_flSpeakNextNagTime > gpGlobals->curtime )
		{
			DevMsg( GetOuter(), "Leader didn't speak due to Nag timer.\n");
			return false;
		}
	}
	
	if ( pExpresser->Speak( concept, GetConceptModifiers( concept ) ) )
	{
		m_flSpeakNextNagTime = gpGlobals->curtime + LEAD_NAG_TIME;
		return true;
	}

	return false;
}

//-------------------------------------

bool CAI_LeadBehavior::IsSpeaking()
{
	CAI_Expresser *pExpresser = GetOuter()->GetExpresser();
	if ( !pExpresser )
		return false;
		
	return pExpresser->IsSpeaking();
}

//-------------------------------------

bool CAI_LeadBehavior::Connect( CAI_LeadBehaviorHandler *pSink )
{
	m_pSink = pSink;
	m_hSinkImplementor = dynamic_cast<CBaseEntity *>(pSink);

	if ( m_hSinkImplementor == NULL )
		DevMsg( 2, "Note: CAI_LeadBehaviorHandler connected to a sink that isn't an entity. Manual fixup on load will be necessary\n" );

	return true;
}

//-------------------------------------

bool CAI_LeadBehavior::Disconnect( CAI_LeadBehaviorHandler *pSink )
{
	Assert( pSink == m_pSink );
	m_pSink = NULL;
	m_hSinkImplementor = NULL;
	return true;
}

//-------------------------------------

AI_BEGIN_CUSTOM_SCHEDULE_PROVIDER( CAI_LeadBehavior )

	DECLARE_CONDITION( COND_LEAD_FOLLOWER_LOST )
	DECLARE_CONDITION( COND_LEAD_FOLLOWER_LAGGING )
	DECLARE_CONDITION( COND_LEAD_FOLLOWER_NOT_LAGGING )
	DECLARE_CONDITION( COND_LEAD_FOLLOWER_VERY_CLOSE )
	DECLARE_CONDITION( COND_LEAD_SUCCESS )
	DECLARE_CONDITION( COND_LEAD_HAVE_FOLLOWER_LOS )
	DECLARE_CONDITION( COND_LEAD_FOLLOWER_MOVED_FROM_MARK )

	//---------------------------------
	//
	// Lead
	//
	DECLARE_TASK( TASK_GET_PATH_TO_LEAD_GOAL )
	DECLARE_TASK( TASK_STOP_LEADING )
	DECLARE_TASK( TASK_LEAD_ARRIVE )
	DECLARE_TASK( TASK_LEAD_SUCCEED )
	DECLARE_TASK( TASK_LEAD_FACE_GOAL )
	DECLARE_TASK( TASK_LEAD_GET_PATH_TO_WAITPOINT )
	DECLARE_TASK( TASK_LEAD_WAVE_TO_PLAYER )
	DECLARE_TASK( TASK_LEAD_PLAYER_NEEDS_WEAPON )
	DECLARE_TASK( TASK_LEAD_MOVE_TO_RANGE )
	DECLARE_TASK( TASK_LEAD_SPEAK_START )
	DECLARE_TASK( TASK_LEAD_RETRIEVE_WAIT )

	DEFINE_SCHEDULE
	( 
		SCHED_LEAD_RETRIEVE,

		"	Tasks"
		"		TASK_GET_PATH_TO_PLAYER			0"
		"		TASK_LEAD_MOVE_TO_RANGE			0"
		"		TASK_STOP_MOVING				0"
		"		TASK_WAIT_FOR_SPEAK_FINISH		0"
		"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_LEAD_RETRIEVE_WAIT"
		"		"
		"	Interrupts"
		"		COND_LEAD_FOLLOWER_VERY_CLOSE"
		"		COND_LIGHT_DAMAGE"
		"		COND_NEW_ENEMY"
	)

	//-------------------------------------

	DEFINE_SCHEDULE
	( 
		SCHED_LEAD_RETRIEVE_WAIT,

		"	Tasks"
		"		TASK_LEAD_RETRIEVE_WAIT			0"
		"		"
		"	Interrupts"
		"		COND_LEAD_FOLLOWER_LOST"
		"		COND_LEAD_FOLLOWER_LAGGING"
		"		COND_LEAD_FOLLOWER_VERY_CLOSE"
		"		COND_LEAD_FOLLOWER_MOVED_FROM_MARK"
		"		COND_LIGHT_DAMAGE"
		"		COND_NEW_ENEMY"
	)

	//---------------------------------

	DEFINE_SCHEDULE
	( 
		SCHED_LEAD_PLAYER,

		"	Tasks"
		"		TASK_WAIT_FOR_SPEAK_FINISH	0"
		"		TASK_GET_PATH_TO_LEAD_GOAL	0"
		"		TASK_WALK_PATH				0"
		"		TASK_WAIT_FOR_MOVEMENT		0"
		"		TASK_STOP_MOVING			0"
		""
		"	Interrupts"
		"		COND_LEAD_FOLLOWER_LOST"
		"		COND_LEAD_FOLLOWER_LAGGING"
		"		COND_LIGHT_DAMAGE"
		"		COND_NEW_ENEMY"
	)

	//---------------------------------

	DEFINE_SCHEDULE
	( 
		SCHED_LEAD_AWAIT_SUCCESS,

		"	Tasks"
		"		TASK_LEAD_FACE_GOAL			0"
		"		TASK_FACE_IDEAL				0"
		"		TASK_LEAD_ARRIVE			0"
		"		TASK_WAIT_INDEFINITE		0"
		""
		"	Interrupts"
		"		COND_LEAD_FOLLOWER_LOST"
		"		COND_LEAD_FOLLOWER_LAGGING"
		"		COND_LEAD_SUCCESS"
		"		COND_LIGHT_DAMAGE"
		"		COND_NEW_ENEMY"
	)

	//---------------------------------

	DEFINE_SCHEDULE
	(
		SCHED_LEAD_SUCCEED,

		"	Tasks"
		"		TASK_LEAD_SUCCEED			0"
		"		TASK_STOP_LEADING			0"
		""
	)

	//---------------------------------
	// This is the schedule Odell uses to pause the tour momentarily
	// if the player lags behind. If the player shows up in a 
	// couple of seconds, the tour will resume. Otherwise, Odell
	// moves to retrieve.
	//---------------------------------

	DEFINE_SCHEDULE
	( 
		SCHED_LEAD_PAUSE,

		"	Tasks"
		"		TASK_STOP_MOVING			1"
		"		TASK_FACE_TARGET			0"
		"		TASK_WAIT					5"
		"		TASK_WAIT_RANDOM			5"
		"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_LEAD_RETRIEVE"
		""
		"	Interrupts"
		"		COND_LEAD_FOLLOWER_VERY_CLOSE"
		"		COND_LEAD_FOLLOWER_NOT_LAGGING"
		"		COND_LEAD_FOLLOWER_LOST"
		"		COND_LIGHT_DAMAGE"
		"		COND_NEW_ENEMY"
	)

	//---------------------------------

	DEFINE_SCHEDULE
	( 
		SCHED_LEAD_WAITFORPLAYER,

		"	Tasks"
		"		TASK_LEAD_GET_PATH_TO_WAITPOINT	0"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_STOP_MOVING				0"
		"		TASK_WAIT						0.5"
		"		TASK_FACE_TARGET				0"
		"		TASK_LEAD_WAVE_TO_PLAYER		0"
		"		TASK_WAIT						5.0"
		"		"
		"	Interrupts"
		"		COND_LEAD_FOLLOWER_VERY_CLOSE"
		"		COND_LIGHT_DAMAGE"
		"		COND_NEW_ENEMY"
	)

	//---------------------------------

	DEFINE_SCHEDULE
	( 
		SCHED_LEAD_PLAYERNEEDSWEAPON,

		"	Tasks"
		"		TASK_FACE_PLAYER				0"
		"		TASK_LEAD_PLAYER_NEEDS_WEAPON	0"
		"		TASK_WAIT_FOR_SPEAK_FINISH		0"
		"		TASK_WAIT						8"
		"		"
		"	Interrupts"
	)

	//---------------------------------

	DEFINE_SCHEDULE
	( 
		SCHED_LEAD_SPEAK_START,

		"	Tasks"
		"		TASK_LEAD_SPEAK_START			0"
		"		TASK_WAIT_FOR_SPEAK_FINISH		0"
		""
		"	Interrupts"
	)

AI_END_CUSTOM_SCHEDULE_PROVIDER()


//-----------------------------------------------------------------------------
//
// Purpose: A level tool to control the lead behavior. Use is not required
//			in order to use behavior.
//

class CAI_LeadGoal : public CAI_GoalEntity,
					 public CAI_LeadBehaviorHandler
{
	DECLARE_CLASS( CAI_LeadGoal, CAI_GoalEntity );
public:
	CAI_LeadGoal()
	 :	m_fArrived( false )
	{
	}

	CAI_LeadBehavior *GetLeadBehavior();

	virtual const char *GetConceptModifiers( const char *pszConcept );

	virtual void InputActivate( inputdata_t &inputdata );
	virtual void InputDeactivate( inputdata_t &inputdata );
	
	DECLARE_DATADESC();
private:

	virtual void OnEvent( int event );
	void InputSetSuccess( inputdata_t &inputdata );
	void InputSetFailure( inputdata_t &inputdata );
	
	bool	 m_fArrived; // @TODO (toml 08-16-02): move arrived tracking onto behavior
	float	 m_flWaitDistance;
	float	 m_flLeadDistance;
	bool	 m_bRun;

	string_t m_iszWaitPointName;

	string_t m_iszStartConceptModifier;
	string_t m_iszAttractPlayerConceptModifier;
	string_t m_iszWaitOverConceptModifier;
	string_t m_iszArrivalConceptModifier;
	string_t m_iszPostArrivalConceptModifier;
	string_t m_iszSuccessConceptModifier;
	string_t m_iszFailureConceptModifier;

	// Output handlers
	COutputEvent	m_OnArrival;
	COutputEvent	m_OnArrivalDone;
	COutputEvent	m_OnSuccess;
	COutputEvent	m_OnFailure;
	COutputEvent	m_OnDone;
};

//-----------------------------------------------------------------------------
//
// CAI_LeadGoal implementation
//

LINK_ENTITY_TO_CLASS( ai_goal_lead, CAI_LeadGoal );

BEGIN_DATADESC( CAI_LeadGoal )

	DEFINE_FIELD( m_fArrived, FIELD_BOOLEAN ),

	DEFINE_KEYFIELD(m_flWaitDistance, 		FIELD_FLOAT, 	"WaitDistance"),
	DEFINE_KEYFIELD(m_iszWaitPointName, 	FIELD_STRING, 	"WaitPointName"),
	DEFINE_KEYFIELD(m_flLeadDistance, 		FIELD_FLOAT, 	"LeadDistance"),
	DEFINE_KEYFIELD(m_bRun, 				FIELD_BOOLEAN, 	"Run"),

	DEFINE_KEYFIELD(m_iszStartConceptModifier,			FIELD_STRING, 	"StartConceptModifier"),
	DEFINE_KEYFIELD(m_iszAttractPlayerConceptModifier,	FIELD_STRING, 	"AttractPlayerConceptModifier"),
	DEFINE_KEYFIELD(m_iszWaitOverConceptModifier, 		FIELD_STRING, 	"WaitOverConceptModifier"),
	DEFINE_KEYFIELD(m_iszArrivalConceptModifier, 		FIELD_STRING, 	"ArrivalConceptModifier"),
	DEFINE_KEYFIELD(m_iszPostArrivalConceptModifier,	FIELD_STRING,	"PostArrivalConceptModifier"),
	DEFINE_KEYFIELD(m_iszSuccessConceptModifier,		FIELD_STRING,	"SuccessConceptModifier"),
	DEFINE_KEYFIELD(m_iszFailureConceptModifier,		FIELD_STRING,	"FailureConceptModifier"),

	DEFINE_OUTPUT( m_OnSuccess, 		"OnSuccess" ),
	DEFINE_OUTPUT( m_OnArrival, 		"OnArrival" ),
	DEFINE_OUTPUT( m_OnArrivalDone, 	"OnArrivalDone" ),
	DEFINE_OUTPUT( m_OnFailure, 		"OnFailure" ),
	DEFINE_OUTPUT( m_OnDone,	  		"OnDone" ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "SetSuccess", InputSetSuccess ),
	DEFINE_INPUTFUNC( FIELD_VOID, "SetFailure", InputSetFailure ),

END_DATADESC()


//-----------------------------------------------------------------------------

CAI_LeadBehavior *CAI_LeadGoal::GetLeadBehavior()
{
	CAI_BaseNPC *pActor = GetActor();
	if ( !pActor )
		return NULL;

	CAI_LeadBehavior *pBehavior;
	if ( !pActor->GetBehavior( &pBehavior ) )
	{
		return NULL;
	}
	
	return pBehavior;
}

//-----------------------------------------------------------------------------

void CAI_LeadGoal::InputSetSuccess( inputdata_t &inputdata )
{
	CAI_LeadBehavior *pBehavior = GetLeadBehavior();
	if ( !pBehavior )
		return;
		
	// @TODO (toml 02-14-03): Hackly!
	pBehavior->SetCondition( CAI_LeadBehavior::COND_LEAD_SUCCESS);
}


//-----------------------------------------------------------------------------

void CAI_LeadGoal::InputSetFailure( inputdata_t &inputdata )
{
	DevMsg( "SetFailure unimplemented\n" );
}


//-----------------------------------------------------------------------------

void CAI_LeadGoal::InputActivate( inputdata_t &inputdata )
{
	BaseClass::InputActivate( inputdata );
	
	CAI_LeadBehavior *pBehavior = GetLeadBehavior();
	if ( !pBehavior )
	{
		DevMsg( "Lead goal entity activated for an NPC that doesn't have the lead behavior\n" );
		return;
	}
	
	AI_LeadArgs_t leadArgs = { GetGoalEntityName(), STRING(m_iszWaitPointName), m_spawnflags, m_flWaitDistance, m_flLeadDistance, m_bRun };
	
	pBehavior->LeadPlayer( leadArgs, this );
}

//-----------------------------------------------------------------------------

void CAI_LeadGoal::InputDeactivate( inputdata_t &inputdata )
{
	BaseClass::InputDeactivate( inputdata );

	CAI_LeadBehavior *pBehavior = GetLeadBehavior();
	if ( !pBehavior )
		return;
		
	pBehavior->StopLeading();
}

//-----------------------------------------------------------------------------

void CAI_LeadGoal::OnEvent( int event )
{
	COutputEvent *pOutputEvent = NULL;

	switch ( event )
	{
		case LBE_ARRIVAL:		pOutputEvent = &m_OnArrival;		break;
		case LBE_ARRIVAL_DONE:	pOutputEvent = &m_OnArrivalDone;	break;
		case LBE_SUCCESS:		pOutputEvent = &m_OnSuccess;		break;
		case LBE_FAILURE:		pOutputEvent = &m_OnFailure;		break;
		case LBE_DONE:			pOutputEvent = &m_OnDone;			break;
	}
	
	// @TODO (toml 08-16-02): move arrived tracking onto behavior
	if ( event == LBE_ARRIVAL )
		m_fArrived = true;

	if ( pOutputEvent )
		pOutputEvent->FireOutput( this, this );
}

//-----------------------------------------------------------------------------

const char *CAI_LeadGoal::GetConceptModifiers( const char *pszConcept )	
{ 
	if ( m_iszStartConceptModifier != NULL_STRING && *STRING(m_iszStartConceptModifier) && strcmp( pszConcept, CONCEPT_LEAD_START) == 0 )
		return STRING( m_iszStartConceptModifier );

	if ( m_iszAttractPlayerConceptModifier != NULL_STRING && *STRING(m_iszAttractPlayerConceptModifier) && strcmp( pszConcept, CONCEPT_LEAD_ATTRACTPLAYER) == 0 )
		return STRING( m_iszAttractPlayerConceptModifier );

	if ( m_iszWaitOverConceptModifier != NULL_STRING && *STRING(m_iszWaitOverConceptModifier) && strcmp( pszConcept, CONCEPT_LEAD_WAITOVER) == 0 )
		return STRING( m_iszWaitOverConceptModifier );

	if ( m_iszArrivalConceptModifier != NULL_STRING && *STRING(m_iszArrivalConceptModifier) && strcmp( pszConcept, CONCEPT_LEAD_ARRIVAL) == 0 )
		return STRING( m_iszArrivalConceptModifier );
		
	if ( m_iszSuccessConceptModifier != NULL_STRING && *STRING(m_iszSuccessConceptModifier) && strcmp( pszConcept, CONCEPT_LEAD_SUCCESS) == 0 )
		return STRING( m_iszSuccessConceptModifier );
		
	if (m_iszFailureConceptModifier != NULL_STRING && *STRING(m_iszFailureConceptModifier) && strcmp( pszConcept, CONCEPT_LEAD_FAILURE) == 0 )
		return STRING( m_iszFailureConceptModifier );
		
	if ( m_fArrived && m_iszPostArrivalConceptModifier != NULL_STRING && *STRING(m_iszPostArrivalConceptModifier) )
		return STRING( m_iszPostArrivalConceptModifier );
	
	return NULL; 
}


//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Purpose: A custom lead goal that waits until the player has a weapon.
//

class CAI_LeadGoal_Weapon : public CAI_LeadGoal
{
	DECLARE_CLASS( CAI_LeadGoal_Weapon, CAI_LeadGoal );
public:

	virtual const char *GetConceptModifiers( const char *pszConcept );
	virtual void InputActivate( inputdata_t &inputdata );

private:
	string_t	m_iszWeaponName;
	string_t	m_iszMissingWeaponConceptModifier;

	DECLARE_DATADESC();
};

//-----------------------------------------------------------------------------
//
// CAI_LeadGoal_Weapon implementation
//

LINK_ENTITY_TO_CLASS( ai_goal_lead_weapon, CAI_LeadGoal_Weapon );

BEGIN_DATADESC( CAI_LeadGoal_Weapon )

	DEFINE_KEYFIELD( m_iszWeaponName, 		FIELD_STRING, 	"WeaponName"),
	DEFINE_KEYFIELD( m_iszMissingWeaponConceptModifier, FIELD_STRING, 	"MissingWeaponConceptModifier"),

END_DATADESC()

//-----------------------------------------------------------------------------

const char *CAI_LeadGoal_Weapon::GetConceptModifiers( const char *pszConcept )	
{ 
	if ( m_iszMissingWeaponConceptModifier != NULL_STRING && *STRING(m_iszMissingWeaponConceptModifier) && strcmp( pszConcept, CONCEPT_LEAD_MISSINGWEAPON) == 0 )
		return STRING( m_iszMissingWeaponConceptModifier );

	return BaseClass::GetConceptModifiers( pszConcept ); 
}


//-----------------------------------------------------------------------------

void CAI_LeadGoal_Weapon::InputActivate( inputdata_t &inputdata )
{
	BaseClass::InputActivate( inputdata );

	CAI_LeadBehavior *pBehavior = GetLeadBehavior();
	if ( pBehavior )
	{
		pBehavior->SetWaitForWeapon( m_iszWeaponName );
	}
}
