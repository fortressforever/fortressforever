//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <assert.h>

#define PROTECTED_THINGS_DISABLE

#include <vgui/Cursor.h>
#include <vgui/IScheme.h>
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/ISurface.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>
#include <vgui/MouseCode.h>

#include <vgui_controls/TreeView.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/ImagePanel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

using namespace vgui;
enum 
{
	WINDOW_BORDER_WIDTH=2 // the width of the window's border
};

#define TREE_INDENT_AMOUNT 20

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: Displays an editable text field for the text control
//-----------------------------------------------------------------------------
class TreeNodeText : public TextEntry
{
public:
    TreeNodeText(Panel *parent, const char *panelName) : TextEntry(parent, panelName)
    {
    }

	virtual void ApplySchemeSettings(IScheme *pScheme)
    {
        TextEntry::ApplySchemeSettings(pScheme);
        SetBorder(NULL);
        SetCursor(dc_arrow);
    }

    virtual void OnKeyCodeTyped(KeyCode code)
    {
        // let parent deal with it (don't chain back to TextEntry)
		CallParentFunction(new KeyValues("KeyCodeTyped", "code", code));
    }

	virtual void OnMousePressed(MouseCode code)
    {
        // let parent deal with it
		CallParentFunction(new KeyValues("MousePressed", "code", code));
	}

    virtual void OnMouseDoublePressed(MouseCode code)
    {
 		CallParentFunction(new KeyValues("MouseDoublePressed", "code", code));
    }

	virtual void OnMouseWheeled(int delta)
    {
		CallParentFunction(new KeyValues("MouseWheeled", "delta", delta));
    }
    // editable - cursor normal, and ability to edit text
};

//-----------------------------------------------------------------------------
// Purpose: icon for the tree node (folder icon, file icon, etc.)
//-----------------------------------------------------------------------------
class TreeNodeImage : public ImagePanel
{
public:
    TreeNodeImage(Panel *parent, const char *name) : ImagePanel(parent, name) {}

 	//!! this could possibly be changed to just disallow mouse input on the image panel
    virtual void OnMousePressed(MouseCode code)
    {
        // let parent deal with it
		CallParentFunction(new KeyValues("MousePressed", "code", code));
    }
	
    virtual void OnMouseDoublePressed(MouseCode code)
    {
        // let parent deal with it
		CallParentFunction(new KeyValues("MouseDoublePressed", "code", code));
    }

    virtual void OnMouseWheeled(int delta)
    {
        // let parent deal with it
		CallParentFunction(new KeyValues("MouseWheeled", "delta", delta));
    }
};

//-----------------------------------------------------------------------------
// Purpose: Scrollable area of the tree control, holds the tree itself only
//-----------------------------------------------------------------------------
class TreeViewSubPanel : public Panel
{
public:
    TreeViewSubPanel(Panel *parent) : Panel(parent) {}

    virtual void ApplySchemeSettings(IScheme *pScheme)
    {
    	Panel::ApplySchemeSettings(pScheme);
    
    	SetBorder(NULL);
   }

	virtual void OnMouseWheeled(int delta)
    {
        // let parent deal with it
		CallParentFunction(new KeyValues("MouseWheeled", "delta", delta));
    }
    virtual void OnMouseDoublePressed(MouseCode code)
    {
        // let parent deal with it
		CallParentFunction(new KeyValues("MouseDoublePressed", "code", code));
    }
};

//-----------------------------------------------------------------------------
// Purpose: A single entry in the tree
//-----------------------------------------------------------------------------
class TreeNode : public Panel
{
	DECLARE_CLASS_SIMPLE( TreeNode, Panel );
public:
    KeyValues           *m_pData;
    int                 m_ItemIndex;
	int					m_ParentIndex;
	int					m_iNodeWidth;
	int					m_iMaxVisibleWidth;

    TreeNodeText        *m_pText;
    TextImage           *m_pExpandImage;
    TreeNodeImage       *m_pImagePanel;

    bool                m_bExpand;
	bool				m_bExpandableWithoutChildren;
	Color				m_SelectedBgColor;
	Color				m_OutOfFocusSelectedBgColor;

    CUtlVector<TreeNode *> m_Children;

    TreeView            *m_pTreeView;

