//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef COMBOBOX_H
#define COMBOBOX_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Menu.h>

namespace vgui
{

class ComboBoxButton;

//-----------------------------------------------------------------------------
// Purpose: Text entry with drop down options list
//-----------------------------------------------------------------------------
class ComboBox : public TextEntry
{
	DECLARE_CLASS_SIMPLE( ComboBox, TextEntry );

public:
	ComboBox(Panel *parent, const char *panelName, int numLines, bool allowEdit);
	~ComboBox();

	// functions designed to be overriden
	virtual void OnShowMenu(Menu *menu) {}
	virtual void OnHideMenu(Menu *menu) {}

	// Set the number of items in the drop down menu.
	virtual void SetNumberOfEditLines( int numLines );

	//  Add an item to the drop down
	virtual int AddItem(const char *itemText, const KeyValues *userData);
	virtual int AddItem(const wchar_t *itemText, const KeyValues *userData);

	virtual int GetItemCount();

	// update the item
	virtual bool UpdateItem(int itemID, const char *itemText,const  KeyValues *userData);
	virtual bool UpdateItem(int itemID, const wchar_t *itemText, const KeyValues *userData);
	virtual bool IsItemIDValid(int itemID);

	// set the enabled state of an item
	virtual void SetItemEnabled(const char *itemText, bool state);
	virtual void SetItemEnabled(int itemID, bool state);
	
	// Removes a single item
	void DeleteItem( int itemID );

	// Remove all items from the drop down menu
	void RemoveAll();
	// deprecated, use above
	void DeleteAllItems()	{ RemoveAll(); }

	// Sorts the items in the list - FIXME does nothing
	virtual void SortItems();

	// Set the visiblity of the drop down menu button.
	virtual void SetDropdownButtonVisible(bool state);

	// Return true if the combobox current has the dropdown menu open
	virtual bool IsDropdownVisible();

	// Activate the item in the menu list,as if that 
	// menu item had been selected by the user
	MESSAGE_FUNC_INT( ActivateItem, "ActivateItem", itemID );
	void ActivateItemByRow(int row);

	int GetActiveItem();
	KeyValues *GetActiveItemUserData();
	KeyValues *GetItemUserData(int itemID);
	void GetItemText( int itemID, wchar_t *text, int bufLenInBytes );
	void GetItemText( int itemID, char *text, int bufLenInBytes );

	// sets a custom menu to use for the dropdown
	virtual void SetMenu( Menu *menu );
	virtual Menu *GetMenu() { return m_pDropDown; }

	// Layout the format of the combo box for drawing on screen
	virtual void PerformLayout();

	/* action signals
		"TextChanged" - signals that the text has changed in the combo box

	*/

	virtual void ShowMenu();
	virtual void HideMenu();
	virtual void OnKillFocus();
	MESSAGE_FUNC( OnMenuClose, "MenuClose" );
	virtual void DoClick();
	virtual void OnSizeChanged(int wide, int tall);

	virtual void SetOpenDirection(Menu::MenuDirection_e direction);

	virtual void SetFont( HFont font );

protected:
	// overrides
	virtual void OnMousePressed(MouseCode code);
	virtual void OnMouseDoublePressed(MouseCode code);
	MESSAGE_FUNC( OnMenuItemSelected, "MenuItemSelected" );
	virtual void OnCommand( const char *command );
	virtual void ApplySchemeSettings(IScheme *pScheme);
	virtual void OnCursorEntered();
	virtual void OnCursorExited();

	// custom message handlers
	MESSAGE_FUNC_WCHARPTR( OnSetText, "SetText", text );
	virtual void OnSetFocus();						// called after the panel receives the keyboard focus
    virtual void OnKeyCodeTyped(KeyCode code);
	virtual void OnKeyTyped(wchar_t unichar);

    void MoveAlongMenuItemList(int direction);


private:
	void DoMenuLayout();

	Menu 				*m_pDropDown;
	ComboBoxButton 		*m_pButton;

	bool 				m_bAllowEdit;
	bool 				m_bHighlight;
	Menu::MenuDirection_e 	m_iDirection;
	int 				m_iOpenOffsetY;
};

} // namespace vgui

#endif // COMBOBOX_H
