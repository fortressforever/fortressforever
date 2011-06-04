#include "cbase.h"
#include "ff_hud_buildstate_sentry.h"
#include "ff_buildableobjects_shared.h"

#define HIDECELLS_NEVER 0
#define HIDECELLS_ALWAYS 1
#define HIDECELLS_IFBUILT 2

DECLARE_HUDELEMENT(CHudBuildStateSentry);
DECLARE_HUD_MESSAGE(CHudBuildStateSentry, SentryMsg);

CHudBuildStateSentry::CHudBuildStateSentry(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudBuildStateSentry")
{
	SetParent(g_pClientMode->GetViewport());

	// Hide when player is dead and not engi
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_NOTENGINEER );

	SetUseToggleText(true);

	m_bBuilt = false;
	m_bBuilding = false;
}
	
CHudBuildStateSentry::~CHudBuildStateSentry() 
{
}

KeyValues* CHudBuildStateSentry::GetDefaultStyleData()
{
	KeyValues *kvPreset = new KeyValues("StyleData");

	kvPreset->SetInt("x", 480);
	kvPreset->SetInt("y", 325);
	kvPreset->SetInt("alignH", 0);
	kvPreset->SetInt("alignV", 2);

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

void CHudBuildStateSentry::VidInit()
{
	wchar_t *tempString = vgui::localize()->Find("#FF_PLAYER_SENTRYGUN");

	if (!tempString) 
		tempString = L"SENTRY";

	SetHeaderText(tempString, false);
	SetHeaderIconChar("1", false);

	m_wszNotBuiltText = vgui::localize()->Find("#HudPanel_NotBuilt"); 

	if (!m_wszNotBuiltText) 
		m_wszNotBuiltText = L"Not Built";
	SetText(m_wszNotBuiltText);

	m_wszBuildingText = vgui::localize()->Find("#HudPanel_Building"); 

	if (!m_wszBuildingText) 
		m_wszBuildingText = L"Building...";

	m_qbSentryHealth->SetLabelText("#FF_ITEM_HEALTH", false);
	m_qbSentryHealth->SetIconChar("a", false);
	m_qbSentryHealth->SetIntensityAmountScaled(true);//max changes (is not 100) so we need to scale to a percentage amount for calculation
	m_qbSentryHealth->SetAmount(0);
	m_qbSentryHealth->SetVisible(false);

	m_qbSentryLevel->SetLabelText("#FF_ITEM_LEVEL", false);
	m_qbSentryLevel->SetIconChar("d", false);
	m_qbSentryLevel->SetAmountMax(3);
	m_qbSentryLevel->SetIntensityControl(1,2,2,3);
	m_qbSentryLevel->SetIntensityValuesFixed(true);
	m_qbSentryLevel->SetAmount(0);
	m_qbSentryLevel->SetVisible(false);

	m_qbCellCounter->SetLabelText("#FF_ITEM_CELLS", false);
	m_qbCellCounter->SetIconChar("p", false);
	m_qbCellCounter->SetIntensityControl(0, (int)(FF_BUILDCOST_SENTRYGUN/3), (int)(FF_BUILDCOST_SENTRYGUN/3) * 2, FF_BUILDCOST_SENTRYGUN);
	m_qbCellCounter->SetAmountMax(FF_BUILDCOST_SENTRYGUN);
	m_iMaxCells = FF_BUILDCOST_SENTRYGUN;

	SetToggleTextVisible(true);
}

void CHudBuildStateSentry::Init() 
{
	ivgui()->AddTickSignal(GetVPanel(), 250); //only update 4 times a second
	HOOK_HUD_MESSAGE(CHudBuildStateSentry, SentryMsg);

	m_qbSentryHealth = AddItem("BuildStateSentryHealth"); 
	m_qbSentryLevel = AddItem("BuildStateSentryLevel"); 
	m_qbCellCounter = AddItem("BuildStateCellCounter"); 

	AddPanelToHudOptions("SentryGun", "#HudPanel_SentryGun", "BuildState", "#HudPanel_BuildableState");
}

void CHudBuildStateSentry::OnTick() 
{
	BaseClass::OnTick();

	if( !engine->IsInGame() | !ShouldDraw() )
		return;

	// Get the local player
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	// Never below zero (dunno why this is here)
	int iCells = max( pPlayer->GetAmmoCount( AMMO_CELLS ), 0);
	iCells = min(iCells, m_iMaxCells);
	// Only update if we've changed cell count
	if ( iCells != m_qbCellCounter->GetAmount() )
		m_qbCellCounter->SetAmount(iCells);

	bool bBuilt = pPlayer->GetSentryGun() && pPlayer->GetSentryGun()->IsBuilt();
	bool bBuilding = pPlayer->GetSentryGun() && !bBuilt;

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
		m_qbSentryHealth->SetVisible(false);
		m_qbSentryLevel->SetVisible(false);
		ShowItem(m_qbCellCounter);
		SetToggleTextVisible(true);
	}
	else if(bBuilt && !m_bBuilt)
	//show quantity bars
	{
		m_bBuilt = true;
		m_qbSentryHealth->SetVisible(true);
		m_qbSentryLevel->SetVisible(true);
		SetToggleTextVisible(false);
	}
}

