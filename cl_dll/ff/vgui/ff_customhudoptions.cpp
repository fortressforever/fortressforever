#include "cbase.h"
#include "ff_customhudoptions.h"

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

using namespace vgui;

DEFINE_GAMEUI(CFFCustomHudOptions, CFFCustomHudOptionsPanel, ffcustomhudoptions);

//-----------------------------------------------------------------------------
// Purpose: Display the ff custom hud options panel
//-----------------------------------------------------------------------------
CON_COMMAND(ff_customhudoptions, NULL)
{
	ffcustomhudoptions->GetPanel()->SetVisible(true);
}

//-----------------------------------------------------------------------------
// Purpose: Populate all the menu stuff
//-----------------------------------------------------------------------------
CFFCustomHudOptionsPanel::CFFCustomHudOptionsPanel(VPANEL parent) : BaseClass( NULL, "FFCustomHudOptions" )
{
	m_pPositionPresets = new CFFCustomHudPositionPresets(this, "CustomHudPositionPresets", "Position");
	m_pPanelStylePresets = new CFFCustomHudPanelStylePresets(this, "CustomHudPanelStylePresets", "PanelStyle");
	m_pItemStylePresets = new CFFCustomHudItemStylePresets(this, "CustomHudItemStylePresets", "ItemStyle");
	m_pAssignPresets = new CFFCustomHudAssignPresets(this, "CustomHudAssignPresets", m_pItemStylePresets, m_pPanelStylePresets, m_pPositionPresets);

	m_pPropertyPages = new PropertySheet(this, "CustomHudPages", true);

	m_pPropertyPages->AddPage(m_pAssignPresets, "#GameUI_AssignPresets");
	m_pPropertyPages->AddPage(m_pPositionPresets, "#GameUI_PositionPresets");
	m_pPropertyPages->AddPage(m_pPanelStylePresets, "#GameUI_PanelStylePresets");
	m_pPropertyPages->AddPage(m_pItemStylePresets, "#GameUI_ItemStylePresets");
	m_pPropertyPages->SetActivePage(m_pAssignPresets);
	m_pPropertyPages->SetDragEnabled(false);
	
	m_pPositionPresets->RemoveActionSignalTarget( m_pPropertyPages );
	m_pPanelStylePresets->RemoveActionSignalTarget( m_pPropertyPages );
	m_pItemStylePresets->RemoveActionSignalTarget( m_pPropertyPages );
	
	m_pPositionPresets->AddActionSignalTarget( this );
	m_pPanelStylePresets->AddActionSignalTarget( this );
	m_pItemStylePresets->AddActionSignalTarget( this );

	m_pOKButton = new Button(this, "OKButton", "", this, "OK");
	m_pCancelButton = new Button(this, "CancelButton", "", this, "Cancel");
	m_pApplyButton = new Button(this, "ApplyButton", "", this, "Apply");

	SetSizeable(false);
	
	LoadControlSettings("resource/ui/FFCustomHudOptions.res");
}
void CFFCustomHudOptionsPanel::OnActivatePositionPage(KeyValues* data)
{
	m_pPropertyPages->SetActivePage(m_pPositionPresets);
}
void CFFCustomHudOptionsPanel::OnActivatePanelStylePage(KeyValues* data)
{
	m_pPropertyPages->SetActivePage(m_pPanelStylePresets);
}
void CFFCustomHudOptionsPanel::OnActivateItemStylePage(KeyValues* data)
{
	m_pPropertyPages->SetActivePage(m_pItemStylePresets);
}
//-----------------------------------------------------------------------------
// Purpose: Catch the buttons. We have OK (save + close), Cancel (close) and
//			Apply (save).
//-----------------------------------------------------------------------------
void CFFCustomHudOptionsPanel::OnButtonCommand(KeyValues *data)
{
	const char *pszCommand = data->GetString("command");

	// Both Apply and OK save the options window stuff
	if (Q_strcmp(pszCommand, "Apply") == 0 || Q_strcmp(pszCommand, "OK") == 0)
	{
		m_pAssignPresets->Apply();
		m_pPositionPresets->Apply();
		m_pPanelStylePresets->Apply();
		m_pItemStylePresets->Apply();

		// Apply doesn't quit the menu
		if (pszCommand[0] == 'A')
		{
			return;
		}
	}
	else
	{
		// Cancelled, so reset the settings
		m_pPositionPresets->Reset();
		m_pPanelStylePresets->Reset();
		m_pItemStylePresets->Reset();
		m_pAssignPresets->Reset();
	}

	// Now make invisible
	SetVisible(false);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFCustomHudOptionsPanel::Apply()
{
	m_pPositionPresets->Apply();
	m_pPanelStylePresets->Apply();
	m_pItemStylePresets->Apply();
	m_pAssignPresets->Apply();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFCustomHudOptionsPanel::Load()
{
	m_pPositionPresets->Load();
	m_pPanelStylePresets->Load();
	m_pItemStylePresets->Load();
	m_pAssignPresets->Load();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFCustomHudOptionsPanel::Reset()
{
	m_pPositionPresets->Reset();
	m_pPanelStylePresets->Reset();
	m_pItemStylePresets->Reset();
	m_pAssignPresets->Reset();
}
void CFFCustomHudOptionsPanel::SetVisible(bool state)
{
	if (state)
	{
		// Centre this panel on the screen for consistency.
		int nWide = GetWide();
		int nTall = GetTall();

		SetPos((ScreenWidth() - nWide) / 2, (ScreenHeight() - nTall) / 2);

		RequestFocus();
		MoveToFront();
	}

	BaseClass::SetVisible(state);
}