    TreeNode(Panel *parent, TreeView *pTreeView) : Panel(parent)
    {
        m_pData = NULL;
        m_pTreeView = pTreeView;
        m_ItemIndex = -1;
		m_iNodeWidth = 0; 
		m_iMaxVisibleWidth = 0;

        m_pExpandImage = new TextImage("+");
        m_pExpandImage->SetPos(3, 3);

        m_pImagePanel = new TreeNodeImage(this, "TreeImage");
        m_pImagePanel->SetPos(TREE_INDENT_AMOUNT, 3);

        m_pText = new TreeNodeText(this, "TreeNode");
        m_pText->SetMultiline(false);
        m_pText->SetEditable(false);
        m_pText->SetPos(TREE_INDENT_AMOUNT*2, 0);

        m_bExpand = false;
		m_bExpandableWithoutChildren = false;
    }

    void SetText(const char *pszText)
    {
        m_pText->SetText(pszText);
		InvalidateLayout();
    }

    void SetFont(HFont font)
    {
        Assert( font );
        if ( !font )
            return;

        m_pText->SetFont(font);
		m_pExpandImage->SetFont(font);
        int i;
        for (i=0;i<GetChildrenCount();i++)
        {
            m_Children[i]->SetFont(font);
        }
    }

    void SetKeyValues(KeyValues *data)
    {
        if (m_pData)
        {
            m_pData->deleteThis();
        }

        m_pData = data->MakeCopy();

        // set text
        m_pText->SetText(data->GetString("Text", ""));
 		m_bExpandableWithoutChildren = data->GetInt("Expand");
        InvalidateLayout();
    }

    bool IsSelected()
    {
		return (m_pTreeView->GetSelectedItem() == m_ItemIndex);
    }

	// currently unused, could be re-used if necessary
	/*
	bool IsInFocus()
	{
        // check if our parent or one of it's children has focus
        VPANEL focus = input()->GetFocus();
        return (HasFocus() || (focus && ipanel()->HasParent(focus, GetVParent())));
	}
	*/

	virtual void PaintBackground()
	{
		// setup panel drawing
		if (IsSelected())
		{
			m_pText->SelectAllText(false);
		}
		else
		{
			m_pText->SelectNoText();
		}

		BaseClass::PaintBackground();
	}

    virtual void PerformLayout()
    {
        BaseClass::PerformLayout();

        int width = 0;
		if (m_pData->GetInt("SelectedImage", 0) == 0 &&
			m_pData->GetInt("Image", 0) == 0)
		{
			width = TREE_INDENT_AMOUNT;
		}
		else
		{
			width = TREE_INDENT_AMOUNT * 2;
		}

		m_pText->SetPos(width, 0);

        int contentWide, contentTall;
		m_pText->SetToFullWidth();
        m_pText->GetSize(contentWide, contentTall);
        width += contentWide;
        SetSize(width, m_pTreeView->GetRowHeight());

		m_iNodeWidth = width;
		CalculateVisibleMaxWidth();
    }

	TreeNode *GetParentNode()
	{
		if (m_pTreeView->m_NodeList.IsValidIndex(m_ParentIndex))
		{
			return m_pTreeView->m_NodeList[m_ParentIndex];
		}
		return NULL;
	}

    int GetChildrenCount()
    {
        return m_Children.Count();
    }

	int ComputeInsertionPosition( TreeNode *pChild )
	{
		if ( !m_pTreeView->m_pSortFunc )
		{
			return GetChildrenCount() - 1;
		}

		int start = 0, end = GetChildrenCount() - 1;
		while (start <= end)
		{
			int mid = (start + end) >> 1;
			if ( m_pTreeView->m_pSortFunc( m_Children[mid]->m_pData, pChild->m_pData ) )
			{
				start = mid + 1;
			}
			else if ( m_pTreeView->m_pSortFunc( pChild->m_pData, m_Children[mid]->m_pData ) )
			{
				end = mid - 1;
			}
			else
			{
				return mid;
			}
		}
		return end;
	}

	int FindChild( TreeNode *pChild )
	{
		if ( !m_pTreeView->m_pSortFunc )
		{
			for ( int i = 0; i < GetChildrenCount(); --i )
			{
				if ( m_Children[i] == pChild )
					return i;
			}
			return -1;
		}

		// Find the first entry <= to the child
		int start = 0, end = GetChildrenCount() - 1;
		while (start <= end)
		{
			int mid = (start + end) >> 1;

			if ( m_Children[mid] == pChild )
				return mid;

			if ( m_pTreeView->m_pSortFunc( m_Children[mid]->m_pData, pChild->m_pData ) )
			{
				start = mid + 1;
			}
			else
			{
				end = mid - 1;
			}
		}

		int nMax = GetChildrenCount();
		while( end < nMax )
		{
			// Stop when we reach a child that has a different value
			if ( m_pTreeView->m_pSortFunc( pChild->m_pData, m_Children[end]->m_pData ) )
				return -1;

			if ( m_Children[end] == pChild )
				return end;

			++end;
		}

		return -1;
	}

