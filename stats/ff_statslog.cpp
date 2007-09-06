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
#include "ff_string.h"

#include <time.h>

//#include <list>
//#include <algorithm>
//#include <string>
#include <vector>

#undef MINMAX_H
#include "minmax.h"

#include "tier0/memdbgon.h"

ConVar stats_enable("ff_stats_enable", "1", FCVAR_NOTIFY|FCVAR_ARCHIVE, "Enable uploading of stats to the Fortress Forever stats database.");
ConVar stats_login("ff_stats_login", "unset", FCVAR_PROTECTED, "Fortress Forever Stats login information for this server");
ConVar stats_secret("ff_stats_pass", "unset", FCVAR_PROTECTED, "Fortress Forever Stats shared secret for this server's login information");

class CFFPlayerStats 
{
public:
	// Default constructor
	CFFPlayerStats( void )
	{
		m_iClass = CLASS_NONE;
		m_iTeam = TEAM_UNASSIGNED;
		m_iUniqueID = -1;
	}

	// Overloaded constructor
	CFFPlayerStats( const char *pszName, const char *pszSteamID, int iClass, int iTeam, int iUniqueID )
	{
		m_sName = pszName;
		m_sSteamID = pszSteamID;
		m_iClass = iClass;
		m_iTeam = iTeam;
		m_iUniqueID = iUniqueID;
	}

	// Deconstructor
	~CFFPlayerStats( void )
	{
		Cleanup();
	}

	// Deallocate
	void Cleanup( void )
	{
		m_vStats.clear();
		m_vStartTimes.clear();
		m_vActions.clear();
	}
	
public:
	CFFString m_sName;
	CFFString m_sSteamID;
	int m_iClass;
	int m_iTeam;
	int m_iUniqueID;
	std::vector< double > m_vStats;
	std::vector< double > m_vStartTimes;
	std::vector< bool > m_vAutoApply;
	std::vector< CFFAction > m_vActions;
};

class CFFStatsLog : public IStatsLog 
{
public:
	CFFStatsLog();
	~CFFStatsLog();
	int GetActionID(const char *statname);
	int GetStatID(const char *statname, stattype_t type = STAT_ADD);
	int GetPlayerID(const char *steamid, int classid, int teamnum, int uniqueid, const char *name);
	void AddStat(int playerid, int statid, double value);
	void AddAction(int playerid, int targetid, int actionid, const char *param, Vector coords, const char *location);
	void StartTimer(int playerid, int statid, bool autoapply = true);
	void StopTimer(int playerid, int statid, bool apply = true);
	void ResetStats();
	void Serialise(char *buffer, int buffer_size);
	void FinalizeStats();

	const char *GetAuthString() const;
	const char *GetTimestampString() const;

	const bool HasData() { return m_vPlayers.size() && m_vStats.size(); }
private:
	// holds all of the player's stats
	std::vector<CFFPlayerStats> m_vPlayers;
	std::vector<CFFStatDef> m_vStats;
	std::vector<CFFActionDef> m_vActions;
};

// singleton
static CFFStatsLog g_StatsLogSingleton;
IStatsLog *g_StatsLog = (IStatsLog *) &g_StatsLogSingleton;

/**
Constructor for the CFFStatsLog class
*/
CFFStatsLog::CFFStatsLog()
{
}

/**
Destructor for the CFFStatsLog class
*/
CFFStatsLog::~CFFStatsLog()
{
	m_vPlayers.clear();
	m_vStats.clear();
	m_vActions.clear();
}

/**
Looks up the ID for a stat with the given name
*/
int CFFStatsLog::GetStatID(const char *statname, stattype_t type)
{
	VPROF_BUDGET( "CFFStatsLog::GetStatID", VPROF_BUDGETGROUP_FF_STATS );

	int i;
	
	// see if we have it already
	for( i = 0; i < (int)m_vStats.size(); i++ )
	{
		// if we do, then return it
		if( m_vStats[i].m_sName == statname )
			return i;
	}

	// otherwise we need to create it
	CFFStatDef s( statname, type );
	m_vStats.push_back( s );
	
	// i should now be the end, which is the one we created
	return i;
}

