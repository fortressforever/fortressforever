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

#include "cbase.h"
#include "ff_hud_HintCenter.h"

#include "hudelement.h"
#include "hud_macros.h"
#include "mathlib.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>

#include <vgui_controls/RichText.h>

#include "c_ff_player.h"
#include "ff_utils.h"

#include "IGameUIFuncs.h" // for key bindings
extern IGameUIFuncs *gameuifuncs; // for key binding details

#include <vgui/ILocalize.h>

using namespace vgui;

//extern ConVar sensitivity;
//extern ConVar cm_capturemouse;
//extern ConVar cm_showmouse;

//#define MENU_PROGRESS_TIME	0.3f

// Global
CHudHintCenter *g_pHintCenter = NULL;
// Global Helper var
CHudHintCenter *g_pHintHelper = NULL;

ConVar hudhints( "cl_hudhints", "0", FCVAR_ARCHIVE, "Display hints" );


DECLARE_HUDELEMENT( CHudHintCenter );
DECLARE_HUD_MESSAGE( CHudHintCenter, FF_SendHint );


//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
CHudHintCenter::~CHudHintCenter( void )
{
	if( m_pHudElementTexture )
	{
		delete m_pHudElementTexture;
		m_pHudElementTexture = NULL;
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



void CHudHintCenter::AddHudHint( unsigned short hintID, const char *pszMessage )
{
	// First off, we're now ignoring hints which are triggered while a hint is
	// playing. We don't queue them up because they'll probably have lost relevancy
	// by the time they are played.
	//if (gpGlobals->curtime < m_flNextHint && m_bActive)
	//{
		//DevMsg("[Hud Hint] Hint ignored (%s)\n", pszMessage);
	//	return;
	//}

	//DevMsg( "[Hud Hint] AddHudHint: %s\n", pszMessage );

	// When adding a hud hint we need to do a couple of things
	// Firstly, check to see if it's a new hint (ie. a hint
	// that the user hasn't seen before)
	//HintVector *sHint = NULL;

	//if (bType == HINT_GENERAL)
	//	sHint = &sGeneralHints;
	//else
	//	sHint = &sMapHints;

	//// Already in list of shown hints, so not active by default
	//m_bActive = sHint->HasElement(wID);

	// Secondly, if it's a new hint, add it to our log file
	// thing
	//if (m_bActive)
	//	sHint->AddToTail(wID);

	// Thirdly, display the new hint
	// Fourthly, if it's not a new hint we simply show
	// an icon on the screen that tells the user they can
	// hit their "show hint" key to view the old hint


	// TODO: TODO: TODO:
	// Do this here for now in case the dev team
	// is testing different durations and/or trying
	// to find the duration they want. Hard code it
	// later in VidInit or Init. This just lets it
	// get updated everytime we get a new hud hint.
	//m_flDuration = hint_duration.GetInt();
	//m_flStarted = gpGlobals->curtime;
	//m_flNextHint = m_flStarted + m_flDuration + 2.0f;


	// Hints are off -- ignore the hint
	if( !hudhints.GetBool() )
		return;
	
	//int foundIndex = -1;
	//for ( int i = 0; i < m_HintVector.Count(); ++i )
	//{
	//	if ( m_HintVector(i).HintIndex == hintID )
	//	{
	//		foundIndex = i;
	//		break;
	//	}
	//}
	
	
	// Save some of the old hints -- note that this will cut them off at an arbitrary point
	char oldHintString[HINT_HISTORY];
	m_pRichText->GetText( 0, oldHintString, sizeof( oldHintString ) ); 
	wchar_t *pszTemp = vgui::localize()->Find( pszMessage );
	m_pRichText->SetText( "" );
	TranslateKeyCommand( pszTemp );
	m_pRichText->InsertString( "\n\n" );
	m_pRichText->InsertString( oldHintString );
	

	int foundIndex = m_HintVector.Find( hintID );
	if (  foundIndex < 0 )  // Hint not shown yet
	{
		m_HintVector.AddToTail( hintID );
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "OpenHintCenter" );
	}
	else if ( m_bHintKeyHeld )
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "OpenHintCenter" );
	else
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "OpenHintCenterIcon" );
	
	m_bFadingOut = false;
	m_bHintCenterVisible = true;
	m_flSelectionTime = gpGlobals->curtime;

	Activate();
	SetMouseInputEnabled(false);

	// And play a sound, if there's one
	//if (pszSound)
	//{
	//	CBasePlayer *pLocal = CBasePlayer::GetLocalPlayer();

	//	if (!pLocal)
	//		return;

	//	CPASAttenuationFilter filter(pLocal, pszSound);

	//	EmitSound_t params;
	//	params.m_pSoundName = pszSound;
	//	params.m_flSoundTime = 0.0f;
	//	params.m_pflSoundDuration = NULL;
	//	params.m_bWarnOnDirectWaveReference = false;

	//	pLocal->EmitSound(filter, pLocal->entindex(), params);
	//}
}



