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

//
// WARNING: A LOT OF THIS IS PROTOTYPICAL CODE I.E. VERY ROUGH AND NOT
//			OPTIMISED (OR EVEN SENSIBLE) IN ANY WAY SHAPE OR FORM.
//


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
#include "ff_gamerules.h"

#include <vgui/ILocalize.h>

#include "ff_hud_chat.h"
#include <igameresources.h>

using namespace vgui;

CHudContextMenu *g_pHudContextMenu = NULL;

extern ConVar sensitivity;
extern ConVar ffdev_spy_scloak_minstartvelocity;

// This is a bit of a hacky way to do it!!
char g_szLastDisguiseComment[128] = { 0 };

ConVar cm_usemouse("cl_cmusemouse", "1", FCVAR_ARCHIVE, "Use the mouse for the context menu");
ConVar cm_capturemouse("cl_cmcapture", "1", FCVAR_ARCHIVE, "Context menu captures mouse");
ConVar cm_hidecursor("cl_cmhidecursor", "0", FCVAR_ARCHIVE, "Show mouse cursor");
ConVar cm_size("cl_cmsize", "120", FCVAR_ARCHIVE, "Size of context radial menu");
ConVar cm_bounds("cl_cmbounds", "120", FCVAR_ARCHIVE, "Bounds of the context radial menu");
ConVar cm_progresstime("cl_cmprogresstime", "0.7", FCVAR_ARCHIVE, "Time to wait for menu progress");
ConVar cm_squash("cl_cmsquash", "0.7", FCVAR_ARCHIVE, "");
ConVar cm_highlightdistance("cl_cmhighlightdistance", "50", FCVAR_ARCHIVE, "Distance for an option to highlight");
ConVar cm_waitforrelease("cl_cmwaitforrelease", "1", FCVAR_ARCHIVE, "Menu waits for mouse release before selection");
ConVar cm_defaultactiontime("cl_cmdefaultactiontime", "0.5", FCVAR_ARCHIVE, "Default action takes place if menu closed within this amount of time");

ConVar cm_aimsentry( "cl_noradialaimsentry", "0", 0, "0 - Aim sentry when selecting option in context menu or 1 - aiming AFTER selecting option in context menu" );

#define MAX_CMD_LEN		64

DECLARE_HUDELEMENT(CHudContextMenu);

// Forward declarations for a menu
extern menu_t ClassDMenu;
extern menu_t FriendlyDMenu;
extern menu_t EnemyDMenu;

// We buffer our commands onto here sequentially.
char szCmdBuffer[MAX_CMD_LEN];

CON_COMMAND(qsentry, "qsentry")
{
	C_FFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());
	if (pPlayer)
		pPlayer->SwapToWeaponSlot(5);
}

CON_COMMAND(qdispenser, "qdispenser")
{
	C_FFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());
	if (pPlayer)
		pPlayer->SwapToWeaponSlot(4);
}

int CheckDisguiseClass( int iClass )
{
	IGameResources *pGr = GameResources();

	if( !g_pHudContextMenu || !pGr )
		return MENU_DIM;

	Assert(g_pHudContextMenu->GetLayerNumber() == 2);

	// This is like "disguise friendly", or "disguise red" so
	// grab the part after "disguise"
	const char *pszTeam = szCmdBuffer + 9;

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
	if( pGr->GetTeamClassLimits( iDisguiseTeam, iClass ) == -1 )
		return MENU_DIM;

	return MENU_SHOW;
}


//-----------------------------------------------------------------------------
// Engineer menu options
//-----------------------------------------------------------------------------

