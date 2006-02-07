//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef AI_SENSES_H
#define AI_SENSES_H

#include "utlvector.h"
#include "simtimer.h"
#include "ai_component.h"
#include "soundent.h"

#if defined( _WIN32 )
#pragma once
#endif

class CBaseEntity;
class CSound;

//-------------------------------------

DECLARE_POINTER_HANDLE( AISightIter_t );
DECLARE_POINTER_HANDLE( AISoundIter_t );

// GetFirstSeenEntity can take these as optional parameters to search for 
// a specific type of entity.
enum seentype_t
{
	SEEN_ALL = -1,	// Default
	SEEN_HIGH_PRIORITY = 0,
	SEEN_NPCS,
	SEEN_MISC
};


const float AI_STANDARD_NPC_SEARCH_TIME = .25;
const float AI_EFFICIENT_NPC_SEARCH_TIME = .55;

//-----------------------------------------------------------------------------
// class CAI_ScriptConditions
//
// Purpose: 
//-----------------------------------------------------------------------------

class CAI_Senses : public CAI_Component
{
public:
	CAI_Senses()
	 : 	m_LookDist(2048),
		m_LastLookDist(-1),
		m_TimeLastLook(-1),
		m_iAudibleList(0),
		m_HighPriorityTimer(0.15),	// every other thinks (5Hz)
		m_NPCsTimer(AI_STANDARD_NPC_SEARCH_TIME),	// every third think (~3Hz)
		m_MiscTimer(0.45)			// every fifth think (2Hz)
	{
		m_SeenArrays[0] = &m_SeenHighPriority;
		m_SeenArrays[1] = &m_SeenNPCs;
		m_SeenArrays[2] = &m_SeenMisc;
	}
	
	float			GetDistLook() const				{ return m_LookDist; }
	void			SetDistLook( float flDistLook ) { m_LookDist = flDistLook; }

	void			PerformSensing();

	void			Listen( void );
	void			Look( int iDistance );// basic sight function for npcs

	bool			ShouldSeeEntity( CBaseEntity *pEntity ); // logical query
	bool			CanSeeEntity( CBaseEntity *pSightEnt ); // more expensive cone & raycast test
	
	bool			DidSeeEntity( CBaseEntity *pSightEnt ) const; //  a less expensive query that looks at cached results from recent conditionsa gathering

	CBaseEntity *	GetFirstSeenEntity( AISightIter_t *pIter, seentype_t iSeenType = SEEN_ALL ) const;
	CBaseEntity *	GetNextSeenEntity( AISightIter_t *pIter ) const;

	CSound *		GetFirstHeardSound( AISoundIter_t *pIter );
	CSound *		GetNextHeardSound( AISoundIter_t *pIter );
	CSound *		GetClosestSound( bool fScent = false, int validTypes = ALL_SOUNDS | ALL_SCENTS );

	bool 			CanHearSound( CSound *pSound );

	//---------------------------------

	DECLARE_SIMPLE_DATADESC();

private:
	int				GetAudibleList() const { return m_iAudibleList; }

	bool			WaitingUntilSeen( CBaseEntity *pSightEnt );

	void			BeginGather();
	void 			NoteSeenEntity( CBaseEntity *pSightEnt );
	void			EndGather( int nSeen, CUtlVector<EHANDLE> *pResult );
	
	bool 			Look( CBaseEntity *pSightEnt );
	int 			LookForHighPriorityEntities( int iDistance );
	int 			LookForNPCs( int iDistance );
	int 			LookForObjects( int iDistance );
	
	bool			SeeEntity( CBaseEntity *pEntity );
	
	float			m_LookDist;				// distance npc sees (Default 2048)
	float			m_LastLookDist;
	float			m_TimeLastLook;
	
	int				m_iAudibleList;				// first index of a linked list of sounds that the npc can hear.
	
	CUtlVector<EHANDLE> m_SeenHighPriority;
	CUtlVector<EHANDLE> m_SeenNPCs;
	CUtlVector<EHANDLE> m_SeenMisc;
	
	CUtlVector<EHANDLE> *m_SeenArrays[3];
	
	CSimTimer		m_HighPriorityTimer;
	CSimTimer		m_NPCsTimer;
	CSimTimer		m_MiscTimer;
};

//-----------------------------------------------------------------------------

class CAI_SensedObjectsManager : public IEntityListener
{
public:
	void Init();
	void Term();

	CBaseEntity *	GetFirst( int *pIter );
	CBaseEntity *	GetNext( int *pIter );

	virtual void 	AddEntity( CBaseEntity *pEntity );

private:
	virtual void 	OnEntitySpawned( CBaseEntity *pEntity );
	virtual void 	OnEntityDeleted( CBaseEntity *pEntity );

	CUtlVector<EHANDLE> m_SensedObjects;
};

extern CAI_SensedObjectsManager g_AI_SensedObjectsManager;

//-----------------------------------------------------------------------------



#endif // AI_SENSES_H
