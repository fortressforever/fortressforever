//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef SECTIONEDLISTPANEL_H
#define SECTIONEDLISTPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <UtlVector.h>
#include <UtlLinkedList.h>
#include <vgui/VGUI.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/PHandle.h>

namespace vgui
{

class CSectionHeader;
class CItemButton;

// sorting function, should return true if itemID1 should be displayed before itemID2
typedef bool (*SectionSortFunc_t)(SectionedListPanel *list, int itemID1, int itemID2);

//-----------------------------------------------------------------------------
// Purpose: List panel control that is divided up into discrete sections
//-----------------------------------------------------------------------------
class SectionedListPanel : public Panel
{
	DECLARE_CLASS_SIMPLE( SectionedListPanel, Panel );

public:
	SectionedListPanel(vgui::Panel *parent, const char *name);
	~SectionedListPanel();

	// adds a new section; returns false if section already exists
	virtual void AddSection(int sectionID, const char *name, SectionSortFunc_t sortFunc = NULL);
	virtual void AddSection(int sectionID, const wchar_t *name, SectionSortFunc_t sortFunc = NULL);

	// clears all the sections - leaves the items in place
	virtual void RemoveAllSections();

	// modifies section info
	virtual void SetSectionFgColor(int sectionID, Color color);
	// forces a section to always be visible
	virtual void SetSectionAlwaysVisible(int sectionID, bool visible = true);

	// adds a new column to a section
	enum ColumnFlags_e
	{
		HEADER_IMAGE	= 0x01,		// set if the header for the column is an image instead of text
		COLUMN_IMAGE	= 0x02,		// set if the column contains an image instead of text (images are looked up by index from the ImageList) (see SetImageList below)
		COLUMN_BRIGHT	= 0x04,		// set if the column text should be the bright color
		COLUMN_CENTER	= 0x08,		// set to center the text/image in the column
		COLUMN_RIGHT	= 0x10,		// set to right-align the text in the column
	};
	virtual bool AddColumnToSection(int sectionID, const char *columnName, const char *columnText, int columnFlags, int width);
	virtual bool AddColumnToSection(int sectionID, const char *columnName, const wchar_t *columnText, int columnFlags, int width);
	
	// modifies the text in an existing column
	virtual bool ModifyColumn(int sectionID, const char *columnName, const wchar_t *columnText);

	// adds an item to the list; returns the itemID of the new item
	virtual int AddItem(int sectionID, const KeyValues *data);

	// modifies an existing item; returns false if the item does not exist
	virtual bool ModifyItem(int itemID, int sectionID, const KeyValues *data);

	// removes an item from the list; returns false if the item does not exist or is already removed
	virtual bool RemoveItem(int itemID);

	// clears the list
	virtual void RemoveAll()		{ DeleteAllItems(); }
	// DeleteAllItems() is deprecated, use RemoveAll();
	virtual void DeleteAllItems();

	// set the text color of an item
	virtual void SetItemFgColor(int itemID, Color color);

	/* MESSAGES SENT:
		"RowSelected"
			"itemID" - the selected item id, -1 if nothing selected

		// when an item has been clicked on
		"RowContextMenu"		"itemID"
		"RowLeftClick"			"itemID"
		"RowDoubleLeftClick"	"itemID"
	*/
	
	// returns the number of columns in a section
	virtual int GetColumnCountBySection(int sectionID);

	// returns the name of a column by section and column index; returns NULL if there are no more columns
	// valid range of columnIndex is [0, GetColumnCountBySection)
	virtual const char *GetColumnNameBySection(int sectionID, int columnIndex);
	virtual const wchar_t *GetColumnTextBySection(int sectionID, int columnIndex);
	virtual int GetColumnFlagsBySection(int sectionID, int columnIndex);
	virtual int GetColumnWidthBySection(int sectionID, int columnIndex);