void CHudHintCenter::MsgFunc_FF_SendHint( bf_read &msg )
{

	// Hints are off -- ignore the hint
	if( !hudhints.GetBool() )
		return;

	//Msg("HINT MESSAGE RECEIVED!!!\n");
	// Buffer
	char szString[ 4096 ];
	//char szSound[ 4096 ];

	// Hint type [general, map]
	//byte bType = msg.ReadByte();

	// Hint id
	unsigned short wID = msg.ReadWord();

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
	
	// Save some of the old hints -- note that this will cut them off at an arbitrary point
	char oldHintString[HINT_HISTORY];
	m_pRichText->GetText( 0, oldHintString, sizeof( oldHintString ) );
	wchar_t *pszTemp = vgui::localize()->Find( szString );
	wchar_t szUnlocalizedStr[4096];
	if( !pszTemp )
	{
		vgui::localize()->ConvertANSIToUnicode( szString, szUnlocalizedStr, 512 );
		pszTemp = szUnlocalizedStr;
	}
	m_pRichText->SetText( "" );
	TranslateKeyCommand( pszTemp );
	//m_pRichText->SetText( szString );
	m_pRichText->InsertString( "\n\n" );
	m_pRichText->InsertString( oldHintString );

	int foundIndex = m_HintVector.Find( wID );
	if (  foundIndex < 0 )  // Hint not shown yet
	{
		m_HintVector.AddToTail( wID );
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "OpenHintCenter" );
	}
	else if ( m_bHintKeyHeld )
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "OpenHintCenter" );
	else
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "OpenHintCenterIcon" );
	
	m_bFadingOut = false;
	m_bHintCenterVisible = true;
	m_flSelectionTime = gpGlobals->curtime;

	Activate();
	SetMouseInputEnabled(false);
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
					Warning( "\nError: Received a hint message that was formatted incorrectly!\n" );
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

	m_pHudElementTexture = new CHudTexture();
	m_pHudElementTexture->bRenderUsingFont = true;
	m_pHudElementTexture->hFont = m_hDisguiseFont;
	m_pHudElementTexture->cCharacterInFont = 'G';


	 //Set up the rich text box that will contain the hud hint stuff
	if (!m_pRichText)
	{
		m_pRichText = new RichText(this, "HudHintText");
		m_pRichText->SetVerticalScrollbar( true );
		m_pRichText->SetPos( text1_xpos, text1_ypos );
		m_pRichText->SetWide( text1_wide );
		m_pRichText->SetTall( text1_tall );

		//m_pRichText->SetBorder( NULL );
		//m_pRichText->InsertColorChange( m_TextColor );
		//m_pRichText->SetFont( m_hTextFont );
		//m_pRichText->SetFgColor( Color(0, 0, 0, 0) );
		m_pRichText->SetPaintBorderEnabled( false );
		//m_pRichText->SetMaximumCharCount( 1024 );
	
	}

}





