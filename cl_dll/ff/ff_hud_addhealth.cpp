

//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_scorelatest.cpp
//	@author Michael Parker (AfterShock)
//	@date 30/06/2007
//	@brief Hud Player Health field - with details of your latest score
//
//	REVISIONS
//	---------
//	30/06/2007, AfterShock: 
//		First created (from ff_hud_spydisguise.cpp)

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"

//#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>

#include "ff_panel.h"
#include "c_ff_player.h"
#include "ff_utils.h"
#include "c_playerresource.h"

#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
static ConVar hud_addhealth("hud_addhealth", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Toggle visibility of added health notices.");

//-----------------------------------------------------------------------------
// Purpose: Displays current disguised class
//-----------------------------------------------------------------------------
class CHudPlayerAddHealth : public CHudElement, public vgui::FFPanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudPlayerAddHealth, vgui::FFPanel );

	CHudPlayerAddHealth( const char *pElementName ) : vgui::FFPanel( NULL, "HudPlayerAddHealth" ), CHudElement( pElementName )
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport() );

		// Hide when player is dead
		SetHiddenBits( HIDEHUD_PLAYERDEAD );
	}

	virtual ~CHudPlayerAddHealth( void )
	{

	}

	virtual void Paint( void );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual bool ShouldDraw( void );
	void MsgFunc_PlayerAddHealth( bf_read &msg );

protected:
	wchar_t		m_pTextHealth[ 1024 ];	// Unicode text buffer
	int			m_iHealth;

private:

	float		m_flStartTime;		// When the message was recevied
	float		m_flDuration;		// Duration of the message

	// Stuff we need to know	
	CPanelAnimationVar( vgui::HFont, m_hHealthFont, "HealthFont", "Default" );

	CPanelAnimationVar( vgui::HFont, m_hHealthFontBG, "HealthFontBG", "Default" );

	CPanelAnimationVarAliasType( float, HealthFont_xpos, "HealthFont_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, HealthFont_ypos, "HealthFont_ypos", "0", "proportional_float" );
};

DECLARE_HUDELEMENT( CHudPlayerAddHealth );
DECLARE_HUD_MESSAGE( CHudPlayerAddHealth, PlayerAddHealth );

//-----------------------------------------------------------------------------
// Purpose: Done on loading game?
//-----------------------------------------------------------------------------
void CHudPlayerAddHealth::Init( void )
{
	HOOK_HUD_MESSAGE( CHudPlayerAddHealth, PlayerAddHealth );

	m_pTextHealth[ 0 ] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudPlayerAddHealth::VidInit( void )
{
	SetPaintBackgroundEnabled( false );
	
	m_pTextHealth[ 0 ] = '\0'; 
}

//-----------------------------------------------------------------------------
// Purpose: Should we draw? (Are we ingame? have we picked a class, etc)
//-----------------------------------------------------------------------------
bool CHudPlayerAddHealth::ShouldDraw() 
{ 
	if( !engine->IsInGame() ) 
		return false; 

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer(); 

	if( !pPlayer ) 
		return false; 

	if( FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer ) ) 
		return false; 

	return true; 
} 

void CHudPlayerAddHealth::MsgFunc_PlayerAddHealth( bf_read &msg )
{
	// Read int and convert to string
	const int ptVal = msg.ReadShort();
	if(ptVal==0)
		return;

	char szString2[ 1024 ];
	Q_snprintf( szString2, sizeof(szString2), "%s%i", ptVal>0?"+":"",ptVal );

	// convert int-string to unicode
	vgui::localize()->ConvertANSIToUnicode( szString2, m_pTextHealth, sizeof( m_pTextHealth ) );

	// play animation (new points value)
	if(ptVal > 0)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "NewAddHealth" );
	}
	else
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "NewSubtractHealth" );
	}

	m_flStartTime = gpGlobals->curtime;
	m_flDuration = 3.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudPlayerAddHealth::Paint() 
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer(); 
	if ( !pPlayer ) 
		return; 

	if(!hud_addhealth.GetBool())
		return;

	if ( m_flStartTime + m_flDuration < gpGlobals->curtime )
		return;

	FFPanel::Paint(); // Draws the background glyphs 

	if( m_pTextHealth[ 0 ] != '\0' )
	{
		surface()->DrawSetTextFont( m_hHealthFont );
		surface()->DrawSetTextColor( GetFgColor() );
		surface()->DrawSetTextPos( HealthFont_xpos, HealthFont_ypos );

		for( wchar_t *wch = m_pTextHealth; *wch != 0; wch++ )
			surface()->DrawUnicodeChar( *wch );

	}
}
