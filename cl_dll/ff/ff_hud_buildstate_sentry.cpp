#include "cbase.h"
#include "ff_hud_buildstate_sentry.h"

CHudBuildStateSentry::CHudBuildStateSentry(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudBuildStateSentry")
{
	SetParent(g_pClientMode->GetViewport());

	// Hide when player is dead
	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	m_bBuilt = false;
}

CHudBuildStateSentry::~CHudBuildStateSentry() 
{
}

void CHudBuildStateSentry::VidInit()
{
	wchar_t *tempString = vgui::localize()->Find("#FF_PLAYER_SENTRYGUN");

	if (!tempString) 
		tempString = L"HEALTH";

	SetHeaderText(tempString);
	SetHeaderIconChar("R");
	
	m_qbSentryHealth->SetLabelText("#FF_ITEM_HEALTH");
	m_qbSentryHealth->SetIconChar(":");
	m_qbSentryHealth->SetIntensityAmountScaled(true);//max changes (is not 100) so we need to scale to a percentage amount for calculation

	m_qbSentryLevel->SetLabelText("#FF_ITEM_LEVEL");
	m_qbSentryLevel->SetAmountMax(3);
	m_qbSentryLevel->SetIntensityControl(1,2,2,3);
	m_qbSentryLevel->SetIntensityValuesFixed(true);
}

void CHudBuildStateSentry::Init() 
{
	ivgui()->AddTickSignal(GetVPanel(), 500); //only update 2 times a second
	HOOK_HUD_MESSAGE(CHudBuildStateSentry, SentryMsg);

	m_qbSentryHealth = AddChild("BuildStateSentryHealth"); 
	m_qbSentryLevel = AddChild("BuildStateSentryLevel"); 
}

void CHudBuildStateSentry::OnTick() 
{
	BaseClass::OnTick();

	CheckCvars();

	if (!engine->IsInGame()) 
		return;

	// Get the local player
	C_FFPlayer *pPlayer = ToFFPlayer(C_BasePlayer::GetLocalPlayer());

	// If the player is not an FFPlayer or is not an Engineer
	if (!pPlayer || pPlayer->GetClassSlot() != CLASS_ENGINEER)
	//hide the panel
	{
		m_bDraw = false;
		SetBarsVisible(false);
		return; //return and don't continue
	}
	else
	//show the panel
	{
		m_bDraw = true;
	}
	
	m_bBuilt = pPlayer->GetSentryGun();
	
	//if not built
	if(!m_bBuilt)
	//hide quantity bars
	{
		SetBarsVisible(false);
	}
	else
	//show quantity bars
	{
		SetBarsVisible(true);
	}
}

void CHudBuildStateSentry::Paint() 
{
	if(!m_bDraw)
		return;

	wchar_t* pText;

	if(!m_bBuilt)
	//if not built
	{
		//paint "Not Built" message
		//LOCALISE THIS
		pText = L"Not Built";	// wide char text
		surface()->DrawSetTextFont( m_hfText ); // set the font	
		surface()->DrawSetTextColor( m_ColorText );
		surface()->DrawSetTextPos( (m_qb_iPositionX + m_qb_iBarMarginHorizontal) * m_flScale, (m_qb_iPositionY + m_qb_iBarMarginVertical) * m_flScale ); // x,y position
		surface()->DrawPrintText( pText, wcslen(pText) ); // print text
	}
	
	//paint header
	BaseClass::Paint();
}

void CHudBuildStateSentry::CheckCvars()
{
	bool updateBarPositions = false;
	
	m_bChildOverride = hud_buildstate_sg_override.GetBool();

	if(m_iX != hud_buildstate_sg_x.GetInt() || m_iY != hud_buildstate_sg_y.GetInt())
	{
		m_iX = hud_buildstate_sg_x.GetInt();
		m_iY = hud_buildstate_sg_y.GetInt();
	}

	if(m_iHorizontalAlign != hud_buildstate_sg_align_horiz.GetInt())
		m_iHorizontalAlign = hud_buildstate_sg_align_horiz.GetInt();

	if(m_iVerticalAlign != hud_buildstate_sg_align_vert.GetInt())
		m_iVerticalAlign = hud_buildstate_sg_align_vert.GetInt();

	if(m_qb_iColumns != hud_buildstate_sg_columns.GetInt())
	{
		m_qb_iColumns = hud_buildstate_sg_columns.GetInt();
		updateBarPositions = true;
	}

	BaseClass::CheckCvars(updateBarPositions);
}

void CHudBuildStateSentry::MsgFunc_SentryMsg(bf_read &msg)
{
    int iHealth = (int) msg.ReadByte();
    int iMaxHealth = (int) msg.ReadByte();
	int iLevel = (int) msg.ReadByte();

	m_qbSentryLevel->SetAmount(iLevel);
	m_qbSentryHealth->SetAmount((int)((float)iHealth/100 * iMaxHealth));
	m_qbSentryHealth->SetAmountMax(iMaxHealth);
}