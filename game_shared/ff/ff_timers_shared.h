/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_timers_shared.h
/// @author Shawn Smith (L0ki)
/// @date May 13, 2005
/// @brief declaration of the base class for the client and server timers
/// 
/// declares the shared base class for client and server timers
/// 
/// Revisions
/// ---------
/// May 13, 2005	L0ki: Initial Creation
/// May 1, 2007		Jiggles: Added support for hint timers

#ifndef FF_TIMERS_SHARED_H
#define FF_TIMERS_SHARED_H

//#include "util_shared.h"
#include <string>
#include <vector>

// no using namespace in headers please... -> Defrag
//using namespace std;

//maximum amount of timers on the client, do we even need this many?
//	1 - detpack countdown
//	2grenades
//	buildable objects (sentry, dispenser, detpack)
#define MAX_FF_CLIENT_TIMERS 8

//maximum amount of server only timers
#define MAX_FF_SERVER_TIMERS 32

//forward declaration for the timer class
class C_FFTimer;
class CFFTimer;

//forward declaration for the hint timer class
class C_FFHintTimer;
class CFFHintTimer;


//callback function for timer events
#ifdef CLIENT_DLL
	typedef void (*pfnTimerCallback)(C_FFTimer *pTimer);
	typedef void (*pfnHintTimerCallback)(C_FFHintTimer *pTimer);
#else
	typedef void (*pfnTimerCallback)(CFFTimer *pTimer);
	typedef void (*pfnHintTimerCallback)(CFFHintTimer *pTimer);
#endif

// basic class for both client and server timers, since they share the same basic functionality
//	this class acts as a very simple countdown timer
class CFFTimerBase
{
public:
	//constructor
	CFFTimerBase( std::string strName, const float flDuration );

	//timer controls
	void StartTimer( void );
	void ResetTimer( void );

	//callbacks
	void SetExpiredCallback(pfnTimerCallback pfnExpiredCallback, bool bRemoveWhenExpired);
	void SetIntervalCallback(pfnTimerCallback pfnIntervalCallback, float flInterval);

	void SetHintExpiredCallback( pfnHintTimerCallback, bool );
	void SetHintIntervalCallback( pfnHintTimerCallback, float );

	//utility
	float GetElapsedTime( void );
	float GetRemainingTime( void );
	bool HasStarted( void );
	bool HasElapsed( void );
	bool Interval( void );
	std::string GetTimerName( void ) const;

	pfnTimerCallback	m_pfnExpiredCallback;
	pfnTimerCallback	m_pfnIntervalCallback;
	float				m_flCallbackInterval;
	bool				m_bRemoveWhenExpired;

	pfnHintTimerCallback	m_pfnHintExpiredCallback;
	pfnHintTimerCallback	m_pfnHintIntervalCallback;

protected:
	float		m_flDuration;
	float		m_flStartTime;
	std::string m_strName;
};

#endif//FF_TIMERS_SHARED_H