/********************************************************************
	created:	2006/08/29
	created:	29:8:2006   18:35
	filename: 	cl_dll\ff\vgui\ff_options\ff_dlightoptions.cpp
	file path:	cl_dll\ff\vgui\ff_dlightoptions
	file base:	ff_dlightoptions
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Dynamic light options page within fortress options

	notes: 
	
	Used to be contained in ff_options.cpp
	Separated by elmo 01/09/2010
*********************************************************************/

#include "cbase.h"
#include "ff_dlightoptions.h"

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

// dlight cvarsesesesssssssesssssssssssssssssssssss
ConVar cl_ffdlight_explosion( "cl_ffdlight_explosion", "1.0", FCVAR_ARCHIVE, "Radius scale of the dynamic light from an explosion (0 disables this type of dlight).", TRUE, 0.0f, TRUE, 2.0f );
ConVar cl_ffdlight_muzzle( "cl_ffdlight_muzzle", "1.0", FCVAR_ARCHIVE, "Radius scale of the dynamic light from a muzzle flash (0 disables this type of dlight).", TRUE, 0.0f, TRUE, 2.0f );
ConVar cl_ffdlight_flamethrower( "cl_ffdlight_flamethrower", "1.0", FCVAR_ARCHIVE, "Radius scale of the dynamic light from a flamethrower (0 disables this type of dlight).", TRUE, 0.0f, TRUE, 2.0f );
ConVar cl_ffdlight_ignited( "cl_ffdlight_ignited", "1.0", FCVAR_ARCHIVE, "Radius scale of the dynamic light from an ignited player or object (0 disables this type of dlight).", TRUE, 0.0f, TRUE, 2.0f );
ConVar cl_ffdlight_napalm( "cl_ffdlight_napalm", "1.0", FCVAR_ARCHIVE, "Radius scale of the dynamic light from napalm flames (0 disables this type of dlight).", TRUE, 0.0f, TRUE, 2.0f );
ConVar cl_ffdlight_ic( "cl_ffdlight_ic", "1.0", FCVAR_ARCHIVE, "Radius scale of the dynamic light from an IC projectile (0 disables this type of dlight).", TRUE, 0.0f, TRUE, 2.0f );
ConVar cl_ffdlight_rocket( "cl_ffdlight_rocket", "1.0", FCVAR_ARCHIVE, "Radius scale of the dynamic light from a rocket (0 disables this type of dlight).", TRUE, 0.0f, TRUE, 2.0f );
ConVar cl_ffdlight_rail( "cl_ffdlight_rail", "1.0", FCVAR_ARCHIVE, "Radius scale of the dynamic light from a rail (0 disables this type of dlight).", TRUE, 0.0f, TRUE, 2.0f );
ConVar cl_ffdlight_conc( "cl_ffdlight_conc", "1.0", FCVAR_ARCHIVE, "Radius scale of the dynamic light from a concussion grenade (0 disables this type of dlight).", TRUE, 0.0f, TRUE, 2.0f );
ConVar cl_ffdlight_flashlight( "cl_ffdlight_flashlight", "1.0", FCVAR_ARCHIVE, "Radius scale of the dynamic light from a concussion grenade (0 disables this type of dlight).", TRUE, 0.0f, TRUE, 2.0f );
ConVar cl_ffdlight_generic( "cl_ffdlight_generic", "1.0", FCVAR_ARCHIVE, "Radius scale of the dynamic light from a generic source (0 disables this type of dlight).", TRUE, 0.0f, TRUE, 2.0f );

// Wrapper CVAR for an archiveable r_maxdlights -- thanks, mirv
void FF_MaxDLights_Callback(ConVar *var, char const *pOldString)
{
	ConVar *c = cvar->FindVar("r_maxdlights");
	if (c)
		c->SetValue(var->GetString());
}
ConVar r_maxdlights_ff( "r_maxdlights_ff", "32", FCVAR_ARCHIVE, "Sets the maximum number of dynamic lights allowed.", TRUE, 0.0f, TRUE, FF_MAXDLIGHTS, FF_MaxDLights_Callback );