    void AddChild(TreeNode *pChild)
    {
		int i = ComputeInsertionPosition( pChild );
		m_Children.InsertAfter( i, pChild );
    }

    void SetNodeExpanded(bool bExpanded)
    {
        m_bExpand = bExpanded;

        if (m_bExpand)
        {
			// see if we have any child nodes
			if (GetChildrenCount() < 1)
			{
				// we need to get our children from the control
				m_pTreeView->GenerateChildrenOfNode(m_ItemIndex);

				// if we still don't have any children, then hide the expand button
				if (GetChildrenCount() < 1)
				{
					m_bExpand = false;
					m_bExpandableWithoutChildren = false;
					m_pTreeView->InvalidateLayout();
					return;
				}
			}

            m_pExpandImage->SetText("-");
        }
        else
        {
            m_pExpandImage->SetText("+");

            // check if we've closed down on one of our children, if so, we get the focus
            int selectedItem = m_pTreeView->GetSelectedItem();
            if (selectedItem != -1 && m_pTreeView->m_NodeList[selectedItem]->HasParent(this))
            {
                m_pTreeView->SetSelectedItem(m_ItemIndex);
            }
        }
		CalculateVisibleMaxWidth();
        m_pTreeView->InvalidateLayout();
    }

    bool IsExpanded()
    {
        return m_bExpand;
    }

    int CountVisibleNodes()
    {
        int count = 1;  // count myself
        if (m_bExpand)
        {
            int i;
            for (i=0;i<m_Children.Count();i++)
            {
                count += m_Children[i]->CountVisibleNodes();
            }
        }
        return count;
    }

	void CalculateVisibleMaxWidth()
	{
		int width;
		if (m_bExpand)
		{
			int childMaxWidth = GetMaxChildrenWidth();
			childMaxWidth += TREE_INDENT_AMOUNT;

			width = max(childMaxWidth, m_iNodeWidth);
		}
		else
		{
			width = m_iNodeWidth;
		}
		if (width != m_iMaxVisibleWidth)
		{
			m_iMaxVisibleWidth = width;
			if (GetParentNode())
			{
				GetParentNode()->OnChildWidthChange();
			}
			else
			{
				m_pTreeView->InvalidateLayout();
			}
		}
	}

	void OnChildWidthChange()
	{
		CalculateVisibleMaxWidth();
	}

	int GetMaxChildrenWidth()
	{
		int maxWidth = 0;
	    int i;
        for (i=0;i<GetChildrenCount();i++)
        {
			int childWidth = m_Children[i]->GetVisibleMaxWidth();
			if (childWidth > maxWidth)
			{
				maxWidth = childWidth; 
			}
		}
		return maxWidth;
	}	

    int GetVisibleMaxWidth()
    {
		return m_iMaxVisibleWidth;
	}

    int GetDepth()
    {
        int depth = 0;
          TreeNode *pParent = GetParentNode();
        while (pParent)
        {								
            depth++;
            pParent = pParent->GetParentNode();
        }
        return depth;
    }

    bool HasParent(TreeNode *pTreeNode)
    {
        TreeNode *pParent = GetParentNode();
        while (pParent)
        {
            if (pParent == pTreeNode)
                return true;
			pParent = pParent->GetParentNode();
        }
        return false;
    }

    bool IsBeingDisplayed()
    {
        TreeNode *pParent = GetParentNode();
        while (pParent)
        {
            // our parents aren't showing us
            if (!pParent->m_bExpand)
                return false;

            pParent = pParent->GetParentNode();
        }
        return true;
    }

	virtual void SetVisible(bool state)
    {
        BaseClass::SetVisible(state);

        bool bChildrenVisible = state && m_bExpand;
        int i;
        for (i=0;i<GetChildrenCount();i++)
        {
            m_Children[i]->SetVisible(bChildrenVisible);
        }
    }

