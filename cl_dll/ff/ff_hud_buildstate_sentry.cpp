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

	m_iHideCells = IF_BUILT;
	m_iHideLevel = IF_BUILT;
	m_iShowPanel = ALWAYS;
}
	
CHudBuildStateSentry::~CHudBuildStateSentry() 
{
}

KeyValues* CHudBuildStateSentry::AddPanelSpecificOptions(KeyValues *kvPanelSpecificOptions)
{
	/*
	AddBooleanOption(kvPanelSpecificOptions, "DamageAlert", "Damage Alert", true);
	AddBooleanOption(kvPanelSpecificOptions, "HealAlert", "Heal Alert", true);
	*/
	
	char szBuffer[16];

	KeyValues* kvHideCells = new KeyValues("Values");
	Q_snprintf( szBuffer, sizeof( szBuffer ), "%i", NEVER );
	kvHideCells->SetString(szBuffer, "Never");
	Q_snprintf( szBuffer, sizeof( szBuffer ), "%i", ALWAYS );
	kvHideCells->SetString(szBuffer, "Always");
	Q_snprintf( szBuffer, sizeof( szBuffer ), "%i", IF_BUILT );
	kvHideCells->SetString(szBuffer, "If Built");
	AddComboOption(kvPanelSpecificOptions, "HideCells", "HideCells", kvHideCells, IF_BUILT);

	KeyValues* kvHideLevel = new KeyValues("Values");
	Q_snprintf( szBuffer, sizeof( szBuffer ), "%i", NEVER );
	kvHideLevel->SetString(szBuffer, "Never");
	Q_snprintf( szBuffer, sizeof( szBuffer ), "%i", ALWAYS );
	kvHideLevel->SetString(szBuffer, "Always");
	Q_snprintf( szBuffer, sizeof( szBuffer ), "%i", IF_BUILT );
	kvHideLevel->SetString(szBuffer, "If Built");
	AddComboOption(kvPanelSpecificOptions, "HideLevel", "HideLevel", kvHideLevel, IF_BUILT);

	KeyValues* kvShow = new KeyValues("Values");
	Q_snprintf( szBuffer, sizeof( szBuffer ), "%i", NEVER );
	kvShow->SetString(szBuffer, "Never");
	Q_snprintf( szBuffer, sizeof( szBuffer ), "%i", ALWAYS );
	kvShow->SetString(szBuffer, "Always");
	Q_snprintf( szBuffer, sizeof( szBuffer ), "%i", IF_BUILT );
	kvShow->SetString(szBuffer, "If Built");
	AddComboOption(kvPanelSpecificOptions, "ShowPanel", "ShowPanel", kvShow, ALWAYS);

	return kvPanelSpecificOptions;
}

