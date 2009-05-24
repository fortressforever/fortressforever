//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Draws CSPort's death notices
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "c_playerresource.h"
#include "clientmode_ff.h"
#include <vgui_controls/controls.h>
#include <vgui_controls/panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <KeyValues.h>
#include "c_baseplayer.h"
#include "c_team.h"
#include "ff_gamerules.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar hud_deathnotice_time( "hud_deathnotice_time", "6", FCVAR_ARCHIVE );

extern ConVar cl_killbeepwav;

// Player entries in a death notice
struct DeathNoticePlayer
{
	char		szName[MAX_PLAYER_NAME_LENGTH];
	int			iEntIndex;
};

// Contents of each entry in our list of death notices
struct DeathNoticeItem 
{
	DeathNoticePlayer	Killer;
	DeathNoticePlayer   Victim;
	CHudTexture *iconDeath;
	int			iSuicide;
	float		flDisplayTime;
	CHudTexture *iconBuildable; // draws after victim name, if it exists
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CHudDeathNotice : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudDeathNotice, vgui::Panel );
public:
	CHudDeathNotice( const char *pElementName );
	void Init( void );
	void VidInit( void );
	virtual bool ShouldDraw( void );
	virtual void Paint( void );
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );

	void SetColorForNoticePlayer( int iTeamNumber );
	void RetireExpiredDeathNotices( void );
	
	virtual void FireGameEvent( IGameEvent * event );

private:

	CPanelAnimationVarAliasType( float, m_flLineHeight, "LineHeight", "15", "proportional_float" );

	CPanelAnimationVar( float, m_flMaxDeathNotices, "MaxDeathNotices", "4" );

	CPanelAnimationVar( bool, m_bRightJustify, "RightJustify", "1" );

	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "HudNumbersTimer" );

	// Texture for skull symbol
	CHudTexture		*m_iconD_skull;

	CUtlVector<DeathNoticeItem> m_DeathNotices;
};

using namespace vgui;

