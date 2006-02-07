//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "c_baseanimatingoverlay.h"
#include "bone_setup.h"
#include "tier0/vprof.h"
#include "engine/IVDebugOverlay.h"
#include "engine/IVEngineCache.h"
#include "eventlist.h"

#include "dt_utlvector_recv.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar r_sequence_debug;

C_BaseAnimatingOverlay::C_BaseAnimatingOverlay()
{
	// FIXME: where does this initialization go now?
	//for ( int i=0; i < MAX_OVERLAYS; i++ )
	//{
	//	memset( &m_Layer[i], 0, sizeof(m_Layer[0]) );
	//	m_Layer[i].nOrder = MAX_OVERLAYS;
	//}

	// FIXME: where does this initialization go now?
	// AddVar( m_Layer, &m_iv_AnimOverlay, LATCH_ANIMATION_VAR );
}

#undef CBaseAnimatingOverlay



BEGIN_RECV_TABLE_NOBASE(CAnimationLayer, DT_Animationlayer)
	RecvPropInt(	RECVINFO_NAME(nSequence,sequence)),
	RecvPropFloat(	RECVINFO_NAME(flCycle,cycle)),
	RecvPropFloat(	RECVINFO_NAME(flPrevCycle,prevcycle)),
	RecvPropFloat(	RECVINFO_NAME(flWeight,weight)),
	RecvPropInt(	RECVINFO_NAME(nOrder,order))
END_RECV_TABLE()


void ResizeAnimationLayerCallback( void *pStruct, int offsetToUtlVector, int len )
{
	C_BaseAnimatingOverlay *pEnt = (C_BaseAnimatingOverlay*)pStruct;
	CUtlVector < C_AnimationLayer > *pVec = &pEnt->m_AnimOverlay;
	CUtlVector< CInterpolatedVar< C_AnimationLayer > > *pVecIV = &pEnt->m_iv_AnimOverlay;
	
	Assert( (char*)pVec - (char*)pEnt == offsetToUtlVector );
	Assert( pVec->Count() == pVecIV->Count() );
	
	int diff = len - pVec->Count();

	if ( diff == 0 )
		return;

	// remove all entries
	for ( int i=0; i < pVec->Count(); i++ )
	{
		pEnt->RemoveVar( &pVec->Element( i ) );
	}

	// adjust vector sizes
	if ( diff > 0 )
	{
		pVec->AddMultipleToTail( diff );
		pVecIV->AddMultipleToTail( diff );
	}
	else
	{
		pVec->RemoveMultiple( len, -diff );
		pVecIV->RemoveMultiple( len, -diff );
	}

	// Rebind all the variables in the ent's list.
	for ( i=0; i < len; i++ )
	{
		pEnt->AddVar( &pVec->Element( i ), &pVecIV->Element( i ), LATCH_ANIMATION_VAR, true );
	}
	// FIXME: need to set historical values of nOrder in pVecIV to MAX_OVERLAY
	
}


BEGIN_RECV_TABLE_NOBASE( C_BaseAnimatingOverlay, DT_OverlayVars )
	 RecvPropUtlVector( 
		RECVINFO_UTLVECTOR_SIZEFN( m_AnimOverlay, ResizeAnimationLayerCallback ), 
		C_BaseAnimatingOverlay::MAX_OVERLAYS,
		RecvPropDataTable(NULL, 0, 0, &REFERENCE_RECV_TABLE( DT_Animationlayer ) ) )
END_RECV_TABLE()


IMPLEMENT_CLIENTCLASS_DT( C_BaseAnimatingOverlay, DT_BaseAnimatingOverlay, CBaseAnimatingOverlay )
	RecvPropDataTable( "overlay_vars", 0, 0, &REFERENCE_RECV_TABLE( DT_OverlayVars ) )
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_BaseAnimatingOverlay )