KeyValues* CHudBuildStateSentry::GetDefaultStyleData()
{
	KeyValues *kvPreset = new KeyValues("StyleData");

	KeyValues *kvPanelSpecificValues = new KeyValues("PanelSpecificValues");
	kvPanelSpecificValues->SetInt("HideCells", IF_BUILT);
	kvPanelSpecificValues->SetInt("HideLevel", IF_BUILT);
	kvPanelSpecificValues->SetInt("ShowPanel", ALWAYS);
	kvPreset->AddSubKey(kvPanelSpecificValues);

	kvPreset->SetInt("x", 580);
	kvPreset->SetInt("y", 325);
	kvPreset->SetInt("alignH", 2);
	kvPreset->SetInt("alignV", 2);

	kvPreset->SetInt("showPanel", 1);
	kvPreset->SetInt("panelMargin", 5);
	kvPreset->SetInt("panelColorMode", FFQuantityPanel::COLOR_MODE_TEAMCOLORED);
	kvPreset->SetInt("panelRed", 255);
	kvPreset->SetInt("panelGreen", 255);
	kvPreset->SetInt("panelBlue", 255);
	kvPreset->SetInt("panelAlpha", 255);

	kvPreset->SetInt("showHeaderText", 1);
	kvPreset->SetInt("headerTextSize", 3);
	kvPreset->SetInt("headerTextShadow", 1);
	kvPreset->SetInt("headerTextAnchorPosition",  FFQuantityPanel::ANCHORPOS_TOPLEFT);
	kvPreset->SetInt("headerTextAlignHoriz",  FFQuantityPanel::ALIGN_LEFT);
	kvPreset->SetInt("headerTextAlignVert",  FFQuantityPanel::ALIGN_BOTTOM);
	kvPreset->SetInt("headerTextX", 16);
	kvPreset->SetInt("headerTextY", -2);
	kvPreset->SetInt("headerTextColorMode", FFQuantityPanel::COLOR_MODE_CUSTOM);
	kvPreset->SetInt("headerTextRed", 255);
	kvPreset->SetInt("headerTextGreen", 255);
	kvPreset->SetInt("headerTextBlue", 255);
	kvPreset->SetInt("headerTextAlpha", 255);

	kvPreset->SetInt("showHeaderIcon", 1);
	kvPreset->SetInt("headerIconSize", 1);
	kvPreset->SetInt("headerIconShadow", 1);
	kvPreset->SetInt("headerIconAnchorPosition",  FFQuantityPanel::ANCHORPOS_TOPLEFT);
	kvPreset->SetInt("headerIconAlignHoriz",  FFQuantityPanel::ALIGN_LEFT);
	kvPreset->SetInt("headerIconAlignVert",  FFQuantityPanel::ALIGN_BOTTOM);
	kvPreset->SetInt("headerIconX", 0);
	kvPreset->SetInt("headerIconY", 0);
	kvPreset->SetInt("headerIconColorMode", FFQuantityPanel::COLOR_MODE_CUSTOM);
	kvPreset->SetInt("headerIconRed", 255);
	kvPreset->SetInt("headerIconGreen", 255);
	kvPreset->SetInt("headerIconBlue", 255);
	kvPreset->SetInt("headerIconAlpha", 255);

	kvPreset->SetInt("showText", 1);
	kvPreset->SetInt("textSize", 3);
	kvPreset->SetInt("textShadow", 1);
	kvPreset->SetInt("textAnchorPosition",  FFQuantityPanel::ANCHORPOS_TOPLEFT);
	kvPreset->SetInt("textAlignHoriz",  FFQuantityPanel::ALIGN_LEFT);
	kvPreset->SetInt("textAlignVert",  FFQuantityPanel::ALIGN_TOP);
	kvPreset->SetInt("textX", 5);
	kvPreset->SetInt("textY", 5);
	kvPreset->SetInt("textColorMode", FFQuantityPanel::COLOR_MODE_CUSTOM);
	kvPreset->SetInt("textRed", 255);
	kvPreset->SetInt("textGreen", 255);
	kvPreset->SetInt("textBlue", 255);
	kvPreset->SetInt("textAlpha", 255);

	kvPreset->SetInt("barOrientation", FFQuantityItem::ORIENTATION_HORIZONTAL);
	kvPreset->SetInt("barWidth", 70);
	kvPreset->SetInt("barHeight", 7);
	kvPreset->SetInt("barBorderWidth", 1);
	kvPreset->SetInt("itemColumns", 1);
	kvPreset->SetInt("itemMarginHorizontal", 0);
	kvPreset->SetInt("itemMarginVertical", 0);

	KeyValues *kvComponent = new KeyValues("Bar");
	kvComponent->SetInt("show", 1);
	kvComponent->SetInt("colorMode", FFQuantityItem::ITEM_COLOR_MODE_FADED);
	kvComponent->SetInt("red", 255);
	kvComponent->SetInt("green", 255);
	kvComponent->SetInt("blue", 255);
	kvComponent->SetInt("alpha", 130);

	kvPreset->AddSubKey(kvComponent);

	kvComponent = new KeyValues("BarBorder");
	kvComponent->SetInt("show", 0);
	kvComponent->SetInt("colorMode", FFQuantityItem::ITEM_COLOR_MODE_CUSTOM);
	kvComponent->SetInt("red", 255);
	kvComponent->SetInt("green", 255);
	kvComponent->SetInt("blue", 255);
	kvComponent->SetInt("alpha", 155);

	kvPreset->AddSubKey(kvComponent);

	kvComponent = new KeyValues("BarBackground");
	kvComponent->SetInt("show", 1);
	kvComponent->SetInt("colorMode", FFQuantityItem::ITEM_COLOR_MODE_FADED);
	kvComponent->SetInt("red", 255);
	kvComponent->SetInt("green", 255);
	kvComponent->SetInt("blue", 255);
	kvComponent->SetInt("alpha", 40);

	kvPreset->AddSubKey(kvComponent);

	kvComponent = new KeyValues("Icon");
	kvComponent->SetInt("show", 0);
	kvComponent->SetInt("colorMode", FFQuantityItem::ITEM_COLOR_MODE_CUSTOM);
	kvComponent->SetInt("red", 255);
	kvComponent->SetInt("green", 255);
	kvComponent->SetInt("blue", 255);
	kvComponent->SetInt("alpha", 255);
	kvComponent->SetInt("shadow", 1);
	kvComponent->SetInt("size", 2);
	kvComponent->SetInt("anchorPosition",  FFQuantityItem::ANCHORPOS_MIDDLELEFT);
	kvComponent->SetInt("alignH", FFQuantityItem::ALIGN_RIGHT);
	kvComponent->SetInt("alignV", FFQuantityItem::ALIGN_MIDDLE);
	kvComponent->SetInt("offsetX", -2);
	kvComponent->SetInt("offsetY", 0);

	kvPreset->AddSubKey(kvComponent);

	kvComponent = new KeyValues("Label");
	kvComponent->SetInt("show", 1);
	kvComponent->SetInt("colorMode", FFQuantityItem::ITEM_COLOR_MODE_CUSTOM);
	kvComponent->SetInt("red", 255);
	kvComponent->SetInt("green", 255);
	kvComponent->SetInt("blue", 255);
	kvComponent->SetInt("alpha", 255);
	kvComponent->SetInt("fontTahoma", 1);
	kvComponent->SetInt("shadow", 1);
	kvComponent->SetInt("size", 1);
	kvComponent->SetInt("anchorPosition",  FFQuantityItem::ANCHORPOS_TOPLEFT);
	kvComponent->SetInt("alignH", FFQuantityItem::ALIGN_LEFT);
	kvComponent->SetInt("alignV", FFQuantityItem::ALIGN_BOTTOM);
	kvComponent->SetInt("offsetX", 0);
	kvComponent->SetInt("offsetY", 0);

	kvPreset->AddSubKey(kvComponent);

	kvComponent = new KeyValues("Amount");
	kvComponent->SetInt("show", 1);
	kvComponent->SetInt("colorMode", FFQuantityItem::ITEM_COLOR_MODE_CUSTOM);
	kvComponent->SetInt("red", 255);
	kvComponent->SetInt("green", 255);
	kvComponent->SetInt("blue", 255);
	kvComponent->SetInt("alpha", 255);
	kvComponent->SetInt("fontTahoma", 1);
	kvComponent->SetInt("shadow", 1);
	kvComponent->SetInt("size", 1);
	kvComponent->SetInt("anchorPosition",  FFQuantityItem::ANCHORPOS_TOPRIGHT);
	kvComponent->SetInt("alignH", FFQuantityItem::ALIGN_RIGHT);
	kvComponent->SetInt("alignV", FFQuantityItem::ALIGN_BOTTOM);
	kvComponent->SetInt("offsetX", 0);
	kvComponent->SetInt("offsetY", 0);

	kvPreset->AddSubKey(kvComponent);

	return kvPreset;
}


