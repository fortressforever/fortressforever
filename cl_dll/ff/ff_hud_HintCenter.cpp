//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_HintCenter.cpp
//	@author Christopher Boylan (Jiggles)
//	@date 2/8/2007
//	@brief HUD Hint/Message display center
//
//	REVISIONS
//	---------
//	2/08/2007, Jiggles:
//		First created - based significantly off of Mulch's ff_hud_hint.
//	8/11/2007, Jiggles:
//		Maaaany changes

#include "cbase.h"
#include "ff_hud_HintCenter.h"

#include "hudelement.h"
#include "hud_macros.h"
#include "mathlib.h"

#include "KeyValues.h"
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>

#include <vgui_controls/RichText.h>
#include <vgui_controls/Button.h>

#include "c_ff_player.h"
#include "ff_utils.h"

#include "IGameUIFuncs.h" // for key bindings
extern IGameUIFuncs *gameuifuncs; // for key binding details

#include <vgui/ILocalize.h>

using namespace vgui;


// Global
CHudHintCenter *g_pHintCenter = NULL;
// Global Helper var
CHudHintCenter *g_pHintHelper = NULL;

ConVar hudhints( "cl_hints", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_USERINFO, "Display hints" );


DECLARE_HUDELEMENT( CHudHintCenter );
DECLARE_HUD_MESSAGE( CHudHintCenter, FF_SendHint );



//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHintCenter::CHudHintCenter( const char *pElementName ) : CHudElement( pElementName ), vgui::Frame( NULL, "HudHintCenter" ) 
{
	SetParent( g_pClientMode->GetViewport() );
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED );
	
	m_pRichText = NULL;

	// initialize dialog
	SetProportional(true);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(true);
	SetSizeable(false);
	SetMoveable(false);
	// hide the system buttons
	SetTitleBarVisible( false );

	m_iCurrentHintIndex = -1;
	m_iLastHintID = -1;


}



