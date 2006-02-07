//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef AI_BEHAVIOR_ACTBUSY_H
#define AI_BEHAVIOR_ACTBUSY_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_behavior.h"
#include "ai_goalentity.h"

//-----------------------------------------------------------------------------
enum busyinterrupt_t
{
	BA_INT_NONE,		// Nothing breaks us out of this
	BA_INT_DANGER,		// Only danger signals interrupts this busy anim. The player will be ignored.
	BA_INT_PLAYER,		// The Player's presence interrupts this busy anim
	BA_INT_AMBUSH,		// We're waiting to ambush enemies. Don't break on danger sounds in front of us.
	BA_INT_COMBAT,		// Only break out if we're shot at.
};

enum busyanimparts_t
{
	BA_BUSY,
	BA_ENTRY,
	BA_EXIT,

	BA_MAX_ANIMS,
};

struct busyanim_t
{
	string_t			iszName;
	Activity			iActivities[BA_MAX_ANIMS];
	string_t			iszSequences[BA_MAX_ANIMS];
	float				flMinTime;		// Min time spent in this busy animation
	float				flMaxTime;		// Max time spent in this busy animation. 0 means continue until interrupted.
	busyinterrupt_t		iBusyInterruptType;
};

#define NO_MAX_TIME -1

class CAI_ActBusyGoal;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CAI_ActBusyBehavior : public CAI_SimpleBehavior
{
	DECLARE_CLASS( CAI_ActBusyBehavior, CAI_SimpleBehavior );
public:
	DECLARE_DATADESC();
	CAI_ActBusyBehavior();

	enum
	{
		// Schedules
		SCHED_ACTBUSY_START_BUSYING = BaseClass::NEXT_SCHEDULE,
		SCHED_ACTBUSY_BUSY,
		SCHED_ACTBUSY_STOP_BUSYING,
		SCHED_ACTBUSY_LEAVE,
		SCHED_ACTBUSY_TELEPORT_TO_BUSY,
		NEXT_SCHEDULE,
		
		// Tasks
		TASK_ACTBUSY_PLAY_BUSY_ANIM = BaseClass::NEXT_TASK,
		TASK_ACTBUSY_PLAY_ENTRY,
		TASK_ACTBUSY_PLAY_EXIT,
		TASK_ACTBUSY_TELEPORT_TO_BUSY,
		TASK_ACTBUSY_WALK_PATH_TO_BUSY,
		TASK_ACTBUSY_GET_PATH_TO_ACTBUSY,
		NEXT_TASK,
		
		// Conditions
		//COND_LEAD_FOLLOWER_LOST = BaseClass::NEXT_CONDITION,
		//NEXT_CONDITION
	};
	
	virtual const char *GetName() {	return "ActBusy"; }

	void	Enable( CAI_ActBusyGoal *pGoal, float flRange, bool bVisibleOnly );
	void	SetBusySearchRange( float flRange );
	void	Disable( void );
	void	ForceActBusy( CAI_ActBusyGoal *pGoal, CAI_Hint *pHintNode = NULL, float flMaxTime = NO_MAX_TIME, bool bVisibleOnly = false, bool bTeleportToBusy = false, Activity activity = ACT_INVALID );
	void	ForceActBusyLeave( bool bVisibleOnly = false );
	void	StopBusying( void );
	bool	CanSelectSchedule( void );
	bool	IsCurScheduleOverridable( void );
	bool	ShouldIgnoreSound( CSound *pSound );
	void	GatherConditions( void );
	void	BuildScheduleTestBits( void );
	void	EndScheduleSelection( void );
	void	CheckAndCleanupOnExit( void );
	bool	FValidateHintType( CAI_Hint *pHint );
	bool	ActBusyNodeStillActive( void );
	bool	IsMovingToBusy( void ) { return m_bMovingToBusy; }
	bool	IsEnabled( void ) { return m_bEnabled; }
	float	GetReasonableFacingDist( void ) { return 0; }	// Actbusy ignores reasonable facing
	bool	IsInterruptable( void );
	bool	ShouldPlayerAvoid( void );
	void	SetUseRenderBounds( bool bUseBounds ) { m_bUseRenderBoundsForCollision = bUseBounds; }

	// Returns true if the current NPC is acting busy, or moving to an actbusy
	bool	IsActive( void );

private:
	virtual int		SelectSchedule( void );
	virtual void	StartTask( const Task_t *pTask );
	virtual void	RunTask( const Task_t *pTask );
	void			NotifyBusyEnding( void );
	bool			HasAnimForActBusy( int iActBusy, busyanimparts_t AnimPart );
	bool			PlayAnimForActBusy( busyanimparts_t AnimPart );

private:
	bool			m_bEnabled;
	bool			m_bForceActBusy;
	Activity		m_ForcedActivity;
	bool			m_bTeleportToBusy;
	bool			m_bLeaving;
	bool			m_bVisibleOnly;
	bool			m_bUseRenderBoundsForCollision;
	float			m_flForcedMaxTime;
	bool			m_bBusy;
	bool			m_bMovingToBusy;
	bool			m_bNeedsToPlayExitAnim;
	float			m_flNextBusySearchTime;	
	float			m_flEndBusyAt;
	float			m_flBusySearchRange;
	bool			m_bInQueue;
	int				m_iCurrentBusyAnim;
	CHandle<CAI_ActBusyGoal> m_hActBusyGoal;

