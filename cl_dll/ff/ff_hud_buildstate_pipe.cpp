#include "cbase.h"
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
	KeyValues *kvPreset = new KeyValues("StyleData");

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
	
void CHudBuildStatePipe::VidInit()
{
	wchar_t *tempString = vgui::localize()->Find("#HudPanel_PipeTrap");

	wcsupr(tempString);

	if (!tempString) 
		tempString = L"PIPE";

	SetHeaderText(tempString, false);
	SetHeaderIconChar("o", false);

	m_wszNotBuiltText = vgui::localize()->Find("#HudPanel_NotBuilt"); 

	if (!m_wszNotBuiltText) 
		m_wszNotBuiltText = L"Not Built";
	SetText(m_wszNotBuiltText, false);

	m_qiPipeLaid->SetLabelText("#FF_ITEM_PIPES", false);
	m_qiPipeLaid->SetIconChar("o", false);
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