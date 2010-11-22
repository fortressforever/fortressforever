//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ff_hud_chat.h"
//#include "c_ff_player.h"
//#include "c_ff_playerresource.h"
#include "hud_macros.h"
#include "text_message.h"
#include "vguicenterprint.h"
#include "vgui/ILocalize.h"

#include <igameresources.h>	// |-- Mirv: Access to play details

bool g_fBlockedStatus[256] = { false };		// |-- Mirv: Hold whether these dudes are blocked

ConVar cl_showtextmsg( "cl_showtextmsg", "1", 0, "Enable/disable text messages printing on the screen." );

ConVar cl_chat_colorize( "cl_chat_colorize", "1", FCVAR_ARCHIVE, "Enable/disable text messages colorization." );
ConVar cl_chat_color_default( "cl_chat_color_default", "255 170 0", FCVAR_ARCHIVE, "Set the default chat text color(before colorization)." );

extern ConVar sv_specchat;

// Yar!
static CHudChat *g_pHudChat = NULL;

// --> Mirv: Colours!
Color g_ColorConsole(153, 255, 153, 255);
Color g_ColorOrange(255, 170, 0, 255);

Color GetDefaultChatColor()
{
	int r, g, b;
	if(sscanf( cl_chat_color_default.GetString(), "%i %i %i", &r, &g, &b ) != 3)
		return g_ColorOrange;
	
	r = clamp(r,0,255);
	g = clamp(g,0,255);
	b = clamp(b,0,255);
	return Color(r, g, b, 255);
}

Color GetClientColor( int clientIndex )
{
	if ( clientIndex == 0 ) // console msg
	{
		return g_ColorConsole;
	}
	else 
	{
		IGameResources *gr = GameResources();

		if (!gr )
			return GetDefaultChatColor();

		return gr->GetTeamColor( gr->GetTeam( clientIndex ) );
	}	
}
// <-- Mirv: Colours!

// Forward declare
class CHudChatLine;

// Dump text
void DumpBufToChatline( CHudChatLine *pChatLine, char *szText, int &iPos )
{
	wchar_t wszTemp[ 128 ];
	vgui::localize()->ConvertANSIToUnicode( szText, wszTemp, sizeof( wszTemp ) );
	pChatLine->InsertString( wszTemp );
	iPos = 0;
}

// converts all '\r' characters to '\n', so that the engine can deal with the properly
// returns a pointer to str
static char* ConvertCRtoNL( char *str )
{
	for ( char *ch = str; *ch != 0; ch++ )
		if ( *ch == '\r' )
			*ch = '\n';
	return str;
}

// converts all '\r' characters to '\n', so that the engine can deal with the properly
// returns a pointer to str
static wchar_t* ConvertCRtoNL( wchar_t *str )
{
	for ( wchar_t *ch = str; *ch != 0; ch++ )
		if ( *ch == L'\r' )
			*ch = L'\n';
	return str;
}

static void StripEndNewlineFromString( char *str )
{
	int s = strlen( str ) - 1;
	if ( s >= 0 )
	{
		if ( str[s] == '\n' || str[s] == '\r' )
			str[s] = 0;
	}
}

static void StripEndNewlineFromString( wchar_t *str )
{
	int s = wcslen( str ) - 1;
	if ( s >= 0 )
	{
		if ( str[s] == L'\n' || str[s] == L'\r' )
			str[s] = 0;
	}
}

DECLARE_HUDELEMENT( CHudChat );

DECLARE_HUD_MESSAGE( CHudChat, SayText );
DECLARE_HUD_MESSAGE( CHudChat, TextMsg );


//=====================
//CHudChatLine
//=====================

void CHudChatLine::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_hFont = pScheme->GetFont( "ChatFont" );
	SetBorder( NULL );
	SetBgColor( Color( 0, 0, 0, 0 ) );
	SetFgColor( Color( 0, 0, 0, 0 ) );

	SetFont( m_hFont );
}

