//====== Copyright � 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: Zombies on cars!
//
//=============================================================================

#ifndef AI_BEHAVIOR_PASSENGER_ZOMBIE_H
#define AI_BEHAVIOR_PASSENGER_ZOMBIE_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_behavior_passenger.h"
#include "ai_utils.h"
#include "vehicle_base.h"

extern impactdamagetable_t gZombiePassengerImpactDamageTable;

class CAI_PassengerBehaviorZombie : public CAI_PassengerBehavior
{
	DECLARE_CLASS( CAI_PassengerBehaviorZombie, CAI_PassengerBehavior );
	DECLARE_DATADESC()

public:

	CAI_PassengerBehaviorZombie( void );

	enum
	{
		// Schedules
		SCHED_PASSENGER_ZOMBIE_ENTER_VEHICLE = BaseClass::NEXT_SCHEDULE,
		SCHED_PASSENGER_ZOMBIE_EXIT_VEHICLE,
		SCHED_PASSENGER_MELEE_ATTACK1,
		SCHED_PASSENGER_ZOMBIE_RANGE_ATTACK1,
		NEXT_SCHEDULE,

		// Tasks
		TASK_PASSENGER_ZOMBIE_RANGE_ATTACK1 = BaseClass::NEXT_TASK,
		TASK_PASSENGER_ZOMBIE_DISMOUNT,
		NEXT_TASK,

		// Conditions
		//COND_ = BaseClass::NEXT_CONDITION,
		//NEXT_CONDITION
	};

	virtual const char *GetName( void ) { return "ZombiePassenger"; }
	virtual string_t	GetRoleName( void ) { return MAKE_STRING( "passenger_zombie" ); }
	virtual int			SelectSchedule( void );
	virtual int			TranslateSchedule( int scheduleType );
	virtual void		GatherConditions( void );
	virtual void		Event_Killed( const CTakeDamageInfo &info );
	virtual void		BuildScheduleTestBits( void );
	virtual void		RunTask( const Task_t *pTask );
	virtual void		StartTask( const Task_t *pTask );
	virtual bool		CanEnterVehicle( void );
	virtual void		EnterVehicle( void );
	virtual void		ExitVehicle( void );

	void				SuppressAttack( float flDuration );

	DEFINE_CUSTOM_SCHEDULE_PROVIDER;

protected:
	
	int					SelectOutsideSchedule( void );
	int					SelectInsideSchedule( void );
	virtual	int			FindExitSequence( void );
	void				StartDismount( void );
	void				FinishDismount( void );
	virtual void		CalculateBodyLean( void );
	virtual void		GatherVehicleStateConditions( void );
	inline bool			CanBeOnEnemyVehicle( void );

private:

	bool				EnemyInVehicle( void );
	void				GetAttachmentPoint( Vector *vecPoint );
	bool				CanJumpToAttachToVehicle( void );
	//bool				WithinAttachRange( void );

	float				m_flLastLateralLean;
	float				m_flLastVerticalLean;
	float				m_flNextLeapTime;
};

#endif // AI_BEHAVIOR_PASSENGER_ZOMBIE_H
