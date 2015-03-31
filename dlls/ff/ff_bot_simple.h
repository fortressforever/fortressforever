#ifndef FF_BOT_SIMPLE_H
#define FF_BOT_SIMPLE_H
#ifdef _WIN32
#pragma once
#endif

#include "ff_player.h"

// This is our bot class.
class CFFBot : public CFFPlayer
{
public:
	DECLARE_CLASS( CFFBot, CFFPlayer );

	virtual void PreThink();
	virtual void PostThink();

	bool			m_bBackwards;

	float			m_flNextTurnTime;
	bool			m_bLastTurnToRight;

	float			m_flNextStrafeTime;
	float			m_flSideMove;

	QAngle			m_ForwardAngle;
	QAngle			m_LastAngles;
	
};

class CBotManager
{
public:
	static CBasePlayer* ClientPutInServerOverride_Bot( edict_t *pEdict, const char *playername )
	{
		// This tells it which edict to use rather than creating a new one.
		CBasePlayer::s_PlayerEdict = pEdict;

		CFFBot *pPlayer = static_cast<CFFBot *>( CreateEntityByName( "ff_bot" ) );
		if ( pPlayer )
		{
			pPlayer->SetPlayerName( playername );
		}

		return pPlayer;
	}
};

// If iTeam or iClass is -1, then a team or class is randomly chosen.
CFFBot *BotPutInServer( int teamNum, int classNum, const char *botname = 0 );

#endif // FF_BOT_SIMPLE_H