void CHudChatLine::PerformFadeout( void )
{
	// Flash + Extra bright when new
	float curtime = gpGlobals->curtime;

	//int lr = m_clrText[0];
	//int lg = m_clrText[1];
	//int lb = m_clrText[2];
	
	//CSPort chat only fades out, no blinking.
	if ( curtime <= m_flExpireTime && curtime > m_flExpireTime - CHATLINE_FADE_TIME )
	{
		float frac = ( m_flExpireTime - curtime ) / CHATLINE_FADE_TIME;

		int alpha = frac * 255;
		alpha = clamp( alpha, 0, 255 );

		SetAlpha(alpha);

		// squeek: None of this is necessary just to change the alpha...
		// plus, this method removes custom colors while fading
		/*wchar_t wbuf[4096];
		
		GetText(0, wbuf, sizeof(wbuf));

		SetText( "" );
		
		if ( m_iNameLength > 0 )
		{
			wchar_t wText[4096];
			// draw the first x characters in the player color
			wcsncpy( wText, wbuf, min( m_iNameLength + 1, MAX_PLAYER_NAME_LENGTH+32) );
			wText[ min( m_iNameLength, MAX_PLAYER_NAME_LENGTH+31) ] = 0;

			m_clrNameColor[3] = alpha;

			InsertColorChange( m_clrNameColor );
			InsertString( wText );

			wcsncpy( wText, wbuf + ( m_iNameLength ), wcslen( wbuf + m_iNameLength ) );
			wText[ wcslen( wbuf + m_iNameLength ) ] = '\0';

			Color c = GetDefaultChatColor();
			InsertColorChange( Color( c[0], c[1], c[2], alpha ) );
			InsertString( wText );
			InvalidateLayout( true );
		}
		else
		{
			InsertColorChange( Color( lr, lg, lb, alpha ) );
			InsertString( wbuf );
		}
		*/
	}
	else
	{
		SetAlpha(255);
	}
	
	OnThink();
}



//=====================
//CHudChatInputLine
//=====================

void CHudChatInputLine::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	vgui::HFont hFont = pScheme->GetFont( "ChatFont" );

	m_pPrompt->SetFont( hFont );
	m_pInput->SetFont( hFont );

	m_pInput->SetFgColor( pScheme->GetColor( "Chat.TypingText", pScheme->GetColor( "Panel.FgColor", Color( 255, 255, 255, 255 ) ) ) );
}



//=====================
//CHudChat
//=====================

CHudChat::CHudChat( const char *pElementName ) : BaseClass( pElementName )
{
	
}

CHudChat::~CHudChat( void )
{
	g_pHudChat = NULL;
}

void CHudChat::CreateChatInputLine( void )
{
	m_pChatInput = new CHudChatInputLine( this, "ChatInputLine" );
	m_pChatInput->SetVisible( false );

	// Only the chat input line is a popup now.
	// This means that we can see the rest of the text behind the scoreboard.
	// When the chat input line goes up the scoreboard will be removed so the 
	// disparity is not obvious.
	m_pChatInput->MakePopup();
}

void CHudChat::CreateChatLines( void )
{
	for ( int i = 0; i < CHAT_INTERFACE_LINES; i++ )
	{
		char sz[ 32 ];
		Q_snprintf( sz, sizeof( sz ), "ChatLine%02i", i );
		m_ChatLines[ i ] = new CHudChatLine( this, sz );
		m_ChatLines[ i ]->SetVisible( false );		
	}
}

void CHudChat::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetBgColor( Color( 0, 0, 0, 0 ) );
	SetFgColor( Color( 0, 0, 0, 0 ) );
}


void CHudChat::Init( void )
{
	BaseClass::Init();

	HOOK_HUD_MESSAGE( CHudChat, SayText );
	HOOK_HUD_MESSAGE( CHudChat, TextMsg );

	g_pHudChat = this;
}

