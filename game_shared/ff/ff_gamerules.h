//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: The TF Game rules object
//
// $Workfile:     $
// $Date: 2005/08/26 22:00:12 $
// $NoKeywords: $
//=============================================================================//

#ifndef FF_GAMERULES_H
#define FF_GAMERULES_H

#ifdef _WIN32
#pragma once
#endif


#include "teamplay_gamerules.h"
#include "convar.h"
#include "gamevars_shared.h"

#ifdef CLIENT_DLL
	#include "c_baseplayer.h"
#else
	#include "player.h"
	#include "ff_buildableobjects_shared.h"
	#include "ff_mapfilter.h"
#endif


#ifdef CLIENT_DLL
	#define CFFGameRules C_FFGameRules
	#define CFFGameRulesProxy C_FFGameRulesProxy
#endif

#ifdef GAME_DLL
	extern ConVar mp_respawndelay;	
#endif

class CFFGameRulesProxy : public CGameRulesProxy
{
public:
	DECLARE_CLASS( CFFGameRulesProxy, CGameRulesProxy );
	DECLARE_NETWORKCLASS();
};

#ifdef GAME_DLL
// Reset flags. Keep the order the same
// as in base.lua!
enum ApplyToFlag_e
{
	AT_KILL_PLAYERS = 0,
	AT_RESPAWN_PLAYERS,
	AT_DROP_ITEMS,
	AT_FORCE_DROP_ITEMS,
	AT_THROW_ITEMS,
	AT_FORCE_THROW_ITEMS,
	AT_RETURN_CARRIED_ITEMS,
	AT_RETURN_DROPPED_ITEMS,
	AT_REMOVE_RAGDOLLS,
	AT_REMOVE_PACKS,
	AT_REMOVE_PROJECTILES,
	AT_REMOVE_BUILDABLES,
	AT_REMOVE_DECALS,
	AT_END_MAP,
	AT_RELOAD_CLIPS,
	AT_ALLOW_RESPAWN,
	AT_DISALLOW_RESPAWN,

	// Change class "ApplyTo" flags
	AT_CHANGECLASS_SCOUT,
	AT_CHANGECLASS_SNIPER,
	AT_CHANGECLASS_SOLDIER,
	AT_CHANGECLASS_DEMOMAN,
	AT_CHANGECLASS_MEDIC,
	AT_CHANGECLASS_HWGUY,
	AT_CHANGECLASS_PYRO,
	AT_CHANGECLASS_SPY,
	AT_CHANGECLASS_ENGINEER,
	AT_CHANGECLASS_CIVILIAN,
	AT_CHANGECLASS_RANDOM,

	// Change team "ApplyTo" flags
	AT_CHANGETEAM_BLUE,
	AT_CHANGETEAM_RED,
	AT_CHANGETEAM_YELLOW,
	AT_CHANGETEAM_GREEN,
	AT_CHANGETEAM_SPEC,

	AT_STOP_PRIMED_GRENS,

	// Yeah
	AT_MAX_FLAG
};
#endif


class CFFGameRules : public CTeamplayRules
{
public:
	DECLARE_CLASS( CFFGameRules, CTeamplayRules );

	virtual bool	ShouldCollide( int collisionGroup0, int collisionGroup1 );

	virtual int		PlayerRelationship(CBaseEntity *pPlayer, CBaseEntity *pTarget);

	// Changed to incorporate buildables and to stop the fugly casting. 
	// Previously, with FCanPlayerTakeDamage(), everything had to be cast down. 
	// You can now pass players, sentries & dispensers to the function.  
	virtual bool	FCanTakeDamage( CBaseEntity *pVictim, CBaseEntity *pAttacker);

	virtual bool	IsTeamplay( void ) { return false;	}
	bool			IsIntermission();

	// Returns whether or not iTeam1 is allied to iTeam2
	int				IsTeam1AlliedToTeam2( int iTeam1, int iTeam2 );

	void			PlayerKilled(CBasePlayer *pVictim, const CTakeDamageInfo &info);


#ifdef CLIENT_DLL

	DECLARE_CLIENTCLASS_NOBASE(); // This makes datatables able to access our private vars.

	CFFGameRules( void ) { m_flRoundStarted = 0.0f; }

#else

	DECLARE_SERVERCLASS_NOBASE(); // This makes datatables able to access our private vars.
	
	CFFGameRules();
	virtual ~CFFGameRules();

	virtual void	RadiusDamage(const CTakeDamageInfo &info, const Vector &vecSrc, float flRadius, int iClassIgnore, CBaseEntity *pEntityIgnore);
	virtual float	GetAdjustedPushForce(float flPushForce, CBaseEntity *pVictim, const CTakeDamageInfo &info);
	virtual float	GetAdjustedDamage(float flDamage, CBaseEntity *pVictim, const CTakeDamageInfo &info);

	virtual float	FlPlayerFallDamage(CBasePlayer *pPlayer);
	virtual bool	FlPlayerFallDeathDoesScreenFade( CBasePlayer *pPlayer );
	virtual bool	ClientCommand( const char *pcmd, CBaseEntity *pEdict );
	virtual void	Think();
	virtual void	BuildableKilled( CFFBuildableObject *pObject, const CTakeDamageInfo& info );

	virtual const char *GetChatPrefix( bool bTeamOnly, CBasePlayer *pPlayer );
	virtual const char *GetChatLocation( bool bTeamOnly, CBasePlayer *pPlayer );

	virtual void	Precache();

	// This resets the map currently in progress
	virtual void	RestartRound( void );

	virtual void	ResetUsingCriteria( bool *pbFlags, int iTeam = TEAM_UNASSIGNED, CFFPlayer *pFFPlayer = NULL, bool bFullReset = false );
	virtual void	GoToIntermission( void );

	virtual void	UpdateSpawnPoints();
	CUtlVector<CBaseEntity*> m_SpawnPoints;

	virtual bool	IsSpawnPointClear( CBaseEntity *pSpot, CBasePlayer *pPlayer );
	virtual bool	IsSpawnPointValid( CBaseEntity *pSpot, CBasePlayer *pPlayer );
	virtual bool	FPlayerCanRespawn( CBasePlayer *pPlayer );
	virtual bool	ClientConnected( edict_t *pEdict, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen );
	virtual void	ClientDisconnected( edict_t *pClient );
	virtual void	LevelShutdown( void );
	
	virtual void	CreateStandardEntities();
	
	virtual void	ClientSettingsChanged( CBasePlayer *pPlayer );

//private:
//	CFFMapFilter	m_hMapFilter;

#endif

protected:

	// Prematch stuff
	float	m_flGameStarted;
	float	m_flNextMsg;
	CNetworkVar( float, m_flRoundStarted );

public:
	void StartGame(bool bAllowReset=true);
	bool HasGameStarted() { return !( m_flGameStarted < 0 ); }
	float GetRoundStart( void ) const { return m_flRoundStarted; }
	void SetRoundStart( float flStartTime ) { m_flRoundStarted = flStartTime; }
};

//-----------------------------------------------------------------------------
// Gets us at the team fortress game rules
//-----------------------------------------------------------------------------

inline CFFGameRules* FFGameRules()
{
	return static_cast<CFFGameRules*>(g_pGameRules);
}

#endif // FF_GAMERULES_H
