//====== Copyright � 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef AI_BEHAVIOR_PASSENGER_H
#define AI_BEHAVIOR_PASSENGER_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_behavior.h"
#include "ai_utils.h"

#ifdef HL2_EPISODIC
#include "vehicle_jeep_episodic.h"
#endif //HL2_EPISODIC

#define STOPPED_VELOCITY_THRESHOLD		32.0f
#define	STOPPED_VELOCITY_THRESHOLD_SQR	(STOPPED_VELOCITY_THRESHOLD*STOPPED_VELOCITY_THRESHOLD)

// Custom activities
extern int ACT_PASSENGER_IDLE;
extern int ACT_PASSENGER_RANGE_ATTACK1;

// ---------------------------------------------
//  Vehicle state
// ---------------------------------------------
struct passengerVehicleState_t
{
	bool	m_bWasBoosting;
	bool	m_bWasOverturned;
	Vector	m_vecLastLocalVelocity;
	Vector	m_vecDeltaVelocity;
	QAngle	m_vecLastAngles;
	float	m_flNextWarningTime;
	float	m_flLastSpeed;

	DECLARE_SIMPLE_DATADESC();
};

// ---------------------------------------------
//  Passenger intent
// ---------------------------------------------
enum passesngerVehicleIntent_e
{
	PASSENGER_INTENT_NONE,
	PASSENGER_INTENT_ENTER,		// We want to be in the vehicle
	PASSENGER_INTENT_EXIT,		// We want to be outside the vehicle
};

// ---------------------------------------------
//  Passenger state functions
// ---------------------------------------------
enum PassengerState_e
{
	PASSENGER_STATE_OUTSIDE = 0,	// Not in the vehicle
	PASSENGER_STATE_ENTERING,
	PASSENGER_STATE_INSIDE,
	PASSENGER_STATE_EXITING,
};

class CAI_PassengerBehavior : public CAI_SimpleBehavior
{
	DECLARE_CLASS( CAI_PassengerBehavior, CAI_SimpleBehavior );
	DECLARE_DATADESC()

public:

	CAI_PassengerBehavior( void );

	enum
	{
		// Schedules
		SCHED_PASSENGER_IDLE = BaseClass::NEXT_SCHEDULE,
		SCHED_PASSENGER_ENTER_VEHICLE,
		SCHED_PASSENGER_EXIT_VEHICLE,
		SCHED_PASSENGER_RUN_TO_ENTER_VEHICLE,
		SCHED_PASSENGER_ENTER_VEHICLE_PAUSE,
		SCHED_PASSENGER_RUN_TO_ENTER_VEHICLE_FAILED,
		NEXT_SCHEDULE,

		// Tasks
		TASK_PASSENGER_ENTER_VEHICLE = BaseClass::NEXT_TASK,
		TASK_PASSENGER_EXIT_VEHICLE,
		TASK_PASSENGER_ATTACH_TO_VEHICLE,
		TASK_PASSENGER_DETACH_FROM_VEHICLE,
		TASK_PASSENGER_SET_IDEAL_ENTRY_YAW,
		NEXT_TASK,

		// Conditions
		COND_VEHICLE_HARD_IMPACT = BaseClass::NEXT_CONDITION,
		COND_ENTERING_VEHICLE,
		COND_EXITING_VEHICLE,
		COND_VEHICLE_MOVED_FROM_MARK,
		COND_VEHICLE_STOPPED,
		COND_VEHICLE_OVERTURNED,
		COND_CANCEL_ENTER_VEHICLE,

		NEXT_CONDITION
	};

	virtual bool	CanSelectSchedule( void );
	virtual int		SelectSchedule( void );
	virtual int		SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );
	virtual void	RunTask( const Task_t *pTask );
	virtual void	StartTask( const Task_t *pTask );
	virtual	void	BuildScheduleTestBits( void );
	virtual int		TranslateSchedule( int scheduleType );
	virtual void	GetEntryTarget( Vector *vecOrigin, QAngle *vecAngles );
	virtual void	GatherConditions( void );

	virtual const char *GetName( void ) { return "Passenger"; }
	virtual string_t GetRoleName( void ) { return MAKE_STRING( "passenger" ); }

	// Enable/disable code
