
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
// CFFScheduleCallback
/////////////////////////////////////////////////////////////////////////////
CFFScheduleCallback::CFFScheduleCallback(const luabind::adl::object& fn, float timer)
{
	m_function = fn;
	m_timeLeft = timer;
	m_timeTotal = timer;
	m_nRepeat = 1;
	m_nParams = 0;
}

/////////////////////////////////////////////////////////////////////////////
CFFScheduleCallback::CFFScheduleCallback(const luabind::adl::object& fn, float timer, int nRepeat)
{
	m_function = fn;
	m_timeLeft = timer;
	m_timeTotal = timer;
	m_nRepeat = nRepeat;
	m_nParams = 0;
}

/////////////////////////////////////////////////////////////////////////////
CFFScheduleCallback::CFFScheduleCallback(const luabind::adl::object& fn,
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
CFFScheduleCallback::CFFScheduleCallback(const luabind::adl::object& fn,
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
CFFScheduleCallback::CFFScheduleCallback(const luabind::adl::object& fn,
										 float timer,
										 int nRepeat,
										 const luabind::adl::object& param1,
										 const luabind::adl::object& param2,
										 const luabind::adl::object& param3)
{
	m_function = fn;
	m_timeLeft = timer;
	m_timeTotal = timer;
	m_nRepeat = nRepeat;
	m_nParams = 3;
	m_params[0] = param1;
	m_params[1] = param2;
	m_params[2] = param3;
}

/////////////////////////////////////////////////////////////////////////////
CFFScheduleCallback::CFFScheduleCallback(const luabind::adl::object& fn,
										 float timer,
										 int nRepeat,
										 const luabind::adl::object& param1,
										 const luabind::adl::object& param2,
										 const luabind::adl::object& param3,
										 const luabind::adl::object& param4)
{
	m_function = fn;
	m_timeLeft = timer;
	m_timeTotal = timer;
	m_nRepeat = nRepeat;
	m_nParams = 4;
	m_params[0] = param1;
	m_params[1] = param2;
	m_params[2] = param3;
	m_params[3] = param4;
}

/////////////////////////////////////////////////////////////////////////////
CFFScheduleCallback::CFFScheduleCallback(const CFFScheduleCallback& rhs)
{
	m_function = rhs.m_function;
	m_timeLeft = rhs.m_timeLeft;
	m_timeTotal = rhs.m_timeTotal;
	m_nRepeat = rhs.m_nRepeat;
	m_nParams = rhs.m_nParams;
	m_params[0] = rhs.m_params[0];
	m_params[1] = rhs.m_params[1];
	m_params[2] = rhs.m_params[2];
	m_params[3] = rhs.m_params[3];
}

/////////////////////////////////////////////////////////////////////////////
bool CFFScheduleCallback::Update()
{
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

			else if(m_nParams == 3)
				m_function(m_params[0], m_params[1], m_params[2]);

			else if(m_nParams == 4)
				m_function(m_params[0], m_params[1], m_params[2], m_params[3]);
		}
		catch(...)
		{

		}

		// repeat only so many times
		if (m_nRepeat > 0)
			--m_nRepeat;

		// schedule is done, so clean up
		if (m_nRepeat == 0)
			return true;

		// reset the timer for repeating shit
		m_timeLeft = m_timeTotal;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////
bool CFFScheduleCallback::IsComplete()
{
	if(m_timeLeft <= 0.0f)
	{
		// schedule is done
		if (m_nRepeat == 0)
			return true;
	}

	return false;
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
									 const luabind::adl::object& fn)
{
	CRC32_t id = ComputeChecksum(szScheduleName);

	// check if the schedule of the specified name already exists
	if(m_schedules.IsValidIndex(m_schedules.Find(id)))
		return;

	// add a new schedule to the list
	CFFScheduleCallback* pCallback = new CFFScheduleCallback(fn,
															timer);

	m_schedules.Insert(id, pCallback);
}

/////////////////////////////////////////////////////////////////////////////
void CFFScheduleManager::AddSchedule(const char* szScheduleName,
									 float timer,
									 const luabind::adl::object& fn,
									 int nRepeat)
{
	CRC32_t id = ComputeChecksum(szScheduleName);

	// check if the schedule of the specified name already exists
	if(m_schedules.IsValidIndex(m_schedules.Find(id)))
		return;

	// add a new schedule to the list
	CFFScheduleCallback* pCallback = new CFFScheduleCallback(fn,
														   timer,
														   nRepeat);

	m_schedules.Insert(id, pCallback);
}

/////////////////////////////////////////////////////////////////////////////
void CFFScheduleManager::AddSchedule(const char* szScheduleName,
									 float timer,
									 const luabind::adl::object& fn,
									 int nRepeat,
									 const luabind::adl::object& param)
{
	CRC32_t id = ComputeChecksum(szScheduleName);

	// check if the schedule of the specified name already exists
	if(m_schedules.IsValidIndex(m_schedules.Find(id)))
		return;

	// add a new schedule to the list
	CFFScheduleCallback* pCallback = new CFFScheduleCallback(fn,
														   timer,
														   nRepeat,
														   param);

	m_schedules.Insert(id, pCallback);
}

/////////////////////////////////////////////////////////////////////////////
void CFFScheduleManager::AddSchedule(const char* szScheduleName,
									 float timer,
									 const luabind::adl::object& fn,
									 int nRepeat,
									 const luabind::adl::object& param1,
									 const luabind::adl::object& param2)
{
	CRC32_t id = ComputeChecksum(szScheduleName);

	// check if the schedule of the specified name already exists
	if(m_schedules.IsValidIndex(m_schedules.Find(id)))
		return;

	// add a new schedule to the list
	CFFScheduleCallback* pCallback = new CFFScheduleCallback(fn,
														   timer,
														   nRepeat,
														   param1,
														   param2);

	m_schedules.Insert(id, pCallback);
}

/////////////////////////////////////////////////////////////////////////////
void CFFScheduleManager::AddSchedule(const char* szScheduleName,
									 float timer,
									 const luabind::adl::object& fn,
									 int nRepeat,
									 const luabind::adl::object& param1,
									 const luabind::adl::object& param2,
									 const luabind::adl::object& param3)
{
	CRC32_t id = ComputeChecksum(szScheduleName);

	// check if the schedule of the specified name already exists
	if(m_schedules.IsValidIndex(m_schedules.Find(id)))
		return;

	// add a new schedule to the list
	CFFScheduleCallback* pCallback = new CFFScheduleCallback(fn,
															timer,
															nRepeat,
															param1,
															param2,
															param3);

	m_schedules.Insert(id, pCallback);
}

/////////////////////////////////////////////////////////////////////////////
void CFFScheduleManager::AddSchedule(const char* szScheduleName,
									 float timer,
									 const luabind::adl::object& fn,
									 int nRepeat,
									 const luabind::adl::object& param1,
									 const luabind::adl::object& param2,
									 const luabind::adl::object& param3,
									 const luabind::adl::object& param4)
{
	CRC32_t id = ComputeChecksum(szScheduleName);

	// check if the schedule of the specified name already exists
	if(m_schedules.IsValidIndex(m_schedules.Find(id)))
		return;

	// add a new schedule to the list
	CFFScheduleCallback* pCallback = new CFFScheduleCallback(fn,
															timer,
															nRepeat,
															param1,
															param2,
															param3,
															param4);

	m_schedules.Insert(id, pCallback);
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
		CFFScheduleCallback* pCallback = m_schedules.Element(it);
		bool isComplete = pCallback->Update();

		if(isComplete)
		{
			// remove and cleanup the schedule callback
			CRC32_t id = m_schedules.Key(it);
			if (m_schedules.IsValidIndex(it))
				it = m_schedules.NextInorder(it);
			
			unsigned short itCheck = m_schedules.Find(id);
			if(m_schedules.IsValidIndex(itCheck))
			{
				CFFScheduleCallback* pCallbackCheck = m_schedules.Element(itCheck);
				if (pCallbackCheck->IsComplete())
					m_schedules.RemoveAt(itCheck);
			}
		}
		else
		{
			if (m_schedules.IsValidIndex(it))
				it = m_schedules.NextInorder(it);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
