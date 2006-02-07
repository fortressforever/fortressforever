//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef AI_BEHAVIOR_LEAD_H
#define AI_BEHAVIOR_LEAD_H

#include "simtimer.h"
#include "ai_behavior.h"

#if defined( _WIN32 )
#pragma once
#endif

typedef const char *AIConcept_t;

// Speak concepts
#define CONCEPT_LEAD_START				"TLK_LEAD_START"
#define CONCEPT_LEAD_ARRIVAL			"TLK_LEAD_ARRIVAL"
#define CONCEPT_LEAD_SUCCESS			"TLK_LEAD_SUCCESS"
#define CONCEPT_LEAD_FAILURE			"lead_fail"
#define CONCEPT_LEAD_COMING_BACK		"TLK_LEAD_COMINGBACK"
#define CONCEPT_LEAD_CATCHUP			"TLK_LEAD_CATCHUP"
#define CONCEPT_LEAD_RETRIEVE			"TLK_LEAD_RETRIEVE"
#define CONCEPT_LEAD_ATTRACTPLAYER		"TLK_LEAD_ATTRACTPLAYER"
#define CONCEPT_LEAD_WAITOVER			"TLK_LEAD_WAITOVER"
#define CONCEPT_LEAD_MISSINGWEAPON		"TLK_LEAD_MISSING_WEAPON"

//-----------------------------------------------------------------------------
// class CAI_LeadBehavior
//
// Purpose:
//
//-----------------------------------------------------------------------------

enum LeadBehaviorEvents_t
{
	LBE_ARRIVAL,
	LBE_ARRIVAL_DONE,
	LBE_SUCCESS,
	LBE_FAILURE,
	LBE_DONE,
};

//-------------------------------------
//
// Handler class interface to listen to and modify actions of the lead behavior.
// Could be an NPC, or another entity (like a goal entity)
//

class CAI_LeadBehaviorHandler
{
public:
	virtual void OnEvent( int event ) {}
	virtual const char *GetConceptModifiers( const char *pszConcept )	{ return NULL; }
};

//-------------------------------------

enum AI_LeadFlags_t
{
	AILF_NO_DEF_SUCCESS	 = 0x01,
	AILF_NO_DEF_FAILURE	 = 0x02,
	AILF_USE_GOAL_FACING = 0x04,
};

struct AI_LeadArgs_t
{
	const char *pszGoal;
	const char *pszWaitPoint;
	unsigned 	flags;
	float		flWaitDistance;
	float		flLeadDistance;
	bool		bRun;

	DECLARE_SIMPLE_DATADESC();
};


class CAI_LeadBehavior : public CAI_SimpleBehavior
{
	DECLARE_CLASS( CAI_LeadBehavior, CAI_SimpleBehavior );
public:
	CAI_LeadBehavior()
	 :	m_pSink(NULL),
		m_LostTimer( 3.0, 4.0 ),
		m_LostLOSTimer( 2.0, 3.0 )
	{
		memset( &m_args, 0, sizeof(m_args) );
		ClearGoal();
	}
	
	virtual void OnRestore();

	virtual const char *GetName() {	return "Lead"; }

	void LeadPlayer( const AI_LeadArgs_t &leadArgs, CAI_LeadBehaviorHandler *pSink = NULL );
	void StopLeading( void );

	virtual bool CanSelectSchedule();
	void BeginScheduleSelection();

	virtual Activity NPC_TranslateActivity( Activity activity );

	virtual bool IsCurTaskContinuousMove();

	bool SetGoal( const AI_LeadArgs_t &args );
	void ClearGoal()										{ m_goal = vec3_origin; m_waitpoint = vec3_origin; m_pSink = NULL; m_weaponname = NULL_STRING; }
	bool HasGoal() const 									{ return (m_goal != vec3_origin); }
	bool HasWaitPoint() const 								{ return (m_waitpoint != vec3_origin); }