/**
Looks up the ID for a action with the given name
*/
int CFFStatsLog::GetActionID(const char *actionname)
{
	VPROF_BUDGET( "CFFStatsLog::GetActionID", VPROF_BUDGETGROUP_FF_STATS );

	int i;
	
	// see if we have it already
	for( i = 0; i < (int)m_vActions.size(); i++ )
	{
		// if we do, then return it
		if( m_vActions[i].m_sName == actionname )
			return i;
	}

	// otherwise we need to create it
	CFFActionDef s( actionname );
	m_vActions.push_back( s );
	
	// i should now be the end, which is the one we created
	return i;
}

/**
Looks up the ID for a stat with the given name
*/
int CFFStatsLog::GetPlayerID(const char *steamid, int classid, int teamnum, int uniqueid, const char *name)
{
	VPROF_BUDGET( "CFFStatsLog::GetPlayerID", VPROF_BUDGETGROUP_FF_STATS );

	int i;
	
	// see if we have it already
	for( i = 0; i < (int)m_vPlayers.size(); i++ )
	{
		// if we do, then return it
		if( ( m_vPlayers[i].m_iUniqueID == uniqueid ) && ( m_vPlayers[i].m_iClass == classid ) ) {
			m_vPlayers[i].m_sSteamID = steamid;
			m_vPlayers[i].m_sName = name;
			return i;
		}
	}

	// otherwise we need to create it
	CFFPlayerStats s( name, steamid, classid, teamnum, uniqueid );
	m_vPlayers.push_back( s );
	
	// i should now be the end, which is the one we created
	return i;
}

/**
Add a value to a statistic for a player. For example, add 1 shot fired with a pistol.
*/
void CFFStatsLog::AddStat(int playerid, int statid, double value)
{
	VPROF_BUDGET( "CFFStatsLog::AddStat", VPROF_BUDGETGROUP_FF_STATS );

	assert(playerid >= 0 && playerid < (int)m_vPlayers.size());
	assert(statid >= 0 && statid < (int)m_vStats.size());

	//DevMsg("[STATS] adding stat %d to %d (+%.2f)\n", statid, playerid, value);

	// make sure it's big enough
	if (m_vPlayers[playerid].m_vStats.size() < m_vStats.size())
		m_vPlayers[playerid].m_vStats.resize(m_vStats.size(), 0.0);

	// update the stat for the appropriate type
	if (m_vStats[statid].m_iType == STAT_ADD) 
	{
		m_vPlayers[playerid].m_vStats[statid] += value;
	} 
	else if (m_vStats[statid].m_iType == STAT_MIN) 
	{
		if (value < m_vPlayers[playerid].m_vStats[statid])
			m_vPlayers[playerid].m_vStats[statid] = value;
	} 
	else if (m_vStats[statid].m_iType == STAT_MAX) 
	{
		if (value > m_vPlayers[playerid].m_vStats[statid])
			m_vPlayers[playerid].m_vStats[statid] = value;
	}

	//DevMsg("Added stat to player %d: %s += %f\n", playerid, m_vStats[statid].m_sName, value);
}

/**
Add an action to the action list.
	playerid: playerid for the player that performed the action (retreived from GetPlayerID())
	targetid: playerid for the target (if applicable), otherwise -1
	actionid: action to log action as (retreived from GetActionID())
	param: a string that is context sensitive towards the action (for example "red_flag")
	coords: coordinates that this action happened at
	location: string representation of the player's location
*/
void CFFStatsLog::AddAction(int playerid, int targetid, int actionid, const char *param, Vector coords, const char *location)
{
	VPROF_BUDGET( "CFFStatsLog::AddAction", VPROF_BUDGETGROUP_FF_STATS );

	assert(playerid >= 0 && playerid < (int)m_vPlayers.size());
	
	float time = gpGlobals->curtime;

	DevMsg("[STATS] adding action %d[%s] to %d (at (%.2f, %.2f, %.2f) aka '%s')\n", actionid, param, playerid, coords.x, coords.y, coords.z, location);

	// build the action def
	CFFAction a( actionid, targetid, time, param, coords, location?location:"" );

	// add it
	m_vPlayers[ playerid ].m_vActions.push_back( a );
}

