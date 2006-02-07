//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Antlion Grub - cannon fodder
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#ifndef	NPC_ANTLIONGRUB_H
#define	NPC_ANTLIONGRUB_H

#include "AI_BaseNPC.h"
#include "soundenvelope.h"
#include "bitstring.h"

class CSprite;

#define	ANTLIONGRUB_MODEL				"models/antlion_grub.mdl"
#define	ANTLIONGRUB_SQUASHED_MODEL		"models/antlion_grub_squashed.mdl"

class CNPC_AntlionGrub : public CAI_BaseNPC
{
public:
	DECLARE_CLASS( CNPC_AntlionGrub, CAI_BaseNPC );
	DECLARE_DATADESC();

	CNPC_AntlionGrub( void );

	virtual int		SelectSchedule( void );
	virtual int		TranslateSchedule( int type );
	int				MeleeAttack1Conditions( float flDot, float flDist );

	bool			HandleInteraction( int interactionType, void *data, CBaseCombatCharacter *sourceEnt );

	void			Precache( void );
	void			Spawn( void );
	void			StartTask( const Task_t *pTask );
	void			RunTask( const Task_t *pTask );
	void			GrubTouch( CBaseEntity *pOther );
	void			EndTouch(  CBaseEntity *pOther );
	void			StickyTouch( CBaseEntity *pOther );
	void			PainSound( void );
	void			PrescheduleThink( void );
	void			HandleAnimEvent( animevent_t *pEvent );
	void			Event_Killed( const CTakeDamageInfo &info );
	void			BuildScheduleTestBits( void );
	void			StopLoopingSounds();

	float			MaxYawSpeed( void ) { 	return 4.0f;	}
	float			StepHeight( ) const { return 12.0f; }	//NOTENOTE: We don't want them to move up too high

	Class_T			Classify( void ) { return CLASS_ANTLION; }

private:

	void			SpawnSquashedGrub( void );
	void			Squash( CBaseEntity *pOther );
	void			BroadcastAlert( void );

	CSoundPatch		*m_pMovementSound;
	CSoundPatch		*m_pVoiceSound;
	CSoundPatch		*m_pHealSound;

	CSprite			*m_pGlowSprite;

	float			m_flNextVoiceChange;	//The next point to alter our voice
	float			m_flSquashTime;			//Amount of time we've been stepped on
	float			m_flNearTime;			//Amount of time the player has been near enough to be considered for healing
	float			m_flHealthTime;			//Amount of time until the next piece of health is given
	float			m_flEnemyHostileTime;	//Amount of time the enemy should be avoided (because they displayed aggressive behavior)

	bool			m_bMoving;
	bool			m_bSquashed;
	bool			m_bSquashValid;
	bool			m_bHealing;
	
	int				m_nHealthReserve;
	int				m_nGlowSpriteHandle;

	DEFINE_CUSTOM_AI;
};

#endif	//NPC_ANTLIONGRUB_H