//we override this from the base class so we can catch the panel specific option values
void CHudBuildStateSentry::OnStyleDataRecieved( KeyValues *kvStyleData )
{
	KeyValues *kvPanelSpecificValues = kvStyleData->FindKey("PanelSpecificValues");
	
	if(kvPanelSpecificValues)
	{
		int iHideCells = kvPanelSpecificValues->GetInt("HideCells", IF_BUILT);
		int iHideLevel = kvPanelSpecificValues->GetInt("HideLevel", IF_BUILT);
		int iShowPanel = kvPanelSpecificValues->GetInt("ShowPanel", ALWAYS);

		if(m_iShowPanel != iShowPanel)
		{
			m_iShowPanel = iShowPanel;
			if(m_iShowPanel == NEVER)
			{
				SetHiddenBits( HIDEHUD_ALWAYS );
			}
			else if(m_iShowPanel == ALWAYS)
			{
				SetHiddenBits(  HIDEHUD_PLAYERDEAD | HIDEHUD_NOTENGINEER );
			}
			else if(m_iShowPanel == IF_BUILT)
			{
				//if
				if(m_bBuilt)
				{
					SetHiddenBits(  HIDEHUD_PLAYERDEAD | HIDEHUD_NOTENGINEER );
				}
				else
				{
					SetHiddenBits(  HIDEHUD_ALWAYS  );
				}
			}
		}
		
		if(m_iHideCells != iHideCells)
		{
			m_iHideCells = iHideCells;
			if(m_iHideCells == NEVER)
			{
				EnableItem(m_qiCellCounter);
			}
			else if(m_iHideCells == ALWAYS)
			{
				DisableItem(m_qiCellCounter);
			}
			else if(m_iHideCells == IF_BUILT)
			{
				//if sg is level 3
				if(m_qiSentryLevel->GetAmount() == 3)
				{
					DisableItem(m_qiCellCounter);
				}
				else
				{
					EnableItem(m_qiCellCounter);
				}
			}
		}
		if(m_iHideLevel != iHideLevel)
		{
			m_iHideLevel = iHideLevel;
			if(m_iHideLevel == NEVER)
			{
				EnableItem(m_qiSentryLevel);
			}
			else if(m_iHideLevel == ALWAYS)
			{
				DisableItem(m_qiSentryLevel);
			}
			else if(m_iHideLevel == IF_BUILT)
			{
				//if sg is level 3
				if(m_qiSentryLevel->GetAmount() == 3)
				{
					DisableItem(m_qiSentryLevel);
				}
				else
				{
					EnableItem(m_qiSentryLevel);
				}
			}
		}
	}

	BaseClass::OnStyleDataRecieved(kvStyleData);	
}

