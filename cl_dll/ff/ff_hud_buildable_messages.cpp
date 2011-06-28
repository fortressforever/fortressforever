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
		// Set our parent window
		SetParent( g_pClientMode->GetViewport( ) );

		// Hide when player is dead
		SetHiddenBits( HIDEHUD_PLAYERDEAD );

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
DECLARE_HUD_MESSAGE( CHudBuildableMessages, DetpackStartTimer );
DECLARE_HUD_MESSAGE( CHudBuildableMessages, DetpackStopTimer );
DECLARE_HUD_MESSAGE( CHudBuildableMessages, Dispenser_EnemiesUsing );
DECLARE_HUD_MESSAGE( CHudBuildableMessages, Dispenser_TouchEnemy );
DECLARE_HUD_MESSAGE( CHudBuildableMessages, Dispenser_Destroyed );
DECLARE_HUD_MESSAGE( CHudBuildableMessages, SentryGun_Destroyed );

void CHudBuildableMessages::Init( void )
{
	HOOK_HUD_MESSAGE( CHudBuildableMessages, DetpackStartTimer );
	HOOK_HUD_MESSAGE( CHudBuildableMessages, DetpackStopTimer );
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
	m_flStartTime = -99;		// |-- Mirv: Fix messages reappearing next map
}

void CHudBuildableMessages::MsgFunc_DetpackStartTimer( bf_read &msg )
{
	int iRead = msg.ReadShort();

	if( iRead )
	{
		/*
		C_FFTimer *pTimer = g_FFTimers.Create( FF_BUILDABLE_TIMER_DETPACK_STRING, ( float )iRead );
		if( pTimer )
		{
			pTimer->m_bRemoveWhenExpired = true;
			pTimer->StartTimer();
		}
		*/
	}
}

