//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hud_crosshair.h"
#include "iclientmode.h"
#include "view.h"
#include "vgui_controls/controls.h"
#include "vgui/ISurface.h"
#include "IVRenderView.h"
#include "ff_weapon_base.h"
#include "c_ff_player.h"
#include "ff_utils.h"
#include "ff_crosshairoptions.h"
#include <vgui/IVGui.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar crosshair( "crosshair", "1", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
ConVar cl_observercrosshair( "cl_observercrosshair", "1", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
ConVar cl_acchargebar("cl_acchargebar", "0", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);

//Tie crosshair values to cheats -GreenMushy
ConVar cl_concaim("cl_concaim", "1", FCVAR_ARCHIVE | FCVAR_CLIENTDLL, "0 = always show crosshair in center. 1 = flash trueaim after shooting. 2 = hide crosshair when conced.");
ConVar cl_concaim_fadetime("cl_concaim_fadetime", "0.25", FCVAR_ARCHIVE | FCVAR_CLIENTDLL | FCVAR_CHEAT, "When cl_concaim is 1, controls the time the crosshair stays visible after shooting. Requires sv_cheats 1");
ConVar cl_concaim_showtrueaim("cl_concaim_showtrueaim", "0", FCVAR_CLIENTDLL | FCVAR_CHEAT, "Good way to learn how to concaim. If set to 1, when conced, the crosshair will show exactly where you will shoot. Requires sv_cheats 1");

#define FFDEV_CONCAIM cl_concaim.GetInt()
#define FFDEV_CONCAIM_FADETIME cl_concaim_fadetime.GetFloat()
#define FFDEV_CONCAIM_SHOWTRUEAIM cl_concaim_showtrueaim.GetBool()
	
using namespace vgui;

int ScreenTransform( const Vector& point, Vector& screen );

DECLARE_HUDELEMENT( CHudCrosshair );

CHudCrosshair::CHudCrosshair( const char *pElementName ) :
  CHudElement( pElementName ), BaseClass( NULL, "HudCrosshair" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_pCrosshair = 0;

	m_clrCrosshair = Color( 0, 0, 0, 0 );

	m_vecCrossHairOffsetAngle.Init();

	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_CROSSHAIR );
}

void CHudCrosshair::Init() 
{
	ivgui()->AddTickSignal(GetVPanel(), 500); //only update 2 times a second
}

void CHudCrosshair::OnTick() 
{
	if( g_pCrosshairOptions->IsReady() )
	{
		// we only used the tick so that we would instigate the crosshair options loading
		ivgui()->RemoveTickSignal( GetVPanel() );
	}
}

void CHudCrosshair::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	m_pDefaultCrosshair = gHUD.GetIcon("crosshair_default");
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
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudCrosshair::ShouldDraw( void )
{
	bool bNeedsDraw;

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return false;

	C_FFPlayer *pFFPlayer = ToFFPlayer( pPlayer );

	// Dunno about this... specs might want a crosshair drawn?
	/*if( FF_IsPlayerSpec( pFFPlayer ) || !FF_HasPlayerPickedClass( pFFPlayer ) )
		return false;*/

	// draw a crosshair only if alive or spectating in eye
	if ( IsXbox() )
	{
		bNeedsDraw = m_pCrosshair && 
			!engine->IsDrawingLoadingImage() &&
			!engine->IsPaused() && 
			!pPlayer->IsSuitEquipped() &&
			g_pClientMode->ShouldDrawCrosshair() &&
			!( pPlayer->GetFlags() & FL_FROZEN ) &&
			( pPlayer->entindex() == render->GetViewEntity() ) &&
			( pPlayer->IsAlive() ||	( pPlayer->GetObserverMode() == OBS_MODE_IN_EYE ) || ( cl_observercrosshair.GetBool() && pPlayer->GetObserverMode() == OBS_MODE_ROAMING ) );
	}
	else
	{
		bNeedsDraw = m_pCrosshair && 
			crosshair.GetInt() &&
			!engine->IsDrawingLoadingImage() &&
			!engine->IsPaused() && 
			g_pClientMode->ShouldDrawCrosshair() &&
			!( pPlayer->GetFlags() & FL_FROZEN ) &&
			( pPlayer->entindex() == render->GetViewEntity() ) &&
			( pPlayer->IsAlive() ||	( pPlayer->GetObserverMode() == OBS_MODE_IN_EYE ) || ( cl_observercrosshair.GetBool() && pPlayer->GetObserverMode() == OBS_MODE_ROAMING ) );
	}

	return ( bNeedsDraw && CHudElement::ShouldDraw() );
}

