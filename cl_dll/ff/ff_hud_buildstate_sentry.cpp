#include "cbase.h"
#include "ff_quantitypanel.h"
#include "ff_hud_quantitybar.h"

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

		m_qbSentryHealth = new CHudQuantityBar(this, "BuildStateSentryHealth"); 
		m_qbSentryLevel = new CHudQuantityBar(this, "BuildStateSentryLevel"); 

		m_bBuilt = false;

		m_flScale = 1.0f;

		vgui::ivgui()->AddTickSignal(GetVPanel(), 250); //only update 4 times a second
	}

	~CHudBuildStateSentry( void ) {}

	virtual void	Init( void );
	virtual void	VidInit( void );
	virtual void	OnTick( void );
	virtual void	Paint( void );

	void	MsgFunc_SentryMsg(bf_read &msg);

protected:
	CHudQuantityBar *m_qbSentryHealth; 
	CHudQuantityBar *m_qbSentryLevel; 

	bool	m_bBuilt;
};

DECLARE_HUDELEMENT(CHudBuildStateSentry);
DECLARE_HUD_MESSAGE(CHudBuildStateSentry, SentryMsg);

void CHudBuildStateSentry::Init() 
{
	HOOK_HUD_MESSAGE(CHudBuildStateSentry, SentryMsg);
	//SetAutoResize(PIN_TOPLEFT,AUTORESIZE_DOWNANDRIGHT,0,0,0,0); <-- this would be nice??

	wchar_t *tempString = vgui::localize()->Find("#FF_PLAYER_SENTRYGUN");

	if (!tempString) 
		tempString = L"HEALTH";

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);
	SetPaintEnabled(false);

	SetHeaderText(tempString);
	SetHeaderIconChar("R");
	
	m_qbSentryHealth->SetLabelText("#FF_ITEM_HEALTH");
	m_qbSentryHealth->SetIconChar(":");
	m_qbSentryHealth->SetVisible(false);
	m_qbSentryHealth->SetPos(40,40);

	m_qbSentryLevel->SetLabelText("#FF_ITEM_LEVEL");
	m_qbSentryLevel->SetAmountMax(3);
	m_qbSentryLevel->SetIntensityControl(1,2,2,3);
	m_qbSentryLevel->SetVisible(false);
	m_qbSentryLevel->SetPos(40,60);
}
void CHudBuildStateSentry::VidInit() 
{
}
void CHudBuildStateSentry::OnTick() 
{
	BaseClass::OnTick();

	//TO-DO
	//I think these should maybe be in VidInit - find out
	SetPos( vgui::scheme()->GetProportionalScaledValue(10),  vgui::scheme()->GetProportionalScaledValue(325) );
	SetSize(vgui::scheme()->GetProportionalScaledValue(120),vgui::scheme()->GetProportionalScaledValue(90));

	if (!engine->IsInGame()) 
		return;

	// Get the local player
	C_FFPlayer *pPlayer = ToFFPlayer(C_BasePlayer::GetLocalPlayer());

	// If the player is not 'valid' or is not an Engineer
	if (!pPlayer || pPlayer->GetClassSlot() != CLASS_ENGINEER)
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
		vgui::surface()->DrawSetTextPos( 10 * m_flScale, 50 * m_flScale ); // x,y position
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