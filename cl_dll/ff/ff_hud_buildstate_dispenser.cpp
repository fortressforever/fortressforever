#include "cbase.h"
#include "ff_hud_buildstate_dispenser.h"

DECLARE_HUDELEMENT(CHudBuildStateDispenser);
DECLARE_HUD_MESSAGE(CHudBuildStateDispenser, DispenserMsg);

CHudBuildStateDispenser::CHudBuildStateDispenser(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudBuildStateSentry")
{
	SetParent(g_pClientMode->GetViewport());

	// Hide when player is dead
	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	m_bBuilt = false;
}

CHudBuildStateDispenser::~CHudBuildStateDispenser() 
{
}

KeyValues* CHudBuildStateDispenser::GetDefaultStyleData()
{
	KeyValues *kvPreset = new KeyValues("StyleData");

	kvPreset->SetInt("x", 640);
	kvPreset->SetInt("y", 300);
	kvPreset->SetInt("alignH", 2);
	kvPreset->SetInt("alignV", 0);
	kvPreset->SetInt("columns", 1);
	kvPreset->SetInt("itemsX", 5);
	kvPreset->SetInt("itemsY", 25);
	kvPreset->SetInt("showHeaderText", 1);
	kvPreset->SetInt("headerTextShadow", 0);
	kvPreset->SetInt("headerTextSize", 4);
	kvPreset->SetInt("headerTextX", 20);
	kvPreset->SetInt("headerTextY", 7);
	kvPreset->SetInt("showHeaderIcon", 1);
	kvPreset->SetInt("headerIconShadow", 0);
	kvPreset->SetInt("headerIconSize", 4);
	kvPreset->SetInt("headerIconX", 3);
	kvPreset->SetInt("headerIconY", 3);
	kvPreset->SetInt("showPanel", 1);
	kvPreset->SetInt("panelColorCustom", 0);
	kvPreset->SetInt("panelRed", 255);
	kvPreset->SetInt("panelGreen", 255);
	kvPreset->SetInt("panelBlue", 255);
	kvPreset->SetInt("panelAlpha", 255);

	kvPreset->SetInt("barWidth", 50);
	kvPreset->SetInt("barHeight", 8);
	kvPreset->SetInt("barBorderWidth", 1);
	kvPreset->SetInt("barMarginHorizontal", 2);
	kvPreset->SetInt("barMarginVertical", 2);
	kvPreset->SetInt("barOrientation", ORIENTATION_HORIZONTAL);

	KeyValues *kvComponent = new KeyValues("Bar");
	kvComponent->SetInt("show", 1);
	kvComponent->SetInt("colorMode", COLOR_MODE_FADED);
	kvComponent->SetInt("red", 255);
	kvComponent->SetInt("green", 255);
	kvComponent->SetInt("blue", 255);
	kvComponent->SetInt("alpha", 255);

	kvPreset->AddSubKey(kvComponent);

	kvComponent = new KeyValues("BarBorder");
	kvComponent->SetInt("show", 1);
	kvComponent->SetInt("colorMode", COLOR_MODE_CUSTOM);
	kvComponent->SetInt("red", 255);
	kvComponent->SetInt("green", 255);
	kvComponent->SetInt("blue", 255);
	kvComponent->SetInt("alpha", 255);

	kvPreset->AddSubKey(kvComponent);

	kvComponent = new KeyValues("BarBackground");
	kvComponent->SetInt("show", 1);
	kvComponent->SetInt("colorMode", COLOR_MODE_FADED);
	kvComponent->SetInt("red", 255);
	kvComponent->SetInt("green", 255);
	kvComponent->SetInt("blue", 255);
	kvComponent->SetInt("alpha", 160);

	kvPreset->AddSubKey(kvComponent);

	kvComponent = new KeyValues("Icon");
	kvComponent->SetInt("show", 1);
	kvComponent->SetInt("colorMode", COLOR_MODE_CUSTOM);
	kvComponent->SetInt("red", 255);
	kvComponent->SetInt("green", 255);
	kvComponent->SetInt("blue", 255);
	kvComponent->SetInt("alpha", 255);
	kvComponent->SetInt("shadow", 0);
	kvComponent->SetInt("size", 3);
	kvComponent->SetInt("alignH", ALIGN_RIGHT);
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
	kvComponent->SetInt("shadow", 0);
	kvComponent->SetInt("size", 3);
	kvComponent->SetInt("alignH", ALIGN_RIGHT);
	kvComponent->SetInt("alignV", ALIGN_TOP);
	kvComponent->SetInt("offsetX", -50);
	kvComponent->SetInt("offsetY", 0);

	kvPreset->AddSubKey(kvComponent);

	kvComponent = new KeyValues("Amount");
	kvComponent->SetInt("show", 1);
	kvComponent->SetInt("colorMode", COLOR_MODE_CUSTOM);
	kvComponent->SetInt("red", 255);
	kvComponent->SetInt("green", 255);
	kvComponent->SetInt("blue", 255);
	kvComponent->SetInt("alpha", 255);
	kvComponent->SetInt("shadow", 0);
	kvComponent->SetInt("size", 3);
	kvComponent->SetInt("alignH", ALIGN_RIGHT);
	kvComponent->SetInt("alignV", ALIGN_BOTTOM);
	kvComponent->SetInt("offsetX", -50);
	kvComponent->SetInt("offsetY", 2);

	kvPreset->AddSubKey(kvComponent);

	return kvPreset;
}

void CHudBuildStateDispenser::VidInit()
{
	wchar_t *tempString = vgui::localize()->Find("#FF_PLAYER_DISPENSER");

	if (!tempString) 
		tempString = L"DISPENSER";

	SetHeaderText(tempString);
	SetHeaderIconChar("Y");

	m_qbDispenserHealth->SetLabelText("#FF_ITEM_HEALTH");
	m_qbDispenserHealth->SetIconChar(":");
	m_qbDispenserHealth->ShowAmountMax(false);

	m_qbDispenserAmmo->SetLabelText("#FF_ITEM_AMMO");
	m_qbDispenserAmmo->SetIconChar("4");
	m_qbDispenserAmmo->ShowAmountMax(false);
}

void CHudBuildStateDispenser::Init() 
{
	ivgui()->AddTickSignal(GetVPanel(), 250); //only update 4 times a second
	HOOK_HUD_MESSAGE(CHudBuildStateDispenser, DispenserMsg);

	m_qbDispenserHealth = AddChild("BuildStateDispenserHealth"); 
	m_qbDispenserAmmo = AddChild("BuildStateDispenserAmmo"); 

	AddPanelToHudOptions("Dispenser", "#HudPanel_Dispenser", "BuildState", "#HudPanel_BuildableState");
}

void CHudBuildStateDispenser::OnTick() 
{
	BaseClass::OnTick();

	if (!engine->IsInGame()) 
		return;

	// Get the local player
	C_FFPlayer *pPlayer = ToFFPlayer(C_BasePlayer::GetLocalPlayer());

	// If the player is not an FFPlayer or is not an Engineer
	if (!pPlayer || pPlayer->GetClassSlot() != CLASS_ENGINEER)
	//hide the panel
	{
		m_bDraw = false;
		SetVisible(false);
		m_qbDispenserHealth->SetVisible(false);
		m_qbDispenserAmmo->SetVisible(false);
		return; //return and don't continue
	}
	else
	//show the panel
	{
		m_bDraw = true;
	}
	
	bool bBuilt = pPlayer->GetDispenser();
	
	//small optimisation by comparing build with what it was previously
	//if not built
	if(!bBuilt && m_bBuilt)
	//hide quantity bars
	{
		m_bBuilt = false;
		//give us some new amounts to that when it's building we have normal values rather than what was left!
		m_qbDispenserHealth->SetAmount(0);
		m_qbDispenserAmmo->SetAmount(0);
		SetVisible(false);
		m_qbDispenserHealth->SetVisible(false);
		m_qbDispenserAmmo->SetVisible(false);
	}
	else if(bBuilt && !m_bBuilt)
	//show quantity bars
	{
		m_bBuilt = true;
		SetVisible(true);
		m_qbDispenserHealth->SetVisible(true);
		m_qbDispenserAmmo->SetVisible(true);
	}
}

void CHudBuildStateDispenser::Paint() 
{
	if(m_bDraw)
	{
		wchar_t* pText;

		if(!m_bBuilt)
		//if not built
		{
			//paint "Not Built" message
			//LOCALISE THIS
			pText = L"Not Built";	// wide char text
			surface()->DrawSetTextFont( m_hfText ); // set the font	
			surface()->DrawSetTextColor( m_ColorText );
			surface()->DrawSetTextPos( (m_iItemPositionX + m_iItemMarginHorizontal) * m_flScale, (m_iItemPositionY + m_iItemMarginVertical) * m_flScale ); // x,y position
			surface()->DrawPrintText( pText, wcslen(pText) ); // print text
		}
		
		//paint header
		BaseClass::Paint();
	}
}

void CHudBuildStateDispenser::MsgFunc_DispenserMsg(bf_read &msg)
{
    int iHealth = (int) msg.ReadByte();
    int iAmmo = (int) msg.ReadByte();

	m_qbDispenserHealth->SetAmount(iHealth);
	m_qbDispenserAmmo->SetAmount(iAmmo);
}