//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INTERPOLATEDVAR_H
#define INTERPOLATEDVAR_H
#ifdef _WIN32
#pragma once
#endif

#include "utlfixedlinkedlist.h"
#include "rangecheckedvar.h"
#include "lerp_functions.h"
#include "animationlayer.h"
#include "convar.h"


#define LATCH_ANIMATION_VAR  (1<<0)		// use AnimTime as sample basis
#define LATCH_SIMULATION_VAR (1<<1)		// use SimulationTime as sample basis

#define EXCLUDE_AUTO_LATCH			(1<<2)
#define EXCLUDE_AUTO_INTERPOLATE	(1<<3)

#define INTERPOLATE_LINEAR_ONLY		(1<<4)	// don't do hermite interpolation



#define EXTRA_INTERPOLATION_HISTORY_STORED 0.05f	// It stores this much extra interpolation history,
													// so you can always call Interpolate() this far
													// in the past from your last call and be able to 
													// get an interpolated value.

// this global keeps the last known server packet tick (to avoid calling engine->GetLastTimestamp() all the time)
extern float g_flLastPacketTimestamp;

inline void Interpolation_SetLastPacketTimeStamp( float timestamp)
{
	Assert( timestamp > 0 );
	g_flLastPacketTimestamp = timestamp;
}


// Before calling Interpolate(), you can use this use this to setup the context if 
// you want to enable extrapolation.
class CInterpolationContext
{
public:
	
	CInterpolationContext()
	{
		m_bOldAllowExtrapolation = s_bAllowExtrapolation;
		m_flOldLastTimeStamp = s_flLastTimeStamp;

		// By default, disable extrapolation unless they call EnableExtrapolation.
		s_bAllowExtrapolation = false;

		// this is the context stack
		m_pNext = s_pHead;
		s_pHead = this;
	}
	
	~CInterpolationContext()
	{
		// restore values from prev stack element
		s_bAllowExtrapolation = m_bOldAllowExtrapolation;
		s_flLastTimeStamp = m_flOldLastTimeStamp;

		Assert( s_pHead == this );
		s_pHead = m_pNext;
	}

	static void EnableExtrapolation(bool state)
	{
		s_bAllowExtrapolation = state;
	}

	static bool IsThereAContext()
	{
		return s_pHead != NULL;
	}

	static bool IsExtrapolationAllowed()
	{
		return s_bAllowExtrapolation;
	}

	static void SetLastTimeStamp(float timestamp)
	{
		s_flLastTimeStamp = timestamp;
	}
	
	static float GetLastTimeStamp()
	{
		return s_flLastTimeStamp;
	}


private:

	CInterpolationContext *m_pNext;
	bool m_bOldAllowExtrapolation;
	float m_flOldLastTimeStamp;

	static CInterpolationContext *s_pHead;
	static bool s_bAllowExtrapolation;
	static float s_flLastTimeStamp;
};


extern ConVar cl_extrapolate_amount;


template< class T >
inline T ExtrapolateInterpolatedVarType( const T &oldVal, const T &newVal, float divisor, float flExtrapolationAmount )
{
	return newVal;
}

inline Vector ExtrapolateInterpolatedVarType( const Vector &oldVal, const Vector &newVal, float divisor, float flExtrapolationAmount )
{
	return Lerp( 1.0f + flExtrapolationAmount * divisor, oldVal, newVal );
}

inline float ExtrapolateInterpolatedVarType( const float &oldVal, const float &newVal, float divisor, float flExtrapolationAmount )
{
	return Lerp( 1.0f + flExtrapolationAmount * divisor, oldVal, newVal );
}

inline QAngle ExtrapolateInterpolatedVarType( const QAngle &oldVal, const QAngle &newVal, float divisor, float flExtrapolationAmount )
{
	return Lerp<QAngle>( 1.0f + flExtrapolationAmount * divisor, oldVal, newVal );
}


// -------------------------------------------------------------------------------------------------------------- //
// IInterpolatedVar interface.
// -------------------------------------------------------------------------------------------------------------- //

class IInterpolatedVar
{
public:
	virtual void Setup( void *pValue, int type ) = 0;
	virtual void SetInterpolationAmount( float seconds ) = 0;
	virtual void NoteChanged( float changetime ) = 0;
	virtual void Reset() = 0;
	virtual void Interpolate( float currentTime ) = 0;
	virtual int	 GetType() const = 0;
	virtual void RestoreToLastNetworked() = 0;
	virtual void Copy( IInterpolatedVar *pSrc ) = 0;
};


// -------------------------------------------------------------------------------------------------------------- //
// CInterpolatedVarArray - the main implementation of IInterpolatedVar.
// -------------------------------------------------------------------------------------------------------------- //

template< typename Type, const int COUNT > 
class CInterpolatedVarArray : public IInterpolatedVar
{
public:
	friend class CInterpolatedVarPrivate;