/*
	DEFINE_FIELD( C_BaseAnimatingOverlay, m_Layer[0][2].nSequence, FIELD_INTEGER ),
	DEFINE_FIELD( C_BaseAnimatingOverlay, m_Layer[0][2].flCycle, FIELD_FLOAT ),
	DEFINE_FIELD( C_BaseAnimatingOverlay, m_Layer[0][2].flPlaybackrate, FIELD_FLOAT),
	DEFINE_FIELD( C_BaseAnimatingOverlay, m_Layer[0][2].flWeight, FIELD_FLOAT),
	DEFINE_FIELD( C_BaseAnimatingOverlay, m_Layer[1][2].nSequence, FIELD_INTEGER ),
	DEFINE_FIELD( C_BaseAnimatingOverlay, m_Layer[1][2].flCycle, FIELD_FLOAT ),
	DEFINE_FIELD( C_BaseAnimatingOverlay, m_Layer[1][2].flPlaybackrate, FIELD_FLOAT),
	DEFINE_FIELD( C_BaseAnimatingOverlay, m_Layer[1][2].flWeight, FIELD_FLOAT),
	DEFINE_FIELD( C_BaseAnimatingOverlay, m_Layer[2][2].nSequence, FIELD_INTEGER ),
	DEFINE_FIELD( C_BaseAnimatingOverlay, m_Layer[2][2].flCycle, FIELD_FLOAT ),
	DEFINE_FIELD( C_BaseAnimatingOverlay, m_Layer[2][2].flPlaybackrate, FIELD_FLOAT),
	DEFINE_FIELD( C_BaseAnimatingOverlay, m_Layer[2][2].flWeight, FIELD_FLOAT),
	DEFINE_FIELD( C_BaseAnimatingOverlay, m_Layer[3][2].nSequence, FIELD_INTEGER ),
	DEFINE_FIELD( C_BaseAnimatingOverlay, m_Layer[3][2].flCycle, FIELD_FLOAT ),
	DEFINE_FIELD( C_BaseAnimatingOverlay, m_Layer[3][2].flPlaybackrate, FIELD_FLOAT),
	DEFINE_FIELD( C_BaseAnimatingOverlay, m_Layer[3][2].flWeight, FIELD_FLOAT),
*/

END_PREDICTION_DATA()


C_AnimationLayer* C_BaseAnimatingOverlay::GetAnimOverlay( int i )
{
	Assert( i >= 0 && i < MAX_OVERLAYS );
	return &m_AnimOverlay[i];
}


void C_BaseAnimatingOverlay::SetNumAnimOverlays( int num )
{
	if ( m_AnimOverlay.Count() < num )
	{
		m_AnimOverlay.AddMultipleToTail( num - m_AnimOverlay.Count() );
	}
	else if ( m_AnimOverlay.Count() > num )
	{
		m_AnimOverlay.RemoveMultiple( num, m_AnimOverlay.Count() - num );
	}
}


int C_BaseAnimatingOverlay::GetNumAnimOverlays() const
{
	return m_AnimOverlay.Count();
}


void C_BaseAnimatingOverlay::GetRenderBounds( Vector& theMins, Vector& theMaxs )
{
	BaseClass::GetRenderBounds( theMins, theMaxs );

	if ( !IsRagdoll() )
	{
		studiohdr_t *pStudioHdr = modelinfo->GetStudiomodel( GetModel() );
		if (!pStudioHdr)
			return;

		int nSequences = pStudioHdr->GetNumSeq();

		int i;
		for (i = 0; i < m_AnimOverlay.Count(); i++)
		{
			if (m_AnimOverlay[i].flWeight > 0.0)
			{
				if ( m_AnimOverlay[i].nSequence >= nSequences )
				{
					continue;
				}

				mstudioseqdesc_t &seqdesc = pStudioHdr->pSeqdesc( m_AnimOverlay[i].nSequence );
				VectorMin( seqdesc.bbmin, theMins, theMins );
				VectorMax( seqdesc.bbmax, theMaxs, theMaxs );
			}
		}
	}
}



