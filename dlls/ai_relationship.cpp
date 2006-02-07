//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ndebugoverlay.h"
#include "ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
class CAI_Relationship : public CBaseEntity, public IEntityListener
{
	DECLARE_CLASS( CAI_Relationship, CBaseEntity );

public:
	CAI_Relationship() { m_iPreviousDisposition = -1; }

	void Spawn();
	void Activate();

	void SetActive( bool bActive );
	void ChangeRelationships( int disposition, bool fReverting );

	void ApplyRelationship();
	void RevertRelationship();

	void UpdateOnRemove();
	void OnRestore();

	bool IsASubject( CBaseEntity *pEntity );
	bool IsATarget( CBaseEntity *pEntity );

	void OnEntitySpawned( CBaseEntity *pEntity );
	void OnEntityDeleted( CBaseEntity *pEntity );

private:
	string_t	m_iszSubject;
	int			m_iDisposition;
	int			m_iRank;
	bool		m_fStartActive;
	bool		m_bIsActive;
	int			m_iPreviousDisposition;
	float		m_flRadius;
	int			m_iPreviousRank;
	bool		m_bReciprocal;

public:
	// Input functions
	void InputApplyRelationship( inputdata_t &inputdata );
	void InputRevertRelationship( inputdata_t &inputdata );

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( ai_relationship, CAI_Relationship );

BEGIN_DATADESC( CAI_Relationship )
	DEFINE_THINKFUNC( ApplyRelationship ),
	
