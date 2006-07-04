//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include "iclientvehicle.h"
#include "ammodef.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include "c_ff_player.h"

#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

static ConVar hud_speedometer( "hud_speedometer", "0", 0, "Toggle speedometer. Disclaimer: We are not responsible if you get a ticket.");

class CHudSpeedometer : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudSpeedometer, CHudNumericDisplay );

public:
	CHudSpeedometer( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();
	virtual void Paint();
private:
	float m_flNextUpdate;
	Vector vecVelocity;
	CPanelAnimationVar( vgui::HFont, m_hNumberFont, "NumberFont" "TextFont", "HudNumbers" );
	
	CPanelAnimationVarAliasType( float, digit_xpos, "digit_xpos", "50", "proportional_float" );
	CPanelAnimationVarAliasType( float, digit_ypos, "digit_ypos", "2", "proportional_float" );
};	

DECLARE_HUDELEMENT( CHudSpeedometer );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudSpeedometer::CHudSpeedometer( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudSpeedometer")
{
	SetHiddenBits( /*HIDEHUD_HEALTH |*/ HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSpeedometer::Init()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSpeedometer::Reset()
{
	m_flNextUpdate = 0.0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSpeedometer::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSpeedometer::OnThink()
{
	if(!hud_speedometer.GetBool())
		return;

	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();

	if(!local)
		return;

	//don't update so fast.
	if(m_flNextUpdate < gpGlobals->curtime)
	{
		vecVelocity = local->GetAbsVelocity();
		m_flNextUpdate = gpGlobals->curtime + 0.1;
	}
}

void CHudSpeedometer::Paint()
{
	if( C_BasePlayer::GetLocalPlayer() && ( C_BasePlayer::GetLocalPlayer()->GetTeamNumber() < TEAM_BLUE ) )
		return;

	if(!hud_speedometer.GetBool())
		return;

	surface()->DrawSetTextFont(m_hNumberFont);
	wchar_t unicode[6];
	swprintf(unicode, L"%d", (int)
		FastSqrt(
		vecVelocity.x * vecVelocity.x 
		+ vecVelocity.y * vecVelocity.y 
		+ vecVelocity.z * vecVelocity.z));
	
	surface()->DrawSetTextPos(digit_xpos, digit_ypos);
	for (wchar_t *ch = unicode; *ch != 0; ch++)
	{
		surface()->DrawSetTextColor(Color(255,255,180,255));
		surface()->DrawUnicodeChar(*ch);
	}
}