void CHudBuildStateSentry::VidInit()
{
	wchar_t *tempString = vgui::localize()->Find("#HudPanel_SentryGun");

	wcsupr(tempString);

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

	m_qiSentryHealth->SetLabelText("#FF_ITEM_HEALTH", false);
	m_qiSentryHealth->SetIconChar("a", false);
	m_qiSentryHealth->SetIntensityAmountScaled(true);//max changes (is not 100) so we need to scale to a percentage amount for calculation
	m_qiSentryHealth->SetAmount(0);
	HideItem(m_qiSentryHealth);

	m_qiSentryLevel->SetLabelText("#FF_ITEM_LEVEL", false);
	m_qiSentryLevel->SetIconChar("d", false);
	m_qiSentryLevel->SetAmountMax(3);
	m_qiSentryLevel->SetIntensityControl(1,2,2,3);
	m_qiSentryLevel->SetIntensityValuesFixed(true);
	m_qiSentryLevel->SetAmount(0);
	HideItem(m_qiSentryLevel);

	m_qiCellCounter->SetLabelText("#FF_ITEM_CELLS", false);
	m_qiCellCounter->SetIconChar("p", false);
	m_qiCellCounter->SetIntensityControl(0, (int)(FF_BUILDCOST_SENTRYGUN/3), (int)(FF_BUILDCOST_SENTRYGUN/3) * 2, FF_BUILDCOST_SENTRYGUN);
	m_qiCellCounter->SetAmountMax(FF_BUILDCOST_SENTRYGUN);
	m_iMaxCells = FF_BUILDCOST_SENTRYGUN;

	SetToggleTextVisible(true);
}

void CHudBuildStateSentry::Init() 
{
	ivgui()->AddTickSignal(GetVPanel(), 250); //only update 4 times a second
	HOOK_HUD_MESSAGE(CHudBuildStateSentry, SentryMsg);

	m_qiSentryHealth = AddItem("BuildStateSentryHealth"); 
	m_qiSentryLevel = AddItem("BuildStateSentryLevel"); 
	m_qiCellCounter = AddItem("BuildStateCellCounter"); 

	AddPanelToHudOptions("SentryGun", "#HudPanel_SentryGun", "BuildState", "#HudPanel_BuildableState");
}

