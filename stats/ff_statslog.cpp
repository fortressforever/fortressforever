/********************************************************************
	created:	2005/12/24
	created:	24:12:2005   1:41
	filename: 	f:\cvs\code\logging\ff_statslog.cpp
	file path:	f:\cvs\code\logging
	file base:	ff_statslog
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "ff_statslog.h"
#include "ff_socks.h"

#include <list>
#include <algorithm>
#include <string>

#include "tier0/memdbgon.h"

int CPlayerStats::refcount = 0;

// Singleton for this.
CFFStatsLogging g_StatsLog;

/**
* Constructor
*/
CFFStatsLogging::CFFStatsLogging() 
{
	m_nPlayers = 0;

	CleanUp();
}

/**
* Destructor
*/
CFFStatsLogging::~CFFStatsLogging() 
{
	CleanUp();
}

/**
* Cleans up all the data stored in this session
*/
void CFFStatsLogging::CleanUp() 
{
	// Destroy all instances of class stats
	for (int i = 0; i < m_nPlayers; i++) 
	{
		if (m_pPlayerStats[i]) 
		{
			delete m_pPlayerStats[i];
			m_pPlayerStats[i] = NULL;
		}
	}

	// Remove all current player things
	for (i = 0; i < MAX_PLAYERS; i++) 
	{
		m_pCurrentPlayers[i] = NULL;
	}

	m_nPlayers = 0;
}

/**
* Sets the currently played map, this should only be done once per map obviously
*
* @param mapname Name of map
*/
void CFFStatsLogging::SetMap(const char *mapname) 
{
	Q_strncpy(m_szMapName, mapname, MAX_MAP_NAME - 1);
}

/**
* Register a player index with a unique id and steam id
*
* @param playerindex Player entity index
* @param playeruid Player unique identifier
* @param steamid Player SteamID
*/
void CFFStatsLogging::RegisterPlayerID(int playerindex, int playeruid, const char *steamid) 
{
	// Remember this playerindex's unique id & steam id for when we create class instances
	m_iPlayerUniqueID[playerindex] = playeruid;
	Q_strncpy(m_szPlayerSteamID[playerindex], steamid, MAX_NETWORKID_LENGTH - 1);
}

/**
* Set the class of this player
* Finds previous allocation of this player's stats for this class
* or creates a new one if needed.
*
* @param playerindex Player entity index
* @param classid Class of player
*/
void CFFStatsLogging::SetClass(int playerindex, int classid) 
{
	// Get the player's uniqueid
	int playeruid = m_iPlayerUniqueID[playerindex];

	// If this player exists cancel the timers
	if (m_pCurrentPlayers[playerindex]) 
	{
		// Turn off all the timers
		for (int i = 0; i < TIMER_MAX; i++) 
			SetTimer(playerindex, (TimerType) i, false);
	}

	// Search through the current set of stats
	for (int i = 0; i < m_nPlayers; i++) 
	{
		// This player/class combination already exists
		if (m_pPlayerStats[i] && m_pPlayerStats[i]->m_iPlayerUid == playeruid && m_pPlayerStats[i]->m_iPlayerClass == classid) 
		{
			// Point to this from now on
			m_pCurrentPlayers[playerindex] = &m_pPlayerStats[i];
			return;
		}
	}

	// This player class combination has not been picked yet
	m_pPlayerStats[m_nPlayers] = new CPlayerStats(playeruid, m_szPlayerSteamID[playerindex], classid);
	m_pCurrentPlayers[playerindex] = &m_pPlayerStats[m_nPlayers];

	m_nPlayers++;
}

/**
* Add a count to a current player
*
* @param playerindex Current player
* @param stat Statistic type
* @param i Increment amount
*/
void CFFStatsLogging::AddToCount(int playerindex, StatisticType stat, int i /* = 1 */) 
{
	if (m_pCurrentPlayers[playerindex]) 
		 (*m_pCurrentPlayers[playerindex])->m_iCounters[stat] += i;
}

/**
* Turn a timer on or off
*
* @param playerindex Current player
* @param timer Timer type
* @param on Timer is on or not
*/
void CFFStatsLogging::SetTimer(int playerindex, TimerType timer, bool on) 
{
	// Make sure the timer status is changing
	if (m_pCurrentPlayers[playerindex] && (*m_pCurrentPlayers[playerindex])->m_fTimerStates[timer] != on) 
	{
		 (*m_pCurrentPlayers[playerindex])->m_flTimers[timer] = gpGlobals->curtime - (*m_pCurrentPlayers[playerindex])->m_flTimers[timer];

		 (*m_pCurrentPlayers[playerindex])->m_fTimerStates[timer] = on;
	}
}

