//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Base combat character with no AI
//
//=====================================================================================//

#include "ai_baseactor.h"
#include "npc_playercompanion.h"
#include "ai_behavior_holster.h"
#include "ai_behavior_functank.h"
#include "soundenvelope.h"

extern ConVar npc_alyx_readiness;

class CNPC_Alyx : public CNPC_PlayerCompanion
{
public:
	DECLARE_CLASS( CNPC_Alyx, CNPC_PlayerCompanion );

	virtual bool	IsHeavyDamage( const CTakeDamageInfo &info );
	virtual void	ModifyOrAppendCriteria( AI_CriteriaSet &set );

	static CNPC_Alyx *GetAlyx( void );

	bool	CreateBehaviors();
	void	Spawn( void );
	void	Activate( void );
	void	StopLoopingSounds( void );
	void	SelectModel();
	void	Precache( void );
	void	SetupAlyxWithoutParent( void );
	void	CreateEmpTool( void );
	void	PrescheduleThink( void );
	void	GatherConditions();
	bool	ShouldPlayerAvoid( void );
	void	AnalyzeGunfireSound( CSound *pSound );
	bool	IsValidEnemy( CBaseEntity *pEnemy );
	void	Event_KilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info );
	void	Event_Killed( const CTakeDamageInfo &info );
	void	EnemyIgnited( CAI_BaseNPC *pVictim );
	void	CombineBallSocketed( int iNumBounces );
	void	PlayerKilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info );
	void	AimGun( void );
	Vector  GetActualShootPosition( const Vector &shootOrigin );
	float	MaxYawSpeed( void );
	void	OnUpdateShotRegulator();
	bool	IsCrouchedActivity( Activity activity );
	bool	OnBeginMoveAndShoot();
	void	SpeakAttacking( void );

	virtual float	GetJumpGravity() const		{ return 1.8f; }

	// Crouching
	Vector  GetCrouchEyeOffset( void ) { return Vector(0,0,50); }
	Vector  GetCrouchGunOffset( void ) { return Vector(0,0,40); }
	bool	EnemyIsValidCrouchTarget( CBaseEntity *pEnemy );
	bool	Stand( void );
	bool	Crouch( void );
	void	DesireCrouch( void );

	// Custom AI
	void	DoCustomCombatAI( void );
	void	DoMobbedCombatAI( void );
	void	DoCustomSpeechAI( void );

	Disposition_t	IRelationType( CBaseEntity *pTarget );
	int				IRelationPriority( CBaseEntity *pTarget );

	Class_T Classify ( void );
	bool	FValidateHintType( CAI_Hint *pHint );
	int		ObjectCaps();
	void	HandleAnimEvent( animevent_t *pEvent );
	bool	FInViewCone( CBaseEntity *pEntity );
	bool	QuerySeeEntity( CBaseEntity *pEntity, bool bOnlyHateOrFearIfNPC = false );
	bool	CanSeeEntityInDarkness( CBaseEntity *pEntity );
	bool	IsCoverPosition( const Vector &vecThreat, const Vector &vecPosition );
	Activity NPC_TranslateActivity ( Activity activity );
	bool	ShouldDeferToFollowBehavior();
	void	BuildScheduleTestBits();
	int		SelectSchedule( void );
	int		SelectScheduleDanger( void );
	int		TranslateSchedule( int scheduleType );
	void	StartTask( const Task_t *pTask );
	void	RunTask( const Task_t *pTask );
	void	OnStateChange( NPC_STATE OldState, NPC_STATE NewState );
	float	LengthOfLastCombat( void ) const;
	bool	IsNavigationUrgent();

	void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	bool	CanBeHitByMeleeAttack( CBaseEntity *pAttacker );
	int		OnTakeDamage_Alive( const CTakeDamageInfo &info );
	bool	FCanCheckAttacks();

	bool	HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);
	virtual bool SpeakIfAllowed( AIConcept_t concept, const char *modifiers = NULL, bool bRespondingToPlayer = false, char *pszOutResponseChosen = NULL, size_t bufsize = 0 );
	
	void	HolsterPistol();
	void	DrawPistol();
	void	Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget = NULL, const Vector *pVelocity = NULL );

	void	SetEMPHolstered( bool bHolstered ) { m_bIsEMPHolstered = bHolstered; }
	bool	IsEMPHolstered() { return m_bIsEMPHolstered; }

	float	GetReadinessDecay() { return 30.0f; }
	virtual bool	IsAllowedToAim();

	virtual void	PainSound( const CTakeDamageInfo &info );
    virtual void	DeathSound( const CTakeDamageInfo &info );

	// Hacking and object interaction
	void	SearchForInteractTargets();
	bool	IsValidInteractTarget( CBaseEntity *pTarget );
	bool	CanInteractWithTarget( CBaseEntity *pTarget );
	void	SetInteractTarget( CBaseEntity *pTarget );
	bool	HasInteractTarget() { return m_hHackTarget != NULL; }
	CBaseEntity *GetInteractTarget() { return m_hHackTarget; }
	void	EmpZapTarget( CBaseEntity *pTarget );

	virtual void OnSeeEntity( CBaseEntity *pEntity );

	void InputAllowInteraction( inputdata_t &inputdata )
	{
		m_bInteractionAllowed = true;
	}
	void InputDisallowInteraction( inputdata_t &inputdata )
	{
		m_bInteractionAllowed = false;
	}
	void InputAllowDarknessSpeech( inputdata_t &inputdata )
	{
		m_bDarknessSpeechAllowed = inputdata.value.Bool();
	}

	virtual void	OnGivenWeapon( CBaseCombatWeapon *pNewWeapon );
	virtual void	Weapon_Equip( CBaseCombatWeapon *pWeapon );
	virtual bool	Weapon_CanUse( CBaseCombatWeapon *pWeapon );

	// Blinding
	virtual void PlayerHasIlluminatedNPC( CBasePlayer *pPlayer, float flDot );
	void		 CheckBlindedByFlare( void );
	bool		 CanBeBlindedByFlashlight( bool bCheckLightSources );
	bool		 PlayerFlashlightOnMyEyes( CBasePlayer *pPlayer );
	bool		 BlindedByFlare( void );
	bool		 CanReload( void );

	virtual bool PickTacticalLookTarget( AILookTargetArgs_t *pArgs );
	virtual void OnSelectedLookTarget( AILookTargetArgs_t *pArgs );
	virtual bool IsReadinessCapable( void );

	virtual	void ReadinessLevelChanged( int iPriorLevel );

	bool IsAllowedToInteract();
	virtual void	BarnacleDeathSound( void );

	virtual const char *GetDeathMessageText( void ) { return "GAMEOVER_ALYXDEAD"; }

	PassengerState_e	GetPassengerState( void );

	void				Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	bool				PlayerInSpread( const Vector &sourcePos, const Vector &targetPos, float flSpread, float maxDistOffCenter, bool ignoreHatedPlayers );