	bool Connect( CAI_LeadBehaviorHandler *);
	bool Disconnect( CAI_LeadBehaviorHandler *);

	void SetWaitForWeapon( string_t iszWeaponName ) { m_weaponname = iszWeaponName; m_flWeaponSafetyTimeOut = gpGlobals->curtime + 60; }

	enum
	{
		// Schedules
		SCHED_LEAD_PLAYER = BaseClass::NEXT_SCHEDULE,
		SCHED_LEAD_PAUSE,
		SCHED_LEAD_RETRIEVE,
		SCHED_LEAD_RETRIEVE_WAIT,
		SCHED_LEAD_SUCCEED,
		SCHED_LEAD_AWAIT_SUCCESS,
		SCHED_LEAD_WAITFORPLAYER,
		SCHED_LEAD_PLAYERNEEDSWEAPON,
		SCHED_LEAD_SPEAK_START,
		NEXT_SCHEDULE,
		
		// Tasks
		TASK_GET_PATH_TO_LEAD_GOAL = BaseClass::NEXT_TASK,
		TASK_STOP_LEADING,
		TASK_LEAD_FACE_GOAL,
		TASK_LEAD_ARRIVE,
		TASK_LEAD_SUCCEED,
		TASK_LEAD_GET_PATH_TO_WAITPOINT,
		TASK_LEAD_WAVE_TO_PLAYER,
		TASK_LEAD_PLAYER_NEEDS_WEAPON,
		TASK_LEAD_SPEAK_START,
		TASK_LEAD_MOVE_TO_RANGE,
		TASK_LEAD_RETRIEVE_WAIT,
		NEXT_TASK,
		
		// Conditions
		COND_LEAD_FOLLOWER_LOST = BaseClass::NEXT_CONDITION,
		COND_LEAD_FOLLOWER_LAGGING,
		COND_LEAD_FOLLOWER_NOT_LAGGING,
		COND_LEAD_FOLLOWER_VERY_CLOSE,
		COND_LEAD_SUCCESS,
		COND_LEAD_HAVE_FOLLOWER_LOS,
		COND_LEAD_FOLLOWER_MOVED_FROM_MARK,
		NEXT_CONDITION

	};
	
private:

	void GatherConditions();
	virtual int SelectSchedule();
	virtual void StartTask( const Task_t *pTask );
	virtual void RunTask( const Task_t *pTask );

	bool Speak( AIConcept_t concept );
	bool IsSpeaking();

	// --------------------------------
	//
	// Sink notifiers. Isolated to limit exposure to actual sink storage,
	// provide debugging pinch pount, and allow for class-local logic
	// in addition to sink logic
	//
	void NotifyEvent( int event )								{ if ( m_pSink ) m_pSink->OnEvent( event ) ; }
	const char * GetConceptModifiers( const char *pszConcept )	{ return ( m_pSink ) ? m_pSink->GetConceptModifiers( pszConcept ) : NULL; }
	
	// --------------------------------

	AI_LeadArgs_t			m_args;
	CAI_LeadBehaviorHandler *m_pSink;
	EHANDLE					m_hSinkImplementor;
	
	// --------------------------------
	
	Vector		m_goal;
	float		m_goalyaw;
	Vector		m_waitpoint;
	float		m_waitdistance;
	float		m_leaddistance;
	string_t	m_weaponname;
	bool		m_run;
	bool		m_hasspokenstart;
	bool		m_hasspokenarrival;
	float		m_flSpeakNextNagTime;
	float		m_flWeaponSafetyTimeOut;
	CAI_MoveMonitor m_MoveMonitor;
	
	CRandStopwatch	m_LostTimer;
	CRandStopwatch  m_LostLOSTimer;
	
	DEFINE_CUSTOM_SCHEDULE_PROVIDER;

	DECLARE_DATADESC();
};

//-----------------------------------------------------------------------------

#endif // AI_BEHAVIOR_LEAD_H
