#include "cbase.h"
#include "ff_quantityhelper.h"
#include "ff_hud_buildstate_detpack.h"

DECLARE_HUDELEMENT(CHudBuildStateDetpack);
DECLARE_HUD_MESSAGE(CHudBuildStateDetpack, DetpackMsg);

CHudBuildStateDetpack::CHudBuildStateDetpack(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudBuildStateDetpack")
{
	SetParent(g_pClientMode->GetViewport());

	// Hide when player is dead
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_NOTDEMOMAN );

	SetUseToggleText(true);

	m_bBuilt = false;
	m_bBuilding = false;
}
	
CHudBuildStateDetpack::~CHudBuildStateDetpack() 
{
}
	
KeyValues* CHudBuildStateDetpack::GetDefaultStyleData()
{
	KeyValues *kvPreset = BaseClass::GetDefaultStyleData();

	kvPreset->SetInt("x", 580);
	kvPreset->SetInt("y", 330);
	kvPreset->SetInt("alignH", FFQuantityHelper::ALIGN_RIGHT);
	kvPreset->SetInt("alignV", FFQuantityHelper::ALIGN_TOP);

	return kvPreset;
}
	
void CHudBuildStateDetpack::VidInit()
{
	wchar_t *tempString = vgui::localize()->Find("#HudPanel_Detpack");

	if (!tempString) 
		tempString = L"DETPACK";
	else
		wcsupr(tempString);

	SetHeaderText(tempString);
	SetHeaderIconChar("5");

	m_wszNotBuiltText = vgui::localize()->Find("#HudPanel_NotBuilt"); 

	if (!m_wszNotBuiltText) 
		m_wszNotBuiltText = L"Not Built";
	SetText(m_wszNotBuiltText);

	m_wszBuildingText = vgui::localize()->Find("#HudPanel_Building"); 

	if (!m_wszBuildingText) 
		m_wszBuildingText = L"Building...";

	m_qiDetpackTimeLeft->SetLabelText("#FF_ITEM_TIMELEFT");
	m_qiDetpackTimeLeft->SetIconChar("f");
	m_qiDetpackTimeLeft->SetIntensityAmountScaled(true);//max changes (is not 100) so we need to scale to a percentage amount for calculation
	m_qiDetpackTimeLeft->SetAmount(0);
	HideItem(m_qiDetpackTimeLeft);

	SetToggleTextVisible(true);
}

void CHudBuildStateDetpack::Init() 
{
	ivgui()->AddTickSignal(GetVPanel(), 250); //only update 4 times a second
	HOOK_HUD_MESSAGE(CHudBuildStateDetpack, DetpackMsg);

	m_qiDetpackTimeLeft = AddItem("HudBuildStateDetpackTimeLeft"); 

	AddPanelToHudOptions("Detpack", "#HudPanel_Detpack", "BuildState", "#HudPanel_BuildableState");
}

void CHudBuildStateDetpack::OnTick() 
{
	BaseClass::OnTick();

	if( !engine->IsInGame() | !ShouldDraw() )
		return;

	// Get the local player
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	bool bBuilt = pPlayer->GetDetpack() && pPlayer->GetDetpack()->IsBuilt();
	bool bBuilding = pPlayer->GetDetpack() && !bBuilt;

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
		HideItem(m_qiDetpackTimeLeft);
		SetToggleTextVisible(true);
	}
	else if(bBuilt && !m_bBuilt)
	//show quantity bars
	{
		m_bBuilt = true;
		ShowItem(m_qiDetpackTimeLeft);
		SetToggleTextVisible(false);
	}
}

void CHudBuildStateDetpack::Paint() 
{
	if(m_bBuilt)
	{
		float flCurTime = gpGlobals->curtime;
		int iDetpackTimeLeft = (int)(m_flDetonateTime - gpGlobals->curtime + 1);
		if(iDetpackTimeLeft < 0)
			m_qiDetpackTimeLeft->SetAmount(m_flDetonateTime - flCurTime);
		else
			m_qiDetpackTimeLeft->SetAmount(iDetpackTimeLeft);
	}
	//paint header
	BaseClass::Paint();
}

void CHudBuildStateDetpack::MsgFunc_DetpackMsg(bf_read &msg)
{
	m_flDetonateTime = msg.ReadFloat();
	m_qiDetpackTimeLeft->SetAmountMax((int) msg.ReadByte());
}