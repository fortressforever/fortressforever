#ifndef FF_TIMEROPTIONS_H
#define FF_TIMEROPTIONS_H

#include "ff_optionspage.h"

#include "KeyValues.h"
#include <vgui_controls/ComboBox.h>

using namespace vgui;

//=============================================================================
// This is a relatively simple timer select screen
//=============================================================================
class CFFTimerOptions : public CFFOptionsPage
{
	DECLARE_CLASS_SIMPLE(CFFTimerOptions, CFFOptionsPage);

public:

	//-----------------------------------------------------------------------------
	// Purpose: Populate all the menu stuff
	//-----------------------------------------------------------------------------
	CFFTimerOptions(Panel *parent, char const *panelName);

	//-----------------------------------------------------------------------------
	// Purpose: This is a bit messy. We need to save the filename without the
	//			extension to the cvar.
	//-----------------------------------------------------------------------------
	void Apply();

	//-----------------------------------------------------------------------------
	// Purpose: Just load again to reset
	//-----------------------------------------------------------------------------
	void Reset();

	//-----------------------------------------------------------------------------
	// Purpose: Load all the timers into the combobox
	//-----------------------------------------------------------------------------
	void Load();

private:

	//-----------------------------------------------------------------------------
	// Purpose: Catch the play button being pressed and play the currently
	//			selected timer
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);

	ComboBox	*m_pTimers;
	Button		*m_pPlayButton;

	ComboBox	*m_pBeeps;
	Button		*m_pPlayButton2;
};

#endif