/**
Start a timer for a particular stat. For example, when a player joins a server,
start the timer for the stat "played", and stop it later when they leave the
server or change classes/teams.
*/
void CFFStatsLog::StartTimer(int playerid, int statid, bool autoapply)
{
	VPROF_BUDGET( "CFFStatsLog::StartTimer", VPROF_BUDGETGROUP_FF_STATS );

	assert(playerid >= 0 && playerid < (int)m_vPlayers.size());
	assert(statid >= 0 && statid < (int)m_vStats.size());

	// make sure it's big enough
	if (m_vPlayers[playerid].m_vStartTimes.size() < m_vStats.size())
		m_vPlayers[playerid].m_vStartTimes.resize(m_vStats.size(), 0.0);
	if (m_vPlayers[playerid].m_vAutoApply.size() < m_vStats.size())
		m_vPlayers[playerid].m_vAutoApply.resize(m_vStats.size(), 0.0);

	// make sure it's stopped
	if (m_vPlayers[playerid].m_vStartTimes[statid] > 0.0001)
	{
		DevWarning("Starting timer for stat %d without stopping it first\n", statid);
		StopTimer(playerid, statid, true);
	}

	// set the start time to now
	m_vPlayers[playerid].m_vStartTimes[statid] = gpGlobals->curtime;
	m_vPlayers[playerid].m_vAutoApply[statid] = autoapply;
}

/**
Stop a timer for a particular stat and optionally add the time (in seconds) to the
stat value. For example, if the stat "played" was stopped, then it would determine
the time it was "running" and add that value to the stat. If apply is false, then
the timer is simply stopped (for example if a stat was fastest "home-run" cap, the
timer would be started when the flag was touched, but cancelled if the flag is dropped)
*/
void CFFStatsLog::StopTimer(int playerid, int statid, bool apply)
{
	VPROF_BUDGET( "CFFStatsLog::StopTimer", VPROF_BUDGETGROUP_FF_STATS );
	
	// don't try this if we're not even running
	if (m_vPlayers.size() == 0)
		return;

	assert(playerid >= 0 && playerid < (int)m_vPlayers.size());
	assert(statid >= 0 && statid < (int)m_vStats.size());

	// make sure it's big enough
	if (m_vPlayers[playerid].m_vStartTimes.size() < m_vStats.size())
		m_vPlayers[playerid].m_vStartTimes.resize(m_vStats.size(), 0.0);
	if (m_vPlayers[playerid].m_vAutoApply.size() < m_vStats.size())
		m_vPlayers[playerid].m_vAutoApply.resize(m_vStats.size(), 0.0);
		
	if (apply && m_vPlayers[playerid].m_vStartTimes[statid] > 0.0f)
		AddStat(playerid, statid, gpGlobals->curtime - m_vPlayers[playerid].m_vStartTimes[statid]);

	m_vPlayers[playerid].m_vStartTimes[statid] = 0.0f;
	m_vPlayers[playerid].m_vAutoApply[statid] = false;	// false so it doesn't attempt to try to apply something that's not on
}

/**
Stop all timers that need to be stopped before the round ends
*/
void CFFStatsLog::FinalizeStats()
{
	int i, j;
	for (i=0; i<(int)m_vPlayers.size(); i++) {
		for (j=0; j<(int)m_vPlayers[i].m_vAutoApply.size(); j++) {
			if (m_vPlayers[i].m_vAutoApply[i]) {
				StopTimer(i, j, true);
			}
		}
	}			
}


/**
Reset all of the statistics for all players. This is useful for starting a new map
*/
void CFFStatsLog::ResetStats()
{
	VPROF_BUDGET( "CFFStatsLog::ResetStats", VPROF_BUDGETGROUP_FF_STATS );

	m_vPlayers.clear();
	m_vStats.clear();
	m_vActions.clear();

	// TODO: Do we care about other stuff resetting?
}

/**
* Retreive the authorisation string that verifies stats sent
*/
const char *CFFStatsLog::GetAuthString() const 
{
	return "ABC";
}

