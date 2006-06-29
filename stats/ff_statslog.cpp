/********************************************************************
	created:	2005/12/24
	created:	24:12:2005   1:41
	filename: 	f:\cvs\code\logging\ff_statslog.cpp
	file path:	f:\cvs\code\logging
	file base:	ff_statslog
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Accumulate player class stats throughout a map.
				This has been designed to use as little lookup time as
				possible since it will be called a lot.

				When this is received by the stats server it should work
				out the average skill of all the people on the server and
				use that to weight how many FP are given.
*********************************************************************/

#include "cbase.h"
#include "ff_statslog.h"
#include "ff_socks.h"
#include "ff_weapon_base.h"
#include "ff_player.h"

#include <list>
#include <algorithm>
#include <string>

#include "tier0/memdbgon.h"

int CPlayerStats::refcount = 0;

// Singleton for this.
CFFStatsLogging g_StatsLog;

// Strings that the PHP recognise the stats by
const char *g_pszStatStrings[] =
{
	"kills",			// STAT_KILLS
	"teamkills",		// STAT_TEAMKILLS
	"deaths",			// STAT_DEATHS
	"roundwins",		// STAT_ROUNDWINS
	"rounddraws",		// STAT_ROUNDDRAWS
	"roundlosses",		// STAT_ROUNDLOSSES
	"score",			// STAT_SCORE
	"teamfor",			// STAT_TEAMFOR
	"teamagainst",		// STAT_TEAMAGAINST
	"heals",			// STAT_HEALS
	"criticalheals",	// STAT_CRITICALHEALS
	"hphealed",			// STAT_HPHEALED
	"cures",			// STAT_CURES
	"infections",		// STAT_INFECTIONS
	"infectionspreads", // STAT_INFECTIONSPREADS
	"infectionkills",	// STAT_INFECTIONKILLS
	"concjumps",		// STAT_CONCJUMPS
	"concdistance",		// STAT_CONCDISTANCE
	"hangtime",			// STAT_HANGTIME
};

// More strings that the php recognises stats by
const char *g_pszTimerStrings[] =
{
	"played",			// STAT_KILLS
};

// FF weapon names
extern const char *s_WeaponAliasInfo[];

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
* @param player Current player
* @param stat Statistic type
* @param i Increment amount
*/
void CFFStatsLogging::AddToCount(CFFPlayer *pPlayer, StatisticType stat, int i /* = 1 */) 
{
	AddToCount(pPlayer->entindex(), stat, i);
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
* Add a count to weapons
*
* @param playerindex Current player
* @param wpn Weapon type
* @param i Increment amount
*/
void CFFStatsLogging::AddToWpnFireCount(int playerindex, FFWeaponID wpn, int i /* = 1 */) 
{
	if (m_pCurrentPlayers[playerindex]) 
		(*m_pCurrentPlayers[playerindex])->m_nWpnFire[wpn] += i;
}

/**
* Add a count to hits with weapons
*
* @param playerindex Current player
* @param wpn Weapon type
* @param i Increment amount
*/
void CFFStatsLogging::AddToWpnHitCount(int playerindex, FFWeaponID wpn, int i /* = 1 */) 
{
	if (m_pCurrentPlayers[playerindex]) 
		(*m_pCurrentPlayers[playerindex])->m_nWpnFire[wpn] += i;
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
	int i, j;

	// Basic header information
	buf.Add("login ff-test\n");
	buf.Add("auth %s\n", GetAuthString());
	buf.Add("date %s\n", GetTimestampString());
	buf.Add("duration %d\n", 1800);
	buf.Add("map %s\n", m_szMapName);
	buf.Add("bluescore %d\n", 0);
	buf.Add("redscore %d\n", 0);
	buf.Add("yellowscore %d\n", 0);
	buf.Add("greenscore %d\n", 0);
	buf.Add("playerdef\n");

	std::list<int> i_Done;
	std::list<int>::iterator i_Find;

	// Loop through defining all players
	// Format: STEAMID NAME UNIQUEID
	for (i = 0; i < m_nPlayers; i++) 
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

	buf.Add("statdef\n");

	// Loop through defining all the stats
	// FORMAT: STATNAME STATID
	for (i = 0; i < STAT_MAX; i++)
		buf.Add("%s %d\n", g_pszStatStrings[i], i);

	// Loop through defining all timers and include these as stats too
	// FORMAT: TIMERNAME TIMERID+STAT_MAX
	for (i = 0; i < TIMER_MAX; i++)
		buf.Add("%s %d\n", g_pszTimerStrings[i], STAT_MAX + i);

	// Loop through defining all weapon stats too
	// FORMAT: WEAPONSTATNAME WEAPONSTATID
	for (i = 0; i < FF_WEAPON_MAX; i++)
	{
		buf.Add("fired_%s %d\n", s_WeaponAliasInfo[i], STAT_MAX + TIMER_MAX + (2 * i));
		buf.Add("hit_%s %d\n", s_WeaponAliasInfo[i], STAT_MAX + TIMER_MAX + (2 * i) + 1);
	}

	buf.Add("weapondef\n");

	// Loop through defining all the weapon names
	// FORMAT: WEAPONNAME WEAPID
	for (i = 0; i < FF_WEAPON_MAX; i++)
		buf.Add("%s %d\n", s_WeaponAliasInfo[i], i);

	// Loop through each player/class combination and print the changed stats
	// FORMAT: UNIQUEID CLASSID STATID STATVALUE
	for (i = 0; i < m_nPlayers; i++) 
	{
		if (!m_pPlayerStats[i]) 
			continue;

		// First print all the normal stats
		for (j = 0; j < STAT_MAX; j++) 
		{
			// Don't send any unchanged values
			if (m_pPlayerStats[i]->m_iCounters[j] == 0) 
				continue;

			buf.Add("%d %d %d %hu\n", m_pPlayerStats[i]->m_iPlayerUid, m_pPlayerStats[i]->m_iPlayerClass, j, m_pPlayerStats[i]->m_iCounters[j]);
		}

		// Then print all timer stats. As integers or floats?
		for (j = 0; j < TIMER_MAX; j++)
		{
			// Don't send any unchanged values
			if (m_pPlayerStats[i]->m_flTimers[j] == 0)
				continue;

			buf.Add("%d %d %d %hu\n", m_pPlayerStats[i]->m_iPlayerUid, m_pPlayerStats[i]->m_iPlayerClass, j, (unsigned short) m_pPlayerStats[i]->m_flTimers[j]);
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