//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
CHudHintCenter::~CHudHintCenter( void )
{
	if( m_pHudIcon )
	{
		delete m_pHudIcon;
		m_pHudIcon = NULL;
	}
	if( m_pHudIconGlow )
	{
		delete m_pHudIconGlow;
		m_pHudIconGlow = NULL;
	}

	if( m_pNextHintButton )
	{
		delete m_pNextHintButton;
		m_pNextHintButton = NULL;
	}
	if( m_pPreviousHintButton )
	{
		delete m_pPreviousHintButton;
		m_pPreviousHintButton = NULL;
	}

	g_pHintHelper = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintCenter::Init( void )
{
	HOOK_HUD_MESSAGE( CHudHintCenter, FF_SendHint );
}



//-----------------------------------------------------------------------------
// Purpose: Adds a hint to the hint box
//-----------------------------------------------------------------------------
void CHudHintCenter::AddHudHint( unsigned short hintID, short NumShow, short hintPriority, const char *pszMessage )
{
	// Hints are off -- ignore the hint
	if( !hudhints.GetBool() )
		return;

	// Ignore hints that are less important than the one currently showing
	if ( hintPriority < m_iLastHintPriority )
	{  
		if ( gpGlobals->curtime < m_flLastHintDuration  )
			return;
	}
	// Also ignore equally-important hints that come in too "quickly"
	else if ( hintPriority == m_iLastHintPriority )
	{
		//	This gives the previous hint 5 seconds to show before it might be "preempted"
		if ( gpGlobals->curtime < ( m_flLastHintDuration - HINTCENTER_FADEOUT_TIME - 5.0f ) )
			return;
	}
	
	// Now, let's see if this hint has been shown yet
	int foundIndex = -1;
	if ( hintID ) // Zero is reserved for MAP hints, which we're gonna show all the time anyway
	{
		HintInfo structHintInfo( hintID, NumShow );
		foundIndex = m_HintVector.Find( structHintInfo );
		if (  foundIndex < 0 )  // Hint not shown yet
		{
			if ( structHintInfo.m_ShowCount != -1 ) // -1 is reserved for "infinite" hints
				structHintInfo.m_ShowCount--;
			m_HintVector.AddToTail( structHintInfo );
		}
		else if ( m_HintVector[ foundIndex ].m_ShowCount > 0 )
			m_HintVector[ foundIndex ].m_ShowCount--;
		else if (  m_HintVector[ foundIndex ].m_ShowCount != -1 )  // Hint has been shown enough times -- ignore it
			return;
	}
	m_iLastHintPriority = hintPriority;
	m_flLastHintDuration = gpGlobals->curtime + HINTCENTER_TIMEOUT_THRESHOLD + HINTCENTER_FADEOUT_TIME;
	
	// Save some of the old hints -- note that this will cut them off at an arbitrary point
	//char oldHintString[HINT_HISTORY];
	//m_pRichText->GetText( 0, oldHintString, sizeof( oldHintString ) ); 
	wchar_t *pszTemp = vgui::localize()->Find( pszMessage );
	wchar_t szUnlocalizedStr[4096];
	if( !pszTemp )
	{
		vgui::localize()->ConvertANSIToUnicode( pszMessage, szUnlocalizedStr, 512 );
		pszTemp = szUnlocalizedStr;
	}
	m_pRichText->SetText( "" );
	TranslateKeyCommand( pszTemp );

	// We don't want multiple copies of the same hint in a row
	// But we will allow this for map hints, because each map hint could be different
	if ( hintID != m_iLastHintID || !hintID )
		m_HintStringsVector.AddToTail( pszTemp );
	if ( m_HintStringsVector.Count() > HINT_HISTORY ) 
		m_HintStringsVector.Remove( 0 );  // Remove the oldest hint

	m_iCurrentHintIndex = m_HintStringsVector.Count() - 1;
	m_iLastHintID = hintID;	

	// There's probably a better way to do this
	// Create the "current # / total #" string, then convert it to Unicode so Surface() can have its way with it :)
	char szHintCtr[10];
	Q_snprintf( szHintCtr, sizeof( szHintCtr ), "%i/%i", m_iCurrentHintIndex + 1, m_HintStringsVector.Count() );
	vgui::localize()->ConvertANSIToUnicode( szHintCtr, m_szHintCounter, sizeof(m_szHintCounter) );

	if (  foundIndex < 0 || m_bHintKeyHeld )  // Hint hasn't been shown yet or user is holding down the hint key
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "OpenHintCenter" );

		// Play the hint sound
		CLocalPlayerFilter filter;
		C_BaseEntity::EmitSound( filter, -1, "Player.Hint" );
	}
	else  // Just show the hint icon
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "OpenHintCenterIcon" );
	
	m_bFadingOut = false;
	m_bHintCenterVisible = true;
	m_flSelectionTime = gpGlobals->curtime;

	Activate();
	SetMouseInputEnabled(false);
}


//-----------------------------------------------------------------------------
// Purpose: Receives a server-side hint
//-----------------------------------------------------------------------------
void CHudHintCenter::MsgFunc_FF_SendHint( bf_read &msg )
{

	// Hints are off -- ignore the hint
	if( !hudhints.GetBool() )
		return;

	// Buffer
	char szString[ 4096 ];
	//char szSound[ 4096 ];

	// Hint type [general, map]
	//byte bType = msg.ReadByte();

	// Hint id
	unsigned short wID = msg.ReadWord();

	// # of times to show the hint
	short NumToShow = msg.ReadShort();

	// Priority of the hint
	short HintImportance = msg.ReadShort();

	// Grab the string up to newline
	if( !msg.ReadString( szString, sizeof( szString ), true ) )
	{
		Warning( "[Hud Hint] String larger than buffer - not parsing!\n" );

		return;
	}

	//if( !msg.ReadString( szSound, sizeof( szSound ), true ) )
	//	Warning( "[Hud Hint] Sound path larger than buffer - ignoring sound!\n" );

	// Pass the string along
	//AddHudHint(bType, wID, szString, szSound[0] == 0 ? NULL : szSound);	
	//vgui::localize()->ConvertANSIToUnicode( szString, m_pText, sizeof( m_pText ) );
	
	AddHudHint( wID, NumToShow, HintImportance, szString );

}




