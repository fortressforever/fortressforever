/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file c_ff_timers.cpp
/// @author Christopher Boylan (Jiggles)
/// @date May 1, 2007
/// @brief implementation of the client side hint timer class
/// 
/// 
/// Revisions
/// ---------
/// May 1, 2007	  Jiggles: Initial Creation -- based on c_ff_timers, but without the vgui code

#include "cbase.h"
#include "c_ff_hint_timers.h"
#include <algorithm>

C_FFHintTimerManager g_FFHintTimers;

C_FFHintTimer::C_FFHintTimer( string strName, const float flDuration )
: CFFTimerBase( strName.c_str(), flDuration )
{
	m_bPaused = false;
	m_flPausedTime = 0.0f;
}

C_FFHintTimer::~C_FFHintTimer()
{
}

void C_FFHintTimer::StartTimer( void )
{
	CFFTimerBase::StartTimer();
}

void C_FFHintTimer::ResetTimer( void )
{
	CFFTimerBase::ResetTimer();
}


void C_FFHintTimer::Pause( void )
{
	if ( m_bPaused )  // Already paused
		return;

	m_flPausedTime = gpGlobals->curtime;
	m_bPaused = true;
}
void C_FFHintTimer::Unpause( void )
{
	if ( !m_bPaused )  // Already unpaused
		return;

	// Just add in the time the timer was paused to the duration
	float flNewDuration = m_flDuration + ( gpGlobals->curtime - m_flPausedTime );
	if ( flNewDuration > 0 )
		m_flDuration = flNewDuration;
	m_bPaused = false;
}


//////////////////////////////////////////////////////////////////////////
// Client side Timer Manager class
//////////////////////////////////////////////////////////////////////////
C_FFHintTimerManager::C_FFHintTimerManager()
{
}

C_FFHintTimerManager::~C_FFHintTimerManager()
{
	//DevMsg("C_FFHintTimerManager::~C_FFHintTimerManager\n");
	DeleteAll();
}

C_FFHintTimer* C_FFHintTimerManager::Create( string strName, float flDuration )
{
	//DevMsg("C_FFHintTimerManager::Create(%s, %f)\n",strName.c_str(),flDuration);
	C_FFHintTimer *pTimer = FindTimer( strName );
	if( pTimer == NULL )
	{
		//if(m_vecTimers.size() < MAX_FF_CLIENT_TIMERS)
		//{
			//DevMsg("\tCreating new timer...");
			pTimer = new C_FFHintTimer(strName, flDuration);
			//DevMsg("0x%X\n",pTimer);
			if(pTimer)
			{
				m_vecTimers.push_back(pTimer);
			}
		//}
		return pTimer;
	}
	//DevMsg("\tA timer with the specified name already exists: 0x%X\n",pTimer);
	return pTimer;
}

C_FFHintTimer* C_FFHintTimerManager::FindTimer( string strName )
{
	//DevMsg("C_FFHintTimerManager::FindTimer(%s)\n",strName.c_str());
	C_FFHintTimer *pTimer = NULL;
	if(!m_vecTimers.empty())
	{
		//DevMsg("\tSearching...\n");
		HintTimerIterator timer = m_vecTimers.begin();
		HintTimerIterator end = m_vecTimers.end();
		while(timer != end)
		{
			pTimer = *timer;
			//DevMsg("\t%s == %s? %i\n",pTimer->GetTimerName().c_str(),strName.c_str(),(pTimer->GetTimerName() == strName));
			if(pTimer->GetTimerName() == strName)
			{
				//DevMsg("\t\tFound match! (0x%X)\n",pTimer);
				return pTimer;
			}
			timer++;
		}
	}
	//DevMsg("\tNo timers exists!\n");
	return NULL;
}

