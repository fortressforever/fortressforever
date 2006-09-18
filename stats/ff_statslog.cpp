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
#include <vector>

#undef MINMAX_H
#include "minmax.h"

#include "tier0/memdbgon.h"

class CFFStatDef 
{
public:
	// Default constructor
	CFFStatDef( void )
	{
		m_sName = NULL;
		m_iType = STAT_INVALID;
	}

	// Overloaded constructor
	CFFStatDef( const char *pszName, stattype_t iType )
	{
		int iLen = Q_strlen( pszName );
		m_sName = new char[ iLen + 1 ];

		Assert( m_sName );

		for( int i = 0; i < iLen; i++ )
			m_sName[ i ] = pszName[ i ];

		m_sName[ iLen ] = '\0';
		m_iType = iType;
	}

	// Deconstructor
	~CFFStatDef( void )
	{
		Cleanup();
	}

	// Deallocate
	void Cleanup( void )
	{
		if( m_sName )
		{
			delete [] m_sName;
			m_sName = NULL;
		}
	}

public:
	char *m_sName;
	stattype_t m_iType;
};

class CFFActionDef 
{
public:
	// Default constructor
	CFFActionDef( void )
	{
		m_sName = NULL;
	}

	// Overloaded constructor
	CFFActionDef( const char *pszName )
	{
		int iLen = Q_strlen( pszName );
		m_sName = new char[ iLen + 1 ];

		Assert( m_sName );

		for( int i = 0; i < iLen; i++ )
			m_sName[ i ] = pszName[ i ];

		m_sName[ iLen ] = '\0';
	}

	// Deconstructor
	~CFFActionDef( void )
	{
		Cleanup();		
	}

	// Deallocate
	void Cleanup( void )
	{
		if( m_sName )
		{
			delete [] m_sName;
			m_sName = NULL;
		}
	}

public:
	char *m_sName;
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
		param = NULL;
		coords.Init();
		location = NULL;
	}

	// Overloaded constructor
	CFFAction( int iActionId, int iTargetId, int iTime, const char *pszParam, const Vector& vecCoords, const char *pszLocation )
	{
		actionid = iActionId;
		targetid = iTargetId;
		time = iTime;

		int iLen = Q_strlen( pszParam );
		param = new char[ iLen + 1 ];

		Assert( param );

		for( int i = 0; i < iLen; i++ )
			param[ i ] = pszParam[ i ];

		param[ iLen ] = '\0';

		coords = vecCoords;

		iLen = Q_strlen( pszLocation );
		location = new char[ iLen + 1 ];

		Assert( location );

		for( int i = 0; i < iLen; i++ )
			location[ i ] = pszLocation[ i ];

		location[ iLen ] = '\0';
	}

	// Deconstructor
	~CFFAction( void )
	{
		Cleanup();
	}

	// Deallocate
	void Cleanup( void )
	{
		if( param )
		{
			delete [] param;
			param = NULL;
		}

		if( location )
		{
			delete [] location;
			location = NULL;
		}
	}

public:
	int actionid;
	int targetid;
	int time;
	char *param;
	Vector coords;
	char *location;
};

class CFFPlayerStats 
{
public:
	// Default constructor
	CFFPlayerStats( void )
	{
		m_sName = NULL;
		m_sSteamID = NULL;
		m_iClass = CLASS_NONE;
		m_iTeam = TEAM_UNASSIGNED;
		m_iUniqueID = -1;
	}

