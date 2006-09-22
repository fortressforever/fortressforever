/********************************************************************
	created:	2005/12/24
	created:	24:12:2005   1:18
	filename: 	f:\cvs\code\dlls\ff_statslog.h
	file path:	f:\cvs\code\dlls
	file base:	ff_statslog
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#ifndef FF_STATSLOG_H
#define FF_STATSLOG_H

#ifdef _WIN32
#pragma once
#endif

#include "ff_statdefs.h"
#include "ff_weapon_base.h"
#include "ff_string.h"

// Forward declarations
class CFFPlayer;

enum stattype_t 
{
	STAT_INVALID = 0,
	STAT_ADD,
	STAT_MIN,
	STAT_MAX
};

class CFFStatDef 
{
public:
	// Default constructor
	CFFStatDef( void )
	{
		m_iType = STAT_INVALID;
	}

	// Overloaded constructor
	CFFStatDef( const char *pszName, stattype_t iType )
	{
		m_sName = pszName;
		m_iType = iType;
	}

public:
	CFFString m_sName;
	stattype_t m_iType;
};

class CFFActionDef 
{
public:
	// Overloaded constructor
	CFFActionDef( const char *pszName )
	{
		m_sName = pszName;
	}

public:
	CFFString m_sName;
};

class CFFAction 
{
public:
	// Default constructor
	CFFAction( void )
	{
		actionid = -1;
		targetid = -1;
		time = -1;
		coords.Init();
	}

	// Overloaded constructor
	CFFAction( int iActionId, int iTargetId, int iTime, const char *pszParam, const Vector& vecCoords, const char *pszLocation )
	{
		actionid = iActionId;
		targetid = iTargetId;
		time = iTime;
		param = pszParam;
		coords = vecCoords;
		location = pszLocation;
	}

public:
	int actionid;
	int targetid;
	int time;
	CFFString param;
	Vector coords;
	CFFString location;
};

// STL stuff has been moved out of here because Valve and STL don't really mix very well!
abstract_class IStatsLog
{
public:
	virtual int GetActionID(const char *statname) = 0;
	virtual int GetStatID(const char *statname, stattype_t type = STAT_ADD) = 0;
	virtual int GetPlayerID(const char *steamid, int classid, int teamnum, int uniqueid, const char *name) = 0;
	virtual void AddStat(int playerid, int statid, double value) = 0;
	virtual void AddAction(int playerid, int targetid, int actionid, int time, const char *param, Vector coords, const char *location) = 0;
	//virtual void StartTimer(int playerid, int statid) = 0;
	//virtual void StopTimer(int playerid, int statid, bool apply = true) = 0;
	virtual void ResetStats() = 0;
	virtual void Serialise(char *buffer, int buffer_size) = 0;
};

// Singleton to use
extern IStatsLog *g_StatsLog;

//
// VVV This stuff ought to be moved out here as well!!
//
class CQuickBuffer
{
	int m_iLen;
	char *m_Buf;
	CQuickBuffer();

public:
	CQuickBuffer(char *buf, int len) 
	{
		m_iLen = len;
		m_Buf = buf;

		m_Buf[0] = 0;
	}

	void Add(const char *fmt, ...) 
	{
		va_list ap;
		char buf[2024];

		va_start(ap, fmt);
		_vsnprintf(buf, sizeof(buf), fmt, ap);
		va_end(ap);

		strncat(m_Buf, buf, m_iLen - strlen(m_Buf));
	}
};

// Function to send stats
void SendStats();

#endif /* FF_STATSLOG_H */
