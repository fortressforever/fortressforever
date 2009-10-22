
// ff_timerman.h

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
class CFFTimer
{
public:
	// 'structors
	CFFTimer(float flStartValue,
					float flTimerIncrement);

	CFFTimer(const CFFTimer& rhs);

	~CFFTimer() {}

public:
	// updates. call only once per frame. returns true if the timer is
	// complete and should be deleted; otherwise returns false
	bool Update();
	float GetTime();
	float GetIncrement();

private:
	// private data
	float	m_flStartTime;			// time until the lua function should be called
	float	m_flStartValue;			// time until the lua function should be called
	float	m_flIncrement;			// total time for a complete cycle
	float	m_flCurTime;			// time until the lua function should be called

};

/////////////////////////////////////////////////////////////////////////////
class CFFTimerManager
{
public:
	// 'structors
	CFFTimerManager();
	~CFFTimerManager();

public:
	void Init();
	void Shutdown();
	void Update();

public:
	// adds a timer
	void AddTimer(const char* szTimerName,
					float flStartValue,
					float flTimerIncrement);

	// removes a timer
	void RemoveTimer(const char* szTimerName);

	// returns time of a timer
	float GetTime(const char* szTimerName);
	float GetIncrement(const char* szTimerName);

private:
	// list of timerss. key is the checksum of an identifying name; it
	// isnt necessarily the name of the lua function to call
	CUtlMap<CRC32_t, CFFTimer*>	m_timers;
};

/////////////////////////////////////////////////////////////////////////////
extern CFFTimerManager _timerman;

/////////////////////////////////////////////////////////////////////////////
