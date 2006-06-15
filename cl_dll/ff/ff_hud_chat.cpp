//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
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

// --> Mirv: Colours!
int g_ColorConsole[3]	= { 153, 255, 153 };
int g_ColorOrange[3]	= { 255, 170, 0 };

int *GetClientColor( int clientIndex )
{
	static int iColor[3];

	if ( clientIndex == 0 ) // console msg
	{
		return g_ColorConsole;
	}
	else 
	{
		IGameResources *gr = GameResources();

		if (!gr )
			return g_ColorOrange;

		Color col = gr->GetTeamColor( gr->GetTeam( clientIndex ) );

		int alpha = 0;
		col.GetColor(iColor[0], iColor[1], iColor[2], alpha);

		return iColor;
	}	
}
// <-- Mirv: Colours!

// Forward declare
class CHudChatLine;

// Dump text
void DumpBufToChatline( CHudChatLine *pChatLine, char *szText, int &iPos )
{
	wchar_t wszTemp[ 4096 ];
	vgui::localize()->ConvertANSIToUnicode( szText, wszTemp, sizeof( wszTemp ) );
	pChatLine->InsertString( wszTemp );
	Q_strcpy( szText, "\0" );
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

	int lr = m_clrText[0];
	int lg = m_clrText[1];
	int lb = m_clrText[2];
	
	//CSPort chat only fades out, no blinking.
	if ( curtime <= m_flExpireTime && curtime > m_flExpireTime - CHATLINE_FADE_TIME )
	{
		float frac = ( m_flExpireTime - curtime ) / CHATLINE_FADE_TIME;

		int alpha = frac * 255;
		alpha = clamp( alpha, 0, 255 );

		wchar_t wbuf[4096];
		
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
			InsertColorChange( Color( g_ColorOrange[0], g_ColorOrange[1], g_ColorOrange[2], alpha ) );
			InsertString( wText );
			InvalidateLayout( true );
		}
		else
		{
			InsertColorChange( Color( lr, lg, lb, alpha ) );
			InsertString( wbuf );
		}
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

void CHudChat::CreateChatInputLine( void )
{
	m_pChatInput = new CHudChatInputLine( this, "ChatInputLine" );
	m_pChatInput->SetVisible( false );
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
	
	if ( bWantsToChat )
	{
		// print raw chat text
		 ChatPrintf( client, "%s", szString );
	}
	else
	{
		// try to lookup translated string
		 Printf( "%s", hudtextmessage->LookupString( szString ) );
	}

	Msg( "%s", szString );
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
// Purpose: 
// Input  : *fmt - 
//			... - 
//-----------------------------------------------------------------------------
void CHudChat::ChatPrintf( int iPlayerIndex, const char *fmt, ... )
{
	va_list marker;
	char msg[4096];

	va_start(marker, fmt);
	Q_vsnprintf(msg, sizeof( msg), fmt, marker);
	va_end(marker);

	// Strip any trailing '\n'
	if ( strlen( msg ) > 0 && msg[ strlen( msg )-1 ] == '\n' )
	{
		msg[ strlen( msg ) - 1 ] = 0;
	}

	// Strip leading \n characters ( or notify/color signifiers )
	char *pmsg = msg;
	while ( *pmsg && ( *pmsg == '\n' || *pmsg == 1 || *pmsg == 2 ) )
	{
		pmsg++;
	}
	
	if ( !*pmsg )
		return;

	CHudChatLine *line = (CHudChatLine *)FindUnusedChatLine();
	if ( !line )
	{
		ExpireOldest();
		line = (CHudChatLine *)FindUnusedChatLine();
	}

	if ( !line )
	{
		return;
	}

	line->SetText( "" );
	
	int iNameLength = 0;
	
	player_info_t sPlayerInfo;
	if ( iPlayerIndex == 0 )
	{
		Q_memset( &sPlayerInfo, 0, sizeof(player_info_t) );
		Q_strncpy( sPlayerInfo.name, "Console", sizeof(sPlayerInfo.name)  );	
	}
	else
	{
		engine->GetPlayerInfo( iPlayerIndex, &sPlayerInfo );
	}	

	const char *pName = sPlayerInfo.name;

	if ( pName )
	{
		const char *nameInString = strstr( pmsg, pName );

		if ( nameInString )
		{
			iNameLength = strlen( pName ) + (nameInString - pmsg);
		}
	}
	else
		line->InsertColorChange( Color( g_ColorOrange[0], g_ColorOrange[1], g_ColorOrange[2], 255 ) );

	char *buf = static_cast<char *>( _alloca( strlen( pmsg ) + 1  ) );
	wchar_t *wbuf = static_cast<wchar_t *>( _alloca( (strlen( pmsg ) + 1 ) * sizeof(wchar_t) ) );
	if ( buf )
	{
		int *iColor = GetClientColor( iPlayerIndex );

		line->SetExpireTime();
	
		// draw the first x characters in the player color
		Q_strncpy( buf, pmsg, min( iNameLength + 1, MAX_PLAYER_NAME_LENGTH+32) );
		buf[ min( iNameLength, MAX_PLAYER_NAME_LENGTH+31) ] = 0;
		line->InsertColorChange( Color( iColor[0], iColor[1], iColor[2], 255 ) );
		line->InsertString( buf );
		Q_strncpy( buf, pmsg + iNameLength, strlen( pmsg ));
		buf[ strlen( pmsg + iNameLength ) ] = '\0';
		line->InsertColorChange( Color( g_ColorOrange[0], g_ColorOrange[1], g_ColorOrange[2], 255 ) );

		// Want to look in buf for any localized strings
		// and convert them to resource strings if possible
		char *pBeg = buf;
		int iAdjust = 1;
		char szTemp[ 4096 ];

		Q_strcpy( szTemp, "\0" );
		int iPos = 0;

		while( pBeg[ 0 ] )
		{
			iAdjust = 1;

			// Found a resource string to localize
			if( ( pBeg[ 0 ] == '#' ) && pBeg[ 1 ] )
			{
				// If there's stuff in our buffer, dump it first
				if( iPos )
					DumpBufToChatline( line, szTemp, iPos );

				// Now get on with tokenizing
				int i = 1;

				while( ( ( pBeg[ i ] >= 'A' ) && ( pBeg[ i ] <= 'Z' ) ) || 
					( ( pBeg[ i ] >= 'a' ) && ( pBeg[ i ] <= 'z' ) ) ||
					( pBeg[ i ] == '_' ) )
					i++;

				char szToken[ 1024 ];
				Q_strncpy( szToken, pBeg, i + 1 );

				wchar_t *pszTemp = vgui::localize()->Find( szToken );
				if( pszTemp )
					line->InsertString( pszTemp );
				else
				{
					vgui::localize()->ConvertANSIToUnicode( szToken, wbuf, sizeof( wbuf ) );
					line->InsertString( wbuf );
				}

				iAdjust = i;
			}
			// Regular characters
			else
			{
				// Add a character to our buffer
				char ch = pBeg[ 0 ];
				szTemp[ iPos++ ] = ch;
				if( iPos == 4096 )
					szTemp[ --iPos ] = '\0';
				else
					szTemp[ iPos ] = '\0';
			}

			pBeg += iAdjust;
		}

		// If there's stuff still in our buffer, dump it
		if( iPos )
			DumpBufToChatline( line, szTemp, iPos );

		line->SetVisible( true );
		line->SetNameLength( iNameLength );
		line->SetNameColor( Color( iColor[0], iColor[1], iColor[2], 255 ) );
	}

	CLocalPlayerFilter filter;
	C_BaseEntity::EmitSound( filter, -1 /*SOUND_FROM_LOCAL_PLAYER*/, "HudChat.Message" );
}

int CHudChat::GetChatInputOffset( void )
{
	if ( m_pChatInput->IsVisible() )
	{
		return m_iFontHeight;
	}
	else
		return 0;
}