    virtual void Paint()
    {
        if (GetChildrenCount() > 0 || m_bExpandableWithoutChildren)
        {
            m_pExpandImage->Paint();
        }

        // set image
        int imageIndex = 0;
        if (IsSelected())
        {
            imageIndex = m_pData->GetInt("SelectedImage", 0);
        }
        else
        {
            imageIndex = m_pData->GetInt("Image", 0);
        }

		if (imageIndex)
		{
			IImage *pImage = m_pTreeView->GetImage(imageIndex);
			if (pImage)
			{
				m_pImagePanel->SetImage(pImage);
			}
			m_pImagePanel->Paint();
		}

        m_pText->Paint();
    }

    virtual void ApplySchemeSettings(IScheme *pScheme)
    {
    	BaseClass::ApplySchemeSettings(pScheme);
    
    	SetBorder(NULL);
		SetFgColor( m_pTreeView->GetFgColor() );
		SetBgColor( m_pTreeView->GetBgColor() );
		SetFont( m_pTreeView->GetFont() );
		m_SelectedBgColor = Color(255, 170, 0, 255);
		m_OutOfFocusSelectedBgColor = Color(170, 90, 0, 255);
    }

	virtual void SetBgColor( Color color )
	{
		BaseClass::SetBgColor( color );
		if ( m_pText )
		{
			m_pText->SetBgColor( color );
		}
	
	}
	
	virtual void SetFgColor( Color color )
	{
		BaseClass::SetFgColor( color );
		if ( m_pText )
		{
			m_pText->SetFgColor( color );
		}
	
	}

    virtual void OnSetFocus()
    {
        m_pText->RequestFocus();
    }

    void SelectPrevChild(TreeNode *pCurrentChild)
    {
        int i;
        for (i=0;i<GetChildrenCount();i++)
        {
            if (m_Children[i] == pCurrentChild)
                break;
        }

        // this shouldn't happen
        if (i == GetChildrenCount())
        {
            Assert(0);
            return;
        }

        // were we on the first child?
        if (i == 0)
        {
            // if so, then we take over!
            m_pTreeView->SetSelectedItem(m_ItemIndex);
        }
        else
        {
            // see if we need to find a grandchild of the previous sibling 
            TreeNode *pChild = m_Children[i-1];

            // if this child is expanded with children, then we have to find the last child
            while (pChild->m_bExpand && pChild->GetChildrenCount()>0)
            {
                // find the last child
                pChild = pChild->m_Children[pChild->GetChildrenCount()-1];
            }
            m_pTreeView->SetSelectedItem(pChild->m_ItemIndex);
        }
    }

    void SelectNextChild(TreeNode *pCurrentChild)
    {
        int i;
        for (i=0;i<GetChildrenCount();i++)
        {
            if (m_Children[i] == pCurrentChild)
                break;
        }

        // this shouldn't happen
        if (i == GetChildrenCount())
        {
            Assert(0);
            return;
        }

        // were we on the last child?
        if (i == GetChildrenCount() - 1)
        {
            // tell our parent to get the next child
			if (GetParentNode())
            {
				GetParentNode()->SelectNextChild(this);
			}
        }
        else
        {
            m_pTreeView->SetSelectedItem(m_Children[i+1]->m_ItemIndex);
        }
    }

    virtual void OnKeyCodeTyped(KeyCode code)
    {
        switch (code)
        {
            case KEY_LEFT:
            {
                if (m_bExpand && GetChildrenCount() > 0)
                {
                    SetNodeExpanded(false);
                }
                else
                {
                    if (GetParentNode())
                    {
                        m_pTreeView->SetSelectedItem(GetParentNode()->m_ItemIndex);
                    }
                }
                break;
            }
            case KEY_RIGHT:
            {
                if (GetChildrenCount() > 0)
                {
                    if (!m_bExpand)
                    {
                        SetNodeExpanded(true);
                    }
                    else
                    {
                        m_pTreeView->SetSelectedItem(m_Children[0]->m_ItemIndex);
                    }
                }
                break;
            }
            case KEY_UP:
            {
                if (GetParentNode())
                {
                    GetParentNode()->SelectPrevChild(this);
                }
                break;
            }
            case KEY_DOWN:
            {
                if (GetChildrenCount() > 0 && m_bExpand)
                {
                    m_pTreeView->SetSelectedItem(m_Children[0]->m_ItemIndex);
                }
                else if (GetParentNode())
                {
                    GetParentNode()->SelectNextChild(this);
                }
                break;
            }
            default:
                BaseClass::OnKeyCodeTyped(code);
                return;
        }
    }