	CInterpolatedVarArray( const char *pDebugName="no debug name" );
	virtual ~CInterpolatedVarArray();

	
// IInterpolatedVar overrides.
public:
	
	virtual void Setup( void *pValue, int type );
	virtual void SetInterpolationAmount( float seconds );
	virtual void NoteChanged( float changetime );
	virtual void Reset();
	virtual void Interpolate( float currentTime );
	virtual int GetType() const;
	virtual void RestoreToLastNetworked();
	virtual void Copy( IInterpolatedVar *pInSrc );


public:

	// Just like the IInterpolatedVar functions, but you can specify an interpolation amount.
	void NoteChanged( float changetime, float interpolation_amount );
	void Interpolate( float currentTime, float interpolation_amount );

	void GetDerivative( Type *pOut, float currentTime );
	void GetDerivative_SmoothVelocity( Type *pOut, float currentTime );	// See notes on ::Derivative_HermiteLinearVelocity for info.

	void ClearHistory();
	void AddToHead( float changeTime, const Type* values, bool bFlushNewer );
	const Type&	GetPrev( int iArrayIndex=0 ) const;
	const Type&	GetCurrent( int iArrayIndex=0 ) const;
	
	// Returns the time difference betweem the most recent sample and its previous sample.
	float	GetInterval() const;
	bool	IsValidIndex( int i );
	Type	*GetHistoryValue( int index, float& changetime, int iArrayIndex=0 );
	int		GetHead();
	int		GetNext( int i );
	void SetHistoryValuesForItem( int item, Type& value );
	void	SetLooping( bool looping, int iArrayIndex=0 );
	
	void SetMaxCount( int newmax );
	int GetMaxCount() const;

	// Get the time of the oldest entry.
	float GetOldestEntry();

	bool GetInterpolationInfo( float currentTime, int *pNewer, int *pOlder, int *pOldest );

protected:

	struct CInterpolatedVarEntry
	{
		float		changetime;
		Type		value[ COUNT ];
	};

	class CInterpolationInfo
	{
	public:
		bool m_bHermite;
		int oldest;	// Only set if using hermite.
		int older;
		int newer;
		float frac;
	};


protected:

	void RemoveOldEntries( float oldesttime );
	void RemoveEntriesPreviousTo( float flTime );

	bool GetInterpolationInfo( 
		CInterpolationInfo *pInfo,
		float currentTime, 
		float interpolation_amount );

	void TimeFixup_Hermite( 
		CInterpolatedVarEntry &fixup,
		CInterpolatedVarEntry*& prev, 
		CInterpolatedVarEntry*& start, 
		CInterpolatedVarEntry*& end	);

	// Force the time between prev and start to be dt (and extend prev out farther if necessary).
	void TimeFixup2_Hermite( 
		CInterpolatedVarEntry &fixup,
		CInterpolatedVarEntry*& prev, 
		CInterpolatedVarEntry*& start, 
		float dt
		);

	void _Extrapolate( 
		Type *pOut,
		CInterpolatedVarEntry *pOld,
		CInterpolatedVarEntry *pNew,
		float flDestinationTime,
		float flMaxExtrapolationAmount
		);

	void _Interpolate( Type *out, float frac, CInterpolatedVarEntry *start, CInterpolatedVarEntry *end );
	void _Interpolate_Hermite( Type *out, float frac, CInterpolatedVarEntry *pOriginalPrev, CInterpolatedVarEntry *start, CInterpolatedVarEntry *end, bool looping = false );
	
	void _Derivative_Hermite( Type *out, float frac, CInterpolatedVarEntry *pOriginalPrev, CInterpolatedVarEntry *start, CInterpolatedVarEntry *end );
	void _Derivative_Hermite_SmoothVelocity( Type *out, float frac, CInterpolatedVarEntry *b, CInterpolatedVarEntry *c, CInterpolatedVarEntry *d );
	void _Derivative_Linear( Type *out, CInterpolatedVarEntry *start, CInterpolatedVarEntry *end );
	
	bool ValidOrder();

	// Get the next element in VarHistory, or just return i if it's an invalid index.
	int SafeNext( int i );


protected:

	// The underlying data element
	Type								*m_pValue;
	// Store networked values so when we latch we can detect which values were changed via networking
	Type								m_LastNetworkedValue[ COUNT ];
	float								m_LastNetworkedTime;
	int									m_fType;
	int									m_ListHead;
	bool								m_bLooping[ COUNT ];
	int									m_nMaxCount;
	float								m_InterpolationAmount;
	const char *m_pDebugName;

	typedef CUtlFixedLinkedList< CInterpolatedVarEntry > VarHistoryType;

	static VarHistoryType& VarHistory()
	{
		static CUtlFixedLinkedList< CInterpolatedVarEntry > ret;
		return ret;
	}
};


