//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "AI_Default.h"
#include "AI_Task.h"
#include "AI_Schedule.h"
#include "AI_Node.h"
#include "AI_Hull.h"
#include "AI_Hint.h"
#include "AI_Squad.h"
#include "AI_Senses.h"
#include "AI_Navigator.h"
#include "AI_Motor.h"
#include "ai_behavior.h"
#include "ai_baseactor.h"
#include "ai_behavior_lead.h"
#include "ai_behavior_follow.h"
#include "ai_behavior_standoff.h"
#include "ai_behavior_assault.h"
#include "npc_playercompanion.h"
#include "soundent.h"
#include "game.h"
#include "NPCEvent.h"
#include "EntityList.h"
#include "activitylist.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "sceneentity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define FISHERMAN_MODEL "models/ati_2004/fisherman.mdl"

//=========================================================
// Fisherman activities
//=========================================================

class CNPC_Fisherman : public CNPC_PlayerCompanion
{
public:
	DECLARE_CLASS( CNPC_Fisherman, CNPC_PlayerCompanion );
	//DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache()
	{
		// Prevents a warning
		SelectModel( );
		BaseClass::Precache();

		PrecacheScriptSound( "NPC_Fisherman.FootstepLeft" );
		PrecacheScriptSound( "NPC_Fisherman.FootstepRight" );
		PrecacheScriptSound( "NPC_Fisherman.Die" );

		PrecacheInstancedScene( "scenes/Expressions/FishermanIdle.vcd" );
		PrecacheInstancedScene( "scenes/Expressions/FishermanAlert.vcd" );
		PrecacheInstancedScene( "scenes/Expressions/FishermanCombat.vcd" );
	}

	void	Spawn( void );
	void	SelectModel();
	Class_T Classify( void );

	void HandleAnimEvent( animevent_t *pEvent );

	bool ShouldLookForBetterWeapon() { return false; }

	void	DeathSound(void);

	DEFINE_CUSTOM_AI;
};


LINK_ENTITY_TO_CLASS( npc_fisherman, CNPC_Fisherman );

//---------------------------------------------------------
// 
//---------------------------------------------------------
/*
IMPLEMENT_SERVERCLASS_ST(CNPC_Fisherman, DT_NPC_Fisherman)
END_SEND_TABLE()
*/

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_Fisherman )

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Fisherman::SelectModel()
{
	SetModelName( AllocPooledString( FISHERMAN_MODEL ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Fisherman::Spawn( void )
{
	Precache();

	m_iHealth = 80;

//	m_iszIdleExpression = MAKE_STRING("scenes/Expressions/FishermanIdle.vcd");
//	m_iszAlertExpression = MAKE_STRING("scenes/Expressions/FishermanAlert.vcd");
//	m_iszCombatExpression = MAKE_STRING("scenes/Expressions/FishermanCombat.vcd");

	BaseClass::Spawn();

	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION );

	NPCInit();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Fisherman::Classify( void )
{
	return	CLASS_PLAYER_ALLY_VITAL;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Fisherman::HandleAnimEvent( animevent_t *pEvent )
{
	switch( pEvent->event )
	{
	case NPC_EVENT_LEFTFOOT:
		{
			EmitSound( "NPC_Fisherman.FootstepLeft", pEvent->eventtime );
		}
		break;
	case NPC_EVENT_RIGHTFOOT:
		{
			EmitSound( "NPC_Fisherman.FootstepRight", pEvent->eventtime );
		}
		break;

	default:
		BaseClass::HandleAnimEvent( pEvent );
		break;
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CNPC_Fisherman::DeathSound()
{
	// Sentences don't play on dead NPCs
	SentenceStop();
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_fisherman, CNPC_Fisherman )

AI_END_CUSTOM_NPC()
