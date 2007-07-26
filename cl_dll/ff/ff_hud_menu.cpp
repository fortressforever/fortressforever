/********************************************************************
	created:	2006/02/12
	created:	12:2:2006   1:03
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\ff_hud_menu.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll\ff
	file base:	ff_hud_menu
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "ff_hud_hint.h"
#include "hudelement.h"
#include "hud_macros.h"

#include "ff_hud_menu.h"
#include "mathlib.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include "c_ff_player.h"
#include "c_ff_hint_timers.h"  // For the disguise hint timer

#include <vgui/ILocalize.h>

#include "ff_hud_chat.h"
#include <igameresources.h>

using namespace vgui;

CHudContextMenu *g_pHudContextMenu = NULL;

extern ConVar sensitivity;
extern ConVar ffdev_spy_scloak_minstartvelocity;

ConVar cm_capturemouse("cl_cmcapture", "1", FCVAR_ARCHIVE, "Context menu captures mouse");
ConVar cm_showmouse("cl_cmshowmouse", "1", FCVAR_ARCHIVE, "Show mouse position");
ConVar cm_size("cl_cmsize", "120", FCVAR_ARCHIVE, "Size of context radial menu");
ConVar cm_bounds("cl_cmbounds", "120", FCVAR_ARCHIVE, "Bounds of the context radial menu");
ConVar cm_progresstime("cl_cmprogresstime", "0.3", FCVAR_ARCHIVE, "Time to wait for menu progress");
ConVar cm_squash("cl_cmsquash", "0.7", FCVAR_ARCHIVE, "");
ConVar cm_highlightdistance("cl_cmhighlightdistance", "50", FCVAR_ARCHIVE, "Distance for an option to highlight");

ConVar cm_aimsentry( "cl_noradialaimsentry", "0", 0, "0 - Aim sentry when selecting option in context menu or 1 - aiming AFTER selecting option in context menu" );

#define SCREEN_MIDDLE_X	320
#define SCREEN_MIDDLE_Y	240

#define SCREEN_MAX_X	640
#define SCREEN_MAX_Y	480

DECLARE_HUDELEMENT(CHudContextMenu);

// Forward declarations for a menu
extern menuoption_t SpyClassDisguise[];

inline int CheckDisguiseClass( int iClass )
{
	IGameResources *pGr = GameResources();

	if( !g_pHudContextMenu || !pGr || !g_pHudContextMenu->GetPrevCmd() )
		return MENU_DIM;

	// This is like "disguise friendly", or "disguise red" so
	// grab the part after "disguise"
	const char *pszTeam = g_pHudContextMenu->GetPrevCmd() + 9;

	Assert( pszTeam );

	// Remeber, pszTeam is going to have a space at the end

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return MENU_DIM;

	if( !pPlayer->IsDisguisable() )
		return MENU_DIM;

	int iTeam = pPlayer->GetTeamNumber();
	int iDisguiseTeam = TEAM_UNASSIGNED;

	if( !Q_strcmp( pszTeam, "friendly " ) )
		iDisguiseTeam = iTeam;
	else if( !Q_strcmp( pszTeam, "enemy " ) )
	{
		// This implies there are only
		// 2 teams on this map. iTeam &
		// something else. Find that
		// something else!

		for( int i = TEAM_BLUE; ( i <= TEAM_GREEN ) && ( iDisguiseTeam == TEAM_UNASSIGNED ); i++ )
			if( ( pGr->GetTeamLimits( i ) != -1 ) && ( i != iTeam ) )
				iDisguiseTeam = i;
	}
	else if( !Q_strcmp( pszTeam, "red " ) )
		iDisguiseTeam = TEAM_RED;
	else if( !Q_strcmp( pszTeam, "blue " ) )
		iDisguiseTeam = TEAM_BLUE;
	else if( !Q_strcmp( pszTeam, "yellow " ) )
		iDisguiseTeam = TEAM_YELLOW;
	else if( !Q_strcmp( pszTeam, "green " ) )
		iDisguiseTeam = TEAM_GREEN;

	// Bail if we didn't get a disguise team
	if( iDisguiseTeam == TEAM_UNASSIGNED )
		return MENU_DIM;

	// Check the class limit for the disguise team
	// Mirv: Hide rather than dim
	if( pGr->GetTeamClassLimits( iDisguiseTeam, iClass ) == -1 )
		return MENU_HIDE;

	return MENU_SHOW;
}


/************************************************************************/
/* These are all possible menu options                                  */
/************************************************************************/
ADD_MENU_OPTION(builddispenser, L"Build Dispenser", 'G', "builddispensers")
{
	C_FFPlayer *ff = C_FFPlayer::GetLocalFFPlayer();

	// Yeah, this is highly unlikely to happen, but just checking anyway
	if( !ff )
		return MENU_DIM;

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( ff->IsBuilding() && ( ff->GetCurBuild() == FF_BUILD_DISPENSER ) )
		return MENU_DIM;

	if (ff->GetDispenser())
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION(detdispenser, L"Detonate Dispenser", 'G', "detdispenser")
{
	C_FFPlayer *ff = C_FFPlayer::GetLocalFFPlayer();

	// Yeah, this is highly unlikely to happen, but just checking anyway
	if( !ff )
		return MENU_DIM;

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( ff->IsBuilding() && ( ff->GetCurBuild() == FF_BUILD_DISPENSER ) )
		return MENU_DIM;

	if (!ff->GetDispenser())
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION(dismantledispenser, L"Dismantle Dispenser", 'G', "dismantledispenser")
{
	C_FFPlayer *ff = C_FFPlayer::GetLocalFFPlayer();

	// Yeah, this is highly unlikely to happen, but just checking anyway
	if( !ff )
		return MENU_DIM;

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( ff->IsBuilding() && ( ff->GetCurBuild() == FF_BUILD_DISPENSER ) )
		return MENU_DIM;

	if (!ff->GetDispenser())
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION(buildsentry, L"Build Sentry", 'G', "buildsentry")
{
	C_FFPlayer *ff = C_FFPlayer::GetLocalFFPlayer();

	// Yeah, this is highly unlikely to happen, but just checking anyway
	if( !ff )
		return MENU_DIM;

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( ff->IsBuilding() && ( ff->GetCurBuild() == FF_BUILD_SENTRYGUN ) )
		return MENU_DIM;

	if (ff->GetDispenser())
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION(detsentry, L"Detonate Sentry", 'G', "detsentry")
{
	C_FFPlayer *ff = C_FFPlayer::GetLocalFFPlayer();

	// Yeah, this is highly unlikely to happen, but just checking anyway
	if( !ff )
		return MENU_DIM;

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( ff->IsBuilding() && ( ff->GetCurBuild() == FF_BUILD_SENTRYGUN ) )
		return MENU_DIM;

	if (!ff->GetSentryGun())
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION(dismantlesentry, L"Dismantle Sentry", 'G', "dismantlesentry")
{
	C_FFPlayer *ff = C_FFPlayer::GetLocalFFPlayer();

	// Yeah, this is highly unlikely to happen, but just checking anyway
	if( !ff )
		return MENU_DIM;

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( ff->IsBuilding() && ( ff->GetCurBuild() == FF_BUILD_SENTRYGUN ) )
		return MENU_DIM;

	if (!ff->GetSentryGun())
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION(aimsentry, L"Aim Sentry", 'G', "aimsentry")
{
	C_FFPlayer *ff = C_FFPlayer::GetLocalFFPlayer();

	// Yeah, this is highly unlikely to happen, but just checking anyway
	if( !ff )
		return MENU_DIM;

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( ff->IsBuilding() && ( ff->GetCurBuild() == FF_BUILD_SENTRYGUN ) )
		return MENU_DIM;

	if (!ff->GetSentryGun())
		return MENU_DIM;

	return MENU_SHOW;
}

// These act as intermediate menus
ADD_MENU_BRANCH(disguiseteam, L"Disguise as friendly", 'G', "disguise friendly ", SpyClassDisguise)
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return MENU_DIM;

	if( !pPlayer->IsDisguisable() )
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_BRANCH(disguiseenemy, L"Disguise as enemy", 'G', "disguise enemy ", SpyClassDisguise)
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return MENU_DIM;

	if( !pPlayer->IsDisguisable() )
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_BRANCH(disguisered, L"Disguise as red", 'G', "disguise red ", SpyClassDisguise)
{
	IGameResources *pGr = GameResources();
	if( !pGr )
		return MENU_DIM;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return MENU_DIM;

	if( !pPlayer->IsDisguisable() )
		return MENU_DIM;

	if (pGr->GetTeamLimits(TEAM_RED) >= 0)
		return MENU_SHOW;

	return MENU_DIM;
}

ADD_MENU_BRANCH(disguiseblue, L"Disguise as blue", 'G', "disguise blue ", SpyClassDisguise)
{
	IGameResources *pGr = GameResources();
	if( !pGr )
		return MENU_DIM;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return MENU_DIM;

	if( !pPlayer->IsDisguisable() )
		return MENU_DIM;

	if (pGr->GetTeamLimits(TEAM_BLUE) >= 0)
		return MENU_SHOW;

	return MENU_DIM;

}

ADD_MENU_BRANCH(disguiseyellow, L"Disguise as yellow", 'G', "disguise yellow ", SpyClassDisguise)
{
	IGameResources *pGr = GameResources();
	if( !pGr )
		return MENU_DIM;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return MENU_DIM;

	if( !pPlayer->IsDisguisable() )
		return MENU_DIM;

	if (pGr->GetTeamLimits(TEAM_YELLOW) >= 0)
		return MENU_SHOW;

	return MENU_DIM;

}

ADD_MENU_BRANCH(disguisegreen, L"Disguise as green", 'G', "disguise green ", SpyClassDisguise)
{
	IGameResources *pGr = GameResources();
	if( !pGr )
		return MENU_DIM;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return MENU_DIM;

	if( !pPlayer->IsDisguisable() )
		return MENU_DIM;

	if (pGr->GetTeamLimits(TEAM_GREEN) >= 0)
		return MENU_SHOW;

	return MENU_DIM;

}

ADD_MENU_OPTION(disguisescout, L"Disguise as scout", '!', "scout")
{
	return CheckDisguiseClass( CLASS_SCOUT );
}

ADD_MENU_OPTION(disguisesniper, L"Disguise as sniper", '@', "sniper")
{
	return CheckDisguiseClass( CLASS_SNIPER );
}

ADD_MENU_OPTION(disguisesoldier, L"Disguise as soldier", '#', "soldier")
{
	return CheckDisguiseClass( CLASS_SOLDIER );
}

ADD_MENU_OPTION(disguisedemoman, L"Disguise as demoman", '$', "demoman")
{
	return CheckDisguiseClass( CLASS_DEMOMAN );
}

ADD_MENU_OPTION(disguisemedic, L"Disguise as medic", '%', "medic")
{
	return CheckDisguiseClass( CLASS_MEDIC );
}

ADD_MENU_OPTION(disguisehwguy, L"Disguise as hwguy", '^', "hwguy")
{
	return CheckDisguiseClass( CLASS_HWGUY );
}

ADD_MENU_OPTION(disguisespy, L"Disguise as spy", '*', "spy")
{
	return CheckDisguiseClass( CLASS_SPY );
}

ADD_MENU_OPTION(disguisepyro, L"Disguise as pyro", '?', "pyro")
{
	return CheckDisguiseClass( CLASS_PYRO );
}

ADD_MENU_OPTION(disguiseengineer, L"Disguise as engineer", '(', "engineer")
{
	return CheckDisguiseClass( CLASS_ENGINEER );
}

ADD_MENU_OPTION(disguisecivilian, L"Disguise as civilian", ')', "civilian")
{
	return CheckDisguiseClass( CLASS_CIVILIAN );
}

//-----------------------------------------------------------------------------
// Detpack menu options
//-----------------------------------------------------------------------------
ADD_MENU_OPTION( det5, L"5", NULL, "detpack 5" )
{
	return MENU_SHOW;
}

ADD_MENU_OPTION( det10, L"10", NULL, "detpack 10" )
{
	return MENU_SHOW;
}

ADD_MENU_OPTION( det20, L"20", NULL, "detpack 20" )
{
	return MENU_SHOW;
}

ADD_MENU_OPTION( det50, L"50", NULL, "detpack 50" )
{
	return MENU_SHOW;
}

//-----------------------------------------------------------------------------
// Cloak options
//-----------------------------------------------------------------------------
ADD_MENU_OPTION( cloak, L"Cloak", 'G', "cloak" )
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return MENU_DIM;

	// Jon: always allow uncloaking if already cloaked
	if (pPlayer->IsCloaked())
		return MENU_SHOW;

	if( !pPlayer->IsCloakable() )
		return MENU_DIM;

	// 0001379: Must be on the ground to cloak
	// added: or also not swimming
	if ( !(pPlayer->GetFlags() & FL_ONGROUND || pPlayer->GetWaterLevel() > WL_NotInWater) )
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION( scloak, L"Silent Cloak", 'G', "scloak" )
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return MENU_DIM;

	// Jon: always allow uncloaking if already cloaked
	if (pPlayer->IsCloaked())
		return MENU_SHOW;

	if( !pPlayer->IsCloakable() )
		return MENU_DIM;

	// 0001379: Must be on the ground to cloak
	// added: or also not swimming
	if ( !(pPlayer->GetFlags() & FL_ONGROUND || pPlayer->GetWaterLevel() > WL_NotInWater) )
		return MENU_DIM;

	// Jon: adding in minimum allowed speed cvar
	if( pPlayer->GetCloakSpeed() > ffdev_spy_scloak_minstartvelocity.GetFloat() )
		return MENU_DIM;

	return MENU_SHOW;
}

//-----------------------------------------------------------------------------
// Sentry Sabotage
//-----------------------------------------------------------------------------

ADD_MENU_OPTION( sentrysabotage, L"Sabotage Sentry", 'G', "sentrysabotage" )
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return MENU_DIM;

	if ( pPlayer->AnyActiveSGSabotages() )
		return MENU_SHOW;
	else return MENU_DIM;
}

/************************************************************************/
/* And these are the actual menus themselves                            */
/************************************************************************/
menuoption_t BuildMenu[]		= { detdispenser, dismantledispenser, detsentry, dismantlesentry, aimsentry };
menuoption_t SpyTeamDisguise2[] = { disguiseenemy, disguiseteam, scloak, cloak, sentrysabotage };
menuoption_t SpyTeamDisguise4[] = { disguiseblue, disguisered, disguiseyellow, disguisegreen, scloak, cloak, sentrysabotage };
menuoption_t SpyClassDisguise[] = { disguisescout, disguisesniper, disguisesoldier, disguisedemoman, disguisemedic, disguisehwguy, disguisepyro, disguisespy, disguiseengineer, disguisecivilian };
menuoption_t DemoDetpackMenu[]	= { det5, det10, det20, det50 };


CHudContextMenu::~CHudContextMenu() 
{
}

void CHudContextMenu::VidInit() 
{
	m_fVisible = false;

	g_pHudContextMenu = this;

	SetPaintBackgroundEnabled(false);
	m_iIcon = 0;

	// Precache the background texture
	m_pHudElementTexture = new CHudTexture();
	m_pHudElementTexture->textureId = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_pHudElementTexture->textureId, "vgui/hud_button", true, false);
}

void CHudContextMenu::Init() 
{
}

void CHudContextMenu::DoCommand(const char *cmd)
{
	if (!m_pszPreviousCmd)
	{
		if( cm_aimsentry.GetBool() && ( strcmp( cmd, "aimsentry" ) == 0 ) )
		{
			// Special case for aimsentry - we "bind" attack1 to
			// aimsentry so they can NOW click anywhere to aim
			C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
			if( !pPlayer )
				return;

			SetStealMouseForAimSentry( true );
			ClientPrintMsg( pPlayer, HUD_PRINTCENTER, "#FF_AIMSENTRY" );
		}
		else
			engine->ClientCmd(cmd);
	}

	// Currently this only supports has 2 levels of menu
	// If we need more, change m_pszPreviousCmd to a character array
	// and concatonate on each menu item's command each time 
	// we progress to next menu
	else
	{
		char buf[256];
		Q_snprintf(buf, 255, "%s%s", m_pszPreviousCmd, cmd);
		engine->ClientCmd(buf);

		//  Jiggles: The player used the menu to disguise!  Good for him/her!
		//				Note: This logic assumes there is only disguise functionality in our 2nd menu level				
		g_FFHintTimers.DeleteTimer( "DisHint" );
	}
}

void CHudContextMenu::Display(bool state)
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if (!pPlayer)
		return;

	// There is a menu and it's cancelling
	if (m_pMenu && m_fVisible == true && state == false)
	{
		if (m_iSelected >= 0)
		{
			if (m_pMenu[m_iSelected].conditionfunc() == MENU_SHOW)
				DoCommand(m_pMenu[m_iSelected].szCommand);
		}
		else if ( pPlayer->GetClassSlot() == CLASS_DEMOMAN ) // So the demoman can set a det by just clicking
			engine->ClientCmd("detpack 5");

		m_fVisible = state;
		return;
	}

	m_fVisible = state;

	// Clear any previous commands
	m_pszPreviousCmd = NULL;

	// Decide which menu is to be shown
	if (pPlayer->GetClassSlot() == CLASS_ENGINEER)
	{
		m_pMenu = &BuildMenu[0];
		m_nOptions = sizeof(BuildMenu) / sizeof(BuildMenu[0]);
	}
	else if (pPlayer->GetClassSlot() == CLASS_SPY)
	{
		int nTeams = 0;
		IGameResources *pGr = GameResources();

		if (!pGr)
		{
			AssertMsg(0, "Can't get GameResources");
			return;
		}

		for (int iTeam = TEAM_BLUE; iTeam <= TEAM_GREEN; iTeam++)
		{
			if (pGr->GetTeamLimits(iTeam) < 0)
				nTeams++;
		}

		if (nTeams == 2)
		{
			m_pMenu = &SpyTeamDisguise2[0];
			m_nOptions = sizeof(SpyTeamDisguise2) / sizeof(SpyTeamDisguise2[0]);
		}
		else
		{
			m_pMenu = &SpyTeamDisguise4[0];
			m_nOptions = sizeof(SpyTeamDisguise4) / sizeof(SpyTeamDisguise4[0]);
		}
	}
	else if( pPlayer->GetClassSlot() == CLASS_DEMOMAN )
	{
		m_pMenu = &DemoDetpackMenu[ 0 ];
		m_nOptions = sizeof( DemoDetpackMenu ) / sizeof( DemoDetpackMenu[ 0 ] );
	}

	SetMenu();
}

void CHudContextMenu::SetMenu()
{
	m_flSelectStart = gpGlobals->curtime;

	float midx = scheme()->GetProportionalScaledValue(SCREEN_MIDDLE_X);
	float midy = scheme()->GetProportionalScaledValue(SCREEN_MIDDLE_Y);
	float dist = scheme()->GetProportionalScaledValue(cm_size.GetInt());

	m_flPosX = midx;
	m_flPosY = midy;

	// Get positions for buttons
	for (int i = 0; i < m_nOptions; i++)
	{
		float increments = (float) i * DEG2RAD(360) / (float) m_nOptions;

		m_flPositions[i][0] = midx + dist * sin(increments);
		m_flPositions[i][1] = midy - dist * cos(increments) * cm_squash.GetFloat();
	}

	// Big enough to fit whole circle
	SetWide(scheme()->GetProportionalScaledValue(SCREEN_MAX_X));
	SetTall(scheme()->GetProportionalScaledValue(SCREEN_MAX_Y));

	// Position in centre
	SetPos(scheme()->GetProportionalScaledValue(SCREEN_MIDDLE_X) - GetWide() * 0.5f, scheme()->GetProportionalScaledValue(SCREEN_MIDDLE_Y) - GetTall() * 0.5f);
}

void CHudContextMenu::Paint() 
{
	if (!m_fVisible) 
		return;

	if (m_flDuration == 0) 
		m_flDuration = 0.001f;

	/*if (gpGlobals->curtime > m_flStartTime + m_flDuration + 1.5f) 
	{
		// Begin to fade
		if (m_fVisible) 
		{
			m_fVisible = false;
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutBuildTimer");
		}
		// Fading time is over
		else if (gpGlobals->curtime > m_flStartTime + m_flDuration + 1.7f) 
		{
			return;
		}
	}*/

/*	float halfbuttonX = scheme()->GetProportionalScaledValue(40.0f);
	float halfbuttonY = scheme()->GetProportionalScaledValue(20.0f);

	// Button boxes
	surface()->DrawSetTexture(m_pHudElementTexture->textureId);
	surface()->DrawSetColor(255, 255, 255, 255);

	// Draw boxes
	for (int i = 0; i < m_nOptions; i++)
		surface()->DrawTexturedRect(m_flPositions[i][0] - halfbuttonX, m_flPositions[i][1] - halfbuttonY, m_flPositions[i][0] + halfbuttonX, m_flPositions[i][1] + halfbuttonY);*/

	// Colours we need later on
	Color highlighted(255, 0, 0, 255);
	Color dimmed(100, 100, 100, 255);

	float dx = m_flPosX - scheme()->GetProportionalScaledValue(SCREEN_MIDDLE_X);
	float dy = m_flPosY - scheme()->GetProportionalScaledValue(SCREEN_MIDDLE_Y);
	float py = scheme()->GetProportionalScaledValue(10.0f);

	int newSelection = -1;

	// Check to see if we're far enough from the middle to be highlighting a button
	float flDistance = FastSqrt(dx * dx + dy * dy);

	if (flDistance > 0.0f)
	{
		float flHighlightBoundary = scheme()->GetProportionalScaledValue(cm_highlightdistance.GetFloat());
		float s = fabs(dy) / flDistance;
		flHighlightBoundary = (1.0f - s) * flHighlightBoundary + s * (flHighlightBoundary * cm_squash.GetFloat());

		// Outside the inside boundary, therefore work out which button to highlight
		if (flDistance > flHighlightBoundary)
		{
			const float pi_2 = M_PI * 2.0f;

			float angle_per_button = pi_2 / m_nOptions;
			float angle = atan2f(dx, -dy) + angle_per_button * 0.5f;

			if (angle < 0)
				angle += pi_2;
			if (angle > pi_2)
				angle -= pi_2;

			newSelection = angle / angle_per_button;
		}
	}

	// Draw text for each box
	for (int i = 0; i < m_nOptions; i++)
	{
		//
		// COLOR
		//
		if (m_pMenu[i].conditionfunc() != MENU_SHOW)
			surface()->DrawSetTextColor(dimmed);
		else if (newSelection == i)
			surface()->DrawSetTextColor(highlighted);
		else
			surface()->DrawSetTextColor(GetFgColor());

		//
		// DRAW ICON
		//
		char character = m_pMenu[i].chIcon;

		//surface()->DrawSetTextColor(Color(100, 100, 100, 100));
		surface()->DrawSetTextFont(m_hMenuIcon);

		int charOffsetX = surface()->GetCharacterWidth(m_hMenuIcon, character) / 2;
		int charOffsetY = surface()->GetFontTall(m_hMenuIcon) / 2;

		wchar_t unicode[2];
		swprintf(unicode, L"%c", character);

		surface()->DrawSetTextPos(m_flPositions[i][0] - charOffsetX, m_flPositions[i][1] - charOffsetY);
		surface()->DrawUnicodeChar(unicode[0]);

		//
		// DRAW TEXT
		//

		surface()->DrawSetTextFont(m_hTextFont);

		// Work out centering and position & draw text
		int offsetX = 0.5f * UTIL_ComputeStringWidth(m_hTextFont, m_pMenu[i].szName);
		surface()->DrawSetTextPos(m_flPositions[i][0] - offsetX, m_flPositions[i][1] + charOffsetY + py);
		
		for (const wchar_t *wch = m_pMenu[i].szName; *wch != 0; wch++)
			surface()->DrawUnicodeChar(*wch);
	}

	// Restart timer if a new selection
	if (newSelection != m_iSelected)
		m_flSelectStart = gpGlobals->curtime;

	m_iSelected = newSelection;

	// Progress to next menu if needed
	// TODO: A cleaner way of doing this
	if (m_iSelected > -1 && gpGlobals->curtime > m_flSelectStart + cm_progresstime.GetFloat())
	{
		// There is a next menu for this option
		// ADDED: added check for conditionfunc to be true because
		// dimmed out "branches" could still be selected.
		if (m_pMenu[m_iSelected].pNextMenu && ( m_pMenu[m_iSelected].conditionfunc() == MENU_SHOW ) )
		{
			m_pszPreviousCmd = m_pMenu[m_iSelected].szCommand;
			m_pMenu = &SpyClassDisguise[0];
			m_nOptions = sizeof(SpyClassDisguise) / sizeof(SpyClassDisguise[0]);

			m_iSelected = -1;
			m_flSelectStart = gpGlobals->curtime;

			SetMenu();
		}
	}

	// Show actual mouse location, just manually drawing a crosshair for now
	if (cm_showmouse.GetBool())
	{
		float ch_short = scheme()->GetProportionalScaledValue(1.0f);
		float ch_long = ch_short * 6.0f;

		surface()->DrawSetTexture(NULL);
		surface()->DrawSetColor(GetFgColor());
		//surface()->DrawTexturedRect(m_flPosX - 5, m_flPosY - 5, m_flPosX + 5, m_flPosY + 5);
		surface()->DrawTexturedRect(m_flPosX - ch_short, m_flPosY - ch_long, m_flPosX + ch_short, m_flPosY + ch_long);
		surface()->DrawTexturedRect(m_flPosX - ch_long, m_flPosY - ch_short, m_flPosX + ch_long, m_flPosY + ch_short);
	}
}

void CHudContextMenu::MouseMove(float *x, float *y) 
{
	if (!m_fVisible)
		return;

	float sensitivity_factor = 1.0f / (sensitivity.GetFloat() == 0 ? 0.001f : sensitivity.GetFloat());

	float midx = scheme()->GetProportionalScaledValue(SCREEN_MIDDLE_X);
	float midy = scheme()->GetProportionalScaledValue(SCREEN_MIDDLE_Y);
	float dist = scheme()->GetProportionalScaledValue(cm_bounds.GetInt());

	// UNDONE: Now capturing within a sphere, not within a box
	//m_flPosX = clamp(m_flPosX + (*x * sensitivity_factor), midx - dist, midx + dist);
	//m_flPosY = clamp(m_flPosY + (*y * sensitivity_factor), midy - dist, midy + dist);

	// Add on the new location to our internal mouse position
	m_flPosX += *x * sensitivity_factor;
	m_flPosY += *y * sensitivity_factor;

	// If capturing mouse then reset any x,y movement so that the player stays stationary
	// Turning this off isn't very handy since you don't want to be moving around if you're
	// going for the aim sentry option
	if (cm_capturemouse.GetBool()) 
	{
		*x = 0;
		*y = 0;
	}

	float flOffsetX = m_flPosX - midx;
	float flOffsetY = m_flPosY - midy;

	float flDistance = FastSqrt((flOffsetX * flOffsetX) + (flOffsetY * flOffsetY));

	// Definitely no clamping required here
	if (flDistance == 0.0f)
		return;

	// Need to take squash'ness into account here
	float s = fabs(flOffsetY) / flDistance;
	dist = (1.0f - s) * dist + s * (dist * cm_squash.GetFloat());

	// Make sure we don't go outside the bounds
	if (flDistance > dist)
	{
		flDistance = dist / flDistance;
		m_flPosX = midx + (flOffsetX * flDistance);
		m_flPosY = midy + (flOffsetY * flDistance);
	}
}

void HudContextMenuInput(float *x, float *y) 
{
	if (g_pHudContextMenu) 
		g_pHudContextMenu->MouseMove(x, y);
}

void HudContextShow(bool visible) 
{
	if (g_pHudContextMenu) 
		g_pHudContextMenu->Display(visible);
}