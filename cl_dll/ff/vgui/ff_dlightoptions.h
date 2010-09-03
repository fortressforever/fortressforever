/********************************************************************
	created:	2006/08/29
	created:	29:8:2006   18:35
	filename: 	cl_dll\ff\vgui\ff_options\ff_dlightoptions.h
	file path:	cl_dll\ff\vgui\ff_dlightoptions
	file base:	ff_dlightoptions
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Dynamic light options page within fortress options

	notes: 
	
	Used to be contained in ff_options.cpp
	Separated by elmo 01/09/2010
*********************************************************************/

#ifndef FF_DLIGHTOPTIONS_H
#define FF_DLIGHTOPTIONS_H

#include "ff_optionspage.h"
#include "ff_inputslider.h"

#define FF_MAXDLIGHTS 512

using namespace vgui;

//=============================================================================
// Our dlight options page. This also is quite big.
//=============================================================================
class CFFDLightOptions : public CFFOptionsPage
{
	DECLARE_CLASS_SIMPLE(CFFDLightOptions, CFFOptionsPage);

public:

	//-----------------------------------------------------------------------------
	// Purpose: Populate all the menu stuff
	//-----------------------------------------------------------------------------
	CFFDLightOptions(Panel *parent, char const *panelName);

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

private:
	//-----------------------------------------------------------------------------
	// Purpose: Update the ConVars
	//-----------------------------------------------------------------------------
	void UpdateConVars();

	//-----------------------------------------------------------------------------
	// Purpose: Update the sliders
	//-----------------------------------------------------------------------------
	void UpdateSliders();

private:

	CFFInputSlider	*m_pFFDLightMax;
	CFFInputSlider	*m_pFFDLightExplosion;
	CFFInputSlider	*m_pFFDLightMuzzle;
	CFFInputSlider	*m_pFFDLightFlamethrower;
	CFFInputSlider	*m_pFFDLightIgnited;
	CFFInputSlider	*m_pFFDLightNapalm;
	CFFInputSlider	*m_pFFDLightIC;
	CFFInputSlider	*m_pFFDLightRocket;
	CFFInputSlider	*m_pFFDLightRail;
	CFFInputSlider	*m_pFFDLightConc;
	CFFInputSlider	*m_pFFDLightFlashlight;
	CFFInputSlider	*m_pFFDLightGeneric;
};
#endif