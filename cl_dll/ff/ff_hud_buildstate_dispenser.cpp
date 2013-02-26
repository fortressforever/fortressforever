#include "cbase.h"
#include "ff_hud_buildstate_dispenser.h"
#include "ff_buildableobjects_shared.h"

DECLARE_HUDELEMENT(CHudBuildStateDispenser);
DECLARE_HUD_MESSAGE(CHudBuildStateDispenser, DispenserMsg);

CHudBuildStateDispenser::CHudBuildStateDispenser(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudBuildStateDispenser")
{
	SetParent(g_pClientMode->GetViewport());

	// Hide when player is dead and not engi
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_NOTENGINEER );

	SetUseToggleText(true);

	m_bBuilt = false;
	m_bBuilding = false;
}

CHudBuildStateDispenser::~CHudBuildStateDispenser() 
{
}

KeyValues* CHudBuildStateDispenser::GetDefaultStyleData()
{
	KeyValues *kvPreset = BaseClass::GetDefaultStyleData();

	kvPreset->SetInt("x", 580);
	kvPreset->SetInt("y", 330);
	kvPreset->SetInt("alignH", FFQuantityPanel::ALIGN_RIGHT);
	kvPreset->SetInt("alignV", FFQuantityPanel::ALIGN_TOP);

	return kvPreset;
}

void CHudBuildStateDispenser::VidInit()
{
	wchar_t *tempString = vgui::localize()->Find("#HudPanel_Dispenser");
	
	wcsupr(tempString);

	if (!tempString) 
		tempString = L"DISPENSER";
	
	SetHeaderText(tempString, false);
	SetHeaderIconChar("4", false);

	m_wszNotBuiltText = vgui::localize()->Find("#HudPanel_NotBuilt"); 

	if (!m_wszNotBuiltText) 
		m_wszNotBuiltText = L"Not Built";
	SetText(m_wszNotBuiltText);

	m_wszBuildingText = vgui::localize()->Find("#HudPanel_Building"); 

	if (!m_wszBuildingText) 
		m_wszBuildingText = L"Building...";
	
	m_qiDispenserHealth->SetLabelText("#FF_ITEM_HEALTH", false);
	m_qiDispenserHealth->SetIconChar("a", false);
	m_qiDispenserHealth->ShowAmountMax(false);
	HideItem(m_qiDispenserAmmo);	

	m_qiDispenserAmmo->SetLabelText("#FF_ITEM_AMMO", false);
	m_qiDispenserAmmo->SetIconChar("r", false);
	m_qiDispenserAmmo->ShowAmountMax(false);
	HideItem(m_qiDispenserHealth);
	
	m_qiCellCounter->SetLabelText("#FF_ITEM_CELLS", false);
	m_qiCellCounter->SetIconChar("p", false);
	m_qiCellCounter->SetIntensityControl(0, (int)(FF_BUILDCOST_DISPENSER/3), (int)(FF_BUILDCOST_DISPENSER/3) * 2, FF_BUILDCOST_DISPENSER);
	m_qiCellCounter->SetAmountMax(FF_BUILDCOST_DISPENSER);
	
	SetToggleTextVisible(true);
}

void CHudBuildStateDispenser::Init() 
{
	ivgui()->AddTickSignal(GetVPanel(), 250); //only update 4 times a second
	HOOK_HUD_MESSAGE(CHudBuildStateDispenser, DispenserMsg);

	m_qiDispenserHealth = AddItem("BuildStateDispenserHealth");
	m_qiDispenserAmmo = AddItem("BuildStateDispenserAmmo");
	m_qiCellCounter = AddItem("BuildStateCellCounter");

	AddPanelToHudOptions("Dispenser", "#HudPanel_Dispenser", "BuildState", "#HudPanel_BuildableState");
}

void CHudBuildStateDispenser::OnTick() 
{
	BaseClass::OnTick();

	if( !engine->IsInGame() | !ShouldDraw() )
		return;

	// Get the local player
	C_FFPlayer *pPlayer = ToFFPlayer(C_BasePlayer::GetLocalPlayer());

	// Never below zero (dunno why this is here)
	int iCells = max( pPlayer->GetAmmoCount( AMMO_CELLS ), 0);
	iCells = min(iCells, FF_BUILDCOST_DISPENSER);
	// Only update if we've changed cell count
	if ( iCells != m_qiCellCounter->GetAmount() )
		m_qiCellCounter->SetAmount(iCells);

	bool bBuilt = pPlayer->GetDispenser() && pPlayer->GetDispenser()->IsBuilt();
	bool bBuilding = pPlayer->GetDispenser() && !bBuilt;

	//small optimisation by comparing building with what it was previously
	//if building
	if(bBuilding && !m_bBuilding)
	//show building text
	{
		SetText(m_wszBuildingText);
		m_bBuilding = bBuilding;
	}
	//if not building
	else if(!bBuilding && m_bBuilding)
	//show not built text
	{
		SetText(m_wszNotBuiltText);
		m_bBuilding = bBuilding;
	}
	
	//small optimisation by comparing build with what it was previously
	//if not built
	if(!bBuilt && m_bBuilt)
	//hide quantity bars
	{
		m_bBuilt = false;
		HideItem(m_qiDispenserHealth);
		HideItem(m_qiDispenserAmmo);
		EnableItem(m_qiCellCounter);
		SetToggleTextVisible(true);
	}
	else if(bBuilt && !m_bBuilt)
	//show quantity bars
	{
		m_bBuilt = true;
		ShowItem(m_qiDispenserHealth);
		ShowItem(m_qiDispenserAmmo);
		DisableItem(m_qiCellCounter);
		SetToggleTextVisible(false);
	}
}

void CHudBuildStateDispenser::Paint() 
{
	//paint header
	BaseClass::Paint();
}

void CHudBuildStateDispenser::MsgFunc_DispenserMsg(bf_read &msg)
{
    int iHealth = (int) msg.ReadByte();
    int iAmmo = (int) msg.ReadByte();

	m_qiDispenserHealth->SetAmount(iHealth);
	m_qiDispenserAmmo->SetAmount(iAmmo);
}