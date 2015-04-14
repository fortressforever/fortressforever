
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"

#include <vgui/ISurface.h>
#include <vgui/ISystem.h>

#include "ff_panel.h"
#include "c_ff_player.h"
#include "ff_utils.h"
#include "in_buttons.h"

#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
static ConVar hud_keystate("hud_keystate", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Toggle visibility of the keys you are pressing.");
static ConVar hud_keystate_spec("hud_keystate_spec", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Toggle visibility of the keys being pressed by your spectator target.");

class CHudKeyStateElement : public vgui::FFPanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudKeyStateElement, vgui::FFPanel );

	CHudKeyStateElement(Panel *parent, const char *unlocalizedText, int buttonBit, vgui::HFont font) : BaseClass(parent)
	{
		m_text = vgui::localize()->Find( unlocalizedText );
		m_buttonBit = buttonBit;
		m_font = font;
	}

	const wchar_t *m_text;
	int m_buttonBit;
	vgui::HFont m_font;
	Color m_activeColor;
	
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);
		m_activeColor = GetSchemeColor( "KeyStatePressed", pScheme );
	}

	bool IsKeyPressedByPlayer(C_FFPlayer *pPlayer) { return pPlayer->m_nButtons & m_buttonBit; }

	virtual void Paint() 
	{
		C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayerOrObserverTarget(); 

		if ( !pPlayer ) 
			return; 

		int textWidth, textHeight;
		surface()->GetTextSize(m_font, m_text, textWidth, textHeight);
		int keyBoxHeight = this->GetTall();
		int keyBoxWidth = this->GetWide();
		Color fgColor = IsKeyPressedByPlayer(pPlayer) ? m_activeColor : GetFgColor();

		surface()->DrawSetColor(m_TeamColorHudBackgroundColour);
		surface()->DrawFilledRect(0, 0, keyBoxWidth, keyBoxHeight);

		surface()->DrawSetColor(fgColor);
		surface()->DrawOutlinedRect(0, 0, keyBoxWidth, keyBoxHeight);
		surface()->DrawOutlinedRect(1, 1, keyBoxWidth-1, keyBoxHeight-1);

		surface()->DrawSetTextFont( m_font );
		surface()->DrawSetTextColor( fgColor );
		surface()->DrawSetTextPos( (int) ((float) keyBoxWidth / 2.0f - (float) textWidth / 2.0f), (int) ((float) keyBoxHeight / 2.0f - (float) textHeight / 2.0f) );

		surface()->DrawUnicodeString( m_text );
	}
};

