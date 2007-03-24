//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_menu_medengy.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 12/04/2006
//	@brief client side Hud Menu for Medic & Engy options
//
//	REVISIONS
//	---------
//	12/04/2006, Mulchman:
//		First created - mainly a copy/paste of CHudContextMenu

#include "cbase.h"
#include "ff_hud_menu_medengy.h"

#include "hudelement.h"
#include "hud_macros.h"
#include "mathlib.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>

#include "c_ff_player.h"
#include "ff_utils.h"

#include <vgui/ILocalize.h>

using namespace vgui;

extern ConVar sensitivity;
extern ConVar cm_capturemouse;
extern ConVar cm_showmouse;

#define MENU_PROGRESS_TIME	0.3f

// Global
CHudContextMenu_MedEngy *g_pMedEngyHudMenu = NULL;

DECLARE_HUDELEMENT( CHudContextMenu_MedEngy );

//-----------------------------------------------------------------------------
// Purpose: Menu elements
//-----------------------------------------------------------------------------
ADD_MENU_OPTION( MedEngyMenu_Armor, L"Armor!", "#FF_MEDENGYMENU_ARMOR" )
{
	return MENU_SHOW;
}

ADD_MENU_OPTION( MedEngyMenu_Medic, L"Medic!", "#FF_MEDENGYMENU_MEDIC" )
{
	return MENU_SHOW;
}

ADD_MENU_OPTION( MedEngyMenu_Ammo, L"Ammo!", "#FF_MEDENGYMENU_AMMO" )
{
	return MENU_SHOW;
}

