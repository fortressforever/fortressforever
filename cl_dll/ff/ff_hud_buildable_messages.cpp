//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_buildable_messages.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 09/28/2005
//	@brief client side Hud message handling for buildable
//			objects when they send messages
//
//	REVISIONS
//	---------
//	09/28/2005, Mulchman: 
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

#include "c_ff_timers.h"
#include "c_ff_buildableobjects.h"
#include "ff_buildableobjects_shared.h"

//=============================================================================
//
//	class CHudBuildableMessages
//
//=============================================================================
// Class to handle when buildable objects send messages to clients
class CHudBuildableMessages : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CHudBuildableMessages, vgui::Panel );

public:
	CHudBuildableMessages( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudBuildableMessages" )
	{
		SetParent( g_pClientMode->GetViewport( ) );
		SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED );

		vgui::ivgui()->AddTickSignal( GetVPanel(), 100 );
	}

	void Init( void );
	void VidInit( void );
	void MsgFunc_DetpackStartTimer( bf_read &msg );
	void MsgFunc_DetpackStopTimer( bf_read &msg );
	void MsgFunc_Dispenser_EnemiesUsing( bf_read &msg );
	void MsgFunc_Dispenser_TouchEnemy( bf_read &msg );
	void MsgFunc_Dispenser_Destroyed( bf_read &msg );
	void MsgFunc_SentryGun_Destroyed( bf_read &msg );
	void OnTick( void );
	void Paint( void );

protected:
	float		m_flStartTime;
	float		m_flDuration;
	wchar_t		m_pText[ FF_BUILD_DISP_STRING_LEN ];	// Unicode text buffer

	void		CalculateWidthHeight( void );

};

DECLARE_HUDELEMENT( CHudBuildableMessages );
DECLARE_HUD_MESSAGE( CHudBuildableMessages, Dispenser_EnemiesUsing );
DECLARE_HUD_MESSAGE( CHudBuildableMessages, Dispenser_TouchEnemy );
DECLARE_HUD_MESSAGE( CHudBuildableMessages, Dispenser_Destroyed );
DECLARE_HUD_MESSAGE( CHudBuildableMessages, SentryGun_Destroyed );

void CHudBuildableMessages::Init( void )
{
	HOOK_HUD_MESSAGE( CHudBuildableMessages, Dispenser_EnemiesUsing );
	HOOK_HUD_MESSAGE( CHudBuildableMessages, Dispenser_TouchEnemy );
	HOOK_HUD_MESSAGE( CHudBuildableMessages, Dispenser_Destroyed );
	HOOK_HUD_MESSAGE( CHudBuildableMessages, SentryGun_Destroyed );

	// 4 seconds to show the messages enough time?
	m_flDuration = 4.0f;

	m_pText[ 0 ] = '\0';
}

void CHudBuildableMessages::VidInit( void )
{
	//SetPaintBackgroundEnabled( false );
	SetVisible( false );
	m_flStartTime = -99.0f;		// |-- Mirv: Fix messages reappearing next map
}

void CHudBuildableMessages::MsgFunc_Dispenser_EnemiesUsing( bf_read &msg )
{
	// Tell the player enemies are using his/her dispenser!
	int iRead = msg.ReadShort();

	if( iRead )
	{
		m_flStartTime = gpGlobals->curtime;

		wchar_t *pszTemp = vgui::localize()->Find( "#FF_DISPENSER_ENEMIESUSING" );
		if( pszTemp )
			wcscpy( m_pText, pszTemp );
		else
			vgui::localize()->ConvertANSIToUnicode( "Enemies are using your dispenser!", m_pText, sizeof( m_pText ) );

		CalculateWidthHeight();
	}
}