void C_BaseAnimatingOverlay::CheckForLayerChanges( studiohdr_t *hdr, float currentTime )
{
	CDisableRangeChecks disableRangeChecks;
	
	// FIXME: damn, there has to be a better way than this.
	int i;
	for (i = 0; i < m_iv_AnimOverlay.Count(); i++)
	{
		CDisableRangeChecks disableRangeChecks; 

		int iHead, iPrev1, iPrev2;
		m_iv_AnimOverlay[i].GetInterpolationInfo( currentTime, &iHead, &iPrev1, &iPrev2 );

		// fake up previous cycle values.
		float t0;
		C_AnimationLayer *pHead = m_iv_AnimOverlay[i].GetHistoryValue( iHead, t0 );
		// reset previous
		float t1;
		C_AnimationLayer *pPrev1 = m_iv_AnimOverlay[i].GetHistoryValue( iPrev1, t1 );
		// reset previous previous
		float t2;
		C_AnimationLayer *pPrev2 = m_iv_AnimOverlay[i].GetHistoryValue( iPrev2, t2 );

		if ( pHead && pPrev1 && pHead->nSequence != pPrev1->nSequence )
		{
	#if _DEBUG
			if ( stricmp( r_sequence_debug.GetString(), hdr->name ) == 0)
			{
				DevMsgRT( "(%5.2f : %30s : %5.3f : %4.2f : %1d)\n", t0, hdr->pSeqdesc( pHead->nSequence ).pszLabel(),  (float)pHead->flCycle,  (float)pHead->flWeight, i );
				DevMsgRT( "(%5.2f : %30s : %5.3f : %4.2f : %1d)\n", t1, hdr->pSeqdesc( pPrev1->nSequence ).pszLabel(),  (float)pPrev1->flCycle, (float)pPrev1->flWeight, i );
				if (pPrev2)
					DevMsgRT( "(%5.2f : %30s : %5.3f : %4.2f : %1d)\n", t2, hdr->pSeqdesc( pPrev2->nSequence ).pszLabel(),  (float)pPrev2->flCycle,  (float)pPrev2->flWeight, i );
			}
	#endif

			if (pPrev1)
			{
				pPrev1->nSequence = pHead->nSequence;
				pPrev1->flCycle = pHead->flPrevCycle;
				pPrev1->flWeight = pHead->flWeight;
			}

			if (pPrev2)
			{
				float num = 0;
				if ( fabs( t0 - t1 ) > 0.001f )
					num = (t2 - t1) / (t0 - t1);

				pPrev2->nSequence = pHead->nSequence;
				float flTemp;
				if (IsSequenceLooping( pHead->nSequence ))
				{
					flTemp = LoopingLerp( num, (float)pHead->flPrevCycle, (float)pHead->flCycle );
				}
				else
				{
					flTemp = Lerp( num, (float)pHead->flPrevCycle, (float)pHead->flCycle );
				}
				pPrev2->flCycle = flTemp;
				pPrev2->flWeight = pHead->flWeight;
			}

			/*
			if (stricmp( r_seq_overlay_debug.GetString(), hdr->name ) == 0)
			{
				DevMsgRT( "(%30s %6.2f : %6.2f : %6.2f)\n", hdr->pSeqdesc( pHead->nSequence ).pszLabel(), (float)pPrev2->flCycle, (float)pPrev1->flCycle, (float)pHead->flCycle );
			}
			*/

			m_iv_AnimOverlay[i].SetLooping( IsSequenceLooping( pHead->nSequence ) );
			m_iv_AnimOverlay[i].Interpolate( currentTime );

			// reset event indexes
			m_flOverlayPrevEventCycle[i] = pHead->flPrevCycle - 0.01;
		}
	}
}



