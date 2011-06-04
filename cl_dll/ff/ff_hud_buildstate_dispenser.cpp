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
	KeyValues *kvPreset = new KeyValues("StyleData");

	kvPreset->SetInt("x", 480);
	kvPreset->SetInt("y", 330);
	kvPreset->SetInt("alignH", 0);
	kvPreset->SetInt("alignV", 0);

	kvPreset->SetInt("columns", 1);
	kvPreset->SetInt("headerTextX", 5);
	kvPreset->SetInt("headerTextY", 5);
	kvPreset->SetInt("headerIconX", 6);
	kvPreset->SetInt("headerIconY", 16);
	kvPreset->SetInt("textX", 35);
	kvPreset->SetInt("textY", 22);
	kvPreset->SetInt("itemsX", 5);
	kvPreset->SetInt("itemsY", 15);
	kvPreset->SetInt("showHeaderText", 1);
	kvPreset->SetInt("showHeaderIcon", 1);
	kvPreset->SetInt("showText", 1);
	kvPreset->SetInt("headerTextShadow", 0);
	kvPreset->SetInt("headerIconShadow", 0);
	kvPreset->SetInt("textShadow", 0);
	kvPreset->SetInt("headerTextSize", 1);
	kvPreset->SetInt("headerIconSize", 11);
	kvPreset->SetInt("textSize", 2);
	kvPreset->SetInt("showPanel", 1);
	kvPreset->SetInt("panelColorCustom", 0);
	kvPreset->SetInt("panelRed", 255);
	kvPreset->SetInt("panelGreen", 255);
	kvPreset->SetInt("panelBlue", 255);
	kvPreset->SetInt("panelAlpha", 255);

	kvPreset->SetInt("barWidth", 60);
	kvPreset->SetInt("barHeight", 10);
	kvPreset->SetInt("barBorderWidth", 1);
	kvPreset->SetInt("barMarginHorizontal", 0);
	kvPreset->SetInt("barMarginVertical", 0);
	kvPreset->SetInt("barOrientation", ORIENTATION_HORIZONTAL);

	KeyValues *kvComponent = new KeyValues("Bar");
	kvComponent->SetInt("show", 1);
	kvComponent->SetInt("colorMode", COLOR_MODE_FADED);
	kvComponent->SetInt("red", 255);
	kvComponent->SetInt("green", 255);
	kvComponent->SetInt("blue", 255);
	kvComponent->SetInt("alpha", 93);

	kvPreset->AddSubKey(kvComponent);

	kvComponent = new KeyValues("BarBorder");
	kvComponent->SetInt("show", 1);
	kvComponent->SetInt("colorMode", COLOR_MODE_CUSTOM);
	kvComponent->SetInt("red", 255);
	kvComponent->SetInt("green", 255);
	kvComponent->SetInt("blue", 255);
	kvComponent->SetInt("alpha", 155);

	kvPreset->AddSubKey(kvComponent);

	kvComponent = new KeyValues("BarBackground");
	kvComponent->SetInt("show", 1);
	kvComponent->SetInt("colorMode", COLOR_MODE_FADED);
	kvComponent->SetInt("red", 255);
	kvComponent->SetInt("green", 255);
	kvComponent->SetInt("blue", 255);
	kvComponent->SetInt("alpha", 65);

	kvPreset->AddSubKey(kvComponent);

	kvComponent = new KeyValues("Icon");
	kvComponent->SetInt("show", 0);
	kvComponent->SetInt("colorMode", COLOR_MODE_CUSTOM);
	kvComponent->SetInt("red", 255);
	kvComponent->SetInt("green", 255);
	kvComponent->SetInt("blue", 255);
	kvComponent->SetInt("alpha", 255);
	kvComponent->SetInt("shadow", 0);
	kvComponent->SetInt("size", 2);
	kvComponent->SetInt("alignH", ALIGN_CENTER);
	kvComponent->SetInt("alignV", ALIGN_MIDDLE);
	kvComponent->SetInt("offsetX", 5);
	kvComponent->SetInt("offsetY", 0);

	kvPreset->AddSubKey(kvComponent);

	kvComponent = new KeyValues("Label");
	kvComponent->SetInt("show", 1);
	kvComponent->SetInt("colorMode", COLOR_MODE_CUSTOM);
	kvComponent->SetInt("red", 255);
	kvComponent->SetInt("green", 255);
	kvComponent->SetInt("blue", 255);
	kvComponent->SetInt("alpha", 255);
	kvComponent->SetInt("shadow", 1);
	kvComponent->SetInt("size", 1);
	kvComponent->SetInt("alignH", ALIGN_LEFT);
	kvComponent->SetInt("alignV", ALIGN_MIDDLE);
	kvComponent->SetInt("offsetX", 54);
	kvComponent->SetInt("offsetY", 0);
	kvComponent->SetInt("fontTahoma", 1);

	kvPreset->AddSubKey(kvComponent);

	kvComponent = new KeyValues("Amount");
	kvComponent->SetInt("show", 1);
	kvComponent->SetInt("colorMode", COLOR_MODE_FADED);
	kvComponent->SetInt("red", 255);
	kvComponent->SetInt("green", 255);
	kvComponent->SetInt("blue", 255);
	kvComponent->SetInt("alpha", 255);
	kvComponent->SetInt("shadow", 0);
	kvComponent->SetInt("size", 0);
	kvComponent->SetInt("alignH", ALIGN_RIGHT);
	kvComponent->SetInt("alignV", ALIGN_CENTER);
	kvComponent->SetInt("offsetX", 3);
	kvComponent->SetInt("offsetY", 1);
	kvComponent->SetInt("fontTahoma", 0);

	kvPreset->AddSubKey(kvComponent);

	return kvPreset;
}