	// Overloaded constructor
	CFFPlayerStats( const char *pszName, const char *pszSteamID, int iClass, int iTeam, int iUniqueID )
	{
		int iLen = Q_strlen( pszName );
		m_sName = new char[ iLen + 1 ];

		Assert( m_sName );

		for( int i = 0; i < iLen; i++ )
			m_sName[ i ] = pszName[ i ];

		m_sName[ iLen ] = '\0';

		iLen = Q_strlen( pszSteamID );
		m_sSteamID = new char[ iLen + 1 ];

		Assert( m_sSteamID );

		for( int i = 0; i < iLen; i++ )
			m_sSteamID[ i ] = pszSteamID[ i ];

		m_sSteamID[ iLen ] = '\0';

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
		if( m_sName )
		{
			delete [] m_sName;
			m_sName = NULL;
		}

		if( m_sSteamID )
		{
			delete [] m_sSteamID;
			m_sSteamID = NULL;
		}

		m_vStats.clear();
		m_vStartTimes.clear();

		for( int i = 0; i < (int)m_vActions.size(); i++ )
			m_vActions[ i ].Cleanup();

		m_vActions.clear();
	}

public:
	char *m_sName;
	char *m_sSteamID;
	int m_iClass;
	int m_iTeam;
	int m_iUniqueID;
	std::vector< double > m_vStats;
	std::vector< double > m_vStartTimes;
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
	// remove all the stuff so we don't have any memory leaks (I hope)
	for( int i = 0; i < (int)m_vPlayers.size(); i++ )
		m_vPlayers[ i ].Cleanup();

	for( int i = 0; i < (int)m_vStats.size(); i++ ) 
		m_vStats[ i ].Cleanup();

	// This wasn't here previously, memory leak (param/location)!
	for( int i = 0; i < (int)m_vActions.size(); i++ )
		m_vActions[ i ].Cleanup();

	m_vPlayers.clear();
	m_vStats.clear();
	m_vActions.clear();
}

