//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Base class for humanoid NPCs intended to fight along side player in close
// environments
//
//=============================================================================//

#ifndef NPC_PLAYERCOMPANION_H
#define NPC_PLAYERCOMPANION_H

#include "ai_playerally.h"

#include "ai_behavior_follow.h"
#include "ai_behavior_standoff.h"
#include "ai_behavior_assault.h"
#include "ai_behavior_lead.h"
#include "ai_behavior_actbusy.h"

#if defined( _WIN32 )
#pragma once
#endif

enum AIReadiness_t
{
	AIRL_RELAXED = 0,
	AIRL_STIMULATED,
	AIRL_AGITATED,
};

enum AIReadinessUse_t
{
	AIRU_NEVER,
	AIRU_ALWAYS,
	AIRU_ONLY_PLAYER_SQUADMATES,
};

#define READINESS_VALUE_RELAXED		0.1f
#define READINESS_VALUE_STIMULATED	0.95f
#define READINESS_VALUE_AGITATED	1.0f


//-----------------------------------------------------------------------------
//
// CLASS: CNPC_PlayerCompanion
//
//-----------------------------------------------------------------------------

class CNPC_PlayerCompanion : public CAI_PlayerAlly
{
	DECLARE_CLASS( CNPC_PlayerCompanion, CAI_PlayerAlly );

public:
	//---------------------------------
	bool			CreateBehaviors();
	void			Precache();
	void			Spawn();
	virtual void	SelectModel() {};

	virtual int		Restore( IRestore &restore );

	//---------------------------------
	int 			ObjectCaps();
	bool 			ShouldAlwaysThink();

	Disposition_t	IRelationType( CBaseEntity *pTarget );
	
	bool			IsSilentSquadMember() const;

	//---------------------------------
	// Behavior
	//---------------------------------
	void 			GatherConditions();
	virtual void	PredictPlayerPush();
	void			BuildScheduleTestBits();

	CSound			*GetBestSound( int validTypes = ALL_SOUNDS );
	bool			QueryHearSound( CSound *pSound );
	bool			ShouldIgnoreSound( CSound * );
	
	int 			SelectSchedule();

	virtual int 	SelectScheduleDanger();
	virtual int 	SelectSchedulePriorityAction();
	virtual int 	SelectScheduleNonCombat()			{ return SCHED_NONE; }
	virtual int 	SelectScheduleCombat();
	int 			SelectSchedulePlayerPush();

	virtual bool	ShouldDeferToFollowBehavior();

	bool			IsValidReasonableFacing( const Vector &vecSightDir, float sightDist );
	
	int 			TranslateSchedule( int scheduleType );
	
	void 			StartTask( const Task_t *pTask );
	void 			RunTask( const Task_t *pTask );
	
	Activity		TranslateActivityReadiness( Activity activity );
	Activity		NPC_TranslateActivity( Activity eNewActivity );
	void 			HandleAnimEvent( animevent_t *pEvent );

	int				GetSoundInterests();
	
	void 			Touch( CBaseEntity *pOther );

	virtual bool	IgnorePlayerPushing( void ) { return false; }

	void			ModifyOrAppendCriteria( AI_CriteriaSet& set );

	//---------------------------------
	// Readiness
	//---------------------------------

protected:
	virtual bool	IsReadinessCapable();
	bool			IsReadinessLocked() { return gpGlobals->curtime < m_flReadinessLockedUntil; }
	void			AddReadiness( float flAdd, bool bOverrideLock = false );
	void			SubtractReadiness( float flAdd, bool bOverrideLock = false );
	void			SetReadinessValue( float flSet );
	void			SetReadinessSensitivity( float flSensitivity ) { m_flReadinessSensitivity = flSensitivity; }
	virtual void	UpdateReadiness();

public:
	float			GetReadinessValue()	{ return m_flReadiness; }
	int				GetReadinessLevel();
	void			SetReadinessLevel( int iLevel, bool bOverrideLock, bool bSlam );

	//---------------------------------
	//---------------------------------
	bool PickTacticalLookTarget( AILookTargetArgs_t *pArgs );

	//---------------------------------
	// Aiming
	//---------------------------------
	CBaseEntity		*GetAimTarget() { return m_hAimTarget; }
	void			SetAimTarget( CBaseEntity *pTarget );
	void			StopAiming( char *pszReason = NULL );
	bool			FindNewAimTarget();
	void			OnNewLookTarget();
	bool			ShouldBeAiming();
	bool			IsAllowedToAim();
	bool			HasAimLOS( CBaseEntity *pAimTarget );
	void			AimGun();
	CBaseEntity		*GetAlternateMoveShootTarget();

	//---------------------------------
	// Combat
	//---------------------------------
	virtual void 	LocateEnemySound() {};

	bool			IsValidEnemy( CBaseEntity *pEnemy );

	bool 			IsSafeFromFloorTurret( const Vector &vecLocation, CBaseEntity *pTurret );

	bool			ShouldMoveAndShoot( void );
	void			OnUpdateShotRegulator();

	void			DecalTrace( trace_t *pTrace, char const *decalName );
	bool 			FCanCheckAttacks();
	Vector 			GetActualShootPosition( const Vector &shootOrigin );
	WeaponProficiency_t CalcWeaponProficiency( CBaseCombatWeapon *pWeapon );
	bool			Weapon_CanUse( CBaseCombatWeapon *pWeapon );
	void			Weapon_Equip( CBaseCombatWeapon *pWeapon );
	
