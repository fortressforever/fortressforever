/********************************************************************
	created:	2006/08/29
	created:	29:8:2006   18:35
	filename: 	cl_dll\ff\vgui\ff_options\ff_crosshairoptions.h
	file path:	cl_dll\ff\vgui\ff_crosshairoptions
	file base:	ff_crosshairoptions
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Crosshair options page within fortress options

	notes: 
	
	Used to be contained in ff_options.cpp
	Separated by elmo 01/09/2010
*********************************************************************/

#ifndef FF_CROSSHAIROPTIONS_H
#define FF_CROSSHAIROPTIONS_H

#include "ff_optionspage.h"

#include "ff_weapon_base.h"
extern const char *s_WeaponAliasInfo[];

#include "filesystem.h"
extern IFileSystem **pFilesystem;

#include "KeyValues.h"

#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/CheckButton.h>

#include "ff_inputslider.h"

#define CROSSHAIRS_FILE		"crosshairs.vdf"
#define CROSSHAIR_SIZES		5					// This needs to be matched in hud_crosshair.h
#define FF_CROSSHAIR_GLOBAL FF_WEAPON_NONE
#define FF_CROSSHAIR_WEAPON_MAX FF_WEAPON_TOMMYGUN
#define FF_CROSSHAIR_HIT 	FF_CROSSHAIR_WEAPON_MAX + 1
#define FF_NUM_CROSSHAIRS 	FF_CROSSHAIR_HIT + 1

using namespace vgui;

//=============================================================================
// Our crosshair options page. This is quite big.
//=============================================================================
class CFFCrosshairOptions : public CFFOptionsPage
{
	DECLARE_CLASS_SIMPLE(CFFCrosshairOptions, CFFOptionsPage);

public:

	//-----------------------------------------------------------------------------
	// Purpose: Populate all the menu stuff
	//-----------------------------------------------------------------------------
	CFFCrosshairOptions(Panel *parent, char const *panelName);

	//-----------------------------------------------------------------------------
	// Purpose: Allow all inner options to be enabled or disabled quickly
	//-----------------------------------------------------------------------------
	virtual void AllowInnerChanges(bool state);

	//-----------------------------------------------------------------------------
	// Purpose: Allow all outer options to be enabled or disabled quickly
	//-----------------------------------------------------------------------------
	virtual void AllowOuterChanges(bool state);

	//-----------------------------------------------------------------------------
	// Purpose: Allow weapon selection stuff to be disabled or enabled quickly
	//-----------------------------------------------------------------------------
	virtual void AllowWeaponSelection(bool state);

	//-----------------------------------------------------------------------------
	// Purpose: Load all the fonts we need
	//-----------------------------------------------------------------------------
	virtual void ApplySchemeSettings(IScheme *pScheme);

	//-----------------------------------------------------------------------------
	// Purpose: This builds keyvalues for each weapon and puts them into an
	//			all-conquering keyvalue that is then saved to file.
	//-----------------------------------------------------------------------------
	void Apply();

	//-----------------------------------------------------------------------------
	// Purpose: Load all the settings from file into memory via keyvalues
	//-----------------------------------------------------------------------------
	void Load();

	//-----------------------------------------------------------------------------
	// Purpose: Reload the stuff from file for now
	//-----------------------------------------------------------------------------
	void Reset();

	//-----------------------------------------------------------------------------
	// Purpose: Get crosshair for a weapon
	//-----------------------------------------------------------------------------
	void GetCrosshair(FFWeaponID iWeapon, char &innerChar, Color &innerCol, int &innerSize, char &outerChar, Color &outerCol, int &outerSize);
	
	//-----------------------------------------------------------------------------
	// Purpose: Get crosshair for a weapon
	//-----------------------------------------------------------------------------
	void GetHitCrosshair(char &innerChar, Color &innerCol, int &innerSize, char &outerChar, Color &outerCol, int &outerSize);

private:
	//-----------------------------------------------------------------------------
	// Purpose: Catch the slider moving
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnUpdateSliders, "SliderMoved", data);

	//-----------------------------------------------------------------------------
	// Purpose: Catch the combo box changing
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnUpdateCombos, "TextChanged", data);

	//-----------------------------------------------------------------------------
	// Purpose: Catch checkbox updating
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnUpdateCheckbox, "CheckButtonChecked", data);

	//-----------------------------------------------------------------------------
	// Purpose: Update the crosshair
	//			This isn't a particularly great function, it should really be
	//			split up into separate responsibilities (update crosshair data,
	//			refresh crosshair preview).
	//-----------------------------------------------------------------------------
	void UpdateCrosshairs();

	//-----------------------------------------------------------------------------
	// Purpose: Update the sliders if a combo box has been changed
	//-----------------------------------------------------------------------------
	void UpdateSliders();

private:

    CFFInputSlider	*m_pInnerScale, *m_pOuterScale;
	CFFInputSlider	*m_pInnerRed, *m_pOuterRed;
	CFFInputSlider	*m_pInnerGreen, *m_pOuterGreen;
	CFFInputSlider	*m_pInnerBlue, *m_pOuterBlue;
	CFFInputSlider	*m_pInnerAlpha, *m_pOuterAlpha;

	ComboBox		*m_pInnerCharacter, *m_pOuterCharacter;
	ComboBox		*m_pCrosshairComboBox;

	Label			*m_pInnerCrosshair, *m_pOuterCrosshair;

	CheckButton		*m_pInnerUseGlobal, *m_pOuterUseGlobal;
	CheckButton		*m_pForceGlobal;

	ImagePanel		*m_pCrosshairBackground;

	HFont			m_hPrimaryCrosshairs[CROSSHAIR_SIZES];
	HFont			m_hSecondaryCrosshairs[CROSSHAIR_SIZES];

	typedef struct
	{
		int		innerScale, outerScale;
		int		innerR, innerG, innerB, innerA;
		int		outerR, outerG, outerB, outerA;
		char	innerChar, outerChar;
		bool	innerUseGlobal, outerUseGlobal;
	} CrosshairInfo_t;

	CrosshairInfo_t	m_sCrosshairInfo[FF_NUM_CROSSHAIRS];

	bool	m_bForceGlobalCrosshair;
};

#endif