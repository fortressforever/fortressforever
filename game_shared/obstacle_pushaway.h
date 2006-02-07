//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef OBSTACLE_PUSHAWAY_H
#define OBSTACLE_PUSHAWAY_H
#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------------------------------
class CPushAwayEnumerator : public IPartitionEnumerator
{
public:
	// Forced constructor
	CPushAwayEnumerator(CBaseEntity **ents, int nMaxEnts)
	{
		m_nAlreadyHit = 0;
		m_AlreadyHit = ents;
		m_nMaxHits = nMaxEnts;
	}
	
	// Actual work code
	virtual IterationRetval_t EnumElement( IHandleEntity *pHandleEntity )
	{
#ifdef CLIENT_DLL
		CBaseEntity *pEnt = ClientEntityList().GetBaseEntityFromHandle( pHandleEntity->GetRefEHandle() );
#else
		CBaseEntity *pEnt = gEntList.GetBaseEntity( pHandleEntity->GetRefEHandle() );
#endif

		if ( pEnt == NULL )
			return ITERATION_CONTINUE;

		if ( pEnt->GetCollisionGroup() != COLLISION_GROUP_PUSHAWAY )
			return ITERATION_CONTINUE;

		if ( m_nAlreadyHit < m_nMaxHits )
		{
			m_AlreadyHit[m_nAlreadyHit] = pEnt;
			m_nAlreadyHit++;
		}

		return ITERATION_CONTINUE;
	}

public:

	CBaseEntity **m_AlreadyHit;
	int m_nAlreadyHit;
	int m_nMaxHits;
};

//-----------------------------------------------------------------------------------------------------
// Retrieves physics objects near pPushingEntity
void AvoidPushawayProps(  CBaseCombatCharacter *pPlayer, CUserCmd *pCmd );
int GetPushawayEnts( CBaseCombatCharacter *pPushingEntity, CBaseEntity **ents, int nMaxEnts, float flPlayerExpand, int PartitionMask, CPushAwayEnumerator *enumerator = NULL );

//-----------------------------------------------------------------------------------------------------
// Pushes physics objects away from the entity
void PerformObstaclePushaway( CBaseCombatCharacter *pPushingEntity );


#endif // OBSTACLE_PUSHAWAY_H
