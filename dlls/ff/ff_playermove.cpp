//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "player_command.h"
#include "igamemovement.h"
#include "in_buttons.h"
#include "ipredictionsystem.h"
#include "ff_player.h"
#include "iservervehicle.h"


static CMoveData g_MoveData;
CMoveData *g_pMoveData = &g_MoveData;

IPredictionSystem *IPredictionSystem::g_pPredictionSystems = NULL;


//-----------------------------------------------------------------------------
// Sets up the move data 
//-----------------------------------------------------------------------------
class CFFPlayerMove : public CPlayerMove
{
DECLARE_CLASS( CFFPlayerMove, CPlayerMove );

public:
	virtual void	StartCommand( CBasePlayer *player, CUserCmd *cmd );
	virtual void	SetupMove( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move );
	virtual void	FinishMove( CBasePlayer *player, CUserCmd *ucmd, CMoveData *move );
};

// PlayerMove Interface
static CFFPlayerMove g_PlayerMove;

//-----------------------------------------------------------------------------
// Singleton accessor
//-----------------------------------------------------------------------------
CPlayerMove *PlayerMove()
{
	return &g_PlayerMove;
}

//-----------------------------------------------------------------------------
// Main setup, finish
//-----------------------------------------------------------------------------

void CFFPlayerMove::StartCommand( CBasePlayer *player, CUserCmd *cmd )
{
	BaseClass::StartCommand( player, cmd );
}

//-----------------------------------------------------------------------------
// Purpose: This is called pre player movement and copies all the data necessary
//          from the player for movement. (Server-side, the client-side version
//          of this code can be found in prediction.cpp.)
//-----------------------------------------------------------------------------
void CFFPlayerMove::SetupMove( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move )
{
	BaseClass::SetupMove( player, ucmd, pHelper, move );

	IServerVehicle *pVehicle = player->GetVehicle();
	if (pVehicle && gpGlobals->frametime != 0)
	{
		pVehicle->SetupMove( player, ucmd, pHelper, move ); 
	}
}


//-----------------------------------------------------------------------------
// Purpose: This is called post player movement to copy back all data that
//          movement could have modified and that is necessary for future
//          movement. (Server-side, the client-side version of this code can 
//          be found in prediction.cpp.)
//-----------------------------------------------------------------------------
void CFFPlayerMove::FinishMove( CBasePlayer *player, CUserCmd *ucmd, CMoveData *move )
{
	// Call the default FinishMove code.
	BaseClass::FinishMove( player, ucmd, move );

	IServerVehicle *pVehicle = player->GetVehicle();
	if (pVehicle && gpGlobals->frametime != 0)
	{
		pVehicle->FinishMove( player, ucmd, move );
	}
}
