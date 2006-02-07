//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#define PROTECTED_THINGS_DISABLE

#include <vgui/IPanel.h>
#include <vgui/IInput.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui/IVGui.h>

#include <vgui_controls/Controls.h>
#include <vgui_controls/MenuButton.h>
#include <vgui_controls/Menu.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
MenuButton::MenuButton(Panel *parent, const char *panelName, const char *text) : Button(parent, panelName, text)
{
	SetUseCaptureMouse(false);
	SetButtonActivationType(ACTIVATE_ONPRESSED);
	m_pMenu = NULL;
	m_iDirection = DOWN;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
MenuButton::~MenuButton() 
{
}

//-----------------------------------------------------------------------------
// Purpose: attaches a menu to the menu button
//-----------------------------------------------------------------------------
void MenuButton::SetMenu(Menu *menu)
{
	m_pMenu = menu;

	if (menu)
	{
		m_pMenu->SetVisible(false);
		m_pMenu->AddActionSignalTarget(this);
		m_pMenu->SetParent(this);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Never draw a focus border
//-----------------------------------------------------------------------------
void MenuButton::DrawFocusBorder(int tx0, int ty0, int tx1, int ty1)
{
}

//-----------------------------------------------------------------------------
// Purpose: Sets the direction from the menu button the menu should open
//-----------------------------------------------------------------------------
void MenuButton::SetOpenDirection(MenuDirection_e direction)
{
	m_iDirection = direction;
}

//-----------------------------------------------------------------------------
// Purpose: hides the menu
//-----------------------------------------------------------------------------
void MenuButton::HideMenu(void)
{
	if (!m_pMenu)
		return;

	// hide the menu
	m_pMenu->SetVisible(false);

	// unstick the button
	BaseClass::ForceDepressed(false);
	Repaint();

	OnHideMenu(m_pMenu);
}

//-----------------------------------------------------------------------------
// Purpose: Called when the menu button loses focus; hides the menu
//-----------------------------------------------------------------------------
void MenuButton::OnKillFocus()
{
	if ( !m_pMenu->HasFocus() )
	{
		HideMenu();
	}
	BaseClass::OnKillFocus();
}

//-----------------------------------------------------------------------------
// Purpose: Called when the menu is closed
//-----------------------------------------------------------------------------
void MenuButton::OnMenuClose()
{
	HideMenu();
	PostActionSignal(new KeyValues("MenuClose"));
}

//-----------------------------------------------------------------------------
// Purpose: Sets the offset from where menu would normally be placed
//			Only is used if menu is ALIGN_WITH_PARENT
//-----------------------------------------------------------------------------
void MenuButton::SetOpenOffsetY(int yOffset)
{
	_openOffsetY = yOffset;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool MenuButton::CanBeDefaultButton(void)
{
    return false;
}

//-----------------------------------------------------------------------------
// Purpose: Handles hotkey accesses
//-----------------------------------------------------------------------------
void MenuButton::DoClick()
{
	if ( !m_pMenu )
	{
		return;
	}

	// menu is already visible, hide the menu
	if (m_pMenu->IsVisible())
	{
		HideMenu();
		return;
	}

	// do nothing if menu is not enabled
	if (!m_pMenu->IsEnabled())
	{
		return;
	}
	// force the menu to Think
	m_pMenu->PerformLayout();

	// move the menu to the correct place below the button
	int x, y, wide, tall;;
	GetSize(wide, tall);

	if (m_iDirection == CURSOR)
	{
		// force the menu to appear where the mouse button was pressed
		input()->GetCursorPos(x, y);
	}
	else if (m_iDirection == ALIGN_WITH_PARENT && GetVParent())
	{
	   x = 0, y = tall;
	   ParentLocalToScreen(x, y);
	   x -= 1; // take border into account
	   y += _openOffsetY;
	}
	else
	{
		x = 0, y = tall;
		LocalToScreen(x, y);
	}

	int mwide, mtall, bwide, btall;
	m_pMenu->GetSize(mwide, mtall);
	GetSize(bwide, btall);

	switch (m_iDirection)
	{
	case UP:
		y -= mtall;
		y -= btall;
		m_pMenu->SetPos(x, y - 1);
		break;

	case DOWN:
		m_pMenu->SetPos(x, y + 1);
		break;

	case LEFT:
	case RIGHT:
	default:
		m_pMenu->SetPos(x + 1, y + 1);
		break;
	};

	// make sure we're on the screen
	int wx, wy, ww, wt, mx, my, mw, mt;
	m_pMenu->GetBounds(mx, my, mw, mt);
	surface()->GetWorkspaceBounds(wx, wy, ww, wt);

	if (mx < wx)
	{
		// we're off the left, move the menu to the right
		mx = wx;
	}
	if (mx + mw > wx + ww)
	{
		// we're off the right, move the menu left
		mx = wx + ww - mw;
	}
	m_pMenu->SetBounds(mx, my, mw, mt);

	// make sure we're at the top of the draw order (and therefore our children as well)
	MoveToFront();

	// notify
	OnShowMenu(m_pMenu);

	// keep the button depressed
	BaseClass::ForceDepressed(true);

	// show the menu
	m_pMenu->SetVisible(true);

	// bring to focus
	m_pMenu->RequestFocus();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void MenuButton::OnKeyCodeTyped(KeyCode code)
{
	bool shift = (input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT));
	bool ctrl = (input()->IsKeyDown(KEY_LCONTROL) || input()->IsKeyDown(KEY_RCONTROL));
	bool alt = (input()->IsKeyDown(KEY_LALT) || input()->IsKeyDown(KEY_RALT));

	if (!shift && !ctrl && !alt)
	{
		switch (code)
		{
		case KEY_ENTER:
			{
				DoClick();
				break;
			}
		}
	}
	BaseClass::OnKeyCodeTyped(code);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void MenuButton::OnCursorEntered()
{
	Button::OnCursorEntered();
	// post a message to the parent menu.
	// forward the message on to the parent of this menu.
	KeyValues *msg = new KeyValues ("CursorEnteredMenuButton");
	// tell the parent this menuitem is the one that was entered so it can open the menu if it wants
	msg->SetInt("VPanel", GetVPanel());
	ivgui()->PostMessage(GetVParent(), msg, NULL);
}
