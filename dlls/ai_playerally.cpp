//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"

#include "sceneentity.h"
#include "ai_playerally.h"
#include "saverestore_utlmap.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//-----------------------------------------------------------------------------

ConVar sk_ally_regen_time( "sk_ally_regen_time", "0.2", FCVAR_NONE, "Time taken for an ally to regenerate a point of health." );
ConVar sv_npc_talker_maxdist( "sv_npc_talker_maxdist", "1024", 0, "NPCs over this distance from the player won't attempt to speak." );
ConVar ai_no_talk_delay( "ai_no_talk_delay", "0" );

//-----------------------------------------------------------------------------

ConceptCategoryInfo_t g_ConceptCategoryInfos[] =
{
	{	10,	20,	0, 0 },
	{	0,	0,	0, 0 },
	{	0,	0,	0, 0 },
};

ConceptInfo_t g_ConceptInfos[] =
{
	{ 	TLK_ANSWER,			SPEECH_IDLE, 		-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_QUESTION,		SPEECH_IDLE, 		-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_IDLE,			SPEECH_IDLE, 		-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT | AICF_TARGET_PLAYER, },
	{ 	TLK_STARE,			SPEECH_IDLE, 		-1,		-1,		-1,		-1,		180,	 0,		AICF_DEFAULT | AICF_TARGET_PLAYER, },
	{ 	TLK_HELLO,			SPEECH_IDLE, 		5,		10,		-1,		-1,		-1,		-1,		AICF_DEFAULT | AICF_SPEAK_ONCE | AICF_PROPAGATE_SPOKEN | AICF_TARGET_PLAYER,	},
	{ 	TLK_PHELLO,			SPEECH_IDLE, 		-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT | AICF_SPEAK_ONCE | AICF_PROPAGATE_SPOKEN | AICF_TARGET_PLAYER,	},
	{ 	TLK_PIDLE,			SPEECH_IDLE, 		-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_PQUESTION,		SPEECH_IDLE, 		-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_SMELL,			SPEECH_IDLE, 		-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_QUESTION_VORT,	SPEECH_IDLE, 		-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_QUESTION_CIT,	SPEECH_IDLE, 		-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_ANSWER_VORT,	SPEECH_IDLE, 		-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_ANSWER_CIT,		SPEECH_IDLE, 		-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{	TLK_USE,			SPEECH_IMPORTANT,	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_STARTFOLLOW,	SPEECH_IMPORTANT,	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_STOPFOLLOW,		SPEECH_IMPORTANT,	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_JOINPLAYER,		SPEECH_IMPORTANT,	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},	
	{ 	TLK_STOP,			SPEECH_IMPORTANT,	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_NOSHOOT,		SPEECH_IMPORTANT,	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_PLHURT1,		SPEECH_IMPORTANT, 	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_PLHURT2,		SPEECH_IMPORTANT, 	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_PLHURT3,		SPEECH_IMPORTANT, 	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_PLPUSH,			SPEECH_IMPORTANT, 	-1,		-1,		-1,		-1,		15,		30,		AICF_DEFAULT,	},
	{ 	TLK_PLRELOAD,		SPEECH_IMPORTANT, 	-1,		-1,		-1,		-1,	    60,		 0,		AICF_DEFAULT,	},
	{ 	TLK_SHOT,			SPEECH_IMPORTANT, 	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_WOUND,			SPEECH_IMPORTANT, 	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_MORTAL,			SPEECH_IMPORTANT, 	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT | AICF_SPEAK_ONCE,	},
	{ 	TLK_SEE_COMBINE,	SPEECH_IMPORTANT, 	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_ENEMY_DEAD,		SPEECH_IMPORTANT, 	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_SELECTED,		SPEECH_IMPORTANT, 	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_COMMANDED,		SPEECH_IMPORTANT, 	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_COMMAND_FAILED,	SPEECH_IMPORTANT, 	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_BETRAYED,		SPEECH_IMPORTANT, 	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_ALLY_KILLED,	SPEECH_IMPORTANT, 	-1,		-1,		-1,		-1,		15,		30,		AICF_DEFAULT,	},
	{ 	TLK_ATTACKING,		SPEECH_IMPORTANT, 	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_HEAL,			SPEECH_IMPORTANT, 	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{ 	TLK_GIVEAMMO,		SPEECH_IMPORTANT, 	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{	TLK_PLYR_PHYSATK,	SPEECH_IMPORTANT,	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{	TLK_NEWWEAPON,		SPEECH_IMPORTANT,	-1,		-1,		-1,		-1,		-1,		-1,		AICF_DEFAULT,	},
	{	TLK_STARTCOMBAT,	SPEECH_IMPORTANT,	-1,		-1,		-1,		-1,		30,		 0,		AICF_DEFAULT,	},
	{	TLK_WATCHOUT,		SPEECH_IMPORTANT,	-1,		-1,		-1,		-1,		30,		45,		AICF_DEFAULT,	},
	{ 	TLK_DANGER,			SPEECH_PRIORITY,	-1,		-1,		-1,		-1,		5,		7,		AICF_DEFAULT,	},
	{ 	TLK_PLDEAD,			SPEECH_PRIORITY,	-1,		-1,		-1,		-1,		100,	 0,		AICF_DEFAULT,	},
	{ 	TLK_HIDEANDRELOAD,	SPEECH_PRIORITY,	-1,		-1,		-1,		-1,		 45,	60,		AICF_DEFAULT,	},
};

//-----------------------------------------------------------------------------

bool ConceptStringLessFunc( const string_t &lhs, const string_t &rhs )	
{ 
	return CaselessStringLessThan( STRING(lhs), STRING(rhs) ); 
}

//-----------------------------------------------------------------------------

CAI_AllySpeechManager::CAI_AllySpeechManager()
{
	m_ConceptTimers.SetLessFunc( ConceptStringLessFunc );
	Assert( !gm_pSpeechManager );
	gm_pSpeechManager = this;
}

CAI_AllySpeechManager::~CAI_AllySpeechManager()
{
	gm_pSpeechManager = NULL;
}

void CAI_AllySpeechManager::Spawn()
{
	for ( int i = 0; i < ARRAYSIZE(g_ConceptInfos); i++ )
		m_ConceptTimers.Insert( AllocPooledString( g_ConceptInfos[i].concept ), CSimpleSimTimer() );
}

ConceptCategoryInfo_t *CAI_AllySpeechManager::GetConceptCategoryInfo( ConceptCategory_t category )
{
	return &g_ConceptCategoryInfos[category];
}

ConceptInfo_t *CAI_AllySpeechManager::GetConceptInfo( AIConcept_t concept )
{
	static CUtlMap<AIConcept_t, ConceptInfo_t *> conceptInfoMap;
	if ( conceptInfoMap.Count() == 0 )
	{
		conceptInfoMap.SetLessFunc( CaselessStringLessThan );
		for ( int i = 0; i < ARRAYSIZE(g_ConceptInfos); i++ )
		{
			conceptInfoMap.Insert( g_ConceptInfos[i].concept, &g_ConceptInfos[i] );
		}
	}
	
	int iResult = conceptInfoMap.Find( concept );
	
	return ( iResult != conceptInfoMap.InvalidIndex() ) ? conceptInfoMap[iResult] : NULL;
}

void CAI_AllySpeechManager::OnSpokeConcept( CAI_PlayerAlly *pPlayerAlly, AIConcept_t concept )
{
	ConceptInfo_t *			pConceptInfo	= GetConceptInfo( concept );
	ConceptCategory_t		category		= ( pConceptInfo ) ? pConceptInfo->category : SPEECH_IDLE;
	ConceptCategoryInfo_t *	pCategoryInfo	= GetConceptCategoryInfo( category );

	if ( pConceptInfo )
	{
		if ( pConceptInfo->flags & AICF_PROPAGATE_SPOKEN ) 
		{
			CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
			CAI_PlayerAlly *pTalker;
			for ( int i = 0; i < g_AI_Manager.NumAIs(); i++ )
			{
				pTalker = dynamic_cast<CAI_PlayerAlly *>(ppAIs[i]);

				if ( pTalker && pTalker != pPlayerAlly && 
					 (pTalker->GetAbsOrigin() - pPlayerAlly->GetAbsOrigin()).LengthSqr() < Square(TALKRANGE_MIN * 2) && 
					 pPlayerAlly->FVisible( pTalker ) )
				{
					// Tell this guy he's already said the concept to the player, too.
					pTalker->GetExpresser()->SetSpokeConcept( concept, NULL, false );
				}
			}
		}
	}

	if ( !ai_no_talk_delay.GetBool() )
	{
		if ( pConceptInfo && pConceptInfo->minGlobalCategoryDelay != -1 )
		{
			Assert( pConceptInfo->maxGlobalCategoryDelay != -1 );
			SetCategoryDelay( pConceptInfo->category, pConceptInfo->minGlobalCategoryDelay, pConceptInfo->maxGlobalCategoryDelay );
		}
		else if ( pCategoryInfo->maxGlobalDelay > 0 )
		{
			SetCategoryDelay( category, pCategoryInfo->minGlobalDelay, pCategoryInfo->maxGlobalDelay );
		}

		if ( pConceptInfo && pConceptInfo->minPersonalCategoryDelay != -1 )
		{
			Assert( pConceptInfo->maxPersonalCategoryDelay != -1 );
			pPlayerAlly->SetCategoryDelay( pConceptInfo->category, pConceptInfo->minPersonalCategoryDelay, pConceptInfo->maxPersonalCategoryDelay );
		}
		else if ( pCategoryInfo->maxPersonalDelay > 0 )
		{
			pPlayerAlly->SetCategoryDelay( category, pCategoryInfo->minPersonalDelay, pCategoryInfo->maxPersonalDelay );
		}

		if ( pConceptInfo && pConceptInfo->minConceptDelay != -1 )
		{
			Assert( pConceptInfo->maxConceptDelay != -1 );
			char iConceptTimer = m_ConceptTimers.Find( MAKE_STRING(concept) );
			if ( iConceptTimer != m_ConceptTimers.InvalidIndex() )
				m_ConceptTimers[iConceptTimer].Set( pConceptInfo->minConceptDelay, pConceptInfo->minConceptDelay );
		}
	}
}

void CAI_AllySpeechManager::SetCategoryDelay( ConceptCategory_t category, float minDelay, float maxDelay )	
{ 
	if ( category != SPEECH_PRIORITY )
		m_ConceptCategoryTimers[category].Set( minDelay, maxDelay ); 
}

bool CAI_AllySpeechManager::CategoryDelayExpired( ConceptCategory_t category )										
{ 
	if ( category == SPEECH_PRIORITY )
		return true;
	return m_ConceptCategoryTimers[category].Expired(); 
}

bool CAI_AllySpeechManager::ConceptDelayExpired( AIConcept_t concept )
{
	char iConceptTimer = m_ConceptTimers.Find( MAKE_STRING(concept) );
	if ( iConceptTimer != m_ConceptTimers.InvalidIndex() )
		return m_ConceptTimers[iConceptTimer].Expired();
	return true;
}

//-----------------------------------------------------------------------------

LINK_ENTITY_TO_CLASS( ai_ally_speech_manager, CAI_AllySpeechManager );

BEGIN_DATADESC( CAI_AllySpeechManager )

	DEFINE_EMBEDDED_AUTO_ARRAY(m_ConceptCategoryTimers),
	DEFINE_UTLMAP( m_ConceptTimers, FIELD_STRING, FIELD_EMBEDDED ),

END_DATADESC()

//-----------------------------------------------------------------------------

CAI_AllySpeechManager *CAI_AllySpeechManager::gm_pSpeechManager;

//-----------------------------------------------------------------------------

CAI_AllySpeechManager *GetAllySpeechManager()
{
	if ( !CAI_AllySpeechManager::gm_pSpeechManager )
	{
		CreateEntityByName( "ai_ally_speech_manager" );
		Assert( CAI_AllySpeechManager::gm_pSpeechManager );
		if ( CAI_AllySpeechManager::gm_pSpeechManager )
			DispatchSpawn( CAI_AllySpeechManager::gm_pSpeechManager );
	}

	return CAI_AllySpeechManager::gm_pSpeechManager;
}

//-----------------------------------------------------------------------------
//
// CLASS: CAI_PlayerAlly
//
//-----------------------------------------------------------------------------

BEGIN_DATADESC( CAI_PlayerAlly )

	DEFINE_EMBEDDED( m_PendingResponse ),
	DEFINE_STDSTRING( m_PendingConcept ),
	DEFINE_FIELD( m_TimePendingSet, FIELD_TIME ),
	DEFINE_FIELD( m_hTalkTarget, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flNextRegenTime, FIELD_TIME ),
	DEFINE_FIELD( m_flTimePlayerStartStare, FIELD_TIME ),
	DEFINE_FIELD( m_hSpeechFilter, FIELD_EHANDLE ),
	DEFINE_EMBEDDED_AUTO_ARRAY(m_ConceptCategoryTimers),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "IdleRespond", InputIdleRespond ),

END_DATADESC()

//-----------------------------------------------------------------------------
// NPCs derived from CAI_PlayerAlly should call this in precache()
//-----------------------------------------------------------------------------
void CAI_PlayerAlly::TalkInit( void )
{
}	

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_PlayerAlly::GatherConditions( void )
{
	BaseClass::GatherConditions();
	
	if ( !HasCondition( COND_SEE_PLAYER ) )
	{
		SetCondition( COND_TALKER_CLIENTUNSEEN );
	}

	CBasePlayer *pLocalPlayer = AI_GetSinglePlayer();

	if ( !pLocalPlayer )
	{
		if ( AI_IsSinglePlayer() )
			SetCondition( COND_TALKER_PLAYER_DEAD );
		return;
	}

	if ( !pLocalPlayer->IsAlive() )
	{
		SetCondition( COND_TALKER_PLAYER_DEAD );
	}
	
	if ( HasCondition( COND_SEE_PLAYER ) )
	{
				
		bool bPlayerIsLooking = false;
		if ( ( pLocalPlayer->GetAbsOrigin() - GetAbsOrigin() ).Length2DSqr() < Square(TALKER_STARE_DIST) )
		{
			if ( pLocalPlayer->FInViewCone( EyePosition() ) )
			{
				if ( pLocalPlayer->GetSmoothedVelocity().LengthSqr() < Square( 100 ) )
					bPlayerIsLooking = true;
			}
		}
		
		if ( bPlayerIsLooking )
		{
			SetCondition( COND_TALKER_PLAYER_STARING );
			if ( m_flTimePlayerStartStare == 0 )
				m_flTimePlayerStartStare = gpGlobals->curtime;
		}
		else
		{
			m_flTimePlayerStartStare = 0;
			ClearCondition( COND_TALKER_PLAYER_STARING );
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_PlayerAlly::GatherEnemyConditions( CBaseEntity *pEnemy )
{
	BaseClass::GatherEnemyConditions( pEnemy );
	if ( GetLastEnemyTime() == 0 || gpGlobals->curtime - GetLastEnemyTime() > 10 )
	{
		if ( HasCondition( COND_SEE_ENEMY ) )
			SpeakIfAllowed( TLK_STARTCOMBAT );
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_PlayerAlly::OnStateChange( NPC_STATE OldState, NPC_STATE NewState )
{
	BaseClass::OnStateChange( OldState, NewState );

	if ( OldState == NPC_STATE_COMBAT )
	{
		DeferAllIdleSpeech();
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_PlayerAlly::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();
	
#ifdef HL2_DLL
	// Vital allies regenerate
	if ( Classify() == CLASS_PLAYER_ALLY_VITAL )
	{
		if ( ( m_flNextRegenTime < gpGlobals->curtime ) && ( GetHealth() < GetMaxHealth() ) )
		{
			TakeHealth( 1, DMG_GENERIC );
			float regenInterval = sk_ally_regen_time.GetFloat();
			if ( g_pGameRules->IsSkillLevel(SKILL_HARD) )
				regenInterval *= 0.5;
			else if ( g_pGameRules->IsSkillLevel(SKILL_EASY) )
				regenInterval *= 1.5;
			m_flNextRegenTime = gpGlobals->curtime + regenInterval + 0.01;
		}
	}
#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CAI_PlayerAlly::SelectSchedule( void )
{
	if ( !HasCondition(COND_RECEIVED_ORDERS) )
	{
		// sustained light wounds?
		if ( m_iHealth <= m_iMaxHealth * 0.75 && IsAllowedToSpeak( TLK_WOUND ) && !GetExpresser()->SpokeConcept(TLK_WOUND) )
		{
			Speak( TLK_WOUND );
		}
		// sustained heavy wounds?
		else if ( m_iHealth <= m_iMaxHealth * 0.5 && IsAllowedToSpeak( TLK_MORTAL) )
		{
			Speak( TLK_MORTAL );
		}
	}

	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CAI_PlayerAlly::SelectSpeechResponse( AIConcept_t concept, const char *pszModifiers, CBaseEntity *pTarget, AISpeechSelection_t *pSelection )
{
	if ( IsAllowedToSpeak( concept ) )
	{
		AI_Response *pResponse = SpeakFindResponse( concept, pszModifiers );
		if ( pResponse )
		{
			pSelection->Set( concept, pResponse, pTarget );
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_PlayerAlly::SetPendingSpeech( AIConcept_t concept, AI_Response *pResponse )
{
	m_PendingResponse = *pResponse;
	pResponse->Release();
	m_PendingConcept = concept;
	m_TimePendingSet = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_PlayerAlly::ClearPendingSpeech()
{
	m_PendingConcept.erase();
	m_TimePendingSet = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool CAI_PlayerAlly::SelectIdleSpeech( AISpeechSelection_t *pSelection )
{
	if ( !IsOkToSpeak( SPEECH_IDLE ) )
		return false;

	CBasePlayer *pTarget = assert_cast<CBasePlayer *>(FindSpeechTarget( AIST_PLAYERS | AIST_FACING_TARGET ));
	if ( pTarget )
	{
		if ( SelectSpeechResponse( TLK_HELLO, NULL, pTarget, pSelection ) )
			return true;
		
		if ( GetTimePlayerStaring() > 6 && !IsMoving() )
		{
			if ( SelectSpeechResponse( TLK_STARE, NULL, pTarget, pSelection ) )
				return true;
		}

		int chance = ( IsMoving() ) ? 20 : 2;
		if ( ShouldSpeakRandom( TLK_IDLE, chance ) && SelectSpeechResponse( TLK_IDLE, NULL, pTarget, pSelection ) )
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool CAI_PlayerAlly::SelectInterjection()
{
	if ( HasPendingSpeech() )
		return false;

	if ( HasCondition(COND_RECEIVED_ORDERS) )
		return false;

	if ( GetState() == NPC_STATE_IDLE || GetState() == NPC_STATE_ALERT )
	{
		AISpeechSelection_t selection;

		if ( SelectIdleSpeech( &selection ) )
		{
			SetSpeechTarget( selection.hSpeechTarget );
			SpeakDispatchResponse( selection.concept.c_str(), selection.pResponse );
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool CAI_PlayerAlly::SelectPlayerUseSpeech()
{
	if( IsOkToSpeakInResponseToPlayer() )
	{
		if ( Speak( TLK_USE ) )
			DeferAllIdleSpeech();
		else
			return Speak( ( !GetExpresser()->SpokeConcept( TLK_HELLO ) ) ? TLK_HELLO : TLK_IDLE );
	}	
	return false;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CAI_PlayerAlly::SelectNonCombatSpeechSchedule()
{
	if ( !HasPendingSpeech() )
	{
		AISpeechSelection_t selection;

		if ( SelectIdleSpeech( &selection ) )
		{
			Assert( selection.pResponse );
			SetSpeechTarget( selection.hSpeechTarget );
			SetPendingSpeech( selection.concept.c_str(), selection.pResponse );
		}
	}
	
	if ( HasPendingSpeech() )
	{
		if ( m_TimePendingSet == gpGlobals->curtime || IsAllowedToSpeak( m_PendingConcept.c_str() ) )
			return SCHED_TALKER_SPEAK_PENDING_IDLE;
	}
	
	return SCHED_NONE;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CAI_PlayerAlly::TranslateSchedule( int schedule )
{
	if ( ( GetState() == NPC_STATE_IDLE || GetState() == NPC_STATE_ALERT ) && 
		 ConditionInterruptsSchedule( schedule, COND_IDLE_INTERRUPT ) &&
		 !HasCondition(COND_RECEIVED_ORDERS) )
	{
		int speechSchedule = SelectNonCombatSpeechSchedule();
		if ( speechSchedule != SCHED_NONE )
			return speechSchedule;
	}

	switch( schedule )
	{
	case SCHED_CHASE_ENEMY_FAILED:
		{
			int baseType = BaseClass::TranslateSchedule(schedule);
			if ( baseType != SCHED_CHASE_ENEMY_FAILED )
				return baseType;

			return SCHED_TAKE_COVER_FROM_ENEMY;
		}
		break;
	}
	return BaseClass::TranslateSchedule( schedule );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_PlayerAlly::OnStartSchedule( int schedule )
{
	if ( schedule == SCHED_HIDE_AND_RELOAD )
		SpeakIfAllowed( TLK_HIDEANDRELOAD );
	BaseClass::OnStartSchedule( schedule );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_PlayerAlly::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_MOVE_AWAY_PATH:
		{
			if ( HasCondition( COND_PLAYER_PUSHING ) && AI_IsSinglePlayer() )
			{
				// @TODO (toml 10-22-04): cope with multiplayer push
				GetMotor()->SetIdealYawToTarget( UTIL_GetLocalPlayer()->WorldSpaceCenter() );
			}
			BaseClass::StartTask( pTask );
			break;
		}

	case TASK_PLAY_SCRIPT:
		SetSpeechTarget( NULL );
		BaseClass::StartTask( pTask );
		break;

	case TASK_TALKER_SPEAK_PENDING:
		if ( !m_PendingConcept.empty() )
		{
			AI_Response *pResponse = new AI_Response;
			*pResponse = m_PendingResponse;
			SpeakDispatchResponse( m_PendingConcept.c_str(), pResponse );
			m_PendingConcept.erase();
			TaskComplete();
		}
		else
			TaskFail( FAIL_NO_SOUND );
		break;

	default:
		BaseClass::StartTask( pTask );
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_PlayerAlly::RunTask( const Task_t *pTask )
{
	BaseClass::RunTask( pTask );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_PlayerAlly::TaskFail( AI_TaskFailureCode_t code )
{
	if ( IsCurSchedule( SCHED_TALKER_SPEAK_PENDING_IDLE, false ) )
		ClearPendingSpeech();
	BaseClass::TaskFail( code );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_PlayerAlly::ClearTransientConditions()
{
	CAI_BaseNPC::ClearTransientConditions();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_PlayerAlly::Touch( CBaseEntity *pOther )
{
	BaseClass::Touch( pOther );

	// Did the player touch me?
	if ( pOther->IsPlayer() )
	{
		// Ignore if pissed at player
		if ( m_afMemory & bits_MEMORY_PROVOKED )
			return;

		// Stay put during speech
		if ( GetExpresser()->IsSpeaking() )
			return;
			
		TestPlayerPushing( pOther );
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_PlayerAlly::OnKilledNPC( CBaseCombatCharacter *pKilled )
{
	if ( pKilled )
	{
		if ( !pKilled->IsNPC() || 
			( pKilled->MyNPCPointer()->GetLastPlayerDamageTime() == 0 ||
			  gpGlobals->curtime - pKilled->MyNPCPointer()->GetLastPlayerDamageTime() > 5 ) )
		{
			SpeakIfAllowed( TLK_ENEMY_DEAD );
		}
	}
}

//-----------------------------------------------------------------------------
void CAI_PlayerAlly::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	const char *pszHitLocCriterion = NULL;

	if ( ptr->hitgroup == HITGROUP_LEFTLEG || ptr->hitgroup == HITGROUP_RIGHTLEG )
	{
		pszHitLocCriterion = "shotloc:leg";
	}
	else if ( ptr->hitgroup == HITGROUP_LEFTARM || ptr->hitgroup == HITGROUP_RIGHTARM )
	{
		pszHitLocCriterion = "shotloc:arm";
	}
	else if ( ptr->hitgroup == HITGROUP_STOMACH )
	{
		pszHitLocCriterion = "shotloc:gut";
	}

	SpeakIfAllowed( TLK_SHOT, pszHitLocCriterion );

	BaseClass::TraceAttack( info, vecDir, ptr );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CAI_PlayerAlly::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	CTakeDamageInfo subInfo = info;
	// Vital allies never take more than 25% of their health in a single hit (except for physics damage)
#ifdef HL2_DLL
	// Don't do damage reduction for DMG_GENERIC. This allows SetHealth inputs to still do full damage.
	if ( subInfo.GetDamageType() != DMG_GENERIC )
	{
		if ( Classify() == CLASS_PLAYER_ALLY_VITAL && !(subInfo.GetDamageType() & DMG_CRUSH) )
		{
			float flDamage = subInfo.GetDamage();
			if ( flDamage > ( GetMaxHealth() * 0.25 ) )
			{
				flDamage = ( GetMaxHealth() * 0.25 );
				subInfo.SetDamage( flDamage );
			}
		}
	}
#endif

	return BaseClass::OnTakeDamage_Alive( subInfo );
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_PlayerAlly::Event_Killed( const CTakeDamageInfo &info )
{
	// notify the player
	if ( IsInPlayerSquad() )
	{
		CBasePlayer *player = AI_GetSinglePlayer();
		if ( player )
		{
			variant_t emptyVariant;
			player->AcceptInput( "OnSquadMemberKilled", this, this, emptyVariant, 0 );
		}
	}

	if ( GetSpeechSemaphore( this )->GetOwner() == this )
		GetSpeechSemaphore( this )->Release();

	CAI_PlayerAlly *pMourner = dynamic_cast<CAI_PlayerAlly *>(FindSpeechTarget( AIST_NPCS ));
	if ( pMourner )
	{
		pMourner->SpeakIfAllowed( TLK_ALLY_KILLED );
	}

	SetTarget( NULL );
	// Don't finish that sentence
	SentenceStop();
	SetUse( NULL );
	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
void CAI_PlayerAlly::PainSound(void)
{
	SpeakIfAllowed( TLK_WOUND );
}

//-----------------------------------------------------------------------------
// Purpose: Implemented to look at talk target
//-----------------------------------------------------------------------------
CBaseEntity *CAI_PlayerAlly::EyeLookTarget( void )
{
	// FIXME: this should be in the VCD
	// FIXME: this is dead code
	if (GetExpresser()->IsSpeaking() && GetSpeechTarget() != NULL)
	{
		return GetSpeechTarget();
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: returns who we're talking to for vcd's
//-----------------------------------------------------------------------------
CBaseEntity *CAI_PlayerAlly::FindNamedEntity( const char *pszName )
{
	if ( !stricmp( pszName, "!speechtarget" ))
	{
		return GetSpeechTarget();
	}

	if ( !stricmp( pszName, "!friend" ))
	{
		return FindSpeechTarget( AIST_NPCS );
	}


	return BaseClass::FindNamedEntity( pszName );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CAI_PlayerAlly::IsValidSpeechTarget( int flags, CBaseEntity *pEntity )
{
	if ( pEntity == this )
		return false;

	if ( !(flags & AIST_IGNORE_RELATIONSHIP) )
	{
		if ( pEntity->IsPlayer() )
		{
			if ( !IsPlayerAlly( (CBasePlayer *)pEntity ) )
				return false;
		}
		else
		{
			if ( IRelationType( pEntity ) != D_LI )
				return false;
		}
	}		

	if ( !pEntity->IsAlive() )
		// don't dead people
		return false;

	// Ignore no-target entities
	if ( pEntity->GetFlags() & FL_NOTARGET )
		return false;

	CAI_BaseNPC *pNPC = pEntity->MyNPCPointer();
	if ( pNPC )
	{
		// If not a NPC for some reason, or in a script.
		if ( (pNPC->m_NPCState == NPC_STATE_SCRIPT || pNPC->m_NPCState == NPC_STATE_PRONE))
			return false;

		if ( pNPC->IsInAScript() )
			return false;

		// Don't bother people who don't want to be bothered
		if ( !pNPC->CanBeUsedAsAFriend() )
			return false;
	}
	
	if ( flags & AIST_FACING_TARGET )
	{
		if ( pEntity->IsPlayer() )
			return HasCondition( COND_SEE_PLAYER );
		else if ( !FInViewCone( pEntity ) )
			return false;
	}

	return FVisible( pEntity );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CBaseEntity *CAI_PlayerAlly::FindSpeechTarget( int flags )
{
	const Vector &	vAbsOrigin 		= GetAbsOrigin();
	float 			closestDistSq 	= FLT_MAX;
	CBaseEntity *	pNearest 		= NULL;
	float			distSq;
	int				i;
	
	if ( flags & AIST_PLAYERS )
	{
		for ( i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );
			if ( pPlayer )
			{
				distSq = ( vAbsOrigin - pPlayer->GetAbsOrigin() ).LengthSqr();
				
				if ( distSq > Square(TALKRANGE_MIN) )
					continue;
					
				if ( !(flags & AIST_ANY_QUALIFIED) && distSq > closestDistSq )
					continue;

				if ( IsValidSpeechTarget( flags, pPlayer ) )
				{
					if ( flags & AIST_ANY_QUALIFIED )
						return pPlayer;

					closestDistSq = distSq;
					pNearest = pPlayer;
				}
			}
		}
	}
	
	if ( flags & AIST_NPCS )
	{
		for ( i = 0; i < g_AI_Manager.NumAIs(); i++ )
		{
			CAI_BaseNPC *pNPC = (g_AI_Manager.AccessAIs())[i];

			distSq = ( vAbsOrigin - pNPC->GetAbsOrigin() ).LengthSqr();
			
			if ( distSq > Square(TALKRANGE_MIN) )
				continue;
				
			if ( !(flags & AIST_ANY_QUALIFIED) && distSq > closestDistSq )
				continue;

			if ( IsValidSpeechTarget( flags, pNPC ) )
			{
				if ( flags & AIST_ANY_QUALIFIED )
					return pNPC;

				closestDistSq = distSq;
				pNearest = pNPC;
			}
		}
	}

	return pNearest;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CAI_PlayerAlly::CanPlaySentence( bool fDisregardState ) 
{ 
	if ( fDisregardState )
		return BaseClass::CanPlaySentence( fDisregardState );
	return IsOkToSpeak(); 
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CAI_PlayerAlly::PlayScriptedSentence( const char *pszSentence, float delay, float volume, soundlevel_t soundlevel, bool bConcurrent, CBaseEntity *pListener )
{
	ClearCondition( COND_PLAYER_PUSHING );	// Forget about moving!  I've got something to say!
	int sentenceIndex = BaseClass::PlayScriptedSentence( pszSentence, delay, volume, soundlevel, bConcurrent, pListener );
	SetSpeechTarget( pListener );
	
	return sentenceIndex;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CAI_PlayerAlly::DeferAllIdleSpeech( float flDelay, CAI_BaseNPC *pIgnore )
{
	CAI_AllySpeechManager *pSpeechManager = GetAllySpeechManager();
	if ( flDelay == -1 )
	{
		ConceptCategoryInfo_t *pCategoryInfo = pSpeechManager->GetConceptCategoryInfo( SPEECH_IDLE );
		pSpeechManager->SetCategoryDelay( SPEECH_IDLE, pCategoryInfo->minGlobalDelay, pCategoryInfo->maxGlobalDelay );
	}
	else
		pSpeechManager->SetCategoryDelay( SPEECH_IDLE, flDelay );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool CAI_PlayerAlly::IsOkToSpeak( ConceptCategory_t category, bool fRespondingToPlayer )
{
	CAI_AllySpeechManager *pSpeechManager = GetAllySpeechManager();
	
	// if not alive, certainly don't speak
	if ( !IsAlive() )
		return false;

	if ( m_spawnflags & SF_NPC_GAG )
		return false;

	// Don't speak if playing a script.
	if ( m_NPCState == NPC_STATE_SCRIPT )
		return false;

	// Don't speak if being eaten by a barnacle
	if ( IsEFlagSet( EFL_IS_BEING_LIFTED_BY_BARNACLE ) )
		return false;

	if ( IsInAScript() )
		return false;

	if ( !fRespondingToPlayer )
	{
		if ( !pSpeechManager->CategoryDelayExpired( category ) || !CategoryDelayExpired( category ) )
			return false;
	}

	if ( category == SPEECH_IDLE )
	{
		if ( GetState() != NPC_STATE_IDLE && GetState() != NPC_STATE_ALERT )
			return false;
		if ( GetSpeechFilter() && GetSpeechFilter()->GetIdleModifier() < 0.001 )
			return false;
	}

	// if player is not in pvs, don't speak
	if ( !UTIL_FindClientInPVS(edict()) )
		return false;

	if ( category != SPEECH_PRIORITY )
	{
		// if someone else is talking, don't speak
		if ( !GetExpresser()->SemaphoreIsAvailable( this ) )
			return false;

		if ( fRespondingToPlayer )
		{
			if ( !GetExpresser()->CanSpeakAfterMyself() )
				return false;
		}
		else
		{
			if ( !GetExpresser()->CanSpeak() )
				return false;
		}

		// Don't talk if we're too far from the player
		CBaseEntity *pPlayer = AI_GetSinglePlayer();
		if ( pPlayer )
		{
			float flDist = sv_npc_talker_maxdist.GetFloat();
			flDist *= flDist;
			if ( (pPlayer->WorldSpaceCenter() - WorldSpaceCenter()).LengthSqr() > flDist )
				return false;
		}
	}

	if ( fRespondingToPlayer )
	{
		// If we're responding to the player, don't respond if the scene has speech in it
		if ( IsRunningScriptedSceneWithSpeech( this ) )
			return false;
	}
	else
	{
		// If we're not responding to the player, don't talk if running a logic_choreo
		if ( IsRunningScriptedScene( this ) )
			return false;
	}

	return true;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool CAI_PlayerAlly::IsOkToSpeak( void )
{
	return IsOkToSpeak( SPEECH_IDLE );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool CAI_PlayerAlly::IsOkToCombatSpeak( void )
{
	return IsOkToSpeak( SPEECH_IMPORTANT );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool CAI_PlayerAlly::IsOkToSpeakInResponseToPlayer( void )
{
	return IsOkToSpeak( SPEECH_IMPORTANT, true );
}

//-----------------------------------------------------------------------------
// Purpose: Return true if I should speak based on the chance & the speech filter's modifier
//-----------------------------------------------------------------------------
bool CAI_PlayerAlly::ShouldSpeakRandom( AIConcept_t concept, int iChance )
{
	CAI_AllySpeechManager *	pSpeechManager	= GetAllySpeechManager();
	ConceptInfo_t *			pInfo			= pSpeechManager->GetConceptInfo( concept );
	ConceptCategory_t		category		= ( pInfo ) ? pInfo->category : SPEECH_IDLE;

	if ( GetSpeechFilter() )
	{
		if ( category == SPEECH_IDLE )
		{
			float flModifier = GetSpeechFilter()->GetIdleModifier();
			if ( flModifier < 0.001 )
				return false;
				
			iChance = floor( (float)iChance / flModifier );
		}
	}
	
	if ( iChance < 1 )
		return false;

	if ( iChance == 1 )
		return true;
		
	return (random->RandomInt(1,iChance) == 1);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CAI_PlayerAlly::IsAllowedToSpeak( AIConcept_t concept, bool bRespondingToPlayer ) 
{ 
	CAI_AllySpeechManager *	pSpeechManager	= GetAllySpeechManager();
	ConceptInfo_t *			pInfo			= pSpeechManager->GetConceptInfo( concept );
	ConceptCategory_t		category		= ( pInfo ) ? pInfo->category : SPEECH_IDLE;

	if ( !IsOkToSpeak( category, bRespondingToPlayer ) )
		return false;

	if ( GetSpeechFilter() && CompareConcepts( concept, TLK_HELLO ) && GetSpeechFilter()->NeverSayHello() )
		return false;
			
	if ( !pSpeechManager->ConceptDelayExpired( concept ) )
		return false;

	if ( ( pInfo && pInfo->flags & AICF_SPEAK_ONCE ) && GetExpresser()->SpokeConcept( concept ) )
		return false;

	if ( !GetExpresser()->CanSpeakConcept( concept ) )
		return false;
	
	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CAI_PlayerAlly::SpeakIfAllowed( AIConcept_t concept, const char *modifiers, bool bRespondingToPlayer ) 
{ 
	if ( IsAllowedToSpeak( concept, bRespondingToPlayer ) )
	{
		return Speak( concept, modifiers );
	}
	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_PlayerAlly::ModifyOrAppendCriteria( AI_CriteriaSet& set )
{
	BaseClass::ModifyOrAppendCriteria( set );

	// Do we have a speech filter? If so, append it's criteria too
	if ( GetSpeechFilter() )
	{
		GetSpeechFilter()->AppendContextToCriteria( set );
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_PlayerAlly::OnSpokeConcept( AIConcept_t concept )
{
	CAI_AllySpeechManager *pSpeechManager = GetAllySpeechManager();
	pSpeechManager->OnSpokeConcept( this, concept );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_PlayerAlly::OnStartSpeaking()
{
	// If you say anything, don't greet the player - you may have already spoken to them
	if ( !GetExpresser()->SpokeConcept( TLK_HELLO ) )
	{
		GetExpresser()->SetSpokeConcept( TLK_HELLO, NULL );
	}
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC(talk_monster_base,CAI_PlayerAlly)

	DECLARE_TASK(TASK_TALKER_SPEAK_PENDING)

	DECLARE_CONDITION(COND_TALKER_CLIENTUNSEEN)
	DECLARE_CONDITION(COND_TALKER_PLAYER_DEAD)
	DECLARE_CONDITION(COND_TALKER_PLAYER_STARING)

	//=========================================================
	// > SCHED_TALKER_SPEAK_PENDING_IDLE
	//=========================================================
	DEFINE_SCHEDULE 
	(
		SCHED_TALKER_SPEAK_PENDING_IDLE,

		"	Tasks"
		"		TASK_TALKER_SPEAK_PENDING		0"
		"		TASK_STOP_MOVING				0"
		"		TASK_SET_ACTIVITY				ACTIVITY:ACT_IDLE"
		"		TASK_WAIT_FOR_SPEAK_FINISH		0"
		"		TASK_WAIT_RANDOM			0.5"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_HEAR_DANGER"
		"		COND_HEAR_COMBAT"
		"		COND_PLAYER_PUSHING"
		"		COND_GIVE_WAY"
	)

	//=========================================================
	// > SCHED_TALKER_SPEAK_PENDING_ALERT
	//=========================================================
	DEFINE_SCHEDULE 
	(
		SCHED_TALKER_SPEAK_PENDING_ALERT,

		"	Tasks"
		"		TASK_TALKER_SPEAK_PENDING		0"
		"		TASK_STOP_MOVING				0"
		"		TASK_SET_ACTIVITY				ACTIVITY:ACT_IDLE"
		"		TASK_WAIT_FOR_SPEAK_FINISH		0"
		"		TASK_WAIT_RANDOM				0.5"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_HEAR_DANGER"
		"		COND_PLAYER_PUSHING"
		"		COND_GIVE_WAY"
	)

	//=========================================================
	// > SCHED_TALKER_SPEAK_PENDING_COMBAT
	//=========================================================
	DEFINE_SCHEDULE 
	(
		SCHED_TALKER_SPEAK_PENDING_COMBAT,

		"	Tasks"
		"		TASK_TALKER_SPEAK_PENDING		0"
		"		TASK_STOP_MOVING				0"
		"		TASK_SET_ACTIVITY				ACTIVITY:ACT_IDLE"
		"		TASK_WAIT_FOR_SPEAK_FINISH		0"
		""
		"	Interrupts"
		"		COND_HEAVY_DAMAGE"
		"		COND_HEAR_DANGER"
	)

AI_END_CUSTOM_NPC()

//-----------------------------------------------------------------------------