template< typename Type, const int COUNT >
inline CInterpolatedVarArray<Type,COUNT>::CInterpolatedVarArray( const char *pDebugName )
{
	m_pDebugName = pDebugName;
	m_pValue = NULL;
	m_fType = LATCH_ANIMATION_VAR;
	m_InterpolationAmount = 0.0f;
	m_ListHead = VarHistory().InvalidIndex();
	memset( m_bLooping, 0x00, sizeof( m_bLooping ) );
	m_nMaxCount = COUNT;
	memset( m_LastNetworkedValue, 0, sizeof( m_LastNetworkedValue ) );
	m_LastNetworkedTime = 0;
}

template< typename Type, const int COUNT >
inline CInterpolatedVarArray<Type,COUNT>::~CInterpolatedVarArray()
{
	ClearHistory();
}

template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::Setup( void *pValue, int type )
{
	m_pValue = ( Type * )pValue;
	m_fType = type;
}

template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::SetInterpolationAmount( float seconds )
{
	m_InterpolationAmount = seconds;
}

template< typename Type, const int COUNT >
inline int CInterpolatedVarArray<Type,COUNT>::GetType() const
{
	return m_fType;
}


template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::NoteChanged( float changetime, float interpolation_amount )
{
	Assert( m_pValue );

	AddToHead( changetime, m_pValue, true );

	memcpy( m_LastNetworkedValue, m_pValue, COUNT * sizeof( Type ) );
	m_LastNetworkedTime = g_flLastPacketTimestamp;
	
	// Since we don't clean out the old entries until Interpolate(), make sure that there
	// aren't any super old entries hanging around.
	RemoveOldEntries( gpGlobals->curtime - interpolation_amount - 2.0f );
}


template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::NoteChanged( float changetime )
{
	NoteChanged( changetime, m_InterpolationAmount );
}


template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::RestoreToLastNetworked()
{
	Assert( m_pValue );
	memcpy( m_pValue, m_LastNetworkedValue, COUNT * sizeof( Type ) );
}

template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::ClearHistory()
{
	while ( m_ListHead != VarHistory().InvalidIndex() )
	{
		int next = VarHistory().Next( m_ListHead );
		VarHistory().Free( m_ListHead );
		m_ListHead = next;
	}

	Assert( m_ListHead == VarHistory().InvalidIndex() );
}

template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::AddToHead( float changeTime, const Type* values, bool bFlushNewer )
{
	int newslot = VarHistory().Alloc( true );
	
	CInterpolatedVarEntry *e = &VarHistory()[ newslot ];
	e->changetime	= changeTime;
	memcpy( e->value, values, m_nMaxCount*sizeof(Type) );

	if ( bFlushNewer )
	{
		// Get rid of anything that has a timestamp after this sample. The server might have
		// corrected our clock and moved us back, so our current changeTime is less than a 
		// changeTime we added samples during previously.
		int insertSpot = m_ListHead;
		while ( insertSpot != VarHistory().InvalidIndex() )
		{
			int next = VarHistory().Next( insertSpot );
			CInterpolatedVarEntry *check = &VarHistory()[ insertSpot ];
			if ( (check->changetime+0.0001f) >= changeTime )
			{
				VarHistory().Free( insertSpot );
				m_ListHead = next;
				insertSpot = m_ListHead;
			}
			else
			{
				break;
			}
		}

		if ( insertSpot == VarHistory().InvalidIndex() )
		{
			m_ListHead = newslot;
		}
		else
		{
			VarHistory().LinkBefore( insertSpot, newslot );
			if ( insertSpot == m_ListHead )
			{
				m_ListHead = newslot;
			}
		}
	}
	else
	{
		int insertSpot = m_ListHead;
		while ( insertSpot != VarHistory().InvalidIndex() )
		{
			CInterpolatedVarEntry *check = &VarHistory()[ insertSpot ];
			if ( check->changetime <= changeTime )
				break;

			int next = VarHistory().Next( insertSpot );
			if ( next == VarHistory().InvalidIndex() )
			{
				VarHistory().LinkAfter( insertSpot, newslot );
				return;
			}
			insertSpot = next;
		}

		VarHistory().LinkBefore( insertSpot, newslot );

		if ( insertSpot == m_ListHead )
		{
			m_ListHead = newslot;
		}
	}
}

template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::Reset()
{
	Assert( m_pValue );

	ClearHistory();

	AddToHead( gpGlobals->curtime, m_pValue, false );
	AddToHead( gpGlobals->curtime, m_pValue, false );
	AddToHead( gpGlobals->curtime, m_pValue, false );

	memcpy( m_LastNetworkedValue, m_pValue, COUNT * sizeof( Type ) );
}


template< typename Type, const int COUNT >
inline float CInterpolatedVarArray<Type,COUNT>::GetOldestEntry()
{
	float lastVal = 0;
	for ( int i = m_ListHead; i != VarHistory().InvalidIndex(); i = VarHistory().Next( i ) )
	{
		lastVal = VarHistory()[i].changetime;
	}
	return lastVal;
}