//-----------------------------------------------------------------------------
// Purpose: Displays current disguised class
//-----------------------------------------------------------------------------
class CHudKeyState : public CHudElement, public vgui::FFPanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudKeyState, vgui::FFPanel );

	CHudKeyState( const char *pElementName ) : vgui::FFPanel( NULL, "HudKeyState" ), CHudElement( pElementName )
	{
		SetParent( g_pClientMode->GetViewport() );
	}

	virtual ~CHudKeyState( void )
	{
		if( m_pForwardState )
		{
			delete m_pForwardState;
			m_pForwardState = NULL;
		}
		if( m_pBackwardState )
		{
			delete m_pBackwardState;
			m_pBackwardState = NULL;
		}
		if( m_pLeftState )
		{
			delete m_pLeftState;
			m_pLeftState = NULL;
		}
		if( m_pRightState )
		{
			delete m_pRightState;
			m_pRightState = NULL;
		}
		if( m_pJumpState )
		{
			delete m_pJumpState;
			m_pJumpState = NULL;
		}
		if( m_pDuckState )
		{
			delete m_pDuckState;
			m_pDuckState = NULL;
		}
	}

	virtual void Paint( void );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual bool ShouldDraw( void );
	virtual void PerformLayout( void );

	CPanelAnimationVar( vgui::HFont, m_hKeyFont, "font", "Default" );
	
	CPanelAnimationVarAliasType( float, forward_xpos, "forward_xpos", "22", "proportional_float" );
	CPanelAnimationVarAliasType( float, forward_ypos, "forward_ypos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, forward_wide, "forward_wide", "20", "proportional_float" );
	CPanelAnimationVarAliasType( float, forward_tall, "forward_tall", "20", "proportional_float" );

	CPanelAnimationVarAliasType( float, back_xpos, "back_xpos", "22", "proportional_float" );
	CPanelAnimationVarAliasType( float, back_ypos, "back_ypos", "22", "proportional_float" );
	CPanelAnimationVarAliasType( float, back_wide, "back_wide", "20", "proportional_float" );
	CPanelAnimationVarAliasType( float, back_tall, "back_tall", "20", "proportional_float" );

	CPanelAnimationVarAliasType( float, left_xpos, "left_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, left_ypos, "left_ypos", "22", "proportional_float" );
	CPanelAnimationVarAliasType( float, left_wide, "left_wide", "20", "proportional_float" );
	CPanelAnimationVarAliasType( float, left_tall, "left_tall", "20", "proportional_float" );
	
	CPanelAnimationVarAliasType( float, right_xpos, "right_xpos", "44", "proportional_float" );
	CPanelAnimationVarAliasType( float, right_ypos, "right_ypos", "22", "proportional_float" );
	CPanelAnimationVarAliasType( float, right_wide, "right_wide", "20", "proportional_float" );
	CPanelAnimationVarAliasType( float, right_tall, "right_tall", "20", "proportional_float" );
	
	CPanelAnimationVarAliasType( float, jump_xpos, "jump_xpos", "33", "proportional_float" );
	CPanelAnimationVarAliasType( float, jump_ypos, "jump_ypos", "44", "proportional_float" );
	CPanelAnimationVarAliasType( float, jump_wide, "jump_wide", "31", "proportional_float" );
	CPanelAnimationVarAliasType( float, jump_tall, "jump_tall", "20", "proportional_float" );
	
	CPanelAnimationVarAliasType( float, duck_xpos, "duck_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, duck_ypos, "duck_ypos", "44", "proportional_float" );
	CPanelAnimationVarAliasType( float, duck_wide, "duck_wide", "31", "proportional_float" );
	CPanelAnimationVarAliasType( float, duck_tall, "duck_tall", "20", "proportional_float" );

	CHudKeyStateElement *m_pForwardState;
	CHudKeyStateElement *m_pBackwardState;
	CHudKeyStateElement *m_pLeftState;
	CHudKeyStateElement *m_pRightState;
	CHudKeyStateElement *m_pJumpState;
	CHudKeyStateElement *m_pDuckState;
};

DECLARE_HUDELEMENT( CHudKeyState );

void CHudKeyState::Init( void )
{
}

void CHudKeyState::PerformLayout()
{
	BaseClass::PerformLayout();

	if (m_pForwardState)
	{
		m_pForwardState->SetPos( forward_xpos, forward_ypos );
		m_pForwardState->SetSize( forward_wide, forward_tall );
	}
	if (m_pBackwardState)
	{
		m_pBackwardState->SetPos( back_xpos, back_ypos );
		m_pBackwardState->SetSize( back_wide, back_tall );
	}
	if (m_pLeftState)
	{
		m_pLeftState->SetPos( left_xpos, left_ypos );
		m_pLeftState->SetSize( left_wide, left_tall );
	}
	if (m_pRightState)
	{
		m_pRightState->SetPos( right_xpos, right_ypos );
		m_pRightState->SetSize( right_wide, right_tall );
	}
	if (m_pJumpState)
	{
		m_pJumpState->SetPos( jump_xpos, jump_ypos );
		m_pJumpState->SetSize( jump_wide, jump_tall );
	}
	if (m_pDuckState)
	{
		m_pDuckState->SetPos( duck_xpos, duck_ypos );
		m_pDuckState->SetSize( duck_wide, duck_tall );
	}
}

/** Called each map load
*/
void CHudKeyState::VidInit( void )
{
	if (!m_pForwardState)
	{
		m_pForwardState = new CHudKeyStateElement(this, "#FF_HUD_KEYSTATE_UPARROW", IN_FORWARD, m_hKeyFont);
	}
	if (!m_pBackwardState)
	{
		m_pBackwardState = new CHudKeyStateElement(this, "#FF_HUD_KEYSTATE_DOWNARROW", IN_BACK, m_hKeyFont);
	}
	if (!m_pLeftState)
	{
		m_pLeftState = new CHudKeyStateElement(this, "#FF_HUD_KEYSTATE_LEFTARROW", IN_MOVELEFT, m_hKeyFont);
	}
	if (!m_pRightState)
	{
		m_pRightState = new CHudKeyStateElement(this, "#FF_HUD_KEYSTATE_RIGHTARROW", IN_MOVERIGHT, m_hKeyFont);
	}
	if (!m_pJumpState)
	{
		m_pJumpState = new CHudKeyStateElement(this, "#Valve_Jump", IN_JUMP, m_hKeyFont);
	}
	if (!m_pDuckState)
	{
		m_pDuckState = new CHudKeyStateElement(this, "#Valve_Duck", IN_DUCK, m_hKeyFont);
	}

	PerformLayout();
}

bool CHudKeyState::ShouldDraw() 
{ 
	if( !engine->IsInGame() ) 
		return false; 

	C_FFPlayer *pLocalPlayer = C_FFPlayer::GetLocalFFPlayer();
	C_FFPlayer *pTarget = C_FFPlayer::GetLocalFFPlayerOrAnyObserverTarget();

	if( !pTarget ) 
		return false;
	
	if( !FF_HasPlayerPickedClass( pTarget ) )
		return false;

	bool isSpectating = pTarget != pLocalPlayer;

	if( !isSpectating && !hud_keystate.GetBool() ) 
		return false; 

	if( isSpectating && !hud_keystate_spec.GetBool() ) 
		return false; 

	return true; 
} 

void CHudKeyState::Paint() 
{
	BaseClass::Paint();
}
