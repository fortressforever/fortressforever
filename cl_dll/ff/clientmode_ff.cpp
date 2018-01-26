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
#include "c_soundscape.h"
#include "engine/IEngineSound.h"
#include "c_te_legacytempents.h"
#include "physpropclientside.h"
#include "ff_gamerules.h"
#include "usermessages.h"
#include "filesystem.h"
#include "KeyValues.h"

#include "ff_discordman.h"

// Bug #0000310: fov doesn't reset |-- Mulch
//ConVar default_fov( "default_fov", "90", FCVAR_NONE );
ConVar default_fov( "default_fov", "90", FCVAR_ARCHIVE, "Default FOV value", true, 80.0, true, 120.0 );
extern ConVar v_viewmodel_fov;

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

	_discord.LevelInit( newmap );

	// Reset client timer to zero (otherwise it'll
	// be where we left off with the last round)
	if( FFGameRules() )
		FFGameRules()->SetRoundStart( 0.0f );
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
	return v_viewmodel_fov.GetFloat();
}

int ClientModeFFNormal::GetDeathMessageStartHeight( void )
{
	return m_pViewport->GetDeathMessageStartHeight();
}

void ClientModeFFNormal::PostRenderVGui()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeFFNormal::Init( void )
{
	gameeventmanager->AddListener( this, "ff_restartround", false );

	ClientModeShared::Init();
}

//-----------------------------------------------------------------------------
// Purpose: Capture FF Specific events
//-----------------------------------------------------------------------------
void ClientModeFFNormal::FireGameEvent( IGameEvent *pEvent )
{
	const char *pszEventName = pEvent->GetName();

	if( Q_strcmp( "ff_restartround", pszEventName ) == 0 )
	{
		// Set up current round offset for client
		//float flCurtime = pEvent->GetFloat( "curtime" );

		// Recreate all client side physics props
		C_PhysPropClientside::RecreateAll();

		// Just tell engine to clear decals
		engine->ClientCmd( "r_cleardecals" );

		tempents->Clear();

		// Stop any looping sounds
		enginesound->StopAllSounds( true );

		Soundscape_OnStopAllSounds(); // Tell the soundscape system
	}

	BaseClass::FireGameEvent( pEvent );
}
