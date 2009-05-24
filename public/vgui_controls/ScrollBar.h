//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/Panel.h>

namespace vgui
{

class Button;
class ScrollBarSlider;

//-----------------------------------------------------------------------------
// Purpose: Generic scrollbar
//			Uses Buttons & SliderBars for the main functionality
//-----------------------------------------------------------------------------
class ScrollBar : public Panel
{
	DECLARE_CLASS_SIMPLE( ScrollBar, Panel );

public:
	ScrollBar(Panel *parent, const char *panelName, bool vertical);

	// Set the value of the scroll bar slider.
	virtual void    SetValue(int value);

	// Get the value of the scroll bar slider.
	virtual int     GetValue();

	// Set the rangeof numbers the slider can scroll through
	virtual void    SetRange(int min,int max);

    virtual void    GetRange(int &min, int &max);

	// Set how many lines are displayed at one time 
	// in the window the scroll bar is attached to.
	virtual void    SetRangeWindow(int rangeWindow);
	
	// Get how many lines are displayed at one time 
	// in the window the scroll bar is attached to.
	virtual int    GetRangeWindow();

	// Check if the scrollbar is vertical or horizontal
	virtual bool    IsVertical();

	// Purpose: Check if the slider can move through one or more pixels per
	// unit of its range.
	virtual bool    HasFullRange();

	// Setup the indexed scroll bar button with the input params.
	virtual void    SetButton(Button* button,int index);
	// Return the indexed scroll bar button
	virtual Button *GetButton(int index);
	// Set up the slider.
	virtual void    SetSlider(ScrollBarSlider* slider);
	// Return a pointer to the slider.
	virtual ScrollBarSlider *GetSlider();
	// Set how far the scroll bar slider moves 
	// when a scroll bar button is pressed
	virtual void    SetButtonPressedScrollValue(int value);

	virtual void    Validate();
	
	// Update and look for clicks when mouse is in the scroll bar window.
	virtual void	OnMouseFocusTicked();

	// Set the slider's Paint border enabled.
	virtual void   SetPaintBorderEnabled(bool state);
	// Set the slider's Paint background enabled.
	virtual void   SetPaintBackgroundEnabled(bool state);
	// Set the slider's Paint enabled.
	virtual void   SetPaintEnabled(bool state);

	/* MESSAGES SENT:
		"ScrollBarSliderMoved"
			"position" - new value of the slider
	*/

protected:

	virtual void PerformLayout();
	virtual void SendSliderMoveMessage(int value);
	virtual void ApplySchemeSettings(IScheme *pScheme);
	virtual void OnSizeChanged(int wide, int tall);

	MESSAGE_FUNC_INT( OnSliderMoved, "ScrollBarSliderMoved", position );
	virtual void RespondToScrollArrow(int const direction);

private:
	Button* _button[2];
	ScrollBarSlider* _slider;
	int     _buttonPressedScrollValue;
	int		_scrollDelay; // used to control delays in scrolling
	bool	_respond;
};

}

#endif // SCROLLBAR_H