//-----------------------------------------------------------------------------
// Purpose: Overrides base reset to not cancel chat at round restart
//-----------------------------------------------------------------------------
void CHudChat::Reset( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pszName - 
//			iSize - 
//			*pbuf - 
//-----------------------------------------------------------------------------
void CHudChat::MsgFunc_SayText( bf_read &msg )
{
	char szString[256];

	int client = msg.ReadByte();

	// --> Mirv: Actually bud, you're blocked
	if( g_fBlockedStatus[client] )
		return;
	// <-- Mirv: Actually bud, you're blocked

	msg.ReadString( szString, sizeof(szString) );
	bool bWantsToChat = msg.ReadByte();

	IGameResources *gr = GameResources();

	if (gr && CBasePlayer::GetLocalPlayer())
	{
		bool bIsSpectator = gr->GetTeam(client) < TEAM_BLUE;
		bool bLocalPlayerSpectator = CBasePlayer::GetLocalPlayer()->GetTeamNumber() < TEAM_BLUE;

		// Don't block rcon messages
		if (client != 0 && !bLocalPlayerSpectator && bIsSpectator && !sv_specchat.GetBool())
			return;
	}

	if ( bWantsToChat )
	{
		// print raw chat text
		 ChatPrintf( client, "%s", szString );
	}
	else
	{
		// try to lookup translated string
		 Printf( "%s", hudtextmessage->LookupString( szString ) );
		 Msg( "%s", szString );
	}
}

wchar_t* ReadLocalizedRadioCommandString( bf_read &msg, wchar_t *pOut, int outSize, bool bStripNewline )
{
	char szString[2048];
	msg.ReadString( szString, sizeof(szString) );

	const wchar_t *pBuf = vgui::localize()->Find( szString );
	if ( pBuf )
	{
		wcsncpy( pOut, pBuf, outSize/sizeof(wchar_t) );
		pOut[outSize/sizeof(wchar_t)-1] = 0;
	}
	else
	{
		vgui::localize()->ConvertANSIToUnicode( szString, pOut, outSize );
	}

	if ( bStripNewline )
		StripEndNewlineFromString( pOut );

	return pOut;
}

// Message handler for text messages
// displays a string, looking them up from the titles.txt file, which can be localised
// parameters:
//   byte:   message direction  ( HUD_PRINTCONSOLE, HUD_PRINTNOTIFY, HUD_PRINTCENTER, HUD_PRINTTALK )
//   string: message
// optional parameters:
//   string: message parameter 1
//   string: message parameter 2
//   string: message parameter 3
//   string: message parameter 4
// any string that starts with the character '#' is a message name, and is used to look up the real message in titles.txt
// the next (optional) one to four strings are parameters for that string (which can also be message names if they begin with '#')
void CHudChat::MsgFunc_TextMsg( bf_read &msg )
{
	char szString[2048];
	int msg_dest = msg.ReadByte();
	
	wchar_t szBuf[5][128];
	wchar_t outputBuf[256];

	for ( int i=0; i<5; ++i )
	{
		msg.ReadString( szString, sizeof(szString) );
		char *tmpStr = hudtextmessage->LookupString( szString, &msg_dest );
		const wchar_t *pBuf = vgui::localize()->Find( tmpStr );
		if ( pBuf )
		{
			// Copy pBuf into szBuf[i].
			int nMaxChars = sizeof( szBuf[i] ) / sizeof( wchar_t );
			wcsncpy( szBuf[i], pBuf, nMaxChars );
			szBuf[i][nMaxChars-1] = 0;
		}
		else
		{
			if ( i )
			{
				StripEndNewlineFromString( tmpStr );  // these strings are meant for subsitution into the main strings, so cull the automatic end newlines
			}
			vgui::localize()->ConvertANSIToUnicode( tmpStr, szBuf[i], sizeof(szBuf[i]) );
		}
	}

	if ( !cl_showtextmsg.GetInt() )
		return;

	int len;
	switch ( msg_dest )
	{
	case HUD_PRINTCENTER:
		vgui::localize()->ConstructString( outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4] );
		internalCenterPrint->Print( ConvertCRtoNL( outputBuf ) );
		break;

	case HUD_PRINTNOTIFY:
		szString[0] = 1;  // mark this message to go into the notify buffer
		vgui::localize()->ConstructString( outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4] );
		vgui::localize()->ConvertUnicodeToANSI( outputBuf, szString+1, sizeof(szString)-1 );
		len = strlen( szString );
		if ( len && szString[len-1] != '\n' && szString[len-1] != '\r' )
		{
			Q_strncat( szString, "\n", sizeof(szString), 1 );
		}
		Msg( "%s", ConvertCRtoNL( szString ) );
		break;

	case HUD_PRINTTALK:
		vgui::localize()->ConstructString( outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4] );
		vgui::localize()->ConvertUnicodeToANSI( outputBuf, szString, sizeof(szString) );
		len = strlen( szString );
		if ( len && szString[len-1] != '\n' && szString[len-1] != '\r' )
		{
			Q_strncat( szString, "\n", sizeof(szString), 1 );
		}
		Printf( "%s", ConvertCRtoNL( szString ) );
		break;

	case HUD_PRINTCONSOLE:
		vgui::localize()->ConstructString( outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4] );
		vgui::localize()->ConvertUnicodeToANSI( outputBuf, szString, sizeof(szString) );
		len = strlen( szString );
		if ( len && szString[len-1] != '\n' && szString[len-1] != '\r' )
		{
			Q_strncat( szString, "\n", sizeof(szString), 1 );
		}
		Msg( "%s", ConvertCRtoNL( szString ) );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Formats the chat line for the client
