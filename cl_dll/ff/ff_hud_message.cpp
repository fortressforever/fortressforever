//========= Copyright © 1996-2004, Valve LLC, All rights reserved. ============
//
// Purpose: Simple HUD element
//
//=============================================================================

#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "view.h"
#include "ff_shareddefs.h"
#include "ff_utils.h"

using namespace vgui;

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

extern ConVar cl_drawhud;
ConVar hud_messages("hud_messages", "2", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Number of FF messages shown on the HUD at a time (set to 0 to disable messages)");

extern enum LuaColors;

#define DEFAULT_MESSAGE_DURATION 5.0f

// Contents of each entry in our list of messages
struct MessageItem 
{
	CHudTexture *pIcon;			// Icon texture reference
	wchar_t		pText[256];		// Unicode text buffer
	Color		pColor;			// Color of the message

	float		flStartTime;	// When the message was recevied
	float		flDuration;		// Duration of the message
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CHudGameMessage : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CHudGameMessage, vgui::Panel );

public:

	CHudGameMessage( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudGameMessage" ) 
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport() );

		SetPaintBackgroundEnabled( false );

		// Never hide
		SetHiddenBits( 0 );
	};

	void	Init( void );
	void	VidInit( void );
	void	Paint( void );
	
	void RetireExpiredMessages( void );
	Color	GetColor( int );
	
	// Callback function for the "GameMessage" user message
	void	MsgFunc_GameMessage( bf_read &msg );

private:

	CUtlVector<MessageItem> m_Messages;

};