void C_BaseAnimatingOverlay::AccumulateLayers( studiohdr_t *hdr, Vector pos[], Quaternion q[], float poseparam[], float currentTime, int boneMask )
{
	BaseClass::AccumulateLayers( hdr, pos, q, poseparam, currentTime, boneMask );
	int i;

	// resort the layers
	int layer[MAX_OVERLAYS];
	for (i = 0; i < MAX_OVERLAYS; i++)
	{
		layer[i] = MAX_OVERLAYS;
	}
	for (i = 0; i < m_AnimOverlay.Count(); i++)
	{
		if (m_AnimOverlay[i].nOrder < MAX_OVERLAYS)
		{
			/*
			Assert( layer[m_AnimOverlay[i].nOrder] == MAX_OVERLAYS );
			layer[m_AnimOverlay[i].nOrder] = i;
			*/
			// hacky code until initialization of new layers is finished
			if (layer[m_AnimOverlay[i].nOrder] != MAX_OVERLAYS)
			{
				m_AnimOverlay[i].nOrder = MAX_OVERLAYS;
			}
			else
			{
				layer[m_AnimOverlay[i].nOrder] = i;
			}
		}
	}

	CheckForLayerChanges( hdr, currentTime );

	int nSequences = hdr->GetNumSeq();

	// add in the overlay layers
	int j;
	for (j = 0; j < MAX_OVERLAYS; j++)
	{
		i = layer[ j ];
		if (i < m_AnimOverlay.Count())
		{
			if ( m_AnimOverlay[i].nSequence >= nSequences )
			{
				continue;
			}

			/*
			DevMsgRT( 1 , "%.3f  %.3f  %.3f\n", currentTime, fWeight, dadt );
			debugoverlay->AddTextOverlay( GetAbsOrigin() + Vector( 0, 0, 64 ), -j - 1, 0, 
				"%2d(%s) : %6.2f : %6.2f", 
					m_AnimOverlay[i].nSequence,
					hdr->pSeqdesc( m_AnimOverlay[i].nSequence )->pszLabel(),
					m_AnimOverlay[i].flCycle, 
					m_AnimOverlay[i].flWeight
					);
			*/

			float fWeight = m_AnimOverlay[i].flWeight;

			if (fWeight > 0)
			{
				// check to see if the sequence changed
				// FIXME: move this to somewhere more reasonable
				// do a nice spline interpolation of the values
				// if ( m_AnimOverlay[i].nSequence != m_iv_AnimOverlay.GetPrev( i )->nSequence )
				float fCycle = m_AnimOverlay[ i ].flCycle;

				fCycle = ClampCycle( fCycle, IsSequenceLooping( m_AnimOverlay[i].nSequence ) );

				if (fWeight > 1)
					fWeight = 1;

				AccumulatePose( hdr, m_pIk, pos, q, m_AnimOverlay[i].nSequence, fCycle, poseparam, boneMask, fWeight, currentTime );

#if _DEBUG
				if (stricmp( r_sequence_debug.GetString(), hdr->name ) == 0)
				{
					if (1)
					{
						DevMsgRT( "%6.2f : %30s : %5.3f : %4.2f : %1d\n", currentTime, hdr->pSeqdesc( m_AnimOverlay[i].nSequence ).pszLabel(), fCycle, fWeight, i );
					}
					else
					{
						int iHead, iPrev1, iPrev2;
						m_iv_AnimOverlay[i].GetInterpolationInfo( currentTime, &iHead, &iPrev1, &iPrev2 );

						// fake up previous cycle values.
						float t0;
						C_AnimationLayer *pHead = m_iv_AnimOverlay[i].GetHistoryValue( iHead, t0 );
						// reset previous
						float t1;
						C_AnimationLayer *pPrev1 = m_iv_AnimOverlay[i].GetHistoryValue( iPrev1, t1 );
						// reset previous previous
						float t2;
						C_AnimationLayer *pPrev2 = m_iv_AnimOverlay[i].GetHistoryValue( iPrev2, t2 );

						if ( pHead && pPrev1 && pPrev2 )
						{
							DevMsgRT( "%6.2f : %30s %6.2f (%6.2f:%6.2f:%6.2f) : %6.2f (%6.2f:%6.2f:%6.2f) : %1d\n", currentTime, hdr->pSeqdesc( m_AnimOverlay[i].nSequence ).pszLabel(), 
								fCycle, (float)pPrev2->flCycle, (float)pPrev1->flCycle, (float)pHead->flCycle,
								fWeight, (float)pPrev2->flWeight, (float)pPrev1->flWeight, (float)pHead->flWeight,
								i );
						}
						else
						{
							DevMsgRT( "%6.2f : %30s %6.2f : %6.2f : %1d\n", currentTime, hdr->pSeqdesc( m_AnimOverlay[i].nSequence ).pszLabel(), fCycle, fWeight, i );
						}

					}
				}
#endif

//#define DEBUG_TF2_OVERLAYS
#if defined( DEBUG_TF2_OVERLAYS )
				engine->Con_NPrintf( 10 + j, "%30s %6.2f : %6.2f : %1d", hdr->pSeqdesc( m_AnimOverlay[i].nSequence ).pszLabel(), fCycle, fWeight, i );
			}
			else
			{
				engine->Con_NPrintf( 10 + j, "%30s %6.2f : %6.2f : %1d", "            ", 0.f, 0.f, i );
#endif
			}
		}
#if defined( DEBUG_TF2_OVERLAYS )
		else
		{
			engine->Con_NPrintf( 10 + j, "%30s %6.2f : %6.2f : %1d", "            ", 0.f, 0.f, i );
		}
#endif
	}
}

