#ifndef FF_MISCOPTIONS_H
#define FF_MISCOPTIONS_H

#include "ff_optionspage.h"

using namespace vgui;

// Jiggles: Begin Miscellaneous Options Tab
//=============================================================================
// This Tab lets the player enable/disable various FF specific options
//=============================================================================
class CFFMiscOptions : public CFFOptionsPage
{
	DECLARE_CLASS_SIMPLE(CFFMiscOptions, CFFOptionsPage);

#define	ROW_HEIGHT 24
#define TITLE_SPACER 5

private:

char	m_szSourceFile[128];

public:

	//-----------------------------------------------------------------------------
	// Purpose: Populate all the menu stuff
	//-----------------------------------------------------------------------------
	CFFMiscOptions(Panel *parent, char const *panelName, const char *pszSourceFile);

	//-----------------------------------------------------------------------------
	// Purpose: Apply the changes	
	//-----------------------------------------------------------------------------
	void Apply();

	//-----------------------------------------------------------------------------
	// Purpose: Just load again to reset
	//-----------------------------------------------------------------------------
	void Reset();

	//-----------------------------------------------------------------------------
	// Purpose: Find the appropriate ConVar states and update the Check Boxes
	//-----------------------------------------------------------------------------
	void Load();

	//-----------------------------------------------------------------------------
	// Purpose: Ask the person who made the damn function!
	//-----------------------------------------------------------------------------
	int GetComboBoxOption(ComboBox *cb, const char *value, const char *keyname = "value");
};

#endif