template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::RemoveOldEntries( float oldesttime )
{
	int c = 0;
	int next = VarHistory().InvalidIndex();
	// Always leave three of entries in the list...
	for ( int i = m_ListHead; i != VarHistory().InvalidIndex(); c++, i = next )
	{
		next = VarHistory().Next( i );

		// Always leave elements 0 1 and 2 alone...
		if ( c <= 2 )
			continue;

		CInterpolatedVarEntry *h = &VarHistory()[ i ];
		// Remove everything off the end until we find the first one that's not too old
		if ( h->changetime > oldesttime )
			continue;

		// Unlink rest of chain
		VarHistory().Free( i );
	}
}


template< typename Type, const int COUNT >
inline int CInterpolatedVarArray<Type,COUNT>::SafeNext( int i )
{
	if ( IsValidIndex( i ) )
		return GetNext( i );
	else
		return i;
}


template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::RemoveEntriesPreviousTo( float flTime )
{
	// Find the 2 samples spanning this time.
	for ( int i=m_ListHead; i != VarHistory().InvalidIndex(); i=VarHistory().Next( i ) )
	{
		if ( VarHistory()[i].changetime < flTime )
		{
			// We need to preserve this sample (ie: the one right before this timestamp)
			// and the sample right before it (for hermite blending), and we can get rid
			// of everything else.
			i = SafeNext( i );
			i = SafeNext( i );
			i = SafeNext( i );	// We keep this one for _Derivative_Hermite_SmoothVelocity.
			
			break;
		}
	}

	// Now remove all samples starting with i.
	int next;
	for ( ; i != VarHistory().InvalidIndex(); i=next )
	{
		next = VarHistory().Next( i );
		VarHistory().Free( i );
	}
}


template< typename Type, const int COUNT >
inline bool CInterpolatedVarArray<Type,COUNT>::GetInterpolationInfo( 
	typename CInterpolatedVarArray<Type,COUNT>::CInterpolationInfo *pInfo,
	float currentTime, 
	float interpolation_amount )
{
	Assert( m_pValue );

	float targettime = currentTime - interpolation_amount;
	int i;

	pInfo->m_bHermite = false;
	pInfo->frac = 0;
	pInfo->oldest = pInfo->older = pInfo->newer = VarHistory().InvalidIndex();
	
	for ( i = m_ListHead; i != VarHistory().InvalidIndex(); i = VarHistory().Next( i ) )
	{
		pInfo->older = i;
		
		float older_change_time = VarHistory()[ i ].changetime;
		if ( older_change_time == 0.0f )
			break;

		if ( targettime < older_change_time )
		{
			pInfo->newer = pInfo->older;
			continue;
		}

		if ( pInfo->newer == VarHistory().InvalidIndex() )
		{
			// Have it linear interpolate between the newest 2 entries.
			pInfo->newer = pInfo->older; 
			return true;
		}

		float newer_change_time = VarHistory()[ pInfo->newer ].changetime;
		float dt = newer_change_time - older_change_time;
		if ( dt > 0.0001f )
		{
			pInfo->frac = ( targettime - older_change_time ) / ( newer_change_time - older_change_time );
			pInfo->frac = min( pInfo->frac, 2.0f );

			int oldestindex = VarHistory().Next( i );

			if ( !(m_fType & INTERPOLATE_LINEAR_ONLY) && oldestindex != VarHistory().InvalidIndex() )
			{
				pInfo->oldest = oldestindex;
				float oldest_change_time = VarHistory()[ oldestindex ].changetime;
				float dt2 = older_change_time - oldest_change_time;
				if ( dt2 > 0.0001f )
				{
					pInfo->m_bHermite = true;
				}
			}
		}
		else
		{
			pInfo->older = pInfo->newer;
		}
		return true;
	}

	// Didn't find any, return last entry???
	if ( pInfo->newer != VarHistory().InvalidIndex() )
	{
		pInfo->older = pInfo->newer;
		return true;
	}

	// This is the single-element case
	pInfo->newer = pInfo->older;
	return (pInfo->older != VarHistory().InvalidIndex());
}


template< typename Type, const int COUNT >
inline bool CInterpolatedVarArray<Type,COUNT>::GetInterpolationInfo( float currentTime, int *pNewer, int *pOlder, int *pOldest )
{
	CInterpolationInfo info;
	bool result = GetInterpolationInfo( &info, currentTime, m_InterpolationAmount );

	if (pNewer)
		*pNewer = info.newer;

	if (pOlder)
		*pOlder = info.older;

	if (pOldest)
		*pOldest = info.oldest;

	return result;
}




