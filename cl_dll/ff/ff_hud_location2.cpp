//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_location2.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 11/25/2006
//	@brief client side Hud Location2 Info
//
//	REVISIONS
//	---------
//	11/25/2006, Mulchman: 
//		First created

#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "view.h"

using namespace vgui;

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>

#include "c_ff_player.h"
#include "ff_utils.h"
#include "ff_panel.h"

//=============================================================================
//
//	class CHudLocation2
//
//=============================================================================
class CHudLocation2 : public CHudElement, public vgui::FFPanel
{
private:
	DECLARE_CLASS_SIMPLE( CHudLocation2, vgui::FFPanel );

public:
	CHudLocation2( const char *pElementName ) : vgui::FFPanel( NULL, "HudLocation2" ), CHudElement( pElementName )
	{
		// Set our parent window
		//SetParent( g_pClientMode->GetViewport() );

		// Hide when player is dead
		//SetHiddenBits( HIDEHUD_PLAYERDEAD );

		SetParent(g_pClientMode->GetViewport());
		SetHiddenBits(/*HIDEHUD_HEALTH | */HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION);

	}

	~CHudLocation2( void ) {}

	void Init( void );
	void VidInit( void );
	void Paint( void );
};

DECLARE_HUDELEMENT( CHudLocation2 );

void CHudLocation2::Init( void )
{
}

void CHudLocation2::VidInit( void )
{	
	SetPaintBackgroundEnabled( true );
}

void CHudLocation2::Paint( void )
{
	if( !engine->IsInGame() )
		return;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if( !pPlayer )
		return;

	if( FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer ) )
		return;

	BaseClass::Paint();
}