void CHudBuildableMessages::MsgFunc_DetpackStopTimer( bf_read &msg )
{
	int iRead = msg.ReadShort();

	if( iRead )
	{
		/*
		C_FFTimer *pTimer = g_FFTimers.FindTimer( FF_BUILDABLE_TIMER_DETPACK_STRING );
		if( pTimer )
			g_FFTimers.DeleteTimer( FF_BUILDABLE_TIMER_DETPACK_STRING );
			*/
	}
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

	if(( m_flStartTime + m_flDuration ) > gpGlobals->curtime )
	{
		if( !IsVisible() )
			SetVisible( true );
	}
	else
	{
		if( IsVisible() )
			SetVisible( false );
	}
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

// Bug #0000387: Sentry HUD information displayed twice
/*
//=============================================================================
//
//	class CHudSentryGunStatus
//
//=============================================================================
// Class to show an owner their SG's health when built
class CHudSentryGunStatus : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CHudSentryGunStatus, vgui::Panel );

public:
	CHudSentryGunStatus( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudSentryGunStatus" )
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport( ) );

		// None
		SetHiddenBits( 0 );

		vgui::ivgui()->AddTickSignal( GetVPanel() );
	}

	void Init( void );
	void VidInit( void );
	//void MsgFunc_DetpackStartTimer( bf_read &msg );
	void OnTick( void );
	void Paint( void );
	
	void Show( void )
	{
		if( !IsVisible() )
			SetVisible( true );
	}
	void Hide( void )
	{
		if( IsVisible() )
			SetVisible( false );
	}

protected:
	int			m_iHealthPerc;
	int			m_iAmmoPerc;
	bool		m_fNoRockets;

	vgui::HFont m_hFont;
	Color		m_colBrightFg;

	// Because of the different coloring, this thing
	// uses a total of 5 strings
	// SENTRYGUN [1] HEALTH [2] <#>% [3] AMMO [4] <%>% [5]
	// Could get away with 4 but am being lazy
	wchar_t		m_szText[ 5 ][ 32 ];
	int			m_iTextWidths[ 5 ];	// for drawing
	int			m_iSpaceWidth;	// width of a space in our current font

};

DECLARE_HUDELEMENT( CHudSentryGunStatus );

void CHudSentryGunStatus::Init( void )
{
	for( int i = 0; i < 5; i++ )
	{
		m_szText[ i ][ 0 ] = '\0';
		m_iTextWidths[ i ] = 0;
	}
}

void CHudSentryGunStatus::VidInit( void )
{
	SetVisible( false );

	// Get our scheme and font information
	vgui::HScheme scheme = vgui::scheme()->GetScheme( "ClientScheme" );

	// "Default" is our font - it's like verana size 9
	m_hFont = vgui::scheme()->GetIScheme( scheme )->GetFont( "Default" );

	// Storing off this color "BrightFg" - it's GetFgColor() with a higher alpha
	m_colBrightFg = vgui::scheme()->GetIScheme( scheme )->GetColor( "BrightFg", Color( 255, 220, 0, 255 ) );

	const wchar_t *pszSpace = L" ";
	m_iSpaceWidth = UTIL_ComputeStringWidth( m_hFont, pszSpace );

	// Build the strings that will never change now

	// The word "SENTRYGUN"
	int iIndex = 0;
	wchar_t *pszTemp = vgui::localize()->Find( "#FF_SENTRYGUN_STRING" );
	if( pszTemp )
		wcscpy( m_szText[ iIndex ], pszTemp );
	else
		vgui::localize()->ConvertANSIToUnicode( "SENTRYGUN", m_szText[ iIndex ], sizeof( m_szText[ iIndex ] ) );
	
	// Get the length of "SENTRYGUN"
	m_iTextWidths[ iIndex ] = UTIL_ComputeStringWidth( m_hFont, m_szText[ iIndex ] );

	// The word "HEALTH"
	iIndex = 1;
	pszTemp = vgui::localize()->Find( "#FF_HEALTH_STRING" );
	if( pszTemp )
		wcscpy( m_szText[ iIndex ], pszTemp );
	else
		vgui::localize()->ConvertANSIToUnicode( "HEALTH", m_szText[ iIndex ], sizeof( m_szText[ iIndex ] ) );

	// Get the length of "HEALTH"
	m_iTextWidths[ iIndex ] = UTIL_ComputeStringWidth( m_hFont, m_szText[ iIndex ] );

	// The word "AMMO"
	iIndex = 3;
	pszTemp = vgui::localize()->Find( "#FF_AMMO_STRING" );
	if( pszTemp )
		wcscpy( m_szText[ iIndex ], pszTemp );
	else
		vgui::localize()->ConvertANSIToUnicode( "AMMO", m_szText[ iIndex ], sizeof( m_szText[ iIndex ] ) );

	// Get the length of "AMMO"
	m_iTextWidths[ iIndex ] = UTIL_ComputeStringWidth( m_hFont, m_szText[ iIndex ] );

	// Set height of our bg box
	SetTall( surface()->GetFontTall( m_hFont ) + 12 );
}

void CHudSentryGunStatus::OnTick( void )
{
	if( !engine->IsInGame() )
		return;

	// Get the local player
	C_FFPlayer *pPlayer = ToFFPlayer( C_BasePlayer::GetLocalPlayer() );
	if( pPlayer )
	{
		// See if we have a sentrygun
		if( pPlayer->m_hSentryGun.Get() )
		{
			C_FFSentryGun *pSentryGun = ( C_FFSentryGun * )pPlayer->m_hSentryGun.Get();
			if( !pSentryGun )
			{
				Warning( "[HudSentryGunStatus] Error!\n" );

				// Stop showing it if we were drawing it
				Hide();

				return;
			}

			// Only want to show it once we actually have a level 1 built (or higher)
			// and we don't set the level until GoLive is called so this is perfect
			// for not drawing our health status until we GoLive the first time
			if( pSentryGun->m_iLevel < 0 )
				Hide();
			else
			{
				// We have at least a level 1 sg (ie. it isn't in the process of being built -
				// it's built and operational)

				m_iHealthPerc = ( (float) pSentryGun->GetHealth() / pSentryGun->GetMaxHealth() ) * 100;	// |-- Mirv: BUG #0000104: Sentry gun health status goes to 0% the moment the sentry loses health

				// Get ammo percentage
				m_iAmmoPerc = pSentryGun->m_iAmmoPercent;
				m_fNoRockets = false;

				// Mirv: We store the lack of rockets in the highest significant bit
				if (m_iAmmoPerc >= 128)
				{
					m_iAmmoPerc -= 128;
					m_fNoRockets = true;
				}

				// TODO: Show a message if level 3 and no rockets - probably add a bool
				// since it's a special thing and not use all the time

				_snwprintf( m_szText[ 2 ], sizeof( m_szText[ 2 ] ) / sizeof( wchar_t ) - 1, L"%i%%", m_iHealthPerc );
				m_iTextWidths[ 2 ] = UTIL_ComputeStringWidth( m_hFont, m_szText[ 2 ] );

				// Mirv: Just tacking on the message to here for now
				_snwprintf( m_szText[ 4 ], sizeof( m_szText[ 4 ] ) / sizeof( wchar_t ) - 1, L"%i%%%s", m_iAmmoPerc, (m_fNoRockets ? L" #FF_NOROCKETS" : L"") );
				m_iTextWidths[ 4 ] = UTIL_ComputeStringWidth( m_hFont, m_szText[ 4 ] );

				// TODO: Later calculate in the width of NO ROCKETS mesage ^^

				// Calculate width of bg box
				int iSum = 0;
				for( int i = 0; i < 5; i++ )
					iSum += m_iTextWidths[ i ];

				// Set width of bg box
				SetWide( ( 4 * m_iSpaceWidth ) + iSum + 12 );				

				// Show it!
				Show();
			}
		}
		else
		{
			// pPlayer->m_hSentryGun.Get() == NULL so stop showing
			Hide();
		}
	}
	else
	{
		// pPlayer == NULL so don't show at all
		Hide();
	}
}

void CHudSentryGunStatus::Paint( void )
{
	int iPos = 4;

	// Draw strings
	for( int i = 0; i < 5; i++ )
	{
		surface()->DrawSetTextFont( m_hFont );

		//if(( i == 2 ) || ( i == 4 ))
			surface()->DrawSetTextColor( m_colBrightFg );
		//else
			//surface()->DrawSetTextColor( GetFgColor() );

		// TODO: Change text pos
		surface()->DrawSetTextPos( iPos, 4 );
		iPos += m_iTextWidths[ i ] + m_iSpaceWidth;
		surface()->DrawPrintText( m_szText[ i ], wcslen( m_szText[ i ] ) );
	}
}
*/