void CHudBuildStateSentry::OnTick() 
{
	VPROF_BUDGET("CHudBuildStateSentry::OnTick", "QuantityBars");
	BaseClass::OnTick();

	// Get the local player
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if( !engine->IsInGame() || pPlayer->GetClassSlot() != CLASS_ENGINEER )
		return;

	// Never below zero (dunno why this is here)
	int iCells = max( pPlayer->GetAmmoCount( AMMO_CELLS ), 0);
	iCells = min(iCells, m_iMaxCells);
	// Only update if we've changed cell count
	if ( iCells != m_qiCellCounter->GetAmount() )
		m_qiCellCounter->SetAmount(iCells);

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
		HideItem(m_qiSentryHealth);
		HideItem(m_qiSentryLevel);
		if(m_iHideCells != ALWAYS)
		{
			EnableItem(m_qiCellCounter);
		}
		if(m_iHideLevel != ALWAYS)
		{
			EnableItem(m_qiSentryLevel);
		}
		SetToggleTextVisible(true);

		if(m_iShowPanel == IF_BUILT)
		{
			SetHiddenBits( HIDEHUD_ALWAYS );
		}
	}
	else if(bBuilt && !m_bBuilt)
	//show quantity bars
	{
		m_bBuilt = true;
		ShowItem(m_qiSentryHealth);
		ShowItem(m_qiSentryLevel);
		SetToggleTextVisible(false);

		if(m_iShowPanel != NEVER)
		{
			SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_NOTENGINEER );
		}
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
		m_qiCellCounter->SetIntensityControl(0, (int)(FF_BUILDCOST_UPGRADE_SENTRYGUN/3), (int)(FF_BUILDCOST_UPGRADE_SENTRYGUN/3) * 2,FF_BUILDCOST_UPGRADE_SENTRYGUN);
		m_qiCellCounter->SetAmountMax(FF_BUILDCOST_UPGRADE_SENTRYGUN);
		m_iMaxCells = FF_BUILDCOST_UPGRADE_SENTRYGUN;
		break;
	case 3: // level 3
		SetHeaderIconChar("3");
		//set intensity for building it again
		m_qiCellCounter->SetIntensityControl(0, (int)(FF_BUILDCOST_SENTRYGUN/3), (int)(FF_BUILDCOST_SENTRYGUN/3) * 2,FF_BUILDCOST_SENTRYGUN);
		m_qiCellCounter->SetAmountMax(FF_BUILDCOST_SENTRYGUN);
		m_iMaxCells = FF_BUILDCOST_SENTRYGUN;
		if(m_iHideCells != NEVER)
		{
			DisableItem(m_qiCellCounter);
		}
		if(m_iHideLevel != NEVER)
		{
			DisableItem(m_qiSentryLevel);
		}
		break;
	case 0: // level 0 (not built)
		SetHeaderIconChar("1");
		//set intensity for build values
		m_qiCellCounter->SetIntensityControl(0, (int)(FF_BUILDCOST_SENTRYGUN/3), (int)(FF_BUILDCOST_SENTRYGUN/3) * 2,FF_BUILDCOST_SENTRYGUN);
		m_qiCellCounter->SetAmountMax(FF_BUILDCOST_SENTRYGUN);
		m_iMaxCells = FF_BUILDCOST_SENTRYGUN;
		if(m_iHideCells != ALWAYS)
		{
			EnableItem(m_qiCellCounter);
		}
		if(m_iHideLevel != ALWAYS)
		{
			EnableItem(m_qiSentryLevel);
		}
		break;
	default: // level 1 or unknown level
		SetHeaderIconChar("1");
		//set intensity for upgrade values
		m_qiCellCounter->SetIntensityControl(0, (int)(FF_BUILDCOST_UPGRADE_SENTRYGUN/3), (int)(FF_BUILDCOST_UPGRADE_SENTRYGUN/3) * 2,FF_BUILDCOST_UPGRADE_SENTRYGUN);
		m_qiCellCounter->SetAmountMax(FF_BUILDCOST_UPGRADE_SENTRYGUN);
		m_iMaxCells = FF_BUILDCOST_UPGRADE_SENTRYGUN;
		break;
	}

	m_qiSentryLevel->SetAmount(iLevel);
	m_qiSentryHealth->SetAmount((int)((float)iHealth/100 * iMaxHealth));
	m_qiSentryHealth->SetAmountMax(iMaxHealth);
}