private:
	EHANDLE	m_hEmpTool;
	EHANDLE	m_hHackTarget;
	CHandle<CAI_Hint>	m_hStealthLookTarget;
	bool	m_bInteractionAllowed;
	float	m_fTimeNextSearchForInteractTargets;
	bool	m_bDarknessSpeechAllowed;
	bool	m_bIsEMPHolstered;
	bool	m_bIsFlashlightBlind;
	float	m_fStayBlindUntil;
	float	m_flDontBlindUntil;
	bool	m_bSpokeLostPlayerInDarkness;
	bool	m_bPlayerFlashlightState;
	bool	m_bHadCondSeeEnemy;
	string_t m_iszCurrentBlindScene;
	float	m_fTimeUntilNextDarknessFoundPlayer;
	float   m_fCombatStartTime;
	float	m_fCombatEndTime;
	float	m_fLastPlayerKill;
	int		m_iNumConsecutivePlayerKills;  // Alyx keeps track of the # of consecutive kills by the player and speaks a response as it increases
	float	m_fLastBarrelExploded;
	int		m_iNumConsecutiveBarrelsExploded;  // Alyx keeps track of the # of consecutive barrels exploded by the player and speaks a response as it increases
	float	m_flNextCrouchTime;

	CSoundPatch		*m_sndDarknessBreathing;

	// Speech timers
	// Theoretically, these shouldn't be needed. Instead, each response
	// should prevent the concept being spoken for the desired time. But
	// until the responses exists, Alyx will spam the response rules forever,
	// so these timers stop that.
	CRandStopwatch  m_SpeechWatch_LostPlayer;
	CSimpleSimTimer m_SpeechTimer_HeardSound;
	CRandStopwatch	m_SpeechWatch_SoundDelay;
	CRandStopwatch	m_SpeechWatch_BreathingRamp;
	CRandStopwatch	m_SpeechWatch_FoundPlayer;
	CStopwatch		m_SpeechWatch_PlayerLooking;

	CAI_MoveMonitor m_MoveMonitor;

	bool m_bHasShotgun;

	CAI_FuncTankBehavior	m_FuncTankBehavior;

	COutputEvent			m_OnFinishInteractWithObject;
	COutputEvent			m_OnPlayerUse;

	virtual CAI_FollowBehavior &GetFollowBehavior( void );
	bool	RunningPassengerBehavior( void );
	void	MaintainFollowBehaviorState( void );