template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::Interpolate( float currentTime, float interpolation_amount )
{
	CInterpolationInfo info;
	if (!GetInterpolationInfo( &info, currentTime, interpolation_amount ))
		return;

	VarHistoryType &history = VarHistory();

	if ( info.m_bHermite )
	{
		// base cast, we have 3 valid sample point
		_Interpolate_Hermite( m_pValue, info.frac, &history[info.oldest], &history[info.older], &history[info.newer] );
	}
	else if ( info.newer == info.older  )
	{
		// This means the server clock got way behind the client clock. Extrapolate the value here based on its
		// previous velocity (out to a certain amount).
		int realOlder = SafeNext( info.newer );
		if ( CInterpolationContext::IsExtrapolationAllowed() &&
			IsValidIndex( realOlder ) &&
			history[realOlder].changetime != 0.0 &&
			interpolation_amount > 0.000001f &&
			CInterpolationContext::GetLastTimeStamp() <= m_LastNetworkedTime )
		{
			// At this point, we know we're out of data and we have the ability to get a velocity to extrapolate with.
			//
			// However, we only want to extraploate if the server is choking. We don't want to extrapolate if 
			// the object legimately stopped moving and the server stopped sending updates for it.
			//
			// The way we know that the server is choking is if we haven't heard ANYTHING from it for a while.
			// The server's update interval should be at least as often as our interpolation amount (otherwise,
			// we wouldn't have the ability to interpolate).
			//
			// So right here, if we see that we haven't gotten any server updates since the last interpolation
			// history update to this entity (and since we're in here, we know that we're out of interpolation data),
			// then we can assume that the server is choking and decide to extrapolate.
			//
			// The End

			// Use the velocity here (extrapolate up to 1/4 of a second).
			_Extrapolate( m_pValue, &history[realOlder], &history[info.newer], currentTime - interpolation_amount, cl_extrapolate_amount.GetFloat() );
		}
		else
		{
			_Interpolate( m_pValue, info.frac, &history[info.older], &history[info.newer] );
		}
	}
	else
	{
		_Interpolate( m_pValue, info.frac, &history[info.older], &history[info.newer] );
	}
	
	// Clear out all entries before the oldest since we should never access them again.
	// Usually, Interpolate() calls never go backwards in time, but C_BaseAnimating::BecomeRagdollOnClient for one
	// goes slightly back in time
	RemoveEntriesPreviousTo( currentTime - interpolation_amount - EXTRA_INTERPOLATION_HISTORY_STORED );
}


template< typename Type, const int COUNT >
void CInterpolatedVarArray<Type,COUNT>::GetDerivative( Type *pOut, float currentTime )
{
	CInterpolationInfo info;
	if (!GetInterpolationInfo( &info, currentTime, m_InterpolationAmount ))
		return;

	if ( info.m_bHermite )
	{
		_Derivative_Hermite( pOut, info.frac, &VarHistory()[info.oldest], &VarHistory()[info.older], &VarHistory()[info.newer] );
	}
	else
	{
		_Derivative_Linear( pOut, &VarHistory()[info.older], &VarHistory()[info.newer] );
	}
}


template< typename Type, const int COUNT >
void CInterpolatedVarArray<Type,COUNT>::GetDerivative_SmoothVelocity( Type *pOut, float currentTime )
{
	CInterpolationInfo info;
	if (!GetInterpolationInfo( &info, currentTime, m_InterpolationAmount ))
		return;

	VarHistoryType &history = VarHistory();
	bool bExtrapolate = false;
	int realOlder = 0;
	
	if ( info.m_bHermite )
	{
		_Derivative_Hermite_SmoothVelocity( pOut, info.frac, &history[info.oldest], &history[info.older], &history[info.newer] );
		return;
	}
	else if ( info.newer == info.older && CInterpolationContext::IsExtrapolationAllowed() )
	{
		// This means the server clock got way behind the client clock. Extrapolate the value here based on its
		// previous velocity (out to a certain amount).
		realOlder = SafeNext( info.newer );
		if ( IsValidIndex( realOlder ) && history[realOlder].changetime != 0.0 )
		{
			// At this point, we know we're out of data and we have the ability to get a velocity to extrapolate with.
			//
			// However, we only want to extraploate if the server is choking. We don't want to extrapolate if 
			// the object legimately stopped moving and the server stopped sending updates for it.
			//
			// The way we know that the server is choking is if we haven't heard ANYTHING from it for a while.
			// The server's update interval should be at least as often as our interpolation amount (otherwise,
			// we wouldn't have the ability to interpolate).
			//
			// So right here, if we see that we haven't gotten any server updates for a whole interpolation 
			// interval, then we know the server is choking.
			//
			// The End
			if ( m_InterpolationAmount > 0.000001f &&
				 CInterpolationContext::GetLastTimeStamp() <= (currentTime - m_InterpolationAmount) )
			{
				bExtrapolate = true;
			}
		}
	}

	if ( bExtrapolate )
	{
		// Get the velocity from the last segment.
		_Derivative_Linear( pOut, &history[realOlder], &history[info.newer] );

		// Now ramp it to zero after cl_extrapolate_amount..
		float flDestTime = currentTime - m_InterpolationAmount;
		float diff = flDestTime - history[info.newer].changetime;
		diff = clamp( diff, 0, cl_extrapolate_amount.GetFloat() * 2 );
		if ( diff > cl_extrapolate_amount.GetFloat() )
		{
			float scale = 1 - (diff - cl_extrapolate_amount.GetFloat()) / cl_extrapolate_amount.GetFloat();
			for ( int i=0; i < m_nMaxCount; i++ )
			{
				pOut[i] *= scale;
			}
		}
	}
	else
	{
		_Derivative_Linear( pOut, &history[info.older], &history[info.newer] );
	}

}


