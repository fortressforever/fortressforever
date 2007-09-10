/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_timers.cpp
/// @author Shawn Smith (L0ki)
/// @date May 13, 2005
/// @brief implementation of the base class for the client and server timers
/// 
/// implements the shared base class for client and server timers
/// 
/// Revisions
/// ---------
/// May 13, 2005	L0ki: Initial Creation

#include "cbase.h"
#include "ff_timers_shared.h"

//////////////////////////////////////////////////////////////////////////
// CFFTimerBase implementation
//////////////////////////////////////////////////////////////////////////
CFFTimerBase::CFFTimerBase( std::string strName, const float flDuration )
:	m_pfnExpiredCallback(NULL), m_pfnIntervalCallback(NULL), m_flCallbackInterval(0), m_bRemoveWhenExpired(false),
m_flDuration( flDuration ), m_strName( strName )
{
}

void CFFTimerBase::StartTimer( void )
{
	m_flStartTime = gpGlobals->curtime; //engine->Time();	// |-- Mirv: Use curtime instead
}

void CFFTimerBase::ResetTimer( void )
{
	m_flStartTime = -1.0f;
}

void CFFTimerBase::SetExpiredCallback(pfnTimerCallback pfnExpiredCallback, bool bRemoveWhenExpired)
{
	m_pfnExpiredCallback = pfnExpiredCallback;
	m_bRemoveWhenExpired = bRemoveWhenExpired;
}

void CFFTimerBase::SetIntervalCallback(pfnTimerCallback pfnIntervalCallback, float flInterval)
{
	m_pfnIntervalCallback = pfnIntervalCallback;
	m_flCallbackInterval = flInterval;
}

// Jiggles: Added for Hint Timers
void CFFTimerBase::SetHintExpiredCallback(pfnHintTimerCallback pfnExpiredCallback, bool bRemoveWhenExpired)
{
	m_pfnHintExpiredCallback = pfnExpiredCallback;
	m_bRemoveWhenExpired = bRemoveWhenExpired;
}

void CFFTimerBase::SetHintIntervalCallback(pfnHintTimerCallback pfnIntervalCallback, float flInterval)
{
	m_pfnHintIntervalCallback = pfnIntervalCallback;
	m_flCallbackInterval = flInterval;
}


float CFFTimerBase::GetElapsedTime( void )
{
	float elapsed = 0.0f;
	if (HasStarted())
		elapsed = (m_flDuration - GetRemainingTime());
	return elapsed;
}

float CFFTimerBase::GetRemainingTime( void )
{
	float remaining = 0.0f;
	if(HasStarted())
		remaining = (m_flDuration - (/*engine->Time()*/ gpGlobals->curtime - m_flStartTime));	// |-- Mirv: Use curtime instead
	return remaining;
}

bool CFFTimerBase::HasStarted( void )
{
	return (m_flStartTime != -1.0f);
}

bool CFFTimerBase::HasElapsed( void )
{
	return ((/*engine->Time()*/ gpGlobals->curtime - m_flStartTime) >= m_flDuration);	// |-- Mirv: Use curtime instead
}

bool CFFTimerBase::Interval( void )
{
	if( fmod( GetElapsedTime(), m_flCallbackInterval ) == 0 )
		return true;
	return false;
}
std::string CFFTimerBase::GetTimerName( void ) const
{
	return m_strName;
}