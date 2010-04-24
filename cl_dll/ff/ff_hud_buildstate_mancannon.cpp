#include "cbase.h"
#include "ff_quantityhelper.h"
#include "ff_hud_buildstate_mancannon.h"

DECLARE_HUDELEMENT(CHudBuildStateManCannon);
DECLARE_HUD_MESSAGE(CHudBuildStateManCannon, ManCannonMsg);

CHudBuildStateManCannon::CHudBuildStateManCannon(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudBuildStateManCannon")
{
	SetParent(g_pClientMode->GetViewport());

	// Hide when player is dead and not scout
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_NOTSCOUT );

	SetUseToggleText(true);

	m_bBuilt = false;
	m_bBuilding = false;
}
	
CHudBuildStateManCannon::~CHudBuildStateManCannon() 
{
}

KeyValues* CHudBuildStateManCannon::GetDefaultStyleData()
{
	KeyValues *kvPreset = BaseClass::GetDefaultStyleData();

	kvPreset->SetInt("x", 580);
	kvPreset->SetInt("y", 330);
	kvPreset->SetInt("alignH", FFQuantityHelper::ALIGN_RIGHT);
	kvPreset->SetInt("alignV", FFQuantityHelper::ALIGN_TOP);

	return kvPreset;
}
	
void CHudBuildStateManCannon::VidInit()
{
	wchar_t *tempString = vgui::localize()->Find("#HudPanel_ManCannon");

	if (!tempString) 
		tempString = L"MANCANNON";
	else
		wcsupr(tempString);

	SetHeaderText(tempString);
	SetHeaderIconChar("6");

	m_wszNotBuiltText = vgui::localize()->Find("#HudPanel_NotBuilt"); 

	if (!m_wszNotBuiltText) 
		m_wszNotBuiltText = L"Not Built";
	SetText(m_wszNotBuiltText);

	m_wszBuildingText = vgui::localize()->Find("#HudPanel_Building"); 

	if (!m_wszBuildingText) 
		m_wszBuildingText = L"Building...";

	m_qiManCannonHealth->SetLabelText("#FF_ITEM_HEALTH");
	m_qiManCannonHealth->SetIconChar("a");
	m_qiManCannonHealth->ShowAmountMax(false);
	m_qiManCannonHealth->SetAmount(0);
	HideItem(m_qiManCannonHealth);

	SetToggleTextVisible(true);
}

void CHudBuildStateManCannon::Init() 
{
	ivgui()->AddTickSignal(GetVPanel(), 250); //only update 4 times a second
	HOOK_HUD_MESSAGE(CHudBuildStateManCannon, ManCannonMsg);

	m_qiManCannonHealth = AddItem("BuildStateManCannonHealth"); 

	AddPanelToHudOptions("ManCannon", "#HudPanel_ManCannon", "BuildState", "#HudPanel_BuildableState");
}

void CHudBuildStateManCannon::OnTick() 
{
	BaseClass::OnTick();

	if( !engine->IsInGame() | !ShouldDraw() )
		return;

	// Get the local player
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	bool bBuilt = pPlayer->GetManCannon() && pPlayer->GetManCannon()->IsBuilt();
	bool bBuilding = pPlayer->GetManCannon() && !bBuilt;

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
		HideItem(m_qiManCannonHealth);
		SetToggleTextVisible(true);
	}
	else if(bBuilt && !m_bBuilt)
	//show quantity bars
	{
		m_bBuilt = true;
		ShowItem(m_qiManCannonHealth);
		SetToggleTextVisible(false);
	}
}

void CHudBuildStateManCannon::Paint() 
{
	//paint header
	BaseClass::Paint();
}

void CHudBuildStateManCannon::MsgFunc_ManCannonMsg(bf_read &msg)
{
	int iHealth = (int) msg.ReadByte();

	m_qiManCannonHealth->SetAmount(iHealth);
}