	bool 			FindCoverPos( CBaseEntity *pEntity, Vector *pResult);
	bool			FindCoverPosInRadius( CBaseEntity *pEntity, const Vector &goalPos, float coverRadius, Vector *pResult );
	bool			FindCoverPos( CSound *pSound, Vector *pResult );
	bool			FindMortarCoverPos( CSound *pSound, Vector *pResult );
	bool 			IsCoverPosition( const Vector &vecThreat, const Vector &vecPosition );

	bool			IsEnemyTurret() { return ( GetEnemy() && IsTurret(GetEnemy()) ); }
	
	static bool		IsMortar( CBaseEntity *pEntity );
	static bool		IsSniper( CBaseEntity *pEntity );
	static bool		IsTurret(  CBaseEntity *pEntity );
	
	//---------------------------------
	// Damage handling
	//---------------------------------
	int 			OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void 			OnFriendDamaged( CBaseCombatCharacter *pSquadmate, CBaseEntity *pAttacker );

	//---------------------------------
	// Hints
	//---------------------------------
	bool			FValidateHintType ( CAI_Hint *pHint );

	//---------------------------------
	// Navigation
	//---------------------------------
	bool			IsValidMoveAwayDest( const Vector &vecDest );
	bool 			ValidateNavGoal();
	bool 			OverrideMove( float flInterval );				// Override to take total control of movement (return true if done so)
	bool			MovementCost( int moveType, const Vector &vecStart, const Vector &vecEnd, float *pCost );
	float			GetIdealSpeed() const;
	float			GetIdealAccel() const;
	bool			OnObstructionPreSteer( AILocalMoveGoal_t *pMoveGoal, float distClear, AIMoveResult_t *pResult );

	//---------------------------------
	// Inputs
	//---------------------------------
	void 			InputOutsideTransition( inputdata_t &inputdata );
	void			InputSetReadinessLow( inputdata_t &inputdata );
	void			InputSetReadinessMedium( inputdata_t &inputdata );
	void			InputSetReadinessHigh( inputdata_t &inputdata );
	void			InputLockReadiness( inputdata_t &inputdata );

protected:
	//-----------------------------------------------------
	// Conditions, Schedules, Tasks
	//-----------------------------------------------------
	enum
	{
		COND_PC_HURTBYFIRE = BaseClass::NEXT_CONDITION,
		COND_PC_SAFE_FROM_MORTAR,
		NEXT_CONDITION,

		SCHED_PC_COWER = BaseClass::NEXT_SCHEDULE,
		SCHED_PC_TAKE_COVER_FROM_BEST_SOUND,
		SCHED_PC_FLEE_FROM_BEST_SOUND,
		SCHED_PC_FAIL_TAKE_COVER_TURRET,
		SCHED_PC_FAKEOUT_MORTAR,
		SCHED_PC_GET_OFF_COMPANION,
		NEXT_SCHEDULE,

		TASK_PC_WAITOUT_MORTAR = BaseClass::NEXT_TASK,
		TASK_PC_GET_PATH_OFF_COMPANION,
		NEXT_TASK,
	};

private:
	void SetupCoverSearch( CBaseEntity *pEntity );
	void CleanupCoverSearch();

	//-----------------------------------------------------
	
	bool			m_bMovingAwayFromPlayer;
	bool			m_bWeightPathsInCover;

	enum eCoverType
	{
		CT_NORMAL,
		CT_TURRET,
		CT_MORTAR
	};

	static eCoverType	gm_fCoverSearchType;
	static bool 		gm_bFindingCoverFromAllEnemies;

	CSimpleSimTimer		m_FakeOutMortarTimer;

	// Derived classes should not use the expresser directly
	virtual CAI_Expresser *GetExpresser()	{ return BaseClass::GetExpresser(); }

protected:
	//-----------------------------------------------------

	CAI_AssaultBehavior		m_AssaultBehavior;
	CAI_FollowBehavior		m_FollowBehavior;
	CAI_StandoffBehavior	m_StandoffBehavior;
	CAI_LeadBehavior		m_LeadBehavior;
	CAI_ActBusyBehavior		m_ActBusyBehavior;

	//-----------------------------------------------------

	// Readiness is a value that's fed by various events in the NPC's AI. It is used
	// to make decisions about what type of posture the NPC should be in (relaxed, agitated).
	// It is not used to make decisions about what to do in the AI. 
	float m_flReadiness;
	float m_flReadinessSensitivity;
	bool m_bReadinessCapable;
	float m_flReadinessLockedUntil;

	//-----------------------------------------------------

	float m_flBoostSpeed;

	//-----------------------------------------------------
	
	CSimpleSimTimer m_AnnounceAttackTimer;

	//-----------------------------------------------------

	EHANDLE m_hAimTarget;

	//-----------------------------------------------------

	static string_t gm_iszMortarClassname;
	static string_t gm_iszFloorTurretClassname;
	static string_t gm_iszGroundTurretClassname;
	static string_t gm_iszShotgunClassname;

	//-----------------------------------------------------

	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;
};

#endif // NPC_PLAYERCOMPANION_H