template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::Interpolate( float currentTime )
{
	Interpolate( currentTime, m_InterpolationAmount );
}

template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::Copy( IInterpolatedVar *pInSrc )
{
	CInterpolatedVarArray<Type,COUNT> *pSrc = dynamic_cast< CInterpolatedVarArray<Type,COUNT>* >( pInSrc );

	if ( !pSrc )
	{
		Assert( false );
		return;
	}

	Assert( m_fType == pSrc->m_fType );

	for ( int i=0; i < COUNT; i++ )
	{
		m_LastNetworkedValue[i] = pSrc->m_LastNetworkedValue[i];
		m_bLooping[i] = pSrc->m_bLooping[i];
	}

	m_LastNetworkedTime = pSrc->m_LastNetworkedTime;

	// Copy the entries.
	int insertSpot = m_ListHead;
	while ( insertSpot != VarHistory().InvalidIndex() )
	{
		int next = VarHistory().Next( insertSpot );
		VarHistory().Free( insertSpot );
		insertSpot = next;
	}
	m_ListHead = VarHistory().InvalidIndex();

	for ( int srcCur=pSrc->m_ListHead; srcCur != pSrc->VarHistory().InvalidIndex(); srcCur = pSrc->VarHistory().Next( srcCur ) )
	{
		int iNew = VarHistory().AddToTail( pSrc->VarHistory()[srcCur] );
		if ( m_ListHead == VarHistory().InvalidIndex() )
			m_ListHead = iNew;
	}
}

template< typename Type, const int COUNT >
inline const Type& CInterpolatedVarArray<Type,COUNT>::GetPrev( int iArrayIndex ) const
{
	Assert( m_pValue );
	Assert( iArrayIndex >= 0 && iArrayIndex < m_nMaxCount );

	int ihead = m_ListHead;
	if ( ihead != VarHistory().InvalidIndex() )
	{
		ihead = VarHistory().Next( ihead );
		if ( ihead != VarHistory().InvalidIndex() )
		{
			CInterpolatedVarEntry const *h = &VarHistory()[ ihead ];
			return h->value[ iArrayIndex ];
		}
	}
	return m_pValue[ iArrayIndex ];
}

template< typename Type, const int COUNT >
inline const Type& CInterpolatedVarArray<Type,COUNT>::GetCurrent( int iArrayIndex ) const
{
	Assert( m_pValue );
	Assert( iArrayIndex >= 0 && iArrayIndex < m_nMaxCount );

	int ihead = m_ListHead;
	if ( ihead != VarHistory().InvalidIndex() )
	{
		CInterpolatedVarEntry const *h = &VarHistory()[ ihead ];
		return h->value[ iArrayIndex ];
	}
	return m_pValue[ iArrayIndex ];
}

template< typename Type, const int COUNT >
inline float CInterpolatedVarArray<Type,COUNT>::GetInterval() const
{	
	int head = m_ListHead;
	if ( head != VarHistory().InvalidIndex() )
	{
		int next = VarHistory().Next( head );
		if ( next != VarHistory().InvalidIndex() )
		{
			CInterpolatedVarEntry const *h = &VarHistory()[ head ];
			CInterpolatedVarEntry const *n = &VarHistory()[ next ];
			
			return ( h->changetime - n->changetime );
		}
	}

	return 0.0f;
}

template< typename Type, const int COUNT >
inline bool	CInterpolatedVarArray<Type,COUNT>::IsValidIndex( int i )
{
	return VarHistory().IsValidIndex( i );
}

template< typename Type, const int COUNT >
inline Type	*CInterpolatedVarArray<Type,COUNT>::GetHistoryValue( int index, float& changetime, int iArrayIndex )
{
	Assert( iArrayIndex >= 0 && iArrayIndex < m_nMaxCount );
	if ( index == VarHistory().InvalidIndex() )
	{
		changetime = 0.0f;
		return NULL;
	}

	CInterpolatedVarEntry *entry = &VarHistory()[ index ];
	changetime = entry->changetime;
	return &entry->value[ iArrayIndex ];
}

