//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef AI_BEHAVIOR_FOLLOW_H
#define AI_BEHAVIOR_FOLLOW_H

#include "simtimer.h"
#include "ai_behavior.h"
#include "ai_goalentity.h"
#include "ai_utils.h"
#include "ai_moveshoot.h"

#if defined( _WIN32 )
#pragma once
#endif

//-------------------------------------

enum AI_Formations_t
{
	AIF_SIMPLE,
	AIF_WIDE,
	AIF_ANTLION,
	AIF_COMMANDER,
	AIF_TIGHT,
	AIF_MEDIUM,
};

//-----------------------------------------------------------------------------
//
// CAI_FollowGoal
//
// Purpose: A level tool to control the follow behavior. Use is not required
//			in order to use behavior.
//
//-----------------------------------------------------------------------------

class CAI_FollowGoal : public CAI_GoalEntity
{
	DECLARE_CLASS( CAI_FollowGoal, CAI_GoalEntity );

public:

	virtual void EnableGoal( CAI_BaseNPC *pAI );
	virtual void DisableGoal( CAI_BaseNPC *pAI  );

	int m_iFormation;

	DECLARE_DATADESC();
};

//-----------------------------------------------------------------------------

struct AI_FollowNavInfo_t
{
	Vector  position;
	float 	range;
	float   tolerance;
	float   followPointTolerance;
	float	targetMoveTolerance;
	float	repathOnRouteTolerance;
	float	walkTolerance;
	float	coverTolerance;
	float	enemyLOSTolerance;
	float	chaseEnemyTolerance;

	DECLARE_SIMPLE_DATADESC();
};

DECLARE_POINTER_HANDLE(AI_FollowManagerInfoHandle_t);

//-------------------------------------

struct AI_FollowParams_t
{
	AI_FollowParams_t( AI_Formations_t formation = AIF_SIMPLE )
	 :	formation(formation)
	{
	}
	
	AI_Formations_t formation;
	
	DECLARE_SIMPLE_DATADESC();
};

//-------------------------------------

class CAI_FollowBehavior : public CAI_SimpleBehavior
{
	DECLARE_CLASS( CAI_FollowBehavior, CAI_SimpleBehavior );
public:
	CAI_FollowBehavior( const AI_FollowParams_t &params = AIF_SIMPLE );
	~CAI_FollowBehavior();
	
	void			SetParameters( const AI_FollowParams_t &params );

	virtual const char *GetName() {	return "Follow"; }
	AI_Formations_t GetFormation() const { return m_params.formation; }

	virtual bool 	CanSelectSchedule();

	const AI_FollowNavInfo_t &GetFollowGoalInfo();
	CBaseEntity *	GetFollowTarget();
	void			SetFollowTarget( CBaseEntity *pLeader, bool fFinishCurSchedule = false );

	CAI_FollowGoal *GetFollowGoal()	{ return m_hFollowGoalEnt; } // if any
	bool			SetFollowGoal( CAI_FollowGoal *pGoal, bool fFinishCurSchedule = false );
	void			ClearFollowGoal( CAI_FollowGoal *pGoal );
	void			SetFollowGoalDirect( CAI_FollowGoal *pGoal );

	virtual bool	FarFromFollowTarget()	{ return ( m_hFollowTarget && (GetAbsOrigin() - m_hFollowTarget->GetAbsOrigin()).LengthSqr() > (75*12)*(75*12) ); }

	virtual bool	TargetIsUnreachable() { return m_bTargetUnreachable; }
	
	int				GetNumFailedFollowAttempts()	{ return m_nFailedFollowAttempts; }
	float			GetTimeFailFollowStarted()		{ return m_flTimeFailFollowStarted; }
	bool			FollowTargetVisible() { return HasCondition( COND_FOLLOW_TARGET_VISIBLE ); };

	bool			IsMovingToFollowTarget();

	float			GetGoalRange();

private:
	friend class CAI_FollowManager;

	virtual void	BeginScheduleSelection();
	virtual void	EndScheduleSelection();
	
	virtual void	CleanupOnDeath( CBaseEntity *pCulprit, bool bFireDeathOutput );