// Input  : *fmt - 
//			... - 
//-----------------------------------------------------------------------------
void CHudChat::ChatPrintf( int iPlayerIndex, const char *fmt, ... )
{
	// hlstriker: Rewrote a lot here

	// Copy the message into szMsg
	va_list marker;
	char szMsg[ 128 ];

	va_start( marker, fmt );
	Q_vsnprintf( szMsg, sizeof( szMsg ), fmt, marker );
	va_end( marker );

	// Strip any trailing '\n'
	if ( strlen( szMsg ) > 0 && szMsg[ strlen( szMsg )-1 ] == '\n' )
		szMsg[ strlen( szMsg )-1 ] = 0;

	// Strip leading \n characters ( or notify/color signifiers )
	char *pMsg = szMsg;
	while ( *pMsg && ( *pMsg == '\n' || *pMsg == 1 || *pMsg == 2 ) )
		pMsg++;
	
	if ( !*pMsg )
		return;

	// Make sure there is an opened chat line
	CHudChatLine *line = (CHudChatLine *)FindUnusedChatLine();
	if ( !line )
	{
		ExpireOldest();
		line = (CHudChatLine *)FindUnusedChatLine();
	}

	if ( !line )
		return;

	// Start printing the message
	const int iBufferSize = strlen( pMsg ) + 1;
	char *buf = static_cast<char *>( _alloca( iBufferSize ) );

	if ( buf )
	{
		// Clear the chat line and set expire time
		line->SetText( "" );
		line->SetExpireTime();
		
		// Get the players name
		player_info_t sPlayerInfo;
		if ( iPlayerIndex == 0 )
		{
			Q_memset( &sPlayerInfo, 0, sizeof( player_info_t ) );
			Q_strncpy( sPlayerInfo.name, "Console", sizeof( sPlayerInfo.name ) );	
		}
		else
			engine->GetPlayerInfo( iPlayerIndex, &sPlayerInfo );

		const char *pName = sPlayerInfo.name;

		// If the players name is in the string draw it
		int iPos = 0;
		int iNameLength = 0;

		if ( pName )
		{
			const char *nameInString = strstr( pMsg, pName );
			if ( nameInString )
			{
				iNameLength = strlen( pName ) + ( nameInString - pMsg );

				// Go ahead and play the correct "chat beep" sound.
				// These strings will be the same length if the "(TEAM)" prefix is missing
				CLocalPlayerFilter filter;
				if ( strlen( pMsg ) == strlen( nameInString ) )
					C_BaseEntity::EmitSound( filter, -1, "HudChat.Message" );
				else
					C_BaseEntity::EmitSound( filter, -1, "HudChat.TeamMessage" );

				// Draw the players name and set names color
				line->InsertColorChange( GetClientColor( iPlayerIndex ) );

				Q_strncpy( buf, pMsg, min( iNameLength + 1, MAX_PLAYER_NAME_LENGTH + 32 ) );
				buf[ min( iNameLength, MAX_PLAYER_NAME_LENGTH + 31) ] = 0;
				DumpBufToChatline( line, buf, iPos );
				Q_strncpy( buf, pMsg + iNameLength, strlen( pMsg ));
				buf[ strlen( pMsg + iNameLength ) ] = '\0';

				line->SetNameLength( iNameLength );
				line->SetNameColor( GetClientColor( iPlayerIndex ) ); // Only for fade out
			}
			else
			{
				Q_strncpy( buf, pMsg, strlen( pMsg ) + 1);
				buf[ strlen( pMsg ) + 1 ] = '\0';
				line->SetNameLength( 0 );
				// Name not found, still need to play "chat beep"
				CLocalPlayerFilter filter;
				C_BaseEntity::EmitSound( filter, -1, "HudChat.Message" );
			}
		}
		else
		{
			Q_strncpy( buf, pMsg, strlen( pMsg ) + 1);
			buf[ strlen( pMsg ) + 1 ] = '\0';
			line->SetNameLength( 0 );
			// No name at all, still need to play "chat beep"
			CLocalPlayerFilter filter;
			C_BaseEntity::EmitSound( filter, -1, "HudChat.Message" );
		}

		// Draw the rest of the message
		line->InsertColorChange( GetDefaultChatColor() );

		char *pBeg = buf;
		int iAdjust = 1;
		char szTemp[ 256 ];
		int iTotalLen = iNameLength;

		while( pBeg[ 0 ] )
		{
			// Change localized strings to resource strings
			if( ( pBeg[ 0 ] == '#' ) && pBeg[ 1 ] )
			{
				// Dump what's already on the buffer
				if( iPos )
					DumpBufToChatline( line, szTemp, iPos );

				// Create the localized string
				int i = 1;
				while( ( ( pBeg[ i ] >= 'A' ) && ( pBeg[ i ] <= 'Z' ) ) || 
					( ( pBeg[ i ] >= 'a' ) && ( pBeg[ i ] <= 'z' ) ) ||
					( pBeg[ i ] == '_' ) )
					i++;

				if( ( iTotalLen + i ) > 127 )
					Q_strncpy( szTemp, pBeg, i - ( ( iTotalLen + i ) - 127 ) + 1 );
				else
					Q_strncpy( szTemp, pBeg, i + 1 );

				// Create and dump the resource string
				wchar_t *pszTemp = vgui::localize()->Find( szTemp );
				if( pszTemp )
				{
					// Found resource string, check if it's length goes past cap
					wchar_t wszTemp[ 130 ];
					Q_wcsncpy( wszTemp, pszTemp, sizeof( wszTemp ) );

					int j = Q_wcslen( wszTemp );
					if( ( iTotalLen + j ) > 127 )
					{
						//int iOverBy = j - ( ( iTotalLen + j ) - 127 );
						wszTemp[ ( j - ( ( iTotalLen + j ) - 127 ) ) ] = '\0';
					}

					line->InsertString( wszTemp );
					iTotalLen = iTotalLen + ( j - i );
				}
				else
					DumpBufToChatline( line, szTemp, iPos );

				iAdjust = i;
			}
			// Change color codes into color
			else if( pBeg[ 0 ] == '^' )
			{
				if( pBeg[ 1 ] >= '0' && pBeg[ 1 ] <= '9' )
				{
					// If there's stuff in our buffer, dump it first
					if( iPos )
						DumpBufToChatline( line, szTemp, iPos );

					enum MarkupColors
					{
						COL_ORANGE,
						COL_BLUE,
						COL_RED,
						COL_YELLOW,
						COL_GREEN,
						COL_WHITE,
						COL_BLACK,
						COL_GREY,
						COL_MAGENTA,
						COL_CYAN,
						NUM_COLORS,
					};

					// hlstriker: Adjusted colors for better visibility
					Color colorMarkup[ NUM_COLORS ];
					colorMarkup[ COL_WHITE ] = Color( 255, 255, 255, 255 );
					colorMarkup[ COL_BLACK ] = Color( 20, 20, 20, 255 );
					colorMarkup[ COL_GREY ] = Color( 188, 188, 188, 255 );
					colorMarkup[ COL_BLUE ] = Color( 60, 60, 255, 255 );
					colorMarkup[ COL_RED ] = Color( 255, 60, 60, 255 );
					colorMarkup[ COL_YELLOW ] = Color( 248, 248, 0, 255 );
					colorMarkup[ COL_GREEN ] = Color( 40, 245, 40, 255 );
					colorMarkup[ COL_MAGENTA ] = Color( 139, 45, 139, 255 );
					colorMarkup[ COL_CYAN ] = Color( 0, 255, 255, 255 );
					colorMarkup[ COL_ORANGE ] = Color( 226, 118, 10, 255 );

					char col[ 2 ];
					col[ 0 ] = pBeg[ 1 ];
					col[ 1 ] = NULL;
					int c = atoi( col );
					Color colo = colorMarkup[ COL_WHITE ];
					if( c >= 0 && c < NUM_COLORS )
						colo = colorMarkup[ c ];

					if( !cl_chat_colorize.GetBool() )
						colo = GetDefaultChatColor();

					line->InsertColorChange( colo );

					iAdjust = 2;
				}
				else
				{
					// If there's stuff in our buffer, dump it first
					if( iPos )
						DumpBufToChatline( line, szTemp, iPos );

					// Just a ^ with no number after terminates the color
					line->InsertColorChange( GetDefaultChatColor() );
					iAdjust = 1;
				}
			}
			// Handle regular characters
			else
			{
				// Add a character to our buffer
				szTemp[ iPos++ ] = pBeg[ 0 ];
				if( iPos == 128 )
					szTemp[ --iPos ] = '\0';
				else
					szTemp[ iPos ] = '\0';
				iAdjust = 1;
			}

			pBeg += iAdjust;

			// Break if passed cap length
			iTotalLen += iAdjust;
			if( iTotalLen > ( 127 - 1 ) )
				break;
		}

		// Dump any text that's left on buffer
		if( iPos )
			DumpBufToChatline( line, szTemp, iPos );

		// Show the message to the client
		line->SetVisible( true );

		// Show message in console
		line->GetText( 0, szTemp, sizeof( szTemp ) );
		Msg( "%s\n", szTemp );
	}
}

