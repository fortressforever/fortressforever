/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file c_ff_timers.cpp
/// @author Shawn Smith (L0ki)
/// @date May 13, 2005
/// @brief implementation of the client side timer class
/// 
/// implements the client side timer class
/// 
/// Revisions
/// ---------
/// May 13, 2005	L0ki: Initial Creation

#include "cbase.h"
#include "c_ff_timers.h"
#include <algorithm>

using namespace vgui;

C_FFTimerManager g_FFTimers;

C_FFTimer::C_FFTimer( std::string strName, const float flDuration )
: CHudElement( strName.c_str() ), vgui::Panel(NULL, strName.c_str()), CFFTimerBase( strName.c_str(), flDuration )
{
	SetParent( g_pClientMode->GetViewport() );
	SetHiddenBits( 0 );

	int iScreenWidth, iScreenHeight;
	GetHudSize( iScreenWidth, iScreenHeight );

	int iWidth = iScreenWidth / 4 + 2, iHeight = 22;
	int nTimers = g_FFTimers.Count();

	SetPos((iScreenWidth / 2) - (iWidth / 2), iScreenHeight - ( ( nTimers + 1 ) * iHeight) );
	SetWide( iWidth );
	SetTall( iHeight );

	m_pProgressBar = new vgui::ProgressBar(this, "FFTimerProgress");
	m_pProgressBar->SetPos( 1, 1 );
	m_pProgressBar->SetSize( iWidth - 2, iHeight - 2 );
	m_pProgressBar->SetVisible( true );
}

C_FFTimer::~C_FFTimer()
{
	if(m_pProgressBar)
		m_pProgressBar->DeletePanel();
}

void C_FFTimer::Paint( void )
{
	return;

	float progress = clamp(GetElapsedTime() / m_flDuration, 0.0f, 1.0f);	// |-- Mirv
	m_pProgressBar->SetProgress( progress );
}
void C_FFTimer::ApplySchemeSettings( IScheme *pScheme )
{
	m_pProgressBar->SetBorder( pScheme->GetBorder( "ComboBoxBorder" ) );
	m_pProgressBar->SetBgColor( Color( 0, 0, 0, 128 ) );
}

void C_FFTimer::StartTimer( void )
{
	CFFTimerBase::StartTimer();
	//SetVisible( true );
	SetVisible(false);
}

void C_FFTimer::ResetTimer( void )
{
	CFFTimerBase::ResetTimer();
	SetVisible( false );
}

//////////////////////////////////////////////////////////////////////////
// Client side Timer Manager class
//////////////////////////////////////////////////////////////////////////
C_FFTimerManager::C_FFTimerManager()
{
}

C_FFTimerManager::~C_FFTimerManager()
{
	//DevMsg("C_FFTimerManager::~C_FFTimerManager\n");
	DeleteAll();
}

C_FFTimer* C_FFTimerManager::Create( std::string strName, float flDuration )
{
	//DevMsg("C_FFTimerManager::Create(%s, %f)\n",strName.c_str(),flDuration);
	C_FFTimer *pTimer = FindTimer( strName );
	if( pTimer == NULL )
	{
		if(m_vecTimers.size() < MAX_FF_CLIENT_TIMERS)
		{
			//DevMsg("\tCreating new timer...");
			pTimer = new C_FFTimer(strName, flDuration);
			//DevMsg("0x%X\n",pTimer);
			if(pTimer)
			{
				m_vecTimers.push_back(pTimer);
			}
		}
		return pTimer;
	}
	//DevMsg("\tA timer with the specified name already exists: 0x%X\n",pTimer);
	return pTimer;
}