	DEFINE_CUSTOM_SCHEDULE_PROVIDER;
};


//-----------------------------------------------------------------------------
// Purpose: A level tool to control the actbusy behavior.
//-----------------------------------------------------------------------------
class CAI_ActBusyGoal : public CAI_GoalEntity
{
	DECLARE_CLASS( CAI_ActBusyGoal, CAI_GoalEntity );
public:
	CAI_ActBusyGoal()
	{
	}

	virtual void NPCMovingToBusy( CAI_BaseNPC *pNPC );
	virtual void NPCAbortedMoveTo( CAI_BaseNPC *pNPC );
	virtual void NPCStartedBusy( CAI_BaseNPC *pNPC );
	virtual void NPCStartedLeavingBusy( CAI_BaseNPC *pNPC );
	virtual void NPCFinishedBusy( CAI_BaseNPC *pNPC );
	virtual void NPCLeft( CAI_BaseNPC *pNPC );

protected:
	CAI_ActBusyBehavior *GetBusyBehaviorForNPC( const char *pszActorName, CBaseEntity *pActivator, const char *sInputName );
	CAI_ActBusyBehavior *GetBusyBehaviorForNPC( CBaseEntity *pEntity, CBaseEntity *pActivator, const char *sInputName );

	void		 EnableGoal( CAI_BaseNPC *pAI );

	// Inputs
	virtual void InputActivate( inputdata_t &inputdata );
	virtual void InputDeactivate( inputdata_t &inputdata );
	void		 InputSetBusySearchRange( inputdata_t &inputdata );
	void		 InputForceNPCToActBusy( inputdata_t &inputdata );
	void		 InputForceThisNPCToActBusy( inputdata_t &inputdata );
	void		 InputForceThisNPCToLeave( inputdata_t &inputdata );

	DECLARE_DATADESC();

protected:
	float			m_flBusySearchRange;
	bool			m_bVisibleOnly;
	COutputEHANDLE	m_OnNPCStartedBusy;
	COutputEHANDLE	m_OnNPCFinishedBusy;
	COutputEHANDLE	m_OnNPCLeft;
};

// Maximum number of nodes allowed in an actbusy queue
#define MAX_QUEUE_NODES		20

//-----------------------------------------------------------------------------
// Purpose: A level tool to control the actbusy behavior to create NPC queues 
//-----------------------------------------------------------------------------
class CAI_ActBusyQueueGoal : public CAI_ActBusyGoal
{
	DECLARE_CLASS( CAI_ActBusyQueueGoal, CAI_ActBusyGoal );
public:
	virtual void Spawn( void );
	virtual void DrawDebugGeometryOverlays( void );
	virtual void NPCMovingToBusy( CAI_BaseNPC *pNPC );
	virtual void NPCStartedBusy( CAI_BaseNPC *pNPC );
	virtual void NPCAbortedMoveTo( CAI_BaseNPC *pNPC );
	virtual void NPCFinishedBusy( CAI_BaseNPC *pNPC );
	virtual void NPCStartedLeavingBusy( CAI_BaseNPC *pNPC );

	virtual void InputActivate( inputdata_t &inputdata );
	void		 InputPlayerStartedBlocking( inputdata_t &inputdata );
	void		 InputPlayerStoppedBlocking( inputdata_t &inputdata );
	void		 InputMoveQueueUp( inputdata_t &inputdata );

	void		 PushNPCBackInQueue( CAI_BaseNPC *pNPC, int iStartingNode );
	void		 RemoveNPCFromQueue( CAI_BaseNPC *pNPC );
	void		 RecalculateQueueCount( void );
	void		 QueueThink( void );
  	void		 MoveQueueUp( void );
  	void		 MoveQueueUpThink( void );
	bool		 NodeIsOccupied( int i );
	CAI_BaseNPC			*GetNPCOnNode( int iNode );
	CAI_ActBusyBehavior *GetQueueBehaviorForNPC( CAI_BaseNPC	*pNPC );

	DECLARE_DATADESC();

private:
	int						m_iCurrentQueueCount;
	CHandle<CAI_Hint>		m_hNodes[ MAX_QUEUE_NODES ];
	bool					m_bPlayerBlockedNodes[ MAX_QUEUE_NODES ];
	EHANDLE					m_hExitNode;
	EHANDLE					m_hExitingNPC;
	bool					m_bForceReachFront;

	// Read from mapdata
	string_t		m_iszNodes[ MAX_QUEUE_NODES ];
	string_t		m_iszExitNode;

	// Outputs
	COutputInt		m_OnQueueMoved;
	COutputEHANDLE	m_OnNPCLeftQueue;
	COutputEHANDLE	m_OnNPCStartedLeavingQueue;
};

#endif // AI_BEHAVIOR_ACTBUSY_H
