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

extern IFileSystem **pFilesystem;

#define CROSSHAIRS_FILE		"crosshairs.vdf"
#define CROSSHAIR_SIZES		5					// This needs to be matched in hud_crosshair.h

extern ConVar cl_timerwav;

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
		m_pWeapon = new ComboBox(this, "Weapon", (int) FF_WEAPON_TOMMYGUN, false);
		m_pWeapon->AddActionSignalTarget(this);
		m_pWeapon->SetEditable(false);

		// Add the global one first
		KeyValues *kv = new KeyValues("W");
		kv->SetString("wpid", "global");
		m_pWeapon->AddItem("Global", kv);
		kv->deleteThis();

		for (int i = 1; i < FF_WEAPON_TOMMYGUN; i++)
		{
			KeyValues *kv = new KeyValues("W");
			kv->SetString("wpid", s_WeaponAliasInfo[i]);
			m_pWeapon->AddItem(VarArgs("#Crosshair_%s", s_WeaponAliasInfo[i]), kv);
			kv->deleteThis();
		}		

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

		for (int i = 0; i < FF_WEAPON_TOMMYGUN; i++)
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
		for (int i = 0; i < FF_WEAPON_TOMMYGUN; i++)
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
		Assert(iCurrentWeapon >= 0 && iCurrentWeapon < FF_WEAPON_TOMMYGUN);

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
		cinfo.innerUseGlobal = iCurrentWeapon == 0 ? 0 : m_pInnerUseGlobal->IsSelected();
		cinfo.outerUseGlobal = iCurrentWeapon == 0 ? 0 : m_pOuterUseGlobal->IsSelected();

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
		if (iCurrentWeapon == 0)
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
		Assert(iCurrentWeapon >= 0 && iCurrentWeapon < FF_WEAPON_TOMMYGUN);

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

	WeaponCrosshair_t	m_sCrosshairInfo[FF_WEAPON_TOMMYGUN];

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
	}

	ComboBox	*m_pTimers;
	Button		*m_pPlayButton;
};

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

	m_pPropertyPages = new PropertySheet(this, "OptionsPages", true);
	m_pPropertyPages->AddPage(m_pCrosshairOptions, "#GameUI_Crosshairs");
	m_pPropertyPages->AddPage(m_pTimerOptions, "#GameUI_Timers");
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

		RequestFocus();
		MoveToFront();
	}

	BaseClass::SetVisible(state);
}