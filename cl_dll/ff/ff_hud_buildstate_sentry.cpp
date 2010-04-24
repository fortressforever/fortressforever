#include "cbase.h"
#include "ff_quantityhelper.h"
#include "ff_hud_buildstate_sentry.h"
#include "ff_buildableobjects_shared.h"

#define HIDECELLS_NEVER 0
#define HIDECELLS_ALWAYS 1
#define HIDECELLS_IFBUILT 2

DECLARE_HUDELEMENT(CHudBuildStateSentry);

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
	
	AddBooleanOption(kvPanelSpecificOptions, "HealthPct", "Health %", false);
	
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
	KeyValues *kvPreset = BaseClass::GetDefaultStyleData();

	KeyValues *kvPanelSpecificValues = new KeyValues("PanelSpecificValues");
	kvPanelSpecificValues->SetInt("HideCells", IF_BUILT);
	kvPanelSpecificValues->SetInt("HideLevel", IF_BUILT);
	kvPanelSpecificValues->SetInt("ShowPanel", ALWAYS);
	kvPanelSpecificValues->SetInt("HealthPct", 0);
	kvPreset->AddSubKey(kvPanelSpecificValues);

	kvPreset->SetInt("x", 580);
	kvPreset->SetInt("y", 325);
	kvPreset->SetInt("alignH", FFQuantityHelper::ALIGN_RIGHT);
	kvPreset->SetInt("alignV", FFQuantityHelper::ALIGN_BOTTOM);

	return kvPreset;
}


