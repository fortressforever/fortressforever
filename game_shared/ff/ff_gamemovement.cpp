//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#include "gamemovement.h"
#include "ff_gamerules.h"
#include "ff_shareddefs.h"
#include "in_buttons.h"
#include "movevars_shared.h"


#ifdef CLIENT_DLL
	#include "c_ff_player.h"
#else
	#include "ff_player.h"
#endif


class CFFGameMovement : public CGameMovement
{
public:
	DECLARE_CLASS( CFFGameMovement, CGameMovement );

	CFFGameMovement();
};


// Expose our interface.
static CFFGameMovement g_GameMovement;
IGameMovement *g_pGameMovement = ( IGameMovement * )&g_GameMovement;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CGameMovement, IGameMovement,INTERFACENAME_GAMEMOVEMENT, g_GameMovement );


// ---------------------------------------------------------------------------------------- //
// CFFGameMovement.
// ---------------------------------------------------------------------------------------- //

CFFGameMovement::CFFGameMovement()
{
	//m_vecViewOffsetNormal = FF_PLAYER_VIEW_OFFSET;
}

