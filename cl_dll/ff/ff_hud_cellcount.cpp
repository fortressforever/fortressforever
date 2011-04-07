//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// ff_hud_cellcount.cpp
//
// implementation of CHudCellCount class
//
#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"

#include "iclientmode.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

using namespace vgui;

#include "hudelement.h"
#include "ff_panel.h"

#include "ConVar.h"

#include "c_ff_player.h"
#include "ff_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INIT_CELLCOUNT -1

//-----------------------------------------------------------------------------
// Purpose: Cell count panel
//-----------------------------------------------------------------------------
class CHudCellCount : public CHudElement, public vgui::FFPanel
{
	DECLARE_CLASS_SIMPLE( CHudCellCount, vgui::FFPanel );

public:
	CHudCellCount( const char *pElementName );
	~CHudCellCount();
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();
	virtual void Paint();

protected:
	CHudTexture		*m_pHudCellIcon;

private:
	
	CPanelAnimationVar( vgui::HFont, m_hIconFont, "IconFont", "WeaponIconsSmall" );
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "HUD_TextSmall" );
	CPanelAnimationVar( Color, m_IconColor, "HUD_Tone_Default", "HUD_Tone_Default" );

	CPanelAnimationVarAliasType( float, text_xpos, "text_xpos", "34", "proportional_float" );
	CPanelAnimationVarAliasType( float, text_ypos, "text_ypos", "10", "proportional_float" );

	CPanelAnimationVarAliasType( float, image_xpos, "image_xpos", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, image_ypos, "image_ypos", "4", "proportional_float" );

	int		m_iCellCount;
};	

DECLARE_HUDELEMENT( CHudCellCount );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudCellCount::CHudCellCount( const char *pElementName ) : CHudElement( pElementName ), vgui::FFPanel( NULL, "HudCellCount" )
{
	SetParent( g_pClientMode->GetViewport() );
	SetHiddenBits( HIDEHUD_PLAYERDEAD );
}

CHudCellCount::~CHudCellCount()
{
	if( m_pHudCellIcon )
	{
		delete m_pHudCellIcon;
		m_pHudCellIcon = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCellCount::Init()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCellCount::Reset()
{
	m_iCellCount	= INIT_CELLCOUNT;
	m_pHudCellIcon = new CHudTexture;
	m_pHudCellIcon->bRenderUsingFont = true;
	m_pHudCellIcon->hFont = m_hIconFont;
	m_pHudCellIcon->cCharacterInFont = '6';

	SetPaintBackgroundEnabled( false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCellCount::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCellCount::OnThink()
{
	// Fix for ToFFPlayer( NULL ) being called.
	if( !engine->IsInGame() )
		return;

	int newCells = 0;	

	C_FFPlayer *local = C_FFPlayer::GetLocalFFPlayer();
	if (!local)
		return;

	// Never below zero
	newCells = max( local->GetAmmoCount( AMMO_CELLS ), 0);

	// Only update the fade if we've changed cell count
	if ( newCells == m_iCellCount )
	{
		return;
	}

	// Play appropriate animation whether cell count has gone up or down
	if( newCells > m_iCellCount )
	{
		// Cell count went up
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "CellCountIncrease" );
	}
	else
	{
		// Cell count went down
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "CellCountDecrease" );
	}

	m_iCellCount = newCells;
}

void CHudCellCount::Paint()
{
	if( !engine->IsInGame() )
		return;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if( !pPlayer )
		return;

	if( FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer ) )
		return;

	if(pPlayer->GetClassSlot() != CLASS_ENGINEER)
		return;

	BaseClass::PaintBackground();

	if (m_pHudCellIcon)
	{
		m_pHudCellIcon->DrawSelf( image_xpos, image_ypos, m_IconColor );
		
		// Get the class as a string
		wchar_t unicode[6];
		swprintf(unicode, L"%d", m_iCellCount);

		// Draw text
		surface()->DrawSetTextFont( m_hTextFont );
		surface()->DrawSetTextColor( m_IconColor );
		surface()->DrawSetTextPos( text_xpos, text_ypos );
		surface()->DrawUnicodeString( unicode );
	}
}
