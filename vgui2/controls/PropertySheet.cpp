//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <vgui/IBorder.h>
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>
#include <vgui/MouseCode.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/PropertyPage.h>
#include "vgui_controls/AnimationController.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: A single tab
//-----------------------------------------------------------------------------
class PageTab : public Button
{
private:
	bool _active;
	Color _textColor;
	Color _dimTextColor;
	int m_bMaxTabWidth;
	IBorder *m_pActiveBorder;
	IBorder *m_pNormalBorder;

public:
	PageTab(Panel *parent, const char *panelName, const char *text, int maxTabWidth) : Button(parent, panelName, text) 
	{
		SetCommand(new KeyValues("TabPressed"));
		_active = false;
		m_bMaxTabWidth = maxTabWidth;
	}

	~PageTab() {}

	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		// set up the scheme settings
		Button::ApplySchemeSettings(pScheme);

		_textColor = GetSchemeColor("PropertySheet.SelectedTextColor", GetFgColor(), pScheme);
		_dimTextColor = GetSchemeColor("PropertySheet.TextColor", GetFgColor(), pScheme);
		m_pActiveBorder = pScheme->GetBorder("TabActiveBorder");
		m_pNormalBorder = pScheme->GetBorder("TabBorder");

		int wide, tall;
		int contentWide, contentTall;
		GetSize(wide, tall);
		GetContentSize(contentWide, contentTall);

		wide = max(m_bMaxTabWidth, contentWide + 10);  // 10 = 5 pixels margin on each side
		SetSize(wide, tall);
	}

	IBorder *GetBorder(bool depressed, bool armed, bool selected, bool keyfocus)
	{
		if (_active)
		{
			return m_pActiveBorder;
		}
		return m_pNormalBorder;
	}

	virtual Color GetButtonFgColor()
	{
		if (_active)
		{
			return _textColor;
		}
		else
		{
			return _dimTextColor;
		}
	}

	virtual void SetActive(bool state)
	{
		_active = state;
		InvalidateLayout();
		Repaint();
	}

    virtual bool CanBeDefaultButton(void)
    {
        return false;
    }

	//Fire action signal when mouse is pressed down instead  of on release.
	virtual void OnMousePressed(MouseCode code) 
	{
        
		// check for context menu open
		if (code == MOUSE_RIGHT)
		{
			PostActionSignal(new KeyValues("OpenContextMenu", "itemID", -1));
		}
		else
		{
			if (!IsEnabled())
				return;
			
			if (!IsMouseClickEnabled(code))
				return;
			
			if (IsUseCaptureMouseEnabled())
			{
				{
					RequestFocus();
					FireActionSignal();
					SetSelected(true);
					Repaint();
				}
				
				// lock mouse input to going to this button
				input()->SetMouseCapture(GetVPanel());
			}
		}
	}

	virtual void OnMouseReleased(MouseCode code)
	{
		// ensure mouse capture gets released
		if (IsUseCaptureMouseEnabled())
		{
			input()->SetMouseCapture(NULL);
		}

		// make sure the button gets unselected
		SetSelected(false);
		Repaint();
	}
};

}; // namespace vgui