/**
* Retreive a timestamp
*/
const char *CFFStatsLog::GetTimestampString() const
{
	// should be enough, and this is single-use so redoing it shouldn't be a problem.
	static char ret[20];
	struct tm timeinfo;

	VCRHook_LocalTime(&timeinfo);

	strftime(ret, 20, "%m/%d/%y %H:%M:%S", &timeinfo);

	/*  	
	static char dateStr[9], timeStr[9];
	_strdate(dateStr);
	_strtime(timeStr);
	Q_snprintf(ret, 19, "%s %s", dateStr, timeStr);
	*/
	
	return ret;
}
/**
Serialise the stored data for sending
*/
void CFFStatsLog::Serialise(char *buffer, int buffer_size)
{
	VPROF_BUDGET( "CFFStatsLog::Serialise", VPROF_BUDGETGROUP_FF_STATS );

	DevMsg("[STATS] Generating Serialized stats log\n");

	// build the auth string here
	const char *pszLogin = stats_login.GetString();
	const char *pszSecret = stats_secret.GetString();
	const char *pszDate = GetTimestampString(); // abuse staticness of return here
	char preAuthString[80];
	Q_snprintf( preAuthString, 80, "%s%s%s", pszLogin, pszSecret, pszDate );
	//DevMsg( "[STATS] preAuthString: [%s]\n", preAuthString );

	int i, j, offset = 0;
	CFFTeam *pTeam;

	// Simple hash used here
	unsigned long hash = 0;
	for(i = 0; i < (int)strlen( preAuthString ); i++)
	{
		hash = (hash<<7) | (hash>>31);
		hash ^= preAuthString[i];
	}

	// Basic header information
	ConVar *hostname = cvar->FindVar("hostname");
	offset += Q_snprintf(buffer+offset, buffer_size-offset, "hostname %s\n", hostname->GetString());
	offset += Q_snprintf(buffer+offset, buffer_size-offset, "login %s\n", pszLogin);
	offset += Q_snprintf(buffer+offset, buffer_size-offset, "auth %08X\n", hash);
	offset += Q_snprintf(buffer+offset, buffer_size-offset, "date %s\n", pszDate);
	offset += Q_snprintf(buffer+offset, buffer_size-offset, "duration %d\n", (int)gpGlobals->curtime);
	offset += Q_snprintf(buffer+offset, buffer_size-offset, "map %s\n", gpGlobals->mapname.ToCStr());
	pTeam = GetGlobalFFTeam(TEAM_BLUE);
	offset += Q_snprintf(buffer+offset, buffer_size-offset, "bluescore %d\n", pTeam ? pTeam->GetScore() : -1);
	pTeam = GetGlobalFFTeam(TEAM_RED);
	offset += Q_snprintf(buffer+offset, buffer_size-offset, "redscore %d\n", pTeam ? pTeam->GetScore() : -1);
	pTeam = GetGlobalFFTeam(TEAM_YELLOW);
	offset += Q_snprintf(buffer+offset, buffer_size-offset, "yellowscore %d\n", pTeam ? pTeam->GetScore() : -1);
	pTeam = GetGlobalFFTeam(TEAM_GREEN);
	offset += Q_snprintf(buffer+offset, buffer_size-offset, "greenscore %d\n", pTeam ? pTeam->GetScore() : -1);
	
	// add the players section
	offset += Q_snprintf(buffer+offset, buffer_size-offset, "players\n");
	for (i=0; i<(int)m_vPlayers.size(); i++) {
		offset += Q_snprintf(buffer+offset, buffer_size-offset, "%s %s %d %d\n",
			m_vPlayers[i].m_sSteamID.GetString(),
			m_vPlayers[i].m_sName.GetString(),
			m_vPlayers[i].m_iTeam,
			m_vPlayers[i].m_iClass);
	}

	// add the actions section
	offset += Q_snprintf(buffer+offset, buffer_size-offset, "actions\n");
	for (i=0; i<(int)m_vPlayers.size(); i++) {
		for (j=0; j<(int)m_vPlayers[i].m_vActions.size(); j++) {
	//for (std::vector<CFFPlayerStats>::const_iterator it = m_vPlayers.begin(); it!=m_vPlayers.end(); it++) {
	//	for (std::vector<CFFAction>::const_iterator jt = (*it).m_vActions.begin(); jt!=(*it).m_vActions.end(); jt++) {
			offset += Q_snprintf(buffer+offset, buffer_size-offset, "%d %d %s %.0f %s %.0f,%.0f,%.0f %s\n",
				i,
				m_vPlayers[i].m_vActions[j].targetid,
				m_vActions[m_vPlayers[i].m_vActions[j].actionid].m_sName.GetString(),
				m_vPlayers[i].m_vActions[j].time,
				m_vPlayers[i].m_vActions[j].param.GetString(),
				m_vPlayers[i].m_vActions[j].coords.x,
				m_vPlayers[i].m_vActions[j].coords.y,
				m_vPlayers[i].m_vActions[j].coords.z,
				m_vPlayers[i].m_vActions[j].location.GetString());
		}
	}

	// add the stats section
	offset += Q_snprintf(buffer+offset, buffer_size-offset, "stats\n");
	for (i=0; i<(int)m_vPlayers.size(); i++) {
		for (j=0; j<(int)m_vPlayers[i].m_vStats.size(); j++) {
			if (m_vPlayers[i].m_vStats[j] == 0.0) continue; // skip unset stats
			offset += Q_snprintf(buffer+offset, buffer_size-offset, "%d %s %.0f\n",
				i,
				m_vStats[j].m_sName.GetString(),
				m_vPlayers[i].m_vStats[j]);
		}
	}
}