CFFDLightOptions::CFFDLightOptions(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
{
	m_pFFDLightMax = new CFFInputSlider(this, "FFDLightMax", "FFDLightMaxInput");
	m_pFFDLightMax->SetRange(0, FF_MAXDLIGHTS);
	m_pFFDLightMax->SetValue(32);

	m_pFFDLightExplosion = new CFFInputSlider(this, "FFDLightExplosion", "FFDLightExplosionInput");
	m_pFFDLightExplosion->SetRange(0, 200);
	m_pFFDLightExplosion->SetValue(1);

	m_pFFDLightMuzzle = new CFFInputSlider(this, "FFDLightMuzzle", "FFDLightMuzzleInput");
	m_pFFDLightMuzzle->SetRange(0, 200);
	m_pFFDLightMuzzle->SetValue(1);

	m_pFFDLightFlamethrower = new CFFInputSlider(this, "FFDLightFlamethrower", "FFDLightFlamethrowerInput");
	m_pFFDLightFlamethrower->SetRange(0, 200);
	m_pFFDLightFlamethrower->SetValue(1);

	m_pFFDLightIgnited = new CFFInputSlider(this, "FFDLightIgnited", "FFDLightIgnitedInput");
	m_pFFDLightIgnited->SetRange(0, 200);
	m_pFFDLightIgnited->SetValue(1);

	m_pFFDLightNapalm = new CFFInputSlider(this, "FFDLightNapalm", "FFDLightNapalmInput");
	m_pFFDLightNapalm->SetRange(0, 200);
	m_pFFDLightNapalm->SetValue(1);

	m_pFFDLightIC = new CFFInputSlider(this, "FFDLightIC", "FFDLightICInput");
	m_pFFDLightIC->SetRange(0, 200);
	m_pFFDLightIC->SetValue(1);

	m_pFFDLightRocket = new CFFInputSlider(this, "FFDLightRocket", "FFDLightRocketInput");
	m_pFFDLightRocket->SetRange(0, 200);
	m_pFFDLightRocket->SetValue(1);

	m_pFFDLightRail = new CFFInputSlider(this, "FFDLightRail", "FFDLightRailInput");
	m_pFFDLightRail->SetRange(0, 200);
	m_pFFDLightRail->SetValue(1);

	m_pFFDLightConc = new CFFInputSlider(this, "FFDLightConc", "FFDLightConcInput");
	m_pFFDLightConc->SetRange(0, 200);
	m_pFFDLightConc->SetValue(1);

	m_pFFDLightFlashlight = new CFFInputSlider(this, "FFDLightFlashlight", "FFDLightFlashlightInput");
	m_pFFDLightFlashlight->SetRange(0, 200);
	m_pFFDLightFlashlight->SetValue(1);

	m_pFFDLightGeneric = new CFFInputSlider(this, "FFDLightGeneric", "FFDLightGenericInput");
	m_pFFDLightGeneric->SetRange(0, 200);
	m_pFFDLightGeneric->SetValue(1);

	LoadControlSettings("resource/ui/FFOptionsSubDLights.res");
}

//-----------------------------------------------------------------------------
// Purpose: Load all the fonts we need
//-----------------------------------------------------------------------------
void CFFDLightOptions::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

//-----------------------------------------------------------------------------
// Purpose: This builds keyvalues for each weapon and puts them into an
//			all-conquering keyvalue that is then saved to file.
//-----------------------------------------------------------------------------
void CFFDLightOptions::Apply()
{
	UpdateConVars();
}

//-----------------------------------------------------------------------------
// Purpose: Load all the settings from file into memory via keyvalues
//-----------------------------------------------------------------------------
void CFFDLightOptions::Load()
{
	// Now update the sliders
	UpdateSliders();
}

//-----------------------------------------------------------------------------
// Purpose: Reload the stuff from file for now
//-----------------------------------------------------------------------------
void CFFDLightOptions::Reset()
{
	Load();
}

//-----------------------------------------------------------------------------
// Purpose: Update the ConVars
//-----------------------------------------------------------------------------
void CFFDLightOptions::UpdateConVars()
{
	r_maxdlights_ff.SetValue(m_pFFDLightMax->GetValue());

	// divide by 100 because the sliders are 0 to 200 while the cvars are 0.0 to 2.0
	cl_ffdlight_explosion.SetValue((float)m_pFFDLightExplosion->GetValue() / 100.0f);
	cl_ffdlight_muzzle.SetValue((float)m_pFFDLightMuzzle->GetValue() / 100.0f);
	cl_ffdlight_flamethrower.SetValue((float)m_pFFDLightFlamethrower->GetValue() / 100.0f);
	cl_ffdlight_ignited.SetValue((float)m_pFFDLightIgnited->GetValue() / 100.0f);
	cl_ffdlight_napalm.SetValue((float)m_pFFDLightNapalm->GetValue() / 100.0f);
	cl_ffdlight_ic.SetValue((float)m_pFFDLightIC->GetValue() / 100.0f);
	cl_ffdlight_rocket.SetValue((float)m_pFFDLightRocket->GetValue() / 100.0f);
	cl_ffdlight_rail.SetValue((float)m_pFFDLightRail->GetValue() / 100.0f);
	cl_ffdlight_conc.SetValue((float)m_pFFDLightConc->GetValue() / 100.0f);
	cl_ffdlight_flashlight.SetValue((float)m_pFFDLightFlashlight->GetValue() / 100.0f);
	cl_ffdlight_generic.SetValue((float)m_pFFDLightGeneric->GetValue() / 100.0f);
}

//-----------------------------------------------------------------------------
// Purpose: Update the sliders
//-----------------------------------------------------------------------------
void CFFDLightOptions::UpdateSliders()
{
	m_pFFDLightMax->SetValue(r_maxdlights_ff.GetFloat(), false);

	// multiply by 100 because the sliders are 0 to 200 while the cvars are 0.0 to 2.0
	m_pFFDLightExplosion->SetValue(cl_ffdlight_explosion.GetFloat() * 100.0f, false);
	m_pFFDLightMuzzle->SetValue(cl_ffdlight_muzzle.GetFloat() * 100.0f, false);
	m_pFFDLightFlamethrower->SetValue(cl_ffdlight_flamethrower.GetFloat() * 100.0f, false);
	m_pFFDLightIgnited->SetValue(cl_ffdlight_ignited.GetFloat() * 100.0f, false);
	m_pFFDLightNapalm->SetValue(cl_ffdlight_napalm.GetFloat() * 100.0f, false);
	m_pFFDLightIC->SetValue(cl_ffdlight_ic.GetFloat() * 100.0f, false);
	m_pFFDLightRocket->SetValue(cl_ffdlight_rocket.GetFloat() * 100.0f, false);
	m_pFFDLightRail->SetValue(cl_ffdlight_rail.GetFloat() * 100.0f, false);
	m_pFFDLightConc->SetValue(cl_ffdlight_conc.GetFloat() * 100.0f, false);
	m_pFFDLightFlashlight->SetValue(cl_ffdlight_flashlight.GetFloat() * 100.0f, false);
	m_pFFDLightGeneric->SetValue(cl_ffdlight_generic.GetFloat() * 100.0f, false);
}