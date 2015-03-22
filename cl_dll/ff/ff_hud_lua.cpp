/********************************************************************
	created:	2006/07/07
	created:	7:7:2006   15:46
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\ff_hud_lua.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll\ff
	file base:	ff_hud_lua
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Lua controlled hud items
				This is a basic starting implementation.

				Some things to think about:
					� Colour control.
					� Loading the ClientScheme.res settings for font
					  sizes and stuff.
					� Do we _REALLY_ want to give them free reign to
					  put their things everywhere? This screws up
					  users' ability to customise their hud.
					  Perhaps it would be better to stick to the 'slot'
					  idea.
					� You can currently have elements of different types
					  with the same names. That way you could have some
					  text and an icon with the same name and get rid of
					  them both with one call.
*********************************************************************/

#include "cbase.h"

#include "ff_hud_lua.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "filesystem.h"

#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

#include "IGameUIFuncs.h" // for key bindings
extern IGameUIFuncs *gameuifuncs; // for key binding details

using namespace vgui;

DECLARE_HUDELEMENT(CHudLua);
DECLARE_HUD_MESSAGE(CHudLua, FF_HudLua);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudLua::CHudLua(const char *pElementName) : CHudElement(pElementName), vgui::Panel(NULL, "HudLua")
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits( HIDEHUD_PLAYERDEAD );
	m_nHudElements = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHudLua::~CHudLua()
{
}

