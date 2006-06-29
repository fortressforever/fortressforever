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

// Forward declarations
class CFFPlayer;

/**
* Stats information
*
* @author Gavin "Mirvin_Monkey" Bramhill
* @version 1.0.0
*/
class CPlayerStats
{
public:
	// Set to public for now
	int		m_iCounters[STAT_MAX];
	float	m_flTimers[TIMER_MAX];
	bool	m_fTimerStates[TIMER_MAX];

	// Hold player information
	int		m_iPlayerUid;
	int		m_iPlayerClass;
	char	m_szSteamID[MAX_NETWORKID_LENGTH];	

	// [DEBUG] Keep track of refs
	static int refcount;

public:
	CPlayerStats(int playeruid, const char *steamid, int classid);
	~CPlayerStats();
};

/**
* Stats logging class
*
* @author Gavin "Mirvin_Monkey" Bramhill
* @version 1.0.0
*/
class CFFStatsLogging
{
private:
	char			m_szMapName[MAX_MAP_NAME];
	int				m_nPlayers;

	int				m_iPlayerUniqueID[MAX_PLAYERS];
	char			m_szPlayerSteamID[MAX_PLAYERS][MAX_NETWORKID_LENGTH];

	CPlayerStats	*m_pPlayerStats[MAX_PLAYERS * 10];
	CPlayerStats	**m_pCurrentPlayers[MAX_PLAYERS];

public:
	CFFStatsLogging();
	~CFFStatsLogging();

	void CleanUp();

	void SetMap(const char *);

	void RegisterPlayerID(int playerindex, int playeruid, const char *steamid);
	void SetClass(int playerindex, int classid);

	void AddToCount(int playerindex, StatisticType stat, int i = 1);
	void AddToCount(CFFPlayer *pPlayer, StatisticType stat, int i = 1);

	void SetTimer(int playerindex, TimerType timer, bool on);

	void Serialise(char *buffer, int buffer_size);

	const char *GetAuthString();
	const char *GetTimestampString();
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
extern CFFStatsLogging g_StatsLog;

#endif /* FF_STATSLOG_H */