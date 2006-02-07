//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <stdio.h>
#include <assert.h>
#include <UtlVector.h>
#include <vstdlib/IKeyValuesSystem.h>

#include <vgui/IBorder.h>
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <KeyValues.h>
#include <vgui/MouseCode.h>

#include <vgui_controls/Panel.h>
#include <vgui_controls/BuildGroup.h>
#include <vgui_controls/Tooltip.h>
#include <vgui_controls/PHandle.h>
#include <vgui_controls/Controls.h>

#include "UtlDict.h"
#include "MemPool.h"

#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
Panel::Panel()
{
	Init(0, 0, 64, 24);
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
Panel::Panel(Panel *parent)
{
	Init(0, 0, 64, 24);
	SetParent(parent);
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
Panel::Panel(Panel *parent, const char *panelName)
{
	Init(0, 0, 64, 24);
	SetName(panelName);
	SetParent(parent);
	SetBuildModeEditable(true);
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
Panel::Panel(Panel *parent, const char *panelName, HScheme scheme)
{
	Init(0, 0, 64, 24);
	SetName(panelName);
	SetParent(parent);
	SetBuildModeEditable(true);
	SetScheme( scheme );
}

//-----------------------------------------------------------------------------
// Purpose: Setup
//-----------------------------------------------------------------------------
void Panel::Init(int x,int y,int wide,int tall)
{
	// get ourselves an internal panel
	_vpanel = ivgui()->AllocPanel();
	ipanel()->Init(_vpanel, this);

	SetPos(x, y);
	SetSize(wide, tall);
	_needsRepaint = false;
	_markedForDeletion = false;
	_autoDelete = true;
	_autoResizeDirection = AUTORESIZE_NO;
	_pinCorner = PIN_TOPLEFT;

	_cursor = dc_arrow;
	_needsLayout = true;
	_needsSchemeUpdate = true;
	m_bNeedsDefaultSettingsApplied = true;
	_border = null;
	_buildGroup = null;
	_paintBorderEnabled = true;
	_paintBackgroundEnabled = true;
	_paintEnabled = true;
	_postChildPaintEnabled=false;
	_panelName = NULL;
	_tabPosition = 0;
	m_bProportional = false;
	m_iScheme = 0;

	_buildModeFlags = 0; // not editable or deletable in buildmode dialog by default

	m_pTooltips = NULL;

	m_bInPerformLayout = false;

	m_flAlpha = 255.0f;
	m_nPaintBackgroundType = 0;
	m_nBgTextureId1 = -1;
	m_nBgTextureId2 = -1;
	m_nBgTextureId3 = -1;
	m_nBgTextureId4 = -1;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
Panel::~Panel()
{
	_autoDelete = false;
	_markedForDeletion = true;

	// remove panel from any list
	SetParent((VPANEL)NULL);

	// Stop our children from pointing at us, and delete them if possible
	while (ipanel()->GetChildCount(GetVPanel()))
	{
		VPANEL child = ipanel()->GetChild(GetVPanel(), 0);
		if (ipanel()->IsAutoDeleteSet(child))
		{
			ipanel()->DeletePanel(child);
		}
		else
		{
			ipanel()->SetParent(child, NULL);
		}
	}

	// free our name
	delete [] _panelName;

	// delete VPanel
	ivgui()->FreePanel(_vpanel);
	_vpanel = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: fully construct this panel so its ready for use right now (i.e fonts loaded, colors set, default label text set, ...)
//-----------------------------------------------------------------------------
void Panel::MakeReadyForUse()
{
//	PerformApplySchemeSettings();
	surface()->SolveTraverse( GetVPanel(), true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::SetName( const char *panelName )
{
	if (_panelName)
	{
		delete [] _panelName;
		_panelName = NULL;
	}

	if (panelName)
	{
		int len = Q_strlen(panelName) + 1;
		_panelName = new char[ len ];
		Q_strncpy( _panelName, panelName, len );
	}
}

//-----------------------------------------------------------------------------
// Purpose: returns the given name of the panel
//-----------------------------------------------------------------------------
const char *Panel::GetName()
{
	if (_panelName)
		return _panelName;

	return "";
}

//-----------------------------------------------------------------------------
// Purpose: returns the name of the module that this instance of panel was compiled into
//-----------------------------------------------------------------------------
const char *Panel::GetModuleName()
{
	return vgui::GetControlsModuleName();
}

//-----------------------------------------------------------------------------
// Purpose: returns the classname of the panel (as specified in the panelmaps)
//-----------------------------------------------------------------------------
const char *Panel::GetClassName()
{
	// loop up the panel map name
	PanelMessageMap *panelMap = GetMessageMap();
	if ( panelMap )
	{
		return panelMap->pfnClassName();
	}

	return "Panel";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::SetPos(int x, int y)
{
	ipanel()->SetPos(GetVPanel(), x, y);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::GetPos(int &x, int &y)
{
	ipanel()->GetPos(GetVPanel(), x, y);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::SetSize(int wide, int tall)
{
	ipanel()->SetSize(GetVPanel(), wide, tall);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::GetSize(int &wide, int &tall)
{
	ipanel()->GetSize(GetVPanel(), wide, tall);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::SetBounds(int x, int y, int wide, int tall)
{
	SetPos(x,y);
	SetSize(wide,tall);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::GetBounds(int &x, int &y, int &wide, int &tall)
{
	GetPos(x, y);
	GetSize(wide, tall);
}

//-----------------------------------------------------------------------------
// Purpose: returns safe handle to parent
//-----------------------------------------------------------------------------
VPANEL Panel::GetVParent()
{
	return ipanel()->GetParent(GetVPanel());
}

//-----------------------------------------------------------------------------
// Purpose: returns a pointer to a controls version of a Panel pointer
//-----------------------------------------------------------------------------
Panel *Panel::GetParent()
{
	// get the parent and convert it to a Panel *
	// this is OK, the hierarchy is guaranteed to be all from the same module, except for the root node
	// the root node always returns NULL when a GetParent() is done so everything is OK
	VPANEL parent = ipanel()->GetParent(GetVPanel());
	if (parent)
	{
		Panel *pParent = ipanel()->GetPanel(parent, GetControlsModuleName());
		Assert(!pParent || !strcmp(pParent->GetModuleName(), GetControlsModuleName()));
		return pParent;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Screen size change notification handler
//-----------------------------------------------------------------------------
void Panel::OnScreenSizeChanged(int nOldWide, int nOldTall)
{
	// post to all children
	for (int i = 0; i < ipanel()->GetChildCount(GetVPanel()); i++)
	{
		VPANEL child = ipanel()->GetChild(GetVPanel(), i);
		PostMessage(child, new KeyValues("OnScreenSizeChanged", "oldwide", nOldWide, "oldtall", nOldTall), NULL);
	}

	// make any currently fullsize window stay fullsize
	int x, y, wide, tall;
	GetBounds(x, y, wide, tall);
	int screenWide, screenTall;
	surface()->GetScreenSize(screenWide, screenTall);
	if (x == 0 && y == 0 && nOldWide == wide && tall == nOldTall)
	{
		// fullsize
		surface()->GetScreenSize(wide, tall);
		SetBounds(0, 0, wide, tall);
	}

	// panel needs to re-get it's scheme settings
	_needsSchemeUpdate = true;

	// invalidate our settings
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::SetVisible(bool state)
{
	ipanel()->SetVisible(GetVPanel(), state);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool Panel::IsVisible()
{
	return ipanel()->IsVisible(GetVPanel());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::SetEnabled(bool state)
{
	if (state != ipanel()->IsEnabled( GetVPanel()))
	{
		ipanel()->SetEnabled(GetVPanel(), state);
		InvalidateLayout(false);
		Repaint();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool Panel::IsEnabled()
{
	return ipanel()->IsEnabled(GetVPanel());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool Panel::IsPopup()
{
	return ipanel()->IsPopup(GetVPanel());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::Repaint()
{
	_needsRepaint = true;
	surface()->Invalidate(GetVPanel());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::Think()
{
	if (IsVisible())
	{	
		// update any tooltips
		if (m_pTooltips)
		{
			m_pTooltips->PerformLayout();
		}
		if (_needsLayout)
		{
			InternalPerformLayout();
		}
	}

	OnThink();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::PaintTraverse(bool repaint, bool allowForce)
{
	if(!IsVisible())
	{
		return;
	}

	if(_needsRepaint && allowForce)
	{
		repaint=true;
	}

	if ( allowForce )
	{
		_needsRepaint=false;
	}

	int clipRect[4];
	ipanel()->GetClipRect(GetVPanel(), clipRect[0], clipRect[1], clipRect[2], clipRect[3]);

	if ((clipRect[2] <= clipRect[0]) || (clipRect[3] <= clipRect[1]))
	{
		repaint = false;
	}

	// set global alpha
	float oldAlphaMultiplier = surface()->DrawGetAlphaMultiplier();
	float newAlphaMultiplier = oldAlphaMultiplier * (m_flAlpha / 255.0f);
	surface()->DrawSetAlphaMultiplier(newAlphaMultiplier);

	if (repaint && (_paintBorderEnabled || _paintBackgroundEnabled || _paintEnabled))
	{
		// draw the background with no inset
		surface()->PushMakeCurrent(GetVPanel(), /* useInsets => */false);
		if (_paintBackgroundEnabled)
		{
			PaintBackground();
		}
		surface()->PopMakeCurrent(GetVPanel());

		// draw the front of the panel with the inset
		surface()->PushMakeCurrent(GetVPanel(), /* useInsets => */true);
		if (_paintEnabled)
		{
			Paint();
		}
		surface()->PopMakeCurrent(GetVPanel());
	}

	// traverse and paint all our children
	int childCount = ipanel()->GetChildCount(GetVPanel());
	for (int i = 0; i < childCount; i++)
	{
		VPANEL child = ipanel()->GetChild(GetVPanel(), i);
		if (surface()->ShouldPaintChildPanel(child))
		{
			ipanel()->PaintTraverse(child, repaint, allowForce);
		}
		else
		{
			// Invalidate the child panel so that it gets redrawn
			surface()->Invalidate(child);
			// keep traversing the tree, just don't allow anyone to paint after here
			ipanel()->PaintTraverse(child, false, false);
		}
	}

	// draw the border last
	if (repaint && _paintBorderEnabled)
	{
		// Paint the border over the background
		if (_border != null)
		{
			surface()->PushMakeCurrent(GetVPanel(), /* useInsets => */false);
			PaintBorder();
			surface()->PopMakeCurrent(GetVPanel());
		}
	}

	if (repaint)
	{
		if (IsBuildGroupEnabled() )//&& HasFocus())
		{
			// outline all selected panels 
			CUtlVector<PHandle> *controlGroup = _buildGroup->GetControlGroup();
			for (int i=0; i < controlGroup->Size(); ++i)
			{
				surface()->PushMakeCurrent(((*controlGroup)[i].Get())->GetVPanel(), false);
				((*controlGroup)[i].Get())->PaintBuildOverlay();
				surface()->PopMakeCurrent(((*controlGroup)[i].Get())->GetVPanel());
			}	
			
			_buildGroup->DrawRulers();						
		}			
	}

	// All of our children have painted, etc, now allow painting in top of them
	//  if desired
	if ( repaint && _postChildPaintEnabled )
	{
		surface()->PushMakeCurrent(GetVPanel(), false);
		PostChildPaint();
		surface()->PopMakeCurrent(GetVPanel());
	}

	surface()->DrawSetAlphaMultiplier(oldAlphaMultiplier);

	surface()->SwapBuffers(GetVPanel());
}



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::PaintBorder()
{
	_border->Paint(GetVPanel());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::PaintBackground()
{ 
	int wide, tall;
	GetSize(wide,tall);
	Color col = GetBgColor();

	switch ( m_nPaintBackgroundType )
	{
	default:
	case 0:
		{
			surface()->DrawSetColor(col);
			surface()->DrawFilledRect(0, 0, wide, tall);
		}
		break;
	case 1:
		{
			DrawTexturedBox( 0, 0, wide, tall, col, 1.0f );
		}
		break;
	case 2:
		{
			DrawBox( 0, 0, wide, tall, col, 1.0f );
		}
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::Paint()
{
	// empty on purpose
	// PaintBackground is painted and default behavior is for Paint to do nothing
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::PostChildPaint()
{
	// Empty on purpose
	// This is called if _postChildPaintEnabled is true and allows painting to
	//  continue on the surface after all of the panel's children have painted 
	//  themselves.  Allows drawing an overlay on top of the children, etc.
}

//-----------------------------------------------------------------------------
// Purpose: Draws a black rectangle around the panel.
//-----------------------------------------------------------------------------
void Panel::PaintBuildOverlay()
{
	int wide,tall;
	GetSize(wide,tall);
	surface()->DrawSetColor(0, 0, 0, 255);

	surface()->DrawFilledRect(0,0,wide,2);           //top
	surface()->DrawFilledRect(0,tall-2,wide,tall);   //bottom
	surface()->DrawFilledRect(0,2,2,tall-2);         //left
	surface()->DrawFilledRect(wide-2,2,wide,tall-2); //right
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the panel's draw code will fully cover it's area
//-----------------------------------------------------------------------------
bool Panel::IsOpaque()
{
	if (IsVisible() && _paintBackgroundEnabled && _bgColor[3] == 255)
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the settings are aligned to the right of the screen
//-----------------------------------------------------------------------------
bool Panel::IsRightAligned()
{
	return (_buildModeFlags & BUILDMODE_SAVE_XPOS_RIGHTALIGNED);
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the settings are aligned to the bottom of the screen
//-----------------------------------------------------------------------------
bool Panel::IsBottomAligned()
{
	return (_buildModeFlags & BUILDMODE_SAVE_YPOS_BOTTOMALIGNED);
}

//-----------------------------------------------------------------------------
// Purpose: sets the parent
//-----------------------------------------------------------------------------
void Panel::SetParent(Panel *newParent)
{
	// Assert that the parent is from the same module as the child
	// FIXME: !!! work out how to handle this properly!
//	Assert(!newParent || !strcmp(newParent->GetModuleName(), GetControlsModuleName()));

	if (newParent)
	{
		SetParent(newParent->GetVPanel());
	}
	else
	{
		SetParent((VPANEL)NULL);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::SetParent(VPANEL newParent)
{
	if (newParent)
	{
		ipanel()->SetParent(GetVPanel(), newParent);
	}
	else
	{
		ipanel()->SetParent(GetVPanel(), NULL);
	}

	if (GetVParent() && !IsPopup())
	{
		SetProportional(ipanel()->IsProportional(GetVParent()));

		// most of the time KBInput == parents kbinput
		if (ipanel()->IsKeyBoardInputEnabled(GetVParent()) != IsKeyBoardInputEnabled())
		{
			SetKeyBoardInputEnabled(ipanel()->IsKeyBoardInputEnabled(GetVParent()));
		}

		if (ipanel()->IsMouseInputEnabled(GetVParent()) != IsMouseInputEnabled())
		{
			SetMouseInputEnabled(ipanel()->IsMouseInputEnabled(GetVParent()));
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::OnChildAdded(VPANEL child)
{
	Assert( !m_bInPerformLayout );
}

//-----------------------------------------------------------------------------
// Purpose: default message handler
//-----------------------------------------------------------------------------
void Panel::OnSizeChanged(int newWide, int newTall)
{
	InvalidateLayout(); // our size changed so force us to layout again
}

//-----------------------------------------------------------------------------
// Purpose: sets Z ordering - lower numbers are always behind higher z's
//-----------------------------------------------------------------------------
void Panel::SetZPos(int z)
{
	ipanel()->SetZPos(GetVPanel(), z);
}

//-----------------------------------------------------------------------------
// Purpose: sets alpha modifier for panel and all child panels [0..255]
//-----------------------------------------------------------------------------
void Panel::SetAlpha(int alpha)
{
	m_flAlpha = alpha;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
int Panel::GetAlpha()
{
	return (int)m_flAlpha;
}

//-----------------------------------------------------------------------------
// Purpose: Moves the panel to the front of the z-order
//-----------------------------------------------------------------------------
void Panel::MoveToFront(void)
{
	// FIXME: only use ipanel() as per src branch?
	if (IsPopup())
	{
		surface()->BringToFront(GetVPanel());
	}
	else
	{
		ipanel()->MoveToFront(GetVPanel());
	}
}

//-----------------------------------------------------------------------------
// Purpose: Iterates up the hierarchy looking for a particular parent
//-----------------------------------------------------------------------------
bool Panel::HasParent(VPANEL potentialParent)
{
	if (!potentialParent)
		return false;

	return ipanel()->HasParent(GetVPanel(), potentialParent);
}

//-----------------------------------------------------------------------------
// Purpose: Finds a child panel by string name
// Output : Panel * - NULL if no panel of that name is found
//-----------------------------------------------------------------------------
Panel *Panel::FindChildByName(const char *childName, bool recurseDown)
{
	for (int i = 0; i < GetChildCount(); i++)
	{
		Panel *pChild = GetChild(i);
		if (!pChild)
			continue;

		if (!stricmp(pChild->GetName(), childName))
		{
			return pChild;
		}

		if (recurseDown)
		{
			Panel *panel = pChild->FindChildByName(childName, recurseDown);
			if ( panel )
			{
				return panel;
			}
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Finds a sibling panel by name
//-----------------------------------------------------------------------------
Panel *Panel::FindSiblingByName(const char *siblingName)
{
	int siblingCount = ipanel()->GetChildCount(GetVParent());
	for (int i = 0; i < siblingCount; i++)
	{
		VPANEL sibling = ipanel()->GetChild(GetVParent(), i);
		Panel *panel = ipanel()->GetPanel(sibling, GetControlsModuleName());
		if (!stricmp(panel->GetName(), siblingName))
		{
			return panel;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Dispatches immediately a message to the parent
//-----------------------------------------------------------------------------
void Panel::CallParentFunction(KeyValues *message)
{
	if (GetVParent())
	{
		ipanel()->SendMessage(GetVParent(), message, GetVPanel());
	}
	if (message)
	{
		message->deleteThis();
	}
}

//-----------------------------------------------------------------------------
// Purpose: if set to true, panel automatically frees itself when parent is deleted
//-----------------------------------------------------------------------------
void Panel::SetAutoDelete(bool state)
{
	_autoDelete = state;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool Panel::IsAutoDeleteSet()
{
	return _autoDelete;
}

//-----------------------------------------------------------------------------
// Purpose: Just calls 'delete this'
//-----------------------------------------------------------------------------
void Panel::DeletePanel()
{
	delete this;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
HScheme Panel::GetScheme()
{
	if (m_iScheme)
	{
		return m_iScheme; // return our internal scheme
	}
	
	if (GetVParent()) // recurse down the heirarchy 
	{
		return ipanel()->GetScheme(GetVParent());
	}

	return scheme()->GetDefaultScheme();
}

//-----------------------------------------------------------------------------
// Purpose: set the scheme to render this panel with by name
//-----------------------------------------------------------------------------
void Panel::SetScheme(const char *tag)
{
	if (strlen(tag) > 0 && scheme()->GetScheme(tag)) // check the scheme exists
	{
		SetScheme(scheme()->GetScheme(tag));
	}
}

//-----------------------------------------------------------------------------
// Purpose: set the scheme to render this panel with 
//-----------------------------------------------------------------------------
void Panel::SetScheme(HScheme scheme)
{
	if (scheme != m_iScheme)
	{
		m_iScheme = scheme;

		// This will cause the new scheme to be applied at a later point
//		InvalidateLayout( false, true );
	}
}


//-----------------------------------------------------------------------------
// Purpose: returns the char of this panels hotkey
//-----------------------------------------------------------------------------
Panel *Panel::HasHotkey(wchar_t key)
{
	return NULL;
}

void Panel::InternalCursorMoved(int x, int y)
{
	if (IsCursorNone() || !IsMouseInputEnabled())
		return;

	if (IsBuildGroupEnabled())
	{
		_buildGroup->CursorMoved(x, y, this);
		return;
	}
	
	if (m_pTooltips)
	{
		m_pTooltips->ShowTooltip(this);
	}

	ScreenToLocal(x, y);

	OnCursorMoved(x, y);
}

void Panel::InternalCursorEntered()
{
	if (IsCursorNone() || !IsMouseInputEnabled())
		return;
	
	if (IsBuildGroupEnabled())
		return;

	if (m_pTooltips)
	{
		m_pTooltips->ResetDelay();
		m_pTooltips->ShowTooltip(this);
	}

	OnCursorEntered();
}

void Panel::InternalCursorExited()
{
	if (IsCursorNone() || !IsMouseInputEnabled())
		return;
	
	if (IsBuildGroupEnabled())
		return;

	if (m_pTooltips)
	{
		m_pTooltips->HideTooltip();
	}

	OnCursorExited();
}

void Panel::InternalMousePressed(int code)
{
	if (IsCursorNone() || !IsMouseInputEnabled())
		return;
	
	if (IsBuildGroupEnabled())
	{
		_buildGroup->MousePressed((MouseCode)code, this);
		return;
	}
	
	OnMousePressed((MouseCode)code);
}

void Panel::InternalMouseDoublePressed(int code)
{
	if (IsCursorNone() || !IsMouseInputEnabled())
		return;
	
	if (IsBuildGroupEnabled())
	{
		_buildGroup->MouseDoublePressed((MouseCode)code, this);
		return;
	}

	OnMouseDoublePressed((MouseCode)code);
}

void Panel::InternalMouseReleased(int code)
{
	if (IsCursorNone() || !IsMouseInputEnabled())
		return;
	
	if (IsBuildGroupEnabled())
	{
		_buildGroup->MouseReleased((MouseCode)code, this);
		return;
	}

	OnMouseReleased((MouseCode)code);
}

void Panel::InternalMouseWheeled(int delta)
{
	if (IsBuildGroupEnabled() || !IsMouseInputEnabled())
	{
		return;
	}

	OnMouseWheeled(delta);
}

void Panel::InternalKeyCodePressed(int code)
{
	if (IsKeyBoardInputEnabled()) 
	{
		OnKeyCodePressed((KeyCode)code);
	}
	else
	{
		CallParentFunction(new KeyValues("KeyCodePressed", "code", code));
	}
}

void Panel::InternalKeyCodeTyped(int code)
{
	if (IsKeyBoardInputEnabled()) 
	{
		if (IsBuildGroupEnabled())
		{
			_buildGroup->KeyCodeTyped((KeyCode)code, this);
			return;
		}
		
		OnKeyCodeTyped((KeyCode)code);
	}
	else
	{
		CallParentFunction(new KeyValues("KeyCodeTyped", "code", code));
	}
}

void Panel::InternalKeyTyped(int unichar)
{
	if (IsKeyBoardInputEnabled())
	{
		if (IsBuildGroupEnabled())
			return;

		OnKeyTyped((wchar_t)unichar);
	}
	else
	{
		CallParentFunction(new KeyValues("KeyTyped", "unichar", unichar));
	}
}

void Panel::InternalKeyCodeReleased(int code)
{
	if (IsKeyBoardInputEnabled()) 
	{
		OnKeyCodeReleased((KeyCode)code);
	}
	else
	{
		CallParentFunction(new KeyValues("KeyCodeReleased", "code", code));
	}
}

void Panel::InternalKeyFocusTicked()
{
	if (IsBuildGroupEnabled())
		return;
	
	OnKeyFocusTicked();
}

void Panel::InternalMouseFocusTicked()
{
	if (IsBuildGroupEnabled())
	{
		// must repaint so the numbers will be accurate
		if (_buildGroup->HasRulersOn())
		{
			PaintTraverse(true);
		}
		return;
	}

	// update cursor
	InternalSetCursor();
	OnMouseFocusTicked();
}


void Panel::InternalSetCursor()
{
	bool visible = IsVisible();

	if (visible)
	{
		// chain up and make sure all our parents are also visible
		VPANEL p = GetVParent();
		while (p)
		{
			visible &= ipanel()->IsVisible(p);
			p = ipanel()->GetParent(p);
		}
	
		// only change the cursor if this panel is visible, and if its part of the main VGUI tree
		if (visible && HasParent(surface()->GetEmbeddedPanel())) 
		{	
			HCursor cursor = GetCursor();
			
			if (IsBuildGroupEnabled())
			{
				cursor = _buildGroup->GetCursor(this);
			}
			
			if (input()->GetCursorOveride())
			{
				cursor = input()->GetCursorOveride();
			}

			surface()->SetCursor(cursor);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame the panel is visible, designed to be overridden
//-----------------------------------------------------------------------------
void Panel::OnThink()
{
}

// input messages handlers (designed for override)
void Panel::OnCursorMoved(int x, int y)
{
}

void Panel::OnCursorEntered()
{
}

void Panel::OnCursorExited()
{
}

void Panel::OnMousePressed(MouseCode code)
{
}

void Panel::OnMouseDoublePressed(MouseCode code)
{
}

void Panel::OnMouseReleased(MouseCode code)
{
}

void Panel::OnMouseWheeled(int delta)
{
	CallParentFunction(new KeyValues("MouseWheeled", "delta", delta));
}

// base implementation forwards Key messages to the Panel's parent - override to 'swallow' the input
void Panel::OnKeyCodePressed(KeyCode code)
{
	CallParentFunction(new KeyValues("KeyCodePressed", "code", code));
}

void Panel::OnKeyCodeTyped(KeyCode code)
{
	// handle focus change
	if (code == KEY_TAB)
	{
		// if shift is down goto previous tab position, otherwise goto next
		if (input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT))
		{
			RequestFocusPrev();
		}
		else
		{
			RequestFocusNext();
		}
	}
	else
	{
		// forward up
		CallParentFunction(new KeyValues("KeyCodeTyped", "code", code));
	}
}

void Panel::OnKeyTyped(wchar_t unichar)
{
	CallParentFunction(new KeyValues("KeyTyped", "unichar", unichar));
}

void Panel::OnKeyCodeReleased(KeyCode code)
{
	CallParentFunction(new KeyValues("KeyCodeReleased", "code", code));
}

void Panel::OnKeyFocusTicked()
{
	CallParentFunction(new KeyValues("KeyFocusTicked"));
}

void Panel::OnMouseFocusTicked()
{
	CallParentFunction(new KeyValues("OnMouseFocusTicked"));
}

bool Panel::IsWithin(int x,int y)
{
	// check against our clip rect
	int clipRect[4];
	ipanel()->GetClipRect(GetVPanel(), clipRect[0], clipRect[1], clipRect[2], clipRect[3]);

	if (x < clipRect[0])
	{
		return false;
	}
	
	if (y < clipRect[1])
	{
		return false;
	}

	if (x >= clipRect[2])
	{
		return false;
	}
	
	if (y >= clipRect[3])
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: determines which is the topmost panel under the coordinates (x, y)
//-----------------------------------------------------------------------------
VPANEL Panel::IsWithinTraverse(int x, int y, bool traversePopups)
{
	// if this one is not visible, its children won't be either
	// also if it doesn't want mouse input its children can't get it either
	if (!IsVisible() || !IsMouseInputEnabled())
		return NULL;

	if (traversePopups)
	{
		// check popups first
		for (int i = GetChildCount() - 1; i >= 0; i--)
		{
			VPANEL panel = ipanel()->GetChild(GetVPanel(), i);
			if (ipanel()->IsPopup(panel))
			{
				panel = ipanel()->IsWithinTraverse(panel, x, y, true);
				if (panel != null)
				{
					return panel;
				}
			}
		}

		// check children recursive, if you find one, just return first one
		// this checks in backwards order so the last child drawn for this panel is chosen which
		// coincides to how it would be visibly displayed
		for (i = GetChildCount() - 1; i >= 0; i--)
		{
			VPANEL panel = ipanel()->GetChild(GetVPanel(), i);
			// we've already checked popups so ignore
			if (!ipanel()->IsPopup(panel))
			{
				panel = ipanel()->IsWithinTraverse(panel, x, y, true);
				if (panel != NULL)
				{
					return panel;
				}
			}
		}
		
		// check ourself
		if (IsWithin(x, y))
		{
			return GetVPanel();
		}
	}
	else
	{
		// since we're not checking popups, it must be within us, so we can check ourself first
		if (IsWithin(x, y))
		{
			// check children recursive, if you find one, just return first one
			// this checks in backwards order so the last child drawn for this panel is chosen which
			// coincides to how it would be visibly displayed
			for (int i = GetChildCount() - 1; i >= 0; i--)
			{
				VPANEL panel = ipanel()->GetChild(GetVPanel(), i);
				// ignore popups
				if (!ipanel()->IsPopup(panel))
				{
					panel = ipanel()->IsWithinTraverse(panel, x, y, false);
					if (panel != NULL)
					{
						return panel;
					}
				}
			}
	
			// not a child, must be us
			return GetVPanel();
		}
	}

	return NULL;
}

void Panel::LocalToScreen(int& x,int& y)
{
	int px, py;
	ipanel()->GetAbsPos(GetVPanel(), px, py);

	x = x + px;
	y = y + py;
}

void Panel::ScreenToLocal(int& x,int& y)
{
	int px, py;
	ipanel()->GetAbsPos(GetVPanel(), px, py);

	x = x - px;
	y = y - py;
}

void Panel::ParentLocalToScreen(int &x, int &y)
{
	int px, py;
	ipanel()->GetAbsPos(GetVParent(), px, py);

	x = x + px;
	y = y + py;
}

void Panel::MakePopup(bool showTaskbarIcon,bool disabled)
{
	surface()->CreatePopup(GetVPanel(), false, showTaskbarIcon,disabled);
}

void Panel::SetCursor(HCursor cursor)
{
	_cursor = cursor;
}

HCursor Panel::GetCursor()
{
	return _cursor;
}

void Panel::SetMinimumSize(int wide,int tall)
{
	ipanel()->SetMinimumSize(GetVPanel(), wide, tall);
}

void Panel::GetMinimumSize(int& wide,int &tall)
{
	ipanel()->GetMinimumSize(GetVPanel(), wide, tall);
}

bool Panel::IsBuildModeEditable()
{
   return true;
}

void Panel::SetBuildModeEditable(bool state)
{
	if (state)
	{
		_buildModeFlags |= BUILDMODE_EDITABLE;
	}
	else
	{
		_buildModeFlags &= ~BUILDMODE_EDITABLE;
	}
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
bool Panel::IsBuildModeDeletable()
{
	return (_buildModeFlags & BUILDMODE_DELETABLE);
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
void Panel::SetBuildModeDeletable(bool state)
{
	if (state)
	{
		_buildModeFlags |= BUILDMODE_DELETABLE;
	}
	else
	{
		_buildModeFlags &= ~BUILDMODE_DELETABLE;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool Panel::IsBuildModeActive()
{
	return _buildGroup ? _buildGroup->IsEnabled() : false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::GetClipRect(int& x0,int& y0,int& x1,int& y1)
{
	ipanel()->GetClipRect(GetVPanel(), x0, y0, x1, y1);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int Panel::GetChildCount()
{
	return ipanel()->GetChildCount(GetVPanel());
}

//-----------------------------------------------------------------------------
// Purpose: returns a child by the specified index
//-----------------------------------------------------------------------------
Panel *Panel::GetChild(int index)
{
	// get the child and cast it to a panel
	// this assumes that the child is from the same module as the this (precondition)
	return ipanel()->GetPanel(ipanel()->GetChild(GetVPanel(), index), GetControlsModuleName());
}

//-----------------------------------------------------------------------------
// Purpose: moves the key focus back
//-----------------------------------------------------------------------------
bool Panel::RequestFocusPrev(VPANEL panel)
{
	// chain to parent
	if (GetVParent())
	{
		return ipanel()->RequestFocusPrev(GetVParent(), GetVPanel());
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool Panel::RequestFocusNext(VPANEL panel)
{
	// chain to parent
	if (GetVParent())
	{
		return ipanel()->RequestFocusNext(GetVParent(), GetVPanel());
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Sets the panel to have the current sub focus
// Input  : direction - the direction in which focus travelled to arrive at this panel; forward = 1, back = -1
//-----------------------------------------------------------------------------
void Panel::RequestFocus(int direction)
{
//	ivgui()->DPrintf2("RequestFocus(%s, %s)\n", GetName(), GetClassName());
	OnRequestFocus(GetVPanel(), NULL);
}

//-----------------------------------------------------------------------------
// Purpose: Called after a panel requests focus to fix up the whole chain
//-----------------------------------------------------------------------------
void Panel::OnRequestFocus(VPANEL subFocus, VPANEL defaultPanel)
{
	CallParentFunction(new KeyValues("OnRequestFocus", "subFocus", subFocus, "defaultPanel", defaultPanel));
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
VPANEL Panel::GetCurrentKeyFocus()
{
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the panel has focus
//-----------------------------------------------------------------------------
bool Panel::HasFocus()
{
	if (input()->GetFocus() == GetVPanel())
	{
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::SetTabPosition(int position)
{
	_tabPosition = position;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int Panel::GetTabPosition()
{
	return _tabPosition;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::InternalFocusChanged(bool lost)
{
/*
	//if focus is gained tell the focusNavGroup about it so its current can be correct
	if( (!lost) && (_focusNavGroup!=null) )
	{
		_focusNavGroup->setCurrentPanel(this);
	}
*/
}

//-----------------------------------------------------------------------------
// Purpose: Called when a panel loses it's mouse capture
//-----------------------------------------------------------------------------
void Panel::OnMouseCaptureLost()
{
	if (m_pTooltips)
	{
		m_pTooltips->ResetDelay();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::AddActionSignalTarget(Panel *messageTarget)
{
	HPanel target = ivgui()->PanelToHandle(messageTarget->GetVPanel());
	if (!_actionSignalTargetDar.HasElement(target))
	{
		_actionSignalTargetDar.AddElement(target);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::AddActionSignalTarget(VPANEL messageTarget)
{
	HPanel target = ivgui()->PanelToHandle(messageTarget);
	if (!_actionSignalTargetDar.HasElement(target))
	{
		_actionSignalTargetDar.AddElement(target);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::RemoveActionSignalTarget(Panel *oldTarget)
{
	_actionSignalTargetDar.RemoveElement(ivgui()->PanelToHandle(oldTarget->GetVPanel()));
}

//-----------------------------------------------------------------------------
// Purpose: Sends a message to all the panels that have requested action signals
//-----------------------------------------------------------------------------
void Panel::PostActionSignal( KeyValues *message )
{
	// add who it was from the message
	message->SetPtr("panel", this);

	for (int i = _actionSignalTargetDar.GetCount() - 1; i > 0; i--)
	{
		VPANEL panel = ivgui()->HandleToPanel(_actionSignalTargetDar[i]);
		if (panel)
		{
			ivgui()->PostMessage(panel, message->MakeCopy(), GetVPanel());
		}
	}

	// do this so we can save on one MakeCopy() call
	if (i == 0)
	{
		VPANEL panel = ivgui()->HandleToPanel(_actionSignalTargetDar[i]);
		if (panel)
		{
			ivgui()->PostMessage(panel, message, GetVPanel());
		}
	}
	else
	{
		message->deleteThis();
	}
}

void Panel::SetBorder(IBorder *border)
{
	_border = border;

	if (border)
	{
		int x, y, x2, y2;
		border->GetInset(x, y, x2, y2);
		ipanel()->SetInset(GetVPanel(), x, y, x2, y2);

		// update our background type based on the bord
		SetPaintBackgroundType(border->GetBackgroundType());
	}
	else
	{
		ipanel()->SetInset(GetVPanel(), 0, 0, 0, 0);
	}
}

IBorder *Panel::GetBorder()
{
	return _border;
}


void Panel::SetPaintBorderEnabled(bool state)
{
	_paintBorderEnabled=state;
}

void Panel::SetPaintBackgroundEnabled(bool state)
{
	_paintBackgroundEnabled=state;
}

void Panel::SetPaintBackgroundType( int type )
{
	// HACK only 0 through 2 supported for now
	m_nPaintBackgroundType = clamp( type, 0, 2 );
}

void Panel::SetPaintEnabled(bool state)
{
	_paintEnabled=state;
}

void Panel::SetPostChildPaintEnabled(bool state)
{
	_postChildPaintEnabled = state;
}

void Panel::GetInset(int& left,int& top,int& right,int& bottom)
{
	ipanel()->GetInset(GetVPanel(), left, top, right, bottom);
}

void Panel::GetPaintSize(int& wide,int& tall)
{
	GetSize(wide, tall);
	if (_border != null)
	{
		int left,top,right,bottom;
		_border->GetInset(left,top,right,bottom);

		wide -= (left+right);
		tall -= (top+bottom);
	}
}

int Panel::GetWide()
{
	int wide, tall;
	ipanel()->GetSize(GetVPanel(), wide, tall);
	return wide;
}

void Panel::SetWide(int wide)
{
	ipanel()->SetSize(GetVPanel(), wide, GetTall());
}

int Panel::GetTall()
{
	int wide, tall;
	ipanel()->GetSize(GetVPanel(), wide, tall);
	return tall;
}

void Panel::SetTall(int tall)
{
	ipanel()->SetSize(GetVPanel(), GetWide(), tall);
}

void Panel::SetBuildGroup(BuildGroup* buildGroup)
{
	//TODO: remove from old group

	Assert(buildGroup != NULL);
	
	_buildGroup = buildGroup;

	_buildGroup->PanelAdded(this);
}

bool Panel::IsBuildGroupEnabled()
{
	if (_buildGroup == NULL)
	{
		return false;
	}

	return _buildGroup->IsEnabled();
}

void Panel::SetBgColor(Color color)
{
	_bgColor = color;
}

void Panel::SetFgColor(Color color)
{
	_fgColor = color;
}

Color Panel::GetBgColor()
{
	return _bgColor;
}

Color Panel::GetFgColor()
{
	return _fgColor;
}

void Panel::InternalPerformLayout()
{
	m_bInPerformLayout = true;
	// make sure the scheme has been applied
	_needsLayout = false;
	PerformLayout();
	m_bInPerformLayout = false;
}

void Panel::PerformLayout()
{
	// this should be overridden to relayout controls
}

void Panel::InvalidateLayout( bool layoutNow, bool reloadScheme )
{
	_needsLayout = true;
	
	if (reloadScheme)
	{
		// make all our children reload the scheme
		_needsSchemeUpdate = true;
	
		for (int i = 0; i < GetChildCount(); i++)
		{
			GetChild(i)->InvalidateLayout(layoutNow, true);
		}
		
		PerformApplySchemeSettings();
	}
	
	if (layoutNow)
	{
		InternalPerformLayout();
		Repaint();
	}
}

bool Panel::IsCursorNone()
{
	HCursor cursor = GetCursor();

	if (!cursor)
	{
		return true;
	}
	
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the cursor is currently over the panel
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool Panel::IsCursorOver(void)
{
	int x, y;
	input()->GetCursorPos(x, y);
	return IsWithin(x, y);
}

//-----------------------------------------------------------------------------
// Purpose: Called when a panel receives a command message from another panel
//-----------------------------------------------------------------------------
void Panel::OnCommand(const char *command)
{
	// design for override, do nothing
}

//-----------------------------------------------------------------------------
// Purpose: panel gained focus message
//-----------------------------------------------------------------------------
void Panel::OnSetFocus()
{
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: panel lost focus message
//-----------------------------------------------------------------------------
void Panel::OnKillFocus()
{
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Sets the object up to be deleted next frame
//-----------------------------------------------------------------------------
void Panel::MarkForDeletion()
{
	if (_markedForDeletion)
		return;

	_markedForDeletion = true;
	_autoDelete = false;

	if (ivgui()->IsRunning())
	{
		ivgui()->MarkPanelForDeletion(GetVPanel());
	}
	else
	{
		delete this;
	}
}

//-----------------------------------------------------------------------------
// Purpose: return true if this object require a perform layout
//-----------------------------------------------------------------------------
bool Panel::IsLayoutInvalid()
{
	return _needsLayout;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
void Panel::SetPinCorner(PinCorner_e pinCorner)
{
	_pinCorner = pinCorner;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
Panel::PinCorner_e Panel::GetPinCorner()
{
	return _pinCorner;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
void Panel::SetAutoResize(AutoResize_e resizeDir)
{
	_autoResizeDirection = resizeDir;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
Panel::AutoResize_e Panel::GetAutoResize()
{
	return _autoResizeDirection;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::ApplySchemeSettings(IScheme *pScheme)
{
	// get colors
	SetFgColor(GetSchemeColor("Panel.FgColor", pScheme));
	SetBgColor(GetSchemeColor("Panel.BgColor", pScheme));

	// mark us as no longer needing scheme settings applied
	_needsSchemeUpdate = false;

	if (IsBuildGroupEnabled())
	{
		_buildGroup->ApplySchemeSettings(pScheme);
		return;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Checks to see if the panel needs it's scheme info setup
//-----------------------------------------------------------------------------
void Panel::PerformApplySchemeSettings()
{
	if (m_bNeedsDefaultSettingsApplied)
	{
		InternalInitDefaultValues( GetAnimMap() );
	}

	if (_needsSchemeUpdate)
	{
		VPROF( "ApplySchemeSettings" );
		IScheme *pScheme = scheme()->GetIScheme( GetScheme() );
		AssertOnce( pScheme );
		if ( pScheme ) // this should NEVER be null, but if it is bad things would happen in ApplySchemeSettings...
		{
			ApplySchemeSettings( pScheme );
			//_needsSchemeUpdate = false;	
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Loads panel details from the resource info
//-----------------------------------------------------------------------------
void Panel::ApplySettings(KeyValues *inResourceData)
{
	// First restore to default values
	if (m_bNeedsDefaultSettingsApplied)
	{
		InternalInitDefaultValues( GetAnimMap() );
	}

	// Let PanelAnimationVars auto-retrieve settings (we restore defaults above
	//  since a script might be missing certain values)
	InternalApplySettings( GetAnimMap(), inResourceData );

	// clear any alignment flags
	_buildModeFlags &= ~(BUILDMODE_SAVE_XPOS_RIGHTALIGNED | BUILDMODE_SAVE_XPOS_CENTERALIGNED | BUILDMODE_SAVE_YPOS_BOTTOMALIGNED | BUILDMODE_SAVE_YPOS_CENTERALIGNED);

	// get the position
	int screenWide, screenTall;
	surface()->GetScreenSize(screenWide, screenTall);
	int x, y;
	GetPos(x, y);
	const char *xstr = inResourceData->GetString( "xpos", NULL );
	const char *ystr = inResourceData->GetString( "ypos", NULL );
	if (xstr)
	{
		// look for alignment flags
		if (xstr[0] == 'r' || xstr[0] == 'R')
		{
			_buildModeFlags |= BUILDMODE_SAVE_XPOS_RIGHTALIGNED;
			xstr++;
		}
		else if (xstr[0] == 'c' || xstr[0] == 'C')
		{
			_buildModeFlags |= BUILDMODE_SAVE_XPOS_CENTERALIGNED;
			xstr++;
		}

		// get the value
		x = atoi(xstr);

		// scale the x up to our screen co-ords
		if (IsProportional())
		{
			x = scheme()->GetProportionalScaledValue(x);
		}

		// now correct the alignment
		if (_buildModeFlags & BUILDMODE_SAVE_XPOS_RIGHTALIGNED)
		{
			x = screenWide - x; 
		}
		else if (_buildModeFlags & BUILDMODE_SAVE_XPOS_CENTERALIGNED)
		{
			x = (screenWide / 2) + x;
		}
	}

	if (ystr)
	{
		// look for alignment flags
		if (ystr[0] == 'r' || ystr[0] == 'R')
		{
			_buildModeFlags |= BUILDMODE_SAVE_YPOS_BOTTOMALIGNED;
			ystr++;
		}
		else if (ystr[0] == 'c' || ystr[0] == 'C')
		{
			_buildModeFlags |= BUILDMODE_SAVE_YPOS_CENTERALIGNED;
			ystr++;
		}
		y = atoi(ystr);
		if (IsProportional())
		{
			// scale the y up to our screen co-ords
			y = scheme()->GetProportionalScaledValue(y);
		}
		// now correct the alignment
		if (_buildModeFlags & BUILDMODE_SAVE_YPOS_BOTTOMALIGNED)
		{
			y = screenTall - y; 
		}
		else if (_buildModeFlags & BUILDMODE_SAVE_YPOS_CENTERALIGNED)
		{
			y = (screenTall / 2) + y;
		}
	}
	SetPos(x, y);

	if (inResourceData->FindKey( "zpos" ))
	{
		SetZPos( inResourceData->GetInt( "zpos" ) );
	}

	// size
	int wide, tall;
	GetSize( wide, tall );
	wide = inResourceData->GetInt( "wide", wide );
	tall = inResourceData->GetInt( "tall", tall );
	if(IsProportional())
	{
		// scale the x and y up to our screen co-ords
		wide = scheme()->GetProportionalScaledValue(wide);
		tall = scheme()->GetProportionalScaledValue(tall);
	}
	
	SetSize( wide, tall );

	SetAutoResize((AutoResize_e)inResourceData->GetInt("AutoResize"));
	SetPinCorner((PinCorner_e)inResourceData->GetInt("PinCorner"));

	// only get colors if we're ignoring the scheme
	if (inResourceData->GetInt("IgnoreScheme", 0))
	{
		PerformApplySchemeSettings();
	}

	// state
	int state = inResourceData->GetInt("visible", 1);
	if (state == 0)
	{
		SetVisible(false);
	}
	else if (state == 1)
	{
		SetVisible(true);
	}

	SetEnabled( inResourceData->GetInt("enabled", true) );

	// tab order
	SetTabPosition(inResourceData->GetInt("tabPosition", 0));

	// tab order
	const char *tooltip = inResourceData->GetString("tooltiptext", NULL);
	if (tooltip && *tooltip)
	{
		GetTooltip()->SetText(tooltip);
	}

	// paint background?
	int nPaintBackground = inResourceData->GetInt("paintbackground", -1);
	if (nPaintBackground >= 0)
	{
		SetPaintBackgroundEnabled( nPaintBackground != 0 );
	}

	// paint border?
	int nPaintBorder = inResourceData->GetInt("paintborder", -1);
	if (nPaintBorder >= 0)
	{
		SetPaintBorderEnabled( nPaintBorder != 0 );
	}

	// check to see if we have a new name assigned
	const char *newName = inResourceData->GetString("fieldName", NULL);
	if (newName)
	{
		SetName(newName);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Saves out a resource description of this panel
//-----------------------------------------------------------------------------
void Panel::GetSettings( KeyValues *outResourceData )
{
	// control class name (so it can be recreated later if needed)
	outResourceData->SetString( "ControlName", GetClassName() );

	// name
	outResourceData->SetString( "fieldName", _panelName );
	
	// positioning
	int screenWide, screenTall;
	surface()->GetScreenSize(screenWide, screenTall);
	int x, y;
	GetPos( x, y );
	if ( IsProportional() )
	{
		x = scheme()->GetProportionalNormalizedValue(x);
		y = scheme()->GetProportionalNormalizedValue(y);
	}
	// correct for alignment
	if (_buildModeFlags & BUILDMODE_SAVE_XPOS_RIGHTALIGNED)
	{
		x = screenWide - x;
		char xstr[32];
		Q_snprintf(xstr, sizeof( xstr ), "r%d", x);
		outResourceData->SetString( "xpos", xstr );
	}
	else if (_buildModeFlags & BUILDMODE_SAVE_XPOS_CENTERALIGNED)
	{
		x = (screenWide / 2) + x;
		char xstr[32];
		Q_snprintf(xstr, sizeof( xstr ), "c%d", x);
		outResourceData->SetString( "xpos", xstr );
	}
	else
	{
		outResourceData->SetInt( "xpos", x );
	}
	if (_buildModeFlags & BUILDMODE_SAVE_YPOS_BOTTOMALIGNED)
	{
		y = screenTall - y;
		char ystr[32];
		Q_snprintf(ystr, sizeof( ystr ), "r%d", y);
		outResourceData->SetString( "ypos", ystr );
	}
	else if (_buildModeFlags & BUILDMODE_SAVE_YPOS_CENTERALIGNED)
	{
		y = (screenTall / 2) + y;
		char ystr[32];
		Q_snprintf(ystr, sizeof( ystr ), "c%d", y);
		outResourceData->SetString( "ypos", ystr );
	}
	else
	{
		outResourceData->SetInt( "ypos", y );
	}

	if (m_pTooltips)
	{
		if (strlen(m_pTooltips->GetText()) > 0)
		{
			outResourceData->SetString("tooltiptext", m_pTooltips->GetText());
		}
	}

	int wide, tall;
	GetSize( wide, tall );
	if (IsProportional())
	{
		wide = scheme()->GetProportionalNormalizedValue(wide);
		tall = scheme()->GetProportionalNormalizedValue(tall);
	}

	int z = ipanel()->GetZPos(GetVPanel());
	if (z)
	{
		outResourceData->SetInt("zpos", z);
	}

	outResourceData->SetInt( "wide", wide );
	outResourceData->SetInt( "tall", tall );

	outResourceData->SetInt("AutoResize", GetAutoResize());
	outResourceData->SetInt("PinCorner", GetPinCorner());

	// state
	outResourceData->SetInt( "visible", IsVisible() );
	outResourceData->SetInt( "enabled", IsEnabled() );

	outResourceData->SetInt( "tabPosition", GetTabPosition() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Color Panel::GetSchemeColor(const char *keyName, IScheme *pScheme)
{
	return pScheme->GetColor(keyName, Color(255, 255, 255, 255));
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Color Panel::GetSchemeColor(const char *keyName, Color defaultColor, IScheme *pScheme)
{
	return pScheme->GetColor(keyName, defaultColor);
}

//-----------------------------------------------------------------------------
// Purpose: Returns a string description of the panel fields for use in the UI
//-----------------------------------------------------------------------------
const char *Panel::GetDescription( void )
{
	static const char *panelDescription = "string fieldName, int xpos, int ypos, int wide, int tall, bool visible, bool enabled, int tabPosition, corner pinCorner, autoresize autoResize, string tooltiptext";
	return panelDescription;
}

//-----------------------------------------------------------------------------
// Purpose: user configuration settings
//			this is used for any control details the user wants saved between sessions
//			eg. dialog positions, last directory opened, list column width
//-----------------------------------------------------------------------------
void Panel::ApplyUserConfigSettings(KeyValues *userConfig)
{
}

//-----------------------------------------------------------------------------
// Purpose: returns user config settings for this control
//-----------------------------------------------------------------------------
void Panel::GetUserConfigSettings(KeyValues *userConfig)
{
}

//-----------------------------------------------------------------------------
// Purpose: optimization, return true if this control has any user config settings
//-----------------------------------------------------------------------------
bool Panel::HasUserConfigSettings()
{
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::InternalInvalidateLayout()
{
	InvalidateLayout(false, false);
}

//-----------------------------------------------------------------------------
// Purpose: called whenever the panel moves
//-----------------------------------------------------------------------------
void Panel::OnMove()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Panel::InternalMove()
{
	OnMove();
	for(int i=0;i<GetChildCount();i++)
	{
		// recursively apply to all children
		GetChild(i)->OnMove();
	}
}

//-----------------------------------------------------------------------------
// Purpose: empty function
//-----------------------------------------------------------------------------
void Panel::OnTick()
{
}

//-----------------------------------------------------------------------------
// Purpose: versioning
//-----------------------------------------------------------------------------
void *Panel::QueryInterface(EInterfaceID id)
{
	if (id == ICLIENTPANEL_STANDARD_INTERFACE)
	{
		return this;
	}

	return NULL;
}


//-----------------------------------------------------------------------------
// Purpose: Map all the base messages to functions
//			ordering from most -> least used improves speed
//-----------------------------------------------------------------------------
MessageMapItem_t Panel::m_MessageMap[] =
{
	MAP_MESSAGE_INT( Panel, "RequestFocus", RequestFocus, "direction" )
};

// IMPLEMENT_PANELMAP( Panel, NULL )
PanelMap_t Panel::m_PanelMap = { Panel::m_MessageMap, ARRAYSIZE(Panel::m_MessageMap), "Panel", NULL };
PanelMap_t *Panel::GetPanelMap( void ) { return &m_PanelMap; }

//-----------------------------------------------------------------------------
// Purpose: !! Soon to replace existing prepare panel map
//-----------------------------------------------------------------------------
void PreparePanelMessageMap(PanelMessageMap *panelMap)
{
	// iterate through the class hierarchy message maps
	while ( panelMap != NULL && !panelMap->processed )
	{
		// hash message map strings into symbols
		for (int i = 0; i < panelMap->entries.Count(); i++)
		{
			MessageMapItem_t *item = &panelMap->entries[i];

			if (item->name)
			{
				item->nameSymbol = KeyValuesSystem()->GetSymbolForString(item->name);
			}
			else
			{
				item->nameSymbol = INVALID_KEY_SYMBOL;
			}
			if (item->firstParamName)
			{
				item->firstParamSymbol = KeyValuesSystem()->GetSymbolForString(item->firstParamName);
			}
			else
			{
				item->firstParamSymbol = INVALID_KEY_SYMBOL;
			}
			if (item->secondParamName)
			{
				item->secondParamSymbol = KeyValuesSystem()->GetSymbolForString(item->secondParamName);
			}
			else
			{
				item->secondParamSymbol = INVALID_KEY_SYMBOL;
			}
		}
		
		panelMap->processed = true;
		panelMap = panelMap->baseMap;
	}
}



//-----------------------------------------------------------------------------
// Purpose: Handles a message
//			Dispatches the message to a set of message maps
//-----------------------------------------------------------------------------
void Panel::OnMessage(const KeyValues *params, VPANEL ifromPanel)
{
	PanelMessageMap *panelMap = GetMessageMap();
	bool bFound = false;
	int iMessageName = params->GetNameSymbol();

	if ( !panelMap->processed )
	{
		PreparePanelMessageMap( panelMap );
	}

	// iterate through the class hierarchy message maps
	for ( ; panelMap != NULL && !bFound; panelMap = panelMap->baseMap )
	{
		// iterate all the entries in the panel map
		for ( int i = 0; i < panelMap->entries.Count(); i++ )
		{
			MessageMapItem_t *pMap = &panelMap->entries[i];

			if (iMessageName == pMap->nameSymbol)
			{
				bFound = true;

				switch (pMap->numParams)
				{
				case 0:
				{
					(this->*(pMap->func))();
					break;
				}
		
				case 1:
				{
					KeyValues *param1 = params->FindKey(pMap->firstParamSymbol);
					if (!param1)
					{
						param1 = const_cast<KeyValues *>(params);
					}

					switch ( pMap->firstParamType )
					{
						case DATATYPE_INT:
							typedef void (Panel::*MessageFunc_Int_t)(int);
							(this->*((MessageFunc_Int_t)pMap->func))( param1->GetInt() );
							break;

						case DATATYPE_PTR:
							typedef void (Panel::*MessageFunc_Ptr_t)( void * );
							(this->*((MessageFunc_Ptr_t)pMap->func))( param1->GetPtr() );
							break;

						case DATATYPE_FLOAT:
							typedef void (Panel::*MessageFunc_Float_t)( float );
							(this->*((MessageFunc_Float_t)pMap->func))( param1->GetFloat() );
							break;

						case DATATYPE_CONSTCHARPTR:
							typedef void (Panel::*MessageFunc_CharPtr_t)( const char * );
							(this->*((MessageFunc_CharPtr_t)pMap->func))( param1->GetString() );
							break;

						case DATATYPE_CONSTWCHARPTR:
							typedef void (Panel::*MessageFunc_WCharPtr_t)( const wchar_t * );
							(this->*((MessageFunc_WCharPtr_t)pMap->func))( param1->GetWString() );
							break;

						case DATATYPE_KEYVALUES:
							typedef void (Panel::*MessageFunc_KeyValues_t)(KeyValues *);
							if ( pMap->firstParamName )
							{
								(this->*((MessageFunc_KeyValues_t)pMap->func))( (KeyValues *)param1->GetPtr() );
							}
							else
							{
								// no param set, so pass in the whole thing
								(this->*((MessageFunc_KeyValues_t)pMap->func))( const_cast<KeyValues *>(params) );
							}
							break;

						default:
							Assert(!("No handler for vgui message function"));
							break;
					}
					break;
				}

				case 2:
				{
					KeyValues *param1 = params->FindKey(pMap->firstParamSymbol);
					if (!param1)
					{
						param1 = const_cast<KeyValues *>(params);
					}
					KeyValues *param2 = params->FindKey(pMap->secondParamSymbol);
					if (!param2)
					{
						param2 = const_cast<KeyValues *>(params);
					}

					if ( (DATATYPE_INT == pMap->firstParamType) && (DATATYPE_INT == pMap->secondParamType) )
					{
						typedef void (Panel::*MessageFunc_IntInt_t)(int, int);
						(this->*((MessageFunc_IntInt_t)pMap->func))( param1->GetInt(), param2->GetInt() );
					}
					else if ( (DATATYPE_PTR == pMap->firstParamType) && (DATATYPE_INT == pMap->secondParamType) )
					{
						typedef void (Panel::*MessageFunc_PtrInt_t)(void *, int);
						(this->*((MessageFunc_PtrInt_t)pMap->func))( param1->GetPtr(), param2->GetInt() );
					}
					else if ( (DATATYPE_CONSTCHARPTR == pMap->firstParamType) && (DATATYPE_INT == pMap->secondParamType) )
					{
						typedef void (Panel::*MessageFunc_ConstCharPtrInt_t)(const char *, int);
						(this->*((MessageFunc_ConstCharPtrInt_t)pMap->func))( param1->GetString(), param2->GetInt() );
					}
					else if ( (DATATYPE_CONSTCHARPTR == pMap->firstParamType) && (DATATYPE_CONSTCHARPTR == pMap->secondParamType) )
					{
						typedef void (Panel::*MessageFunc_ConstCharPtrConstCharPtr_t)(const char *, const char *);
						(this->*((MessageFunc_ConstCharPtrConstCharPtr_t)pMap->func))( param1->GetString(), param2->GetString() );
					}
					else if ( (DATATYPE_INT == pMap->firstParamType) && (DATATYPE_CONSTCHARPTR == pMap->secondParamType) )
					{
						typedef void (Panel::*MessageFunc_IntConstCharPtr_t)(int, const char *);
						(this->*((MessageFunc_IntConstCharPtr_t)pMap->func))( param1->GetInt(), param2->GetString() );
					}
					else if ( (DATATYPE_PTR == pMap->firstParamType) && (DATATYPE_CONSTCHARPTR == pMap->secondParamType) )
					{
						typedef void (Panel::*MessageFunc_PtrConstCharPtr_t)(void *, const char *);
						(this->*((MessageFunc_PtrConstCharPtr_t)pMap->func))( param1->GetPtr(), param2->GetString() );
					}
					else if ( (DATATYPE_PTR == pMap->firstParamType) && (DATATYPE_CONSTWCHARPTR == pMap->secondParamType) )
					{
						typedef void (Panel::*MessageFunc_PtrConstCharPtr_t)(void *, const wchar_t *);
						(this->*((MessageFunc_PtrConstCharPtr_t)pMap->func))( param1->GetPtr(), param2->GetWString() );
					}
					else
					{
						// the message isn't handled
						ivgui()->DPrintf( "Message '%s', sent to '%s', has invalid parameter types\n", params->GetName(), GetName() );
					}
					break;
				}

				default:
					Assert(!("Invalid number of parameters"));
					break;
				}

				// break the loop
				bFound = true;
				break;
			}
		}
	}

	if (!bFound)
	{
		OnOldMessage(const_cast<KeyValues *>(params), ifromPanel);
	}
}

void Panel::OnOldMessage(KeyValues *params, VPANEL ifromPanel)
{
	bool bFound = false;
	// message map dispatch
	int iMessageName = params->GetNameSymbol();

	PanelMap_t *panelMap = GetPanelMap();
	if ( !panelMap->processed )
	{
		PreparePanelMap( panelMap );
	}

	// iterate through the class hierarchy message maps
	for ( ; panelMap != NULL && !bFound; panelMap = panelMap->baseMap )
	{
		MessageMapItem_t *pMessageMap = panelMap->dataDesc;

		for ( int i = 0; i < panelMap->dataNumFields; i++ )
		{
			if (iMessageName == pMessageMap[i].nameSymbol)
			{
				// call the mapped function
				switch ( pMessageMap[i].numParams )
				{
				case 2:
					if ( (DATATYPE_INT == pMessageMap[i].firstParamType) && (DATATYPE_INT == pMessageMap[i].secondParamType) )
					{
						typedef void (Panel::*MessageFunc_IntInt_t)(int, int);
						(this->*((MessageFunc_IntInt_t)pMessageMap[i].func))( params->GetInt(pMessageMap[i].firstParamName), params->GetInt(pMessageMap[i].secondParamName) );
					}
					else if ( (DATATYPE_PTR == pMessageMap[i].firstParamType) && (DATATYPE_INT == pMessageMap[i].secondParamType) )
					{
						typedef void (Panel::*MessageFunc_PtrInt_t)(void *, int);
						(this->*((MessageFunc_PtrInt_t)pMessageMap[i].func))( params->GetPtr(pMessageMap[i].firstParamName), params->GetInt(pMessageMap[i].secondParamName) );
					}
					else if ( (DATATYPE_CONSTCHARPTR == pMessageMap[i].firstParamType) && (DATATYPE_INT == pMessageMap[i].secondParamType) )
					{
						typedef void (Panel::*MessageFunc_ConstCharPtrInt_t)(const char *, int);
						(this->*((MessageFunc_ConstCharPtrInt_t)pMessageMap[i].func))( params->GetString(pMessageMap[i].firstParamName), params->GetInt(pMessageMap[i].secondParamName) );
					}
					else if ( (DATATYPE_CONSTCHARPTR == pMessageMap[i].firstParamType) && (DATATYPE_CONSTCHARPTR == pMessageMap[i].secondParamType) )
					{
						typedef void (Panel::*MessageFunc_ConstCharPtrConstCharPtr_t)(const char *, const char *);
						(this->*((MessageFunc_ConstCharPtrConstCharPtr_t)pMessageMap[i].func))( params->GetString(pMessageMap[i].firstParamName), params->GetString(pMessageMap[i].secondParamName) );
					}
					else if ( (DATATYPE_INT == pMessageMap[i].firstParamType) && (DATATYPE_CONSTCHARPTR == pMessageMap[i].secondParamType) )
					{
						typedef void (Panel::*MessageFunc_IntConstCharPtr_t)(int, const char *);
						(this->*((MessageFunc_IntConstCharPtr_t)pMessageMap[i].func))( params->GetInt(pMessageMap[i].firstParamName), params->GetString(pMessageMap[i].secondParamName) );
					}
					else if ( (DATATYPE_PTR == pMessageMap[i].firstParamType) && (DATATYPE_CONSTCHARPTR == pMessageMap[i].secondParamType) )
					{
						typedef void (Panel::*MessageFunc_PtrConstCharPtr_t)(void *, const char *);
						(this->*((MessageFunc_PtrConstCharPtr_t)pMessageMap[i].func))( params->GetPtr(pMessageMap[i].firstParamName), params->GetString(pMessageMap[i].secondParamName) );
					}
					else if ( (DATATYPE_PTR == pMessageMap[i].firstParamType) && (DATATYPE_CONSTWCHARPTR == pMessageMap[i].secondParamType) )
					{
						typedef void (Panel::*MessageFunc_PtrConstCharPtr_t)(void *, const wchar_t *);
						(this->*((MessageFunc_PtrConstCharPtr_t)pMessageMap[i].func))( params->GetPtr(pMessageMap[i].firstParamName), params->GetWString(pMessageMap[i].secondParamName) );
					}
					else
					{
						// the message isn't handled
						ivgui()->DPrintf( "Message '%s', sent to '%s', has invalid parameter types\n", params->GetName(), GetName() );
					}
					break;

				case 1:
					switch ( pMessageMap[i].firstParamType )
					{
					case DATATYPE_BOOL:
						typedef void (Panel::*MessageFunc_Bool_t)(bool);
						(this->*((MessageFunc_Bool_t)pMessageMap[i].func))( (bool)params->GetInt(pMessageMap[i].firstParamName) );
						break;

					case DATATYPE_CONSTCHARPTR:
						typedef void (Panel::*MessageFunc_ConstCharPtr_t)(const char *);
						(this->*((MessageFunc_ConstCharPtr_t)pMessageMap[i].func))( (const char *)params->GetString(pMessageMap[i].firstParamName) );
						break;

					case DATATYPE_CONSTWCHARPTR:
						typedef void (Panel::*MessageFunc_ConstCharPtr_t)(const char *);
						(this->*((MessageFunc_ConstCharPtr_t)pMessageMap[i].func))( (const char *)params->GetWString(pMessageMap[i].firstParamName) );
						break;

					case DATATYPE_INT:
						typedef void (Panel::*MessageFunc_Int_t)(int);
						(this->*((MessageFunc_Int_t)pMessageMap[i].func))( params->GetInt(pMessageMap[i].firstParamName) );
						break;

					case DATATYPE_FLOAT:
						typedef void (Panel::*MessageFunc_Float_t)(float);
						(this->*((MessageFunc_Float_t)pMessageMap[i].func))( params->GetFloat(pMessageMap[i].firstParamName) );
						break;

					case DATATYPE_PTR:
						typedef void (Panel::*MessageFunc_Ptr_t)(void *);
						(this->*((MessageFunc_Ptr_t)pMessageMap[i].func))( (void *)params->GetPtr(pMessageMap[i].firstParamName) );
						break;

					case DATATYPE_KEYVALUES:
						typedef void (Panel::*MessageFunc_KeyValues_t)(KeyValues *);
						if ( pMessageMap[i].firstParamName )
						{
							(this->*((MessageFunc_KeyValues_t)pMessageMap[i].func))( (KeyValues *)params->GetPtr(pMessageMap[i].firstParamName) );
						}
						else
						{
							(this->*((MessageFunc_KeyValues_t)pMessageMap[i].func))( params );
						}
						break;

					default:
						// the message isn't handled
						ivgui()->DPrintf( "Message '%s', sent to '%s', has an invalid parameter type\n", params->GetName(), GetName() );
						break;
					}

					break;

				default:
					(this->*(pMessageMap[i].func))();
					break;
				};

				// break the loop
				bFound = true;
				break;
			}
		}
	}

		// message not handled
	/* debug code
	if ( !bFound )
	{
		ivgui()->DPrintf( "Message '%s' not handled by panel '%s'\n", messageName, GetName() );
	}
	*/
}

//-----------------------------------------------------------------------------
// Purpose: Safe call to get info from child panel by name
//-----------------------------------------------------------------------------
bool Panel::RequestInfoFromChild(const char *childName, KeyValues *outputData)
{
	Panel *panel = FindChildByName(childName);
	if (panel)
	{
		return panel->RequestInfo(outputData);
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Posts a message
//-----------------------------------------------------------------------------
void Panel::PostMessage(Panel *target, KeyValues *message, float delay)
{
	ivgui()->PostMessage(target->GetVPanel(), message, GetVPanel(), delay);
}

void Panel::PostMessage(VPANEL target, KeyValues *message, float delaySeconds)
{
	ivgui()->PostMessage(target, message, GetVPanel(), delaySeconds);
}

//-----------------------------------------------------------------------------
// Purpose: Safe call to post a message to a child by name
//-----------------------------------------------------------------------------
void Panel::PostMessageToChild(const char *childName, KeyValues *message)
{
	Panel *panel = FindChildByName(childName);
	if (panel)
	{
		ivgui()->PostMessage(panel->GetVPanel(), message, GetVPanel());
	}
	else
	{
		message->deleteThis();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Requests some information from the panel
//			Look through the message map for the handler
//-----------------------------------------------------------------------------
bool Panel::RequestInfo( KeyValues *outputData )
{
	if ( InternalRequestInfo( GetAnimMap(), outputData ) )
	{
		return true;
	}

	if (GetVParent())
	{
		return ipanel()->RequestInfo(GetVParent(), outputData);
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: sets a specified value in the control - inverse of RequestInfo
//-----------------------------------------------------------------------------
bool Panel::SetInfo(KeyValues *inputData)
{
	if ( InternalSetInfo( GetAnimMap(), inputData ) )
	{
		return true;
	}

	// doesn't chain to parent
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Prepares the hierarchy panel maps for use (with message maps etc)
//-----------------------------------------------------------------------------
void Panel::PreparePanelMap( PanelMap_t *panelMap )
{
	// iterate through the class hierarchy message maps
	while ( panelMap != NULL && !panelMap->processed )
	{
		// fixup cross-dll boundary panel maps
		if ( panelMap->baseMap == (PanelMap_t*)0x00000001 )
		{
			panelMap->baseMap = &Panel::m_PanelMap;
		}

		// hash message map strings into symbols
		for (int i = 0; i < panelMap->dataNumFields; i++)
		{
			MessageMapItem_t *item = &panelMap->dataDesc[i];

			if (item->name)
			{
				item->nameSymbol = KeyValuesSystem()->GetSymbolForString(item->name);
			}
			else
			{
				item->nameSymbol = INVALID_KEY_SYMBOL;
			}
			if (item->firstParamName)
			{
				item->firstParamSymbol = KeyValuesSystem()->GetSymbolForString(item->firstParamName);
			}
			else
			{
				item->firstParamSymbol = INVALID_KEY_SYMBOL;
			}
			if (item->secondParamName)
			{
				item->secondParamSymbol = KeyValuesSystem()->GetSymbolForString(item->secondParamName);
			}
			else
			{
				item->secondParamSymbol = INVALID_KEY_SYMBOL;
			}
		}
		
		panelMap->processed = true;
		panelMap = panelMap->baseMap;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called to delete the panel
//-----------------------------------------------------------------------------
void Panel::OnDelete()
{
	Assert(_heapchk() == _HEAPOK);
	delete this;
	Assert(_heapchk() == _HEAPOK);
}

//-----------------------------------------------------------------------------
// Purpose: Panel handle implementation
//			Returns a pointer to a valid panel, NULL if the panel has been deleted
//-----------------------------------------------------------------------------
Panel *PHandle::Get() 
{
	if (m_iPanelID != INVALID_PANEL)
	{
		VPANEL panel = ivgui()->HandleToPanel(m_iPanelID);
		if (panel)
		{
			Panel *vguiPanel = ipanel()->GetPanel(panel, GetControlsModuleName());
			return vguiPanel;
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: sets the smart pointer
//-----------------------------------------------------------------------------
Panel *PHandle::Set(Panel *pent)
{
	if (pent)
	{
		m_iPanelID = ivgui()->PanelToHandle(pent->GetVPanel());
	}
	else
	{
		m_iPanelID = INVALID_PANEL;
	}
	return pent; 
}

//-----------------------------------------------------------------------------
// Purpose: Returns a handle to a valid panel, NULL if the panel has been deleted
//-----------------------------------------------------------------------------
VPANEL VPanelHandle::Get()
{
	if (m_iPanelID != INVALID_PANEL)
	{
		return ivgui()->HandleToPanel(m_iPanelID);
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: sets the smart pointer
//-----------------------------------------------------------------------------
VPANEL VPanelHandle::Set(VPANEL pent)
{
	if (pent)
	{
		m_iPanelID = ivgui()->PanelToHandle(pent);
	}
	else
	{
		m_iPanelID = INVALID_PANEL;
	}
	return pent; 
}

//-----------------------------------------------------------------------------
// Purpose: returns a pointer to the tooltip object associated with the panel
//-----------------------------------------------------------------------------
Tooltip *Panel::GetTooltip()
{
	if (!m_pTooltips)
	{
		m_pTooltips = new Tooltip(this, NULL);
	}

	return m_pTooltips;
}

//-----------------------------------------------------------------------------
// Purpose: sets the proportional flag on this panel and all it's children
//-----------------------------------------------------------------------------
void Panel::SetProportional(bool state)
{ 

	if( state != m_bProportional ) // only do something if the state changes
	{
		m_bProportional = state;	

		for(int i=0;i<GetChildCount();i++)
		{
			// recursively apply to all children
			GetChild(i)->SetProportional(IsProportional());
		}
	}
	InvalidateLayout();
}


void Panel::SetKeyBoardInputEnabled( bool state )
{
	ipanel()->SetKeyBoardInputEnabled( GetVPanel(), state );
	for ( int i = 0; i < GetChildCount(); i++ )
	{
		GetChild(i)->SetKeyBoardInputEnabled( state );
	}
	
}

void Panel::SetMouseInputEnabled( bool state )
{
	ipanel()->SetMouseInputEnabled( GetVPanel(), state );
/*	for(int i=0;i<GetChildCount();i++)
	{
		GetChild(i)->SetMouseInput(state);
	}*/
	vgui::surface()->CalculateMouseVisible();

}

bool Panel::IsKeyBoardInputEnabled()
{
	return ipanel()->IsKeyBoardInputEnabled( GetVPanel() );
}

bool Panel::IsMouseInputEnabled()
{
	return ipanel()->IsMouseInputEnabled( GetVPanel() );
}

class CFloatProperty : public vgui::IPanelAnimationPropertyConverter
{
public:
	virtual void GetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		kv->SetFloat( entry->name(), *(float *)data );
	}
	
	virtual void SetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		*(float *)data = kv->GetFloat( entry->name() );
	}

	virtual void InitFromDefault( Panel *panel, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		*(float *)data = atof( entry->defaultvalue() );
	}
};

class CProportionalFloatProperty : public vgui::IPanelAnimationPropertyConverter
{
public:
	virtual void GetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		float f = *(float *)data;
		f = scheme()->GetProportionalNormalizedValue( f );
		kv->SetFloat( entry->name(), f );
	}
	
	virtual void SetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		float f = kv->GetFloat( entry->name() );
		f = scheme()->GetProportionalScaledValue( f );
		*(float *)data = f;
	}

	virtual void InitFromDefault( Panel *panel, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		float f = atof( entry->defaultvalue() );
		f = scheme()->GetProportionalScaledValue( f );
		*(float *)data = f;
	}
};

class CIntProperty : public vgui::IPanelAnimationPropertyConverter
{
public:
	virtual void GetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		kv->SetInt( entry->name(), *(int *)data );
	}
	
	virtual void SetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		*(int *)data = kv->GetInt( entry->name() );
	}

	virtual void InitFromDefault( Panel *panel, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		*(int *)data = atoi( entry->defaultvalue() );
	}
};

class CProportionalIntProperty : public vgui::IPanelAnimationPropertyConverter
{
public:
	virtual void GetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		int i = *(int *)data;
		i = scheme()->GetProportionalNormalizedValue( i );
		kv->SetInt( entry->name(), i );
	}
	
	virtual void SetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		int i = kv->GetInt( entry->name() );
		i = scheme()->GetProportionalScaledValue( i );
		*(int *)data = i;
	}
	virtual void InitFromDefault( Panel *panel, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		int i = atoi( entry->defaultvalue() );
		i = scheme()->GetProportionalScaledValue( i );
		*(int *)data = i;
	}
};

class CColorProperty : public vgui::IPanelAnimationPropertyConverter
{
public:
	virtual void GetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		kv->SetColor( entry->name(), *(Color *)data );
	}
	
	virtual void SetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		vgui::IScheme *scheme = vgui::scheme()->GetIScheme( panel->GetScheme() );
		Assert( scheme );
		if ( scheme )
		{
			void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );

			char const *colorName = kv->GetString( entry->name() );
			if ( !colorName || !colorName[0] )
			{
				*(Color *)data = kv->GetColor( entry->name() );
			}
			else
			{
				*(Color *)data = scheme->GetColor( colorName, Color( 0, 0, 0, 0 ) );
			}
		}
	}

	virtual void InitFromDefault( Panel *panel, PanelAnimationMapEntry *entry )
	{
		vgui::IScheme *scheme = vgui::scheme()->GetIScheme( panel->GetScheme() );
		Assert( scheme );
		if ( scheme )
		{
			void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
			*(Color *)data = scheme->GetColor( entry->defaultvalue(), Color( 0, 0, 0, 0 ) );
		}
	}
};

class CBoolProperty : public vgui::IPanelAnimationPropertyConverter
{
public:
	virtual void GetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		kv->SetInt( entry->name(), *(bool *)data ? 1 : 0 );
	}
	
	virtual void SetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		*(bool *)data = kv->GetInt( entry->name() ) ? true : false;
	}

	virtual void InitFromDefault( Panel *panel, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		bool b = false;
		if ( !stricmp( entry->defaultvalue(), "true" )||
			atoi( entry->defaultvalue() )!= 0 )
		{
			b = true;
		}

		*(bool *)data = b;
	}
};

class CStringProperty : public vgui::IPanelAnimationPropertyConverter
{
public:
	virtual void GetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		kv->SetString( entry->name(), (char *)data );
	}
	
	virtual void SetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		strcpy( (char *)data, kv->GetString( entry->name() ) );
	}

	virtual void InitFromDefault( Panel *panel, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		strcpy( ( char * )data, entry->defaultvalue() );
	}
};

class CHFontProperty : public vgui::IPanelAnimationPropertyConverter
{
public:
	virtual void GetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		vgui::IScheme *scheme = vgui::scheme()->GetIScheme( panel->GetScheme() );
		Assert( scheme );
		if ( scheme )
		{
			void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
			char const *fontName = scheme->GetFontName( *(HFont *)data );
			kv->SetString( entry->name(), fontName );
		}
	}
	
	virtual void SetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		vgui::IScheme *scheme = vgui::scheme()->GetIScheme( panel->GetScheme() );
		Assert( scheme );
		if ( scheme )
		{
			void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
			char const *fontName = kv->GetString( entry->name() );
			*(HFont *)data = scheme->GetFont( fontName, panel->IsProportional() );
		}
	}

	virtual void InitFromDefault( Panel *panel, PanelAnimationMapEntry *entry )
	{
		vgui::IScheme *scheme = vgui::scheme()->GetIScheme( panel->GetScheme() );
		Assert( scheme );
		if ( scheme )
		{
			void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
			*(HFont *)data = scheme->GetFont( entry->defaultvalue(), panel->IsProportional() );
		}
	}
};

class CTextureIdProperty : public vgui::IPanelAnimationPropertyConverter
{
public:
	virtual void GetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		int currentId = *(int *)data;

		// lookup texture name for id
		char texturename[ 512 ];
		if ( currentId != -1 &&
			surface()->DrawGetTextureFile( currentId, texturename, sizeof( texturename ) ) )
		{
			kv->SetString( entry->name(), texturename );
		}
		else
		{
			kv->SetString( entry->name(), "" );
		}
	}
	
	virtual void SetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );

		int currentId = -1;

		char const *texturename = kv->GetString( entry->name() );
		if ( texturename && texturename[ 0 ] )
		{
			currentId = surface()->DrawGetTextureId( texturename );
			if ( currentId == -1 )
			{
				currentId = surface()->CreateNewTextureID();
			}
			surface()->DrawSetTextureFile( currentId, texturename, false, true );
		}

		*(int *)data = currentId;
	}

	virtual void InitFromDefault( Panel *panel, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );

		int currentId = -1;

		char const *texturename = entry->defaultvalue();
		if ( texturename && texturename[ 0 ] )
		{
			currentId = surface()->DrawGetTextureId( texturename );
			if ( currentId == -1 )
			{
				currentId = surface()->CreateNewTextureID();
			}
			surface()->DrawSetTextureFile( currentId, texturename, false, true );
		}

		*(int *)data = currentId;
	}
};

static CFloatProperty floatconverter;
static CProportionalFloatProperty p_floatconverter;
static CIntProperty intconverter;
static CProportionalIntProperty p_intconverter;
static CColorProperty colorconverter;
static CBoolProperty boolconverter;
static CStringProperty stringconverter;
static CHFontProperty fontconverter;
static CTextureIdProperty textureidconverter;

static CUtlDict< IPanelAnimationPropertyConverter *, int > g_AnimationPropertyConverters;

static IPanelAnimationPropertyConverter *FindConverter( char const *typeName )
{
	int lookup = g_AnimationPropertyConverters.Find( typeName );
	if ( lookup == g_AnimationPropertyConverters.InvalidIndex() )
		return NULL;

	IPanelAnimationPropertyConverter *converter = g_AnimationPropertyConverters[ lookup ];
	return converter;
}

void Panel::AddPropertyConverter( char const *typeName, IPanelAnimationPropertyConverter *converter )
{
	int lookup = g_AnimationPropertyConverters.Find( typeName );
	if ( lookup != g_AnimationPropertyConverters.InvalidIndex() )
	{
		Msg( "Already have converter for type %s, ignoring...\n", typeName );
		return;
	}

	g_AnimationPropertyConverters.Insert( typeName, converter );
}

//-----------------------------------------------------------------------------
// Purpose: Static method to initialize all needed converters
//-----------------------------------------------------------------------------
void Panel::InitPropertyConverters( void )
{
	static bool initialized = false;
	if ( initialized )
		return;
	initialized = true;

	AddPropertyConverter( "float", &floatconverter );
	AddPropertyConverter( "int", &intconverter );
	AddPropertyConverter( "Color", &colorconverter );
//	AddPropertyConverter( "vgui::Color", &colorconverter );
	AddPropertyConverter( "bool", &boolconverter );
	AddPropertyConverter( "char", &stringconverter );
	AddPropertyConverter( "string", &stringconverter );
	AddPropertyConverter( "HFont", &fontconverter );
	AddPropertyConverter( "vgui::HFont", &fontconverter );

	// This is an aliased type for proportional float
	AddPropertyConverter( "proportional_float", &p_floatconverter );
	AddPropertyConverter( "proportional_int", &p_intconverter );

	AddPropertyConverter( "textureid", &textureidconverter );
}

bool Panel::InternalRequestInfo( PanelAnimationMap *map, KeyValues *outputData )
{
	if ( !map )
		return false;

	Assert( outputData );

	char const *name = outputData->GetName();

	PanelAnimationMapEntry *e = FindPanelAnimationEntry( name, map );
	if ( e )
	{
		IPanelAnimationPropertyConverter *converter = FindConverter( e->type() );
		if ( converter )
		{
			converter->GetData( this, outputData, e );
			return true;
		}
	}

	return false;
}

bool Panel::InternalSetInfo( PanelAnimationMap *map, KeyValues *inputData )
{
	if ( !map )
		return false;

	Assert( inputData );

	char const *name = inputData->GetName();

	PanelAnimationMapEntry *e = FindPanelAnimationEntry( name, map );
	if ( e )
	{
		IPanelAnimationPropertyConverter *converter = FindConverter( e->type() );
		if ( converter )
		{
			converter->SetData( this, inputData, e );
			return true;
		}
	}

	return false;
}

PanelAnimationMapEntry *Panel::FindPanelAnimationEntry( char const *scriptname, PanelAnimationMap *map )
{
	if ( !map )
		return NULL;

	Assert( scriptname );

	// Look through mapping for entry
	int c = map->entries.Count();
	for ( int i = 0; i < c; i++ )
	{
		PanelAnimationMapEntry *e = &map->entries[ i ];

		if ( !stricmp( e->name(), scriptname ) )
		{
			return e;
		}
	}

	// Recurse
	if ( map->baseMap )
	{
		return FindPanelAnimationEntry( scriptname, map->baseMap );
	}

	return NULL;
}

// Recursively invoke settings for PanelAnimationVars
void Panel::InternalApplySettings( PanelAnimationMap *map, KeyValues *inResourceData)
{
	// Loop through keys
	KeyValues *kv;
	
	for ( kv = inResourceData->GetFirstSubKey(); kv; kv = kv->GetNextKey() )
	{
		char const *varname = kv->GetName();

		PanelAnimationMapEntry *entry = FindPanelAnimationEntry( varname, GetAnimMap() );
		if ( entry )
		{
			// Set value to value from script
			IPanelAnimationPropertyConverter *converter = FindConverter( entry->type() );
			if ( converter )
			{
				converter->SetData( this, inResourceData, entry );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: sets the default values of all CPanelAnimationVars
//-----------------------------------------------------------------------------
void  Panel::InternalInitDefaultValues( PanelAnimationMap *map )
{
	m_bNeedsDefaultSettingsApplied = false;

	// Look through mapping for entry
	int c = map->entries.Count();
	for ( int i = 0; i < c; i++ )
	{
		PanelAnimationMapEntry *e = &map->entries[ i ];
		Assert( e );
		IPanelAnimationPropertyConverter *converter = FindConverter( e->type() );
		if ( !converter )
			continue;

		converter->InitFromDefault( this, e );
	}

	if ( map->baseMap )
	{	
		InternalInitDefaultValues( map->baseMap );
	}
}


//-----------------------------------------------------------------------------
// Purpose: draws a selection box
//-----------------------------------------------------------------------------
void Panel::DrawBox(int x, int y, int wide, int tall, Color color, float normalizedAlpha )
{
	if ( m_nBgTextureId1 == -1 ||
		 m_nBgTextureId2 == -1 ||
		 m_nBgTextureId3 == -1 ||
		 m_nBgTextureId4 == -1 )
	{
		return;
	}

	color[3] *= normalizedAlpha;

	// work out our bounds
	int cornerWide, cornerTall;
	surface()->DrawGetTextureSize(m_nBgTextureId1, cornerWide, cornerTall);

	// draw the background in the areas not occupied by the corners
	// draw it in three horizontal strips
	surface()->DrawSetColor(color);
	surface()->DrawFilledRect(x + cornerWide,	y,						x + wide - cornerWide,	y + cornerTall);
	surface()->DrawFilledRect(x,				y + cornerTall,			x + wide,				y + tall - cornerTall);
	surface()->DrawFilledRect(x + cornerWide,	y + tall - cornerTall,	x + wide - cornerWide,	y + tall);

	// draw the corners
	surface()->DrawSetTexture(m_nBgTextureId1);
	surface()->DrawTexturedRect(x, y, x + cornerWide, y + cornerTall);
	surface()->DrawSetTexture(m_nBgTextureId2);
	surface()->DrawTexturedRect(x + wide - cornerWide, y, x + wide, y + cornerTall);
	surface()->DrawSetTexture(m_nBgTextureId3);
	surface()->DrawTexturedRect(x + wide - cornerWide, y + tall - cornerTall, x + wide, y + tall);
	surface()->DrawSetTexture(m_nBgTextureId4);
	surface()->DrawTexturedRect(x + 0, y + tall - cornerTall, x + cornerWide, y + tall);
}

//-----------------------------------------------------------------------------
// Purpose: draws a selection box
//-----------------------------------------------------------------------------
void Panel::DrawTexturedBox(int x, int y, int wide, int tall, Color color, float normalizedAlpha )
{
	if ( m_nBgTextureId1 == -1 )
		return;

	color[3] *= normalizedAlpha;

	surface()->DrawSetColor( color );
	surface()->DrawSetTexture(m_nBgTextureId1);
	surface()->DrawTexturedRect(x, y, x + wide, y + tall);
}

//-----------------------------------------------------------------------------
// Purpose: Utility class for handling message map allocation
//-----------------------------------------------------------------------------
class CPanelMessageMapDictionary
{
public:
	CPanelMessageMapDictionary() : m_PanelMessageMapPool( sizeof(PanelMessageMap), 32, CMemoryPool::GROW_FAST, "CPanelMessageMapDictionary::m_PanelMessageMapPool" )
	{
		m_MessageMaps.RemoveAll();
	}

	PanelMessageMap	*FindOrAddPanelMessageMap( char const *className );
	PanelMessageMap	*FindPanelMessageMap( char const *className );
private:

	struct PanelMessageMapDictionaryEntry
	{
		PanelMessageMap *map;
	};

	char const *StripNamespace( char const *className );
	
	CUtlDict< PanelMessageMapDictionaryEntry, int > m_MessageMaps;
	CMemoryPool m_PanelMessageMapPool;
};


char const *CPanelMessageMapDictionary::StripNamespace( char const *className )
{
	if ( !strnicmp( className, "vgui::", 6 ) )
	{
		return className + 6;
	}
	return className;
}

//-----------------------------------------------------------------------------
// Purpose: Find but don't add mapping
//-----------------------------------------------------------------------------
PanelMessageMap *CPanelMessageMapDictionary::FindPanelMessageMap( char const *className )
{
	int lookup = m_MessageMaps.Find( StripNamespace( className ) );
	if ( lookup != m_MessageMaps.InvalidIndex() )
	{
		return m_MessageMaps[ lookup ].map;
	}
	return NULL;
}

#include <tier0/memdbgoff.h>
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
PanelMessageMap *CPanelMessageMapDictionary::FindOrAddPanelMessageMap( char const *className )
{
	PanelMessageMap *map = FindPanelMessageMap( className );
	if ( map )
		return map;

	PanelMessageMapDictionaryEntry entry;
	// use the alloc in place method of new
	entry.map = new (m_PanelMessageMapPool.Alloc(sizeof(PanelMessageMap))) PanelMessageMap;
	Construct(entry.map);
	m_MessageMaps.Insert( StripNamespace( className ), entry );
	return entry.map;
}

#include <tier0/memdbgon.h>


CPanelMessageMapDictionary& GetPanelMessageMapDictionary()
{
	static CPanelMessageMapDictionary dictionary;
	return dictionary;
}

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
PanelMessageMap *FindOrAddPanelMessageMap( char const *className )
{
	return GetPanelMessageMapDictionary().FindOrAddPanelMessageMap( className );
}

//-----------------------------------------------------------------------------
// Purpose: Find but don't add mapping
//-----------------------------------------------------------------------------
PanelMessageMap *FindPanelMessageMap( char const *className )
{
	return GetPanelMessageMapDictionary().FindPanelMessageMap( className );
}

}
