//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "studio.h"
#include "activitylist.h"
#include "engine/IEngineSound.h"
#include "ai_activity.h"
#include "animation.h"
#include "bone_setup.h"
#include "scriptevent.h"
#include "npcevent.h"
#include "eventlist.h"
#include "tier0/vprof.h"

#if !defined( CLIENT_DLL ) && !defined( MAKEXVCD )
#include "util.h"
#include "enginecallback.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#pragma warning( disable : 4244 )
#define iabs(i) (( (i) >= 0 ) ? (i) : -(i) )

int ExtractBbox( CStudioHdr *pstudiohdr, int sequence, Vector& mins, Vector& maxs )
{
	if (! pstudiohdr)
		return 0;

	if (!pstudiohdr->SequencesAvailable())
		return 0;

	mstudioseqdesc_t	&seqdesc = pstudiohdr->pSeqdesc( sequence );
	
	mins = seqdesc.bbmin;

	maxs = seqdesc.bbmax;

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//
// Input  : *pstudiohdr - 
//			iSequence - 
//
// Output : mstudioseqdesc_t
//-----------------------------------------------------------------------------

extern int g_nActivityListVersion;
extern int g_nEventListVersion;

void SetEventIndexForSequence( mstudioseqdesc_t &seqdesc )
{
	if ( &seqdesc == NULL )
		 return;

	seqdesc.flags |= STUDIO_EVENT;

	if ( seqdesc.numevents == 0 )
		 return;

	for ( int index = 0; index < (int)seqdesc.numevents; index++ )
	{
		mstudioevent_t *pevent = seqdesc.pEvent( index );

		if ( !pevent )
			 continue;

		if ( pevent->type & AE_TYPE_NEWEVENTSYSTEM )
		{
			const char *pEventName = pevent->pszEventName();
			
			int iEventIndex = EventList_IndexForName( pEventName );
				
			if ( iEventIndex == -1 )
			{
				pevent->event = EventList_RegisterPrivateEvent( pEventName );
			}
			else
			{
				pevent->event = iEventIndex;
				pevent->type |= EventList_GetEventType( iEventIndex );
			}
		}
	}
}

mstudioevent_t *GetEventIndexForSequence( mstudioseqdesc_t &seqdesc )
{
	if (!(seqdesc.flags & STUDIO_EVENT))
	{
		SetEventIndexForSequence( seqdesc );
	}

	return seqdesc.pEvent( 0 );
}


void BuildAllAnimationEventIndexes( CStudioHdr *pstudiohdr )
{
	if ( !pstudiohdr )
		return;

	if( pstudiohdr->GetEventListVersion() != g_nEventListVersion )
	{
		for ( int i = 0 ; i < pstudiohdr->GetNumSeq() ; i++ )
		{
			SetEventIndexForSequence( pstudiohdr->pSeqdesc( i ) );
		}

		pstudiohdr->SetEventListVersion( g_nEventListVersion );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Ensures that activity / index relationship is recalculated
// Input  :
// Output :
//-----------------------------------------------------------------------------
void ResetEventIndexes( CStudioHdr *pstudiohdr )
{	
	if (! pstudiohdr)
		return;

	pstudiohdr->SetEventListVersion( g_nEventListVersion - 1 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

void SetActivityForSequence( CStudioHdr *pstudiohdr, int i )
{
	int iActivityIndex;
	const char *pszActivityName;
	mstudioseqdesc_t &seqdesc = pstudiohdr->pSeqdesc( i );

	seqdesc.flags |= STUDIO_ACTIVITY;

	pszActivityName = GetSequenceActivityName( pstudiohdr, i );
	if ( pszActivityName[0] != '\0' )
	{
		iActivityIndex = ActivityList_IndexForName( pszActivityName );
		
		if ( iActivityIndex == -1 )
		{
			// Allow this now.  Animators can create custom activities that are referenced only on the client or by scripts, etc.
			//Warning( "***\nModel %s tried to reference unregistered activity: %s \n***\n", pstudiohdr->name, pszActivityName );
			//Assert(0);
			// HACK: the client and server don't share the private activity list so registering it on the client would hose the server
#ifdef CLIENT_DLL
			seqdesc.flags &= ~STUDIO_ACTIVITY;
#else
			seqdesc.activity = ActivityList_RegisterPrivateActivity( pszActivityName );
#endif
		}
		else
		{
			seqdesc.activity = iActivityIndex;
		}
	}
}

//=========================================================
// IndexModelSequences - set activity and event indexes for all model
// sequences that have them.
//=========================================================

void IndexModelSequences( CStudioHdr *pstudiohdr )
{
	int i;

	if (! pstudiohdr)
		return;

	if (!pstudiohdr->SequencesAvailable())
		return;

	for ( i = 0 ; i < pstudiohdr->GetNumSeq() ; i++ )
	{
		SetActivityForSequence( pstudiohdr, i );
		SetEventIndexForSequence( pstudiohdr->pSeqdesc( i ) );
	}

	pstudiohdr->SetActivityListVersion( g_nActivityListVersion );
}

//-----------------------------------------------------------------------------
// Purpose: Ensures that activity / index relationship is recalculated
// Input  :
// Output :
//-----------------------------------------------------------------------------
void ResetActivityIndexes( CStudioHdr *pstudiohdr )
{	
	if (! pstudiohdr)
		return;

	pstudiohdr->SetActivityListVersion( g_nActivityListVersion - 1 );
}

void VerifySequenceIndex( CStudioHdr *pstudiohdr )
{
	if ( !pstudiohdr )
	{
		return;
	}

	if( pstudiohdr->GetActivityListVersion( ) != g_nActivityListVersion )
	{
		// this model's sequences have not yet been indexed by activity
		IndexModelSequences( pstudiohdr );
	}
}

#if !defined( MAKEXVCD )
bool IsInPrediction()
{
	return CBaseEntity::GetPredictionPlayer() != NULL;
}

int SelectWeightedSequence( CStudioHdr *pstudiohdr, int activity, int curSequence )
{
	VPROF( "SelectWeightedSequence" );

	if (! pstudiohdr)
		return 0;

	if (!pstudiohdr->SequencesAvailable())
		return 0;

	VerifySequenceIndex( pstudiohdr );

	int weighttotal = 0;
	int seq = ACTIVITY_NOT_AVAILABLE;
	int weight = 0;
	for (int i = 0; i < pstudiohdr->GetNumSeq(); i++)
	{
		int curActivity = GetSequenceActivity( pstudiohdr, i, &weight );
		if (curActivity == activity)
		{
			if ( curSequence == i && weight < 0 )
			{
				seq = i;
				break;
			}
			weighttotal += iabs(weight);
			
			int randomValue;

			if ( IsInPrediction() )
				randomValue = SharedRandomInt( "SelectWeightedSequence", 0, weighttotal - 1, i );
			else
				randomValue = RandomInt( 0, weighttotal - 1 );
			
			if (!weighttotal || randomValue < iabs(weight))
				seq = i;
		}
	}

	return seq;
}
#endif

int SelectHeaviestSequence( CStudioHdr *pstudiohdr, int activity )
{
	if ( !pstudiohdr )
		return 0;

	VerifySequenceIndex( pstudiohdr );

	int maxweight = 0;
	int seq = ACTIVITY_NOT_AVAILABLE;
	int weight = 0;
	for (int i = 0; i < pstudiohdr->GetNumSeq(); i++)
	{
		int curActivity = GetSequenceActivity( pstudiohdr, i, &weight );
		if (curActivity == activity)
		{
			if ( iabs(weight) > maxweight )
			{
				maxweight = iabs(weight);
				seq = i;
			}
		}
	}

	return seq;
}

void GetEyePosition ( CStudioHdr *pstudiohdr, Vector &vecEyePosition )
{
	if ( !pstudiohdr )
	{
		Warning( "GetEyePosition() Can't get pstudiohdr ptr!\n" );
		return;
	}

	vecEyePosition = pstudiohdr->eyeposition();
}


//-----------------------------------------------------------------------------
// Purpose: Looks up an activity by name.
// Input  : label - Name of the activity to look up, ie "ACT_IDLE"
// Output : Activity index or ACT_INVALID if not found.
//-----------------------------------------------------------------------------
int LookupActivity( CStudioHdr *pstudiohdr, const char *label )
{
	VPROF( "LookupActivity" );

	if ( !pstudiohdr )
	{
		return 0;
	}

	for ( int i = 0; i < pstudiohdr->GetNumSeq(); i++ )
	{
		mstudioseqdesc_t &seqdesc = pstudiohdr->pSeqdesc( i );
		if ( stricmp( seqdesc.pszActivityName(), label ) == 0 )
		{
			return seqdesc.activity;
		}
	}

	return ACT_INVALID;
}

#if !defined( MAKEXVCD )
//-----------------------------------------------------------------------------
// Purpose: Looks up a sequence by sequence name first, then by activity name.
// Input  : label - The sequence name or activity name to look up.
// Output : Returns the sequence index of the matching sequence, or ACT_INVALID.
//-----------------------------------------------------------------------------
int LookupSequence( CStudioHdr *pstudiohdr, const char *label )
{
	VPROF( "LookupSequence" );

	if (! pstudiohdr)
		return 0;

	if (!pstudiohdr->SequencesAvailable())
		return 0;

	//
	// Look up by sequence name.
	//
	for (int i = 0; i < pstudiohdr->GetNumSeq(); i++)
	{
		mstudioseqdesc_t	&seqdesc = pstudiohdr->pSeqdesc( i );
		if (stricmp( seqdesc.pszLabel(), label ) == 0)
			return i;
	}

	//
	// Not found, look up by activity name.
	//
	int nActivity = LookupActivity( pstudiohdr, label );
	if (nActivity != ACT_INVALID )
	{
		return SelectWeightedSequence( pstudiohdr, nActivity );
	}

	return ACT_INVALID;
}

void GetSequenceLinearMotion( CStudioHdr *pstudiohdr, int iSequence, const float poseParameter[], Vector *pVec )
{
	if (! pstudiohdr)
	{
		Msg( "Bad pstudiohdr in GetSequenceLinearMotion()!\n" );
		return;
	}

	if (!pstudiohdr->SequencesAvailable())
		return;

	if( iSequence < 0 || iSequence >= pstudiohdr->GetNumSeq() )
	{
		// Don't spam on bogus model
		if ( pstudiohdr->GetNumSeq() > 0 )
		{
			static int msgCount = 0;
			while ( ++msgCount < 10 )
			{
				Msg( "Bad sequence (%i out of %i max) in GetSequenceLinearMotion() for model '%s'!\n", iSequence, pstudiohdr->GetNumSeq(), pstudiohdr->pszName() );
			}
		}
		pVec->Init();
		return;
	}

	QAngle vecAngles;
	Studio_SeqMovement( pstudiohdr, iSequence, 0, 1.0, poseParameter, (*pVec), vecAngles );
}
#endif

const char *GetSequenceName( CStudioHdr *pstudiohdr, int iSequence )
{
	if( !pstudiohdr || iSequence < 0 || iSequence >= pstudiohdr->GetNumSeq() )
	{
		if ( pstudiohdr )
		{
			Msg( "Bad sequence in GetSequenceName() for model '%s'!\n", pstudiohdr->pszName() );
		}
		return "Unknown";
	}

	mstudioseqdesc_t	&seqdesc = pstudiohdr->pSeqdesc( iSequence );
	return seqdesc.pszLabel();
}

const char *GetSequenceActivityName( CStudioHdr *pstudiohdr, int iSequence )
{
	if( !pstudiohdr || iSequence < 0 || iSequence >= pstudiohdr->GetNumSeq() )
	{
		if ( pstudiohdr )
		{
			Msg( "Bad sequence in GetSequenceActivityName() for model '%s'!\n", pstudiohdr->pszName() );
		}
		return "Unknown";
	}

	mstudioseqdesc_t	&seqdesc = pstudiohdr->pSeqdesc( iSequence );
	return seqdesc.pszActivityName( );
}

int GetSequenceFlags( CStudioHdr *pstudiohdr, int sequence )
{
	if ( !pstudiohdr || 
		 !pstudiohdr->SequencesAvailable() ||
		sequence < 0 || 
		sequence >= pstudiohdr->GetNumSeq() )
	{
		return 0;
	}

	mstudioseqdesc_t &seqdesc = pstudiohdr->pSeqdesc( sequence );

	return seqdesc.flags;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pstudiohdr - 
//			sequence - 
//			type - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool HasAnimationEventOfType( CStudioHdr *pstudiohdr, int sequence, int type )
{
	if ( !pstudiohdr || sequence >= pstudiohdr->GetNumSeq() )
		return false;

	mstudioseqdesc_t &seqdesc = pstudiohdr->pSeqdesc( sequence );
	if ( !&seqdesc )
		return false;

	mstudioevent_t *pevent = GetEventIndexForSequence( seqdesc );
	if ( !pevent )
		return false;

	if (seqdesc.numevents == 0 )
		return false;

	int index;
	for ( index = 0; index < (int)seqdesc.numevents; index++ )
	{
		if ( pevent[ index ].event == type )
		{
			return true;
		}
	}

	return false;
}

int GetAnimationEvent( CStudioHdr *pstudiohdr, int sequence, animevent_t *pNPCEvent, float flStart, float flEnd, int index )
{
	if ( !pstudiohdr || sequence >= pstudiohdr->GetNumSeq() || !pNPCEvent )
		return 0;

	mstudioseqdesc_t &seqdesc = pstudiohdr->pSeqdesc( sequence );
	if (seqdesc.numevents == 0 || index >= (int)seqdesc.numevents )
		return 0;

	// Msg( "flStart %f flEnd %f (%d) %s\n", flStart, flEnd, seqdesc.numevents, seqdesc.label );
	mstudioevent_t *pevent = GetEventIndexForSequence( seqdesc );
	for (; index < (int)seqdesc.numevents; index++)
	{
		// Don't send client-side events to the server AI
		if ( pevent[index].type & AE_TYPE_NEWEVENTSYSTEM )
		{
			if ( pevent[index].type & AE_TYPE_CLIENT )
				 continue;
		}
		else if ( pevent[index].event >= EVENT_CLIENT ) //Adrian - Support the old event system
			continue;
	
		bool bOverlapEvent = false;

		if (pevent[index].cycle >= flStart && pevent[index].cycle < flEnd)
		{
			bOverlapEvent = true;
		}
		// FIXME: doesn't work with animations being played in reverse
		else if ((seqdesc.flags & STUDIO_LOOPING) && flEnd < flStart)
		{
			if (pevent[index].cycle >= flStart || pevent[index].cycle < flEnd)
			{
				bOverlapEvent = true;
			}
		}

		if (bOverlapEvent)
		{
			pNPCEvent->pSource = NULL;
			pNPCEvent->cycle = pevent[index].cycle;
#if !defined( MAKEXVCD )
			pNPCEvent->eventtime = gpGlobals->curtime;
#else
			pNPCEvent->eventtime = 0.0f;
#endif
			pNPCEvent->event = pevent[index].event;
			pNPCEvent->options = pevent[index].pszOptions();
			pNPCEvent->type	= pevent[index].type;
			return index + 1;
		}
	}
	return 0;
}



int FindTransitionSequence( CStudioHdr *pstudiohdr, int iCurrentSequence, int iGoalSequence, int *piDir )
{
	if ( !pstudiohdr )
		return iGoalSequence;

	if ( !pstudiohdr->SequencesAvailable() )
		return iGoalSequence;

	if ( ( iCurrentSequence < 0 ) || ( iCurrentSequence >= pstudiohdr->GetNumSeq() ) )
		return iGoalSequence;

	if ( ( iGoalSequence < 0 ) || ( iGoalSequence >= pstudiohdr->GetNumSeq() ) )
	{
		// asking for a bogus sequence.  Punt.
		Assert( 0 );
		return iGoalSequence;
	}


	// bail if we're going to or from a node 0
	if (pstudiohdr->EntryNode( iCurrentSequence ) == 0 || pstudiohdr->EntryNode( iGoalSequence ) == 0)
	{
		*piDir = 1;
		return iGoalSequence;
	}

	int	iEndNode;

	// Msg( "from %d to %d: ", pEndNode->iEndNode, pGoalNode->iStartNode );

	// check to see if we should be going forward or backward through the graph
	if (*piDir > 0)
	{
		iEndNode = pstudiohdr->ExitNode( iCurrentSequence );
	}
	else
	{
		iEndNode = pstudiohdr->EntryNode( iCurrentSequence );
	}

	// if both sequences are on the same node, just go there
	if (iEndNode == pstudiohdr->EntryNode( iGoalSequence ))
	{
		*piDir = 1;
		return iGoalSequence;
	}

	int iInternNode = pstudiohdr->GetTransition( iEndNode, pstudiohdr->EntryNode( iGoalSequence ) );

	// if there is no transitionial node, just go to the goal sequence
	if (iInternNode == 0)
		return iGoalSequence;

	int i;

	// look for someone going from the entry node to next node it should hit
	// this may be the goal sequences node or an intermediate node
	for (i = 0; i < pstudiohdr->GetNumSeq(); i++)
	{
		mstudioseqdesc_t &seqdesc = pstudiohdr->pSeqdesc(i );
		if (pstudiohdr->EntryNode( i ) == iEndNode && pstudiohdr->ExitNode( i ) == iInternNode)
		{
			*piDir = 1;
			return i;
		}
		if (seqdesc.nodeflags)
		{
			if (pstudiohdr->ExitNode( i ) == iEndNode && pstudiohdr->EntryNode( i ) == iInternNode)
			{
				*piDir = -1;
				return i;
			}
		}
	}

	// this means that two parts of the node graph are not connected.
	DevMsg( 2, "error in transition graph: %s to %s\n",  pstudiohdr->pszNodeName( iEndNode ), pstudiohdr->pszNodeName( pstudiohdr->EntryNode( iGoalSequence ) ));
	// Go ahead and jump to the goal sequence
	return iGoalSequence;
}

void SetBodygroup( CStudioHdr *pstudiohdr, int& body, int iGroup, int iValue )
{
	if (! pstudiohdr)
		return;

	if (iGroup >= pstudiohdr->numbodyparts())
		return;

	mstudiobodyparts_t *pbodypart = pstudiohdr->pBodypart( iGroup );

	if (iValue >= pbodypart->nummodels)
		return;

	int iCurrent = (body / pbodypart->base) % pbodypart->nummodels;

	body = (body - (iCurrent * pbodypart->base) + (iValue * pbodypart->base));
}


int GetBodygroup( CStudioHdr *pstudiohdr, int body, int iGroup )
{
	if (! pstudiohdr)
		return 0;

	if (iGroup >= pstudiohdr->numbodyparts())
		return 0;

	mstudiobodyparts_t *pbodypart = pstudiohdr->pBodypart( iGroup );

	if (pbodypart->nummodels <= 1)
		return 0;

	int iCurrent = (body / pbodypart->base) % pbodypart->nummodels;

	return iCurrent;
}

const char *GetBodygroupName( CStudioHdr *pstudiohdr, int iGroup )
{
	if ( !pstudiohdr)
		return "";

	if (iGroup >= pstudiohdr->numbodyparts())
		return "";

	mstudiobodyparts_t *pbodypart = pstudiohdr->pBodypart( iGroup );
	return pbodypart->pszName();
}

int FindBodygroupByName( CStudioHdr *pstudiohdr, const char *name )
{
	if ( !pstudiohdr )
		return -1;

	int group;
	for ( group = 0; group < pstudiohdr->numbodyparts(); group++ )
	{
		mstudiobodyparts_t *pbodypart = pstudiohdr->pBodypart( group );
		if ( !Q_strcasecmp( name, pbodypart->pszName() ) )
		{
			return group;
		}
	}

	return -1;
}

int GetBodygroupCount( CStudioHdr *pstudiohdr, int iGroup )
{
	if ( !pstudiohdr )
		return 0;

	if (iGroup >= pstudiohdr->numbodyparts())
		return 0;

	mstudiobodyparts_t *pbodypart = pstudiohdr->pBodypart( iGroup );
	return pbodypart->nummodels;
}

int GetNumBodyGroups( CStudioHdr *pstudiohdr )
{
	if ( !pstudiohdr )
		return 0;

	return pstudiohdr->numbodyparts();
}

int GetSequenceActivity( CStudioHdr *pstudiohdr, int sequence, int *pweight )
{
	if (!pstudiohdr || !pstudiohdr->SequencesAvailable() )
	{
		if (pweight)
			*pweight = 0;
		return 0;
	}

	mstudioseqdesc_t &seqdesc = pstudiohdr->pSeqdesc( sequence );

	if (!(seqdesc.flags & STUDIO_ACTIVITY))
	{
		SetActivityForSequence( pstudiohdr, sequence );
	}
	if (pweight)
		*pweight = seqdesc.actweight;
	return seqdesc.activity;
}


void GetAttachmentLocalSpace( CStudioHdr *pstudiohdr, int attachIndex, matrix3x4_t &pLocalToWorld )
{
	if ( attachIndex >= 0 )
	{
		const mstudioattachment_t &pAttachment = pstudiohdr->pAttachment(attachIndex);
		MatrixCopy( pAttachment.local, pLocalToWorld );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pstudiohdr - 
//			*name - 
// Output : int
//-----------------------------------------------------------------------------
int FindHitboxSetByName( CStudioHdr *pstudiohdr, const char *name )
{
	if ( !pstudiohdr )
		return -1;

	for ( int i = 0; i < pstudiohdr->numhitboxsets(); i++ )
	{
		mstudiohitboxset_t *set = pstudiohdr->pHitboxSet( i );
		if ( !set )
			continue;

		if ( !stricmp( set->pszName(), name ) )
			return i;
	}

	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pstudiohdr - 
//			setnumber - 
// Output : char const
//-----------------------------------------------------------------------------
const char *GetHitboxSetName( CStudioHdr *pstudiohdr, int setnumber )
{
	if ( !pstudiohdr )
		return "";

	mstudiohitboxset_t *set = pstudiohdr->pHitboxSet( setnumber );
	if ( !set )
		return "";

	return set->pszName();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pstudiohdr - 
// Output : int
//-----------------------------------------------------------------------------
int GetHitboxSetCount( CStudioHdr *pstudiohdr )
{
	if ( !pstudiohdr )
		return 0;

	return pstudiohdr->numhitboxsets();
}
