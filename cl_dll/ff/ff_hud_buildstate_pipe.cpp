#include "cbase.h"
#include "ff_quantityhelper.h"
#include "ff_hud_buildstate_pipe.h"

enum {
	RESET_PIPES=0,
	INCREMENT_PIPES,
	DECREMENT_PIPES
};

DECLARE_HUDELEMENT(CHudBuildStatePipe);
DECLARE_HUD_MESSAGE(CHudBuildStatePipe, PipeMsg);

CHudBuildStatePipe::CHudBuildStatePipe(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudBuildStatePipe")
{
	SetParent(g_pClientMode->GetViewport());

	// Hide when player is dead
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_NOTDEMOMAN );

	SetUseToggleText(true);

	m_bBuilt = false;
	m_iNumPipes = 0;
}
	
CHudBuildStatePipe::~CHudBuildStatePipe() 
{
}
	
KeyValues* CHudBuildStatePipe::GetDefaultStyleData()
{
	KeyValues *kvPreset = BaseClass::GetDefaultStyleData();

	kvPreset->SetInt("x", 580);
	kvPreset->SetInt("y", 325);
	kvPreset->SetInt("alignH", FFQuantityHelper::ALIGN_RIGHT);
	kvPreset->SetInt("alignV", FFQuantityHelper::ALIGN_BOTTOM);

	return kvPreset;
}
	
void CHudBuildStatePipe::VidInit()
{
	wchar_t *tempString = vgui::localize()->Find("#HudPanel_PipeTrap");

	if (!tempString) 
		tempString = L"PIPE";
	else
		wcsupr(tempString);

	SetHeaderText(tempString);
	SetHeaderIconChar("o");

	m_wszNotBuiltText = vgui::localize()->Find("#HudPanel_NotBuilt"); 

	if (!m_wszNotBuiltText) 
		m_wszNotBuiltText = L"Not Built";
	SetText(m_wszNotBuiltText);

	m_qiPipeLaid->SetLabelText("#FF_ITEM_PIPES");
	m_qiPipeLaid->SetIconChar("o");
	m_qiPipeLaid->SetIntensityAmountScaled(true);//max changes (is not 100) so we need to scale to a percentage amount for calculation
	m_qiPipeLaid->SetAmount(0);
	m_qiPipeLaid->SetAmountMax(8);
	HideItem(m_qiPipeLaid);

	SetToggleTextVisible(true);
}

void CHudBuildStatePipe::Init() 
{
	ivgui()->AddTickSignal(GetVPanel(), 250); //only update 4 times a second
	HOOK_HUD_MESSAGE(CHudBuildStatePipe, PipeMsg);

	m_qiPipeLaid = AddItem("HudBuildStatePipeLaid"); 

	AddPanelToHudOptions("Pipe Trap", "#HudPanel_PipeTrap", "BuildState", "#HudPanel_BuildableState");
}

void CHudBuildStatePipe::OnTick() 
{
	BaseClass::OnTick();

	if( !engine->IsInGame() | !ShouldDraw() )
		return;
 
   	bool bBuilt = m_iNumPipes > 0;

	//small optimisation by comparing build with what it was previously
	//if not built
	if(!bBuilt && m_bBuilt)
	//hide quantity bars
	{
		m_bBuilt = false;
		HideItem(m_qiPipeLaid);
		SetToggleTextVisible(true);
	}
	else if(bBuilt && !m_bBuilt)
	//show quantity bars
	{
		m_bBuilt = true;
		ShowItem(m_qiPipeLaid);
		SetToggleTextVisible(false);
	}
}

void CHudBuildStatePipe::Paint() 
{
	//paint header
	BaseClass::Paint();
}

void CHudBuildStatePipe::MsgFunc_PipeMsg(bf_read &msg)
{
	int iIncrementPipes = (int) msg.ReadByte();
	switch (iIncrementPipes)
	{
	case INCREMENT_PIPES:
		m_iNumPipes++;
		break;
	case DECREMENT_PIPES:
		m_iNumPipes--;
		break;
	case RESET_PIPES:
	default:
		m_iNumPipes = 0;
		break;
	}
	m_iNumPipes = max(0, m_iNumPipes);
	m_qiPipeLaid->SetAmount(m_iNumPipes);
}