//-----------------------------------------------------------------------------
// Purpose: Actual menu
//-----------------------------------------------------------------------------
menuoption_t MedEngyMenu[] = { MedEngyMenu_Armor, MedEngyMenu_Medic, MedEngyMenu_Ammo };

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
CHudContextMenu_MedEngy::~CHudContextMenu_MedEngy( void )
{
	if( m_pHudElementTexture )
	{
		delete m_pHudElementTexture;
		m_pHudElementTexture = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudContextMenu_MedEngy::Init( void )
{
	m_pHudElementTexture = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Called each map load
//-----------------------------------------------------------------------------
void CHudContextMenu_MedEngy::VidInit( void )
{
	// Menu initially not visible
	m_bMenuVisible = false;

	// Set global pointer to this hud element
	g_pMedEngyHudMenu = this;

	// Don't draw a background
	SetPaintBackgroundEnabled( false );

	// Deallocate
	if( m_pHudElementTexture )
	{
		delete m_pHudElementTexture;
		m_pHudElementTexture = NULL;
	}

	// Precache the background texture
	m_pHudElementTexture = new CHudTexture();
	m_pHudElementTexture->textureId = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_pHudElementTexture->textureId, "vgui/hud_button", true, false );

	// Set the menu to draw... (only 1 menu so doing this here)
	m_pMenu = &MedEngyMenu[0];
	m_nOptions = sizeof( MedEngyMenu ) / sizeof( MedEngyMenu[0] );
}

//-----------------------------------------------------------------------------
// Purpose: Should the hud element draw or not?
//-----------------------------------------------------------------------------
bool CHudContextMenu_MedEngy::ShouldDraw( void )
{
	if( !engine->IsInGame() )
		return false;

	if( !m_bMenuVisible )
		return false;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return false;

	if( FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer ) )
		return false;

	if( !pPlayer->IsAlive() )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Draw
//-----------------------------------------------------------------------------
void CHudContextMenu_MedEngy::Paint( void )
{
	if( m_flDuration == 0 )
		m_flDuration = 0.001f;

	float halfbuttonX = scheme()->GetProportionalScaledValue( 40.0f );
	float halfbuttonY = scheme()->GetProportionalScaledValue( 20.0f );

	// Button boxes
	surface()->DrawSetTexture(m_pHudElementTexture->textureId);
	surface()->DrawSetColor( 255, 255, 255, 255 );

	// Draw boxes
	for( int i = 0; i < m_nOptions; i++ )
		surface()->DrawTexturedRect( m_flPositions[i][0] - halfbuttonX, m_flPositions[i][1] - halfbuttonY, m_flPositions[i][0] + halfbuttonX, m_flPositions[i][1] + halfbuttonY );

	// Set and get height of font
	surface()->DrawSetTextFont( m_hTextFont );
	int offsetY = 0.5f * surface()->GetFontTall( m_hTextFont );

	// Colors we need later on
	Color highlighted( 255, 0, 0, 255 );
	Color dimmed( 100, 100, 100, 255 );

	float dx = m_flPosX - scheme()->GetProportionalScaledValue( 320 );
	float dy = m_flPosY - scheme()->GetProportionalScaledValue( 240 );

	int newSelection = -1;

	// If we're not in the middle then get the right button
	if( ( dx * dx ) + ( dy * dy ) > 2500 )
	{
		const float pi_2 = M_PI * 2.0f;

		float angle_per_button = pi_2 / m_nOptions;
		float angle = atan2f( dx, -dy ) + angle_per_button * 0.5f;

		if( angle < 0 )
			angle += pi_2;
		if( angle > pi_2 )
			angle -= pi_2;

		newSelection = angle / angle_per_button;
	}

	// Draw text for each box
	for( int i = 0; i < m_nOptions; i++ )
	{
		surface()->DrawSetTextColor( GetFgColor() );

		// Dimmed out
		if( m_pMenu[i].conditionfunc() != MENU_SHOW )
			surface()->DrawSetTextColor( dimmed );

		// Highlighted
		else if( newSelection == i )
			surface()->DrawSetTextColor( highlighted );

		// Work out centering and position & draw text
		int offsetX = 0.5f * UTIL_ComputeStringWidth( m_hTextFont, m_pMenu[i].szName );
		surface()->DrawSetTextPos( m_flPositions[i][0] - offsetX, m_flPositions[i][1] - offsetY );

		for( const wchar_t *wch = m_pMenu[i].szName; *wch != 0; wch++ )
			surface()->DrawUnicodeChar( *wch );
	}

	// Restart timer if a new selection
	if( newSelection != m_iSelected )
		m_flSelectStart = gpGlobals->curtime;

	m_iSelected = newSelection;

	// Show actual mouse location for debugging
	if( cm_showmouse.GetBool() )
	{
		surface()->DrawSetTexture( m_pHudElementTexture->textureId );
		surface()->DrawSetColor( 255, 255, 255, 255 );
		surface()->DrawTexturedRect( m_flPosX - 5, m_flPosY - 5, m_flPosX + 5, m_flPosY + 5 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: User is pressing their menu key
//-----------------------------------------------------------------------------
void CHudContextMenu_MedEngy::KeyDown( void )
{
	// First time set up the menu
	if( !m_bMenuVisible )
		SetMenu();

	m_bMenuVisible = true;
}

//-----------------------------------------------------------------------------
// Purpose: User let go of menu key
//-----------------------------------------------------------------------------
void CHudContextMenu_MedEngy::SetMenu( void )
{
	m_flSelectStart = gpGlobals->curtime;

	float midx = scheme()->GetProportionalScaledValue( 320 );
	float midy = scheme()->GetProportionalScaledValue( 240 );
	float dist = scheme()->GetProportionalScaledValue( 120 );

	m_flPosX = midx;
	m_flPosY = midy;

	// Get positions for buttons
	for( int i = 0; i < m_nOptions; i++ )
	{
		float increments = ( float )i * DEG2RAD( 360 ) / ( float )m_nOptions;

		m_flPositions[i][0] = midx + dist * sin( increments );
		m_flPositions[i][1] = midy - dist * cos( increments ) * 0.6f;
	}

	// Big enough to fit whole circle
	SetWide( scheme()->GetProportionalScaledValue( 640 ) );
	SetTall( scheme()->GetProportionalScaledValue( 480 ) );

	// Position in center
	SetPos( scheme()->GetProportionalScaledValue( 320 ) - GetWide() * 0.5f, scheme()->GetProportionalScaledValue( 240 ) - GetTall() * 0.5f );
}

//-----------------------------------------------------------------------------
// Purpose: User let go of menu key
//-----------------------------------------------------------------------------
void CHudContextMenu_MedEngy::KeyUp( void )
{
	// There is a menu and it's cancelling
	if( m_bMenuVisible && m_pMenu )
	{
		if( m_iSelected >= 0 )
		{
			if( m_pMenu[m_iSelected].conditionfunc() == MENU_SHOW )
				DoCommand( m_pMenu[m_iSelected].szCommand );
		}
		else
		{
			// Do medic call anyway
			DoCommand( m_pMenu[1].szCommand );
		}
	}

	m_bMenuVisible = false;
}

//-----------------------------------------------------------------------------
// Purpose: Issue a command
//-----------------------------------------------------------------------------
void CHudContextMenu_MedEngy::DoCommand( const char *pszCmd )
{
	//if( !m_pszPreviousCmd )
	//{
	//engine->ClientCmd( pszCmd );
	//}

	// Take resource string and try to find its unicode
	// equivalent. This might be dumb for not US guys.
	wchar_t szString[ 512 ];
	wchar_t *pszString = vgui::localize()->Find( pszCmd );
	if( !pszString )
	{
		vgui::localize()->ConvertANSIToUnicode( pszCmd, szString, 512 );
		pszString = szString;
	}

	// Then convert back to ansi and execute the string
	char szAnsiString[ 512 ];
	vgui::localize()->ConvertUnicodeToANSI( pszString, szAnsiString, 512 );
	engine->ClientCmd( szAnsiString );

	/* -- No layered menus, so ignore this
	// Currently this only supports has 2 levels of menu
	// If we need more, change m_pszPreviousCmd to a character array
	// and concatonate on each menu item's command each time 
	// we progress to next menu
	else
	{
		char szBuf[256];
		Q_snprintf( szBuf, 255, "%s%s", m_pszPreviousCmd, pszCmd );
		engine->ClientCmd( szBuf );
	}
	*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudContextMenu_MedEngy::MouseMove( float *x, float *y ) 
{
	float sensitivity_factor = 1.0f / ( sensitivity.GetFloat() == 0 ? 0.001f : sensitivity.GetFloat() );

	float midx = scheme()->GetProportionalScaledValue( 320 );
	float midy = scheme()->GetProportionalScaledValue( 240 );
	float dist = scheme()->GetProportionalScaledValue( 120 );

	m_flPosX = clamp( m_flPosX + ( *x * sensitivity_factor ), midx - dist, midx + dist );
	m_flPosY = clamp( m_flPosY + ( *y * sensitivity_factor ), midy - dist, midy + dist );

	if( m_bMenuVisible && cm_capturemouse.GetBool() )
	{
		*x = 0;
		*y = 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Send mouse to the menu
//-----------------------------------------------------------------------------
void HudContextMenuInput_MedEngy( float *x, float *y ) 
{
	if( g_pMedEngyHudMenu )
		g_pMedEngyHudMenu->MouseMove( x, y );
}