void CHudBuildStateDispenser::VidInit()
{
	wchar_t *tempString = vgui::localize()->Find("#FF_PLAYER_DISPENSER");
	
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
	
	m_qbDispenserHealth->SetLabelText("#FF_ITEM_HEALTH", false);
	m_qbDispenserHealth->SetIconChar("a", false);
	m_qbDispenserHealth->ShowAmountMax(false);
	m_qbDispenserAmmo->SetVisible(false);	

	m_qbDispenserAmmo->SetLabelText("#FF_ITEM_AMMO", false);
	m_qbDispenserAmmo->SetIconChar("r", false);
	m_qbDispenserAmmo->ShowAmountMax(false);
	m_qbDispenserHealth->SetVisible(false);
	
	m_qbCellCounter->SetLabelText("#FF_ITEM_CELLS", false);
	m_qbCellCounter->SetIconChar("p", false);
	m_qbCellCounter->SetIntensityControl(0, (int)(FF_BUILDCOST_DISPENSER/3), (int)(FF_BUILDCOST_DISPENSER/3) * 2, FF_BUILDCOST_DISPENSER);
	m_qbCellCounter->SetAmountMax(FF_BUILDCOST_DISPENSER);
	
	SetToggleTextVisible(true);
}

void CHudBuildStateDispenser::Init() 
{
	ivgui()->AddTickSignal(GetVPanel(), 250); //only update 4 times a second
	HOOK_HUD_MESSAGE(CHudBuildStateDispenser, DispenserMsg);

	m_qbDispenserHealth = AddItem("BuildStateDispenserHealth");
	m_qbDispenserAmmo = AddItem("BuildStateDispenserAmmo");
	m_qbCellCounter = AddItem("BuildStateCellCounter");

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
	if ( iCells != m_qbCellCounter->GetAmount() )
		m_qbCellCounter->SetAmount(iCells);

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
		m_qbDispenserHealth->SetVisible(false);
		m_qbDispenserAmmo->SetVisible(false);
		ShowItem(m_qbCellCounter);
		SetToggleTextVisible(true);
	}
	else if(bBuilt && !m_bBuilt)
	//show quantity bars
	{
		m_bBuilt = true;
		m_qbDispenserHealth->SetVisible(true);
		m_qbDispenserAmmo->SetVisible(true);
		HideItem(m_qbCellCounter);
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

	m_qbDispenserHealth->SetAmount(iHealth);
	m_qbDispenserAmmo->SetAmount(iAmmo);
}