/********************************************************************
	created:	2006/08/29
	created:	29:8:2006   18:35
	filename: 	cl_dll\ff\vgui\ff_options\ff_crosshairoptions.cpp
	file path:	cl_dll\ff\vgui\ff_crosshairoptions
	file base:	ff_crosshairoptions
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Crosshair options page within fortress options

	notes: 
	
	Used to be contained in ff_options.cpp
	Separated by elmo 01/09/2010
*********************************************************************/

#include "cbase.h"
#include "ff_crosshairoptions.h"

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

CFFCrosshairOptions *g_pCrosshairOptions = NULL;

CFFCrosshairOptions::CFFCrosshairOptions(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
{
	g_pCrosshairOptions = this;

	memset(&m_sCrosshairInfo, 0, sizeof(m_sCrosshairInfo));

	m_bLoaded = false;

	const char *pszInner = "1234567890-=!@#$%^?*()_+";
	int nInner = strlen(pszInner);

	m_pInnerCharacter = new ComboBox(this, "InnerCharacter", nInner, false);
	m_pInnerCharacter->AddActionSignalTarget(this);

	const char *pszOuter = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int nOuter = strlen(pszOuter);

	m_pOuterCharacter = new ComboBox(this, "OuterCharacter", nOuter, false);
	m_pOuterCharacter->AddActionSignalTarget(this);

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

	m_pInnerScale = new CFFInputSlider(this, "InnerScale", "InnerScaleInput");
	m_pInnerScale->SetRange(1, CROSSHAIR_SIZES);
	m_pInnerScale->SetValue(1 + CROSSHAIR_SIZES / 2);

	m_pOuterScale = new CFFInputSlider(this, "OuterScale", "OuterScaleInput");
	m_pOuterScale->SetRange(1, CROSSHAIR_SIZES);
	m_pOuterScale->SetValue(1 + CROSSHAIR_SIZES / 2);

	m_pInnerRed = new CFFInputSlider(this, "InnerRed", "InnerRedInput");
	m_pInnerRed->SetRange(0, 255);
	m_pInnerRed->SetValue(255);

	m_pOuterRed = new CFFInputSlider(this, "OuterRed", "OuterRedInput");
	m_pOuterRed->SetRange(0, 255);
	m_pOuterRed->SetValue(255);

	m_pInnerGreen = new CFFInputSlider(this, "InnerGreen", "InnerGreenInput");
	m_pInnerGreen->SetRange(0, 255);
	m_pInnerGreen->SetValue(255);

	m_pOuterGreen = new CFFInputSlider(this, "OuterGreen", "OuterGreenInput");
	m_pOuterGreen->SetRange(0, 255);
	m_pOuterGreen->SetValue(255);

	m_pInnerBlue = new CFFInputSlider(this, "InnerBlue", "InnerBlueInput");
	m_pInnerBlue->SetRange(0, 255);
	m_pInnerBlue->SetValue(255);

	m_pOuterBlue = new CFFInputSlider(this, "OuterBlue", "OuterBlueInput");
	m_pOuterBlue->SetRange(0, 255);
	m_pOuterBlue->SetValue(255);

	m_pInnerAlpha = new CFFInputSlider(this, "InnerAlpha", "InnerAlphaInput");
	m_pInnerAlpha->SetRange(0, 255);
	m_pInnerAlpha->SetValue(255);

	m_pOuterAlpha = new CFFInputSlider(this, "OuterAlpha", "OuterAlphaInput");
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
}
//-----------------------------------------------------------------------------
// Purpose: Allow all inner options to be enabled or disabled quickly
//-----------------------------------------------------------------------------
void CFFCrosshairOptions::AllowInnerChanges(bool state)
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
void CFFCrosshairOptions::AllowOuterChanges(bool state)
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
void CFFCrosshairOptions::AllowWeaponSelection(bool state)
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
void CFFCrosshairOptions::ApplySchemeSettings(IScheme *pScheme)
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
void CFFCrosshairOptions::Apply()
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

bool CFFCrosshairOptions::IsReady()
{
	if(vgui::filesystem())
	{
		if(!m_bLoaded)
		{
			Load();
		}
		else
		{
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Load all the settings from file into memory via keyvalues
//-----------------------------------------------------------------------------
void CFFCrosshairOptions::Load()
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

	m_bLoaded = true;
}

//-----------------------------------------------------------------------------
// Purpose: Reload the stuff from file for now
//-----------------------------------------------------------------------------
void CFFCrosshairOptions::Reset()
{
	Load();
}

//-----------------------------------------------------------------------------
// Purpose: Get crosshair for a weapon
//-----------------------------------------------------------------------------
void CFFCrosshairOptions::GetCrosshair(FFWeaponID iWeapon, char &innerChar, Color &innerCol, int &innerSize, char &outerChar, Color &outerCol, int &outerSize)
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
void CFFCrosshairOptions::GetHitCrosshair(char &innerChar, Color &innerCol, int &innerSize, char &outerChar, Color &outerCol, int &outerSize)
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

//-----------------------------------------------------------------------------
// Purpose: Update the crosshair
//			This isn't a particularly great function, it should really be
//			split up into separate responsibilities (update crosshair data,
//			refresh crosshair preview).
//-----------------------------------------------------------------------------
void CFFCrosshairOptions::UpdateCrosshairs()
{
	int iCurrentWeapon = m_pWeapon->GetActiveItem();
	//assert forgot to include hit
	Assert(iCurrentWeapon >= 0 && iCurrentWeapon <= FF_WEAPON_TOMMYGUN + 1);

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
void CFFCrosshairOptions::UpdateSliders()
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


//-----------------------------------------------------------------------------
// Purpose: Catch the slider moving
//-----------------------------------------------------------------------------
void CFFCrosshairOptions::OnUpdateSliders(KeyValues *data)
{
	UpdateCrosshairs();
}

//-----------------------------------------------------------------------------
// Purpose: Catch checkbox updating
//-----------------------------------------------------------------------------
void CFFCrosshairOptions::OnUpdateCheckButton(KeyValues *data)
{
	UpdateCrosshairs();
}

//-----------------------------------------------------------------------------
// Purpose: Catch the combo box changing
//-----------------------------------------------------------------------------
void CFFCrosshairOptions::OnUpdateCombos(KeyValues *data)
{
	// Make sure the sliders are set correct
	if (data->GetPtr("panel") == m_pWeapon)
	{
		UpdateSliders();
	}

	UpdateCrosshairs();
}

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