private:
	enum
	{
		COND_ALYX_HAS_INTERACT_TARGET = BaseClass::NEXT_CONDITION,
		COND_ALYX_NO_INTERACT_TARGET,
		COND_ALYX_CAN_INTERACT_WITH_TARGET, // Hack target is in a suitable state and location to hack
		COND_ALYX_CAN_NOT_INTERACT_WITH_TARGET,
		COND_ALYX_PLAYER_TURNED_ON_FLASHLIGHT,
		COND_ALYX_PLAYER_TURNED_OFF_FLASHLIGHT,
		COND_ALYX_PLAYER_FLASHLIGHT_EXPIRED,
		COND_ALYX_IN_DARK,					// lights are out and she can't see
	};

	enum
	{
		SCHED_ALYX_PREPARE_TO_INTERACT_WITH_TARGET = BaseClass::NEXT_SCHEDULE,
		SCHED_ALYX_WAIT_TO_INTERACT_WITH_TARGET,
		SCHED_ALYX_INTERACT_WITH_TARGET,
		SCHED_ALYX_INTERACTION_INTERRUPTED,
		SCHED_ALYX_FINISH_INTERACTING_WITH_TARGET,
		SCHED_ALYX_HOLSTER_EMP,
		SCHED_ALYX_ALERT_FACE_AWAYFROM_BESTSOUND,
		SCHED_ALYX_RANGE_ATTACK1,
		SCHED_ALYX_ALERT_REACT_TO_COMBAT_SOUND,
		SCHED_ALYX_COMBAT_FACE,
		SCHED_ALYX_WAKE_ANGRY,
		SCHED_ALYX_NEW_WEAPON,
		SCHED_ALYX_IDLE_STAND,
		SCHED_ALYX_ALERT_FACE_BESTSOUND,
		SCHED_ALYX_FALL_TO_GROUND,
	};

	enum 
	{
		
		TASK_ALYX_BEGIN_INTERACTION = BaseClass::NEXT_TASK,
		TASK_ALYX_COMPLETE_INTERACTION,
		TASK_ALYX_ANNOUNCE_HACK,
		TASK_ALYX_GET_PATH_TO_INTERACT_TARGET,
		TASK_ALYX_WAIT_HACKING,
		TASK_ALYX_HOLSTER_PISTOL,
		TASK_ALYX_DRAW_PISTOL,
		TASK_ALYX_HOLSTER_AND_DESTROY_PISTOL,
		TASK_ALYX_BUILD_COMBAT_FACE_PATH,
		TASK_ALYX_SET_IDLE_ACTIVITY,
		TASK_ALYX_FALL_TO_GROUND,
	};

	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;
};
