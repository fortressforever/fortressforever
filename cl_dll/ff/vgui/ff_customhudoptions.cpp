#include "cbase.h"
#include "ff_customhudoptions.h"

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Populate all the menu stuff
//-----------------------------------------------------------------------------
CFFCustomHudOptions::CFFCustomHudOptions(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
{
	m_pArrangementPresets = new CFFCustomHudArrangementPresets(this, "CustomHudArrangementPresets", "Arrangement");
	m_pStylePresets = new CFFCustomHudStylePresets(this, "CustomHudStylePresets", "Style");
	m_pAssignPresets = new CFFCustomHudAssignPresets(this, "CustomHudAssignPresets", m_pStylePresets, m_pArrangementPresets);

	m_pPropertyPages = new PropertySheet(this, "CustomHudPages", true);

	m_pPropertyPages->AddPage(m_pArrangementPresets, "#GameUI_ArrangementPresets");
	m_pPropertyPages->AddPage(m_pStylePresets, "#GameUI_StylePresets");
	m_pPropertyPages->AddPage(m_pAssignPresets, "#GameUI_AssignPresets");
	m_pPropertyPages->SetActivePage(m_pAssignPresets);
	m_pPropertyPages->SetDragEnabled(false);
	
	LoadControlSettings("resource/ui/FFOptionsSubCustomHud.res");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFCustomHudOptions::AllowChanges(bool state)
{

}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFCustomHudOptions::Apply()
{
	m_pAssignPresets->Apply();
	m_pArrangementPresets->Apply();
	m_pStylePresets->Apply();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFCustomHudOptions::Load()
{
	m_pAssignPresets->Load();
	m_pArrangementPresets->Load();
	m_pStylePresets->Load();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFCustomHudOptions::Reset()
{
	m_pAssignPresets->Reset();
	m_pArrangementPresets->Reset();
	m_pStylePresets->Reset();
}