DECLARE_HUDELEMENT( CHudGameMessage );
DECLARE_HUD_MESSAGE( CHudGameMessage, GameMessage );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudGameMessage::VidInit( void )
{
	// Store off a reference to our icon
	m_Messages.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudGameMessage::Init( void )
{
	HOOK_HUD_MESSAGE( CHudGameMessage, GameMessage );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudGameMessage::MsgFunc_GameMessage( bf_read &msg )
{
	
	byte wType = msg.ReadByte();

	if (wType < 0)
		return;

	// Read in our string
	char szString[256];
	msg.ReadString( szString, sizeof(szString) );

	// Do we have too many death messages in the queue?
	if ( m_Messages.Count() > 0 &&
		m_Messages.Count() >= hud_messages.GetInt() )
	{
		// Remove the oldest one in the queue, which will always be the first
		m_Messages.Remove(0);
	}

	MessageItem messageMsg;
	
	// Convert it to localize friendly unicode
	wchar_t *pszTemp = vgui::localize()->Find( szString );
	if( pszTemp )
		wcscpy( messageMsg.pText, pszTemp );
	else
		vgui::localize()->ConvertANSIToUnicode( szString, messageMsg.pText, sizeof( messageMsg.pText ) );
	
	if (m_Messages.Count() > 0)
	{
		if (Q_wcscmp( messageMsg.pText, m_Messages[m_Messages.Count() - 1].pText ) == 0)
			return;
	}

	switch (wType)
	{
	case HUD_MESSAGE:
		{
			messageMsg.flDuration = DEFAULT_MESSAGE_DURATION;
			messageMsg.pColor = Color(255, 255, 255, 255);

			break;
		}

	case HUD_MESSAGE_DURATION:
		{
			float t_flDuration = msg.ReadFloat(); // fDuration
			if (t_flDuration > 0)
				messageMsg.flDuration = t_flDuration; 
			else
				messageMsg.flDuration = DEFAULT_MESSAGE_DURATION;
			messageMsg.pColor = Color(255, 255, 255, 255);

			break;
		}

	case HUD_MESSAGE_COLOR:
		{
			float t_flDuration = msg.ReadFloat(); // fDuration
			if (t_flDuration > 0)
				messageMsg.flDuration = t_flDuration; 
			else
				messageMsg.flDuration = DEFAULT_MESSAGE_DURATION;
			messageMsg.pColor = GetColor( msg.ReadShort() ); // iColorID

			break;
		}

	case HUD_MESSAGE_COLOR_CUSTOM:
		{
			float t_flDuration = msg.ReadFloat(); // fDuration
			if (t_flDuration > 0)
				messageMsg.flDuration = t_flDuration; 
			else
				messageMsg.flDuration = DEFAULT_MESSAGE_DURATION;
			messageMsg.pColor = Color( msg.ReadShort(), msg.ReadShort(), msg.ReadShort() ); // iRed, iGreen, iBlue

			break;
		}
	}


	messageMsg.flStartTime = gpGlobals->curtime;
	messageMsg.pIcon = gHUD.GetIcon( "message_icon" );
	
	m_Messages.AddToTail( messageMsg );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudGameMessage::Paint( void )
{
	if ( !cl_drawhud.GetBool() || !hud_messages.GetBool() || !m_Messages.Count() )
		return;

	int iCount = m_Messages.Count();
	for ( int i = 0; i < iCount; i++ )
	{
		// Find our fade based on our time shown
		float dt = m_Messages[i].flDuration - ( gpGlobals->curtime - m_Messages[i].flStartTime );
		float flAlpha = 0.0f;
		float flStartFade = (m_Messages[i].flDuration > 4.0f) ? 2.0f : m_Messages[i].flDuration / 2;
		//DevMsg("dt: %f\n", dt);
		if (flStartFade > dt)
		{
			float normalizedTime = dt / flStartFade;
			float normalizedSquared = normalizedTime * normalizedTime;
			flAlpha = normalizedSquared * 255.0f;
			flAlpha = clamp( flAlpha, 0.0f, 255.0f );
			//DevMsg("normaltime: %f normalsquared: %f alpha: %f\n", normalizedTime, normalizedSquared, flAlpha);
		}
		else
			flAlpha = 255.0f;
		

		// Get our scheme and font information
		vgui::HScheme scheme = vgui::scheme()->GetScheme( "ClientScheme" );
		vgui::HFont hFont = vgui::scheme()->GetIScheme(scheme)->GetFont( "CloseCaption_Normal" );

		// Draw our text
		surface()->DrawSetTextFont( hFont ); // set the font	
		surface()->DrawSetTextColor( m_Messages[i].pColor.r(), m_Messages[i].pColor.g(), m_Messages[i].pColor.b(), flAlpha ); // custom color
		
		int y = (surface()->GetFontTall( hFont ) + 5) * i;

		// --> Mirv: Fixed hud message alignment
		//	surface()->DrawSetTextPos( 32, 8 ); // x,y position

		// Get the various sizes of things
		int iStringWidth, iStringHeight, iContainerWidth, iContainerHeight;

		surface()->GetTextSize(hFont, m_Messages[i].pText, iStringWidth, iStringHeight);
		GetSize(iContainerWidth, iContainerHeight);
		surface()->DrawSetTextPos((iContainerWidth / 2) - (iStringWidth / 2), y); // x,y position
		// <-- Mirv: Fixed hud message alignment

		surface()->DrawPrintText( m_Messages[i].pText, wcslen(m_Messages[i].pText) ); // print text
	}
	
	// Now retire any death notices that have expired
	RetireExpiredMessages();
}

//-----------------------------------------------------------------------------
// Purpose: This message handler may be better off elsewhere
//-----------------------------------------------------------------------------
void CHudGameMessage::RetireExpiredMessages( void )
{
	// Loop backwards because we might remove one
	int iSize = m_Messages.Size();
	for ( int i = iSize-1; i >= 0; i-- )
	{
		if ( m_Messages[i].flStartTime + m_Messages[i].flDuration < gpGlobals->curtime )
		{
			m_Messages.Remove(i);
		}
	}
}

Color CHudGameMessage::GetColor( int iColorID )
{
	switch (iColorID)
	{
		case LUA_COLOR_BLUE:
			return COLOR_BLUE;
			break;
		case LUA_COLOR_RED:
			return COLOR_RED;
			break;
		case LUA_COLOR_YELLOW:
			return COLOR_YELLOW;
			break;
		case LUA_COLOR_GREEN:
			return COLOR_GREEN;
			break;
		case LUA_COLOR_BLACK:
			return Color( 0,0,0 );
			break;
		case LUA_COLOR_ORANGE:
			return Color( 255,161,66 );
			break;
		case LUA_COLOR_PINK:
			return Color( 255,66,161 );
			break;
		case LUA_COLOR_PURPLE:
			return Color( 161,66,255 );
			break;
		case LUA_COLOR_GREY:
			return COLOR_GREY;
			break;
		case LUA_COLOR_DEFAULT:
		case LUA_COLOR_WHITE:
		case LUA_COLOR_INVALID:
		default:
			return Color( 255,255,255 );
			break;
	}
}