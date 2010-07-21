#include "cbase.h"
#include "ff_hud_buildstate_base.h"

#include "c_ff_player.h" //required to cast base player

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar hud_buildstate_sg_x( "hud_buildstate_sg_x", "0", FCVAR_ARCHIVE, "Panel's X position on 640 480 Resolution");
static ConVar hud_buildstate_sg_y( "hud_buildstate_sg_y", "0", FCVAR_ARCHIVE, "Panel's Y Position on 640 480 Resolution");
static ConVar hud_buildstate_sg_align_horiz( "hud_buildstate_sg_align_horiz", "0", FCVAR_ARCHIVE, "Panel's horizontal alignment to the specified position (0=left, 1=center, 2=right");
static ConVar hud_buildstate_sg_align_vert( "hud_buildstate_sg_align_vert", "0", FCVAR_ARCHIVE, "Panel's vertical alignment to the specified position (0=top, 1=middle, 2=bottom");
static ConVar hud_buildstate_sg_columns( "hud_buildstate_sg_columns", "1", FCVAR_ARCHIVE, "Number of quantity bar columns");

class CHudBuildStateSentry : public CHudElement, public CHudBuildStateBase
{
	DECLARE_CLASS_SIMPLE( CHudBuildStateSentry, CHudBuildStateBase );

public:
	CHudBuildStateSentry(const char *pElementName) : CHudElement(pElementName), CHudBuildStateBase(NULL, "HudBuildStateSentry")
	{
		SetParent(g_pClientMode->GetViewport());
		SetHiddenBits( 0 );

		m_bBuilt = false;

		vgui::ivgui()->AddTickSignal(GetVPanel(), 500); //only update 2 times a second
	}

	~CHudBuildStateSentry( void ) {}

	virtual void	Init( void );
	virtual void	OnTick( void );
	virtual void	Paint( void );
	
	void	MsgFunc_SentryMsg(bf_read &msg);

protected:
	virtual void CheckCvars();
private:
	// could probably do this without these now
	// but would need an alternative of selecting the one you want easily
	CHudQuantityBar *m_qbSentryHealth;
	CHudQuantityBar *m_qbSentryLevel;

	bool	m_bBuilt;
};

DECLARE_HUDELEMENT(CHudBuildStateSentry);
DECLARE_HUD_MESSAGE(CHudBuildStateSentry, SentryMsg);

void CHudBuildStateSentry::CheckCvars()
{
	bool updateBarPositions = false;

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

void CHudBuildStateSentry::Init() 
{
	HOOK_HUD_MESSAGE(CHudBuildStateSentry, SentryMsg);

	wchar_t *tempString = vgui::localize()->Find("#FF_PLAYER_SENTRYGUN");

	if (!tempString) 
		tempString = L"HEALTH";

	m_qbSentryHealth = AddChild("BuildStateSentryHealth"); 
	m_qbSentryLevel = AddChild("BuildStateSentryLevel"); 

	SetHeaderText(tempString);
	SetHeaderIconChar("R");
	
	m_qbSentryHealth->SetLabelText("#FF_iTEM_HEALTH");
	m_qbSentryHealth->SetIconChar(":");
	m_qbSentryHealth->SetVisible(false);

	m_qbSentryLevel->SetLabelText("#FF_iTEM_lEVEL");
	m_qbSentryLevel->SetAmountMax(3);
	m_qbSentryLevel->SetIntensityControl(1,2,2,3);
	m_qbSentryLevel->SetVisible(false);
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
		vgui::surface()->DrawSetTextPos( m_qb_iPositionX * m_flScale, m_qb_iPositionY * m_flScale ); // x,y position
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