int CHudChat::GetChatInputOffset( void )
{
	// For now lets not shift text around when we're writing.
	// It is a bit jumpy and doesn't look so good.
	return m_iFontHeight;

	if ( m_pChatInput->IsVisible() )
	{
		return m_iFontHeight;
	}
	else
		return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Does a ClientPrint message but from the client (didn't want to
//			overwrite the client version which does... nothing, literally)
//-----------------------------------------------------------------------------
void ClientPrintMsg( C_BasePlayer *player, int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4 )
{
	if( g_pHudChat )
	{
		char szString[2048];

		wchar_t szBuf[5][128];
		wchar_t outputBuf[256];

		for ( int i=0; i<5; ++i )
		{
			switch( i )
			{
			case 0: Q_snprintf( szString, sizeof( szString ), "%s", msg_name ); break;
			case 1: Q_snprintf( szString, sizeof( szString ), "%s", param1 ); break;
			case 2: Q_snprintf( szString, sizeof( szString ), "%s", param2 ); break;
			case 3: Q_snprintf( szString, sizeof( szString ), "%s", param3 ); break;
			case 4: Q_snprintf( szString, sizeof( szString ), "%s", param4 ); break;
			}
			
			char *tmpStr = hudtextmessage->LookupString( szString, &msg_dest );
			const wchar_t *pBuf = vgui::localize()->Find( tmpStr );
			if ( pBuf )
			{
				// Copy pBuf into szBuf[i].
				int nMaxChars = sizeof( szBuf[i] ) / sizeof( wchar_t );
				wcsncpy( szBuf[i], pBuf, nMaxChars );
				szBuf[i][nMaxChars-1] = 0;
			}
			else
			{
				if ( i )
				{
					StripEndNewlineFromString( tmpStr );  // these strings are meant for subsitution into the main strings, so cull the automatic end newlines
				}
				vgui::localize()->ConvertANSIToUnicode( tmpStr, szBuf[i], sizeof(szBuf[i]) );
			}
		}

		if ( !cl_showtextmsg.GetInt() )
			return;

		//int len;
		switch ( msg_dest )
		{
		case HUD_PRINTCENTER:
			vgui::localize()->ConstructString( outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4] );
			internalCenterPrint->Print( ConvertCRtoNL( outputBuf ) );
			break;

			/*
		case HUD_PRINTNOTIFY:
			szString[0] = 1;  // mark this message to go into the notify buffer
			vgui::localize()->ConstructString( outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4] );
			vgui::localize()->ConvertUnicodeToANSI( outputBuf, szString+1, sizeof(szString)-1 );
			len = strlen( szString );
			if ( len && szString[len-1] != '\n' && szString[len-1] != '\r' )
			{
				Q_strncat( szString, "\n", sizeof(szString), 1 );
			}
			Msg( "%s", ConvertCRtoNL( szString ) );
			break;

		case HUD_PRINTTALK:
			vgui::localize()->ConstructString( outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4] );
			vgui::localize()->ConvertUnicodeToANSI( outputBuf, szString, sizeof(szString) );
			len = strlen( szString );
			if ( len && szString[len-1] != '\n' && szString[len-1] != '\r' )
			{
				Q_strncat( szString, "\n", sizeof(szString), 1 );
			}
			g_pHudChat->Printf( "%s", ConvertCRtoNL( szString ) );
			break;

		case HUD_PRINTCONSOLE:
			vgui::localize()->ConstructString( outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4] );
			vgui::localize()->ConvertUnicodeToANSI( outputBuf, szString, sizeof(szString) );
			len = strlen( szString );
			if ( len && szString[len-1] != '\n' && szString[len-1] != '\r' )
			{
				Q_strncat( szString, "\n", sizeof(szString), 1 );
			}
			Msg( "%s", ConvertCRtoNL( szString ) );
			break;
			*/
		}
	}
}
