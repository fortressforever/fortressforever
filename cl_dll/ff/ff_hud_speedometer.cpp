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

// ELMO *** 
#define BHOP_CAP_SOFT 1.4f // as defined in ff_gamemovement.cpp
#define BHOP_CAP_HARD 2.0f // as defined in ff_gamemovement.cpp
#define SPEEDO_COLOR_RED Color(255,0,0,255)
#define SPEEDO_COLOR_ORANGE Color(255,128,0,255)
#define SPEEDO_COLOR_GREEN Color(0,255,0,255)
#define SPEEDO_COLOR_DEFAULT Color(255,255,255,255)
// *** ELMO

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

static ConVar hud_speedometer( "hud_speedometer", "0", FCVAR_ARCHIVE, "Toggle speedometer. Disclaimer: We are not responsible if you get a ticket.");
// ELMO *** 
static ConVar hud_speedometer_color( "hud_speedometer_color", "2", FCVAR_ARCHIVE, "0=No color, 1=Stepped Color, 2=Fading Color (RED > hardcap :: ORANGE > softcap :: GREEN > run speed :: WHITE < run speed)", true, 0.0f, true, 2.0f);

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
// ELMO ***
	Color ColorFade( int currentVal, int maxVal, Color maxColor, int minVal, Color minColor );
	Color speedColor;
	int maxVelocity;
	float full, f1, f2;
	int avgVelocity;
// *** ELMO
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
	avgVelocity = (int) FastSqrt(
		vecVelocity.x * vecVelocity.x 
		+ vecVelocity.y * vecVelocity.y );

// ELMO *** 
	maxVelocity = C_BasePlayer::GetLocalPlayer()->MaxSpeed();

	if( avgVelocity > BHOP_CAP_HARD * maxVelocity && hud_speedometer_color.GetInt() > 0) // above hard cap
		speedColor = SPEEDO_COLOR_RED;
	else if(avgVelocity  > BHOP_CAP_SOFT * maxVelocity && hud_speedometer_color.GetInt() > 0) // above soft cap
		if(hud_speedometer_color.GetInt() == 2)
			speedColor = ColorFade( avgVelocity, BHOP_CAP_HARD*maxVelocity, SPEEDO_COLOR_RED, BHOP_CAP_SOFT*maxVelocity, SPEEDO_COLOR_ORANGE );
		else
			speedColor = SPEEDO_COLOR_ORANGE;	
	else if( avgVelocity > maxVelocity && hud_speedometer_color.GetInt() > 0) // above max run speed
		if(hud_speedometer_color.GetInt() == 2)
			speedColor = ColorFade( avgVelocity, BHOP_CAP_SOFT*maxVelocity, SPEEDO_COLOR_ORANGE, maxVelocity, SPEEDO_COLOR_GREEN );
		else
			speedColor = SPEEDO_COLOR_GREEN;
	else // below max run speed
		speedColor = SPEEDO_COLOR_DEFAULT;

// *** ELMO

	wchar_t unicode[6];
	swprintf(unicode, L"%d", avgVelocity);

	// Hey voogru, why not .Length() ? Does the same thing :)
	
	surface()->DrawSetTextPos(digit_xpos, digit_ypos);
	for (wchar_t *ch = unicode; *ch != 0; ch++)
	{
		surface()->DrawSetTextColor(speedColor);
		surface()->DrawUnicodeChar(*ch);
	}
}

// ELMO *** I don't know if there is a function for this already. I would expect so but where?
Color CHudSpeedometer::ColorFade( int currentVal, int maxVal, Color maxColor, int minVal, Color minColor )
{
	full = maxVal - minVal;
	f1 = (maxVal - currentVal) / full;
	f2 = (currentVal - minVal) / full;
	return Color(
		(int) (maxColor.r() * f2 + minColor.r() * f1),
		(int) (maxColor.g() * f2 + minColor.g() * f1),
		(int) (maxColor.b() * f2 + minColor.b() * f1),
		255);
}
// *** ELMO