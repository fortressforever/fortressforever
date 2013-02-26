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
	KeyValues *kvPreset = BaseClass::GetDefaultStyleData();

	KeyValues *kvPanelSpecificValues = new KeyValues("PanelSpecificValues");
	kvPanelSpecificValues->SetInt("HideCells", IF_BUILT);
	kvPanelSpecificValues->SetInt("HideLevel", IF_BUILT);
	kvPanelSpecificValues->SetInt("ShowPanel", ALWAYS);
	kvPreset->AddSubKey(kvPanelSpecificValues);

	kvPreset->SetInt("x", 580);
	kvPreset->SetInt("y", 325);
	kvPreset->SetInt("alignH", FFQuantityPanel::ALIGN_RIGHT);
	kvPreset->SetInt("alignV", FFQuantityPanel::ALIGN_BOTTOM);

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