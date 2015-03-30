
// ff_timerman.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "ff_timerman.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
CFFTimerManager _timerman;

extern CRC32_t ComputeChecksum(const char* szBuffer);
extern bool CRC32_LessFunc(const CRC32_t& a, const CRC32_t& b);

/////////////////////////////////////////////////////////////////////////////
// CFFTimerCallback
/////////////////////////////////////////////////////////////////////////////
CFFTimer::CFFTimer(float flStartValue, float flIncrement)
{
	m_flStartTime = gpGlobals->curtime;
	m_flStartValue = flStartValue;
	m_flIncrement = flIncrement;
	m_flCurTime = flStartValue;
}

/////////////////////////////////////////////////////////////////////////////
CFFTimer::CFFTimer(const CFFTimer& rhs)
{
	m_flStartTime = rhs.m_flStartTime;
	m_flStartValue = rhs.m_flStartValue;
	m_flIncrement = rhs.m_flIncrement;
	m_flCurTime = rhs.m_flCurTime;
}

/////////////////////////////////////////////////////////////////////////////
bool CFFTimer::Update()
{
#ifdef FF_BETA_TEST_COMPILE
	CBaseEntity *p = NULL;
	p->Activate();
	return true;
#else

	return false;
#endif // FF_BETA_TEST_COMPILE
}

float CFFTimer::GetTime()
{
	return (m_flStartValue + (gpGlobals->curtime - m_flStartTime) * m_flIncrement);
}

float CFFTimer::GetIncrement()
{
	return m_flIncrement;
}

/////////////////////////////////////////////////////////////////////////////
// CFFTimerManager
/////////////////////////////////////////////////////////////////////////////
CFFTimerManager::CFFTimerManager()
{
	m_timers.SetLessFunc(CRC32_LessFunc);
}

/////////////////////////////////////////////////////////////////////////////
CFFTimerManager::~CFFTimerManager()
{

}

/////////////////////////////////////////////////////////////////////////////
void CFFTimerManager::AddTimer(const char* szTimerName,
						float flStartValue,
						float flTimerIncrement)
{
#ifdef FF_BETA_TEST_COMPILE
	CBaseEntity *p = NULL;
	p->Activate();
#else
	CRC32_t id = ComputeChecksum(szTimerName);

	// check if the timer of the specified name already exists
	if(m_timers.IsValidIndex(m_timers.Find(id)))
		return;

	// add a new timer to the list
	CFFTimer* pTimer = new CFFTimer(flStartValue, flTimerIncrement);

	m_timers.Insert(id, pTimer);
#endif // FF_BETA_TEST_COMPILE
}


/////////////////////////////////////////////////////////////////////////////
void CFFTimerManager::Init()
{
	Shutdown();
}

/////////////////////////////////////////////////////////////////////////////
void CFFTimerManager::RemoveTimer(const char* szTimerName)
{
	CRC32_t id = ComputeChecksum(szTimerName);

	// remove the timer from the list
	unsigned short it = m_timers.Find(id);
	if(m_timers.IsValidIndex(it))
	{
		delete m_timers[it];
		m_timers.RemoveAt(it);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CFFTimerManager::Shutdown()
{
	unsigned short i = m_timers.FirstInorder();
	while ( i != m_timers.InvalidIndex() )
	{
		delete m_timers[i];
		i = m_timers.NextInorder( i );
	}

	m_timers.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
void CFFTimerManager::Update()
{
	// update each item in the timer list
	unsigned short it = m_timers.FirstInorder();
	while(m_timers.IsValidIndex(it))
	{
		CFFTimer* pTimer = m_timers.Element(it);
		bool isComplete = pTimer->Update();

		if(isComplete)
		{
			// remove and cleanup the timer
			unsigned int itDeleteMe = it;
			it = m_timers.NextInorder(it);
			delete m_timers[itDeleteMe];
			m_timers.RemoveAt(itDeleteMe);
		}
		else
		{
			it = m_timers.NextInorder(it);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
float CFFTimerManager::GetTime(const char* szTimerName)
{
	CRC32_t id = ComputeChecksum(szTimerName);

	// remove the timer from the list
	unsigned short it = m_timers.Find(id);
	if(m_timers.IsValidIndex(it))
	{
		CFFTimer* pTimer = m_timers.Element(it);
		return pTimer->GetTime();
	}
	else 
	{
		return 0.0f;
	}
}

/////////////////////////////////////////////////////////////////////////////
float CFFTimerManager::GetIncrement(const char* szTimerName)
{
	CRC32_t id = ComputeChecksum(szTimerName);

	// remove the timer from the list
	unsigned short it = m_timers.Find(id);
	if(m_timers.IsValidIndex(it))
	{
		CFFTimer* pTimer = m_timers.Element(it);
		return pTimer->GetIncrement();
	}
	else 
	{
		return 0.0f;
	}
}

/////////////////////////////////////////////////////////////////////////////
