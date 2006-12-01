
// ff_scheduleman.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "ff_scheduleman.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
CFFScheduleManager _scheduleman;

/////////////////////////////////////////////////////////////////////////////
// computes the checksum of a given string
CRC32_t ComputeChecksum(const char* szBuffer)
{
	CRC32_t checksum;
	CRC32_Init(&checksum);
	CRC32_ProcessBuffer(&checksum, szBuffer, strlen(szBuffer));
	CRC32_Final(&checksum);
	return checksum;
}

/////////////////////////////////////////////////////////////////////////////
bool CRC32_LessFunc(const CRC32_t& a, const CRC32_t& b)
{
	return a < b;
}

/////////////////////////////////////////////////////////////////////////////
// CFFScheduleCallack
/////////////////////////////////////////////////////////////////////////////
CFFScheduleCallack::CFFScheduleCallack(const luabind::adl::object& fn, float timer, int nRepeat)
{
	m_function = fn;
	m_timeLeft = timer;
	m_timeTotal = timer;
	m_nRepeat = nRepeat;
	m_nParams = 0;
}

/////////////////////////////////////////////////////////////////////////////
CFFScheduleCallack::CFFScheduleCallack(const luabind::adl::object& fn,
									   float timer,
									   int nRepeat,
									   const luabind::adl::object& param)
{
	m_function = fn;
	m_timeLeft = timer;
	m_timeTotal = timer;
	m_nRepeat = nRepeat;
	m_nParams = 1;
	m_params[0] = param;
}

/////////////////////////////////////////////////////////////////////////////
CFFScheduleCallack::CFFScheduleCallack(const luabind::adl::object& fn,
									   float timer,
									   int nRepeat,
									   const luabind::adl::object& param1,
									   const luabind::adl::object& param2)
{
	m_function = fn;
	m_timeLeft = timer;
	m_timeTotal = timer;
	m_nRepeat = nRepeat;
	m_nParams = 2;
	m_params[0] = param1;
	m_params[1] = param2;
}

/////////////////////////////////////////////////////////////////////////////
CFFScheduleCallack::CFFScheduleCallack(const CFFScheduleCallack& rhs)
{
	m_function = rhs.m_function;
	m_timeLeft = rhs.m_timeLeft;
	m_timeTotal = rhs.m_timeTotal;
	m_nRepeat = rhs.m_nRepeat;
	m_nParams = rhs.m_nParams;
	m_params[0] = m_params[0];
	m_params[1] = m_params[1];
}

/////////////////////////////////////////////////////////////////////////////
bool CFFScheduleCallack::Update()
{
#ifdef FF_BETA_TEST_COMPILE
	CBaseEntity *p = NULL;
	p->Activate();
	return FF_BASTARD_HACKERS;
#else
	m_timeLeft -= gpGlobals->frametime;

	if(m_timeLeft <= 0.0f)
	{
		// call the lua function
		try
		{
			if(m_nParams == 0)
				m_function();

			else if(m_nParams == 1)
				m_function(m_params[0]);

			else if(m_nParams == 2)
				m_function(m_params[0], m_params[1]);
		}
		catch(...)
		{

		}

		// repeat value -1 indicates infinite
		if(m_nRepeat == -1)
			return true;

		--m_nRepeat;

		// check if the schedule should be cleaned up
		if(m_nRepeat < 1)
			return true;

		// reset the timer
		m_timeLeft = m_timeTotal;
	}

	return false;
#endif // FF_BETA_TEST_COMPILE
}

/////////////////////////////////////////////////////////////////////////////
// CFFScheduleManager
/////////////////////////////////////////////////////////////////////////////
CFFScheduleManager::CFFScheduleManager()
{
	m_schedules.SetLessFunc(CRC32_LessFunc);
}

/////////////////////////////////////////////////////////////////////////////
CFFScheduleManager::~CFFScheduleManager()
{

}

/////////////////////////////////////////////////////////////////////////////
void CFFScheduleManager::AddSchedule(const char* szScheduleName,
									 float timer,
									 const luabind::adl::object& fn,
									 int nRepeat)
{
#ifdef FF_BETA_TEST_COMPILE
	CBaseEntity *p = NULL;
	p->Activate();
#else
	CRC32_t id = ComputeChecksum(szScheduleName);

	// check if the schedule of the specified name already exists
	if(m_schedules.IsValidIndex(m_schedules.Find(id)))
		return;

	// add a new schedule to the list
	CFFScheduleCallack* pCallback = new CFFScheduleCallack(fn,
														   timer,
														   nRepeat);

	m_schedules.Insert(id, pCallback);
#endif // FF_BETA_TEST_COMPILE
}

/////////////////////////////////////////////////////////////////////////////
void CFFScheduleManager::AddSchedule(const char* szScheduleName,
									 float timer,
									 const luabind::adl::object& fn,
									 int nRepeat,
									 const luabind::adl::object& param)
{
#ifdef FF_BETA_TEST_COMPILE
	CBaseEntity *p = NULL;
	p->Activate();
#else
	CRC32_t id = ComputeChecksum(szScheduleName);

	// check if the schedule of the specified name already exists
	if(m_schedules.IsValidIndex(m_schedules.Find(id)))
		return;

	// add a new schedule to the list
	CFFScheduleCallack* pCallback = new CFFScheduleCallack(fn,
														   timer,
														   nRepeat,
														   param);

	m_schedules.Insert(id, pCallback);
#endif // FF_BETA_TEST_COMPILE
}

/////////////////////////////////////////////////////////////////////////////
void CFFScheduleManager::AddSchedule(const char* szScheduleName,
									 float timer,
									 const luabind::adl::object& fn,
									 int nRepeat,
									 const luabind::adl::object& param1,
									 const luabind::adl::object& param2)
{
#ifdef FF_BETA_TEST_COMPILE
	CBaseEntity *p = NULL;
	p->Activate();
#else
	CRC32_t id = ComputeChecksum(szScheduleName);

	// check if the schedule of the specified name already exists
	if(m_schedules.IsValidIndex(m_schedules.Find(id)))
		return;

	// add a new schedule to the list
	CFFScheduleCallack* pCallback = new CFFScheduleCallack(fn,
														   timer,
														   nRepeat,
														   param1,
														   param2);

	m_schedules.Insert(id, pCallback);
#endif // FF_BETA_TEST_COMPILE
}

/////////////////////////////////////////////////////////////////////////////
void CFFScheduleManager::Init()
{
	Shutdown();
}

/////////////////////////////////////////////////////////////////////////////
void CFFScheduleManager::RemoveSchedule(const char* szScheduleName)
{
	CRC32_t id = ComputeChecksum(szScheduleName);

	// remove the schedule from the list
	unsigned short it = m_schedules.Find(id);
	if(m_schedules.IsValidIndex(it))
		m_schedules.RemoveAt(it);
}

/////////////////////////////////////////////////////////////////////////////
void CFFScheduleManager::Shutdown()
{
	m_schedules.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
void CFFScheduleManager::Update()
{
	// update each item in the schedule list
	unsigned short it = m_schedules.FirstInorder();
	while(m_schedules.IsValidIndex(it))
	{
		CFFScheduleCallack* pCallback = m_schedules.Element(it);
		bool isComplete = pCallback->Update();

		if(isComplete)
		{
			// remove and cleanup the schedule callback
			unsigned int itDeleteMe = it;
			it = m_schedules.NextInorder(it);
			m_schedules.RemoveAt(itDeleteMe);
		}
		else
		{
			it = m_schedules.NextInorder(it);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