#ifdef HL2_EPISODIC
	void	Enable( CPropJeepEpisodic *pVehicle );
#endif //HL2_EPISODIC
	void	Disable( void );
	bool	IsEnabled( void ) const { return m_bEnabled; }

	virtual void	EnterVehicle( void );
	virtual void	ExitVehicle( void );

	void	AddPhysicsPush( float force );

	CPropVehicleDriveable *GetTargetVehicle( void ) const { return m_hVehicle; }

	PassengerState_e	GetPassengerState( void ) const { return m_PassengerState; }

protected:
	
	void			SetTransitionSequence( int nSequence );

	virtual void	OnExitVehicleFailed( void ) { }	// NPC attempted to leave vehicle, but was unable to
	virtual void	GatherVehicleStateConditions( void );

	// ------------------------------------------
	//  Entry/exit transition code
	// ------------------------------------------
	
	virtual void	FinishEnterVehicle( void );
	virtual void	FinishExitVehicle( void );

	void	DrawDebugTransitionInfo( const Vector &vecIdealPos, const QAngle &vecIdealAngles, const Vector &vecAnimPos, const QAngle &vecAnimAngles );
	void	FixInterpolation( void );
	void	GetEntryPoint( int nSequence, Vector *vecEntryPoint, QAngle *vecEntryAngles = NULL );
	bool	GetExitPoint( int nSequence, Vector *vecExitPoint, QAngle *vecExitAngles = NULL );
	bool	ReserveEntryPoint( VehicleSeatQuery_e eSeatSearchType );
	bool	ReserveExitPoint( void );
	bool	FindGroundAtPosition( const Vector &in, float flUpDelta, float flDownDelta, Vector *out );
	bool	DoTransitionMovement( void );
	bool	GetSequenceBlendAmount( float flCycle, float *posBlend, float *angBlend );
	bool	LocalIntervalMovement( float flInterval, bool &bMoveSeqFinished, Vector &newPosition, QAngle &newAngles );
	void	GetTransitionAnimationIdeal( float flCycle, const Vector &vecTargetPos, const QAngle &vecTargetAngles, Vector *idealOrigin, QAngle *idealAngles );
	float	GetNextCycleForInterval( int nSequence, float flInterval );
	void	GetLocalVehicleVelocity( Vector *pOut  );
	void	CacheBlendTargets( void );

	int		FindEntrySequence( bool bNearest = false );
	int		FindExitSequence( void );
	bool	IsValidTransitionPoint( const Vector &vecStartPos, const Vector &vecEndPos );

	void	SetPassengerState( PassengerState_e state ) { m_PassengerState = state; }

	PassengerState_e	m_PassengerState;	// State we're in, for the vehicle

	// ---------------------------------------------

	bool	IsPassengerHostile( void );

	passengerVehicleState_t			m_vehicleState;			// Internal vehicle state

	CHandle	<CPropVehicleDriveable>	m_hVehicle;				// The vehicle we're bound to
	CHandle <CEntityBlocker>		m_hBlocker;				// Blocking entity for space reservation
	Vector							m_vecTargetPosition;	// Target destination for exiting the vehicle
	QAngle							m_vecTargetAngles;		// Target angles for exiting the vehicle
	bool							m_bEnabled;				// If the behavior is running
	passesngerVehicleIntent_e		m_PassengerIntent;		// Gives us information about whether we're meant to get in/out, etc.

	int								m_nTransitionSequence;		// Animation we're using to transition with
	float							m_flOriginStartFrame;
	float							m_flOriginEndFrame;
	float							m_flAnglesStartFrame;
	float							m_flAnglesEndFrame;

protected:
	DEFINE_CUSTOM_SCHEDULE_PROVIDER;
};

#endif // AI_BEHAVIOR_PASSENGER_H