void C_BaseAnimatingOverlay::DoAnimationEvents( void )
{
	// don't fire events when paused
	if ( gpGlobals->frametime == 0.0f  )
		return;

	studiohdr_t *hdr = GetModelPtr();
	if ( !hdr )
	{
		return;
	}

	engineCache->EnterCriticalSection();

	int nSequences = hdr->GetNumSeq();

	BaseClass::DoAnimationEvents( );

	bool watch = false; // Q_strstr( hdr->name, "rifle" ) ? true : false;

	CheckForLayerChanges( hdr, gpGlobals->curtime ); // !!!

	int j;
	for (j = 0; j < m_AnimOverlay.Count(); j++)
	{
		if ( m_AnimOverlay[j].nSequence >= nSequences )
		{
			continue;
		}

		mstudioseqdesc_t &seqdesc = hdr->pSeqdesc( m_AnimOverlay[j].nSequence );
		if ( seqdesc.numevents == 0 )
			continue;

		// stalled?
		if (m_AnimOverlay[j].flCycle == m_flOverlayPrevEventCycle[j])
			continue;

		// check for looping
		BOOL bLooped = false;
		if (m_AnimOverlay[j].flCycle <= m_flOverlayPrevEventCycle[j])
		{
			if (m_flOverlayPrevEventCycle[j] - m_AnimOverlay[j].flCycle > 0.5)
			{
				bLooped = true;
			}
			else
			{
				engineCache->ExitCriticalSection( );
				// things have backed up, which is bad since it'll probably result in a hitch in the animation playback
				// but, don't play events again for the same time slice
				return;
			}
		}

		mstudioevent_t *pevent = seqdesc.pEvent( 0 );

		// This makes sure events that occur at the end of a sequence occur are
		// sent before events that occur at the beginning of a sequence.
		if (bLooped)
		{
			for (int i = 0; i < seqdesc.numevents; i++)
			{
				// ignore all non-client-side events
				if ( pevent[i].type & AE_TYPE_NEWEVENTSYSTEM )
				{
					if ( !( pevent[i].type & AE_TYPE_CLIENT ) )
						 continue;
				}
				else if ( pevent[i].event < 5000 ) //Adrian - Support the old event system
					continue;
			
				if ( pevent[i].cycle <= m_flOverlayPrevEventCycle[j] )
					continue;
				
				if ( watch )
				{
					Msg( "%i FE %i Looped cycle %f, prev %f ev %f (time %.3f)\n",
						gpGlobals->tickcount,
						pevent[i].event,
						pevent[i].cycle,
						m_flOverlayPrevEventCycle[j],
						m_AnimOverlay[j].flCycle,
						gpGlobals->curtime );
				}
					
					
				FireEvent( GetAbsOrigin(), GetAbsAngles(), pevent[ i ].event, pevent[ i ].options );
			}

			// Necessary to get the next loop working
			m_flOverlayPrevEventCycle[j] = -0.01;
		}

		for (int i = 0; i < seqdesc.numevents; i++)
		{
			if ( pevent[i].type & AE_TYPE_NEWEVENTSYSTEM )
			{
				if ( !( pevent[i].type & AE_TYPE_CLIENT ) )
					 continue;
			}
			else if ( pevent[i].event < 5000 ) //Adrian - Support the old event system
				continue;

			if ( (pevent[i].cycle > m_flOverlayPrevEventCycle[j] && pevent[i].cycle <= m_AnimOverlay[j].flCycle) )
			{
				if ( watch )
				{
					Msg( "%i (seq: %d) FE %i Normal cycle %f, prev %f ev %f (time %.3f)\n",
						gpGlobals->tickcount,
						m_AnimOverlay[j].nSequence,
						pevent[i].event,
						pevent[i].cycle,
						m_flOverlayPrevEventCycle[j],
						m_AnimOverlay[j].flCycle,
						gpGlobals->curtime );
				}

				FireEvent( GetAbsOrigin(), GetAbsAngles(), pevent[ i ].event, pevent[ i ].options );
			}
		}

		m_flOverlayPrevEventCycle[j] = m_AnimOverlay[j].flCycle;
	}

	engineCache->ExitCriticalSection( );
}