DECLARE_HUDELEMENT( CHudDeathNotice );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudDeathNotice::CHudDeathNotice( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "HudDeathNotice" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_iconD_skull = NULL;

	SetHiddenBits( HIDEHUD_MISCSTATUS );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );
	SetPaintBackgroundEnabled( false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::Init( void )
{
	gameeventmanager->AddListener(this, "player_death", false );
	gameeventmanager->AddListener( this, "dispenser_killed", false );
	gameeventmanager->AddListener( this, "sentrygun_killed", false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::VidInit( void )
{
	m_iconD_skull = gHUD.GetIcon( "d_skull" );
	m_DeathNotices.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: Draw if we've got at least one death notice in the queue
//-----------------------------------------------------------------------------
bool CHudDeathNotice::ShouldDraw( void )
{
	return ( CHudElement::ShouldDraw() && ( m_DeathNotices.Count() ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::SetColorForNoticePlayer( int iTeamNumber )
{
	surface()->DrawSetTextColor( GameResources()->GetTeamColor( iTeamNumber ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::Paint()
{
	if ( !m_iconD_skull )
		return;

	int yStart = GetClientModeFFNormal()->GetDeathMessageStartHeight();

	surface()->DrawSetTextFont( m_hTextFont );
	surface()->DrawSetTextColor( GameResources()->GetTeamColor( 0 ) );


	int iCount = m_DeathNotices.Count();
	for ( int i = 0; i < iCount; i++ )
	{
		CHudTexture *icon = m_DeathNotices[i].iconDeath;
		if ( !icon )
			continue;

		wchar_t victim[ 256 ];
		wchar_t killer[ 256 ];

		// Get the team numbers for the players involved
		int iKillerTeam = 0;
		int iVictimTeam = 0;

		if( g_PR )
		{
			iKillerTeam = g_PR->GetTeam( m_DeathNotices[i].Killer.iEntIndex );
			iVictimTeam = g_PR->GetTeam( m_DeathNotices[i].Victim.iEntIndex );
		}

		vgui::localize()->ConvertANSIToUnicode( m_DeathNotices[i].Victim.szName, victim, sizeof( victim ) );
		vgui::localize()->ConvertANSIToUnicode( m_DeathNotices[i].Killer.szName, killer, sizeof( killer ) );

		//bool bSelfKill = ( (  == GR_TEAMMATE ) && ( m_DeathNotices[i].Killer.iEntIndex != m_DeathNotices[i].Victim.iEntIndex ) );

		// Get the local position for this notice
		int len = UTIL_ComputeStringWidth( m_hTextFont, victim );
		int y = yStart + (m_flLineHeight * i);

		int iconWide;
		int iconTall;

		if( icon->bRenderUsingFont )
		{
			iconWide = surface()->GetCharacterWidth( icon->hFont, icon->cCharacterInFont );
			iconTall = surface()->GetFontTall( icon->hFont );
		}
		else
		{
			float scale = ( (float)ScreenHeight() / 480.0f );	//scale based on 640x480
			iconWide = (int)( scale * (float)icon->Width() );
			iconTall = (int)( scale * (float)icon->Height() );
		}


		int iconBuildableWide = 0;
		int iconBuildableTall = 0;

		// Add room for killed buildable icon
		CHudTexture *iconBuildable = m_DeathNotices[i].iconBuildable;
		if ( iconBuildable )
		{
			if( iconBuildable->bRenderUsingFont )
			{
				iconBuildableWide = surface()->GetCharacterWidth( iconBuildable->hFont, iconBuildable->cCharacterInFont );
				iconBuildableTall = surface()->GetFontTall( iconBuildable->hFont );
			}
			else
			{
				float scale = ( (float)ScreenHeight() / 480.0f );	//scale based on 640x480
				iconBuildableWide = (int)( scale * (float)iconBuildable->Width() );
				iconBuildableTall = (int)( scale * (float)iconBuildable->Height() );
			}
		}

		int x;
		if ( m_bRightJustify )
		{
			x =	GetWide() - len - iconWide - 5;

			// keep moving over for buildable icon
			x -= iconBuildableWide ? iconBuildableWide + 5 : 0;
		}
		else
		{
			x = 0;
		}
		
		// --> Mirv: Shove over a bit
		y += 16;
		x -= 28;
		// <--


		// Only draw killers name if it wasn't a suicide
		if ( !m_DeathNotices[i].iSuicide )
		{
			if ( m_bRightJustify )
			{
				x -= UTIL_ComputeStringWidth( m_hTextFont, killer );
			}

			SetColorForNoticePlayer( iKillerTeam );

			// Draw killer's name
			surface()->DrawSetTextPos( x, y );
			surface()->DrawSetTextFont( m_hTextFont );
			surface()->DrawUnicodeString( killer );
			surface()->DrawGetTextPos( x, y );

			x += 5;	// |-- Mirv: 5px gap
		}
		Color iconTeamKillColor(0, 185, 0 , 250);
		Color iconColor( 255, 80, 0, 255 );
		

		// Don't include self kills when determining if teamkill
		//bool bTeamKill = (iKillerTeam == iVictimTeam && m_DeathNotices[i].Killer.iEntIndex != m_DeathNotices[i].Victim.iEntIndex);
		bool bTeamKill = ( ( FFGameRules()->IsTeam1AlliedToTeam2( iKillerTeam, iVictimTeam ) == GR_TEAMMATE ) && ( !m_DeathNotices[i].iSuicide ) );
		
		// Draw death weapon
		//If we're using a font char, this will ignore iconTall and iconWide
		icon->DrawSelf( x, y - (iconTall / 4), iconWide, iconTall, bTeamKill ? iconTeamKillColor : iconColor );
		x += iconWide + 5;		// |-- Mirv: 5px gap

		SetColorForNoticePlayer( iVictimTeam );

		// Draw victims name
		surface()->DrawSetTextPos( x, y );
		surface()->DrawSetTextFont( m_hTextFont );	//reset the font, draw icon can change it
		surface()->DrawUnicodeString( victim );
		surface()->DrawGetTextPos( x, y );

		// draw a team colored buildable icon
		if (iconBuildable)
		{
			x += 5;
			iconBuildable->DrawSelf( x, y - (iconBuildableTall / 4), iconBuildableWide, iconBuildableTall, Color(GameResources()->GetTeamColor(iVictimTeam)) );
		}
	}

	// Now retire any death notices that have expired
	RetireExpiredDeathNotices();
}

//-----------------------------------------------------------------------------
// Purpose: This message handler may be better off elsewhere
//-----------------------------------------------------------------------------
void CHudDeathNotice::RetireExpiredDeathNotices( void )
{
	// Loop backwards because we might remove one
	int iSize = m_DeathNotices.Size();
	for ( int i = iSize-1; i >= 0; i-- )
	{
		if ( m_DeathNotices[i].flDisplayTime < gpGlobals->curtime )
		{
			m_DeathNotices.Remove(i);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Server's told us that someone's died
//-----------------------------------------------------------------------------
void CHudDeathNotice::FireGameEvent( IGameEvent * event )
{
	if (!g_PR)
		return;

	if ( hud_deathnotice_time.GetFloat() == 0 )
		return;

	// the event should be "player_death"
	int killer = engine->GetPlayerForUserID( event->GetInt("attacker") );
	int victim = engine->GetPlayerForUserID( event->GetInt("userid") );
	const char *killedwith = event->GetString( "weapon" );

	// Stuffs for handling buildable deaths
	bool bBuildableKilled = false;

	// headshot deaths
	bool bHeadshot = false;

	// trigger hurt death
	bool bTriggerHurt = false;
	char tempTriggerHurtString[128];
	char fullkilledwith[128];

	if ( killedwith && *killedwith )
	{
		// #0001568: Falling into pitt damage shows electrocution icon, instead of falling damage icon.  Until now there was no way
		// to identify different types of trigger_hurt deaths, so I added a "damagetype" field to the player_death event.
		// if a trigger_hurt is the killer, check the damage type and append it to the killer's name in a temp string (post-fix append)
		// e.g. if it's DMG_FALL, it's falling damage, so the complete string becomes death_trigger_hurt_fall.  
		// Match up the corresponding icons in scripts/ff_hud_textures.txt		-> Defrag

		if( Q_stricmp( "trigger_hurt", killedwith ) == 0 )
		{
			bTriggerHurt = true;

			switch( event->GetInt( "damagetype" ))
			{
			case DMG_FALL:
				Q_snprintf( tempTriggerHurtString, sizeof(tempTriggerHurtString), "%s_fall", killedwith );
				break;

			case DMG_DROWN:
				Q_snprintf( tempTriggerHurtString, sizeof(tempTriggerHurtString), "%s_drown", killedwith );
				break;

			case DMG_SHOCK:
				Q_snprintf( tempTriggerHurtString, sizeof(tempTriggerHurtString), "%s_shock", killedwith );
				break;

			default:
				Q_strncpy( tempTriggerHurtString, killedwith, sizeof(tempTriggerHurtString));
				break;
			}
		}	

		if( bTriggerHurt )	// copy different string if it's a trigger_hurt death (see above)
		{
			Q_snprintf( fullkilledwith, sizeof(fullkilledwith), "death_%s", tempTriggerHurtString );
		}
		else
		{
			Q_snprintf( fullkilledwith, sizeof(fullkilledwith), "death_%s", killedwith );
		}		
	}
	else
	{
		fullkilledwith[0] = 0;
	}

	// Do we have too many death messages in the queue?
	if ( m_DeathNotices.Count() > 0 &&
		m_DeathNotices.Count() >= (int)m_flMaxDeathNotices )
	{
		// Remove the oldest one in the queue, which will always be the first
		m_DeathNotices.Remove(0);
	}

	// Get the names of the players
	const char *killer_name = g_PR->GetPlayerName( killer );
	const char *victim_name = g_PR->GetPlayerName( victim );

	//bool bTeamKill = (g_PR->GetTeam(killer) == g_PR->GetTeam(victim));
	bool bTeamKill = ( ( FFGameRules()->IsTeam1AlliedToTeam2( g_PR->GetTeam( killer ), g_PR->GetTeam( victim ) ) == GR_TEAMMATE ) && ( killer != victim ) );

	if ( !killer_name )
		killer_name = "";
	if ( !victim_name )
		victim_name = "";

	// going to make these use icons instead of text
/*
	// Buildable stuff
	char pszVictimMod[ MAX_PLAYER_NAME_LENGTH + 24 ];
	if( !Q_strcmp( event->GetName(), "dispenser_killed" ) ) 
	{
		bBuildableKilled = true;
		Q_snprintf( pszVictimMod, sizeof( pszVictimMod ), "%s's Dispenser", victim_name );
		victim_name = const_cast< char * >( pszVictimMod );
	}
	else if( !Q_strcmp( event->GetName(), "sentrygun_killed" ) )
	{
		bBuildableKilled = true;
		Q_snprintf( pszVictimMod, sizeof( pszVictimMod ), "%s's Sentrygun", victim_name );
		victim_name = const_cast< char * >( pszVictimMod );
	}
*/

	// Make a new death notice
	DeathNoticeItem deathMsg;
	deathMsg.Killer.iEntIndex = killer;
	deathMsg.Victim.iEntIndex = victim;
	Q_strncpy( deathMsg.Killer.szName, killer_name, MAX_PLAYER_NAME_LENGTH );
	Q_strncpy( deathMsg.Victim.szName, victim_name, MAX_PLAYER_NAME_LENGTH );
	deathMsg.flDisplayTime = gpGlobals->curtime + hud_deathnotice_time.GetFloat();

	// buildable kills
	if( !Q_strcmp( event->GetName(), "sentrygun_killed" ) )
	{
		deathMsg.iconBuildable = gHUD.GetIcon("death_weapon_deploysentrygun");
		bBuildableKilled = true;
	}
	else if( !Q_strcmp( event->GetName(), "dispenser_killed" ) ) 
	{
		deathMsg.iconBuildable = gHUD.GetIcon("death_weapon_deploydispenser");
		bBuildableKilled = true;
	}
	else
		deathMsg.iconBuildable = NULL;
	
	deathMsg.iSuicide = ( !killer || ( ( killer == victim ) && ( !bBuildableKilled ) ) );

	// 0000336: If we have a Detpack...
	// NOTE: may need these changes for the SG and Dispenser in order for the death status icons to work right
	if (Q_stricmp(killedwith, "Detpack") == 0)
	{
		deathMsg.iconDeath = gHUD.GetIcon("death_weapon_deploydetpack");
	}
	// 0001292: If we have a Dispenser
	else if (Q_stricmp(killedwith, "Dispenser") == 0)
	{
		deathMsg.iconDeath = gHUD.GetIcon("death_weapon_deploydispenser");
	}
	// 0001292: If we have a Sentrygun
	else if (Q_stricmp(killedwith, "Sentrygun") == 0)
	{
		deathMsg.iconDeath = gHUD.GetIcon("death_weapon_deploysentrygun");
	}
	// only 1 weapon can kill with a headshot
	else if (Q_stricmp(killedwith, "BOOM_HEADSHOT") == 0)
	{
		// need the _weapon_sniperrifle in case people create "death_notice" entries in other weapon scripts...we just want the sniper rifle's
		deathMsg.iconDeath = gHUD.GetIcon("death_BOOM_HEADSHOT_weapon_sniperrifle");
		bHeadshot = true;
	}
	else if (Q_stricmp(killedwith, "caltrop") == 0)
	{
		deathMsg.iconDeath = gHUD.GetIcon("death_grenade_caltrop");
	}
	else if (Q_stricmp(killedwith, "grenade_napalmlet") == 0)
	{
		deathMsg.iconDeath = gHUD.GetIcon("death_grenade_napalm");
	}
	else
	{
		// Try and find the death identifier in the icon list
		DevMsg("Death BY: %s.\n",fullkilledwith );
		deathMsg.iconDeath = gHUD.GetIcon( fullkilledwith );
	}

	// Show weapon if it was a suicide too
	if ( !deathMsg.iconDeath /*|| deathMsg.iSuicide*/ )
	{
		// Can't find it, so use the default skull & crossbones icon
		DevMsg("Default death icon!" );
		deathMsg.iconDeath = gHUD.GetIcon("death_world");
	}

	// Add it to our list of death notices
	m_DeathNotices.AddToTail( deathMsg );

	char sDeathMsg[512];

	// Record the death notice in the console
	if ( deathMsg.iSuicide )
	{
		if ( !strcmp( fullkilledwith, "d_worldspawn" ) )
		{
			Q_snprintf( sDeathMsg, sizeof( sDeathMsg ), "%s died.\n", deathMsg.Victim.szName );
		}
		else	//d_world
		{
			Q_snprintf( sDeathMsg, sizeof( sDeathMsg ), "%s suicided.\n", deathMsg.Victim.szName );
		}
	}
	else
	{
		Q_snprintf( sDeathMsg, sizeof( sDeathMsg ), "%s %skilled %s", deathMsg.Killer.szName, bTeamKill ? "team" : "", deathMsg.Victim.szName );

		if ( fullkilledwith && *fullkilledwith && (*fullkilledwith > 13 ) )
		{
			Q_strncat( sDeathMsg, VarArgs( " with %s.\n", fullkilledwith+6 ), sizeof( sDeathMsg ), COPY_ALL_CHARACTERS );
		}
	}

	// play the killbeep
	CBasePlayer *pLocalPlayer = CBasePlayer::GetLocalPlayer();
	if ( pLocalPlayer && killer == GetLocalPlayerIndex() )
	{
		char buf[MAX_PATH];
		Q_snprintf( buf, MAX_PATH - 1, "player/deathbeep/%s.wav", cl_killbeepwav.GetString() );

		CPASAttenuationFilter filter(pLocalPlayer, buf);

		EmitSound_t params;
		params.m_pSoundName = buf;
		params.m_flSoundTime = 0.0f;
		params.m_pflSoundDuration = NULL;
		params.m_bWarnOnDirectWaveReference = false;
		params.m_nChannel = CHAN_STATIC;

		if ( bTeamKill )
		{
			params.m_nPitch = 88;
			params.m_flVolume = VOL_NORM;
		}
		else if ( deathMsg.iSuicide )
		{
			params.m_nPitch = 94;
			params.m_flVolume = 0.5f;
		}
		else if ( bBuildableKilled )
		{
			params.m_nPitch = 106;
			params.m_flVolume = VOL_NORM;
		}
		else if ( bHeadshot )
		{
			params.m_nPitch = 112;
			params.m_flVolume = 0.5f;
		}
		else
		{
			params.m_nPitch = PITCH_NORM;
			params.m_flVolume = VOL_NORM;
		}

		pLocalPlayer->EmitSound(filter, pLocalPlayer->entindex(), params);
	}

	Msg( "%s", sDeathMsg );
}