extern void GetCrosshair(FFWeaponID iWeapon, char &innerChar, Color &innerCol, int &innerSize, char &outerChar, Color &outerCol, int &outerSize);	// |-- Mirv

void CHudCrosshair::Paint( void )
{
	if ( !m_pCrosshair )
		return;

	if ( !IsCurrentViewAccessAllowed() )
		return;

	C_FFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

	if (!pPlayer)
		return;

	C_FFPlayer *pActivePlayer = pPlayer;

	// if we're speccing someone, then treat them as the player
	if (pPlayer->IsObserver() && pPlayer->GetObserverMode() == OBS_MODE_IN_EYE)
		pActivePlayer = ToFFPlayer(pPlayer->GetObserverTarget());

	if (!pActivePlayer)	
		return;

	m_curViewAngles = CurrentViewAngles();
	m_curViewOrigin = CurrentViewOrigin();

	float x, y;
	x = ScreenWidth()/2;
	y = ScreenHeight()/2;

	float x_chargebar, y_chargebar;
	x_chargebar = ScreenWidth()/2;
	y_chargebar = ScreenHeight()/2;

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
		x_chargebar += 0.5f * screen[0] * ScreenWidth() + 0.5f;
		y_chargebar += 0.5f * screen[1] * ScreenHeight() + 0.5f;
	}

	// AfterShock: Conc aim -> plot crosshair properly
	if ( ( FFDEV_CONCAIM_SHOWTRUEAIM ) && ( (pActivePlayer->m_flConcTime > gpGlobals->curtime) || (pActivePlayer->m_flConcTime < 0) ) )
	{
		QAngle angles;
		Vector forward;
		Vector point, screen;

		// this code is wrong
		// AfterShock: No, the code is now right!
		angles = pActivePlayer->EyeAngles();
		AngleVectors( angles, &forward );
		forward *= 10000.0f;
		VectorAdd( m_curViewOrigin, forward, point );
		ScreenTransform( point, screen );

		x = (screen[0]*0.5 + 0.5f) * ScreenWidth();
		y = (1 - ( screen[1]*0.5 + 0.5f ) ) * ScreenHeight();
		x_chargebar = x;
		y_chargebar = y;
	}
	// hide crosshair
	else if ( ( FFDEV_CONCAIM == 2) && ( (pActivePlayer->m_flConcTime > gpGlobals->curtime) || (pActivePlayer->m_flConcTime < 0) ) )
	{
		x = -1;
		y = -1;
	}
	// flash crosshair
	else if ( ( FFDEV_CONCAIM == 1) && ( (pActivePlayer->m_flConcTime > gpGlobals->curtime) || (pActivePlayer->m_flConcTime < 0) ) )
	{
		// if should be flashing
		if (gpGlobals->curtime < pActivePlayer->m_flTrueAimTime + FFDEV_CONCAIM_FADETIME)
		{
			QAngle angles;
			Vector forward;
			Vector point, screen;

			// this code is wrong
			// AfterShock: No, the code is now right!
			angles = pActivePlayer->EyeAngles();
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

	C_FFWeaponBase *pWeapon = pActivePlayer->GetActiveFFWeapon();

	// No crosshair for no weapon
	if (!pWeapon)
		return;

	FFWeaponID weaponID = pWeapon->GetWeaponID();

	// Weapons other than these don't get crosshairs
	if (weaponID <= FF_WEAPON_NONE || weaponID > FF_WEAPON_TOMMYGUN)
		return;

	Color innerCol, outerCol;
	char innerChar, outerChar;
	int innerSize, outerSize;
	wchar_t unicode[2];

	//
	// TODO: Clean this up!!!!
	//

	HFont currentFont;
	GetCrosshair(weaponID, innerChar, innerCol, innerSize, outerChar, outerCol, outerSize);

	// concaim 1 = flash xhair when shooting
	if ( ( FFDEV_CONCAIM == 1) && ( (pActivePlayer->m_flConcTime > gpGlobals->curtime) || (pActivePlayer->m_flConcTime < 0) ) )
	{
		//Get the weapon and see if you should draw the crosshair while conced
		if( weaponID == FF_WEAPON_ASSAULTCANNON || 
			weaponID == FF_WEAPON_SUPERNAILGUN || 
			weaponID == FF_WEAPON_FLAMETHROWER || 
			weaponID == FF_WEAPON_NAILGUN ||
			weaponID == FF_WEAPON_AUTORIFLE)
		{
			//If it was one of these weapons, just return before it tries to draw anything
			return;
		}

		// calculate alphas
		float flFlashAlpha = clamp(1.0f - (gpGlobals->curtime - pActivePlayer->m_flTrueAimTime)/FFDEV_CONCAIM_FADETIME, 0.0f, 1.0f);
		// set alphas
		outerCol[3] *= flFlashAlpha;
		innerCol[3] *= flFlashAlpha;
	}

	currentFont = m_hSecondaryCrosshairs[clamp(outerSize, 1, CROSSHAIR_SIZES) - 1];

	surface()->DrawSetTextColor(outerCol.r(), outerCol.g(), outerCol.b(), outerCol.a());
	surface()->DrawSetTextFont(currentFont);

	int charOffsetX = surface()->GetCharacterWidth(currentFont, outerChar) / 2;
	int charOffsetY = surface()->GetFontTall(currentFont) / 2;

	swprintf(unicode, L"%c", outerChar);

	surface()->DrawSetTextPos(x - charOffsetX, y - charOffsetY);
	surface()->DrawUnicodeChar(unicode[0]);

	currentFont = m_hPrimaryCrosshairs[clamp(innerSize, 1, CROSSHAIR_SIZES) - 1];

	surface()->DrawSetTextColor(innerCol.r(), innerCol.g(), innerCol.b(), innerCol.a());
	surface()->DrawSetTextFont(currentFont);

	charOffsetX = surface()->GetCharacterWidth(currentFont, innerChar) / 2;
	charOffsetY = surface()->GetFontTall(currentFont) / 2;

	swprintf(unicode, L"%c", innerChar);

	surface()->DrawSetTextPos(x - charOffsetX, y - charOffsetY);
	surface()->DrawUnicodeChar(unicode[0]);
	// <-- Mirv

	// Mulch: Draw charge bar!
	if( (weaponID == FF_WEAPON_HOOKGUN) )
	{
		Vector vecForward;
		pActivePlayer->EyeVectors( &vecForward );

		VectorNormalize( vecForward );

		// Get eye position
		Vector vecOrigin = pActivePlayer->EyePosition();

		trace_t tr;
		UTIL_TraceLine( vecOrigin, vecOrigin + ( vecForward * 1000.0f ), MASK_SOLID, pActivePlayer, COLLISION_GROUP_DEBRIS, &tr );

		// If we hit something...
		if( tr.DidHit() )
		{
			int iLeft = x_chargebar - charOffsetX;
			int iTop = y_chargebar + charOffsetY;
			int iRight = iLeft + (charOffsetX * 2);
			int iBottom = iTop + 10;

			surface()->DrawSetColor( innerCol.r(), innerCol.g(), innerCol.b(), 150 );
			surface()->DrawFilledRect( iLeft, iTop, iRight, iBottom );

			surface()->DrawSetColor( outerCol.r(), outerCol.g(), outerCol.b(), 200 );		
			surface()->DrawOutlinedRect( iLeft, iTop, iRight, iBottom );
		}
	}
	else if( (weaponID == FF_WEAPON_ASSAULTCANNON) && (cl_acchargebar.GetBool()) )
	{
		extern float GetAssaultCannonCharge();
		float flCharge = GetAssaultCannonCharge();

		if( flCharge <= 0.0f )
			return;

		int iLeft = x_chargebar - charOffsetX;
		int iTop = y_chargebar + charOffsetY;
		int iRight = iLeft + (charOffsetX * 2);
		int iBottom = iTop + 10;

		surface()->DrawSetColor( innerCol.r(), innerCol.g(), innerCol.b(), 150 );
		surface()->DrawFilledRect( iLeft, iTop, iLeft + ((float)(iRight - iLeft) * (flCharge / 100.0f)), iBottom );

		surface()->DrawSetColor( outerCol.r(), outerCol.g(), outerCol.b(), 200 );		
		surface()->DrawOutlinedRect( iLeft, iTop, iRight, iBottom );
	}
	else if( weaponID == FF_WEAPON_SNIPERRIFLE )
	{
		extern float GetSniperRifleCharge();
		float flCharge = GetSniperRifleCharge();

		if( flCharge <= 1.0f )
			return;

		int iLeft = x_chargebar - charOffsetX;
		int iTop = y_chargebar + charOffsetY;
		int iRight = iLeft + (charOffsetX * 2);
		int iBottom = iTop + 10;

		surface()->DrawSetColor( innerCol.r(), innerCol.g(), innerCol.b(), 150 );
		surface()->DrawFilledRect( iLeft, iTop, iLeft + ((float)(iRight - iLeft) * (flCharge / 100.0f)), iBottom );

		surface()->DrawSetColor( outerCol.r(), outerCol.g(), outerCol.b(), 200 );		
		surface()->DrawOutlinedRect( iLeft, iTop, iRight, iBottom );
	}
	else if ( weaponID == FF_WEAPON_JUMPDOWN )
	{
		extern float GetJumpdownCharge();
		float flCharge = GetJumpdownCharge();
		
		if( flCharge <= 0.0f )
			return;

		int iLeft = x_chargebar - charOffsetX;
		int iTop = y_chargebar + charOffsetY;
		int iRight = iLeft + (charOffsetX * 2);
		int iBottom = iTop + 10;

		if ( flCharge == 1.0f )
		{
			surface()->DrawSetColor( 128, 255, 64, 150 );
		}
		else
		{
			surface()->DrawSetColor( innerCol.r(), innerCol.g(), innerCol.b(), 150 );
		}
		surface()->DrawFilledRect( iLeft, iTop, iLeft + ((float)(iRight - iLeft) * (flCharge)), iBottom );

		surface()->DrawSetColor( outerCol.r(), outerCol.g(), outerCol.b(), 200 );		
		surface()->DrawOutlinedRect( iLeft, iTop, iRight, iBottom );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCrosshair::SetCrosshairAngle( const QAngle& angle )
{
	VectorCopy( angle, m_vecCrossHairOffsetAngle );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCrosshair::SetCrosshair( CHudTexture *texture, Color& clr )
{
	m_pCrosshair = texture;
	m_clrCrosshair = clr;
}

//-----------------------------------------------------------------------------
// Purpose: Resets the crosshair back to the default
//-----------------------------------------------------------------------------
void CHudCrosshair::ResetCrosshair()
{
	SetCrosshair( m_pDefaultCrosshair, Color(255, 255, 255, 255) );
}
