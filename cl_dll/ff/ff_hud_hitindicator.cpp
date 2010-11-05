//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "hud_macros.h"
#include "view.h"
#include "vgui_controls/controls.h"
#include <vgui_controls/Panel.h>
#include "vgui/ISurface.h"
#include "IVRenderView.h"
#include "ff_weapon_base.h"
#include "c_ff_player.h"
#include "ff_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	CROSSHAIR_SIZES	5	// This needs to be matched in ff_options.cpp

ConVar hud_hitindicator_time("hud_hitindicator_time", "0.75", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Length of time the hit indicator shows for (set to 0 to disable).");
extern ConVar cl_concaim_movexhair;
extern ConVar cl_concaim_fadetime;

#define FFDEV_CONCAIM_MOVEXHAIR cl_concaim_movexhair.GetInt()
#define FFDEV_CONCAIM_FADETIME cl_concaim_fadetime.GetFloat()

using namespace vgui;

int ScreenTransform( const Vector& point, Vector& screen );

//-----------------------------------------------------------------------------
// Purpose: HUD Hit indication
//-----------------------------------------------------------------------------
class CHudHitIndicator : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CHudHitIndicator, vgui::Panel );

public:

	CHudHitIndicator( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudHitIndicator" )
	{
		vgui::Panel *pParent = g_pClientMode->GetViewport();
		SetParent( pParent );

		SetPaintBackgroundEnabled( false );

		m_vecCrossHairOffsetAngle.Init();

		SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_CROSSHAIR );
	}

	void			SetCrosshairAngle( const QAngle& angle );
	void			Init();
	void			Reset();

	// Handler for our message
	void			MsgFunc_Hit(bf_read &msg);

protected:
	virtual void	ApplySchemeSettings( vgui::IScheme *scheme );
	virtual void	Paint();

private:
	// Crosshair sprite and colors
	QAngle			m_vecCrossHairOffsetAngle;
	
	float			m_flStartTime;

	QAngle			m_curViewAngles;
	Vector			m_curViewOrigin;

	vgui::HFont		m_hPrimaryCrosshairs[CROSSHAIR_SIZES];
	vgui::HFont		m_hSecondaryCrosshairs[CROSSHAIR_SIZES];
};

DECLARE_HUDELEMENT(CHudHitIndicator);
DECLARE_HUD_MESSAGE(CHudHitIndicator, Hit);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHitIndicator::Reset()
{
	m_vecCrossHairOffsetAngle.Init();
	m_flStartTime = 0.0;
}

void CHudHitIndicator::Init()
{
	HOOK_HUD_MESSAGE(CHudHitIndicator, Hit);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHitIndicator::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	SetPaintBackgroundEnabled( false );

	// --> Mirv
	vgui::HScheme CrossHairScheme = vgui::scheme()->LoadSchemeFromFile("resource/CrosshairScheme.res", "CrosshairScheme");

	for (int i = 0; i < CROSSHAIR_SIZES; i++)
	{
		m_hPrimaryCrosshairs[i] = vgui::scheme()->GetIScheme(CrossHairScheme)->GetFont(VarArgs("PrimaryCrosshairs%d", (i + 1)));
		m_hSecondaryCrosshairs[i] = vgui::scheme()->GetIScheme(CrossHairScheme)->GetFont(VarArgs("SecondaryCrosshairs%d", (i + 1)));
	}
	// <-- Mirv


    SetSize( ScreenWidth(), ScreenHeight() );
}

//-----------------------------------------------------------------------------
// Purpose: Message handler for Damage message
//-----------------------------------------------------------------------------
void CHudHitIndicator::MsgFunc_Hit(bf_read &msg)
{
	m_flStartTime = gpGlobals->curtime;
}

extern void GetHitCrosshair(char &innerChar, Color &innerCol, int &innerSize, char &outerChar, Color &outerCol, int &outerSize);	// |-- squeek