//we override this from the base class so we can catch the panel specific option values
void CHudBuildStateSentry::OnStyleDataRecieved( KeyValues *kvStyleData )
{
	KeyValues *kvPanelSpecificValues = kvStyleData->FindKey("PanelSpecificValues");
	
	if(kvPanelSpecificValues)
	{
		int iHealthPct = kvPanelSpecificValues->GetInt("HealthPct", 0);
		int iHideCells = kvPanelSpecificValues->GetInt("HideCells", IF_BUILT);
		int iHideLevel = kvPanelSpecificValues->GetInt("HideLevel", IF_BUILT);
		int iShowPanel = kvPanelSpecificValues->GetInt("ShowPanel", ALWAYS);

		m_qiSentryHealth->ShowAmountMax(iHealthPct != 1);

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

	if (!tempString) 
		tempString = L"SENTRY";
	else 
		wcsupr(tempString);

	SetHeaderText(tempString);
	SetHeaderIconChar("1");

	m_wszNotBuiltText = vgui::localize()->Find("#HudPanel_NotBuilt"); 

	if (!m_wszNotBuiltText) 
		m_wszNotBuiltText = L"Not Built";
	SetText(m_wszNotBuiltText);

	m_wszBuildingText = vgui::localize()->Find("#HudPanel_Building"); 

	if (!m_wszBuildingText) 
		m_wszBuildingText = L"Building...";

	m_qiSentryHealth->SetLabelText("#FF_ITEM_HEALTH");
	m_qiSentryHealth->SetIconChar("a");
	m_qiSentryHealth->SetIntensityAmountScaled(true);//max changes (is not 100) so we need to scale to a percentage amount for calculation
	m_qiSentryHealth->SetAmount(0);
	DisableItem(m_qiSentryHealth);

	m_qiSentryLevel->SetLabelText("#FF_ITEM_LEVEL");
	m_qiSentryLevel->SetIconChar("d");
	m_qiSentryLevel->SetAmountMax(3);
	m_qiSentryLevel->SetIntensityControl(1,2,2,3);
	m_qiSentryLevel->SetIntensityValuesFixed(true);
	m_qiSentryLevel->SetAmount(0);

	m_qiCellCounter->SetLabelText("#FF_ITEM_CELLS");
	m_qiCellCounter->SetIconChar("p");
	m_qiCellCounter->SetIntensityControl(0, (int)(FF_BUILDCOST_SENTRYGUN/3), (int)(FF_BUILDCOST_SENTRYGUN/3) * 2, FF_BUILDCOST_SENTRYGUN);
	m_qiCellCounter->SetAmountMax(FF_BUILDCOST_SENTRYGUN);

	SetToggleTextVisible(true);
}

void CHudBuildStateSentry::Init() 
{
	ivgui()->AddTickSignal(GetVPanel(), 250); //only update 4 times a second

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

	C_FFSentryGun *pSentryGun =  pPlayer->GetSentryGun();
	bool bBuilt = pSentryGun && pSentryGun->IsBuilt();
	bool bBuilding = pSentryGun && !bBuilt;

	//small optimisation by comparing building with what it was previously
	//if building
	if(bBuilding && !m_bBuilding)
	//show building text
	{
		SetText(m_wszBuildingText, true);
		m_bBuilding = bBuilding;
	}
	//if not building
	else if(!bBuilding && m_bBuilding)
	//show not built text
	{
		SetText(m_wszNotBuiltText, true);
		m_bBuilding = bBuilding;
	}

	//small optimisation by comparing build with what it was previously
	//if not built
	if(!bBuilt && m_bBuilt)
	//hide quantity bars
	{
		m_bBuilt = false;
		DisableItem(m_qiSentryHealth);
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
		EnableItem(m_qiSentryHealth);
		if(m_iHideLevel != ALWAYS)
		{
			EnableItem(m_qiSentryLevel);
		}
		SetToggleTextVisible(false);

		if(m_iShowPanel != NEVER)
		{
			SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_NOTENGINEER );
		}
	}

	int iMaxCells = 0;

	if (bBuilt) 
	{
		int iMaxHealth = pSentryGun->GetMaxHealth();
		int iHealth = pSentryGun->GetHealth();
		m_qiSentryHealth->SetAmount(iHealth);
		m_qiSentryHealth->SetAmountMax(iMaxHealth);
	}

	int iLevel = bBuilt ? pSentryGun->GetLevel() : 0;

	// set icon according to SG level
	switch(iLevel)
	{
	case 1:
		SetHeaderIconChar("1", true);
		//set upgrade values
		m_qiCellCounter->SetIntensityControl(0, (int)(FF_BUILDCOST_UPGRADE_SENTRYGUN/3), (int)(FF_BUILDCOST_UPGRADE_SENTRYGUN/3) * 2, FF_BUILDCOST_UPGRADE_SENTRYGUN);
		m_qiCellCounter->SetAmountMax(FF_BUILDCOST_UPGRADE_SENTRYGUN);

		iMaxCells = FF_BUILDCOST_UPGRADE_SENTRYGUN;
		break;
	case 2:
		SetHeaderIconChar("2", true);
		//set upgrade values
		m_qiCellCounter->SetIntensityControl(0, (int)(FF_BUILDCOST_UPGRADE_SENTRYGUN/3), (int)(FF_BUILDCOST_UPGRADE_SENTRYGUN/3) * 2, FF_BUILDCOST_UPGRADE_SENTRYGUN);
		m_qiCellCounter->SetAmountMax(FF_BUILDCOST_UPGRADE_SENTRYGUN);

		iMaxCells = FF_BUILDCOST_UPGRADE_SENTRYGUN;
		break;
	case 3:
		SetHeaderIconChar("3", true);
		//set new build values
		m_qiCellCounter->SetIntensityControl(0, (int)(FF_BUILDCOST_SENTRYGUN/3), (int)(FF_BUILDCOST_SENTRYGUN/3) * 2, FF_BUILDCOST_SENTRYGUN);
		m_qiCellCounter->SetAmountMax(FF_BUILDCOST_SENTRYGUN);

		if(m_iHideCells != NEVER)
		{
			DisableItem(m_qiCellCounter);
		}

		if(m_iHideLevel != NEVER)
		{
			DisableItem(m_qiSentryLevel);
		}

		iMaxCells = FF_BUILDCOST_SENTRYGUN;
		break;
	default:
	case 0:
		SetHeaderIconChar("1", true);
		//set new build values
		m_qiCellCounter->SetIntensityControl(0, (int)(FF_BUILDCOST_SENTRYGUN/3), (int)(FF_BUILDCOST_SENTRYGUN/3) * 2, FF_BUILDCOST_SENTRYGUN);
		m_qiCellCounter->SetAmountMax(FF_BUILDCOST_SENTRYGUN);

		iMaxCells = FF_BUILDCOST_SENTRYGUN;
		break;
	}

	
	if(m_iHideCells != ALWAYS)
	{
		int iCells = pPlayer->GetAmmoCount( AMMO_CELLS );
		iCells = min(iCells, iMaxCells);
		m_qiCellCounter->SetAmount(iCells);
	}

	m_qiSentryLevel->SetAmount(iLevel);
}