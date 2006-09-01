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

#include "ff_statdefs.h"
#include "ff_weapon_base.h"

#include <vector>

// Forward declarations
class CFFPlayer;

enum stattype_t {
	STAT_ADD,
	STAT_MIN,
	STAT_MAX
};
struct CFFStatDef {
	char *m_sName;
	stattype_t m_iType;
};
struct CFFActionDef {
	char *m_sName;
};
struct CFFAction {
	int actionid;
	int targetid;
	int time;
	char *param;
	Vector coords;
	char *location;
};

struct CFFPlayerStats {
	char *m_sName;
	char *m_sSteamID;
	int m_iClass;
	int m_iTeam;
	int m_iUniqueID;
	std::vector<double> m_vStats;
	std::vector<double> m_vStartTimes;
	std::vector<CFFAction> m_vActions;
};



class CFFStatsLog {
public:
	CFFStatsLog();
	~CFFStatsLog();
	int GetActionID(const char *statname);
	int GetStatID(const char *statname, stattype_t type = STAT_ADD);
	int GetPlayerID(const char *steamid, int classid, int teamnum, int uniqueid, const char *name);
	void AddStat(int playerid, int statid, double value);
	void AddAction(int playerid, int targetid, int actionid, int time, const char *param, Vector coords, const char *location);
	void StartTimer(int playerid, int statid);
	void StopTimer(int playerid, int statid, bool apply = true);
	void ResetStats();
	void Serialise(char *buffer, int buffer_size);

	const char *GetAuthString();
	const char *GetTimestampString();

private:
	// holds all of the player's stats
	std::vector<CFFPlayerStats> m_vPlayers;
	std::vector<CFFStatDef> m_vStats;
	std::vector<CFFActionDef> m_vActions;
};

/**
* Temporary for now!!! Obviously std::string would be a better alternative
*
* @author Gavin "Mirvin_Monkey" Bramhill
*/
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

// Singleton to use
extern CFFStatsLog g_StatsLog;

#endif /* FF_STATSLOG_H */