void CHudHitIndicator::Paint( void )
{
	if ( !CHudElement::ShouldDraw() || hud_hitindicator_time.GetFloat() == 0 || (m_flStartTime + hud_hitindicator_time.GetFloat() < gpGlobals->curtime) )
		return;

	if ( !IsCurrentViewAccessAllowed() )
		return;
	
	C_FFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

	m_curViewAngles = CurrentViewAngles();
	m_curViewOrigin = CurrentViewOrigin();

	float x, y;
	x = ScreenWidth()/2;
	y = ScreenHeight()/2;

	// MattB - m_vecCrossHairOffsetAngle is the autoaim angle.
	// if we're not using autoaim, just draw in the middle of the 
	// screen
	if ( m_vecCrossHairOffsetAngle != vec3_angle )
	{
		Assert(0);	// |-- Mirv
		QAngle angles;
		Vector forward;
		Vector point, screen;

		// this code is wrong
		angles = m_curViewAngles + m_vecCrossHairOffsetAngle;
		AngleVectors( angles, &forward );
		VectorAdd( m_curViewOrigin, forward, point );
		ScreenTransform( point, screen );

		x += 0.5f * screen[0] * ScreenWidth() + 0.5f;
		y += 0.5f * screen[1] * ScreenHeight() + 0.5f;
	}
	
	// AfterShock: Conc aim -> plot crosshair properly
	if ( ( FFDEV_CONCAIM_MOVEXHAIR == 1) && ( (pPlayer->m_flConcTime > gpGlobals->curtime) || (pPlayer->m_flConcTime < 0) ) )
	{
		QAngle angles;
		Vector forward;
		Vector point, screen;

		// this code is wrong
		// AfterShock: No, the code is now right!
		angles = pPlayer->EyeAngles();
		AngleVectors( angles, &forward );
		forward *= 10000.0f;
		VectorAdd( m_curViewOrigin, forward, point );
		ScreenTransform( point, screen );

		x = (screen[0]*0.5 + 0.5f) * ScreenWidth();
		y = (1 - ( screen[1]*0.5 + 0.5f ) ) * ScreenHeight();
	}
	else if ( ( FFDEV_CONCAIM_MOVEXHAIR == 2) && ( (pPlayer->m_flConcTime > gpGlobals->curtime) || (pPlayer->m_flConcTime < 0) ) )
	{
		x = -1;
		y = -1;
	}
	else if ( ( FFDEV_CONCAIM_MOVEXHAIR == 3) && ( (pPlayer->m_flConcTime > gpGlobals->curtime) || (pPlayer->m_flConcTime < 0) ) )
	{
		// if should be flashing
		if (gpGlobals->curtime < pPlayer->m_flTrueAimTime + FFDEV_CONCAIM_FADETIME)
		{
			QAngle angles;
			Vector forward;
			Vector point, screen;

			// this code is wrong
			// AfterShock: No, the code is now right!
			angles = pPlayer->EyeAngles();
			AngleVectors( angles, &forward );
			forward *= 10000.0f;
			VectorAdd( m_curViewOrigin, forward, point );
			ScreenTransform( point, screen );

			x = (screen[0]*0.5 + 0.5f) * ScreenWidth();
			y = (1 - ( screen[1]*0.5 + 0.5f ) ) * ScreenHeight();
		}
		// else don't draw xhair at all
		else
		{
			x = -1;
			y = -1;
		}
	}

	// --> Mirv: Crosshair stuff
	//m_pCrosshair->DrawSelf( 
	//		x - 0.5f * m_pCrosshair->Width(), 
	//		y - 0.5f * m_pCrosshair->Height(),
	//		m_clrCrosshair );

	Color innerCol, outerCol;
	char innerChar, outerChar;
	int innerSize, outerSize;
	wchar_t unicode[2];

	//
	// TODO: Clean this up!!!!
	//

	HFont currentFont;
	GetHitCrosshair(innerChar, innerCol, innerSize, outerChar, outerCol, outerSize);

	// Find our fade based on our time shown
	float dt = ( m_flStartTime - gpGlobals->curtime );
	float flAlpha = SimpleSplineRemapVal( dt, 0.0f, hud_hitindicator_time.GetFloat(), 255, 0 );
	flAlpha = clamp( flAlpha, 0.0f, 255.0f );
	
	// movexhair 3 = flash xhair when shooting
	if ( ( FFDEV_CONCAIM_MOVEXHAIR == 3) && ( (pPlayer->m_flConcTime > gpGlobals->curtime) || (pPlayer->m_flConcTime < 0) ) )
	{
		// calculate alphas
		float flFlashAlpha = clamp(1.0f - (gpGlobals->curtime - pPlayer->m_flTrueAimTime)/FFDEV_CONCAIM_FADETIME, 0.0f, 1.0f);
		// set alphas
		outerCol[3] *= flFlashAlpha;
		innerCol[3] *= flFlashAlpha;
	}

	currentFont = m_hSecondaryCrosshairs[clamp(outerSize, 1, CROSSHAIR_SIZES) - 1];

	surface()->DrawSetTextColor(outerCol.r(), outerCol.g(), outerCol.b(), flAlpha / 255.0f * outerCol.a());
	surface()->DrawSetTextFont(currentFont);

	int charOffsetX = surface()->GetCharacterWidth(currentFont, outerChar) / 2;
	int charOffsetY = surface()->GetFontTall(currentFont) / 2;

	swprintf(unicode, L"%c", outerChar);

	surface()->DrawSetTextPos(x - charOffsetX, y - charOffsetY);
	surface()->DrawUnicodeChar(unicode[0]);

	currentFont = m_hPrimaryCrosshairs[clamp(innerSize, 1, CROSSHAIR_SIZES) - 1];

	surface()->DrawSetTextColor(innerCol.r(), innerCol.g(), innerCol.b(), flAlpha / 255.0f * innerCol.a());
	surface()->DrawSetTextFont(currentFont);

	charOffsetX = surface()->GetCharacterWidth(currentFont, innerChar) / 2;
	charOffsetY = surface()->GetFontTall(currentFont) / 2;

	swprintf(unicode, L"%c", innerChar);

	surface()->DrawSetTextPos(x - charOffsetX, y - charOffsetY);
	surface()->DrawUnicodeChar(unicode[0]);
	// <-- Mirv

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHitIndicator::SetCrosshairAngle( const QAngle& angle )
{
	VectorCopy( angle, m_vecCrossHairOffsetAngle );
}

