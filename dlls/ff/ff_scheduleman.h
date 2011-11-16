
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
class CFFScheduleCallback
{
public:
	// 'structors
	CFFScheduleCallback(const luabind::adl::object& fn,
						float timer);

	CFFScheduleCallback(const luabind::adl::object& fn,
						float timer,
						int nRepeat);

	CFFScheduleCallback(const luabind::adl::object& fn,
						float timer,
						int nRepeat,
						const luabind::adl::object& param);

	CFFScheduleCallback(const luabind::adl::object& fn,
						float timer,
						int nRepeat,
						const luabind::adl::object& param1,
						const luabind::adl::object& param2);

	CFFScheduleCallback(const luabind::adl::object& fn,
						float timer,
						int nRepeat,
						const luabind::adl::object& param1,
						const luabind::adl::object& param2,
						const luabind::adl::object& param3);

	CFFScheduleCallback(const luabind::adl::object& fn,
						float timer,
						int nRepeat,
						const luabind::adl::object& param1,
						const luabind::adl::object& param2,
						const luabind::adl::object& param3,
						const luabind::adl::object& param4);

	CFFScheduleCallback(const CFFScheduleCallback& rhs);

	~CFFScheduleCallback() {}

public:
	// updates. call only once per frame. returns true if the schedule is
	// complete and should be deleted; otherwise returns false
	bool Update();
	
	// returns true if this schedule is completely done and should be deleted
	bool IsComplete();

private:
	// private data
	luabind::adl::object m_function;	// handle to the lua function to call
	float	m_timeLeft;					// time until the lua function should be called
	float	m_timeTotal;				// total time for a complete cycle
	int		m_nRepeat;					// number of times to cycle (-1 is infinite)
	int		m_nParams;					// number of params to pass to the function
	luabind::adl::object m_params[4];	// params to pass to function
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
					const luabind::adl::object& fn);

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

	void AddSchedule(const char* szScheduleName,
					float timer,
					const luabind::adl::object& fn,
					int nRepeat,
					const luabind::adl::object& param1,
					const luabind::adl::object& param2,
					const luabind::adl::object& param3);

	void AddSchedule(const char* szScheduleName,
					float timer,
					const luabind::adl::object& fn,
					int nRepeat,
					const luabind::adl::object& param1,
					const luabind::adl::object& param2,
					const luabind::adl::object& param3,
					const luabind::adl::object& param4);

	// removes a schedule
	void RemoveSchedule(const char* szScheduleName);

private:
	// list of schedules. key is the checksum of an identifying name; it
	// isnt necessarily the name of the lua function to call
	CUtlMap<CRC32_t, CFFScheduleCallback*>	m_schedules;
};

/////////////////////////////////////////////////////////////////////////////
extern CFFScheduleManager _scheduleman;

/////////////////////////////////////////////////////////////////////////////