//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
PropertySheet::PropertySheet(Panel *parent, const char *panelName) : Panel(parent, panelName)
{
	_activePage = NULL;
	_activeTab = NULL;
	_tabWidth = 64;
	_activeTabIndex = 0;
	_showTabs = true;
	_combo = NULL;
    _tabFocus = false;
	m_flPageTransitionEffectTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor, associates pages with a combo box
//-----------------------------------------------------------------------------
PropertySheet::PropertySheet(Panel *parent, const char *panelName, ComboBox *combo) : Panel(parent, panelName)
{
	_activePage = NULL;
	_activeTab = NULL;
	_tabWidth = 64;
	_activeTabIndex = 0;
	_combo=combo;
	_combo->AddActionSignalTarget(this);
	_showTabs = false;
    _tabFocus = false;
	m_flPageTransitionEffectTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
PropertySheet::~PropertySheet()
{
}

//-----------------------------------------------------------------------------
// Purpose: adds a page to the sheet
//-----------------------------------------------------------------------------
void PropertySheet::AddPage(Panel *page, const char *title)
{
	if (!page)
		return;

	// don't add the page if we already have it
	if (m_Pages.HasElement(page))
		return;

	PageTab *tab = new PageTab(this, "tab", title, _tabWidth);
	if(_showTabs)
	{
		tab->AddActionSignalTarget(this);
	}
	else if (_combo)
	{
		_combo->AddItem(title, NULL);
	}
	m_PageTabs.AddToTail(tab);

	m_Pages.AddToTail(page);

	page->SetParent(this);
	page->AddActionSignalTarget(this);
	PostMessage(page, new KeyValues("ResetData"));

	page->SetVisible(false);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void PropertySheet::SetActivePage(Panel *page)
{
	// walk the list looking for this page
	int index = m_Pages.Find(page);
	if (!m_Pages.IsValidIndex(index))
		return;

	ChangeActiveTab(index);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void PropertySheet::SetTabWidth(int pixels)
{
	_tabWidth = pixels;
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: reloads the data in all the property page
//-----------------------------------------------------------------------------
void PropertySheet::ResetAllData()
{
	// iterate all the dialogs resetting them
	for (int i = 0; i < m_Pages.Count(); i++)
	{
		ipanel()->SendMessage(m_Pages[i]->GetVPanel(), new KeyValues("ResetData"), GetVPanel());
	}
}

//-----------------------------------------------------------------------------
// Purpose: Applies any changes made by the dialog
//-----------------------------------------------------------------------------
void PropertySheet::ApplyChanges()
{
	// iterate all the dialogs resetting them
	for (int i = 0; i < m_Pages.Count(); i++)
	{
		ipanel()->SendMessage(m_Pages[i]->GetVPanel(), new KeyValues("ApplyChanges"), GetVPanel());
	}
}

//-----------------------------------------------------------------------------
// Purpose: gets a pointer to the currently active page
//-----------------------------------------------------------------------------
Panel *PropertySheet::GetActivePage()
{
	return _activePage;
}

//-----------------------------------------------------------------------------
// Purpose: gets a pointer to the currently active tab
//-----------------------------------------------------------------------------
Panel *PropertySheet::GetActiveTab()
{
	return _activeTab;
}

//-----------------------------------------------------------------------------
// Purpose: returns the number of panels in the sheet
//-----------------------------------------------------------------------------
int PropertySheet::GetNumPages()
{
	return m_Pages.Count();
}

//-----------------------------------------------------------------------------
// Purpose: returns the name contained in the active tab
// Input  : a text buffer to contain the output 
//-----------------------------------------------------------------------------
void PropertySheet::GetActiveTabTitle(char *textOut, int bufferLen)
{
	if(_activeTab) _activeTab->GetText(textOut, bufferLen);
}

//-----------------------------------------------------------------------------
// Purpose: returns the name contained in the active tab
// Input  : a text buffer to contain the output 
//-----------------------------------------------------------------------------
bool PropertySheet::GetTabTitle(int i, char *textOut, int bufferLen)
{
	if (i < 0 && i > m_PageTabs.Count()) 
	{
		return false;
	}

	m_PageTabs[i]->GetText(textOut, bufferLen);
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the index of the currently active page
//-----------------------------------------------------------------------------
int PropertySheet::GetActivePageNum()
{
	for (int i = 0; i < m_Pages.Count(); i++)
	{
		if (m_Pages[i] == _activePage) 
		{
			return i;
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: Forwards focus requests to current active page
//-----------------------------------------------------------------------------
void PropertySheet::RequestFocus(int direction)
{
    if (direction == -1 || direction == 0)
    {
    	if (_activePage)
    	{
    		_activePage->RequestFocus(direction);
            _tabFocus = false;
    	}
    }
    else 
    {
        if (_showTabs && _activeTab)
        {
            _activeTab->RequestFocus(direction);
            _tabFocus = true;
        }
		else if (_activePage)
    	{
    		_activePage->RequestFocus(direction);
            _tabFocus = false;
    	}
    }
}

//-----------------------------------------------------------------------------
// Purpose: moves focus back
//-----------------------------------------------------------------------------
bool PropertySheet::RequestFocusPrev(VPANEL panel)
{
    if (_tabFocus || !_showTabs || !_activeTab)
    {
        _tabFocus = false;
        return BaseClass::RequestFocusPrev(panel);
    }
    else
    {
        if (GetVParent())
        {
            PostMessage(GetVParent(), new KeyValues("FindDefaultButton"));
        }
        _activeTab->RequestFocus(-1);
        _tabFocus = true;
        return true;
    }
}

//-----------------------------------------------------------------------------
// Purpose: moves focus forward
//-----------------------------------------------------------------------------
bool PropertySheet::RequestFocusNext(VPANEL panel)
{
    if (!_tabFocus || !_activePage)
    {
        return BaseClass::RequestFocusNext(panel);
    }
    else
    {
        if (!_activeTab)
        {
            return BaseClass::RequestFocusNext(panel);
        }
        else
        {
            _activePage->RequestFocus(1);
            _tabFocus = false;
            return true;
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Gets scheme settings
//-----------------------------------------------------------------------------
void PropertySheet::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	// a little backwards-compatibility with old scheme files
	IBorder *pBorder = pScheme->GetBorder("PropertySheetBorder");
	if (pBorder == pScheme->GetBorder("Default"))
	{
		// get the old name
		pBorder = pScheme->GetBorder("RaisedBorder");
	}

	SetBorder(pBorder);
	m_flPageTransitionEffectTime = atof(pScheme->GetResourceString("PropertySheet.TransitionEffectTime"));
}

//-----------------------------------------------------------------------------
// Purpose: Paint our border specially, with the tabs in mind
//-----------------------------------------------------------------------------
void PropertySheet::PaintBorder()
{
	IBorder *border = GetBorder();
	if (!border)
		return;

	// draw the border, but with a break at the active tab
	int px = 0, py = 0, pwide = 0, ptall = 0;
	if (_activeTab)
	{
		_activeTab->GetBounds(px, py, pwide, ptall);
		ptall -= 1;
	}

	// draw the border underneath the buttons, with a break
	int wide, tall;
	GetSize(wide, tall);
	border->Paint(0, py + ptall, wide, tall, IBorder::SIDE_TOP, px + 1, px + pwide - 1);
}

//-----------------------------------------------------------------------------
// Purpose: Lays out the dialog
//-----------------------------------------------------------------------------
void PropertySheet::PerformLayout()
{
	BaseClass::PerformLayout();

	if (!_activePage)
	{
		// first page becomes the active page
		ChangeActiveTab(0);
		if (_activePage)
			_activePage->RequestFocus(0);
	}

	int x, y, wide, tall;
	GetBounds(x, y, wide, tall);
	if (_activePage)
	{
		if(_showTabs)
		{
			_activePage->SetBounds(0, 28, wide, tall - 28);
		}
		else
		{
			_activePage->SetBounds(0, 0, wide, tall );
		}
		_activePage->InvalidateLayout();
	}

	
	int xtab;
	int limit = m_PageTabs.Count();

	xtab = 0;

	// draw the visible tabs
	if (_showTabs)
	{
		for (int i = 0; i < limit; i++)
		{
            int width, tall;
            m_PageTabs[i]->GetSize(width, tall);
			if (m_PageTabs[i] == _activeTab)
			{
				// active tab is taller
				_activeTab->SetBounds(xtab, 2, width, 27);
			}
			else
			{
				m_PageTabs[i]->SetBounds(xtab, 4, width, 25);
			}
			m_PageTabs[i]->SetVisible(true);
			xtab += (width + 1);
		}
	}
	else
	{
		for (int i = 0; i < limit; i++)
		{
			m_PageTabs[i]->SetVisible(false);
		}
	}

	// ensure draw order (page drawing over all the tabs except one)
	if (_activePage)
	{
		_activePage->MoveToFront();
		_activePage->Repaint();
	}
	if (_activeTab)
	{
		_activeTab->MoveToFront();
		_activeTab->Repaint();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Switches the active panel
//-----------------------------------------------------------------------------
void PropertySheet::OnTabPressed(Panel *panel)
{
	// look for the tab in the list
	for (int i = 0; i < m_PageTabs.Count(); i++)
	{
		if (m_PageTabs[i] == panel)
		{
			// flip to the new tab
			ChangeActiveTab(i);
			return;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: returns the panel associated with index i
// Input  : the index of the panel to return 
//-----------------------------------------------------------------------------
Panel *PropertySheet::GetPage(int i) 
{
	if(i<0 && i>m_Pages.Count()) 
	{
		return NULL;
	}

	return m_Pages[i];
}


//-----------------------------------------------------------------------------
// Purpose: disables page by name
//-----------------------------------------------------------------------------
void PropertySheet::DisablePage(const char *title) 
{
	SetPageEnabled(title, false);
}

//-----------------------------------------------------------------------------
// Purpose: enables page by name
//-----------------------------------------------------------------------------
void PropertySheet::EnablePage(const char *title) 
{
	SetPageEnabled(title, true);
}

//-----------------------------------------------------------------------------
// Purpose: enabled or disables page by name
//-----------------------------------------------------------------------------
void PropertySheet::SetPageEnabled(const char *title, bool state) 
{
	for (int i = 0; i < m_PageTabs.Count(); i++)
	{
		if (_showTabs)
		{
			char tmp[50];
			m_PageTabs[i]->GetText(tmp,50);
			if (!strnicmp(title,tmp,strlen(tmp)))
			{	
				m_PageTabs[i]->SetEnabled(state);
			}
		}
		else
		{
			_combo->SetItemEnabled(title,state);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: deletes the page associated with panel
// Input  : *panel - the panel of the page to remove
//-----------------------------------------------------------------------------
void PropertySheet::DeletePage(Panel *panel) 
{
	if (! m_Pages.HasElement(panel))
		return;

	int location = m_Pages.Find(panel);
	if (_activePage == panel) 
	{
		// if this page is currently active, backup to the page before this.
		ChangeActiveTab( location - 1 ); 
	}
	//	changeActiveTab( 0 ); 

	// ASSUMPTION = that the number of pages equals number of tabs
	if(_showTabs)
	{
		m_PageTabs[location]->RemoveActionSignalTarget(this);
	}
	// now remove the panels
	PageTab *tab  = m_PageTabs[location];
	m_PageTabs.Remove(location);
	tab->MarkForDeletion();
	
	m_Pages.FindAndRemove(panel);
	panel->MarkForDeletion();

	PerformLayout();
}

//-----------------------------------------------------------------------------
// Purpose: flips to the new tab, sending out all the right notifications
//			flipping to a tab activates the tab.
//-----------------------------------------------------------------------------
void PropertySheet::ChangeActiveTab(int index)
{
	if (!m_Pages.IsValidIndex(index))
		return;

	if (m_Pages[index] == _activePage)
	{
		_activeTab->RequestFocus();
		_tabFocus = true;
		return;
	}

	// make sure any distant tab is completely hidden (since the prior page could still be fading out)
	if (m_hPreviouslyActivePage.Get())
	{
		m_hPreviouslyActivePage->SetVisible(false);
	}

	m_hPreviouslyActivePage = _activePage;
	// notify old page
	if (_activePage)
	{
		ivgui()->PostMessage(_activePage->GetVPanel(), new KeyValues("PageHide"), GetVPanel());
		KeyValues *msg = new KeyValues("PageTabActivated");
		msg->SetPtr("panel", (Panel *)NULL);
		ivgui()->PostMessage(_activePage->GetVPanel(), msg, GetVPanel());
	}
	if (_activeTab)
	{
		//_activeTabIndex=index;
		_activeTab->SetActive(false);

		// does the old tab have the focus?
		_tabFocus = _activeTab->HasFocus();
	}
	else
	{
		_tabFocus = false;
	}

	// flip page
	_activePage = m_Pages[index];
	_activeTab = m_PageTabs[index];
	_activeTabIndex = index;

	_activePage->SetVisible(true);
	_activePage->MoveToFront();
	
	_activeTab->SetVisible(true);
	_activeTab->MoveToFront();
	_activeTab->SetActive(true);

	if (_tabFocus)
	{
		// if a tab already has focused,give the new tab the focus
		_activeTab->RequestFocus();
	}
	else
	{
		// otherwise, give the focus to the page
		_activePage->RequestFocus();
	}

	if (!_showTabs)
	{
		_combo->ActivateItemByRow(index);
	}


	// transition effect
	if (m_flPageTransitionEffectTime)
	{
		if (m_hPreviouslyActivePage.Get())
		{
			// fade out the previous page
			GetAnimationController()->RunAnimationCommand(m_hPreviouslyActivePage, "Alpha", 0.0f, 0.0f, m_flPageTransitionEffectTime / 2, AnimationController::INTERPOLATOR_LINEAR);
		}

		// fade in the new page
		_activePage->MakeReadyForUse();
		_activePage->SetAlpha(0);
		GetAnimationController()->RunAnimationCommand(_activePage, "Alpha", 255.0f, m_flPageTransitionEffectTime / 2, m_flPageTransitionEffectTime / 2, AnimationController::INTERPOLATOR_LINEAR);
	}
	else
	{
		if (m_hPreviouslyActivePage.Get())
		{
			// no transition, just hide the previous page
			m_hPreviouslyActivePage->SetVisible(false);
		}
	}

	// notify
	ivgui()->PostMessage(_activePage->GetVPanel(), new KeyValues("PageShow"), GetVPanel());

	KeyValues *msg = new KeyValues("PageTabActivated");
	msg->SetPtr("panel", (Panel *)_activeTab);
	ivgui()->PostMessage(_activePage->GetVPanel(), msg, GetVPanel());

	// tell parent
	PostActionSignal(new KeyValues("PageChanged"));

	// Repaint
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Gets the panel with the specified hotkey, from the current page
//-----------------------------------------------------------------------------
Panel *PropertySheet::HasHotkey(wchar_t key)
{
	if (!_activePage)
		return NULL;

	for (int i = 0; i < _activePage->GetChildCount(); i++)
	{
		Panel *hot = _activePage->GetChild(i)->HasHotkey(key);
		if (hot)
		{
			return hot;
		}
	}
	
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: catches the opencontextmenu event
//-----------------------------------------------------------------------------
void PropertySheet::OnOpenContextMenu()
{
	// tell parent
	PostActionSignal(new KeyValues("OpenContextMenu"));
}

//-----------------------------------------------------------------------------
// Purpose: Handle key presses, through tabs.
//-----------------------------------------------------------------------------
void PropertySheet::OnKeyCodeTyped(KeyCode code)
{
	switch (code)
	{
		// for now left and right arrows just open or close submenus if they are there.
	case KEY_RIGHT:
		{
			ChangeActiveTab(_activeTabIndex+1);
			break;
		}
	case KEY_LEFT:
		{
			ChangeActiveTab(_activeTabIndex-1);
			break;
		}
	default:
		BaseClass::OnKeyCodeTyped(code);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called by the associated combo box (if in that mode), changes the current panel
//-----------------------------------------------------------------------------
void PropertySheet::OnTextChanged(Panel *panel,const wchar_t *wszText)
{
	if ( panel == _combo )
	{
		wchar_t tabText[30];
		for(int i = 0 ; i < m_PageTabs.Count() ; i++ )
		{
			tabText[0] = 0;
			m_PageTabs[i]->GetText(tabText,30);
			if ( !wcsicmp(wszText,tabText) )
			{
				ChangeActiveTab(i);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void PropertySheet::OnCommand(const char *command)
{
    // propogate the close command to our parent
	if (!stricmp(command, "Close") && GetVParent())
    {
		CallParentFunction(new KeyValues("Command", "command", command));
    }
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void PropertySheet::OnApplyButtonEnable()
{
	// tell parent
	PostActionSignal(new KeyValues("ApplyButtonEnable"));	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void PropertySheet::OnCurrentDefaultButtonSet(Panel *defaultButton)
{
	// forward the message up
	if (GetVParent())
	{
		KeyValues *msg = new KeyValues("CurrentDefaultButtonSet");
		msg->SetPtr("button", defaultButton);
		PostMessage(GetVParent(), msg);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void PropertySheet::OnDefaultButtonSet(Panel *defaultButton)
{
	// forward the message up
	if (GetVParent())
	{
		KeyValues *msg = new KeyValues("DefaultButtonSet");
		msg->SetPtr("button", defaultButton);
		PostMessage(GetVParent(), msg);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void PropertySheet::OnFindDefaultButton()
{
    if (GetVParent())
    {
        PostMessage(GetVParent(), new KeyValues("FindDefaultButton"));
    }
}
