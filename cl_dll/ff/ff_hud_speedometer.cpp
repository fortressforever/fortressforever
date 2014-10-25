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
#include "ff_utils.h"
#include "ff_shareddefs.h" //added to use colors stored within!

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include "c_ff_player.h"

#include <vgui/ILocalize.h>

// ELMO *** 
#define BHOP_CAP_SOFT 1.4f // as defined in ff_gamemovement.cpp
#define BHOP_CAP_HARD 1.9f // as defined in ff_gamemovement.cpp
// *** ELMO

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

static ConVar hud_speedometer( "hud_speedometer", "0", FCVAR_ARCHIVE, "Toggle speedometer. Disclaimer: We are not responsible if you get a ticket.");
static ConVar hud_speedometer_avg( "hud_speedometer_avg", "0", FCVAR_ARCHIVE, "Toggle average speedometer.");
// ELMO *** 
static ConVar hud_speedometer_color( "hud_speedometer_color", "2", FCVAR_ARCHIVE, "0=No color, 1=Stepped Color, 2=Fading Color (RED > hardcap :: ORANGE > softcap :: GREEN > run speed :: WHITE < run speed)", true, 0.0f, true, 2.0f);
static ConVar hud_speedometer_avg_color( "hud_speedometer_avg_color", "0", FCVAR_ARCHIVE, "0=No color, 1=Stepped Color, 2=Fading Color (RED > hardcap :: ORANGE > softcap :: GREEN > run speed :: WHITE < run speed)", true, 0.0f, true, 2.0f);

Color ColorFade( int currentVal, int minVal, int maxVal, Color minColor, Color maxColor );

//-----------------------------------------------------------------------------
// Purpose: Displays current disguised class
//-----------------------------------------------------------------------------
class CHudSpeedometer : public CHudElement, public vgui::FFPanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudSpeedometer, vgui::FFPanel );

	CHudSpeedometer( const char *pElementName ) : vgui::FFPanel( NULL, "HudSpeedometer" ), CHudElement( pElementName )
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport() );

		// Hide when player is dead
		SetHiddenBits( /*HIDEHUD_HEALTH |*/ HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
	}

	virtual ~CHudSpeedometer( void )
	{

	}

	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();
	virtual void Paint();

private:
// ELMO ***
	float m_flAvgVelocity;
	unsigned int m_iNumUpdates;
	int m_iVelocity;
// *** ELMO
	float m_flNextUpdate;

private:
	// Stuff we need to know	
	CPanelAnimationVar( vgui::HFont, m_hSpeedFont, "SpeedFont", "HudNumbers" );
	CPanelAnimationVar( vgui::HFont, m_hAvgSpeedFont, "AvgSpeedFont", "Default" );
	CPanelAnimationVar( Color, m_TextColor, "TextColor", "HUD_Tone_Default" );

	CPanelAnimationVarAliasType( float, SpeedFont_xpos, "SpeedFont_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, SpeedFont_ypos, "SpeedFont_ypos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, AvgSpeedFont_xpos, "AvgSpeedFont_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, AvgSpeedFont_ypos, "AvgSpeedFont_ypos", "0", "proportional_float" );
};

DECLARE_HUDELEMENT( CHudSpeedometer );