ADD_MENU_OPTION(builddispenser, "#FF_CM_BUILDDISPENSER", 'Q', "qdispenser")
{
	C_FFPlayer *ff = C_FFPlayer::GetLocalFFPlayer();

	// Yeah, this is highly unlikely to happen, but just checking anyway
	if( !ff )
		return MENU_DIM;

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( ff->IsBuilding() && ( ff->GetCurrentBuild() == FF_BUILD_DISPENSER ) )
		return MENU_DIM;

	if (ff->GetDispenser())
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION(detdispenser, "#FF_CM_DETDISPENSER", 'P', "detdispenser")
{
	C_FFPlayer *ff = C_FFPlayer::GetLocalFFPlayer();

	// Yeah, this is highly unlikely to happen, but just checking anyway
	if( !ff )
		return MENU_DIM;

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( ff->IsBuilding() && ( ff->GetCurrentBuild() == FF_BUILD_DISPENSER ) )
		return MENU_DIM;

	if (!ff->GetDispenser())
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION(dismantledispenser, "#FF_CM_DISMANTLEDISPENSER", 'Q', "dismantledispenser")
{
	C_FFPlayer *ff = C_FFPlayer::GetLocalFFPlayer();

	// Yeah, this is highly unlikely to happen, but just checking anyway
	if( !ff )
		return MENU_DIM;

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( ff->IsBuilding() && ( ff->GetCurrentBuild() == FF_BUILD_DISPENSER ) )
		return MENU_DIM;

	if (!ff->GetDispenser())
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION(buildsentry, "#FF_CM_BUILDSENTRY", 'S', "qsentry")
{
	C_FFPlayer *ff = C_FFPlayer::GetLocalFFPlayer();

	// Yeah, this is highly unlikely to happen, but just checking anyway
	if( !ff )
		return MENU_DIM;

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( ff->IsBuilding() && ( ff->GetCurrentBuild() == FF_BUILD_SENTRYGUN ) )
		return MENU_DIM;

	if (ff->GetSentryGun())
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION(detsentry, "#FF_CM_DETSENTRY", 'R', "detsentry")
{
	C_FFPlayer *ff = C_FFPlayer::GetLocalFFPlayer();

	// Yeah, this is highly unlikely to happen, but just checking anyway
	if( !ff )
		return MENU_DIM;

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( ff->IsBuilding() && ( ff->GetCurrentBuild() == FF_BUILD_SENTRYGUN ) )
		return MENU_DIM;

	if (!ff->GetSentryGun())
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION(dismantlesentry, "#FF_CM_DISMANTLESENTRY", 'S', "dismantlesentry")
{
	C_FFPlayer *ff = C_FFPlayer::GetLocalFFPlayer();

	// Yeah, this is highly unlikely to happen, but just checking anyway
	if( !ff )
		return MENU_DIM;

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( ff->IsBuilding() && ( ff->GetCurrentBuild() == FF_BUILD_SENTRYGUN ) )
		return MENU_DIM;

	if (!ff->GetSentryGun())
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION(aimsentry, "#FF_CM_AIMSENTRY", 'O', "aimsentry")
{
	C_FFPlayer *ff = C_FFPlayer::GetLocalFFPlayer();

	// Yeah, this is highly unlikely to happen, but just checking anyway
	if( !ff )
		return MENU_DIM;

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( ff->IsBuilding() && ( ff->GetCurrentBuild() == FF_BUILD_SENTRYGUN ) )
		return MENU_DIM;

	if (!ff->GetSentryGun())
		return MENU_DIM;

	return MENU_SHOW;
}

//-----------------------------------------------------------------------------
// Disguise menu options
//-----------------------------------------------------------------------------

ADD_MENU_BRANCH(disguiseteam, "#FF_CM_DISGUISEFRIENDLY", 'M', "", &FriendlyDMenu)
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return MENU_DIM;

	if( !pPlayer->IsDisguisable() )
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_BRANCH(disguiseenemy, "#FF_CM_DISGUISEENEMY", 'N', "", &EnemyDMenu)
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return MENU_DIM;

	if( !pPlayer->IsDisguisable() )
		return MENU_DIM;

	return MENU_SHOW;
}

enum AvailableAs_t { ENEMY, FRIENDLY };

int TeamAvailableForDisguise(int iTeam, AvailableAs_t as)
{
	IGameResources *pGr = GameResources();
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if (!pGr || !pPlayer)
		return MENU_DIM;

	if (!pPlayer->IsDisguisable())
		return MENU_DIM;

	if (pGr->GetTeamLimits(iTeam) < 0)
		return MENU_DIM;

	bool bAllied = FFGameRules()->IsTeam1AlliedToTeam2(pPlayer->GetTeamNumber(), iTeam);

	return ((bAllied == (as == FRIENDLY)) ? MENU_SHOW : MENU_DIM);
}

ADD_MENU_BRANCH(disguise_blue_friendly, "#FF_CM_DISGUISEBLUE", 'T', "disguise blue ", &ClassDMenu) { return TeamAvailableForDisguise(TEAM_BLUE, FRIENDLY); }
ADD_MENU_BRANCH(disguise_red_friendly, "#FF_CM_DISGUISERED", 'U', "disguise red ", &ClassDMenu) { return TeamAvailableForDisguise(TEAM_RED, FRIENDLY); }
ADD_MENU_BRANCH(disguise_yellow_friendly, "#FF_CM_DISGUISEYELLOW", 'V', "disguise yellow ", &ClassDMenu) { return TeamAvailableForDisguise(TEAM_YELLOW, FRIENDLY); }
ADD_MENU_BRANCH(disguise_green_friendly, "#FF_CM_DISGUISEGREEN", 'W', "disguise green ", &ClassDMenu) { return TeamAvailableForDisguise(TEAM_GREEN, FRIENDLY); }
ADD_MENU_BRANCH(disguise_blue_enemy, "#FF_CM_DISGUISEBLUE", 'T', "disguise blue ", &ClassDMenu) { return TeamAvailableForDisguise(TEAM_BLUE, ENEMY); }
ADD_MENU_BRANCH(disguise_red_enemy, "#FF_CM_DISGUISERED", 'U', "disguise red ", &ClassDMenu) { return TeamAvailableForDisguise(TEAM_RED, ENEMY); }
ADD_MENU_BRANCH(disguise_yellow_enemy, "#FF_CM_DISGUISEYELLOW", 'V', "disguise yellow ", &ClassDMenu) { return TeamAvailableForDisguise(TEAM_YELLOW, ENEMY); }
ADD_MENU_BRANCH(disguise_green_enemy, "#FF_CM_DISGUISEGREEN", 'W', "disguise green ", &ClassDMenu) { return TeamAvailableForDisguise(TEAM_GREEN, ENEMY); }

ADD_MENU_OPTION(disguisescout, "#FF_CM_DISGUISESCOUT", '!', "scout") { return CheckDisguiseClass( CLASS_SCOUT ); }
ADD_MENU_OPTION(disguisesniper, "#FF_CM_DISGUISESNIPER", '@', "sniper") {	return CheckDisguiseClass( CLASS_SNIPER ); }
ADD_MENU_OPTION(disguisesoldier, "#FF_CM_DISGUISESOLDIER", '#', "soldier") { return CheckDisguiseClass( CLASS_SOLDIER ); }
ADD_MENU_OPTION(disguisedemoman, "#FF_CM_DISGUISEDEMOMAN", '$', "demoman") { return CheckDisguiseClass( CLASS_DEMOMAN ); }
ADD_MENU_OPTION(disguisemedic, "#FF_CM_DISGUISEMEDIC", '%', "medic") { return CheckDisguiseClass( CLASS_MEDIC ); }
ADD_MENU_OPTION(disguisehwguy, "#FF_CM_DISGUISEHWGUY", '^', "hwguy") { return CheckDisguiseClass( CLASS_HWGUY ); }
ADD_MENU_OPTION(disguisespy, "#FF_CM_DISGUISESPY", '*', "spy") { return CheckDisguiseClass( CLASS_SPY ); }
ADD_MENU_OPTION(disguisepyro, "#FF_CM_DISGUISEPYRO", '?', "pyro") { return CheckDisguiseClass( CLASS_PYRO ); }
ADD_MENU_OPTION(disguiseengineer, "#FF_CM_DISGUISEENGINEER", '(', "engineer") { return CheckDisguiseClass( CLASS_ENGINEER ); }
ADD_MENU_OPTION(disguisecivilian, "#FF_CM_DISGUISECIVILIAN", ')', "civilian") { return CheckDisguiseClass( CLASS_CIVILIAN ); }

ADD_MENU_OPTION(lastdisguise, "#FF_CM_DISGUISELAST", 'J', "disguise last") { return (g_szLastDisguiseComment[0] == 0 ? MENU_DIM : MENU_SHOW); }


//-----------------------------------------------------------------------------
// Detpack menu options
//-----------------------------------------------------------------------------
ADD_MENU_OPTION( det5, "#FF_CM_DETPACK5", 'C', "detpack 5" ) { return MENU_SHOW; }
ADD_MENU_OPTION( det10, "#FF_CM_DETPACK10", 'D', "detpack 10" ) { return MENU_SHOW; }
ADD_MENU_OPTION( det20, "#FF_CM_DETPACK20", 'E', "detpack 20" ) { return MENU_SHOW; }
ADD_MENU_OPTION( det50, "#FF_CM_DETPACK50", 'F', "detpack 50" ) { return MENU_SHOW; }

//-----------------------------------------------------------------------------
// Cloak options
//-----------------------------------------------------------------------------
int CanCloak()
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

ADD_MENU_OPTION( smartcloak, "#FF_CM_SMARTCLOAK", 'A', "smartcloak" ) { return CanCloak(); }
//ADD_MENU_OPTION( scloak, "#FF_CM_SCLOAK", 'B', "scloak" ) { return CanCloak(); }


//-----------------------------------------------------------------------------
// Sentry Sabotage
//-----------------------------------------------------------------------------

ADD_MENU_OPTION( sentrysabotage, "#FF_CM_SABOTAGESENTRY", 'H', "sentrysabotage" )
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return MENU_DIM;

	if ( pPlayer->AnyActiveSentrySabotages() )
		return MENU_SHOW;
	else return MENU_DIM;
}

ADD_MENU_OPTION(dispensersabotage, "#FF_CM_SABOTAGEDISPENSER", 'I', "dispensersabotage")
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if (!pPlayer || !pPlayer->AnyActiveDispenserSabotages())
		return MENU_DIM;

	return MENU_SHOW;
}


//-----------------------------------------------------------------------------
// Medic/Engineer stuff
//-----------------------------------------------------------------------------
ADD_MENU_OPTION( need_armor, "#FF_CM_CALLARMOR", '(', "engyme" ) { return MENU_SHOW; }
ADD_MENU_OPTION( need_medic, "#FF_CM_CALLMEDIC", '%', "saveme" ) { return MENU_SHOW; }
ADD_MENU_OPTION( need_ammo, "#FF_CM_CALLAMMO", '^', "ammome" ) { return MENU_SHOW; }


//-----------------------------------------------------------------------------
// Menu option lists
//-----------------------------------------------------------------------------
MenuOption EngineerOptionList[] = { aimsentry, builddispenser, detdispenser, dismantledispenser, dismantlesentry, detsentry, buildsentry };
MenuOption DemomanOptionList[] = { det5, det10, det20, det50 };
MenuOption SpyOptionList[] = { lastdisguise, disguiseenemy, smartcloak, sentrysabotage, dispensersabotage, disguiseteam };
MenuOption ClassDOptionList[] = { disguisescout, disguisesniper, disguisesoldier, disguisedemoman, disguisemedic, disguisehwguy, disguisepyro, disguisespy, disguiseengineer, disguisecivilian };
MenuOption FriendlyDOptionList[] = { disguise_blue_friendly, disguise_red_friendly, disguise_yellow_friendly, disguise_green_friendly };
MenuOption EnemyDOptionList[] = { disguise_blue_enemy, disguise_red_enemy, disguise_yellow_enemy, disguise_green_enemy };
MenuOption CallOptionList[] = { need_armor, need_medic, need_ammo };

//-----------------------------------------------------------------------------
// Menus themselves
//-----------------------------------------------------------------------------
menu_t EngineerMenu = { ARRAYSIZE(EngineerOptionList), EngineerOptionList, "aimsentry" };
menu_t DemomanMenu = { ARRAYSIZE(DemomanOptionList), DemomanOptionList, "detpack 5" };
menu_t SpyMenu = { ARRAYSIZE(SpyOptionList), SpyOptionList, "smartcloak" };
menu_t ClassDMenu = { ARRAYSIZE(ClassDOptionList), ClassDOptionList, NULL };
menu_t FriendlyDMenu = { ARRAYSIZE(FriendlyDOptionList), FriendlyDOptionList, NULL };
menu_t EnemyDMenu = { ARRAYSIZE(EnemyDOptionList), EnemyDOptionList, NULL };
menu_t CallMenu = { ARRAYSIZE(CallOptionList), CallOptionList, "saveme" };

menu_t Menus[] = { EngineerMenu, DemomanMenu, SpyMenu, ClassDMenu, FriendlyDMenu, EnemyDMenu, CallMenu };

CHudContextMenu::~CHudContextMenu() 
{
}

void CHudContextMenu::VidInit() 
{
	m_fVisible = false;
	g_pHudContextMenu = this;
	SetPaintBackgroundEnabled(false);

	// Set up localisation
	for (int iMenu = 0; iMenu < ARRAYSIZE(Menus); iMenu++)
	{
		for (int iOption = 0; iOption < Menus[iMenu].size; iOption++)
		{
			// It might be safe to just point to the string in the table, but maybe not.
			MenuOption *pOption = &Menus[iMenu].options[iOption];
			wchar_t *localised = localize()->Find(pOption->szName);
			if (localised)
			{
				int len = Q_wcslen(localised) + 1;
				pOption->wszText = new wchar_t [len];
				Q_wcsncpy(pOption->wszText, localised, len * 2);	// Takes length in bytes
			}
			else
			{
				int len = strlen(pOption->szName) + 1;
				pOption->wszText = new wchar_t [len * 2];
				localize()->ConvertANSIToUnicode(pOption->szName, pOption->wszText, len * 2);	// Takes length in bytes
			}
		}
	}
}

void CHudContextMenu::Init() 
{
}

void CHudContextMenu::DoCommand(const char *cmd)
{
	if (m_nLayer == 0)	// Is this check really needed anyway?
	{
		// They are disguising as last, so do it if possible
		if (Q_strncmp(cmd, "disguise last", 13) == 0 && g_szLastDisguiseComment[0] != 0)
		{
			engine->ClientCmd(g_szLastDisguiseComment);
			return;
		}

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

	// Add on the command to the command buffer then execute
	else
	{
		Q_strcat(szCmdBuffer, cmd, MAX_CMD_LEN);
		engine->ClientCmd(szCmdBuffer);

		// If disguise command, save this for later
		if (Q_strncmp(szCmdBuffer, "disguise", 8) == 0)
			Q_strncpy(g_szLastDisguiseComment, szCmdBuffer, sizeof(g_szLastDisguiseComment));

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
			// Make sure this is a valid button (i.e. not disabled)
			if (m_pMenu->options[m_iSelected].conditionfunc && m_pMenu->options[m_iSelected].conditionfunc() == MENU_SHOW)
			{
				pPlayer->EmitSound("ContextMenu.Select");
				DoCommand(m_pMenu->options[m_iSelected].szCommand);
			}
		}
		// If this menu has a default command and the user exited within the default action time then run the command
		else if (m_flMenuStart + cm_defaultactiontime.GetFloat() > gpGlobals->curtime && m_pMenu->default_cmd)
			engine->ClientCmd(m_pMenu->default_cmd);

		else
			pPlayer->EmitSound("ContextMenu.Close");

		m_fVisible = state;
		m_pMenu = NULL;
		return;
	}

	if (state && state != m_fVisible)
		pPlayer->EmitSound("ContextMenu.Open");

	m_fVisible = state;
	m_nLayer = 0;
	szCmdBuffer[0] = 0;
	m_flMenuStart = gpGlobals->curtime;

	if (m_pMenu == NULL)
	{
		switch (pPlayer->GetClassSlot())
		{
		case CLASS_ENGINEER: m_pMenu = &EngineerMenu; break;
		case CLASS_SPY: m_pMenu = &SpyMenu; break;
		case CLASS_DEMOMAN: m_pMenu = &DemomanMenu; break;
		default: m_pMenu = &CallMenu;
		}
	}

	SetMenu();
}

void CHudContextMenu::SetMenu()
{
	m_flSelectStart = gpGlobals->curtime;

	int x, y, iWidth, iTall;
	GetParent()->GetBounds(x, y, iWidth, iTall);

	float midx = iWidth * 0.5f;
	float midy = iTall * 0.5f;
	float dist = scheme()->GetProportionalScaledValue(cm_size.GetInt());

	m_flPosX = midx;
	m_flPosY = midy;

	m_nOptions = m_pMenu->size;

	int iFirstOpt = 0;
	int nAvailableOptions = 0;

	// Get positions for buttons
	for (int i = 0; i < m_nOptions; i++)
	{
		float increments = (float) i * DEG2RAD(360) / (float) m_nOptions;

		m_flPositions[i][0] = midx + dist * sin(increments);
		m_flPositions[i][1] = midy - dist * cos(increments) * cm_squash.GetFloat();

		// Count number of available options
		if (m_pMenu->options[i].conditionfunc() == MENU_SHOW)
		{
			iFirstOpt = i;
			nAvailableOptions++;
		}
	}

	// Only one available option, go straight into it if it is a branch
	if (nAvailableOptions == 1 && m_pMenu->options[iFirstOpt].pNextMenu)
	{
		ProgressToNextMenu(iFirstOpt);
		return;
	}

	// Put this over the entire screen
	SetWide(iWidth);
	SetTall(iTall);
	SetPos(0, 0);
}

void CHudContextMenu::Paint() 
{
	if (!m_fVisible) 
		return;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	// Colours we need later on
	Color highlighted(255, 0, 0, 255);
	Color dimmed(100, 100, 100, 255);

	// Get screen bounds sorted out
	int x, y, iWidth, iTall;
	GetParent()->GetBounds(x, y, iWidth, iTall);

	float dx = m_flPosX - (iWidth * 0.5f);
	float dy = m_flPosY - (iTall * 0.5f);
	float py = scheme()->GetProportionalScaledValue(2.0f);

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
		if (m_pMenu->options[i].conditionfunc() != MENU_SHOW)
			surface()->DrawSetTextColor(dimmed);
		else if (newSelection == i)
			surface()->DrawSetTextColor(highlighted);
		else
			surface()->DrawSetTextColor(GetFgColor());

		//
		// DRAW ICON
		//
		char character = m_pMenu->options[i].chIcon;

		//surface()->DrawSetTextColor(Color(100, 100, 100, 100));
		surface()->DrawSetTextFont(m_hMenuIcon);

		int iconOffsetX = surface()->GetCharacterWidth(m_hMenuIcon, character) / 2;
		int iconOffsetY = surface()->GetFontTall(m_hMenuIcon) / 2;

		wchar_t unicode[2];
		swprintf(unicode, L"%c", character);

		surface()->DrawSetTextPos(m_flPositions[i][0] - iconOffsetX, m_flPositions[i][1] - iconOffsetY);
		surface()->DrawUnicodeChar(unicode[0]);

		//
		// DRAW TEXT
		//

		surface()->DrawSetTextFont(m_hTextFont);

		// Work out centering and position & draw text
		int textOffsetX = 0.5f * UTIL_ComputeStringWidth(m_hTextFont, m_pMenu->options[i].wszText);
		surface()->DrawSetTextPos(m_flPositions[i][0] - textOffsetX, m_flPositions[i][1] + iconOffsetY + py);
		
		for (const wchar_t *wch = m_pMenu->options[i].wszText; *wch != 0; wch++)
			surface()->DrawUnicodeChar(*wch);

		//
		// DRAW SHORTCUT NUMBER
		//

		char chDisplay = (i == 9 ? '0' : '1' + i);

		int numberOffsetX = surface()->GetCharacterWidth(m_hTextFont, chDisplay) / 2;
		int textHeightX = surface()->GetFontTall(m_hTextFont);
		surface()->DrawSetTextPos(m_flPositions[i][0] - numberOffsetX, m_flPositions[i][1] + iconOffsetY + py + textHeightX);

		swprintf(unicode, L"%c", chDisplay);
		surface()->DrawUnicodeChar(unicode[0]);
	}

	// Restart timer & play sound if a new selection
	if (newSelection != m_iSelected)
	{
		m_flSelectStart = gpGlobals->curtime;
		m_iSelected = newSelection;

		pPlayer->EmitSound("ContextMenu.MouseOver");
	}


	// Progress to next menu if needed OR select the select thing
	// TODO: A cleaner way of doing this
	if (m_iSelected > -1 && gpGlobals->curtime > m_flSelectStart + cm_progresstime.GetFloat())
	{
		// There is a next menu for this option
		// ADDED: added check for conditionfunc to be true because
		// dimmed out "branches" could still be selected.
		if (m_pMenu->options[m_iSelected].pNextMenu && ( m_pMenu->options[m_iSelected].conditionfunc() == MENU_SHOW ) )
		{
			ProgressToNextMenu(m_iSelected);
			pPlayer->EmitSound("ContextMenu.NextMenu");
		}
		// Timed out on a valid option, so escape menu (selecting whatever option it was)
		else if (cm_waitforrelease.GetBool() == false && m_pMenu->options[m_iSelected].conditionfunc() == MENU_SHOW)
		{
			Display(false);
			return;
		}
	}

	// Show actual mouse location, just manually drawing a crosshair for now
	if (cm_usemouse.GetBool() && !cm_hidecursor.GetBool())
	{
		float ch_short = scheme()->GetProportionalScaledValue(1.0f);
		float ch_long = ch_short * 6.0f;

		surface()->DrawSetTexture(NULL);
		surface()->DrawSetColor(GetFgColor());
		surface()->DrawTexturedRect(m_flPosX - ch_short, m_flPosY - ch_long, m_flPosX + ch_short, m_flPosY + ch_long);
		surface()->DrawTexturedRect(m_flPosX - ch_long, m_flPosY - ch_short, m_flPosX + ch_long, m_flPosY + ch_short);
	}
}

void CHudContextMenu::ProgressToNextMenu(int iOption)
{
	Q_strcat(szCmdBuffer, m_pMenu->options[iOption].szCommand, MAX_CMD_LEN);

	m_pMenu = (menu_t *) m_pMenu->options[iOption].pNextMenu;
	m_nOptions = m_pMenu->size;

	m_iSelected = -1;
	m_flSelectStart = gpGlobals->curtime;
	m_nLayer++;

	SetMenu();
}

void CHudContextMenu::MouseMove(float *x, float *y) 
{
	if (!m_fVisible || !cm_usemouse.GetBool())
		return;

	float sensitivity_factor = 1.0f / (sensitivity.GetFloat() == 0 ? 0.001f : sensitivity.GetFloat());

	int bx, by, iWidth, iTall;
	GetParent()->GetBounds(bx, by, iWidth, iTall);

	float midx = iWidth * 0.5f;
	float midy = iTall * 0.5f;
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

int CHudContextMenu::KeyEvent(int down, int keynum, const char *pszCurrentBinding)
{
	if (!m_fVisible || !down) 
		return 1;

	if (keynum < '0' || keynum > '9')
		return 1;

	int iMenuOption = (keynum == '0' ? 9 : keynum - '1');

	if (iMenuOption >= m_nOptions)
		return 1;

	// They've selected this, so we place the mouse cursor over the box
	// and pretend its been there a while (so menus will automatically swap)
	// TODO: Maybe we should use mousemove for this so that we stay within bounds
	m_flPosX = m_flPositions[iMenuOption][0];
	m_flPosY = m_flPositions[iMenuOption][1];

	// Give it a couple of fractions of a second to draw before swapping
	m_iSelected = iMenuOption;
	m_flSelectStart = gpGlobals->curtime - cm_progresstime.GetFloat() - 0.1f;

	// Squelch this key
	return 0;
}

int HudContextMenuInput(int down, int keynum, const char *pszCurrentBinding)
{
	if (g_pHudContextMenu)
		return g_pHudContextMenu->KeyEvent(down, keynum, pszCurrentBinding);
	return 1;
}

void HudContextMenuInput(float *x, float *y) 
{
	if (g_pHudContextMenu) 
		g_pHudContextMenu->MouseMove(x, y);
}

void HudContextShow(bool visible) 
{
	if (!g_pHudContextMenu) 
		return;

	if (visible)
		g_pHudContextMenu->m_pMenu = NULL;

	g_pHudContextMenu->Display(visible);
}

void HudContextShowCalls(bool visible)
{
	if (!g_pHudContextMenu) 
		return;

	if (visible)
		g_pHudContextMenu->m_pMenu = &CallMenu;

	g_pHudContextMenu->Display(visible);
}

void HudContextForceClose()
{
	if (!g_pHudContextMenu)
		return;

	g_pHudContextMenu->m_pMenu = NULL;
	g_pHudContextMenu->Display(false);

}
