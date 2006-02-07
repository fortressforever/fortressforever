//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "engine/IEngineSound.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/Controls.h"
#include "vgui_controls/Panel.h"
#include "vgui/ISurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	HEALTH_WARNING_THRESHOLD	25

static ConVar	hud_quickinfo( "hud_quickinfo", "0", FCVAR_ARCHIVE );

#define QUICKINFO_EVENT_DURATION	1.0f
#define	QUICKINFO_BRIGHTNESS_FULL	255
#define	QUICKINFO_BRIGHTNESS_DIM	64
#define	QUICKINFO_FADE_IN_TIME		0.5f
#define QUICKINFO_FADE_OUT_TIME		2.0f

/*
==================================================
CHUDQuickInfo 
==================================================
*/

using namespace vgui;

class CHUDQuickInfo : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHUDQuickInfo, vgui::Panel );
public:
	CHUDQuickInfo( const char *pElementName );
	void Init( void );
	void VidInit( void );
	bool ShouldDraw( void );
	virtual void OnThink();
	virtual void Paint();
	
	virtual void ApplySchemeSettings( IScheme *scheme );
private:
	
	void	DrawWarning( int x, int y, CHudTexture *icon, float &time );
	void	UpdateEventTime( void );
	bool	EventTimeElapsed( void );

	int		m_lastAmmo;
	int		m_lastHealth;

	float	m_ammoFade;
	float	m_healthFade;

	bool	m_warnAmmo;
	bool	m_warnHealth;

	bool	m_bFadedOut;
	
	bool	m_bDimmed;			// Whether or not we are dimmed down
	float	m_flLastEventTime;	// Last active event (controls dimmed state)

	CHudTexture	*m_icon_c;

	CHudTexture	*m_icon_rbn;	// right bracket
	CHudTexture	*m_icon_lbn;	// left bracket

	CHudTexture	*m_icon_rb;		// right bracket, full
	CHudTexture	*m_icon_lb;		// left bracket, full
	CHudTexture	*m_icon_rbe;	// right bracket, empty
	CHudTexture	*m_icon_lbe;	// left bracket, empty

};

DECLARE_HUDELEMENT( CHUDQuickInfo );

//
//-----------------------------------------------------
//
CHUDQuickInfo::CHUDQuickInfo( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "HUDQuickInfo" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_CROSSHAIR );
}

void CHUDQuickInfo::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	SetPaintBackgroundEnabled( false );
}


/*
==================================================
Init
==================================================
*/

void CHUDQuickInfo::Init( void )
{
	m_ammoFade		= 0.0f;
	m_healthFade	= 0.0f;

	m_lastAmmo		= 0;
	m_lastHealth	= 100;

	m_warnAmmo		= false;
	m_warnHealth	= false;

	m_bFadedOut			= false;
	m_bDimmed			= false;
	m_flLastEventTime   = 0.0f;
}

/*
==================================================
VidInit
==================================================
*/

void CHUDQuickInfo::VidInit( void )
{
	Init();

	m_icon_c = gHUD.GetIcon( "crosshair" );
	m_icon_rb = gHUD.GetIcon( "crosshair_right_full" );
	m_icon_lb = gHUD.GetIcon( "crosshair_left_full" );
	m_icon_rbe = gHUD.GetIcon( "crosshair_right_empty" );
	m_icon_lbe = gHUD.GetIcon( "crosshair_left_empty" );

	m_icon_rbn = gHUD.GetIcon( "crosshair_right" );
	m_icon_lbn = gHUD.GetIcon( "crosshair_left" );
}

/*
==================================================
DrawWarning
==================================================
*/

void CHUDQuickInfo::DrawWarning( int x, int y, CHudTexture *icon, float &time )
{
	float scale	= (int)( fabs(sin(gpGlobals->curtime*8.0f)) * 128.0);

	//Only fade out at the low point of our blink
	if ( time <= (gpGlobals->frametime * 200.0f) )
	{
		if ( scale < 40 )
		{
			time = 0.0f;
			return;
		}
		else
		{
			//Counteract the offset below to survive another frame
			time += (gpGlobals->frametime * 200.0f);
		}
	}
	
	//Update our time
	time -= (gpGlobals->frametime * 200.0f);
	Color caution = gHUD.m_clrCaution;
	caution[3] = scale * 255;

	icon->DrawSelf( x, y, caution );
}


//-----------------------------------------------------------------------------
// Purpose: Checks if the hud element needs to fade out
//-----------------------------------------------------------------------------
void CHUDQuickInfo::OnThink()
{
	BaseClass::OnThink();

	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( player == NULL )
		return;

	// see if we should fade in/out
	bool bFadeOut = player->IsZoomed();

	// check if the state has changed
	if ( m_bFadedOut != bFadeOut )
	{
		m_bFadedOut = bFadeOut;

		m_bDimmed = false;

		if ( bFadeOut )
		{
			g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( this, "Alpha", 0.0f, 0.0f, 0.25f, vgui::AnimationController::INTERPOLATOR_LINEAR );
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( this, "Alpha", QUICKINFO_BRIGHTNESS_FULL, 0.0f, QUICKINFO_FADE_IN_TIME, vgui::AnimationController::INTERPOLATOR_LINEAR );
		}
	}
	else if ( !m_bFadedOut )
	{
		// If we're dormant, fade out
		if ( EventTimeElapsed() )
		{
			if ( !m_bDimmed )
			{
				m_bDimmed = true;
				g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( this, "Alpha", QUICKINFO_BRIGHTNESS_DIM, 0.0f, QUICKINFO_FADE_OUT_TIME, vgui::AnimationController::INTERPOLATOR_LINEAR );
			}
		}
		else if ( m_bDimmed )
		{
			// Fade back up, we're active
			m_bDimmed = false;
			g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( this, "Alpha", QUICKINFO_BRIGHTNESS_FULL, 0.0f, QUICKINFO_FADE_IN_TIME, vgui::AnimationController::INTERPOLATOR_LINEAR );
		}
	}
}