/**
Looks up the ID for a stat with the given name
*/
int CFFStatsLog::GetStatID(const char *statname, stattype_t type)
{
	int i;
	
	// see if we have it already
	for (i=0; i<(int)m_vStats.size(); i++)
	{
		// if we do, then return it
		if (FStrEq(m_vStats[i].m_sName, statname))
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
	int i;
	
	// see if we have it already
	for (i=0; i<(int)m_vActions.size(); i++)
	{
		// if we do, then return it
		if (FStrEq(m_vActions[i].m_sName, actionname))
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
	int i;
	
	// see if we have it already
	for (i=0; i<(int)m_vPlayers.size(); i++)
	{
		// if we do, then return it
		if (FStrEq(m_vPlayers[i].m_sSteamID, steamid) && m_vPlayers[i].m_iClass == classid)
			return i;
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
	assert(playerid >= 0 && playerid < (int)m_vPlayers.size());
	assert(statid >= 0 && statid < (int)m_vStats.size());

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
	time: number of seconds since the beginning of the round
	param: a string that is context sensitive towards the action (for example "red_flag")
	coords: coordinates that this action happened at
	location: string representation of the player's location
*/
void CFFStatsLog::AddAction(int playerid, int targetid, int actionid, int time, const char *param, Vector coords, const char *location)
{
	assert(playerid >= 0 && playerid < (int)m_vPlayers.size());

	// build the action def
	CFFAction a( actionid, targetid, time, param, coords, location );

	// add it
	m_vPlayers[ playerid ].m_vActions.push_back( a );
}

/**
Start a timer for a particular stat. For example, when a player joins a server,
start the timer for the stat "played", and stop it later when they leave the
server or change classes/teams.
*/
void CFFStatsLog::StartTimer(int playerid, int statid)
{
	assert(playerid >= 0 && playerid < (int)m_vPlayers.size());
	assert(statid >= 0 && statid < (int)m_vStats.size());

	// make sure it's big enough
	if (m_vPlayers[playerid].m_vStartTimes.size() < m_vStats.size())
		m_vPlayers[playerid].m_vStartTimes.resize(m_vStats.size(), 0.0);

	// make sure it's stopped
	if (m_vPlayers[playerid].m_vStartTimes[statid] < 0.0001)
	{
		DevWarning("Starting timer for stat %d without stopping it first\n", statid);
		StopTimer(playerid, statid, true);
	}

	// set the start time to now
	m_vPlayers[playerid].m_vStartTimes[statid] = gpGlobals->curtime;
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
	assert(playerid >= 0 && playerid < (int)m_vPlayers.size());
	assert(statid >= 0 && statid < (int)m_vStats.size());

	// make sure it's big enough
	if (m_vPlayers[playerid].m_vStartTimes.size() < m_vStats.size())
		m_vPlayers[playerid].m_vStartTimes.resize(m_vStats.size(), 0.0);

	if (apply)
		AddStat(playerid, statid, gpGlobals->curtime - m_vPlayers[playerid].m_vStartTimes[statid]);

	m_vPlayers[playerid].m_vStartTimes[statid] = 0;
}

/**
Reset all of the statistics for all players. This is useful for starting a new map
*/
void CFFStatsLog::ResetStats()
{
	for( int i = 0; i < (int)m_vPlayers.size(); i++ )
	{
		m_vPlayers[ i ].Cleanup();
	}

	m_vPlayers.clear();

	// TODO: Do we care about other stuff resetting?
}

/**
* Retreive the authorisation string that verifies stats sent
*/
const char *CFFStatsLog::GetAuthString() 
{
	return "ABC";
}

/**
* Retreive a timestamp
*/
const char *CFFStatsLog::GetTimestampString() 
{
	return "22-Jan-2006 00:03 UTC";
}

/**
Serialise the stored data for sending
*/
void CFFStatsLog::Serialise(char *buffer, int buffer_size)
{
	DevMsg("[STATS] Generating Serialized stats log\n");
	CQuickBuffer buf(buffer, buffer_size);
	int i, j;

	// Basic header information
	buf.Add("login ff-test\n");
	buf.Add("auth %s\n", GetAuthString());
	buf.Add("date %s\n", GetTimestampString());
	buf.Add("duration %d\n", 1800);
	buf.Add("map %s\n", "fry_baked");
	buf.Add("bluescore %d\n", 0);
	buf.Add("redscore %d\n", 0);
	buf.Add("yellowscore %d\n", 0);
	buf.Add("greenscore %d\n", 0);
	
	// add the players section
	buf.Add("players\n");
	for (i=0; i<(int)m_vPlayers.size(); i++) {
		buf.Add("%s %s %d %d\n",
			m_vPlayers[i].m_sSteamID,
			m_vPlayers[i].m_sName,
			m_vPlayers[i].m_iTeam,
			m_vPlayers[i].m_iClass);
	}

	// add the actions section
	buf.Add("actions\n");
	for (i=0; i<(int)m_vPlayers.size(); i++) {
		for (j=0; j<(int)m_vPlayers[i].m_vActions.size(); j++) {
			buf.Add("%d %d %s %d %s %s %s\n",
				i,
				m_vPlayers[i].m_vActions[j].targetid,
				m_vActions[m_vPlayers[i].m_vActions[j].actionid],
				m_vPlayers[i].m_vActions[j].time,
				m_vPlayers[i].m_vActions[j].param,
				"",
				m_vPlayers[i].m_vActions[j].location);
		}
	}

	// add the stats section
	buf.Add("stats\n");
	for (i=0; i<(int)m_vPlayers.size(); i++) {
		for (j=0; j<(int)m_vPlayers[i].m_vStats.size(); j++) {
			if (m_vPlayers[i].m_vStats[j] == 0.0) continue; // skip unset stats
			buf.Add("%d %s %f\n",
				i,
				m_vStats[j].m_sName,
				m_vPlayers[i].m_vStats[j]);
		}
	}
}

/**
* Send the stats to the remote server
*/
void SendStats() 
{
	return;

	char buf[131072];

	DevWarning("[STATS] Sending stats...\n");

	/*g_StatsLog*/ g_StatsLogSingleton.Serialise(buf, sizeof(buf));

	DevMsg(buf);

	Socks sock;

	// Open up a socket
	if (!sock.Open(/*SOCK_STREAM */ 1, 0)) 
	{
		DevWarning("[STATS] Could not open socket\n");
		return;
	}

	// Connect to remote host
	if (!sock.Connect("localhost", 80)) 
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