//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "hud.h"
#include "clientmode_ff.h"
#include "cdll_client_int.h"
#include "iinput.h"
#include "vgui/isurface.h"
#include "vgui/ipanel.h"
#include <vgui_controls/AnimationController.h>
#include "ivmodemanager.h"
//#include "BuyMenu.h"
#include "filesystem.h"
#include "vgui/ivgui.h"
#include "keydefs.h"
#include "hud_chat.h"
#include "view_shared.h"
#include "view.h"
#include "ivrenderview.h"
#include "model_types.h"
#include "iefx.h"
#include "dlight.h"
#include <imapoverview.h>
#include "c_playerresource.h"
#include <keyvalues.h>
#include "text_message.h"
#include "panelmetaclassmgr.h"

// Bug #0000310: fov doesn't reset |-- Mulch
//ConVar default_fov( "default_fov", "90", FCVAR_NONE );
ConVar default_fov( "default_fov", "90", FCVAR_ARCHIVE, "Default FOV value", true, 80.0, true, 120.0 );

IClientMode *g_pClientMode = NULL;


// --------------------------------------------------------------------------------- //
// CFFModeManager.
// --------------------------------------------------------------------------------- //

class CFFModeManager : public IVModeManager
{
public:
	virtual void	Init();
	virtual void	SwitchMode( bool commander, bool force ) {}
	virtual void	LevelInit( const char *newmap );
	virtual void	LevelShutdown( void );
	virtual void	ActivateMouse( bool isactive ) {}
};

static CFFModeManager g_ModeManager;
IVModeManager *modemanager = ( IVModeManager * )&g_ModeManager;

// --------------------------------------------------------------------------------- //
// CFFModeManager implementation.
// --------------------------------------------------------------------------------- //

#define SCREEN_FILE		"scripts/vgui_screens.txt"

void CFFModeManager::Init()
{
	g_pClientMode = GetClientModeNormal();
	
	PanelMetaClassMgr()->LoadMetaClassDefinitionFile( SCREEN_FILE );
}

void CFFModeManager::LevelInit( const char *newmap )
{
	g_pClientMode->LevelInit( newmap );
}

void CFFModeManager::LevelShutdown( void )
{
	g_pClientMode->LevelShutdown();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ClientModeFFNormal::ClientModeFFNormal()
{
}

//-----------------------------------------------------------------------------
// Purpose: If you don't know what a destructor is by now, you are probably going to get fired
//-----------------------------------------------------------------------------
ClientModeFFNormal::~ClientModeFFNormal()
{
}


void ClientModeFFNormal::InitViewport()
{
	m_pViewport = new FFViewport();
	m_pViewport->Start( gameuifuncs, gameeventmanager );
}

ClientModeFFNormal g_ClientModeNormal;

IClientMode *GetClientModeNormal()
{
	return &g_ClientModeNormal;
}


ClientModeFFNormal* GetClientModeFFNormal()
{
	Assert( dynamic_cast< ClientModeFFNormal* >( GetClientModeNormal() ) );

	return static_cast< ClientModeFFNormal* >( GetClientModeNormal() );
}

float ClientModeFFNormal::GetViewModelFOV( void )
{
	return 74.0f;
}

int ClientModeFFNormal::GetDeathMessageStartHeight( void )
{
	return m_pViewport->GetDeathMessageStartHeight();
}

void ClientModeFFNormal::PostRenderVGui()
{
}




