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

#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include "vgui/ff_vgui_timer.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudLua);
DECLARE_HUD_MESSAGE(CHudLua, FF_HudLua);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudLua::CHudLua(const char *pElementName) : CHudElement(pElementName), vgui::Panel(NULL, "HudLua")
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(0);
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

	char szIdentifier[256];

	if (!msg.ReadString(szIdentifier, 255))
		return;

	if (wType == HUD_REMOVE)
	{
		RemoveElement(szIdentifier);
		return;
	}

	int xPos = msg.ReadShort();
	int yPos = msg.ReadShort();

	switch (wType)
	{
	case HUD_ICON:
		{
			char szSource[256];
			if (!msg.ReadString(szSource, 255))
				return;

			int iWidth = msg.ReadShort();
			int iHeight = msg.ReadShort();
			int iAlign = msg.ReadShort();
	
			HudIcon(szIdentifier, xPos, yPos, szSource, iWidth, iHeight, iAlign);

			break;
		}

	case HUD_ICON_ALIGNXY:
		{
			char szSource[256];
			if (!msg.ReadString(szSource, 255))
				return;

			int iWidth = msg.ReadShort();
			int iHeight = msg.ReadShort();
			int iAlignX = msg.ReadShort();
			int iAlignY = msg.ReadShort();

			HudIcon(szIdentifier, xPos, yPos, szSource, iWidth, iHeight, iAlignX, iAlignY);

			break;
		}

	case HUD_TEXT:
		{
			if (xPos < 0 || yPos < 0)
				return;

			char szText[256];
			if (!msg.ReadString(szText, 255))
				return;

			HudText(szIdentifier, xPos, yPos, szText);

			break;
		}

	case HUD_TEXT_ALIGN:
		{
			if (xPos < 0 || yPos < 0)
				return;

			char szText[256];
			if (!msg.ReadString(szText, 255))
				return;

			int iAlign = msg.ReadShort();

			HudText(szIdentifier, xPos, yPos, szText, iAlign);

			break;
		}

	case HUD_TEXT_ALIGNXY:
		{
			if (xPos < 0 || yPos < 0)
				return;

			char szText[256];
			if (!msg.ReadString(szText, 255))
				return;

			int iAlignX = msg.ReadShort();
			int iAlignY = msg.ReadShort();

			HudText(szIdentifier, xPos, yPos, szText, iAlignX, iAlignY);

			break;
		}

	case HUD_TIMER:
		{
			if (xPos < 0 || yPos < 0)
				return;

			int		iValue = msg.ReadShort();
			float	flSpeed = msg.ReadFloat();

			HudTimer(szIdentifier, xPos, yPos, iValue, flSpeed);

			break;
		}

	case HUD_TIMER_ALIGN:
		{
			if (xPos < 0 || yPos < 0)
				return;

			int		iValue = msg.ReadShort();
			float	flSpeed = msg.ReadFloat();

			int iAlign = msg.ReadShort();

			HudTimer(szIdentifier, xPos, yPos, iValue, flSpeed, iAlign);

			break;
		}

	case HUD_TIMER_ALIGNXY:
		{
			if (xPos < 0 || yPos < 0)
				return;

			int		iValue = msg.ReadShort();
			float	flSpeed = msg.ReadFloat();

			int iAlignX = msg.ReadShort();
			int iAlignY = msg.ReadShort();

			HudTimer(szIdentifier, xPos, yPos, iValue, flSpeed, iAlignX, iAlignY);

			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Create a new icon on the hud
//-----------------------------------------------------------------------------
void CHudLua::HudIcon(const char *pszIdentifier, int iX, int iY, const char *pszSource, int iWidth, int iHeight, int iAlign)
{
	// Create or find the correct hud element
	ImagePanel *pImagePanel = dynamic_cast<ImagePanel *> (GetHudElement(pszIdentifier, HUD_ICON));

	if (!pImagePanel)
		return;

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
		int scaledX = scheme()->GetProportionalScaledValue( iX );
		int scaledW = scheme()->GetProportionalScaledValue( iWidth );
		switch (iAlign)
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

		//int iProperXPosition = ScreenWidth() - scheme()->GetProportionalScaledValue( iWidth ) - 5;
		pImagePanel->SetPos( iProperXPosition, scheme()->GetProportionalScaledValue( iY ) );
		
		pImagePanel->SetShouldScaleImage( true );
		pImagePanel->SetWide( scheme()->GetProportionalScaledValue( iWidth ) );
		pImagePanel->SetTall( scheme()->GetProportionalScaledValue( iHeight ) );
	}

	pImagePanel->SetImage(pszSource);
	pImagePanel->SetVisible(true);
}

//-----------------------------------------------------------------------------
// Purpose: Create a new icon on the hud (added y alignment)
//-----------------------------------------------------------------------------
void CHudLua::HudIcon(const char *pszIdentifier, int iX, int iY, const char *pszSource, int iWidth, int iHeight, int iAlignX, int iAlignY)
{
	// Create or find the correct hud element
	ImagePanel *pImagePanel = dynamic_cast<ImagePanel *> (GetHudElement(pszIdentifier, HUD_ICON_ALIGNXY));

	if (!pImagePanel)
		return;

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
	pImagePanel->SetVisible(true);
}

//-----------------------------------------------------------------------------
// Purpose: Create a new text box on the hud
//-----------------------------------------------------------------------------
void CHudLua::HudText(const char *pszIdentifier, int iX, int iY, const char *pszText)
{
	Label *pLabel = dynamic_cast<Label *> (GetHudElement(pszIdentifier, HUD_TEXT));

	if (!pLabel)
		return;

	// Now set this label up
	pLabel->SetPos( scheme()->GetProportionalScaledValue( iX ), scheme()->GetProportionalScaledValue( iY ) );
	pLabel->SetText(pszText);
	pLabel->SizeToContents();

	pLabel->SetVisible(true);
}

//-----------------------------------------------------------------------------
// Purpose: Create a new text box on the hud - X alignment
//-----------------------------------------------------------------------------
void CHudLua::HudText(const char *pszIdentifier, int iX, int iY, const char *pszText, int iAlign)
{
	int iWidth = 0;

	int iProperXPosition = 0;
	int scaledX = scheme()->GetProportionalScaledValue( iX );
	int scaledW = scheme()->GetProportionalScaledValue( iWidth );
	switch (iAlign)
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

	HudText(pszIdentifier, iProperXPosition, iY, pszText);
}

//-----------------------------------------------------------------------------
// Purpose: Create a new text box on the hud - X & Y alignment
//-----------------------------------------------------------------------------
void CHudLua::HudText(const char *pszIdentifier, int iX, int iY, const char *pszText, int iAlignX, int iAlignY)
{
	int iWidth = 0;
	int iHeight = 0;

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

	HudText(pszIdentifier, iProperXPosition, iProperYPosition, pszText);
}

//-----------------------------------------------------------------------------
// Purpose: Create a new timer on the hud
//-----------------------------------------------------------------------------
void CHudLua::HudTimer(const char *pszIdentifier, int iX, int iY, float flValue, float flSpeed)
{
	Timer *pTimer = dynamic_cast<Timer *> (GetHudElement(pszIdentifier, HUD_TIMER));

	if (!pTimer)
		return;

	// Now set this label up
	pTimer->SetPos( scheme()->GetProportionalScaledValue( iX ), scheme()->GetProportionalScaledValue( iY ) );
	pTimer->SetTimerValue(flValue);
	pTimer->SetTimerSpeed(flSpeed);
	pTimer->SetTimerDrawClockStyle(true);
	pTimer->StartTimer(true);

	pTimer->SetVisible(true);
}

//-----------------------------------------------------------------------------
// Purpose: Create a new timer on the hud - X alignment
//-----------------------------------------------------------------------------
void CHudLua::HudTimer(const char *pszIdentifier, int iX, int iY, float flValue, float flSpeed, int iAlign)
{
	int iWidth = 0;

	int iProperXPosition = 0;
	int scaledX = scheme()->GetProportionalScaledValue( iX );
	int scaledW = scheme()->GetProportionalScaledValue( iWidth );
	switch (iAlign)
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

	HudTimer(pszIdentifier, iProperXPosition, iY, flValue, flSpeed);
}

//-----------------------------------------------------------------------------
// Purpose: Create a new timer on the hud - X & Y alignment
//-----------------------------------------------------------------------------
void CHudLua::HudTimer(const char *pszIdentifier, int iX, int iY, float flValue, float flSpeed, int iAlignX, int iAlignY)
{
	int iWidth = 0;
	int iHeight = 0;

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

	HudTimer(pszIdentifier, iProperXPosition, iProperYPosition, flValue, flSpeed);
}

//-----------------------------------------------------------------------------
// Purpose: Find a hud element for a particular identifier
//-----------------------------------------------------------------------------
Panel *CHudLua::GetHudElement(const char *pszIdentifier, HudElementType_t iType, bool bCreateNew)
{
	// Return existing one
	for (int i = 0; i < m_nHudElements; i++)
	{
		if (iType == m_sHudElements[i].iType && Q_strcmp(pszIdentifier, m_sHudElements[i].szIdentifier) == 0)
			return m_sHudElements[i].pPanel;
	}

	// We don't want to create a new one
	if (!bCreateNew)
		return NULL;

	// Don't create a new one if we've reached max elements
	if (m_nHudElements == MAX_HUD_ELEMENTS)
	{
		AssertMsg(0, "MAX_HUD_ELEMENTS reached!");
		return NULL;
	}

	Panel *pPanel = NULL;

	// Return a new one of the correct type
	switch (iType)
	{
	case HUD_ICON:
		pPanel = new ImagePanel(this, pszIdentifier);
		break;

	case HUD_ICON_ALIGNXY:
		pPanel = new ImagePanel(this, pszIdentifier);
		break;

	case HUD_TEXT:
		pPanel = new Label(this, pszIdentifier, "");
		break;

	case HUD_TIMER:
		pPanel = new Timer(this, pszIdentifier);
		break;
	}

	// It's possible that because of memory problems we couldn't
	// create this hud element
	if (!pPanel)
		return NULL;

	// Store tracking data about this hud element
	m_sHudElements[m_nHudElements].pPanel = pPanel;
	m_sHudElements[m_nHudElements].iType = iType;
	Q_strncpy(m_sHudElements[m_nHudElements].szIdentifier, pszIdentifier, 127);

	m_nHudElements++;

	return pPanel;
}

//-----------------------------------------------------------------------------
// Purpose: Remove (hide) any elements with this name
//-----------------------------------------------------------------------------
void CHudLua::RemoveElement(const char *pszIdentifier)
{
	for (int i = 0; i < m_nHudElements; i++)
	{
		if (Q_strcmp(pszIdentifier, m_sHudElements[i].szIdentifier) == 0)
			m_sHudElements[i].pPanel->SetVisible(false);
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