//-------------------------------------------------------------------------------
// Purpose: Translate key commands into user's current key bind, and add the 
//			hint text to the text box. 
//			-For example, if the hint string says {+duck} and the user has the
//			 duck command bound to the SHIFT key, this function translates the 
//			 hint string into SHIFT.
//-------------------------------------------------------------------------------
bool CHudHintCenter::TranslateKeyCommand( wchar_t *psHintMessage )
{
	if ( !psHintMessage )
	{
		Warning( "Error: Received a hint message that was formatted incorrectly!\n" );
		return false;
	}

	int i = 0;
	for (;;)
	{
		if ( psHintMessage[i] == '{' )
		{
			m_pRichText->InsertChar( ' ' );
			
			char sKeyCommand[30];
			int iKeyIndex = 0;
			for (;;)
			{
				i++;


				// This shouldn't happen unless someone used the {} incorrectly
				if ( psHintMessage[i] == '\0' || iKeyIndex > 30 )
				{
					Warning( "Error: Received a hint message that was formatted incorrectly!\n" );
					return false;
				}
				// We've got the whole command -- now find out what key it's bound to
				if ( psHintMessage[i] == '}' )
				{
					sKeyCommand[iKeyIndex] = '\0';
					//Msg( "\nCommand: %s\n", sKeyCommand );
					const char *sConvertedCommand = gameuifuncs->Key_NameForKey( gameuifuncs->GetEngineKeyCodeForBind( sKeyCommand ) );
					//Msg( "\nConverted Command: %s\n", sConvertedCommand );
					m_pRichText->InsertString( sConvertedCommand );
					m_pRichText->InsertChar( ' ' );
					i++;
					break;
				}
				//Msg( "\nChar: %c\n", psHintMessage[i] );
				sKeyCommand[iKeyIndex] = psHintMessage[i];
				iKeyIndex++;

			}
		}

		// We're done!
		if ( psHintMessage[i] == '\0' )
			return true;

		m_pRichText->InsertChar( psHintMessage[i] );
		i++;
	}

}


