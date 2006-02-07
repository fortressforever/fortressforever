//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef NPC_STALKER_H
#define NPC_STALKER_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_basenpc.h"
#include "entityoutput.h"


class CBeam;
class CSprite;
class CScriptedTarget;


class CNPC_Stalker : public CAI_BaseNPC
{
	DECLARE_CLASS( CNPC_Stalker, CAI_BaseNPC );

public:
	float			m_flNextAttackSoundTime;
	float			m_flNextBreatheSoundTime;
	float			m_flNextScrambleSoundTime;
	float			m_flNextNPCThink;

	// ------------------------------
	//	Laser Beam
	// ------------------------------
	int					m_eBeamPower;
	Vector				m_vLaserDir;
	Vector				m_vLaserTargetPos;
	float				m_fBeamEndTime;
	float				m_fBeamRechargeTime;
	float				m_fNextDamageTime;
	float				m_nextSmokeTime;
	float				m_bPlayingHitWall;
	float				m_bPlayingHitFlesh;
	CBeam*				m_pBeam;
	CSprite*			m_pLightGlow;
	void				KillAttackBeam(void);
	void				DrawAttackBeam(void);
	void				CalcBeamPosition(void);
	Vector				LaserStartPosition(Vector vStalkerPos);


	// ------------------------------
	//	Scripted Target Burns
	// ------------------------------
	CScriptedTarget*	m_pScriptedTarget;		// My current scripted target
	void				SetScriptedTarget( CScriptedTarget *pScriptedTarget );

	Vector				m_vLaserCurPos;			// Last position successfully burned
	bool				InnateWeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions );
	Vector				ScriptedBurnPosition(void);
	
	// ------------------------------
	//	Dormancy
	// ------------------------------
	CAI_Schedule*	WakeUp(void);
	void			GoDormant(void);

public:
	void			Spawn( void );
	void			Precache( void );
	float			MaxYawSpeed( void );
	Class_T			Classify ( void );
	
	void			StartTask( const Task_t *pTask );
	void			RunTask( const Task_t *pTask );
	virtual int		SelectSchedule ( void );
	virtual int		TranslateSchedule( int scheduleType );
	int				OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void			OnScheduleChange();

	void			StalkerThink(void);

	int				MeleeAttack1Conditions ( float flDot, float flDist );
	int				RangeAttack1Conditions ( float flDot, float flDist );
	void			HandleAnimEvent( animevent_t *pEvent );

	bool			FValidateHintType(CAI_Hint *pHint);
	Activity		GetHintActivity( short sHintType, Activity HintsActivity );
	float			GetHintDelay( short sHintType );

	void			IdleSound( void );
	void			DeathSound( void );
	void			PainSound( void );

	bool			HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);
	void			Event_Killed( const CTakeDamageInfo &info );
	void			DoSmokeEffect( const Vector &position );

	void			AddZigZagToPath(void);
	void			StartAttackBeam();
	void			UpdateAttackBeam();

	CNPC_Stalker(void);

	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;
};

#endif // NPC_STALKER_H
