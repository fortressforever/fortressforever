/********************************************************************
	created:	2006/08/29
	created:	29:8:2006   18:39
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_options.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_options
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "ff_options.h"

#include "KeyValues.h"
#include "filesystem.h"
#include "utlmap.h"
#include "vstdlib/icommandline.h"

#include "ff_weapon_base.h"
extern const char *s_WeaponAliasInfo[];

#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/Slider.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/MessageBox.h>

using namespace vgui;
extern IFileSystem **pFilesystem;

#define CROSSHAIRS_FILE		"crosshairs.vdf"
#define CROSSHAIR_SIZES		5					// This needs to be matched in hud_crosshair.h

extern ConVar cl_timerwav;
extern ConVar cl_killbeepwav;

ConVar cl_bunnyhop_disablepogojump( "cl_jumpqueue", "0.0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Enables jump queue (have to let go and press jump in between concurrent jumps) if set to 1" );

ConVar hud_takesshots( "hud_takesshots", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Takes a screenshot at the end of each map if set to 1 (0 to disable)" );

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

#define FF_MAXDLIGHTS 512

// Wrapper CVAR for an archiveable r_maxdlights -- thanks, mirv
void FF_MaxDLights_Callback(ConVar *var, char const *pOldString)
{
	// have to manually do limits because of the callback
	if (var->GetInt() < 0)
		var->SetValue(0);
	if (var->GetInt() > FF_MAXDLIGHTS)
		var->SetValue(FF_MAXDLIGHTS);

	ConVar *c = cvar->FindVar("r_maxdlights");
	if (c)
		c->SetValue(var->GetString());
}
ConVar r_maxdlights_ff( "r_maxdlights_ff", "32", FCVAR_ARCHIVE, "Sets the maximum number of dynamic lights allowed.", FF_MaxDLights_Callback );

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

class CFFCrosshairOptions;
CFFCrosshairOptions *g_pCrosshairOptions = NULL;

//=============================================================================
// This just keeps a slider hooked up with a text element.
//=============================================================================
class CInputSlider : public Slider
{
	DECLARE_CLASS_SIMPLE(CInputSlider, Slider)

public:

	//-----------------------------------------------------------------------------
	// Purpose: Link this slider in with its input box
	//-----------------------------------------------------------------------------
	CInputSlider(Panel *parent, char const *panelName, char const *inputName) : BaseClass(parent, panelName)
	{
		m_pInputBox = new TextEntry(parent, inputName);
		m_pInputBox->SetAllowNumericInputOnly(true);
		//m_pInputBox->SetEditable(false);
		m_pInputBox->AddActionSignalTarget(this);
		//m_pInputBox->SendNewLine(true);

		AddActionSignalTarget(parent);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Transfer the value onto the input box
	//-----------------------------------------------------------------------------
	virtual void SetValue(int value, bool bTriggerChangeMessage = true)
	{
		m_pInputBox->SetText(VarArgs("%d", value));
		BaseClass::SetValue(value, bTriggerChangeMessage);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Keep the input box up to date
	//-----------------------------------------------------------------------------
	virtual void SetEnabled(bool state)
	{
		m_pInputBox->SetEnabled(state);
		BaseClass::SetEnabled(state);
	}


private:

	//-----------------------------------------------------------------------------
	// Purpose: Allow the input box to change this value
	//-----------------------------------------------------------------------------
	virtual void UpdateFromInput(int iValue, bool bTriggerChangeMessage = true)
	{
		BaseClass::SetValue(iValue, bTriggerChangeMessage);
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	int GetInputValue()
	{
		if (m_pInputBox->GetTextLength() == 0)
			return -1;

		char szValue[5];
		m_pInputBox->GetText(szValue, 4);

		if (!szValue[0])
			return -1;

		int iValue = atoi(szValue);

		// Since text is disabled on this box (and hopefully Valve haven't messed that
		// up, the next checks aren't need

		// atoi returned zero so make sure that the box is 1 character long and that
		// character is 0, otherwise it could just be some text.
		//if (iValue == 0 && (szValue[0] != '0' || m_pInputBox->GetTextLength() != 1))
		//	return -1;

		// Make sure that this number is as long as the string
		//

		return iValue;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the text box being changed and update the slider
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnTextChanged, "TextChanged", data)
	{
		// Apparently this is a good check
		if (!m_pInputBox->HasFocus())
			return;

		int iValue = GetInputValue();

		int iMin, iMax;
		GetRange(iMin, iMax);

		iValue = clamp(iValue, iMin, iMax);

		UpdateFromInput(iValue, false);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Don't let the box be left on invalid values
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnKillFocus, "TextKillFocus", data)
	{
		int iValue = GetInputValue();

		int iMin, iMax;
		GetRange(iMin, iMax);

		iValue = clamp(iValue, iMin, iMax);

		m_pInputBox->SetText(VarArgs("%d", iValue));
	}

	TextEntry *m_pInputBox;
};

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
	CFFCrosshairOptions(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
	{
		g_pCrosshairOptions = this;

		memset(&m_sCrosshairInfo, 0, sizeof(m_sCrosshairInfo));

		const char *pszInner = "1234567890-=!@#$%^?*()_+";
		int nInner = strlen(pszInner);

		m_pInnerCharacter = new ComboBox(this, "InnerCharacter", nInner, false);
		m_pInnerCharacter->AddActionSignalTarget(this);
		m_pInnerCharacter->SetEditable(false);

		const char *pszOuter = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		int nOuter = strlen(pszOuter);

		m_pOuterCharacter = new ComboBox(this, "OuterCharacter", nOuter, false);
		m_pOuterCharacter->AddActionSignalTarget(this);
		m_pOuterCharacter->SetEditable(false);

		// Populate the inner crosshair shape list
		for (int i = 0; i < nInner; i++)
		{
			KeyValues *kv = new KeyValues("IC");
			kv->SetString("character", VarArgs("%c", pszInner[i]));
			m_pInnerCharacter->AddItem(VarArgs("Shape %d", i + 1), kv);
			kv->deleteThis();
		}

		// Populate the outer crosshair shape list
		for (int i = 0; i < nOuter; i++)
		{
			KeyValues *kv = new KeyValues("OC");
			kv->SetString("character", VarArgs("%c", pszOuter[i]));
			m_pOuterCharacter->AddItem(VarArgs("Shape %d", i + 1), kv);
			kv->deleteThis();
		}

		m_pInnerScale = new CInputSlider(this, "InnerScale", "InnerScaleInput");
		m_pInnerScale->SetRange(1, CROSSHAIR_SIZES);
		m_pInnerScale->SetValue(1 + CROSSHAIR_SIZES / 2);

		m_pOuterScale = new CInputSlider(this, "OuterScale", "OuterScaleInput");
		m_pOuterScale->SetRange(1, CROSSHAIR_SIZES);
		m_pOuterScale->SetValue(1 + CROSSHAIR_SIZES / 2);

		m_pInnerRed = new CInputSlider(this, "InnerRed", "InnerRedInput");
		m_pInnerRed->SetRange(0, 255);
		m_pInnerRed->SetValue(255);

		m_pOuterRed = new CInputSlider(this, "OuterRed", "OuterRedInput");
		m_pOuterRed->SetRange(0, 255);
		m_pOuterRed->SetValue(255);

		m_pInnerGreen = new CInputSlider(this, "InnerGreen", "InnerGreenInput");
		m_pInnerGreen->SetRange(0, 255);
		m_pInnerGreen->SetValue(255);

		m_pOuterGreen = new CInputSlider(this, "OuterGreen", "OuterGreenInput");
		m_pOuterGreen->SetRange(0, 255);
		m_pOuterGreen->SetValue(255);

		m_pInnerBlue = new CInputSlider(this, "InnerBlue", "InnerBlueInput");
		m_pInnerBlue->SetRange(0, 255);
		m_pInnerBlue->SetValue(255);

		m_pOuterBlue = new CInputSlider(this, "OuterBlue", "OuterBlueInput");
		m_pOuterBlue->SetRange(0, 255);
		m_pOuterBlue->SetValue(255);

		m_pInnerAlpha = new CInputSlider(this, "InnerAlpha", "InnerAlphaInput");
		m_pInnerAlpha->SetRange(0, 255);
		m_pInnerAlpha->SetValue(255);

		m_pOuterAlpha = new CInputSlider(this, "OuterAlpha", "OuterAlphaInput");
		m_pOuterAlpha->SetRange(0, 255);
		m_pOuterAlpha->SetValue(255);

		m_pInnerUseGlobal = new CheckButton(this, "InnerUseGlobal", "");
		m_pOuterUseGlobal = new CheckButton(this, "OuterUseGlobal", "");

		m_pForceGlobal = new CheckButton(this, "UseGlobal", "");

		// Display of crosshair, Z-ordered so that inner is over outer
		m_pInnerCrosshair = new Label(this, "innerDisplay", "1");
		m_pInnerCrosshair->SetZPos(2);
		m_pOuterCrosshair = new Label(this, "outerDisplay", "a");
		m_pOuterCrosshair->SetZPos(1);

		// Background showing some ingame screenshot for crosshair comparison
		m_pCrosshairBackground = new ImagePanel(this, "CrosshairBackground");
		m_pCrosshairBackground->SetImage("crosshairbg");
		m_pCrosshairBackground->SetZPos(-1);

		// Weapons list
		m_pWeapon = new ComboBox(this, "Weapon", (int) FF_WEAPON_TOMMYGUN + 2, false);
		m_pWeapon->AddActionSignalTarget(this);
		m_pWeapon->SetEditable(false);

		// Add the global one first
		KeyValues *kv = new KeyValues("W");
		kv->SetString("wpid", "global");
		m_pWeapon->AddItem("Global", kv);
		kv->deleteThis();

		for (int i = 1; i <= FF_WEAPON_TOMMYGUN; i++)
		{
			KeyValues *kv = new KeyValues("W");
			kv->SetString("wpid", s_WeaponAliasInfo[i]);
			m_pWeapon->AddItem(VarArgs("#%s", s_WeaponAliasInfo[i]), kv);
			kv->deleteThis();
		}

		// Add the hit one last
		KeyValues *kv2 = new KeyValues("W");
		kv2->SetString("wpid", "hit");
		m_pWeapon->AddItem("Hit", kv2);
		kv2->deleteThis();

		LoadControlSettings("resource/ui/FFOptionsSubCrosshairs.res");

		// Now load settings
		Load();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Allow all inner options to be enabled or disabled quickly
	//-----------------------------------------------------------------------------
	virtual void AllowInnerChanges(bool state)
	{
		m_pInnerCharacter->SetEnabled(state);
		m_pInnerScale->SetEnabled(state);
		m_pInnerRed->SetEnabled(state);
		m_pInnerGreen->SetEnabled(state);
		m_pInnerBlue->SetEnabled(state);
		m_pInnerAlpha->SetEnabled(state);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Allow all outer options to be enabled or disabled quickly
	//-----------------------------------------------------------------------------
	virtual void AllowOuterChanges(bool state)
	{
		m_pOuterCharacter->SetEnabled(state);
		m_pOuterScale->SetEnabled(state);
		m_pOuterRed->SetEnabled(state);
		m_pOuterGreen->SetEnabled(state);
		m_pOuterBlue->SetEnabled(state);
		m_pOuterAlpha->SetEnabled(state);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Allow weapon selection stuff to be disabled or enabled quickly
	//-----------------------------------------------------------------------------
	virtual void AllowWeaponSelection(bool state)
	{
		m_pInnerUseGlobal->SetEnabled(state);
		m_pOuterUseGlobal->SetEnabled(state);

		if (!state && m_pWeapon->GetActiveItem() != 0)
		{
			m_pWeapon->ActivateItemByRow(0);
		}
		
		m_pWeapon->SetEnabled(state);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Load all the fonts we need
	//-----------------------------------------------------------------------------
	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		vgui::HScheme CrossHairScheme = vgui::scheme()->LoadSchemeFromFile("resource/CrosshairScheme.res", "CrosshairScheme");

		for (int i = 0; i < CROSSHAIR_SIZES; i++)
		{
			m_hPrimaryCrosshairs[i] = vgui::scheme()->GetIScheme(CrossHairScheme)->GetFont(VarArgs("PrimaryCrosshairs%d", (i + 1)));
			m_hSecondaryCrosshairs[i] = vgui::scheme()->GetIScheme(CrossHairScheme)->GetFont(VarArgs("SecondaryCrosshairs%d", (i + 1)));
		}

		BaseClass::ApplySchemeSettings(pScheme);
	}

	//-----------------------------------------------------------------------------
	// Purpose: This builds keyvalues for each weapon and puts them into an
	//			all-conquering keyvalue that is then saved to file.
	//-----------------------------------------------------------------------------
	void Apply()
	{
		KeyValues *kv = new KeyValues("Crosshairs");
		kv->SetInt("globalCrosshairs", m_bForceGlobalCrosshair);

		for (int i = 0; i <= FF_WEAPON_TOMMYGUN; i++)
		{
			const WeaponCrosshair_t &cinfo = m_sCrosshairInfo[i];

			KeyValues *k = new KeyValues(s_WeaponAliasInfo[i]);
			k->SetString("innerChar", VarArgs("%c", cinfo.innerChar));
			k->SetInt("innerScale", cinfo.innerScale);
			k->SetInt("innerR", cinfo.innerR);
			k->SetInt("innerG", cinfo.innerG);
			k->SetInt("innerB", cinfo.innerB);
			k->SetInt("innerA", cinfo.innerA);
			k->SetInt("innerUseGlobal", cinfo.innerUseGlobal);

			k->SetString("outerChar", VarArgs("%c", cinfo.outerChar));
			k->SetInt("outerScale", cinfo.outerScale);
			k->SetInt("outerR", cinfo.outerR);
			k->SetInt("outerG", cinfo.outerG);
			k->SetInt("outerB", cinfo.outerB);
			k->SetInt("outerA", cinfo.outerA);
			k->SetInt("outerUseGlobal", cinfo.outerUseGlobal);

			kv->AddSubKey(k);
		}
		
		const WeaponCrosshair_t &cinfo = m_sCrosshairInfo[FF_WEAPON_TOMMYGUN + 1];

		KeyValues *k = new KeyValues("hit");
		k->SetString("innerChar", VarArgs("%c", cinfo.innerChar));
		k->SetInt("innerScale", cinfo.innerScale);
		k->SetInt("innerR", cinfo.innerR);
		k->SetInt("innerG", cinfo.innerG);
		k->SetInt("innerB", cinfo.innerB);
		k->SetInt("innerA", cinfo.innerA);
		k->SetInt("innerUseGlobal", cinfo.innerUseGlobal);

		k->SetString("outerChar", VarArgs("%c", cinfo.outerChar));
		k->SetInt("outerScale", cinfo.outerScale);
		k->SetInt("outerR", cinfo.outerR);
		k->SetInt("outerG", cinfo.outerG);
		k->SetInt("outerB", cinfo.outerB);
		k->SetInt("outerA", cinfo.outerA);
		k->SetInt("outerUseGlobal", cinfo.outerUseGlobal);

		kv->AddSubKey(k);

		kv->SaveToFile(*pFilesystem, CROSSHAIRS_FILE);
		kv->deleteThis();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Load all the settings from file into memory via keyvalues
	//-----------------------------------------------------------------------------
	void Load()
	{
		KeyValues *kv = new KeyValues("Crosshairs");
		kv->LoadFromFile(*pFilesystem, CROSSHAIRS_FILE);
		m_bForceGlobalCrosshair = kv->GetInt("globalCrosshairs", 0);

		// Loop through keyvalues looking for each weapon
		for (int i = 0; i <= FF_WEAPON_TOMMYGUN; i++)
		{
			WeaponCrosshair_t &cinfo = m_sCrosshairInfo[i];

			KeyValues *k = kv->FindKey(s_WeaponAliasInfo[i]);

			if (k)
			{
				cinfo.innerChar = k->GetString("innerChar", "1")[0];
				cinfo.innerScale = k->GetInt("innerScale", 255);
				cinfo.innerR = k->GetInt("innerR", 255);
				cinfo.innerG = k->GetInt("innerG", 255);
				cinfo.innerB = k->GetInt("innerB", 255);
				cinfo.innerA = k->GetInt("innerA", 255);
				cinfo.innerUseGlobal = k->GetInt("innerUseGlobal", 1);

				cinfo.outerChar = k->GetString("outerChar", "1")[0];
				cinfo.outerScale = k->GetInt("outerScale", 255);
				cinfo.outerR = k->GetInt("outerR", 255);
				cinfo.outerG = k->GetInt("outerG", 255);
				cinfo.outerB = k->GetInt("outerB", 255);
				cinfo.outerA = k->GetInt("outerA", 255);
				cinfo.outerUseGlobal = k->GetInt("outerUseGlobal", 1);
			}
			else
			{
				// Couldn't load at all, give some defaults
				cinfo.innerChar = '1';
				cinfo.outerChar = 'a';

				cinfo.innerScale = cinfo.outerScale = 255;
				cinfo.innerR = cinfo.outerR = 255;
				cinfo.innerG = cinfo.outerG = 255;
				cinfo.innerB = cinfo.outerB = 255;
				cinfo.innerA = cinfo.outerA= 255;

				// Derive all of them from global
				if (i != 0)
				{
					cinfo.innerUseGlobal = true;
					cinfo.outerUseGlobal = true;
				}
			}
		}

		// hit crosshair
		WeaponCrosshair_t &cinfo = m_sCrosshairInfo[FF_WEAPON_TOMMYGUN + 1];

		KeyValues *k = kv->FindKey("hit");

		if (k)
		{
			cinfo.innerChar = k->GetString("innerChar", "1")[0];
			cinfo.innerScale = k->GetInt("innerScale", 255);
			cinfo.innerR = k->GetInt("innerR", 255);
			cinfo.innerG = k->GetInt("innerG", 255);
			cinfo.innerB = k->GetInt("innerB", 255);
			cinfo.innerA = k->GetInt("innerA", 255);
			cinfo.innerUseGlobal = k->GetInt("innerUseGlobal", 1);

			cinfo.outerChar = k->GetString("outerChar", "1")[0];
			cinfo.outerScale = k->GetInt("outerScale", 255);
			cinfo.outerR = k->GetInt("outerR", 255);
			cinfo.outerG = k->GetInt("outerG", 255);
			cinfo.outerB = k->GetInt("outerB", 255);
			cinfo.outerA = k->GetInt("outerA", 255);
			cinfo.outerUseGlobal = k->GetInt("outerUseGlobal", 1);
		}
		else
		{
			// Couldn't load at all, give some defaults
			cinfo.innerChar = '!';
			cinfo.outerChar = 'U';

			cinfo.innerScale = 3;
			cinfo.outerScale = 3;
			cinfo.innerR = 255;
			cinfo.outerR = 255;
			cinfo.innerG = 0;
			cinfo.outerG = 0;
			cinfo.innerB = 0;
			cinfo.outerB = 0;
			cinfo.innerA = 255;
			cinfo.outerA= 255;

			cinfo.innerUseGlobal = cinfo.outerUseGlobal = false;
		}

		kv->deleteThis();

		// Default to the global weapon
		m_pWeapon->ActivateItemByRow(0);

		// Check the global crosshairs if needed
		m_pForceGlobal->SetSelected(m_bForceGlobalCrosshair);

		// Now update the sliders
		UpdateSliders();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Reload the stuff from file for now
	//-----------------------------------------------------------------------------
	void Reset()
	{
		Load();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Get crosshair for a weapon
	//-----------------------------------------------------------------------------
	void GetCrosshair(FFWeaponID iWeapon, char &innerChar, Color &innerCol, int &innerSize, char &outerChar, Color &outerCol, int &outerSize)
	{
		Assert(iWeapon >= 0 && iWeapon <= FF_WEAPON_TOMMYGUN);

		WeaponCrosshair_t &cinfo = m_sCrosshairInfo[iWeapon];
		WeaponCrosshair_t *pCrosshair = &cinfo;

		if (cinfo.innerUseGlobal || m_bForceGlobalCrosshair)
		{
			pCrosshair = &m_sCrosshairInfo[0];
		}

		innerChar = pCrosshair->innerChar;
		innerCol = Color(pCrosshair->innerR, pCrosshair->innerG, pCrosshair->innerB, pCrosshair->innerA);
		innerSize = pCrosshair->innerScale;

		if (!cinfo.outerUseGlobal && !m_bForceGlobalCrosshair)
		{
			pCrosshair = &cinfo;
		}

		outerChar = pCrosshair->outerChar;
		outerCol = Color(pCrosshair->outerR, pCrosshair->outerG, pCrosshair->outerB, pCrosshair->outerA);
		outerSize = pCrosshair->outerScale;
	}
	
	//-----------------------------------------------------------------------------
	// Purpose: Get crosshair for a weapon
	//-----------------------------------------------------------------------------
	void GetHitCrosshair(char &innerChar, Color &innerCol, int &innerSize, char &outerChar, Color &outerCol, int &outerSize)
	{
		WeaponCrosshair_t &cinfo = m_sCrosshairInfo[FF_WEAPON_TOMMYGUN + 1];
		WeaponCrosshair_t *pCrosshair = &cinfo;

		if (cinfo.innerUseGlobal || m_bForceGlobalCrosshair)
		{
			pCrosshair = &m_sCrosshairInfo[0];
		}

		innerChar = pCrosshair->innerChar;
		innerCol = Color(pCrosshair->innerR, pCrosshair->innerG, pCrosshair->innerB, pCrosshair->innerA);
		innerSize = pCrosshair->innerScale;

		if (!cinfo.outerUseGlobal && !m_bForceGlobalCrosshair)
		{
			pCrosshair = &cinfo;
		}

		outerChar = pCrosshair->outerChar;
		outerCol = Color(pCrosshair->outerR, pCrosshair->outerG, pCrosshair->outerB, pCrosshair->outerA);
		outerSize = pCrosshair->outerScale;
	}

private:

	//-----------------------------------------------------------------------------
	// Purpose: Catch the slider moving
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnUpdateSliders, "SliderMoved", data)
	{
		UpdateCrosshairs();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the combo box changing
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnUpdateCombos, "TextChanged", data)
	{
		// Make sure the sliders are set correct
		if (data->GetPtr("panel") == m_pWeapon)
		{
			UpdateSliders();
		}

		UpdateCrosshairs();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch checkbox updating
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnUpdateCheckbox, "CheckButtonChecked", data)
	{
		UpdateCrosshairs();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Update the crosshair
	//			This isn't a particularly great function, it should really be
	//			split up into separate responsibilities (update crosshair data,
	//			refresh crosshair preview).
	//-----------------------------------------------------------------------------
	void UpdateCrosshairs()
	{
		int iCurrentWeapon = m_pWeapon->GetActiveItem();
		Assert(iCurrentWeapon >= 0 && iCurrentWeapon <= FF_WEAPON_TOMMYGUN);

		WeaponCrosshair_t &cinfo = m_sCrosshairInfo[iCurrentWeapon];
		cinfo.innerChar = m_pInnerCharacter->GetActiveItemUserData()->GetString("character")[0];
		cinfo.innerScale = m_pInnerScale->GetValue();
		cinfo.innerR = m_pInnerRed->GetValue();
		cinfo.innerG = m_pInnerGreen->GetValue();
		cinfo.innerB = m_pInnerBlue->GetValue();
		cinfo.innerA = m_pInnerAlpha->GetValue();

		cinfo.outerChar = m_pOuterCharacter->GetActiveItemUserData()->GetString("character")[0];
		cinfo.outerScale = m_pOuterScale->GetValue();
		cinfo.outerR = m_pOuterRed->GetValue();
		cinfo.outerG = m_pOuterGreen->GetValue();
		cinfo.outerB = m_pOuterBlue->GetValue();
		cinfo.outerA = m_pOuterAlpha->GetValue();

		// Don't allow the individual use globals to be selected if we're editing global
		cinfo.innerUseGlobal = (iCurrentWeapon == 0 || iCurrentWeapon == FF_WEAPON_TOMMYGUN + 1) ? 0 : m_pInnerUseGlobal->IsSelected();
		cinfo.outerUseGlobal = (iCurrentWeapon == 0 || iCurrentWeapon == FF_WEAPON_TOMMYGUN + 1) ? 0 : m_pOuterUseGlobal->IsSelected();

		m_bForceGlobalCrosshair = m_pForceGlobal->IsSelected();

		if (m_bForceGlobalCrosshair)
		{
			AllowInnerChanges(true);
			AllowOuterChanges(true);
		}
		else
		{
			AllowInnerChanges(!cinfo.innerUseGlobal);
			AllowOuterChanges(!cinfo.outerUseGlobal);
		}
		
		AllowWeaponSelection(!m_bForceGlobalCrosshair);

		// Don't allow them to select "use global" for an inner/outer if they are
		// editing the global one (logical)
		if (iCurrentWeapon == 0 || iCurrentWeapon == FF_WEAPON_TOMMYGUN + 1)
		{
			m_pInnerUseGlobal->SetEnabled(false);
			m_pOuterUseGlobal->SetEnabled(false);
		}

		WeaponCrosshair_t *pDrawCrosshair = &cinfo;

		// If they have inner globals selected, point to the globals for this
		if (cinfo.innerUseGlobal || m_bForceGlobalCrosshair)
		{
			pDrawCrosshair = &m_sCrosshairInfo[0];
		}

		m_pInnerCrosshair->SetFont(m_hPrimaryCrosshairs[clamp(pDrawCrosshair->innerScale, 1, CROSSHAIR_SIZES) - 1]);

		m_pInnerCrosshair->SetFgColor(Color(pDrawCrosshair->innerR, pDrawCrosshair->innerG, pDrawCrosshair->innerB, pDrawCrosshair->innerA));
		m_pInnerCrosshair->SetText(VarArgs("%c", pDrawCrosshair->innerChar));

		// Now point to the other crosshair (if we're not using global)
		if (!cinfo.outerUseGlobal && !m_bForceGlobalCrosshair)
		{
			pDrawCrosshair = &cinfo;
		}

		m_pOuterCrosshair->SetFont(m_hSecondaryCrosshairs[clamp(pDrawCrosshair->outerScale, 1, CROSSHAIR_SIZES) - 1]);

		m_pOuterCrosshair->SetFgColor(Color(pDrawCrosshair->outerR, pDrawCrosshair->outerG, pDrawCrosshair->outerB, pDrawCrosshair->outerA));
		m_pOuterCrosshair->SetText(VarArgs("%c", pDrawCrosshair->outerChar));
	}

	//-----------------------------------------------------------------------------
	// Purpose: Update the sliders if a combo box has been changed
	//-----------------------------------------------------------------------------
	void UpdateSliders()
	{
		int iCurrentWeapon = m_pWeapon->GetActiveItem();
		Assert(iCurrentWeapon >= 0 && iCurrentWeapon <= FF_WEAPON_TOMMYGUN);

		const WeaponCrosshair_t &cinfo = m_sCrosshairInfo[iCurrentWeapon];

		m_pInnerScale->SetValue(cinfo.innerScale, false);
		m_pInnerRed->SetValue(cinfo.innerR, false);
		m_pInnerGreen->SetValue(cinfo.innerG, false);
		m_pInnerBlue->SetValue(cinfo.innerB, false);
		m_pInnerAlpha->SetValue(cinfo.innerA, false);
		m_pInnerUseGlobal->SetSelected(cinfo.innerUseGlobal);

		m_pOuterScale->SetValue(cinfo.outerScale, false);
		m_pOuterRed->SetValue(cinfo.outerR, false);
		m_pOuterGreen->SetValue(cinfo.outerG, false);
		m_pOuterBlue->SetValue(cinfo.outerB, false);
		m_pOuterAlpha->SetValue(cinfo.outerA, false);
		m_pOuterUseGlobal->SetSelected(cinfo.outerUseGlobal);

		// Find the correct inner shape
		for (int i = 0; i < m_pInnerCharacter->GetItemCount(); i++)
		{
			KeyValues *kv = m_pInnerCharacter->GetItemUserData(i);
			if (kv->GetString("character")[0] == cinfo.innerChar)
			{
				m_pInnerCharacter->ActivateItemByRow(i);
			}
		}
		
		// Find the correct outer shape
		for (int i = 0; i < m_pOuterCharacter->GetItemCount(); i++)
		{
			KeyValues *kv = m_pOuterCharacter->GetItemUserData(i);
			if (kv->GetString("character")[0] == cinfo.outerChar)
			{
				m_pOuterCharacter->ActivateItemByRow(i);
			}
		}
	}

private:

    CInputSlider	*m_pInnerScale, *m_pOuterScale;
	CInputSlider	*m_pInnerRed, *m_pOuterRed;
	CInputSlider	*m_pInnerGreen, *m_pOuterGreen;
	CInputSlider	*m_pInnerBlue, *m_pOuterBlue;
	CInputSlider	*m_pInnerAlpha, *m_pOuterAlpha;

	ComboBox		*m_pInnerCharacter, *m_pOuterCharacter;
	ComboBox		*m_pWeapon;

	Label			*m_pInnerCrosshair, *m_pOuterCrosshair;

	CheckButton		*m_pInnerUseGlobal, *m_pOuterUseGlobal;
	CheckButton		*m_pForceGlobal;

	ImagePanel		*m_pCrosshairBackground;

	HFont			m_hPrimaryCrosshairs[CROSSHAIR_SIZES];
	HFont			m_hSecondaryCrosshairs[CROSSHAIR_SIZES];

private:

	typedef struct
	{
		int		innerScale, outerScale;
		int		innerR, innerG, innerB, innerA;
		int		outerR, outerG, outerB, outerA;
		char	innerChar, outerChar;
		bool	innerUseGlobal, outerUseGlobal;
	} WeaponCrosshair_t;

	WeaponCrosshair_t	m_sCrosshairInfo[FF_WEAPON_TOMMYGUN + 2];

	bool	m_bForceGlobalCrosshair;
};

//-----------------------------------------------------------------------------
// Purpose: A function to allow the current crosshair stuff to be retrieved
//-----------------------------------------------------------------------------
void GetCrosshair(FFWeaponID iWeapon, char &innerChar, Color &innerCol, int &innerSize, char &outerChar, Color &outerCol, int &outerSize)
{
	Assert(g_pCrosshairOptions);

	g_pCrosshairOptions->GetCrosshair(iWeapon, innerChar, innerCol, innerSize, outerChar, outerCol, outerSize);
}

//-----------------------------------------------------------------------------
// Purpose: A function to allow the current hit crosshair stuff to be retrieved
//-----------------------------------------------------------------------------
void GetHitCrosshair(char &innerChar, Color &innerCol, int &innerSize, char &outerChar, Color &outerCol, int &outerSize)
{
	Assert(g_pCrosshairOptions);

	g_pCrosshairOptions->GetHitCrosshair(innerChar, innerCol, innerSize, outerChar, outerCol, outerSize);
}

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
	CFFDLightOptions(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
	{
		m_pFFDLightMax = new CInputSlider(this, "FFDLightMax", "FFDLightMaxInput");
		m_pFFDLightMax->SetRange(0, FF_MAXDLIGHTS);
		m_pFFDLightMax->SetValue(32);

		m_pFFDLightExplosion = new CInputSlider(this, "FFDLightExplosion", "FFDLightExplosionInput");
		m_pFFDLightExplosion->SetRange(0, 200);
		m_pFFDLightExplosion->SetValue(1);

		m_pFFDLightMuzzle = new CInputSlider(this, "FFDLightMuzzle", "FFDLightMuzzleInput");
		m_pFFDLightMuzzle->SetRange(0, 200);
		m_pFFDLightMuzzle->SetValue(1);

		m_pFFDLightFlamethrower = new CInputSlider(this, "FFDLightFlamethrower", "FFDLightFlamethrowerInput");
		m_pFFDLightFlamethrower->SetRange(0, 200);
		m_pFFDLightFlamethrower->SetValue(1);

		m_pFFDLightIgnited = new CInputSlider(this, "FFDLightIgnited", "FFDLightIgnitedInput");
		m_pFFDLightIgnited->SetRange(0, 200);
		m_pFFDLightIgnited->SetValue(1);

		m_pFFDLightNapalm = new CInputSlider(this, "FFDLightNapalm", "FFDLightNapalmInput");
		m_pFFDLightNapalm->SetRange(0, 200);
		m_pFFDLightNapalm->SetValue(1);

		m_pFFDLightIC = new CInputSlider(this, "FFDLightIC", "FFDLightICInput");
		m_pFFDLightIC->SetRange(0, 200);
		m_pFFDLightIC->SetValue(1);

		m_pFFDLightRocket = new CInputSlider(this, "FFDLightRocket", "FFDLightRocketInput");
		m_pFFDLightRocket->SetRange(0, 200);
		m_pFFDLightRocket->SetValue(1);

		m_pFFDLightRail = new CInputSlider(this, "FFDLightRail", "FFDLightRailInput");
		m_pFFDLightRail->SetRange(0, 200);
		m_pFFDLightRail->SetValue(1);

		m_pFFDLightConc = new CInputSlider(this, "FFDLightConc", "FFDLightConcInput");
		m_pFFDLightConc->SetRange(0, 200);
		m_pFFDLightConc->SetValue(1);

		m_pFFDLightFlashlight = new CInputSlider(this, "FFDLightFlashlight", "FFDLightFlashlightInput");
		m_pFFDLightFlashlight->SetRange(0, 200);
		m_pFFDLightFlashlight->SetValue(1);

		m_pFFDLightGeneric = new CInputSlider(this, "FFDLightGeneric", "FFDLightGenericInput");
		m_pFFDLightGeneric->SetRange(0, 200);
		m_pFFDLightGeneric->SetValue(1);

		LoadControlSettings("resource/ui/FFOptionsSubDLights.res");

		// Now load settings
		Load();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Load all the fonts we need
	//-----------------------------------------------------------------------------
	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);
	}

	//-----------------------------------------------------------------------------
	// Purpose: This builds keyvalues for each weapon and puts them into an
	//			all-conquering keyvalue that is then saved to file.
	//-----------------------------------------------------------------------------
	void Apply()
	{
		UpdateConVars();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Load all the settings from file into memory via keyvalues
	//-----------------------------------------------------------------------------
	void Load()
	{
		// Now update the sliders
		UpdateSliders();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Reload the stuff from file for now
	//-----------------------------------------------------------------------------
	void Reset()
	{
		Load();
	}

private:

	//-----------------------------------------------------------------------------
	// Purpose: Catch the slider moving
	//-----------------------------------------------------------------------------
	//MESSAGE_FUNC_PARAMS(OnUpdateSliders, "SliderMoved", data)
	//{
	//	UpdateConVars();
	//}

	//-----------------------------------------------------------------------------
	// Purpose: Catch checkbox updating
	//-----------------------------------------------------------------------------
	//MESSAGE_FUNC_PARAMS(OnUpdateCheckbox, "CheckButtonChecked", data)
	//{
	//	UpdateConVars();
	//}

	//-----------------------------------------------------------------------------
	// Purpose: Update the ConVars
	//-----------------------------------------------------------------------------
	void UpdateConVars()
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
	void UpdateSliders()
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

private:

	CInputSlider	*m_pFFDLightMax;
	CInputSlider	*m_pFFDLightExplosion;
	CInputSlider	*m_pFFDLightMuzzle;
	CInputSlider	*m_pFFDLightFlamethrower;
	CInputSlider	*m_pFFDLightIgnited;
	CInputSlider	*m_pFFDLightNapalm;
	CInputSlider	*m_pFFDLightIC;
	CInputSlider	*m_pFFDLightRocket;
	CInputSlider	*m_pFFDLightRail;
	CInputSlider	*m_pFFDLightConc;
	CInputSlider	*m_pFFDLightFlashlight;
	CInputSlider	*m_pFFDLightGeneric;

};

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
	CFFTimerOptions(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
	{
		m_pTimers = new ComboBox(this, "TimerList", 0, false);
		m_pPlayButton = new Button(this, "PlayButton", "", this, "Play");

		m_pBeeps = new ComboBox(this, "BeepList", 0, false);
		m_pPlayButton2 = new Button(this, "PlayButton2", "", this, "Play2");

		LoadControlSettings("resource/ui/FFOptionsSubTimer.res");
	}

	//-----------------------------------------------------------------------------
	// Purpose: This is a bit messy. We need to save the filename without the
	//			extension to the cvar.
	//-----------------------------------------------------------------------------
	void Apply()
	{
		const char *pszTimer = m_pTimers->GetActiveItemUserData()->GetString("file");

		if (!pszTimer)
			return;

		int nLen = strlen(pszTimer);

		if (nLen < 5)
			return;

		char buf[128];
		Q_snprintf(buf, 127, "%s", pszTimer);
		buf[nLen - 4] = 0;

		cl_timerwav.SetValue(buf);

		// same for death beep

		const char *pszBeep = m_pBeeps->GetActiveItemUserData()->GetString("file");

		if (!pszBeep)
			return;

		int nLen2 = strlen(pszBeep);

		if (nLen2 < 5)
			return;

		char buf2[128];
		Q_snprintf(buf2, 127, "%s", pszBeep);
		buf2[nLen2 - 4] = 0;

		cl_killbeepwav.SetValue(buf2);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Just load again to reset
	//-----------------------------------------------------------------------------
	void Reset()
	{
		Load();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Load all the timers into the combobox
	//-----------------------------------------------------------------------------
	void Load()
	{
		int iCurrent = 0;
		m_pTimers->DeleteAllItems();

		FileFindHandle_t findHandle;
		const char *pFilename = (*pFilesystem)->FindFirstEx("sound/timers/*.wav", "MOD", &findHandle);
		
		while (pFilename != NULL) 
		{
			KeyValues *kv = new KeyValues("timers");
			kv->SetString("file", pFilename);
			int iNew= m_pTimers->AddItem(pFilename, kv);
			kv->deleteThis();

			int nLength = strlen(cl_timerwav.GetString());

			// This is our timer file
			if (Q_strncmp(pFilename, cl_timerwav.GetString(), nLength) == 0)
			{
				iCurrent = iNew;
			}

			pFilename = (*pFilesystem)->FindNext(findHandle);
		}

		(*pFilesystem)->FindClose(findHandle);

		m_pTimers->ActivateItemByRow(iCurrent);

		// same for death beeps
		int iCurrent2 = 0;
		m_pBeeps->DeleteAllItems();

		FileFindHandle_t findHandle2;
		const char *pFilename2 = (*pFilesystem)->FindFirstEx("sound/player/deathbeep/*.wav", "MOD", &findHandle2);
		
		while (pFilename2 != NULL) 
		{
			KeyValues *kv = new KeyValues("Beeps");
			kv->SetString("file", pFilename2);
			int iNew= m_pBeeps->AddItem(pFilename2, kv);
			kv->deleteThis();

			int nLength = strlen(cl_killbeepwav.GetString());

			// This is our timer file
			if (Q_strncmp(pFilename2, cl_killbeepwav.GetString(), nLength) == 0)
			{
				iCurrent2 = iNew;
			}

			pFilename2 = (*pFilesystem)->FindNext(findHandle2);
		}

		(*pFilesystem)->FindClose(findHandle2);

		m_pBeeps->ActivateItemByRow(iCurrent2);
	}

private:

	//-----------------------------------------------------------------------------
	// Purpose: Catch the play button being pressed and play the currently
	//			selected timer
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data)
	{
		const char *pszCommand = data->GetString("command");

		if (Q_strcmp(pszCommand, "Play") == 0)
		{
			const char *pszTimer = m_pTimers->GetActiveItemUserData()->GetString("file");

			if (pszTimer)
			{
				engine->ClientCmd(VarArgs("play timers/%s\n", pszTimer));
			}
		}
		else if (Q_strcmp(pszCommand, "Play2") == 0)
		{
			const char *pszBeep = m_pBeeps->GetActiveItemUserData()->GetString("file");

			if (pszBeep)
			{
				engine->ClientCmd(VarArgs("play player/deathbeep/%s\n", pszBeep));
			}
		}
	}

	ComboBox	*m_pTimers;
	Button		*m_pPlayButton;

	ComboBox	*m_pBeeps;
	Button		*m_pPlayButton2;
};

int GetComboBoxOption(ComboBox *cb, const char *value, const char *keyname = "value")
{
	int n = cb->GetItemCount();
	int l = strlen(value);
	for (int i = 0; i < n; i++)
	{
		KeyValues *kvItem = cb->GetItemUserData(i);
		const char *pszItemValue = kvItem->GetString(keyname);
		if (kvItem && Q_strncmp(pszItemValue, value, l) == 0) {
			return i;
		}
	}
	return -1;
}

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
	CFFMiscOptions(Panel *parent, char const *panelName, const char *pszSourceFile) : BaseClass(parent, panelName)
	{
		LoadControlSettings("resource/ui/FFOptionsSubMisc.res");

		Q_strncpy(m_szSourceFile, pszSourceFile, 127);

		int iYCoords = TITLE_SPACER;

		// Put all our options stuff in a keyfile now
		KeyValues *kvOptions = new KeyValues("Options");
		kvOptions->LoadFromFile(*pFilesystem, m_szSourceFile);

		// Loop through creating new options for each one
		for (KeyValues *kvOption = kvOptions->GetFirstSubKey(); kvOption != NULL; kvOption = kvOption->GetNextKey())
		{
			const char *pszType = kvOption->GetString("type", "boolean");

			const char *pszName = kvOption->GetName();
			const char *pszCaption = kvOption->GetString("caption");

			// A little separator
			if (Q_strncmp(pszName, "heading", 7) == 0)
			{
				Label *l = new Label(this, "label", pszCaption);

				if (l)
				{
					l->SetPos(25, iYCoords + TITLE_SPACER);
					l->SetSize(250, ROW_HEIGHT);
					iYCoords += ROW_HEIGHT + TITLE_SPACER;	// Add extra bit on
				}
			}

			// Boolean is just a simple checkbox
			else if (Q_strncmp(pszType, "boolean", 7) == 0)
			{
				CheckButton *cb = new CheckButton(this, pszName, pszCaption);

				if (!cb)
					continue;

				cb->SetPos(30, iYCoords);
				cb->SetSize(450, ROW_HEIGHT);

				iYCoords += ROW_HEIGHT;
			}
			// Discrete is a combobox with a label
			else if (Q_strncmp(pszType, "discrete", 8) == 0)
			{
				KeyValues *kvValues = kvOption->FindKey("values", false);
				int nValues = 0;

				if (!kvValues)
					continue;

				// First count all the values so we know how many lines are
				// needed for the combobox
				nValues = 0;
				KeyValues *kvValue = kvValues->GetFirstSubKey();
				while (kvValue)
				{
					nValues++;
					kvValue = kvValue->GetNextKey();
				}

				ComboBox *cb = new ComboBox(this, pszName, nValues, false);

				if (!cb)
					continue;

				kvValues = kvOption->FindKey("values", false);

				if (!kvValues)
					continue;

				// Now go through all the values and add them to the combobox
				kvValue = kvValues->GetFirstSubKey();
				while (kvValue)
				{
					const char *pszValue = kvValue->GetName();
					const char *pszCaption = kvValues->GetString(pszValue);
					kvValue = kvValue->GetNextKey();

					KeyValues *kvItem = new KeyValues("kvItem");
					kvItem->SetString("value", pszValue);
					cb->AddItem(pszCaption, kvItem);
					kvItem->deleteThis();
				}

				cb->SetPos(30, iYCoords);
				cb->SetSize(80, ROW_HEIGHT - 4);
				cb->ActivateItemByRow(0);

				// Create a handy label too so we know what this is
				Label *l = new Label(this, "label", pszCaption);

				if (l)
				{
					l->SetPos(120, iYCoords);
					l->SetSize(450, ROW_HEIGHT);
				}

				iYCoords += ROW_HEIGHT;
			}
		}

		/*m_pHints = new CheckButton( this, "HintCheck", "Enable Hints" );
		m_pHintsConVar = NULL;

		m_pARCheck = new CheckButton( this, "ARCheck", "Enable Auto-Reload" );
		m_pAutoRLConVar = NULL;

		m_pAutoKillCheck = new CheckButton( this, "AKCheck", "Change Class Instantly" );
		m_pAutoKillConVar = NULL;

		m_pBlurCheck = new CheckButton( this, "BlurCheck", "Enable Motion Blur" );
		m_pBlurConVar = NULL;

		LoadControlSettings("resource/ui/FFOptionsSubMisc.res");*/
	}

	//-----------------------------------------------------------------------------
	// Purpose: Apply the player's changes
	//			
	//-----------------------------------------------------------------------------
	void Apply()
	{
		/*if ( m_pHintsConVar )
			m_pHintsConVar->SetValue( m_pHints->IsSelected() );

		if ( m_pAutoRLConVar )
			m_pAutoRLConVar->SetValue( m_pARCheck->IsSelected() );

		if ( m_pAutoKillConVar )
			m_pAutoKillConVar->SetValue( m_pAutoKillCheck->IsSelected() );

		if ( m_pBlurConVar )
			m_pBlurConVar->SetValue( m_pBlurCheck->IsSelected() );*/

		KeyValues *kvOptions = new KeyValues("Options");
		kvOptions->LoadFromFile(*pFilesystem, m_szSourceFile);

		// Loop through creating new options for each one
		for (KeyValues *kvOption = kvOptions->GetFirstSubKey(); kvOption != NULL; kvOption = kvOption->GetNextKey())
		{
			const char *pszCvar = kvOption->GetString("cvar");
			const char *pszName = kvOption->GetName();

			Panel *pChild = FindChildByName(pszName);

			if (!pChild)
				continue;

			ConVar *pCvar = cvar->FindVar(pszCvar);

			if (!pCvar)
				continue;

			// This is a bad show old chap
			if (CheckButton *cb = dynamic_cast <CheckButton *> (pChild))
			{
				pCvar->SetValue(cb->IsSelected());
			}
			else if (ComboBox *cb = dynamic_cast <ComboBox *> (pChild))
			{
				// Only replace the cvar with this option if it is not a custom one
				const char *pszValue = cb->GetActiveItemUserData()->GetString("value");
				if (Q_strncmp(pszValue, "custom", 6) != 0)
					pCvar->SetValue(pszValue);
			}
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Just load again to reset
	//-----------------------------------------------------------------------------
	void Reset()
	{
		Load();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Find the appropriate ConVar states and update the Check Boxes
	//-----------------------------------------------------------------------------
	void Load()
	{
		KeyValues *kvOptions = new KeyValues("Options");
		kvOptions->LoadFromFile(*pFilesystem, m_szSourceFile);

		// Loop through creating new options for each one
		for (KeyValues *kvOption = kvOptions->GetFirstSubKey(); kvOption != NULL; kvOption = kvOption->GetNextKey())
		{
			const char *pszCvar = kvOption->GetString("cvar");
			const char *pszName = kvOption->GetName();

			Panel *pChild = FindChildByName(pszName);

			if (!pChild)
				continue;

			ConVar *pCvar = cvar->FindVar(pszCvar);

			if (!pCvar)
				continue;

			// This is a bad show old chap
			if (CheckButton *cb = dynamic_cast <CheckButton *> (pChild))
			{
				cb->SetSelected(pCvar->GetBool());
			}
			else if (ComboBox *cb = dynamic_cast <ComboBox *> (pChild))
			{
				int option = GetComboBoxOption(cb, pCvar->GetString());

				// Option doesn't exist, so add a "custom field"
				if (option == -1)
				{
					int custom = GetComboBoxOption(cb, "custom");

					// Need to add the custom field
					if (custom == -1)
					{
						KeyValues *kvItem = new KeyValues("kvItem");
						kvItem->SetString("value", "custom");
						cb->AddItem("custom", kvItem);
						kvItem->deleteThis();
					}

					custom = GetComboBoxOption(cb, "custom");
					cb->ActivateItem(custom);
				}
				else
				{
					// We've found this item, so activate it
					cb->ActivateItem(option);

					// However lets remove the custom row if it exists
					int custom = GetComboBoxOption(cb, "custom");
					if (custom != -1)
						cb->DeleteItem(custom);
				}

				// Stuff below replaced by GetComboBoxOption()
				/*const char *pszCvarValue = pCvar->GetString();
				for (int i = 0; i < cb->GetItemCount(); i++)
				{
					KeyValues *kvItem = cb->GetItemUserData(i);
					const char *pszItemValue = kvItem->GetString("value");
					if (kvItem && Q_strncmp(pszItemValue, pszCvarValue, 5) == 0) {
						cb->ActivateItem(i);
						break;
					}
				}*/
			}
		}


		/*if ( !m_pHintsConVar )
			m_pHintsConVar = cvar->FindVar( "cl_hints" );
		if ( m_pHintsConVar )
			m_pHints->SetSelected( m_pHintsConVar->GetBool() );

		if ( !m_pAutoRLConVar )
			m_pAutoRLConVar = cvar->FindVar( "cl_autoreload" );
		if ( m_pAutoRLConVar )
			m_pARCheck->SetSelected( m_pAutoRLConVar->GetBool() );

		if ( !m_pAutoKillConVar )
			m_pAutoKillConVar = cvar->FindVar( "cl_classautokill" );
		if ( m_pAutoKillConVar )
			m_pAutoKillCheck->SetSelected( m_pAutoKillConVar->GetBool() );

		if ( !m_pBlurConVar )
			m_pBlurConVar = cvar->FindVar( "cl_dynamicblur" );
		if ( m_pBlurConVar )
			m_pBlurCheck->SetSelected( m_pBlurConVar->GetBool() );*/
	}

private:

	//-----------------------------------------------------------------------------
	// Purpose: Catch checkbox updating -- not needed, yet
	//-----------------------------------------------------------------------------
	//MESSAGE_FUNC_PARAMS(OnUpdateCheckbox, "CheckButtonChecked", data)
	//{
	//}

	CheckButton		*m_pHints;			// The enable/disable hints check box
	ConVar			*m_pHintsConVar;	// Pointer to the cl_hints convar

	CheckButton		*m_pARCheck;		// The enable/disable autoreload check box
	ConVar			*m_pAutoRLConVar;	// Pointer to the cl_autoreload convar

	CheckButton		*m_pAutoKillCheck;	// The enable/disable classautokill check box
	ConVar			*m_pAutoKillConVar;	// Pointer to the cl_classautokill convar

	CheckButton		*m_pBlurCheck;		// The enable/disable speed blur check box
	ConVar			*m_pBlurConVar;		// Pointer to the cl_dynamicblur convar
};

// Jiggles: End Miscellaneous Options Tab

DEFINE_GAMEUI(CFFOptions, CFFOptionsPanel, ffoptions);

//-----------------------------------------------------------------------------
// Purpose: Display the ff options
//-----------------------------------------------------------------------------
CON_COMMAND(ff_options, NULL)
{
	ffoptions->GetPanel()->SetVisible(true);
}

//-----------------------------------------------------------------------------
// Purpose: Set up our main options screen. This involves creating all the
//			tabbed pages too.
//-----------------------------------------------------------------------------
CFFOptionsPanel::CFFOptionsPanel(vgui::VPANEL parent) : BaseClass(NULL, "FFOptionsPanel")
{
	SetParent(parent);
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme");
	SetScheme(scheme);

	// Centre this panel on the screen for consistency.
	int nWide = GetWide();
	int nTall = GetTall();

	SetPos((ScreenWidth() - nWide) / 2, (ScreenHeight() - nTall) / 2);

	// This should be visible since we're only showing it when selected in the
	// main menu.
	SetVisible(false);

	m_pCrosshairOptions = new CFFCrosshairOptions(this, "CrosshairOptions");
	m_pTimerOptions = new CFFTimerOptions(this, "TimerOptions");
	m_pMiscOptions1 = new CFFMiscOptions(this, "MiscOptions", "resource/Options1.vdf");
	m_pMiscOptions2 = new CFFMiscOptions(this, "MiscOptions", "resource/Options2.vdf");
	m_pMiscOptions3 = new CFFMiscOptions(this, "MiscOptions", "resource/Options3.vdf");
	m_pMiscOptions4 = new CFFMiscOptions(this, "MiscOptions", "resource/Options4.vdf");
	m_pDLightOptions = new CFFDLightOptions(this, "DLightOptions");

	m_pPropertyPages = new PropertySheet(this, "OptionsPages", true);
	m_pPropertyPages->AddPage(m_pCrosshairOptions, "#GameUI_Crosshairs");
	m_pPropertyPages->AddPage(m_pTimerOptions, "#GameUI_Timers");
	m_pPropertyPages->AddPage(m_pMiscOptions1, "#GameUI_Misc1");
	m_pPropertyPages->AddPage(m_pMiscOptions2, "#GameUI_Misc2");
	m_pPropertyPages->AddPage(m_pMiscOptions3, "#GameUI_Misc3");
	m_pPropertyPages->AddPage(m_pMiscOptions4, "#GameUI_Misc4");
	m_pPropertyPages->AddPage(m_pDLightOptions, "#GameUI_DLights");
	m_pPropertyPages->SetActivePage(m_pCrosshairOptions);
	m_pPropertyPages->SetDragEnabled(false);

	m_pOKButton = new Button(this, "OKButton", "", this, "OK");
	m_pCancelButton = new Button(this, "CancelButton", "", this, "Cancel");
	m_pApplyButton = new Button(this, "ApplyButton", "", this, "Apply");

	SetSizeable(false);
	
	LoadControlSettings("resource/ui/FFOptions.res");
}

//-----------------------------------------------------------------------------
// Purpose: Catch the buttons. We have OK (save + close), Cancel (close) and
//			Apply (save).
//-----------------------------------------------------------------------------
void CFFOptionsPanel::OnButtonCommand(KeyValues *data)
{
	const char *pszCommand = data->GetString("command");

	// Both Apply and OK save the options window stuff
	if (Q_strcmp(pszCommand, "Apply") == 0 || Q_strcmp(pszCommand, "OK") == 0)
	{
		m_pCrosshairOptions->Apply();
		m_pTimerOptions->Apply();
		m_pMiscOptions1->Apply();
		m_pMiscOptions2->Apply();
		m_pMiscOptions3->Apply();
		m_pDLightOptions->Apply();

		// Apply doesn't quit the menu
		if (pszCommand[0] == 'A')
		{
			return;
		}
	}
	else
	{
		// Cancelled, so reset the settings
		m_pCrosshairOptions->Reset();
		m_pTimerOptions->Reset();
		m_pMiscOptions1->Reset();
		m_pMiscOptions2->Reset();
		m_pMiscOptions3->Reset();
		m_pDLightOptions->Reset();
	}

	// Now make invisible
	SetVisible(false);
}

//-----------------------------------------------------------------------------
// Purpose: Catch this options menu coming on screen and load up the info needed
//			for the option pages.
//-----------------------------------------------------------------------------
void CFFOptionsPanel::SetVisible(bool state)
{
	if (state)
	{
		m_pCrosshairOptions->Load();
		m_pTimerOptions->Load();
		m_pMiscOptions1->Load();
		m_pMiscOptions2->Load();
		m_pMiscOptions3->Load();
		m_pDLightOptions->Load();

		RequestFocus();
		MoveToFront();
	}

	BaseClass::SetVisible(state);
}

// This is the mod version string (defined in ff_gamerules.cpp)
// Jon: reading from a file now
//extern const char *MOD_CLIENT_VERSION;
char *GetModVersion();

// This is the URL for checking updates.
char *szSplashUrl = "http://www.fortress-forever.com/notifier/check.php?c=%s&s=%s";

// Singleton for our splash panel
static CFFSplashPanel *g_pSplashPanel = NULL;

class SplashHTML : public HTML
{
	DECLARE_CLASS_SIMPLE(SplashHTML, HTML);

public:
	SplashHTML(Panel *parent, const char *name, bool allowJavaScript = false) : HTML(parent, name, allowJavaScript) {}

	void OnFinishURL(const char *url)
	{
		BaseClass::OnFinishURL(url);

		// We've been redirected to a page that we need to display
		if (url && Q_strstr(url, "update_notice") != NULL)
		{
			Msg("Client/Server out of date\n");
			GetParent()->SetVisible(true);
			RequestFocus();
			GetParent()->MoveToFront();
		}
		else
		{
			Msg("Client/Server up to date\n");
		}
	}
};

CFFSplashPanel::CFFSplashPanel(vgui::VPANEL parent)
		: BaseClass(NULL, "FFSplashPanel")
{
	Assert(g_pSplashPanel == NULL);
	g_pSplashPanel = this;

	m_pSplashHTML = new SplashHTML(this, "FFSplashPanelHTML");
	m_pSplashHTML->SetVisible(true);

	SetParent(parent);
	SetSizeable(false);

	LoadControlSettings("resource/ui/FFSplash.res");

	// Do our message box in here too
	if (CommandLine()->FindParm("-showdxmsg"))
	{
		MessageBox *pMessageBox = new MessageBox("DirectX Changed", "DirectX level changed\nPlease now quit and load Fortress Forever normally for the settings to take effect.", 0);
		pMessageBox->SetVisible(true);
		pMessageBox->RequestFocus();
		pMessageBox->MoveToFront();
		pMessageBox->ShowWindow();
	}

	CheckUpdate();
};

void CFFSplashPanel::CheckUpdate(const char *pszServerVersion /*= NULL*/)
{
	m_pSplashHTML->OpenURL(VarArgs(szSplashUrl, GetModVersion(), pszServerVersion ? pszServerVersion : ""));
	m_pSplashHTML->SetVisible(true);
}

void CheckModUpdate(const char *pszServerVersion = NULL)
{
	Assert(g_pSplashPanel);
	if (g_pSplashPanel)
	{
		g_pSplashPanel->CheckUpdate(pszServerVersion);
	}
}

DEFINE_GAMEUI(CFFSplash, CFFSplashPanel, ffsplash);

float g_flLastCheck = -15.0f;

// We have received the server's version, so check on the website
CON_COMMAND(sync_version, "Sync version")
{
	if (engine->Cmd_Argc() > 1 && g_flLastCheck < gpGlobals->realtime)
	{
		g_flLastCheck = gpGlobals->realtime + 3.0f;
		const char *pszVersion = engine->Cmd_Argv(1);
		CheckModUpdate(pszVersion);
		Msg("Server version %s\n", pszVersion);
	}
}