template< typename Type, const int COUNT >
inline int CInterpolatedVarArray<Type,COUNT>::GetHead()
{
	return m_ListHead;
}

template< typename Type, const int COUNT >
inline int CInterpolatedVarArray<Type,COUNT>::GetNext( int i )
{
	return VarHistory().Next( i );
}

template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::SetHistoryValuesForItem( int item, Type& value )
{
	Assert( item >= 0 && item < m_nMaxCount );

	int i;
	for ( i = m_ListHead; i != VarHistory().InvalidIndex(); i = VarHistory().Next( i ) )
	{
		CInterpolatedVarEntry *entry = &VarHistory()[ i ];
		entry->value[ item ] = value;
	}
}

template< typename Type, const int COUNT >
inline void	CInterpolatedVarArray<Type,COUNT>::SetLooping( bool looping, int iArrayIndex )
{
	Assert( iArrayIndex >= 0 && iArrayIndex < m_nMaxCount );
	m_bLooping[ iArrayIndex ] = looping;
}

template< typename Type, const int COUNT >
inline void	CInterpolatedVarArray<Type,COUNT>::SetMaxCount( int newmax )
{
	Assert( newmax <= COUNT );
	bool changed = ( newmax != m_nMaxCount ) ? true : false;
	m_nMaxCount = newmax;
	// Wipe everything any time this changes!!!
	if ( changed )
	{
		Reset();
	}
}


template< typename Type, const int COUNT >
inline int CInterpolatedVarArray<Type,COUNT>::GetMaxCount() const
{
	return m_nMaxCount;
}


template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::_Interpolate( Type *out, float frac, CInterpolatedVarEntry *start, CInterpolatedVarEntry *end )
{
	Assert( start );
	Assert( end );
	
	if ( start == end )
	{
		// quick exit
		for ( int i = 0; i < m_nMaxCount; i++ )
		{
			out[i] = end->value[i];
			Lerp_Clamp( out[i] );
		}
		return;
	}

	Assert( frac >= 0.0f && frac <= 1.0f );

	// Note that QAngle has a specialization that will do quaternion interpolation here...
	for ( int i = 0; i < m_nMaxCount; i++ )
	{
		if ( m_bLooping[ i ] )
		{
			out[i] = LoopingLerp( frac, start->value[i], end->value[i] );
		}
		else
		{
			out[i] = Lerp( frac, start->value[i], end->value[i] );
		}
		Lerp_Clamp( out[i] );
	}
}


template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::_Extrapolate( 
	Type *pOut,
	CInterpolatedVarEntry *pOld,
	CInterpolatedVarEntry *pNew,
	float flDestinationTime,
	float flMaxExtrapolationAmount
	)
{
	if ( fabs( pOld->changetime - pNew->changetime ) < 0.001f || flDestinationTime <= pNew->changetime )
	{
		for ( int i=0; i < m_nMaxCount; i++ )
			pOut[i] = pNew->value[i];
	}
	else
	{
		float flExtrapolationAmount = min( flDestinationTime - pNew->changetime, flMaxExtrapolationAmount );

		float divisor = 1.0f / (pNew->changetime - pOld->changetime);
		for ( int i=0; i < m_nMaxCount; i++ )
		{
			pOut[i] = ExtrapolateInterpolatedVarType( pOld->value[i], pNew->value[i], divisor, flExtrapolationAmount );
		}
	}
}


template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::TimeFixup2_Hermite( 
	typename CInterpolatedVarArray<Type,COUNT>::CInterpolatedVarEntry &fixup,
	typename CInterpolatedVarArray<Type,COUNT>::CInterpolatedVarEntry*& prev, 
	typename CInterpolatedVarArray<Type,COUNT>::CInterpolatedVarEntry*& start, 
	float dt1
	)
{
	float dt2 = start->changetime - prev->changetime;

	// If times are not of the same interval renormalize the earlier sample to allow for uniform hermite spline interpolation
	if ( fabs( dt1 - dt2 ) > 0.0001f &&
		dt2 > 0.0001f )
	{
		// Renormalize
		float frac = dt1 / dt2;

		// Fixed interval into past
		fixup.changetime = start->changetime - dt1;

		for ( int i = 0; i < m_nMaxCount; i++ )
		{
			fixup.value[i] = Lerp( 1-frac, prev->value[i], start->value[i] );
		}

		// Point previous sample at fixed version
		prev = &fixup;
	}
}


template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::TimeFixup_Hermite( 
	typename CInterpolatedVarArray<Type,COUNT>::CInterpolatedVarEntry &fixup,
	typename CInterpolatedVarArray<Type,COUNT>::CInterpolatedVarEntry*& prev, 
	typename CInterpolatedVarArray<Type,COUNT>::CInterpolatedVarEntry*& start, 
	typename CInterpolatedVarArray<Type,COUNT>::CInterpolatedVarEntry*& end	)
{
	TimeFixup2_Hermite( fixup, prev, start, end->changetime - start->changetime );
}