//-----------------------------------------------------------------------------
// Purpose: Initialise the hud today
//-----------------------------------------------------------------------------
void CHudLua::VidInit()
{
	SetPaintBackgroundEnabled(false);

	for( int i = 0; i < m_nHudElements; i++ )
	{
		if( m_sHudElements[ i ].pPanel )
		{
			m_sHudElements[ i ].pPanel->DeletePanel();
			m_sHudElements[ i ].pPanel = NULL;
		}
	}

	m_nHudElements = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Initialise hud
//-----------------------------------------------------------------------------
void CHudLua::Init()
{
	HOOK_HUD_MESSAGE(CHudLua, FF_HudLua);

	gameeventmanager->AddListener( this, "ff_restartround", false );
}

//-----------------------------------------------------------------------------
// Purpose: Receive message and determine what to create
//-----------------------------------------------------------------------------
void CHudLua::MsgFunc_FF_HudLua(bf_read &msg)
{
	byte wType = msg.ReadByte();

	if (wType < 0)
		return;


	int hudIdentifier = msg.ReadShort();

	if (wType == HUD_REMOVE)
	{
		RemoveElement(hudIdentifier);
		return;
	}

	switch (wType)
	{
	case HUD_ICON:
		{
			int xPos = msg.ReadShort();
			int yPos = msg.ReadShort();

			char szSource[256];
			if (!msg.ReadString(szSource, 255))
				return;

			int iWidth = msg.ReadShort();
			int iHeight = msg.ReadShort();
			int iAlignX = msg.ReadShort();
			int iAlignY = msg.ReadShort();

			HudIcon(hudIdentifier, xPos, yPos, szSource, iWidth, iHeight, iAlignX, iAlignY);

			break;
		}
		
	case HUD_BOX:
		{
			int xPos = msg.ReadShort();
			int yPos = msg.ReadShort();
			int iWidth = msg.ReadShort();
			int iHeight = msg.ReadShort();
			int iRed = msg.ReadShort();
			int iGreen = msg.ReadShort();
			int iBlue = msg.ReadShort();
			int iAlpha = msg.ReadShort();
			int iBorderRed = msg.ReadShort();
			int iBorderGreen = msg.ReadShort();
			int iBorderBlue = msg.ReadShort();
			int iBorderAlpha = msg.ReadShort();
			int iBorderWidth = msg.ReadShort();
			int iAlignX = msg.ReadShort();
			int iAlignY = msg.ReadShort();

			HudBox(hudIdentifier, xPos, yPos, iWidth, iHeight, Color(iRed,iGreen,iBlue,iAlpha), Color(iBorderRed,iBorderGreen,iBorderBlue,iBorderAlpha), iBorderWidth, iAlignX, iAlignY);

			break;
		}

	case HUD_TEXT:
		{
			int xPos = msg.ReadShort();
			int yPos = msg.ReadShort();

			char szText[256];
			if (!msg.ReadString(szText, 255))
				return;

			int iAlignX = msg.ReadShort();
			int iAlignY = msg.ReadShort();
			int iSize = msg.ReadShort();

			HudText(hudIdentifier, xPos, yPos, szText, iAlignX, iAlignY, iSize);

			break;
		}

	case HUD_TIMER:
		{
			int xPos = msg.ReadShort();
			int yPos = msg.ReadShort();

			float	flValue = msg.ReadFloat();
			float	flSpeed = msg.ReadFloat();

			int iAlignX = msg.ReadShort();
			int iAlignY = msg.ReadShort();
			int iSize = msg.ReadShort();

			HudTimer(hudIdentifier, xPos, yPos, flValue, flSpeed, iAlignX, iAlignY, iSize);

			break;
		}
	} // end switch (wType)
}

//-----------------------------------------------------------------------------
// Purpose: Create a new icon on the hud
//-----------------------------------------------------------------------------
void CHudLua::HudIcon(int hudIdentifier, int iX, int iY, const char *pszSource, int iWidth, int iHeight, int iAlignX, int iAlignY)
{
	ImagePanel *pImagePanel = dynamic_cast<ImagePanel *> (GetHudElement(hudIdentifier, HUD_ICON));

	if (!pImagePanel)
		return;

	// set to item-specific defaults if values weren't set or are invalid
	iAlignX = (iAlignX < 0) ? 1 : iAlignX;
	iAlignY = (iAlignY < 0) ? 0 : iAlignY;

	// Yo mirv: Think this is a good idea?
	// Assume x & y are in 640 x 480 so we need to scale so
	// use the proportional scale thingy

	// Now set this label up
	//pImagePanel->SetPos( scheme()->GetProportionalScaledValue( iX ), scheme()->GetProportionalScaledValue( iY ) );

	// Jiggles: The above line displays the flag icon incorrectly on non 4:3 display resolutions
	// -- See: Mantis issue 0001144 -- http://beta.fortress-forever.com/bugtracker/view.php?id=1144
	// Instead, I scale the CHudLua container to the current display resolution, then calculate an x-position
	// 5 pixels to the right of the screen.
	// This solution has a minor problem, though: If the player changes screen resolution while carrying a flag,
	// the flag icon will not be repositioned until he/she picks up a new one.

	if( ( iWidth > 0 ) && ( iHeight > 0 ) )
	{
		SetSize(ScreenWidth(), ScreenHeight());
		int iProperXPosition = 0;
		int iProperYPosition = 0;
		int scaledX = scheme()->GetProportionalScaledValue( iX );
		int scaledY = scheme()->GetProportionalScaledValue( iY );
		int scaledW = scheme()->GetProportionalScaledValue( iWidth );
		int scaledH = scheme()->GetProportionalScaledValue( iHeight );
		switch (iAlignX)
		{
		case 1 : //HUD_ALIGNX_RIGHT :
			iProperXPosition = ( ScreenWidth() - scaledX ) - scaledW;
			break;
		case 2 : //HUD_ALIGNX_CENTERLEFT :
			iProperXPosition = ((ScreenWidth() / 2) - scaledX) - scaledW;
			break;
		case 3 : //HUD_ALIGNX_CENTERRIGHT :
			iProperXPosition = (ScreenWidth() / 2) + scaledX;
			break;
		case 4 : //HUD_ALIGNX_CENTER :
			iProperXPosition = (ScreenWidth() / 2) - (scaledW / 2) + scaledX;
			break;
		case 0 : //HUD_ALIGNX_LEFT : 
		default :
			iProperXPosition = scaledX;
			break;
		}
		switch (iAlignY)
		{
		case 1 : //HUD_ALIGNY_BOTTOM :
			iProperYPosition = ( ScreenHeight() - scaledY ) - scaledH;
			break;
		case 2 : //HUD_ALIGNY_CENTERUP :
			iProperYPosition = ((ScreenHeight() / 2) - scaledY) - scaledH;
			break;
		case 3 : //HUD_ALIGNY_CENTERDOWN :
			iProperYPosition = (ScreenHeight() / 2) + scaledY;
			break;
		case 4 : //HUD_ALIGNY_CENTER :
			iProperYPosition = (ScreenHeight() / 2) - (scaledH / 2) + scaledY;
			break;
		case 0 : //HUD_ALIGNY_TOP : 
		default :
			iProperYPosition = scaledY;
			break;
		}

		//int iProperXPosition = ScreenWidth() - scheme()->GetProportionalScaledValue( iWidth ) - 5;
		//pImagePanel->SetPos( iProperXPosition, scheme()->GetProportionalScaledValue( iY ) );
		pImagePanel->SetPos( iProperXPosition, iProperYPosition );

		pImagePanel->SetShouldScaleImage( true );
		pImagePanel->SetWide( scheme()->GetProportionalScaledValue( iWidth ) );
		pImagePanel->SetTall( scheme()->GetProportionalScaledValue( iHeight ) );
	}

	pImagePanel->SetImage(pszSource);
	//pImagePanel->MoveToFront();
	pImagePanel->SetVisible(true);
}

//-----------------------------------------------------------------------------
// Purpose: Create a new box on the hud
//-----------------------------------------------------------------------------
void CHudLua::HudBox(int hudIdentifier, int iX, int iY, int iWidth, int iHeight, Color clr, Color clrBorder, int iBorderWidth, int iAlignX, int iAlignY)
{
	// Create or find the correct hud element
	FFLuaBox *pLabel = dynamic_cast<FFLuaBox *> (GetHudElement(hudIdentifier, HUD_BOX));

	if (!pLabel)
		return;

	// set to item-specific defaults if values weren't set or are invalid
	iAlignX = (iAlignX < 0) ? 1 : iAlignX;
	iAlignY = (iAlignY < 0) ? 0 : iAlignY;

	// Yo mirv: Think this is a good idea?
	// Assume x & y are in 640 x 480 so we need to scale so
	// use the proportional scale thingy

	// Now set this label up
	//pLabel->SetPos( scheme()->GetProportionalScaledValue( iX ), scheme()->GetProportionalScaledValue( iY ) );

	// Jiggles: The above line displays the flag icon incorrectly on non 4:3 display resolutions
	// -- See: Mantis issue 0001144 -- http://beta.fortress-forever.com/bugtracker/view.php?id=1144
	// Instead, I scale the CHudLua container to the current display resolution, then calculate an x-position
	// 5 pixels to the right of the screen.
	// This solution has a minor problem, though: If the player changes screen resolution while carrying a flag,
	// the flag icon will not be repositioned until he/she picks up a new one.

	if( ( iWidth > 0 ) && ( iHeight > 0 ) )
	{
		SetSize(ScreenWidth(), ScreenHeight());
		int iProperXPosition = 0;
		int iProperYPosition = 0;
		int scaledX = scheme()->GetProportionalScaledValue( iX );
		int scaledY = scheme()->GetProportionalScaledValue( iY );
		int scaledW = scheme()->GetProportionalScaledValue( iWidth );
		int scaledH = scheme()->GetProportionalScaledValue( iHeight );
		switch (iAlignX)
		{
		case 1 : //HUD_ALIGNX_RIGHT :
			iProperXPosition = ( ScreenWidth() - scaledX ) - scaledW;
			break;
		case 2 : //HUD_ALIGNX_CENTERLEFT :
			iProperXPosition = ((ScreenWidth() / 2) - scaledX) - scaledW;
			break;
		case 3 : //HUD_ALIGNX_CENTERRIGHT :
			iProperXPosition = (ScreenWidth() / 2) + scaledX;
			break;
		case 4 : //HUD_ALIGNX_CENTER :
			iProperXPosition = (ScreenWidth() / 2) - (scaledW / 2) + scaledX;
			break;
		case 0 : //HUD_ALIGNX_LEFT : 
		default :
			iProperXPosition = scaledX;
			break;
		}
		switch (iAlignY)
		{
		case 1 : //HUD_ALIGNY_BOTTOM :
			iProperYPosition = ( ScreenHeight() - scaledY ) - scaledH;
			break;
		case 2 : //HUD_ALIGNY_CENTERUP :
			iProperYPosition = ((ScreenHeight() / 2) - scaledY) - scaledH;
			break;
		case 3 : //HUD_ALIGNY_CENTERDOWN :
			iProperYPosition = (ScreenHeight() / 2) + scaledY;
			break;
		case 4 : //HUD_ALIGNY_CENTER :
			iProperYPosition = (ScreenHeight() / 2) - (scaledH / 2) + scaledY;
			break;
		case 0 : //HUD_ALIGNY_TOP : 
		default :
			iProperYPosition = scaledY;
			break;
		}

		//int iProperXPosition = ScreenWidth() - scheme()->GetProportionalScaledValue( iWidth ) - 5;
		//pLabel->SetPos( iProperXPosition, scheme()->GetProportionalScaledValue( iY ) );
		pLabel->SetPos( iProperXPosition, iProperYPosition );

		pLabel->SetWide( scheme()->GetProportionalScaledValue( iWidth ) );
		pLabel->SetTall( scheme()->GetProportionalScaledValue( iHeight ) );
	}

	//IScheme *pScheme = scheme()->GetIScheme( pLabel->GetScheme() );

	pLabel->SetBorderColor( clrBorder );
	pLabel->SetBorderWidth( iBorderWidth );
	pLabel->SetBoxColor( clr );

	pLabel->SetPaintBackgroundEnabled(true);
	pLabel->SetPaintEnabled( true );

	//pLabel->MoveToFront();
	pLabel->SetVisible(true);
}

//-----------------------------------------------------------------------------
// Purpose: Create a new text box on the hud - x & y alignment
//-----------------------------------------------------------------------------
void CHudLua::HudText(int hudIdentifier, int iX, int iY, const char *pszText, int iAlignX, int iAlignY, int iSize)
{
	// Create or find the correct hud element
	Label *pLabel = dynamic_cast<Label *> (GetHudElement(hudIdentifier, HUD_TEXT));

	if (!pLabel)
		return;

	// set to item-specific defaults if values weren't set or are invalid
	iAlignX = (iAlignX < 0) ? 0 : iAlignX;
	iAlignY = (iAlignY < 0) ? 0 : iAlignY;

	char szTranslatedText[1024];
	
	// Now set this label up
	if ( TranslateKeyCommand( pszText, szTranslatedText, sizeof(szTranslatedText) ) )
		pLabel->SetText(szTranslatedText);
	else
		pLabel->SetText(pszText);
	
	if (iSize >= 1 && iSize <=5)
	{
		IScheme *pScheme = scheme()->GetIScheme( pLabel->GetScheme() );
		if ( pScheme )
		{
			HFont font = pScheme->GetFont( VarArgs("LuaText%d",iSize), pLabel->IsProportional() );
			if ( font != INVALID_FONT )
				pLabel->SetFont( font );
		}
	}

	pLabel->SizeToContents();

	int iProperXPosition = 0;
	int iProperYPosition = 0;
	int scaledX = scheme()->GetProportionalScaledValue( iX );
	int scaledY = scheme()->GetProportionalScaledValue( iY );
	int scaledW = 0; // surface()->GetCharacterWidth(pTimer->GetFont(), '0' ) * 5;
	int scaledH = 0; // surface()->GetFontTall( pTimer->GetFont() );
	pLabel->GetContentSize( scaledW, scaledH );

	switch (iAlignX)
	{
	case 1 : //HUD_ALIGNX_RIGHT :
		iProperXPosition = ( ScreenWidth() - scaledX ) - scaledW;
		break;
	case 2 : //HUD_ALIGNX_CENTERLEFT :
		iProperXPosition = ((ScreenWidth() / 2) - scaledX) - scaledW;
		break;
	case 3 : //HUD_ALIGNX_CENTERRIGHT :
		iProperXPosition = (ScreenWidth() / 2) + scaledX;
		break;
	case 4 : //HUD_ALIGNX_CENTER :
		iProperXPosition = (ScreenWidth() / 2) - (scaledW / 2) + scaledX;
		break;
	case 5 : //HUD_ALIGNX_RIGHT_STRINGSTART :
		iProperXPosition = ScreenWidth() - scaledX;
		break;
	case 6 : //HUD_ALIGNX_LEFT_STRINGEND :
		iProperXPosition = scaledX - scaledW;
		break;
	case 0 : //HUD_ALIGNX_LEFT : 
	default :
		iProperXPosition = scaledX;
		break;
	}
	switch (iAlignY)
	{
	case 1 : //HUD_ALIGNY_BOTTOM :
		iProperYPosition = ( ScreenHeight() - scaledY ) - scaledH;
		break;
	case 2 : //HUD_ALIGNY_CENTERUP :
		iProperYPosition = ((ScreenHeight() / 2) - scaledY) - scaledH;
		break;
	case 3 : //HUD_ALIGNY_CENTERDOWN :
		iProperYPosition = (ScreenHeight() / 2) + scaledY;
		break;
	case 4 : //HUD_ALIGNY_CENTER :
		iProperYPosition = (ScreenHeight() / 2) - (scaledH / 2) + scaledY;
		break;
	case 0 : //HUD_ALIGNY_TOP : 
	default :
		iProperYPosition = scaledY;
		break;
	}

	pLabel->SetPos( iProperXPosition, iProperYPosition );

	//pLabel->MoveToFront();
	pLabel->SetVisible(true);
}

//-------------------------------------------------------------------------------
// Purpose: Translate key commands into user's current key bind, and add the 
//			hint text to the text box. 
//			-For example, if the hint string says {+duck} and the user has the
//			 duck command bound to the SHIFT key, this function translates the 
//			 hint string into SHIFT.
//-------------------------------------------------------------------------------
bool CHudLua::TranslateKeyCommand( const char *szMessage, char *szTranslated, int iBufferSizeInBytes )
{
	if ( !szMessage )
	{
		Warning( "Error: Received a HudText message that was formatted incorrectly!\n" );
		return false;
	}

	int i = 0;
	int iTrans = 0;
	for (;;)
	{
		if ( szMessage[i] == '%' )
		{
			char sKeyCommand[30];
			int iKeyIndex = 0;
			for (;;)
			{
				i++;

				// This shouldn't happen unless someone used %bind% incorrectly
				if ( szMessage[i] == '\0' || iKeyIndex > 30 )
				{
					return false;
				}
				// We've got the whole command -- now find out what key it's bound to
				if ( szMessage[i] == '%' )
				{
					sKeyCommand[iKeyIndex] = '\0';
					//Msg( "\nCommand: %s\n", sKeyCommand );
					const char *sConvertedCommand = gameuifuncs->Key_NameForKey( gameuifuncs->GetEngineKeyCodeForBind( sKeyCommand ) );
					//Msg( "\nConverted Command: %s\n", sConvertedCommand );
					for( const char *wch = sConvertedCommand; *wch != 0; wch++ )
					{
						szTranslated[iTrans] = *wch;
						iTrans++;
					}
					i++;
					break;
				}
				//Msg( "\nChar: %c\n", szMessage[i] );
				sKeyCommand[iKeyIndex] = szMessage[i];
				iKeyIndex++;

			}
		}

		szTranslated[iTrans] = szMessage[i];

		// We're done!
		if ( szMessage[i] == '\0' )
			return true;
		i++;
		iTrans++;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Create a new timer on the hud - x & y alignment
//-----------------------------------------------------------------------------
void CHudLua::HudTimer(int hudIdentifier, int iX, int iY, float flValue, float flSpeed, int iAlignX, int iAlignY, int iSize)
{
	// Create or find the correct hud element
	Timer *pTimer = dynamic_cast<Timer *> (GetHudElement(hudIdentifier, HUD_TIMER));

	if (!pTimer)
		return;

	// set to item-specific defaults if values weren't set or are invalid
	iAlignX = (iAlignX < 0) ? 0 : iAlignX;
	iAlignY = (iAlignY < 0) ? 0 : iAlignY;

	pTimer->SetTimerValue(flValue);
	pTimer->SetTimerSpeed(flSpeed);
	pTimer->SetTimerDrawClockStyle(true);
	pTimer->StartTimer(true);
	
	if (iSize >= 1 && iSize <=5)
	{
		IScheme *pScheme = scheme()->GetIScheme( pTimer->GetScheme() );
		if ( pScheme )
		{
			HFont font = pScheme->GetFont( VarArgs("LuaText%d",iSize), pTimer->IsProportional() );
			if ( font != INVALID_FONT )
				pTimer->SetFont( font );
		}
	}

	pTimer->SetContentAlignment(vgui::Label::a_center);
	pTimer->UpdateTimer();
	pTimer->SizeToContents();

	int iProperXPosition = 0;
	int iProperYPosition = 0;
	int scaledX = scheme()->GetProportionalScaledValue( iX );
	int scaledY = scheme()->GetProportionalScaledValue( iY );
	int scaledW = 0; // surface()->GetCharacterWidth(pTimer->GetFont(), '0' ) * 5;
	int scaledH = 0; // surface()->GetFontTall( pTimer->GetFont() );
	pTimer->GetContentSize( scaledW, scaledH );

	switch (iAlignX)
	{
	case 1 : //HUD_ALIGNX_RIGHT :
		iProperXPosition = ( ScreenWidth() - scaledX ) - scaledW;
		break;
	case 2 : //HUD_ALIGNX_CENTERLEFT :
		iProperXPosition = ((ScreenWidth() / 2) - scaledX) - scaledW;
		break;
	case 3 : //HUD_ALIGNX_CENTERRIGHT :
		iProperXPosition = (ScreenWidth() / 2) + scaledX;
		break;
	case 4 : //HUD_ALIGNX_CENTER :
		iProperXPosition = (ScreenWidth() / 2) - (scaledW / 2) + scaledX;
		break;
	case 5 : //HUD_ALIGNX_RIGHT_STRINGSTART :
		iProperXPosition = ScreenWidth() - scaledX;
		break;
	case 6 : //HUD_ALIGNX_LEFT_STRINGEND :
		iProperXPosition = scaledX - scaledW;
		break;
	case 0 : //HUD_ALIGNX_LEFT : 
	default :
		iProperXPosition = scaledX;
		break;
	}
	switch (iAlignY)
	{
	case 1 : //HUD_ALIGNY_BOTTOM :
		iProperYPosition = ( ScreenHeight() - scaledY ) - scaledH;
		break;
	case 2 : //HUD_ALIGNY_CENTERUP :
		iProperYPosition = ((ScreenHeight() / 2) - scaledY) - scaledH;
		break;
	case 3 : //HUD_ALIGNY_CENTERDOWN :
		iProperYPosition = (ScreenHeight() / 2) + scaledY;
		break;
	case 4 : //HUD_ALIGNY_CENTER :
		iProperYPosition = (ScreenHeight() / 2) - (scaledH / 2) + scaledY;
		break;
	case 0 : //HUD_ALIGNY_TOP : 
	default :
		iProperYPosition = scaledY;
		break;
	}

	// Now set this label up
	pTimer->SetPos( iProperXPosition, iProperYPosition );

	//pTimer->MoveToFront();
	pTimer->SetVisible(true);
}

//-----------------------------------------------------------------------------
// Purpose: Find a hud element for a particular identifier
//-----------------------------------------------------------------------------
Panel *CHudLua::GetHudElement(int hudIdentifier, HudElementType_t iType)
{
	// Return existing one
	for (int i = 0; i < m_nHudElements; i++)
	{
		if (iType == m_sHudElements[i].iType && m_sHudElements[i].hudIdentifier == hudIdentifier)
			return m_sHudElements[i].pPanel;
	}

	// Don't create a new one if we've reached max elements
	if (m_nHudElements == MAX_HUD_ELEMENTS)
	{
		AssertMsg(0, "MAX_HUD_ELEMENTS reached!");
		return NULL;
	}

	Panel *pPanel = NULL;
	char szPanelName[16];
	sprintf(szPanelName, "ff_hud_lua_%d", hudIdentifier);

	// Return a new one of the correct type
	switch (iType)
	{
		case HUD_ICON:
		{
			pPanel = new ImagePanel(this, szPanelName);
		}
		break;

		case HUD_BOX:
		{
			pPanel = new FFLuaBox(this, szPanelName);
		}
		break;

		case HUD_TEXT:
		{
			pPanel = new Label(this, szPanelName, "");

			Label *pLabel = dynamic_cast<Label *>(pPanel);
			if (pLabel)
			{
				IScheme *pScheme = scheme()->GetIScheme( pLabel->GetScheme() );
				if ( pScheme )
				{
					HFont font = pScheme->GetFont( "LuaText_Default", pLabel->IsProportional() );
						if ( font != INVALID_FONT )
							pLabel->SetFont( font );
				}
			}
		}
		break;

		case HUD_TIMER:
		{
			pPanel = new Timer(this, szPanelName);

			Timer *pTimer = dynamic_cast<Timer *>(pPanel);
			if (pTimer)
			{
				IScheme *pScheme = scheme()->GetIScheme( pTimer->GetScheme() );
				if ( pScheme )
				{
					HFont font = pScheme->GetFont( "LuaText_Default", pTimer->IsProportional() );
					if ( font != INVALID_FONT )
						pTimer->SetFont( font );
				}
			}
		}
		break;
	}

	// It's possible that because of memory problems we couldn't
	// create this hud element
	if (!pPanel)
		return NULL;

	// Store tracking data about this hud element
	m_sHudElements[m_nHudElements].pPanel = pPanel;
	m_sHudElements[m_nHudElements].iType = iType;
	m_sHudElements[m_nHudElements].hudIdentifier = hudIdentifier;

	m_nHudElements++;

	return pPanel;
}

//-----------------------------------------------------------------------------
// Purpose: Remove (hide) any elements with this name
//-----------------------------------------------------------------------------
void CHudLua::RemoveElement(int identifier)
{
	for (int i = 0; i < m_nHudElements; i++)
	{
		if (m_sHudElements[i].hudIdentifier == identifier)
		{
			m_sHudElements[i].pPanel->SetVisible(false);
			return;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: On restart round, remove any icons on screen
//-----------------------------------------------------------------------------
void CHudLua::FireGameEvent( IGameEvent *pEvent )
{
	const char *pszEventName = pEvent->GetName();

	if( Q_strcmp( "ff_restartround", pszEventName ) == 0 )
	{
		// Clear all hud stuff
		VidInit();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Should we draw? (Are we ingame? have we picked a class, etc)
//-----------------------------------------------------------------------------
bool CHudLua::ShouldDraw() 
{ 
	if( !engine->IsInGame() ) 
		return false; 

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer(); 

	if( !pPlayer ) 
		return false; 

	if( pPlayer->GetTeamNumber() == TEAM_UNASSIGNED || (!FF_HasPlayerPickedClass( pPlayer ) && !FF_IsPlayerSpec( pPlayer )) )
		return false; 

	return true; 
} 