C_FFTimer* C_FFTimerManager::FindTimer( std::string strName )
{
	//DevMsg("C_FFTimerManager::FindTimer(%s)\n",strName.c_str());
	C_FFTimer *pTimer = NULL;
	if(!m_vecTimers.empty())
	{
		//DevMsg("\tSearching...\n");
		TimerIterator timer = m_vecTimers.begin();
		TimerIterator end = m_vecTimers.end();
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

void C_FFTimerManager::DeleteTimer( std::string strName )
{
	//DevMsg( "C_FFTimerManager::DeleteTimer(%s) - Num active timers: %i\n",strName.c_str(), m_vecTimers.size() );
	
	if( !m_vecTimers.empty() )
	{
		//DevMsg( "\tSearching...\n" );

		// Edited to comply with VS 2005's stricter iterator stuff -> Defrag
		// bool bFound = false;

		TimerIterator tb = m_vecTimers.begin(), te = m_vecTimers.end(); //, ts = NULL;
		for( ; ( tb != te ) /*&& !bFound*/; tb++ )
		{
			if( ( *( tb ) )->GetTimerName() == strName )
			{
				//DevMsg( "\tFound match! (0x%X)\n", *( tb ) );				
				
				TimerIterator iterFound = tb;
				RemoveEntry( iterFound );
				
				return;
				//bFound = true;
			}
		}

		//if( bFound )
		//	RemoveEntry( ts );

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

void C_FFTimerManager::DeleteTimer( C_FFTimer *pTimer )
{
	//DevMsg( "C_FFTimerManager::DeleteTimer(0x%X) - Num active timers: %i\n", pTimer, m_vecTimers.size() );

	// Try and delete it if the ptr isn't NULL
	// and we've got some timers we're managing

	// edited to deal with VS 2005's iterator strictness stuff -> Defrag

	if( pTimer && !m_vecTimers.empty() )
	{
		//DevMsg( "\tSearching...\n" );
		
		// Gotta find this particular C_FFTimer * in our vector
		// bool bFound = false;
		TimerIterator tb = m_vecTimers.begin(), te = m_vecTimers.end(); //, ts = NULL;
		for( ; ( tb != te ) /*&& !bFound */ ; tb++ )
		{			
			if( *( tb ) == pTimer )
			{
				//DevMsg( "\tFound a guy to erase: 0x%X\n", *tb );
				TimerIterator iterFound = tb;
				RemoveEntry( iterFound );
				return;
				//bFound = true;
			}
		}

		//if( bFound )
		//	RemoveEntry( ts );
	}

	//DevMsg( "Number of active timers: %i\n", m_vecTimers.size() );
}

void C_FFTimerManager::RemoveEntry( TimerIterator ti )
{
	// set iterator to equal .end() as the default value
	if( ti != m_vecTimers.end() )
	{
		// Stop animating this guy
		( *( ti ) )->ResetTimer();

		// Now erase actual timer
		//DevMsg( "\tErasing!\n" );
		m_vecTimers.erase( ti, ti + 1 );
	}
}

void C_FFTimerManager::DeleteAll( void )
{
	//DevMsg("C_FFTimerManager::DeleteAll\n");
	if(!m_vecTimers.empty())
	{
		TimerIterator timer = m_vecTimers.begin();
		TimerIterator end = m_vecTimers.end();
		C_FFTimer *pTimer = NULL;
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

void C_FFTimerManager::SimulateTimers ( void )
{
	if(!m_vecTimers.empty())
	{
		TimerIterator timer = m_vecTimers.begin();
		TimerIterator end = m_vecTimers.end();
		C_FFTimer *pTimer = NULL;
		while(timer != end)
		{
			pTimer = *timer;

			//has the timer even started?
			if(pTimer->HasStarted())
			{
				if(pTimer->Interval())
				{
					if(pTimer->m_pfnIntervalCallback != NULL)
						(*pTimer->m_pfnIntervalCallback)(pTimer);
				}

				//first, check to see if the timer has expired
				if(pTimer->HasElapsed())
				{
					//call the callback function if one exists
					if(pTimer->m_pfnExpiredCallback != NULL)
						(*pTimer->m_pfnExpiredCallback)(pTimer);

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

int C_FFTimerManager::Count( void )
{
	return m_vecTimers.size();
}