#define STATS_HOST "www.burntpopcorn.net"
#define STATS_URL "/ff/storestats.php"
#define STATS_BOUNDARY "STATSBOUNDSzx9n12"
void SendStats() 
{
	Socks sock;

	VPROF_BUDGET( "CFFStatsLog::SendStats", VPROF_BUDGETGROUP_FF_STATS );

	if (!stats_enable.GetBool())
		goto end;
		
	if (!g_StatsLogSingleton.HasData())
		goto end;

	g_StatsLogSingleton.FinalizeStats();
		
	// this is kind of wasteful :(
	char buf[100000], buf2[120000];

	DevMsg("[STATS] Sending stats...\n");

	g_StatsLogSingleton.Serialise(buf, sizeof(buf));
	//DevMsg(buf);
	//DevMsg("------\n\n");

	// generate post-data first (size is used in the part below)
	Q_snprintf(buf2, sizeof(buf2),
		"--" STATS_BOUNDARY "\r\n"
        "Content-Disposition: form-data; name=\"data\"\r\n"
        "\r\n%s\r\n"
        "--" STATS_BOUNDARY "\r\n",

		buf);

	// generate http request
    Q_snprintf(buf, sizeof(buf),
		"POST %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Connection: close\r\n"
		"Content-type: multipart/form-data, boundary=" STATS_BOUNDARY "\r\n"
		"Content-length: %d\r\n\r\n"
		"%s",
		
		STATS_URL,
		STATS_HOST,
		strlen(buf2),
		buf2);

	DevMsg(buf);
	//UTIL_LogPrintf(buf);

	// Open up a socket
	if (!sock.Open(/*SOCK_STREAM */ 1, 0)) 
	{
		Warning("[STATS] Could not open socket\n");
		goto end;
	}

	// Connect to remote host
	if (!sock.Connect(STATS_HOST, 80)) 
	{
		Warning("[STATS] Could not connect to remote host\n");
		goto end;
	}

	// Send data
	if (!sock.Send(buf)) 
	{
		Warning("[STATS] Could not send data to remote host\n");
		sock.Close();
		goto end;
	}

	// Send data
	if (!sock.Send(buf)) 
	{
		Warning("[STATS] Could not send data to remote host\n");
		sock.Close();
		goto end;
	}

	int a;

	// Send data
	if ((a = sock.Recv(buf, sizeof(buf)-1)) == 0) 
	{
		Warning("[STATS] Did not get response from stats server\n");
		sock.Close();
		return;
	}

	buf[a] = '\0';

	DevMsg("[STATS] Successfully sent stats data. Response:\n---\n%s\n---\n", buf);

	// Close socket
	sock.Close();

end:
	// now that we've sent them, we can reset them.	
	g_StatsLogSingleton.ResetStats();
}
