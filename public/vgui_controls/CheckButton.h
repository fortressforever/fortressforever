//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CHECKBUTTON_H
#define CHECKBUTTON_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/ToggleButton.h>

class CheckImage;

namespace vgui
{

class TextImage;

//-----------------------------------------------------------------------------
// Purpose: Tick-box button
//-----------------------------------------------------------------------------
class CheckButton : public ToggleButton
{
	DECLARE_CLASS_SIMPLE( CheckButton, ToggleButton );

public:
	CheckButton(Panel *parent, const char *panelName, const char *text);
	~CheckButton();

	// Check the button
	virtual void SetSelected(bool state);

	// sets whether or not the state of the check can be changed
	// if this is set to false, then no input in the code or by the user can change it's state
	virtual void SetCheckButtonCheckable(bool state);

protected:
	virtual void ApplySchemeSettings(IScheme *pScheme);
	MESSAGE_FUNC_PTR( OnCheckButtonChecked, "CheckButtonChecked", panel );
	virtual Color GetButtonFgColor();

	virtual IBorder *GetBorder(bool depressed, bool armed, bool selected, bool keyfocus);

	/* MESSAGES SENT
		"CheckButtonChecked" - sent when the check button state is changed
			"state"	- button state: 1 is checked, 0 is unchecked
	*/

private:
	enum { CHECK_INSET = 6 };
	CheckImage *_checkBoxImage;
	bool m_bCheckButtonCheckable;
	Color _selectedFgColor;
	friend CheckImage;
};

} // namespace vgui

#endif // CHECKBUTTON_H
