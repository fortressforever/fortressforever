//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef TREEVIEW_H
#define TREEVIEW_H

#ifdef _WIN32
#pragma once
#endif

#include <UtlLinkedList.h>
#include <UtlVector.h>
#include <vgui/VGUI.h>
#include <vgui_controls/Panel.h>

class KeyValues;

namespace vgui
{

class ExpandButton;
class TreeNode;
class TreeViewSubPanel;

// sorting function, should return true if node1 should be displayed before node2
typedef bool (*TreeViewSortFunc_t)(KeyValues *node1, KeyValues *node2);

class TreeView : public Panel
{
	DECLARE_CLASS_SIMPLE( TreeView, Panel );

public:
    TreeView(Panel *parent, const char *panelName);
    ~TreeView();

    void SetSortFunc(TreeViewSortFunc_t pSortFunc);

    virtual int AddItem(KeyValues *data, int parentItemIndex);

	virtual int GetRootItemIndex();
	virtual int GetNumChildren( int itemIndex );
	virtual int GetChild( int iParentItemIndex, int iChild ); // between 0 and GetNumChildren( iParentItemIndex ).

    virtual int GetItemCount(void);
    virtual KeyValues *GetItemData(int itemIndex);
    virtual KeyValues *GetSelectedItemData();
    virtual void RemoveItem(int itemIndex, bool bPromoteChildren);
    virtual void RemoveAll();
    virtual bool ModifyItem(int itemIndex, KeyValues *data);
	virtual int GetItemParent(int itemIndex);

    virtual void SetFont(HFont font);

    virtual void SetImageList(ImageList *imageList, bool deleteImageListWhenDone);

    virtual void SetSelectedItem(int itemIndex);

	// set colors for individual elments
	virtual void SetItemFgColor(int itemIndex, Color color);
	virtual void SetItemBgColor(int itemIndex, Color color);

	// returns the id of the currently selected item, -1 if nothing is selected
	virtual int GetSelectedItem();

	// returns true if the itemID is valid for use
	virtual bool IsItemIDValid(int itemIndex);

	// item iterators
	// iterate from [0..GetHighestItemID()], 
	// and check each with IsItemIDValid() before using
	virtual int GetHighestItemID();

    virtual void ExpandItem(int itemIndex, bool bExpand);
	virtual bool IsItemExpanded( int itemIndex );

    virtual void MakeItemVisible(int itemIndex);
	
	// This tells which of the visible items is the top one.
	virtual void GetVBarInfo( int &top, int &nItemsVisible );

	virtual HFont GetFont();

	/* message sent

		"TreeViewItemSelected"
			called when the selected item changes

	*/
    int GetRowHeight();

protected:
	// functions to override
	// called when a node, marked as "Expand", needs to generate it's child nodes when expanded
	virtual void GenerateChildrenOfNode(int itemIndex) {}

	// override to open a custom context menu on a node being selected and right-clicked
	virtual void GenerateContextMenu( int itemIndex, int x, int y ) {}

	// overrides
	virtual void OnMouseWheeled(int delta);
	virtual void OnSizeChanged(int wide, int tall); 
	virtual void PerformLayout();
	virtual void ApplySchemeSettings(IScheme *pScheme);
	MESSAGE_FUNC( OnSliderMoved, "ScrollBarSliderMoved" );
	virtual void SetBgColor( Color color );

//	virtual void OnMousePressed(enum MouseCode code);
//	virtual void OnKeyCodeTyped(enum KeyCode code);

private:
	ScrollBar			*_hbar, *_vbar;
	int                 _rowHeight;

	bool m_bDeleteImageListWhenDone;
	ImageList *m_pImageList;


    friend TreeNode;
    IImage* GetImage(int index);        // to be accessed by TreeNodes

    TreeNode                *m_pRootNode;
    TreeViewSortFunc_t      m_pSortFunc;
    HFont                   m_Font;

    // cross reference - no hierarchy ordering in this list
    CUtlLinkedList<TreeNode *, int>   m_NodeList;
    
    TreeNode                *m_pSelectedItem;
    TreeViewSubPanel        *m_pSubPanel;
};

}

#endif // TREEVIEW_H