void CHudBuildStateSentry::Paint() 
{
	//paint header
	BaseClass::Paint();
}

void CHudBuildStateSentry::MsgFunc_SentryMsg(bf_read &msg)
{
    int iHealth = (int) msg.ReadByte();
    int iMaxHealth = (int) msg.ReadByte();
	int iLevel = (int) msg.ReadByte();

	// set icon according to SG level
	switch(iLevel)
	{
	case 2: // level 2
		SetHeaderIconChar("2");
		//set intensity for upgrade values
		m_qbCellCounter->SetIntensityControl(0, (int)(FF_BUILDCOST_UPGRADE_SENTRYGUN/3), (int)(FF_BUILDCOST_UPGRADE_SENTRYGUN/3) * 2,FF_BUILDCOST_UPGRADE_SENTRYGUN);
		m_qbCellCounter->SetAmountMax(FF_BUILDCOST_UPGRADE_SENTRYGUN);
		m_iMaxCells = FF_BUILDCOST_UPGRADE_SENTRYGUN;
		break;
	case 3: // level 3
		SetHeaderIconChar("3");
		//set intensity for building it again
		m_qbCellCounter->SetIntensityControl(0, (int)(FF_BUILDCOST_SENTRYGUN/3), (int)(FF_BUILDCOST_SENTRYGUN/3) * 2,FF_BUILDCOST_SENTRYGUN);
		m_qbCellCounter->SetAmountMax(FF_BUILDCOST_SENTRYGUN);
		m_iMaxCells = FF_BUILDCOST_SENTRYGUN;
		HideItem(m_qbCellCounter);
		break;
	case 0: // level 0 (not built)
		SetHeaderIconChar("1");
		//set intensity for build values
		m_qbCellCounter->SetIntensityControl(0, (int)(FF_BUILDCOST_SENTRYGUN/3), (int)(FF_BUILDCOST_SENTRYGUN/3) * 2,FF_BUILDCOST_SENTRYGUN);
		m_qbCellCounter->SetAmountMax(FF_BUILDCOST_SENTRYGUN);
		m_iMaxCells = FF_BUILDCOST_SENTRYGUN;
		ShowItem(m_qbCellCounter);
		break;
	default: // level 1 or unknown level
		SetHeaderIconChar("1");
		//set intensity for upgrade values
		m_qbCellCounter->SetIntensityControl(0, (int)(FF_BUILDCOST_UPGRADE_SENTRYGUN/3), (int)(FF_BUILDCOST_UPGRADE_SENTRYGUN/3) * 2,FF_BUILDCOST_UPGRADE_SENTRYGUN);
		m_qbCellCounter->SetAmountMax(FF_BUILDCOST_UPGRADE_SENTRYGUN);
		m_iMaxCells = FF_BUILDCOST_UPGRADE_SENTRYGUN;
		break;
	}

	m_qbSentryLevel->SetAmount(iLevel);
	m_qbSentryHealth->SetAmount((int)((float)iHealth/100 * iMaxHealth));
	m_qbSentryHealth->SetAmountMax(iMaxHealth);
}