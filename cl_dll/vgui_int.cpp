//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "vgui_int.h"
#include "ienginevgui.h"
#include "itextmessage.h"
#include "vguicenterprint.h"
#include "iloadingdisc.h"
#include "ifpspanel.h"
#include "imessagechars.h"
#include "inetgraphpanel.h"
#include "idebugoverlaypanel.h"
#include <vgui/isurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include "tier0/vprof.h"
#include "iclientmode.h"
#include <vgui_controls/Panel.h>
#include <KeyValues.h>
#include "FileSystem.h"
#include "ff_options.h"
#include "ff_gamemodes.h"

using namespace vgui;

void MP3Player_Create( vgui::VPANEL parent );
void MP3Player_Destroy();

#include <vgui/IInputInternal.h>
vgui::IInputInternal *g_InputInternal = NULL;

#include <vgui_controls/Controls.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void GetVGUICursorPos( int& x, int& y )
{
	vgui::input()->GetCursorPos(x, y);
}

void SetVGUICursorPos( int x, int y )
{
	if ( !g_bTextMode )
	{
		vgui::input()->SetCursorPos(x, y);
	}
}

class CHudTextureHandleProperty : public vgui::IPanelAnimationPropertyConverter
{
public:
	virtual void GetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		CHudTextureHandle *pHandle = ( CHudTextureHandle * )data;

		// lookup texture name for id
		if ( pHandle->Get() )
		{
			kv->SetString( entry->name(), pHandle->Get()->szShortName );
		}
		else
		{
			kv->SetString( entry->name(), "" );
		}
	}
	
	virtual void SetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		
		CHudTextureHandle *pHandle = ( CHudTextureHandle * )data;

		const char *texturename = kv->GetString( entry->name() );
		if ( texturename && texturename[ 0 ] )
		{
			CHudTexture *currentTexture = gHUD.GetIcon( texturename );
			pHandle->Set( currentTexture );
		}
		else
		{
			pHandle->Set( NULL );
		}
	}

	virtual void InitFromDefault( Panel *panel, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );

		CHudTextureHandle *pHandle = ( CHudTextureHandle * )data;

		const char *texturename = entry->defaultvalue();
		if ( texturename && texturename[ 0 ] )
		{
			CHudTexture *currentTexture = gHUD.GetIcon( texturename );
			pHandle->Set( currentTexture );
		}
		else
		{
			pHandle->Set( NULL );
		}
	}
};

static CHudTextureHandleProperty textureHandleConverter;

static void VGui_OneTimeInit()
{
	static bool initialized = false;
	if ( initialized )
		return;
	initialized = true;

	vgui::Panel::AddPropertyConverter( "CHudTextureHandle", &textureHandleConverter );

}

bool VGui_Startup( CreateInterfaceFn appSystemFactory )
{
	if ( !vgui::VGui_InitInterfacesList( "CLIENT", &appSystemFactory, 1 ) )
		return false;

	g_InputInternal = (IInputInternal *)appSystemFactory( VGUI_INPUTINTERNAL_INTERFACE_VERSION,  NULL );
	if ( !g_InputInternal )
	{
		return false; // c_vguiscreen.cpp needs this!
	}

	VGui_OneTimeInit();

	// Create any root panels for .dll
	VGUI_CreateClientDLLRootPanel();

	// Make sure we have a panel
	VPANEL root = VGui_GetClientDLLRootPanel();
	if ( !root )
	{
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void VGui_CreateGlobalPanels( void )
{
	VPANEL gameToolParent = enginevgui->GetPanel( PANEL_CLIENTDLL_TOOLS );
	VPANEL toolParent = enginevgui->GetPanel( PANEL_TOOLS );
	VPANEL uiParent = enginevgui->GetPanel( PANEL_GAMEUIDLL );	// |-- Mirv
#if defined( TRACK_BLOCKING_IO )
#if !defined( _XBOX )
	VPANEL gameDLLPanel = enginevgui->GetPanel( PANEL_GAMEDLL );
#else
	VPANEL gameDLLPanel = enginevgui->GetPanel( PANEL_ROOT );
#endif
#endif
	// Part of game
	textmessage->Create( gameToolParent );
	internalCenterPrint->Create( gameToolParent );
#ifndef _XBOX
	loadingdisc->Create( gameToolParent );
	messagechars->Create( gameToolParent );
#endif

	// Debugging or related tool
	fps->Create( toolParent );
#if defined( TRACK_BLOCKING_IO )
	iopanel->Create( gameDLLPanel );
#endif
#ifndef _XBOX
	netgraphpanel->Create( toolParent );
#endif
	debugoverlaypanel->Create( gameToolParent );

#ifndef _XBOX
	// Create mp3 player off of tool parent panel
	MP3Player_Create( toolParent );
#endif

	// --> Mirv: Create extra gameui panels
	ffoptions->Create(uiParent);
	ffgamemodes->Create(uiParent);
	// <-- Mirv
}

void VGui_Shutdown()
{
	VGUI_DestroyClientDLLRootPanel();
#ifndef _XBOX
	MP3Player_Destroy();
	netgraphpanel->Destroy();
#endif
	debugoverlaypanel->Destroy();
#if defined( TRACK_BLOCKING_IO )
	iopanel->Destroy();
#endif
	fps->Destroy();

#ifndef _XBOX
	messagechars->Destroy();
	loadingdisc->Destroy();
#endif
	internalCenterPrint->Destroy();
	textmessage->Destroy();

	// --> Mirv: Destroy extra gameui panels
	ffoptions->Destroy();
	ffgamemodes->Destroy();
	// <-- Mirv

	if ( g_pClientMode )
	{
		g_pClientMode->VGui_Shutdown();
	}

	// Make sure anything "marked for deletion"
	//  actually gets deleted before this dll goes away
	vgui::ivgui()->RunFrame();
}

static ConVar cl_showpausedimage( "cl_showpausedimage", "1", 0, "Show the 'Paused' image when game is paused." );

//-----------------------------------------------------------------------------
// Things to do before rendering vgui stuff...
//-----------------------------------------------------------------------------
void VGui_PreRender()
{
	VPROF( "VGui_PreRender" );
#ifndef _XBOX
	loadingdisc->SetLoadingVisible( engine->IsDrawingLoadingImage() && !engine->IsPlayingDemo() );
	loadingdisc->SetPausedVisible( !enginevgui->IsGameUIVisible() && cl_showpausedimage.GetBool() && engine->IsPaused() && !engine->IsTakingScreenshot() && !engine->IsPlayingDemo() );
#endif
}

void VGui_PostRender()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : cl_panelanimation - 
//-----------------------------------------------------------------------------
CON_COMMAND( cl_panelanimation, "Shows panel animation variables: <panelname | blank for all panels>." )
{
	if ( engine->Cmd_Argc() == 2 )
	{
		PanelAnimationDumpVars( engine->Cmd_Argv(1) );
	}
	else
	{
		PanelAnimationDumpVars( NULL );
	}
}

void GetHudSize( int& w, int &h )
{
	vgui::surface()->GetScreenSize( w, h );

	VPANEL hudParent = enginevgui->GetPanel( PANEL_CLIENTDLL );
	if ( hudParent )
	{
		vgui::ipanel()->GetSize( hudParent, w, h );
	}
}