	// returns the id of the currently selected item, -1 if nothing is selected
	virtual int GetSelectedItem();

	// sets which item is currently selected
	virtual void SetSelectedItem(int itemID);

	// returns the data of a selected item
	// InvalidateItem(itemID) needs to be called if the KeyValues are modified
	virtual KeyValues *GetItemData(int itemID);

	// returns what section an item is in
	virtual int GetItemSection(int itemID);

	// forces an item to redraw (use when keyvalues have been modified)
	virtual void InvalidateItem(int itemID);

	// returns true if the itemID is valid for use
	virtual bool IsItemIDValid(int itemID);
	virtual int GetHighestItemID();

	// returns the number of items (ignoring section dividers)
	virtual int GetItemCount();

	// returns the item ID from the row, again ignoring section dividers - valid from [0, GetItemCount )
	virtual int GetItemIDFromRow(int row);		

	// gets the local coordinates of a cell
	virtual bool GetCellBounds(int itemID, int column, int &x, int &y, int &wide, int &tall);

	// set up a field for editing
	virtual void EnterEditMode(int itemID, int column, vgui::Panel *editPanel);

	// leaves editing mode
	virtual void LeaveEditMode();

	// returns true if we are currently in inline editing mode
	virtual bool IsInEditMode();

	// sets whether or not the vertical scrollbar should ever be displayed
	virtual void SetVerticalScrollbar(bool state);

	// returns the size required to fully draw the contents of the panel
	virtual void GetContentSize(int &wide, int &tall);

	// image handling
	virtual void SetImageList(ImageList *imageList, bool deleteImageListWhenDone);

    virtual void ScrollToItem(int iItem);

	virtual void SetProportional(bool state);

protected:
	virtual void PerformLayout();
	virtual void ApplySchemeSettings(IScheme *pScheme);
	virtual void ApplySettings(KeyValues *inResourceData);
	virtual void OnSizeChanged(int wide, int tall);
	virtual void OnMouseWheeled(int delta);
	virtual void OnMousePressed(enum MouseCode code);
	virtual void OnKeyCodeTyped(enum KeyCode code);
	virtual void OnSetFocus();						// called after the panel receives the keyboard focus

private:
	MESSAGE_FUNC( OnSliderMoved, "ScrollBarSliderMoved" );

	void AddSectionHelper(int sectionID, CSectionHeader *header, SectionSortFunc_t sortFunc);
	int GetSectionTall();
	void LayoutPanels(int &contentTall);

	// Returns the index of a new item button, reusing an existing item button if possible
	int GetNewItemButton();

	friend CItemButton;
	void SetSelectedItem(CItemButton *item);
	DHANDLE<CItemButton> m_hSelectedItem;

	struct column_t
	{
		char m_szColumnName[32];
		wchar_t m_szColumnText[64];
		int m_iColumnFlags;
		int m_iWidth;
	};
	struct section_t
	{
		int m_iID;
		bool m_bAlwaysVisible;
		CSectionHeader *m_pHeader;
		CUtlVector<column_t> m_Columns;
		SectionSortFunc_t m_pSortFunc;
	};

	CUtlVector<section_t> 				m_Sections;
	CUtlLinkedList<CItemButton *, int> 	m_Items;
	CUtlLinkedList<CItemButton *, int> 	m_FreeItems;
    CUtlVector<CItemButton *> 			m_SortedItems;

	PHandle m_hEditModePanel;
	int m_iEditModeItemID;
	int m_iEditModeColumn;
	int m_iContentHeight;
	int m_iLineSpacing;

	int FindSectionIndexByID(int sectionID);
    void ReSortList();

	ScrollBar *m_pScrollBar;
	ImageList *m_pImageList;
	bool m_bDeleteImageListWhenDone;
	bool m_bSortNeeded;
	bool m_bVerticalScrollbarEnabled;
};

} // namespace vgui

#endif // SECTIONEDLISTPANEL_H