	DEFINE_KEYFIELD( m_iszSubject, FIELD_STRING, "subject" ),
	DEFINE_KEYFIELD( m_iDisposition, FIELD_INTEGER, "disposition" ),
	DEFINE_KEYFIELD( m_iRank, FIELD_INTEGER, "rank" ),
	DEFINE_KEYFIELD( m_fStartActive, FIELD_BOOLEAN, "StartActive" ),
	DEFINE_FIELD( m_bIsActive, FIELD_BOOLEAN ),
	DEFINE_KEYFIELD( m_flRadius, FIELD_FLOAT, "radius" ),
	DEFINE_FIELD( m_iPreviousDisposition, FIELD_INTEGER ),
	DEFINE_FIELD( m_iPreviousRank, FIELD_INTEGER ),
	DEFINE_KEYFIELD( m_bReciprocal, FIELD_BOOLEAN, "reciprocal" ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "ApplyRelationship", InputApplyRelationship ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RevertRelationship", InputRevertRelationship ),
END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_Relationship::Spawn()
{
	m_bIsActive = false;

	if (m_iszSubject == NULL_STRING)
	{
		DevWarning("ai_relationship '%s' with no subject specified, removing.\n", GetDebugName());
		UTIL_Remove(this);
	}
	else if (m_target == NULL_STRING)
	{
		DevWarning("ai_relationship '%s' with no target specified, removing.\n", GetDebugName());
		UTIL_Remove(this);
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_Relationship::Activate()
{
	if ( m_fStartActive )
	{
		ApplyRelationship();

		// Clear this flag so that nothing happens when the level is loaded (which calls activate again)
		m_fStartActive = false;
	}

	BaseClass::Activate();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bActive - 
//-----------------------------------------------------------------------------
void CAI_Relationship::SetActive( bool bActive )
{
	if ( bActive && !m_bIsActive )
	{
		// Start getting entity updates!
		gEntList.AddListenerEntity( this );
	}
	else if ( !bActive && m_bIsActive )
	{
		// Stop getting entity updates!
		gEntList.RemoveListenerEntity( this );
	}

	m_bIsActive = bActive;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_Relationship::InputApplyRelationship( inputdata_t &inputdata )
{
	ApplyRelationship();
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_Relationship::InputRevertRelationship( inputdata_t &inputdata )
{
	RevertRelationship();
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_Relationship::ApplyRelationship()
{
	// @TODO (toml 10-22-04): sort out MP relationships 
	if ( AI_IsSinglePlayer() && !UTIL_GetLocalPlayer() )
	{
		SetThink( &CAI_Relationship::ApplyRelationship );
		SetNextThink( gpGlobals->curtime );
	}

	if ( !m_bIsActive )
	{
		SetActive( true );
	}

	ChangeRelationships( m_iDisposition, false );
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_Relationship::RevertRelationship()
{
	if ( m_bIsActive )
	{
		ChangeRelationships( m_iPreviousDisposition, true );
		SetActive( false );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_Relationship::UpdateOnRemove()
{
	gEntList.RemoveListenerEntity( this );
	// @TODO (toml 07-21-04): Should this actually revert on kill?
	// RevertRelationship();
	BaseClass::UpdateOnRemove();
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_Relationship::OnRestore()
{
	BaseClass::OnRestore();
	if ( m_bIsActive )
	{
		gEntList.AddListenerEntity( this );
	}
}


//---------------------------------------------------------
//---------------------------------------------------------
bool CAI_Relationship::IsASubject( CBaseEntity *pEntity )
{
	if( pEntity->NameMatches( m_iszSubject ) )
		return true;

	if( pEntity->ClassMatches( m_iszSubject ) )
		return true;

	return false;
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAI_Relationship::IsATarget( CBaseEntity *pEntity )
{
	if( pEntity->NameMatches( m_target ) )
		return true;

	if( pEntity->ClassMatches( m_target ) )
		return true;

	return false;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_Relationship::OnEntitySpawned( CBaseEntity *pEntity )
{
	if( IsATarget( pEntity ) || IsASubject( pEntity ) )
	{
		ApplyRelationship();
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_Relationship::OnEntityDeleted( CBaseEntity *pEntity )
{
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_Relationship::ChangeRelationships( int disposition, bool fReverting )
{
 	if( fReverting && m_iPreviousDisposition == -1 )
	{
		// Trying to revert without having ever set the relationships!
		DevMsg( 2, "ai_relationship cannot revert changes before they are applied!\n");
		return;
	}

	float radiusSq = Square( m_flRadius );

	const int MAX_HANDLED = 256;

	CUtlVectorFixed<CBaseCombatCharacter *, MAX_HANDLED> subjectList;
	CUtlVectorFixed<CBaseCombatCharacter *, MAX_HANDLED> targetList;

	// Search for targets and subjects.
	int i;

	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		if ( subjectList.Count() == MAX_HANDLED || targetList.Count() == MAX_HANDLED )
		{
			DevMsg( "Too many entities handled by ai_relationship %s\n", GetDebugName() );
			break;
		}

		CBasePlayer	*pPlayer = UTIL_PlayerByIndex( i );
		if ( pPlayer )
		{
			if( IsASubject( pPlayer ) )
			{
				if ( m_flRadius == 0.0 || GetAbsOrigin().DistToSqr( pPlayer->GetAbsOrigin() ) <= radiusSq )
					subjectList.AddToTail( pPlayer );
			}
			else if( IsATarget( pPlayer ) )
			{
				targetList.AddToTail( pPlayer );
			}
		}
	}

	for ( i = 0; i < g_AI_Manager.NumAIs(); i++ )
	{
		if ( subjectList.Count() == MAX_HANDLED || targetList.Count() == MAX_HANDLED )
		{
			DevMsg( "Too many entities handled by ai_relationship %s\n", GetDebugName() );
			break;
		}

		CAI_BaseNPC *pNPC = (g_AI_Manager.AccessAIs())[i];
		if ( pNPC )
		{
			if( IsASubject( pNPC ) )
			{
				if ( m_flRadius == 0.0 || GetAbsOrigin().DistToSqr( pNPC->GetAbsOrigin() ) <= radiusSq )
					subjectList.AddToTail( pNPC );
			}
			else if( IsATarget( pNPC ) )
			{
				targetList.AddToTail( pNPC );
			}
		}
	}

	// If either list is still empty, we have a problem.
	if( subjectList.Count() == 0 )
	{
		DevMsg( 2, "ai_relationship '%s' finds no subject(s) called: %s\n", GetDebugName(), STRING( m_iszSubject ) );
		return;
	}
	else if ( targetList.Count() == 0 )
	{
		DevMsg( 2, "ai_relationship '%s' finds no target(s) called: %s\n", GetDebugName(), STRING( m_target ) );
		return;
	}

	// Ok, lists are populated. Apply all relationships.
	int j;
	for ( i = 0 ; i < subjectList.Count(); i++ )
	{
		CBaseCombatCharacter *pSubject = subjectList[ i ];

		for ( j = 0 ; j < targetList.Count(); j++ )
		{
			CBaseCombatCharacter *pTarget = targetList[ j ];

			if( m_iPreviousDisposition == -1 && !fReverting )
			{
				// Set previous disposition.
				m_iPreviousDisposition = pSubject->IRelationType( pTarget );
				m_iPreviousRank = pSubject->IRelationPriority( pTarget );
			}

			if( fReverting )
			{
				pSubject->AddEntityRelationship( pTarget, (Disposition_t)m_iPreviousDisposition, m_iPreviousRank );

				if( m_bReciprocal )
				{
					pTarget->AddEntityRelationship( pSubject, (Disposition_t)m_iPreviousDisposition, m_iPreviousRank );
				}
			}
			else if( pSubject->IRelationType(pTarget) != disposition || pSubject->IRelationPriority(pTarget) != m_iRank )
			{
				pSubject->AddEntityRelationship( pTarget, (Disposition_t)disposition, m_iRank );

				if( m_bReciprocal )
				{
					pTarget->AddEntityRelationship( pSubject, (Disposition_t)disposition, m_iRank );
				}
			}
		}
	}
}