template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::_Interpolate_Hermite( 
	Type *out, 
	float frac, 
	CInterpolatedVarEntry *prev, 
	CInterpolatedVarEntry *start, 
	CInterpolatedVarEntry *end, 
	bool looping )
{
	Assert( start );
	Assert( end );

	// Disable range checks because we can produce weird values here and it's not an error.
	// After interpolation, we will clamp the values.
	CDisableRangeChecks disableRangeChecks; 

	CInterpolatedVarEntry fixup;
	TimeFixup_Hermite( fixup, prev, start, end );

	for( int i = 0; i < m_nMaxCount; i++ )
	{
		// Note that QAngle has a specialization that will do quaternion interpolation here...
		if ( m_bLooping[ i ] )
		{
			out[ i ] = LoopingLerp_Hermite( frac, prev->value[i], start->value[i], end->value[i] );
		}
		else
		{
			out[ i ] = Lerp_Hermite( frac, prev->value[i], start->value[i], end->value[i] );
		}

		// Clamp the output from interpolation. There are edge cases where something like m_flCycle
		// can get set to a really high or low value when we set it to zero after a really small
		// time interval (the hermite blender will think it's got a really high velocity and
		// skyrocket it off into la-la land).
		Lerp_Clamp( out[i] );
	}
}

template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::_Derivative_Hermite( 
	Type *out, 
	float frac, 
	CInterpolatedVarEntry *prev, 
	CInterpolatedVarEntry *start, 
	CInterpolatedVarEntry *end )
{
	Assert( start );
	Assert( end );

	// Disable range checks because we can produce weird values here and it's not an error.
	// After interpolation, we will clamp the values.
	CDisableRangeChecks disableRangeChecks; 

	CInterpolatedVarEntry fixup;
	TimeFixup_Hermite( fixup, prev, start, end );

	float divisor = 1.0f / (end->changetime - start->changetime);

	for( int i = 0; i < m_nMaxCount; i++ )
	{
		Assert( !m_bLooping[ i ] );
		out[i] = Derivative_Hermite( frac, prev->value[i], start->value[i], end->value[i] );
		out[i] *= divisor;
	}
}


template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::_Derivative_Hermite_SmoothVelocity( 
	Type *out, 
	float frac, 
	CInterpolatedVarEntry *b, 
	CInterpolatedVarEntry *c, 
	CInterpolatedVarEntry *d )
{
	CInterpolatedVarEntry fixup;
	TimeFixup_Hermite( fixup, b, c, d );
	for ( int i=0; i < m_nMaxCount; i++ )
	{
		Type prevVel = (c->value[i] - b->value[i]) / (c->changetime - b->changetime);
		Type curVel  = (d->value[i] - c->value[i]) / (d->changetime - c->changetime);
		out[i] = Lerp( frac, prevVel, curVel );
	}
}


template< typename Type, const int COUNT >
inline void CInterpolatedVarArray<Type,COUNT>::_Derivative_Linear( 
	Type *out, 
	CInterpolatedVarEntry *start, 
	CInterpolatedVarEntry *end )
{
	if ( start == end || fabs( start->changetime - end->changetime ) < 0.0001f )
	{
		for( int i = 0; i < m_nMaxCount; i++ )
		{
			out[ i ] = start->value[i] * 0;
		}
	}
	else 
	{
		float divisor = 1.0f / (end->changetime - start->changetime);
		for( int i = 0; i < m_nMaxCount; i++ )
		{
			out[ i ] = (end->value[i] - start->value[i]) * divisor;
		}
	}
}


template< typename Type, const int COUNT >
inline bool CInterpolatedVarArray<Type,COUNT>::ValidOrder()
{
	float newestchangetime = 0.0f;
	bool first = true;
	for ( int i = GetHead(); IsValidIndex( i ); i = GetNext( i ) )
	{
		CInterpolatedVarEntry *entry = &VarHistory()[ i ];
		if ( first )
		{
			first = false;
			newestchangetime = entry->changetime;
			continue;
		}

		// They should get older as wel walk backwards
		if ( entry->changetime > newestchangetime )
		{
			Assert( 0 );
			return false;
		}

		newestchangetime = entry->changetime;
	}

	return true;
}


// -------------------------------------------------------------------------------------------------------------- //
// CInterpolatedVar.
// -------------------------------------------------------------------------------------------------------------- //

template< typename Type >
class CInterpolatedVar : public CInterpolatedVarArray< Type, 1 >
{
public:
	CInterpolatedVar( const char *pDebugName="no debug name" );
};

template< typename Type >
inline CInterpolatedVar<Type>::CInterpolatedVar( const char *pDebugName )
	: CInterpolatedVarArray< Type, 1 >( pDebugName )
{
}


#endif // INTERPOLATEDVAR_H