void CHudBuildableMessages::MsgFunc_Dispenser_TouchEnemy( bf_read &msg )
{
	// Tell the player the dispenser they touched's custom message (yeah, English!?)
	int iRead = msg.ReadShort();

	if( iRead )
	{
		char szText[ FF_BUILD_DISP_STRING_LEN ];
		msg.ReadString( szText, 256 );

		// DevMsg( "[Dispenser] %s\n", szText );

		// the "@1" means there's no message set
		if( Q_strcmp( szText, "@1" ) == 0 )
			return;

		m_flStartTime = gpGlobals->curtime;
		vgui::localize()->ConvertANSIToUnicode( szText, m_pText, sizeof( m_pText ) );
		CalculateWidthHeight();
	}
}

void CHudBuildableMessages::MsgFunc_Dispenser_Destroyed( bf_read &msg )
{
	// Tell the player his dispenser died
	int iRead = msg.ReadShort();

	if( iRead )
	{
		m_flStartTime = gpGlobals->curtime;

		wchar_t *pszTemp = vgui::localize()->Find( "#FF_DISPENSER_DESTROYED" );
		if( pszTemp )
			wcscpy( m_pText, pszTemp );
		else
			vgui::localize()->ConvertANSIToUnicode( "Your dispenser has been destroyed!", m_pText, sizeof( m_pText ) );

		CalculateWidthHeight();
	}
}

void CHudBuildableMessages::MsgFunc_SentryGun_Destroyed( bf_read &msg )
{
	// Tell the player his sentrygun died
	int iRead = msg.ReadShort();

	if( iRead )
	{
		m_flStartTime = gpGlobals->curtime;

		wchar_t *pszTemp = vgui::localize()->Find( "#FF_SENTRYGUN_DESTROYED" );
		if( pszTemp )
			wcscpy( m_pText, pszTemp );
		else
			vgui::localize()->ConvertANSIToUnicode( "Your sentrygun has been destroyed!", m_pText, sizeof( m_pText ) );
		
		CalculateWidthHeight();
	}
}

void CHudBuildableMessages::OnTick( void )
{
	if( !engine->IsInGame() )
		return;

	if(m_flDuration == 0.0f)
		m_flDuration = 4.0f;

	if( ( m_flStartTime + m_flDuration ) > gpGlobals->curtime )
		SetPaintEnabled(true);
	else
		SetPaintEnabled(false);
}

void CHudBuildableMessages::Paint( void )
{
	// Find our fade based on our time shown
	//float dt = ( ( m_flStartTime + m_flDuration ) - gpGlobals->curtime );
	//float flAlpha = SimpleSplineRemapVal( dt, 0.0f, m_flDuration, 255, 0 );
	//flAlpha = clamp( flAlpha, 0.0f, 255.0f );

	// Get our scheme and font information
	vgui::HScheme scheme = vgui::scheme()->GetScheme( "ClientScheme" );
	vgui::HFont hFont = vgui::scheme()->GetIScheme( scheme )->GetFont( "CloseCaption_Normal" );

	// Draw our text
	surface()->DrawSetTextFont( hFont ); // set the font	
	//surface()->DrawSetTextColor( 255, 255, 255, 255 ); // white
	surface()->DrawSetTextColor( GetFgColor() );
	surface()->DrawSetTextPos( 4, 4 ); // x,y position
	surface()->DrawPrintText( m_pText, wcslen( m_pText ) ); // print text
}

void CHudBuildableMessages::CalculateWidthHeight( void )
{
	vgui::HScheme scheme = vgui::scheme()->GetScheme( "ClientScheme" );
	vgui::HFont hFont = vgui::scheme()->GetIScheme( scheme )->GetFont( "CloseCaption_Normal" );

	int iWide = UTIL_ComputeStringWidth( hFont, m_pText ) + 12;
	int iTall = surface()->GetFontTall( hFont ) + 12;

	// Set the width/height of this 'panel'
	SetWide( iWide );
	SetTall( iTall );

	// Get the screen width/height
	int iScreenWide, iScreenTall;
	surface()->GetScreenSize( iScreenWide, iScreenTall );

	// Get our current x/y position of this 'panel'
	int x, y;
    GetPos( x, y );

	// Set a new position - centered on the screen w/ the same y val
	SetPos( ( iScreenWide / 2 ) - ( iWide / 2 ), y );
}