//-----------------------------------------------------------------------------
// Purpose: Done on loading game?
//-----------------------------------------------------------------------------
void CHudSpeedometer::Init( void )
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudSpeedometer::VidInit( void )
{
	Reset();
	m_flAvgVelocity = 0.0;
	m_iNumUpdates = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSpeedometer::Reset()
{
	m_flNextUpdate = 0.0;
}

//-----------------------------------------------------------------------------
// Purpose: Should we draw? (Are we ingame? have we picked a class, etc)
//-----------------------------------------------------------------------------
void CHudSpeedometer::OnThink() 
{ 
	if (!hud_speedometer.GetBool() && !hud_speedometer_avg.GetBool())
		return;

	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();

	if(!local)
		return;

	//don't update so fast.
	if(m_flNextUpdate < gpGlobals->curtime)
	{
		Vector vecVelocity = local->GetAbsVelocity();
		m_iVelocity = (int) FastSqrt(
			vecVelocity.x * vecVelocity.x 
			+ vecVelocity.y * vecVelocity.y );

		// average[i+1] = (average[i]*i + value[i+1])/(i+1)
		m_flAvgVelocity = (m_flAvgVelocity*m_iNumUpdates + (float)m_iVelocity)/(m_iNumUpdates+1);
		m_iNumUpdates++;

		m_flNextUpdate = gpGlobals->curtime + 0.1;
	}
	
} 

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudSpeedometer::Paint() 
{
	if (!hud_speedometer.GetBool() && !hud_speedometer_avg.GetBool())
		return;

	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();

	if(!local)
		return;

	float maxVelocity = local->MaxSpeed();
	Color speedColor;

	// regular speedometer
	if( hud_speedometer.GetBool() )
	{
		if( m_iVelocity > BHOP_CAP_HARD * maxVelocity && hud_speedometer_color.GetInt() > 0) // above hard cap
			speedColor = INTENSITYSCALE_COLOR_RED;
		else if(m_iVelocity-1  > BHOP_CAP_SOFT * maxVelocity && hud_speedometer_color.GetInt() > 0) // above soft cap
			if(hud_speedometer_color.GetInt() == 2)
				speedColor = ColorFade( m_iVelocity, BHOP_CAP_SOFT*maxVelocity, BHOP_CAP_HARD*maxVelocity, INTENSITYSCALE_COLOR_ORANGE, INTENSITYSCALE_COLOR_RED );
			else
				speedColor = INTENSITYSCALE_COLOR_ORANGE;	
		else if( m_iVelocity > maxVelocity && hud_speedometer_color.GetInt() > 0) // above max run speed
			if(hud_speedometer_color.GetInt() == 2)
				if( m_iVelocity > (maxVelocity+3*(BHOP_CAP_SOFT*maxVelocity - maxVelocity)/4) && hud_speedometer_color.GetInt() > 0) // above max run speed
					speedColor = ColorFade( m_iVelocity, maxVelocity+3*(BHOP_CAP_SOFT*maxVelocity - maxVelocity)/4, BHOP_CAP_SOFT*maxVelocity, INTENSITYSCALE_COLOR_YELLOW, INTENSITYSCALE_COLOR_ORANGE );
				else
					speedColor = ColorFade( m_iVelocity, maxVelocity, maxVelocity+3*(BHOP_CAP_SOFT*maxVelocity - maxVelocity)/4, INTENSITYSCALE_COLOR_GREEN, INTENSITYSCALE_COLOR_YELLOW );
			else
				speedColor = INTENSITYSCALE_COLOR_GREEN;
		else // below max run speed
			speedColor = INTENSITYSCALE_COLOR_DEFAULT;

		surface()->DrawSetTextFont( m_hSpeedFont );
		surface()->DrawSetTextColor( speedColor );
		surface()->DrawSetTextPos( SpeedFont_xpos, SpeedFont_ypos );

		wchar_t unicode[6];
		swprintf(unicode, L"%d", (int)m_iVelocity);

		for( wchar_t *wch = unicode; *wch != 0; wch++ )
			surface()->DrawUnicodeChar( *wch );
	}

	// average speedometer
	if( hud_speedometer_avg.GetBool() )
	{
		if( m_flAvgVelocity > BHOP_CAP_HARD * maxVelocity && hud_speedometer_avg_color.GetInt() > 0) // above hard cap
			speedColor = INTENSITYSCALE_COLOR_RED;
		else if(m_flAvgVelocity-1  > BHOP_CAP_SOFT * maxVelocity && hud_speedometer_avg_color.GetInt() > 0) // above soft cap
			if(hud_speedometer_avg_color.GetInt() == 2)
				speedColor = ColorFade( m_flAvgVelocity, BHOP_CAP_SOFT*maxVelocity, BHOP_CAP_HARD*maxVelocity, INTENSITYSCALE_COLOR_ORANGE, INTENSITYSCALE_COLOR_RED );
			else
				speedColor = INTENSITYSCALE_COLOR_ORANGE;	
		else if( m_flAvgVelocity > maxVelocity && hud_speedometer_avg_color.GetInt() > 0) // above max run speed
			if(hud_speedometer_avg_color.GetInt() == 2)
				if( m_flAvgVelocity > (maxVelocity+3*(BHOP_CAP_SOFT*maxVelocity - maxVelocity)/4) && hud_speedometer_color.GetInt() > 0) // above max run speed
					speedColor = ColorFade( m_flAvgVelocity, maxVelocity+3*(BHOP_CAP_SOFT*maxVelocity - maxVelocity)/4, BHOP_CAP_SOFT*maxVelocity, INTENSITYSCALE_COLOR_YELLOW, INTENSITYSCALE_COLOR_ORANGE );
				else
					speedColor = ColorFade( m_flAvgVelocity, maxVelocity, maxVelocity+3*(BHOP_CAP_SOFT*maxVelocity - maxVelocity)/4, INTENSITYSCALE_COLOR_GREEN, INTENSITYSCALE_COLOR_YELLOW );
			else
				speedColor = INTENSITYSCALE_COLOR_GREEN;
		else // below max run speed
			speedColor = INTENSITYSCALE_COLOR_DEFAULT;

		surface()->DrawSetTextFont( m_hAvgSpeedFont );
		if (hud_speedometer.GetBool())
			surface()->DrawSetTextPos( AvgSpeedFont_xpos, AvgSpeedFont_ypos );
		else
			surface()->DrawSetTextPos( AvgSpeedFont_xpos, AvgSpeedFont_ypos + SpeedFont_ypos );

		surface()->DrawSetTextColor( m_TextColor );

		wchar_t textunicode[12];
		swprintf(textunicode, L"Average: ");

		for( wchar_t *wch = textunicode; *wch != 0; wch++ )
			surface()->DrawUnicodeChar( *wch );

		surface()->DrawSetTextColor( speedColor );

		wchar_t unicode[6];
		swprintf(unicode, L"%d", (int)m_flAvgVelocity);

		for( wchar_t *wch = unicode; *wch != 0; wch++ )
			surface()->DrawUnicodeChar( *wch );
	}
}