/**
* Retreive the authorisation string that verifies stats sent
*/
const char *CFFStatsLogging::GetAuthString() 
{
	return "ABC";
}

/**
* Retreive a timestamp
*/
const char *CFFStatsLogging::GetTimestampString() 
{
	return "22-Jan-2006 00:03 UTC";
}

/**
* Serialise the stored data for sending
*/
void CFFStatsLogging::Serialise(char *buffer, int buffer_size) 
{
	CQuickBuffer buf(buffer, buffer_size);

	buf.Add("login ff-test\n");
	buf.Add("auth %s\n", GetAuthString());
	buf.Add("date %s\n", GetTimestampString());
	buf.Add("duration %d\n", 1800);
	buf.Add("map %s\n", m_szMapName);
	buf.Add("bluescore %d\n", 0);
	buf.Add("redscore %d\n", 0);
	buf.Add("yellowscore %d\n", 0);
	buf.Add("greenscore %d\n", 0);
	buf.Add("players\n");

	std::list<int> i_Done;
	std::list<int>::iterator i_Find;

	for (int i = 0; i < m_nPlayers; i++) 
	{
		if (!m_pPlayerStats[i]) 
			continue;

		i_Find = find(i_Done.begin(), i_Done.end(), i); // Search the list.
		
		// Make sure we haven't already listed this one
		if (i_Find == i_Done.end()) 
		{
			buf.Add("%s %s %d\n", m_pPlayerStats[i]->m_szSteamID, "RANDOMNAME", m_pPlayerStats[i]->m_iPlayerUid);
			i_Done.push_back(i);
		}
	}

	buf.Add("stats\n");

	for (int i = 0; i < m_nPlayers; i++) 
	{
		if (!m_pPlayerStats[i]) 
			continue;

		for (int j = 0; j < STAT_MAX; j++) 
		{
			// Don't send any unchanged values
			if (m_pPlayerStats[i]->m_iCounters[j] == 0) 
				continue;

			buf.Add("%d %d %d %d\n", m_pPlayerStats[i]->m_iPlayerUid, m_pPlayerStats[i]->m_iPlayerClass, j, m_pPlayerStats[i]->m_iCounters[j]);
		}
	}
}

/**
* Constructor
*
* @param playeruid Player unique identifier
* @param steamid Player steam id
* @param classid Player class
*/
CPlayerStats::CPlayerStats(int playeruid, const char *steamid, int classid) 
{
	m_iPlayerUid = playeruid;
	m_iPlayerClass = classid;

	Q_strncpy(m_szSteamID, steamid, MAX_NETWORKID_LENGTH - 1);

	memset(&m_iCounters, 0, sizeof(m_iCounters));
	memset(&m_flTimers, 0, sizeof(m_flTimers));

	for (int i = 0; i < TIMER_MAX; i++) 
		m_fTimerStates[i] = false;

	CPlayerStats::refcount++;
}

/**
* Destructor
*/
CPlayerStats::~CPlayerStats() 
{
	CPlayerStats::refcount--;
}

/**
* Send the stats to the remote server
*/
void SendStats() 
{
	char buf[131072];

	DevWarning("[STATS] Sending stats...\n");

	g_StatsLog.Serialise(buf, sizeof(buf));

	DevMsg(buf);

	return;

	Socks sock;

	// Open up a socket
	if (!sock.Open(/*SOCK_STREAM */ 1, 0)) 
	{
		DevWarning("[STATS] Could not open socket\n");
		return;
	}

	// Connect to remote host
	if (!sock.Connect("buzz", 27999)) 
	{
		DevWarning("[STATS] Could not connect to remote host\n");
		return;
	}

	// Send data
	if (!sock.Send(buf)) 
	{
		DevWarning("[STATS] Could not send data to remote host\n");
		sock.Close();
		return;
	}

	char response[128];
	int a;

	// Send data
	if ((a = sock.Recv(response, 127)) == 0) 
	{
		DevWarning("[STATS] Did not get response from stats server\n");
		sock.Close();
		return;
	}

	response[a] = '\0';

	DevMsg("[STATS] Successfully sent stats data(Response: %s) \n", response);

	// Close socket
	sock.Close();
}