	virtual void OnMouseWheeled(int delta)
    {
		CallParentFunction(new KeyValues("MouseWheeled", "delta", delta));
    }

    virtual void OnMousePressed(enum MouseCode code)
    {
		int x, y;
		input()->GetCursorPos(x, y);

		if (code == MOUSE_LEFT)
		{
			ScreenToLocal(x, y);
			if (x < TREE_INDENT_AMOUNT)
			{
				SetNodeExpanded(!m_bExpand);
	//            m_pTreeView->SetSelectedItem(m_ItemIndex);    // explorer doesn't actually select item when it expands an item
				// purposely commented out in case we want to change the behavior
			}
			else
			{
				SetNodeExpanded(true);
				m_pTreeView->SetSelectedItem(m_ItemIndex);
			}
		}
		else if (code == MOUSE_RIGHT)
		{
			// context menu selection
			// select the item
			m_pTreeView->SetSelectedItem(m_ItemIndex);

			// ask parent to context menu
			m_pTreeView->GenerateContextMenu(m_ItemIndex, x, y);
		}
    }
    
    void PositionAndSetVisibleNodes(int &nStart, int &nCount, int x, int &y)
    {
        // position ourselves
        if (nStart == 0)
        {
            BaseClass::SetVisible(true);
            SetPos(x, y);
            y += m_pTreeView->GetRowHeight();      // _rowHeight
            nCount--;
        }
        else // still looking for first element
        {
            nStart--;
            BaseClass::SetVisible(false);
        }

        x += TREE_INDENT_AMOUNT;
        int i;
        for (i=0;i<GetChildrenCount();i++)
        {
            if (nCount > 0 && m_bExpand)
            {
                m_Children[i]->PositionAndSetVisibleNodes(nStart, nCount, x, y);
            }
            else
            {
                m_Children[i]->SetVisible(false);   // this will make all grand children hidden as well
            }
        }
    }

