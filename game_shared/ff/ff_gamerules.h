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
#endif


#ifdef CLIENT_DLL
	#define CFFGameRules C_FFGameRules
	#define CFFGameRulesProxy C_FFGameRulesProxy
#endif


class CFFGameRulesProxy : public CGameRulesProxy
{
public:
	DECLARE_CLASS( CFFGameRulesProxy, CGameRulesProxy );
	DECLARE_NETWORKCLASS();
};


class CFFGameRules : public CTeamplayRules
{
public:
	DECLARE_CLASS( CFFGameRules, CTeamplayRules );

	virtual bool	ShouldCollide( int collisionGroup0, int collisionGroup1 );

	virtual int		PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );
	virtual bool	IsTeamplay( void ) { return false;	}	

#ifdef CLIENT_DLL

	DECLARE_CLIENTCLASS_NOBASE(); // This makes datatables able to access our private vars.

#else

	DECLARE_SERVERCLASS_NOBASE(); // This makes datatables able to access our private vars.
	
	CFFGameRules();
	virtual ~CFFGameRules();


	virtual float FlPlayerFallDamage( CBasePlayer *pPlayer );
	virtual bool FlPlayerFallDeathDoesScreenFade( CBasePlayer *pPlayer );
	virtual bool ClientCommand( const char *pcmd, CBaseEntity *pEdict );
	virtual void RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore );
	virtual void Think();
	virtual void BuildableKilled( CFFBuildableObject *pObject, const CTakeDamageInfo& info );

	virtual const char *GetChatPrefix( bool bTeamOnly, CBasePlayer *pPlayer );

	virtual void Precache( void );	// |-- Mirv: Just need a function that occurs at the start of each map to reset pm

private:

	void RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore, bool bIgnoreWorld );


#endif

protected:
	// --> Mirv: Prematch
	void StartGame();
	float m_flGameStarted;

public:
	bool HasGameStarted() { return !( m_flGameStarted < 0 ); }
	// <-- Mirv: Prematch


};

//-----------------------------------------------------------------------------
// Gets us at the team fortress game rules
//-----------------------------------------------------------------------------

inline CFFGameRules* FFGameRules()
{
	return static_cast<CFFGameRules*>(g_pGameRules);
}


#endif // FF_GAMERULES_H