/*
==================================================
Draw
==================================================
*/

bool CHUDQuickInfo::ShouldDraw( void )
{
	return ( CHudElement::ShouldDraw() && !engine->IsDrawingLoadingImage() && !engine->IsPaused() );
}

/*
==================================================
Draw
==================================================
*/

void CHUDQuickInfo::Paint()
{
	if ( !m_icon_c || !m_icon_rb || !m_icon_rbe || !m_icon_lb || !m_icon_lbe )
		return;

	int		xCenter	= ( ScreenWidth() - m_icon_c->Width() ) / 2;
	int		yCenter = ( ScreenHeight() - m_icon_c->Height() ) / 2;
	int		scalar = 138;
	
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( player == NULL )
		return;

	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if ( pWeapon == NULL )
		return;

	//Get our values
	int	health	= player->GetHealth();
	int	ammo	= pWeapon->Clip1();

	//Check our health for a warning
	if ( health != m_lastHealth )
	{
		UpdateEventTime();
		m_lastHealth	= health;

		if ( health <= HEALTH_WARNING_THRESHOLD )
		{
			if ( m_warnHealth == false )
			{
				m_healthFade = 255;
				m_warnHealth = true;
				
				CLocalPlayerFilter filter;
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "HUDQuickInfo.LowHealth" );
			}
		}
		else
		{
			m_warnHealth = false;
		}
	}

	// Check our ammo for a warning
	if ( ammo != m_lastAmmo )
	{
		UpdateEventTime();
		m_lastAmmo	= ammo;

		// Find how far through the current clip we are
		float ammoPerc = (float) ammo / (float) pWeapon->GetMaxClip1();

		// Warn if we're below a certain percentage of our clip's size
		if (( pWeapon->GetMaxClip1() > 1 ) && ( ammoPerc <= ( 1.0f - CLIP_PERC_THRESHOLD )))
		{
			if ( m_warnAmmo == false )
			{
				m_ammoFade = 255;
				m_warnAmmo = true;

				CLocalPlayerFilter filter;
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "HUDQuickInfo.LowAmmo" );
			}
		}
		else
		{
			m_warnAmmo = false;
		}
	}

	Color clrNormal = gHUD.m_clrNormal;
	clrNormal[3] = 255 * scalar;
	m_icon_c->DrawSelf( xCenter, yCenter, clrNormal );

	// adjust center for the bigger crosshairs
	xCenter	= ScreenWidth() / 2;
	yCenter = ( ScreenHeight() - m_icon_lb->Height() ) / 2;

	if ( !hud_quickinfo.GetInt() )
	{
		// no quickinfo, just draw the small versions of the crosshairs
		clrNormal[3] = 196;
		m_icon_lbn->DrawSelf(xCenter - (m_icon_lbn->Width() * 2), yCenter, clrNormal);
		m_icon_rbn->DrawSelf(xCenter + m_icon_rbn->Width(), yCenter, clrNormal);
		return;
	}

	int	sinScale = (int)( fabs(sin(gpGlobals->curtime*8.0f)) * 128.0f );

	//Update our health
	if ( m_healthFade > 0.0f )
	{
		DrawWarning( xCenter - (m_icon_lb->Width() * 2), yCenter, m_icon_lb, m_healthFade );
	}
	else
	{
		float	healthPerc = (float) health / 100.0f;

		Color healthColor = m_warnHealth ? gHUD.m_clrCaution : gHUD.m_clrNormal;
		
		if ( m_warnHealth )
		{
			healthColor[3] = 255 * sinScale;
		}
		else
		{
			healthColor[3] = 255 * scalar;
		}
		
		gHUD.DrawIconProgressBar( xCenter - (m_icon_lb->Width() * 2), yCenter, m_icon_lb, m_icon_lbe, ( 1.0f - healthPerc ), healthColor, CHud::HUDPB_VERTICAL );
	}

	//Update our ammo
	if ( m_ammoFade > 0.0f )
	{
		DrawWarning( xCenter + m_icon_rb->Width(), yCenter, m_icon_rb, m_ammoFade );
	}
	else
	{
		float ammoPerc = 1.0f - ( (float) ammo / (float) pWeapon->GetMaxClip1() );

		Color ammoColor = m_warnAmmo ? gHUD.m_clrCaution : gHUD.m_clrNormal;
		
		if ( m_warnAmmo )
		{
			ammoColor[3] = 255 * sinScale;
		}
		else
		{
			ammoColor[3] = 255 * scalar;
		}
		
		gHUD.DrawIconProgressBar( xCenter + m_icon_rb->Width(), yCenter, m_icon_rb, m_icon_rbe, ammoPerc, ammoColor, CHud::HUDPB_VERTICAL );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHUDQuickInfo::UpdateEventTime( void )
{
	m_flLastEventTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHUDQuickInfo::EventTimeElapsed( void )
{
	if (( gpGlobals->curtime - m_flLastEventTime ) > QUICKINFO_EVENT_DURATION )
		return true;

	return false;
}