    // counts items above this item including itself
    int CountVisibleIndex()
    {
        int nCount = 1; // myself
        if (GetParentNode())
        {
            int i;
            for (i=0;i<GetParentNode()->GetChildrenCount();i++)
            {
                if (GetParentNode()->m_Children[i] == this)
                    break;

                nCount += GetParentNode()->m_Children[i]->CountVisibleNodes();
            }
            return nCount + GetParentNode()->CountVisibleIndex();
        }
        else
            return nCount;
    }
};

}; // namespace vgui

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
TreeView::TreeView(Panel *parent, const char *panelName) : Panel(parent, panelName)
{
    _rowHeight = 20;
    m_pRootNode = NULL;
    m_pSelectedItem = NULL;
    m_pImageList = NULL;
    m_bDeleteImageListWhenDone = false;
    m_pSortFunc = NULL;
    m_Font = 0;

    m_pSubPanel = new TreeViewSubPanel(this);
    m_pSubPanel->SetVisible(true);
    m_pSubPanel->SetPos(0,0);

	_hbar = new ScrollBar(this, "HorizScrollBar", false);
	_hbar->AddActionSignalTarget(this);
	_hbar->SetVisible(false);

	_vbar = new ScrollBar(this, "VertScrollBar", true);
	_vbar->SetVisible(false);
	_vbar->AddActionSignalTarget(this);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
TreeView::~TreeView()
{
    if (m_pImageList)
    {
        if (m_bDeleteImageListWhenDone)
        {
            delete m_pImageList;
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TreeView::SetSortFunc(TreeViewSortFunc_t pSortFunc)
{
    m_pSortFunc = pSortFunc;
}

HFont TreeView::GetFont()
{
	return m_Font;
}	

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TreeView::SetFont(HFont font)
{
	Assert( font );
	if ( !font )
		return;

    m_Font = font;
	_rowHeight = surface()->GetFontTall(font) + 2;

    if (m_pRootNode)
    {
        m_pRootNode->SetFont(font);
    }
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int TreeView::GetRowHeight()
{
    return _rowHeight;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int TreeView::AddItem(KeyValues *data, int parentItemIndex)
{
    Assert(parentItemIndex == -1 || m_NodeList.IsValidIndex(parentItemIndex));

    TreeNode *pTreeNode = new TreeNode(m_pSubPanel, this);
    pTreeNode->m_ItemIndex = m_NodeList.AddToTail(pTreeNode);
    pTreeNode->SetKeyValues(data);
	pTreeNode->SetFont(m_Font);
	pTreeNode->SetBgColor(GetBgColor());

    // there can be only one root
    if (parentItemIndex == -1)
    {
        Assert(m_pRootNode == NULL);
        m_pRootNode = pTreeNode;
        pTreeNode->m_ParentIndex = -1;
    }
    else
    {
        pTreeNode->m_ParentIndex = parentItemIndex;

        // add to parent list
        pTreeNode->GetParentNode()->AddChild(pTreeNode);
    }

	SETUP_PANEL( pTreeNode );

	return pTreeNode->m_ItemIndex;
}


int TreeView::GetRootItemIndex()
{
	if ( m_pRootNode )
		return m_pRootNode->m_ItemIndex;
	else
		return -1;
}


int TreeView::GetNumChildren( int itemIndex )
{
	if ( itemIndex == -1 )
		return 0;

	return m_NodeList[itemIndex]->m_Children.Count();
}


int TreeView::GetChild( int iParentItemIndex, int iChild )
{
	return m_NodeList[iParentItemIndex]->m_Children[iChild]->m_ItemIndex;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int TreeView::GetItemCount(void)
{
    return m_NodeList.Count();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
KeyValues* TreeView::GetItemData(int itemIndex)
{
    if (!m_NodeList.IsValidIndex(itemIndex))
        return NULL;
    else
        return m_NodeList[itemIndex]->m_pData;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
KeyValues* TreeView::GetSelectedItemData()
{
    if (!m_pSelectedItem)
        return NULL;
    else
        return m_pSelectedItem->m_pData;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TreeView::RemoveItem(int itemIndex, bool bPromoteChildren)
{
	// HACK: there's a bug with RemoveItem where panels are lingering. This gets around it temporarily.
	bool bFullDelete = false;
	if ( itemIndex < 0 )
	{
		itemIndex = -itemIndex;
		bFullDelete = true;
	}

	if (!m_NodeList.IsValidIndex(itemIndex))
       return;


    TreeNode *pNode = m_NodeList[itemIndex];
    TreeNode *pParent = pNode->GetParentNode();

    // are we promoting the children
    if (bPromoteChildren && pParent)
    {
        int i;
        for (i=0;i<pNode->GetChildrenCount();i++)
        {
            TreeNode *pChild = pNode->m_Children[i];
            pChild->m_ParentIndex = pParent->m_ItemIndex;
        }
    }
    else
    {
        // delete our children
        if ( bFullDelete )
		{
			while ( pNode->GetChildrenCount() )
				RemoveItem( -pNode->m_Children[0]->m_ItemIndex, false );
		}
		else
		{		
			int i;
			for (i=0;i<pNode->GetChildrenCount();i++)
			{
				TreeNode *pDeleteChild = pNode->m_Children[i];
				RemoveItem(pDeleteChild->m_ItemIndex, false);
			}
		}
    }

    // remove from our parent's children list
    if (pParent)
    {
        pParent->m_Children.FindAndRemove(pNode);
    }

    // finally get rid of ourselves from the main list
    m_NodeList.Remove(itemIndex);
	
	if ( bFullDelete )
		delete pNode;
	else
		pNode->MarkForDeletion();
    
	// Make sure we don't leave ourselves with an invalid selected item.
	if ( m_pSelectedItem == pNode )
		m_pSelectedItem = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TreeView::RemoveAll()
{
    int i;
    for (i=0;i<m_NodeList.MaxElementIndex();i++)
    {
        if (!m_NodeList.IsValidIndex(i))
            continue;

        m_NodeList[i]->MarkForDeletion();
    }
    m_NodeList.RemoveAll();
	m_pRootNode = NULL;
	m_pSelectedItem = NULL;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool TreeView::ModifyItem(int itemIndex, KeyValues *data)
{
    if (!m_NodeList.IsValidIndex(itemIndex))
        return false;

    TreeNode *pNode = m_NodeList[itemIndex];
	TreeNode *pParent = pNode->GetParentNode();
	bool bReSort = ( m_pSortFunc && pParent );
	int nChildIndex = -1;
	if ( bReSort )
	{
		nChildIndex = pParent->FindChild( pNode );
	}

    pNode->SetKeyValues(data);

	// Changing the data can cause it to re-sort
	if ( bReSort )
	{
		int nChildren = pParent->GetChildrenCount();
		bool bLeftBad = (nChildIndex > 0) && m_pSortFunc( pNode->m_pData, pParent->m_Children[nChildIndex-1]->m_pData );
		bool bRightBad = (nChildIndex < nChildren - 1) && m_pSortFunc( pParent->m_Children[nChildIndex+1]->m_pData, pNode->m_pData );
		if ( bLeftBad || bRightBad )
		{
			pParent->m_Children.Remove( nChildIndex );
			pParent->AddChild( pNode );
		}
	}

    InvalidateLayout();

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: set the fg color of an element in the tree view
//-----------------------------------------------------------------------------
void TreeView::SetItemFgColor(int itemIndex, Color color)
{
	Assert( m_NodeList.IsValidIndex(itemIndex) );
	if ( !m_NodeList.IsValidIndex(itemIndex) )
		return;

	TreeNode *pNode = m_NodeList[itemIndex];
	pNode->SetFgColor( color );
}

//-----------------------------------------------------------------------------
// Purpose: set the bg color of an element in the tree view
//-----------------------------------------------------------------------------
void TreeView::SetItemBgColor(int itemIndex, Color color)
{
	Assert( m_NodeList.IsValidIndex(itemIndex) );
	if ( !m_NodeList.IsValidIndex(itemIndex) )
		return;

	TreeNode *pNode = m_NodeList[itemIndex];
	pNode->SetBgColor( color );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int TreeView::GetItemParent(int itemIndex)
{
	return m_NodeList[itemIndex]->m_ParentIndex;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TreeView::SetImageList(ImageList *imageList, bool deleteImageListWhenDone)
{
    if (m_pImageList)
    {
        if (m_bDeleteImageListWhenDone)
        {
            delete m_pImageList;
        }
    }
    m_pImageList = imageList;
    m_bDeleteImageListWhenDone = deleteImageListWhenDone;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
IImage *TreeView::GetImage(int index)
{
    return m_pImageList->GetImage(index);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TreeView::SetSelectedItem(int itemIndex)
{
    if (!m_NodeList.IsValidIndex(itemIndex))
        return;

    m_pSelectedItem = m_NodeList[itemIndex];
    m_pSelectedItem->RequestFocus();

    MakeItemVisible(itemIndex);

    PostActionSignal(new KeyValues("TreeViewItemSelected", "itemIndex", itemIndex));
    InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int TreeView::GetSelectedItem()
{
    if (m_pSelectedItem)
    {
        return m_pSelectedItem->m_ItemIndex;
    }
    else
        return -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool TreeView::IsItemIDValid(int itemIndex)
{
    return m_NodeList.IsValidIndex(itemIndex);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int TreeView::GetHighestItemID()
{
    return m_NodeList.MaxElementIndex();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TreeView::ExpandItem(int itemIndex, bool bExpand)
{
    if (!m_NodeList.IsValidIndex(itemIndex))
        return;

    m_NodeList[itemIndex]->SetNodeExpanded(bExpand);
    InvalidateLayout();
}

bool TreeView::IsItemExpanded( int itemIndex )
{
    if (!m_NodeList.IsValidIndex(itemIndex))
        return false;

    return m_NodeList[itemIndex]->IsExpanded();
}
	

//-----------------------------------------------------------------------------
// Purpose: Scrolls the list according to the mouse wheel movement
//-----------------------------------------------------------------------------
void TreeView::OnMouseWheeled(int delta)
{
	int val = _vbar->GetValue();
	val -= (delta * 3);
	_vbar->SetValue(val);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TreeView::OnSizeChanged(int wide, int tall)
{
	BaseClass::OnSizeChanged(wide, tall);
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TreeView::PerformLayout()
{
    if (!m_pRootNode)
        return;

    int wide, tall;
    GetSize( wide, tall );

    bool vbarNeeded = false;
    bool hbarNeeded = false;

    // okay we have to check if we need either scroll bars, since if we need one
    // it might make it necessary to have the other one
    int nodesVisible = tall / _rowHeight;

	// count the number of visible items
	int visibleItemCount = m_pRootNode->CountVisibleNodes();
    int maxWidth = m_pRootNode->GetVisibleMaxWidth() + 10; // 10 pixel buffer

    vbarNeeded = visibleItemCount > nodesVisible;

    if (!vbarNeeded)
    {
        if (maxWidth > wide)
        {
            hbarNeeded = true;

            // recalculate if vbar is needed now
            // double check that we really don't need it
            nodesVisible = (tall - _hbar->GetTall()) / _rowHeight;
            vbarNeeded = visibleItemCount > nodesVisible;
        }
    }
    else
    {
        // we've got the vertical bar here, so shrink the width
        hbarNeeded = maxWidth > (wide - (_vbar->GetWide()+2));

        if (hbarNeeded)
        {
            nodesVisible = (tall - _hbar->GetTall()) / _rowHeight;
        }
    }

    int subPanelWidth = wide;
    int subPanelHeight = tall;

	int vbarPos = 0;
    if (vbarNeeded)
    {
        subPanelWidth -= (_vbar->GetWide() + 2);
        int barSize = tall;
        if (hbarNeeded)
        {
            barSize -= _hbar->GetTall();
        }

    	//!! need to make it recalculate scroll positions
    	_vbar->SetVisible(true);
    	_vbar->SetEnabled(false);
    	_vbar->SetRangeWindow( nodesVisible );
    	_vbar->SetRange( 0, visibleItemCount);	
    	_vbar->SetButtonPressedScrollValue( 1 );

    	_vbar->SetPos(wide - (_vbar->GetWide() + WINDOW_BORDER_WIDTH), 0);
    	_vbar->SetSize(_vbar->GetWide(), barSize - 2);

        // need to figure out
        vbarPos = _vbar->GetValue();
    }
    else
    {
    	_vbar->SetVisible(false);
    }

    int hbarPos = 0;
    if (hbarNeeded)
    {
        subPanelHeight -= (_hbar->GetTall() + 2);
        int barSize = wide;
        if (vbarNeeded)
        {
            barSize -= _vbar->GetWide();
        }
        _hbar->SetVisible(true);
        _hbar->SetEnabled(false);
        _hbar->SetRangeWindow( barSize );
        _hbar->SetRange( 0, maxWidth);	
        _hbar->SetButtonPressedScrollValue( 10 );

        _hbar->SetPos(0, tall - (_hbar->GetTall() + WINDOW_BORDER_WIDTH));
        _hbar->SetSize(barSize - 2, _hbar->GetTall());

        hbarPos = _hbar->GetValue();
    }
    else
    {
    	_hbar->SetVisible(false);
    }

    m_pSubPanel->SetSize(subPanelWidth, subPanelHeight);

	int y = 0;
    m_pRootNode->PositionAndSetVisibleNodes(vbarPos, visibleItemCount, -hbarPos, y);
    
    Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TreeView::MakeItemVisible(int itemIndex)
{
    // first make sure that all parents are expanded
    TreeNode *pNode = m_NodeList[itemIndex];
    TreeNode *pParent = pNode->GetParentNode();
    while (pParent)
    {
        if (!pParent->m_bExpand)
        {
            pParent->SetNodeExpanded(true);
        }
        pParent = pParent->GetParentNode();
    }

    // recalculate scroll bar due to possible exapnsion
    PerformLayout();

    if (!_vbar->IsVisible())
        return;

    int visibleIndex = pNode->CountVisibleIndex()-1;
    int range = _vbar->GetRangeWindow();
    int vbarPos = _vbar->GetValue();

    // do we need to scroll up or down?
    if (visibleIndex < vbarPos)
    {
        _vbar->SetValue(visibleIndex);
    }
    else if (visibleIndex+1 > vbarPos+range)
    {
        _vbar->SetValue(visibleIndex+1-range);
    }
    InvalidateLayout();
}

void TreeView::GetVBarInfo( int &top, int &nItemsVisible )
{
    int wide, tall;
    GetSize( wide, tall );
    nItemsVisible = tall / _rowHeight;

    if ( _vbar->IsVisible() )
	{
		top = _vbar->GetValue();
	}
	else
	{
		top = 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TreeView::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBorder(pScheme->GetBorder("ButtonDepressedBorder"));
	SetBgColor(GetSchemeColor("TreeView.BgColor", GetSchemeColor("WindowDisabledBgColor", pScheme), pScheme));
	SetFont( pScheme->GetFont( "Default", IsProportional() ) );
	m_pSubPanel->SetBgColor( GetBgColor() );

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TreeView::SetBgColor( Color color ) 
{
	BaseClass::SetBgColor( color );
	m_pSubPanel->SetBgColor( color );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TreeView::OnSliderMoved()
{
	InvalidateLayout();
	Repaint();
}
