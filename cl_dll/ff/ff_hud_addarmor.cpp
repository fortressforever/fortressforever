

//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_scorelatest.cpp
//	@author Michael Parker (AfterShock)
//	@date 30/06/2007
//	@brief Hud Player Armor field - with details of your latest score
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
static ConVar hud_addarmor("hud_addarmor", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Toggle visibility of added armor notices.");

//-----------------------------------------------------------------------------
// Purpose: Displays current disguised class
//-----------------------------------------------------------------------------
class CHudPlayerAddArmor : public CHudElement, public vgui::FFPanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudPlayerAddArmor, vgui::FFPanel );

	CHudPlayerAddArmor( const char *pElementName ) : vgui::FFPanel( NULL, "HudPlayerAddArmor" ), CHudElement( pElementName )
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport() );

		// Hide when player is dead
		SetHiddenBits( HIDEHUD_PLAYERDEAD );
	}

	virtual ~CHudPlayerAddArmor( void )
	{

	}

	virtual void Paint( void );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual bool ShouldDraw( void );
	void MsgFunc_PlayerAddArmor( bf_read &msg );

protected:
	wchar_t		m_pTextArmor[ 1024 ];	// Unicode text buffer
	int			m_iArmor;

private:

	float		m_flStartTime;		// When the message was recevied
	float		m_flDuration;		// Duration of the message

	// Stuff we need to know	
	CPanelAnimationVar( vgui::HFont, m_hArmorFont, "ArmorFont", "Default" );

	CPanelAnimationVar( vgui::HFont, m_hArmorFontBG, "ArmorFontBG", "Default" );

	CPanelAnimationVarAliasType( float, ArmorFont_xpos, "ArmorFont_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, ArmorFont_ypos, "ArmorFont_ypos", "0", "proportional_float" );
};

DECLARE_HUDELEMENT( CHudPlayerAddArmor );
DECLARE_HUD_MESSAGE( CHudPlayerAddArmor, PlayerAddArmor );

//-----------------------------------------------------------------------------
// Purpose: Done on loading game?
//-----------------------------------------------------------------------------
void CHudPlayerAddArmor::Init( void )
{
	HOOK_HUD_MESSAGE( CHudPlayerAddArmor, PlayerAddArmor );

	m_pTextArmor[ 0 ] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudPlayerAddArmor::VidInit( void )
{
	SetPaintBackgroundEnabled( false );
	
	m_pTextArmor[ 0 ] = '\0'; 
}

//-----------------------------------------------------------------------------
// Purpose: Should we draw? (Are we ingame? have we picked a class, etc)
//-----------------------------------------------------------------------------
bool CHudPlayerAddArmor::ShouldDraw() 
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

void CHudPlayerAddArmor::MsgFunc_PlayerAddArmor( bf_read &msg )
{
	// Read int and convert to string
	const int ptVal = msg.ReadShort();
	if(ptVal==0)
		return;

	char szString2[ 1024 ];
	Q_snprintf( szString2, sizeof(szString2), "%s%i", ptVal>0?"+":"",ptVal );

	// convert int-string to unicode
	vgui::localize()->ConvertANSIToUnicode( szString2, m_pTextArmor, sizeof( m_pTextArmor ) );

	// play animation (new points value)
	if(ptVal > 0)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "NewAddArmor" );
	}
	else
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "NewSubtractArmor" );
	}

	m_flStartTime = gpGlobals->curtime;
	m_flDuration = 3.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudPlayerAddArmor::Paint() 
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer(); 
	if ( !pPlayer ) 
		return; 

	if(!hud_addarmor.GetBool())
		return;

	if ( m_flStartTime + m_flDuration < gpGlobals->curtime )
		return;

	FFPanel::Paint(); // Draws the background glyphs 

	if( m_pTextArmor[ 0 ] != '\0' )
	{
		surface()->DrawSetTextFont( m_hArmorFont );
		surface()->DrawSetTextColor( GetFgColor() );
		surface()->DrawSetTextPos( ArmorFont_xpos, ArmorFont_ypos );

		for( wchar_t *wch = m_pTextArmor; *wch != 0; wch++ )
			surface()->DrawUnicodeChar( *wch );

	}
}