//-----------------------------------------------------------------------------
// Purpose: updates animation status
//-----------------------------------------------------------------------------
void CHudHintCenter::OnThink( void )
{
	// Time out after awhile of inactivity
	if ( ( ( gpGlobals->curtime - m_flSelectionTime ) > SELECTION_TIMEOUT_THRESHOLD ) && !m_bHintKeyHeld )
	{
		if (!m_bFadingOut)
		{
			// start fading out
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "FadeOutHintCenter" );
			m_bFadingOut = true;
		}
		else if (gpGlobals->curtime - m_flSelectionTime > SELECTION_TIMEOUT_THRESHOLD + SELECTION_FADEOUT_TIME)
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
	if( !engine->IsInGame() )
		return false;

	if( !m_bHintCenterVisible )
		return false;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return false;

	if( FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer ) )
		return false;

	//if( !pPlayer->IsAlive() )
	//	return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Draw
//-----------------------------------------------------------------------------
void CHudHintCenter::Paint( void )
{

	//if (!ShouldDraw())
	//	return;

	// Draw!
	if( m_pHudElementTexture )
	{
		// Paint foreground/background stuff
		//BaseClass::PaintBackground();
		
		DrawBox( 0, 0, GetWide(), GetTall(), m_BGBoxColor, m_flSelectionAlphaOverride / 255.0f );
		DrawHollowBox( 0, 0, GetWide(), GetTall(), m_TextColor, m_flSelectionAlphaOverride / 255.0f );
		
		//C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
		//Color clr = pPlayer->GetTeamColor();
		//Color clr = GetFgColor();

		// Draw the icon
		m_pHudElementTexture->DrawSelf( image1_xpos, image1_ypos, m_TextColor );

		//Look up the resource string
		//wchar_t *pszText = vgui::localize()->Find( Class_IntToResourceString( pPlayer->GetDisguisedClass() ) );

		// Draw text
		//surface()->DrawSetTextFont( m_hTextFont );
		//surface()->DrawSetTextColor( m_TextColor );
		//surface()->DrawSetTextPos( text1_xpos, text1_ypos );
		//surface()->DrawUnicodeString( szText );
		//surface()->DrawUnicodeString( m_pText );
		
	}

}

//-----------------------------------------------------------------------------
// Purpose: User is pressing their menu key
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
// Purpose: User let go of menu key
//-----------------------------------------------------------------------------
void CHudHintCenter::KeyUp( void )
{
	m_bHintKeyHeld = false;
	//m_bHintCenterVisible = false;
	//Msg( "\nKEY UP!!!\n" );

	// start fading out
	//g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "FadeOutHintCenter" );
	//m_bFadingOut = true;
	m_flSelectionTime = gpGlobals->curtime - SELECTION_TIMEOUT_THRESHOLD;
	SetMouseInputEnabled(false);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
//void CHudHintCenter::MouseMove( float *x, float *y ) 
//{
//	float sensitivity_factor = 1.0f / ( sensitivity.GetFloat() == 0 ? 0.001f : sensitivity.GetFloat() );
//
//	float midx = scheme()->GetProportionalScaledValue( 320 );
//	float midy = scheme()->GetProportionalScaledValue( 240 );
//	float dist = scheme()->GetProportionalScaledValue( 120 );
//
//	m_flPosX = clamp( m_flPosX + ( *x * sensitivity_factor ), midx - dist, midx + dist );
//	m_flPosY = clamp( m_flPosY + ( *y * sensitivity_factor ), midy - dist, midy + dist );
//
//	if( m_bHintCenterVisible && cm_capturemouse.GetBool() )
//	{
//		*x = 0;
//		*y = 0;
//	}
//}

//-----------------------------------------------------------------------------
// Purpose: Send mouse to the menu
//-----------------------------------------------------------------------------
//void HudHintCenterInput( float *x, float *y ) 
//{
//	if( g_pHintCenter )
//		g_pHintCenter->MouseMove( x, y );
//}
