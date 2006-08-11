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

	if (xPos < 0 || yPos < 0)
		return;

	switch (wType)
	{
	case HUD_ICON:
		{
			char szSource[256];
			if (!msg.ReadString(szSource, 255))
				return;

			HudIcon(szIdentifier, xPos, yPos, szSource);

			break;
		}

	case HUD_TEXT:
		{
			char szText[256];
			if (!msg.ReadString(szText, 255))
				return;

			HudText(szIdentifier, xPos, yPos, szText);

			break;
		}

	case HUD_TIMER:
		{
			int		iValue = msg.ReadShort();
			float	flSpeed = msg.ReadFloat();

			HudTimer(szIdentifier, xPos, yPos, iValue, flSpeed);

			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Create a new icon on the hud
//-----------------------------------------------------------------------------
void CHudLua::HudIcon(const char *pszIdentifier, int iX, int iY, const char *pszSource)
{
	// Create or find the correct hud element
	ImagePanel *pImagePanel = dynamic_cast<ImagePanel *> (GetHudElement(pszIdentifier, HUD_ICON));

	if (!pImagePanel)
		return;

	// Yo mirv: Think this is a good idea?
	// Assume x & y are in 640 x 480 so we need to scale so
	// use the proportional scale thingy

	// Now set this label up
	pImagePanel->SetPos( scheme()->GetProportionalScaledValue( iX ), scheme()->GetProportionalScaledValue( iY ) );
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