//-----------------------------------------------------------------------------
// Purpose: Resizes the text box and buttons if the user changes resolutions
//-----------------------------------------------------------------------------
void CHudHintCenter::PerformLayout()
{
	BaseClass::PerformLayout();

	if ( m_pRichText )
	{
		m_pRichText->SetPos( text1_xpos, text1_ypos );
		m_pRichText->SetSize( text1_wide, text1_tall );
	}

	if( m_pNextHintButton )
	{
		m_pNextHintButton->SetPos( NextB_xpos, NextB_ypos );
		m_pNextHintButton->SetSize( B_wide, B_tall );
	}

	if( m_pPreviousHintButton )
	{
		m_pPreviousHintButton->SetPos( PrevB_xpos, PrevB_ypos );
		m_pPreviousHintButton->SetSize( B_wide, B_tall );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Called each map load
//-----------------------------------------------------------------------------
void CHudHintCenter::VidInit( void )
{

	// Point our helper to us
	g_pHintHelper = this;
	
	// Menu initially not visible
	m_bHintCenterVisible = false;

	m_bHintKeyHeld = false;

	// Set global pointer to this hud element
	g_pHintCenter = this;

	// Don't draw a background
	//SetPaintBackgroundEnabled( false );

	// Deallocate
	//if( m_pHudElementTexture )
	//{
	//	delete m_pHudElementTexture;
	//	m_pHudElementTexture = NULL;
	//}

	m_pHudIcon = new CHudTexture();
	m_pHudIcon->bRenderUsingFont = true;
	m_pHudIcon->hFont = m_hIconFont;
	m_pHudIcon->cCharacterInFont = 'G';

	m_pHudIconGlow = new CHudTexture();
	m_pHudIconGlow->bRenderUsingFont = true;
	m_pHudIconGlow->hFont = m_hIconFontGlow;
	m_pHudIconGlow->cCharacterInFont = 'G';



	 //Set up the rich text box that will contain the hud hint stuff
	if (!m_pRichText)
	{
		m_pRichText = new RichText(this, "HudHintText");
		m_pRichText->SetVerticalScrollbar( false );
		m_pRichText->SetPos( text1_xpos, text1_ypos );
		m_pRichText->SetSize( text1_wide, text1_tall );

		//m_pRichText->SetBorder( NULL );
		//m_pRichText->InsertColorChange( m_TextColor );
		//m_pRichText->SetFont( m_hTextFont );
		//m_pRichText->SetFgColor( Color(0, 0, 0, 0) );
		m_pRichText->SetPaintBorderEnabled( false );
		//m_pRichText->SetMaximumCharCount( 1024 );
	
	}

	// Set up the next/previous hint buttons
	if( !m_pNextHintButton )
	{
		m_pNextHintButton = new Button(this, "NextButton", ">>", this, "Next");
		m_pNextHintButton->SetPos( NextB_xpos, NextB_ypos );
		m_pNextHintButton->SetSize( B_wide, B_tall );
		m_pNextHintButton->SetPaintBorderEnabled( false );
	}
	if( !m_pPreviousHintButton )
	{
		m_pPreviousHintButton = new Button(this, "PrevButton", "<<", this, "Prev");
		m_pPreviousHintButton->SetPos( PrevB_xpos, PrevB_ypos );
		m_pPreviousHintButton->SetSize( B_wide, B_tall );
		m_pPreviousHintButton->SetPaintBorderEnabled( false );
	}

	m_flLastHintDuration = 0.0f;
	m_iLastHintPriority = 0;
}





//-----------------------------------------------------------------------------
// Purpose: updates animation status
//-----------------------------------------------------------------------------
void CHudHintCenter::OnThink( void )
{
	// Time out after awhile of inactivity
	if ( ( ( gpGlobals->curtime - m_flSelectionTime ) > HINTCENTER_TIMEOUT_THRESHOLD ) && !m_bHintKeyHeld )
	{
		if (!m_bFadingOut)
		{
			// start fading out
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "FadeOutHintCenter" );
			m_bFadingOut = true;
		}
		else if (gpGlobals->curtime - m_flSelectionTime > HINTCENTER_TIMEOUT_THRESHOLD + HINTCENTER_FADEOUT_TIME)
		{
			// finished fade, close
			HideSelection();
		}
	}
	else if (m_bFadingOut)
	{
		// stop us fading out, show the animation again
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "OpenHintCenter" );
		m_bFadingOut = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Opens hint center
//-----------------------------------------------------------------------------
void CHudHintCenter::OpenSelection( void )
{
	//Assert(!IsInSelectionMode());

	//CBaseHudWeaponSelection::OpenSelection();
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("OpenHintCenter");
}

//-----------------------------------------------------------------------------
// Purpose: Closes hint center immediately
//-----------------------------------------------------------------------------
void CHudHintCenter::HideSelection( void )
{
	//CBaseHudWeaponSelection::HideSelection();
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("CloseHintCenter");
	m_bFadingOut = false;
	m_bHintCenterVisible = false;
	OnClose();
	//m_pRichText->SetVerticalScrollbar(false);
}



//-----------------------------------------------------------------------------
// Purpose: Should the hud element draw or not?
//-----------------------------------------------------------------------------
bool CHudHintCenter::ShouldDraw( void )
{
	if( !CHudElement::ShouldDraw() )
		return false;

	if( !m_bHintCenterVisible )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Draw
//-----------------------------------------------------------------------------
void CHudHintCenter::Paint( void )
{

	// Draw!
	if( m_pHudIcon && m_pHudIconGlow )
	{
		
		DrawBox( 0, 0, GetWide(), GetTall(), m_BGBoxColor, m_flSelectionAlphaOverride / 255.0f );
		DrawHollowBox( 0, 0, GetWide(), GetTall(), m_TextColor, m_flSelectionAlphaOverride / 255.0f );
		
		//C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
		//Color clr = pPlayer->GetTeamColor();
		//Color clr = GetFgColor();

		// Draw the icon & "glow"
		m_pHudIconGlow->DrawSelf( image1_xpos, image1_ypos, m_TextColor );
		m_pHudIcon->DrawSelf( image1_xpos, image1_ypos, m_TextColor );

		//Look up the resource string
		//wchar_t *pszText = vgui::localize()->Find( Class_IntToResourceString( pPlayer->GetDisguisedClass() ) );

		// Draw hint counter
		surface()->DrawSetTextFont( m_hTextFont );
		surface()->DrawSetTextColor( m_TextColor );
		surface()->DrawSetTextPos( index_xpos, index_ypos );
		surface()->DrawUnicodeString( m_szHintCounter );
	}

}

//-----------------------------------------------------------------------------
// Purpose: User is pressing the hint key
//-----------------------------------------------------------------------------
void CHudHintCenter::KeyDown( void )
{
	// stop us fading out, show the animation again
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "OpenHintCenter" );
	m_bFadingOut = false;

	m_bHintKeyHeld = true;
	m_bHintCenterVisible = true;
	m_flSelectionTime = gpGlobals->curtime;
	//Msg( "\nKEY DOWN!!!\n" );
	Activate();
	SetMouseInputEnabled(true);
	//m_pRichText->SetVerticalScrollbar(true);
}


//-----------------------------------------------------------------------------
// Purpose: User let go of the hint key
//-----------------------------------------------------------------------------
void CHudHintCenter::KeyUp( void )
{
	m_bHintKeyHeld = false;
	//m_bHintCenterVisible = false;
	//Msg( "\nKEY UP!!!\n" );

	// start fading out
	//g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "FadeOutHintCenter" );
	//m_bFadingOut = true;
	m_flSelectionTime = gpGlobals->curtime - HINTCENTER_TIMEOUT_THRESHOLD;
	SetMouseInputEnabled(false);
}


//-----------------------------------------------------------------------------
// Purpose: Catch the next/previous hint buttons. 
//-----------------------------------------------------------------------------
void CHudHintCenter::OnCommand( const char *command )
{
	// The Next Hint button was pressed
	if ( Q_strcmp( command, "Next" ) == 0 )
	{
		if ( m_pRichText )
		{	// Don't fall off the end!
			if ( m_iCurrentHintIndex < ( m_HintStringsVector.Count() - 1 ) )
			{
				m_iCurrentHintIndex++;
				m_pRichText->SetText( "" );
				TranslateKeyCommand( m_HintStringsVector[m_iCurrentHintIndex] );
				// There's probably a better way to do this
				// Create the "current # / total #" string, then convert it to Unicode so Surface() can have its way with it :)
				char szHintCtr[10];
				Q_snprintf( szHintCtr, sizeof( szHintCtr ), "%i/%i", m_iCurrentHintIndex + 1, m_HintStringsVector.Count() );
				vgui::localize()->ConvertANSIToUnicode( szHintCtr, m_szHintCounter, sizeof(m_szHintCounter) );
			}
		}
	}
	// The Previous Hint button was pressed
	else if ( Q_strcmp( command, "Prev" ) == 0 )
	{
		if ( m_pRichText )
		{	// Don't fall off the beginning!
			if ( m_iCurrentHintIndex > 0 )
			{
				m_iCurrentHintIndex--;
				m_pRichText->SetText( "" );
				TranslateKeyCommand( m_HintStringsVector[m_iCurrentHintIndex] );
				// There's probably a better way to do this
				// Create the "current # / total #" string, then convert it to Unicode so Surface() can have its way with it :)
				char szHintCtr[10];
				Q_snprintf( szHintCtr, sizeof( szHintCtr ), "%i/%i", m_iCurrentHintIndex + 1, m_HintStringsVector.Count() );
				vgui::localize()->ConvertANSIToUnicode( szHintCtr, m_szHintCounter, sizeof(m_szHintCounter) );
			}
		}
	}
}