void C_FFHintTimerManager::DeleteTimer( string strName )
{
	//DevMsg( "C_FFHintTimerManager::DeleteTimer(%s) - Num active timers: %i\n",strName.c_str(), m_vecTimers.size() );
	
	if( !m_vecTimers.empty() )
	{
		//DevMsg( "\tSearching...\n" );

		bool bFound = false;
		HintTimerIterator tb = m_vecTimers.begin(), te = m_vecTimers.end(), ts = NULL;
		for( ; ( tb != te ) && !bFound; tb++ )
		{
			if( ( *( tb ) )->GetTimerName() == strName )
			{
				//DevMsg( "\tFound match! (0x%X)\n", *( tb ) );				
				ts = tb;
				bFound = true;
			}
		}

		if( bFound )
			RemoveEntry( ts );

		/*
		TimerIterator timer = m_vecTimers.begin();
		TimerIterator end = m_vecTimers.end();
		while(timer != end)
		{
			pTimer = *timer;
			DevMsg("\t%s == %s? %i\n",pTimer->GetTimerName().c_str(),strName.c_str(),(pTimer->GetTimerName() == strName));
			if(pTimer->GetTimerName() == strName)
			{
				DevMsg("\t\tFound match! (0x%X)\n",pTimer);
				DevMsg("\t\tDeleting...");
				delete pTimer;
				pTimer = NULL;
				m_vecTimers.erase( std::remove( m_vecTimers.begin(), m_vecTimers.end(), pTimer), m_vecTimers.end() );
				DevMsg("Done!\n");
				return;
			}
			timer++;
		}
		DevMsg("\tNo matches found!\n");
		*/
	}

	//DevMsg( "Number of active timers: %i\n", m_vecTimers.size() );
}

void C_FFHintTimerManager::DeleteTimer( C_FFHintTimer *pTimer )
{
	//DevMsg( "C_FFHintTimerManager::DeleteTimer(0x%X) - Num active timers: %i\n", pTimer, m_vecTimers.size() );

	// Try and delete it if the ptr isn't NULL
	// and we've got some timers we're managing
	if( pTimer && !m_vecTimers.empty() )
	{
		//DevMsg( "\tSearching...\n" );
		
		// Gotta find this particular C_FFHintTimer * in our vector
		bool bFound = false;
		HintTimerIterator tb = m_vecTimers.begin(), te = m_vecTimers.end(), ts = NULL;
		for( ; ( tb != te ) && !bFound; tb++ )
		{			
			if( *( tb ) == pTimer )
			{
				//DevMsg( "\tFound a guy to erase: 0x%X\n", *tb );
				ts = tb;
				bFound = true;
			}
		}

		if( bFound )
			RemoveEntry( ts );
	}

	//DevMsg( "Number of active timers: %i\n", m_vecTimers.size() );
}

void C_FFHintTimerManager::RemoveEntry( HintTimerIterator ti )
{
	if( ti != NULL )
	{
		// Stop animating this guy
		( *( ti ) )->ResetTimer();

		// Now erase actual timer
		//DevMsg( "\tErasing!\n" );
		m_vecTimers.erase( ti, ti + 1 );
	}
}

void C_FFHintTimerManager::DeleteAll( void )
{
	//DevMsg("C_FFHintTimerManager::DeleteAll\n");
	if(!m_vecTimers.empty())
	{
		HintTimerIterator timer = m_vecTimers.begin();
		HintTimerIterator end = m_vecTimers.end();
		C_FFHintTimer *pTimer = NULL;
		while(timer != end)
		{
			pTimer = *timer;
			delete pTimer;
			pTimer = NULL;
			timer++;
		}
		m_vecTimers.clear();
	}
	//DevMsg("\tNo timers exist!\n");
}

void C_FFHintTimerManager::SimulateTimers ( void )
{
	if(!m_vecTimers.empty())
	{
		HintTimerIterator timer = m_vecTimers.begin();
		HintTimerIterator end = m_vecTimers.end();
		C_FFHintTimer *pTimer = NULL;
		while(timer != end)
		{
			pTimer = *timer;

			// Is the timer currently running?
			if( pTimer->HasStarted() && !pTimer->IsPaused() )
			{
				if(pTimer->Interval())
				{
					if(pTimer->m_pfnHintIntervalCallback != NULL)
						(*pTimer->m_pfnHintIntervalCallback)(pTimer);
				}

				//first, check to see if the timer has expired
				if(pTimer->HasElapsed())
				{
					//call the callback function if one exists
					if(pTimer->m_pfnHintExpiredCallback != NULL)
						(*pTimer->m_pfnHintExpiredCallback)(pTimer);

					//remove the timer if desired (but show for half a second extra)
					if(pTimer->m_bRemoveWhenExpired)
					{
						if (pTimer->GetRemainingTime() < -0.5f)	// |-- Mirv: Delay removal
						{
							delete pTimer;
							pTimer = NULL;
							m_vecTimers.erase( timer );
						}
					}
					else //otherwise reset it
						pTimer->ResetTimer();
				}
			}
			timer++;
		}
	}
}

int C_FFHintTimerManager::Count( void )
{
	return m_vecTimers.size();
}