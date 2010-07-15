#include "cbase.h"
#include "ff_quantitypanel.h"

#include "c_ff_player.h" //required to cast base player

#include "iclientmode.h" //to set panel parent as the cliends viewport

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CHudBuildStateSentry : public CHudElement, public vgui::FFQuantityPanel
{
	DECLARE_CLASS_SIMPLE( CHudBuildStateSentry, vgui::FFQuantityPanel );

public:
	CHudBuildStateSentry(const char *pElementName) : CHudElement(pElementName), vgui::FFQuantityPanel(NULL, "HudBuildStateSentry")
	{
		SetParent(g_pClientMode->GetViewport());
		SetHiddenBits( 0 );
		m_bBuilt = false;
		m_flScale = 1.0f;
		vgui::ivgui()->AddTickSignal(GetVPanel(), 250); //only update 4 times a second
	}

	~CHudBuildStateSentry( void ) {}

	virtual void	Init( void );
	virtual void	OnTick( void );
	virtual void	Paint( void );
	virtual void	ApplySchemeSettings( vgui::IScheme *pScheme );

	void	MsgFunc_SentryMsg(bf_read &msg);

private:
	// could probably do this without these now
	// but would need an alternative of selecting the one you want easily
	CHudQuantityBar *m_qbSentryHealth;
	CHudQuantityBar *m_qbSentryLevel;

	bool	m_bBuilt;
	vgui::HFont m_hfText;
};

DECLARE_HUDELEMENT(CHudBuildStateSentry);
DECLARE_HUD_MESSAGE(CHudBuildStateSentry, SentryMsg);

void CHudBuildStateSentry::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	m_hfText = pScheme->GetFont( "QuantityPanel", IsProportional() );
	
	UpdateQuantityBarPositions();
	FFQuantityPanel::ApplySchemeSettings( pScheme );
}

void CHudBuildStateSentry::Init() 
{
	HOOK_HUD_MESSAGE(CHudBuildStateSentry, SentryMsg);

	wchar_t *tempString = vgui::localize()->Find("#FF_PLAYER_SENTRYGUN");

	if (!tempString) 
		tempString = L"HEALTH";

	m_qbSentryHealth = AddChild("BuildStateSentryHealth"); 
	m_qbSentryLevel = AddChild("BuildStateSentryLevel"); 

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);
	SetPaintEnabled(false);

	SetHeaderText(tempString);
	SetHeaderIconChar("R");
	
	m_qbSentryHealth->SetLabelText("#FF_ITEM_HEALTH");
	m_qbSentryHealth->SetIconChar(":");
	m_qbSentryHealth->SetVisible(false);

	m_qbSentryLevel->SetLabelText("#FF_ITEM_LEVEL");
	m_qbSentryLevel->SetAmountMax(3);
	m_qbSentryLevel->SetIntensityControl(1,2,2,3);
	m_qbSentryLevel->SetVisible(false);
}

void CHudBuildStateSentry::OnTick() 
{
	SetPos( vgui::scheme()->GetProportionalScaledValue(10),  vgui::scheme()->GetProportionalScaledValue(190) );
	//SetPos( vgui::scheme()->GetProportionalScaledValue(0),  vgui::scheme()->GetProportionalScaledValue(0) );

	BaseClass::OnTick();

	if (!engine->IsInGame()) 
		return;

	// Get the local player
	C_FFPlayer *pPlayer = ToFFPlayer(C_BasePlayer::GetLocalPlayer());

	// If the player is not an FFPlayer or is not an Engineer
	if (!pPlayer && pPlayer->GetClassSlot() != CLASS_ENGINEER)
	//hide the panel
	{
		SetPaintBackgroundEnabled(false);
		SetPaintBorderEnabled(false);
		SetPaintEnabled(false);
		return; //return and don't continue
	}
	else
	//show the panel
	{
		SetPaintBackgroundEnabled(true);
		SetPaintBorderEnabled(true);
		SetPaintEnabled(true);
	}
	
	m_bBuilt = pPlayer->GetSentryGun();
	
	//if not built
	if(!m_bBuilt)
	//hide quantity bars
	{
		m_qbSentryHealth->SetVisible(false);
		m_qbSentryLevel->SetVisible(false);
	}
	else
	//show quantity bars
	{
		m_qbSentryHealth->SetVisible(true);
		m_qbSentryLevel->SetVisible(true);
	}
}

void CHudBuildStateSentry::Paint() 
{
	wchar_t* pText;

	if(!m_bBuilt)
	//if not built
	{
		//paint "Not Built" message
		//LOCALISE THIS
		pText = L"Not Built";	// wide char text
		vgui::surface()->DrawSetTextFont( m_hfText ); // set the font	
		vgui::surface()->DrawSetTextColor( m_ColorText );
		vgui::surface()->DrawSetTextPos( m_iQuantityBarPositionX * m_flScale, m_iQuantityBarPositionY * m_flScale ); // x,y position
		vgui::surface()->DrawPrintText( pText, wcslen(pText) ); // print text
	}
	
	//paint header
	BaseClass::Paint();
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