
// ff_scheduleman.h

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef UTLMAP_H
	#include "utlmap.h"
#endif
#ifndef CHECKSUM_CRC_H
	#include "checksum_crc.h"
#endif
#ifndef LUABIND_OBJECT_050419_HPP
	#include "luabind/object.hpp"
#endif

/////////////////////////////////////////////////////////////////////////////
class CFFScheduleCallack
{
public:
	// 'structors
	CFFScheduleCallack(const luabind::adl::object& fn, float timer, int nRepeat);

	CFFScheduleCallack(const luabind::adl::object& fn,
					   float timer,
					   int nRepeat,
					   const luabind::adl::object& param);

	CFFScheduleCallack(const luabind::adl::object& fn,
					   float timer,
					   int nRepeat,
					   const luabind::adl::object& param1,
					   const luabind::adl::object& param2);

	~CFFScheduleCallack() {}

public:
	// updates. call only once per frame. returns true if the schedule is
	// complete and should be deleted; otherwise returns false
	bool Update();

private:
	// private data
	luabind::adl::object m_function;
	float	m_timeLeft;					// time until the lua function should be called
	float	m_timeTotal;				// total time for a complete cycle
	int		m_nRepeat;					// number of times to cycle (-1 is infinite)
	int		m_nParams;					// number of params to pass to the function
	luabind::adl::object m_params[2];	// params to pass to function
};

/////////////////////////////////////////////////////////////////////////////
class CFFScheduleManager
{
public:
	// 'structors
	CFFScheduleManager();
	~CFFScheduleManager();

public:
	void Init();
	void Shutdown();
	void Update();

public:
	// adds a schedule
	void AddSchedule(const char* szScheduleName,
					 float timer,
					 const luabind::adl::object& fn,
					 int nRepeat);

	void AddSchedule(const char* szScheduleName,
					 float timer,
					 const luabind::adl::object& fn,
					 int nRepeat,
					 const luabind::adl::object& param);

	void AddSchedule(const char* szScheduleName,
					 float timer,
					 const luabind::adl::object& fn,
					 int nRepeat,
					 const luabind::adl::object& param1,
					 const luabind::adl::object& param2);

	// removes a schedule
	void RemoveSchedule(const char* szScheduleName);

private:
	// list of schedules. key is the checksum of an identifying name; it
	// isnt necessarily the name of the lua function to call
	CUtlMap<CRC32_t, CFFScheduleCallack*>	m_schedules;
};

/////////////////////////////////////////////////////////////////////////////
extern CFFScheduleManager _scheduleman;

/////////////////////////////////////////////////////////////////////////////