	virtual void	Precache();
	virtual void 	GatherConditions();
	virtual int		SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );
	virtual int		SelectSchedule();
	virtual int		TranslateSchedule( int scheduleType );
	virtual void	OnStartSchedule( int scheduleType );
	virtual void	StartTask( const Task_t *pTask );
	virtual void	RunTask( const Task_t *pTask );
	virtual void	TaskComplete( bool fIgnoreSetFailedCondition = false );
	void			BuildScheduleTestBits();
	virtual Activity NPC_TranslateActivity( Activity activity );

	bool			IsCurScheduleFollowSchedule();

	virtual bool	IsCurTaskContinuousMove();
	virtual void	OnMovementFailed();
	virtual void	OnMovementComplete();
	virtual bool	FValidateHintType( CAI_Hint *pHint );
	
	bool			IsValidEnemy(CBaseEntity *pEnemy);
	bool			IsValidCover( const Vector &vLocation, CAI_Hint const *pHint );
	bool			IsValidShootPosition( const Vector &vLocation, CAI_Node *pNode, CAI_Hint const *pHint );
	bool 			FindCoverFromEnemyAtFollowTarget( float coverRadius, Vector *pResult );
	
	bool			ShouldAlwaysThink();

	bool			ShouldMoveToFollowTarget();

	int 			SelectScheduleManagePosition();
	int 			SelectScheduleFollowPoints();
	int 			SelectScheduleMoveToFormation();

	void			GetFollowTargetViewLoc( Vector *pResult);
	bool			ValidateFaceTarget(  Vector *pFaceTarget );

	//----------------------------

	bool			ShouldUseFollowPoints();
	bool			HasFollowPoint();
	void 			SetFollowPoint( CAI_Hint *pHintNode );
	void			ClearFollowPoint();
	const Vector &	GetFollowPoint();
	CAI_Hint *		FindFollowPoint();
	bool			IsFollowPointInRange();
	bool			ShouldIgnoreFollowPointFacing();

	//----------------------------
	
	virtual bool	ShouldFollow();
	bool 			UpdateFollowPosition();
	const Vector &	GetGoalPosition();
	float 			GetGoalTolerance();
	bool			PlayerIsPushing();

	bool			IsFollowTargetInRange();

	bool			IsFollowGoalInRange( float tolerance, float zTolerance = -1.0 ); // zTolerance defaults to hull height
	virtual bool	IsChaseGoalInRange();

	void			NoteFailedFollow();
	void			NoteSuccessfulFollow();

	//----------------------------

	enum
	{
		SCHED_FOLLOWER_MOVE_AWAY_FAIL = BaseClass::NEXT_SCHEDULE,								// Turn back toward player
		SCHED_FOLLOWER_MOVE_AWAY_END,
		SCHED_FOLLOW,
		SCHED_FOLLOWER_IDLE_STAND,
		SCHED_MOVE_TO_FACE_FOLLOW_TARGET,
		SCHED_FACE_FOLLOW_TARGET,
		SCHED_FOLLOWER_GO_TO_WAIT_POINT,
		SCHED_FOLLOWER_GO_TO_WAIT_POINT_FAIL,
		SCHED_FOLLOWER_STAND_AT_WAIT_POINT,
		NEXT_SCHEDULE,

		TASK_CANT_FOLLOW = BaseClass::NEXT_TASK,
		TASK_FACE_FOLLOW_TARGET,
		TASK_MOVE_TO_FOLLOW_POSITION,
		TASK_GET_PATH_TO_FOLLOW_POSITION,
		TASK_SET_FOLLOW_TARGET_MARK,
		TASK_FOLLOWER_FACE_TACTICAL,
		TASK_SET_FOLLOW_DELAY,
		TASK_GET_PATH_TO_FOLLOW_POINT,
		TASK_ARRIVE_AT_FOLLOW_POINT,
		TASK_SET_FOLLOW_POINT_STAND_SCHEDULE,
		TASK_BEGIN_STAND_AT_WAIT_POINT,
		NEXT_TASK,

		COND_TARGET_MOVED_FROM_MARK = BaseClass::NEXT_CONDITION,
		COND_FOUND_WAIT_POINT,
		COND_FOLLOW_DELAY_EXPIRED,
		COND_FOLLOW_TARGET_VISIBLE,
		COND_FOLLOW_TARGET_NOT_VISIBLE,
		COND_FOLLOW_WAIT_POINT_INVALID,
		NEXT_CONDITION,
	};

	DEFINE_CUSTOM_SCHEDULE_PROVIDER;
	
	//----------------------------
	
	EHANDLE 		   				m_hFollowTarget;
	AI_FollowNavInfo_t 				m_FollowNavGoal;
	float							m_flTimeUpdatedFollowPosition;
	bool							m_bFirstFacing;
	float							m_flTimeFollowTargetVisible;
	
	CAI_MoveMonitor	   				m_TargetMonitor;
	bool							m_bTargetUnreachable;

	int								m_nFailedFollowAttempts;
	float							m_flTimeFailFollowStarted;
	Vector							m_vFollowMoveAnchor;

	bool							m_bMovingToCover;
	
	CRandStopwatch	   				m_FollowDelay;
	
	//---------------------------------

	Activity						m_CurrentFollowActivity;

	//---------------------------------
	
	CRandSimTimer					m_TimeBlockUseWaitPoint;
	CSimTimer						m_TimeCheckForWaitPoint;
	CAI_Hint *						m_pInterruptWaitPoint;
	
	//---------------------------------

	CRandSimTimer					m_TimeBeforeSpreadFacing;
	CRandSimTimer					m_TimeNextSpreadFacing;

	//---------------------------------
	
	AI_FollowManagerInfoHandle_t 	m_hFollowManagerInfo;
	AI_FollowParams_t				m_params;

	//---------------------------------
	
	CHandle<CAI_FollowGoal>			m_hFollowGoalEnt;
	
	//---------------------------------
	
	DECLARE_DATADESC();
};

//-------------------------------------

inline const AI_FollowNavInfo_t &CAI_FollowBehavior::GetFollowGoalInfo()
{
	return m_FollowNavGoal;
}

//-------------------------------------

inline const Vector &CAI_FollowBehavior::GetGoalPosition()
{
	return m_FollowNavGoal.position;
}

//-------------------------------------

inline float CAI_FollowBehavior::GetGoalTolerance()
{
	return m_FollowNavGoal.tolerance;
}

//-------------------------------------

inline float CAI_FollowBehavior::GetGoalRange()
{
	return m_FollowNavGoal.range;
}

//-------------------------------------

inline bool CAI_FollowBehavior::IsFollowGoalInRange( float tolerance, float zTolerance )
{
	const Vector &origin = WorldSpaceCenter();
	const Vector &goal = GetGoalPosition();
	if ( zTolerance == -1 )
		zTolerance = GetHullHeight();
	if ( fabs( origin.z - goal.z ) > zTolerance )
		return false;
	float distanceSq = ( goal.AsVector2D() - origin.AsVector2D() ).LengthSqr();
	tolerance += 0.1;

	return ( distanceSq < tolerance*tolerance );
}

//-----------------------------------------------------------------------------

#endif // AI_BEHAVIOR_FOLLOW_H
