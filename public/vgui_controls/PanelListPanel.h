//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef PANELLISTPANEL_H
#define PANELLISTPANEL_H

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

//-----------------------------------------------------------------------------
// Purpose: A list of variable height child panels
//  each list item consists of a label-panel pair. Height of the item is
// determined from the lable.
//-----------------------------------------------------------------------------
class PanelListPanel : public Panel
{
	DECLARE_CLASS_SIMPLE( PanelListPanel, Panel );

public:
	PanelListPanel( vgui::Panel *parent, char const *panelName );
	~PanelListPanel();

	// DATA & ROW HANDLING
	// The list now owns the panel
	virtual int AddItem( Panel *labelPanel, Panel *panel );
	virtual int	GetItemCount();

	virtual Panel *GetItemLabel(int itemID); 
	virtual Panel *GetItemPanel(int itemID); 

	virtual void RemoveItem(int itemID); // removes an item from the table (changing the indices of all following items)
	virtual void DeleteAllItems(); // clears and deletes all the memory used by the data items
	void RemoveAll();

	// painting
	virtual vgui::Panel *GetCellRenderer( int row );

	// layout
	void SetFirstColumnWidth( int width );
	int GetFirstColumnWidth();
	void MoveScrollBarToTop();

	// selection
	void SetSelectedPanel( Panel *panel );
	Panel *GetSelectedPanel();
	/*
		On a panel being selected, a message gets sent to it
			"PanelSelected"		int "state"
		where state is 1 on selection, 0 on deselection
	*/

	void		SetVerticalBufferPixels( int buffer );

protected:
	// overrides
	virtual void OnSizeChanged(int wide, int tall);
	MESSAGE_FUNC_INT( OnSliderMoved, "ScrollBarSliderMoved", position );
	virtual void PerformLayout();
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnMouseWheeled(int delta);

private:
	int	ComputeVPixelsNeeded();

	enum { DEFAULT_HEIGHT = 24, PANELBUFFER = 5 };

	typedef struct dataitem_s
	{
		// Always store a panel pointer
		Panel *panel;
		Panel *labelPanel;
	} DATAITEM;

	// list of the column headers

	CUtlLinkedList<DATAITEM, int>		m_DataItems;
	CUtlVector<int>						m_SortedItems;

	ScrollBar				*m_vbar;
	Panel					*m_pPanelEmbedded;

	PHandle					m_hSelectedItem;
	int						m_iFirstColumnWidth;
	int						m_iDefaultHeight;
	int						m_iPanelBuffer;
};

}
#endif // PANELLISTPANEL_H
