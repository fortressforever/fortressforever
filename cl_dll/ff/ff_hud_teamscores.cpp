//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_teamscores.cpp
//	@author Michael Parker (AfterShock)
//	@date 27/05/2007
//	@brief Hud Player Score field - with details of your latest score
//
//	REVISIONS
//	---------
//	15/06/2007, AfterShock: 
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

static ConVar hud_teamscores("hud_teamscores", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Toggle visible team scores on the HUD.");

//-----------------------------------------------------------------------------
// Purpose: Displays current disguised class
//-----------------------------------------------------------------------------
class CHudTeamScores : public CHudElement, public vgui::FFPanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudTeamScores, vgui::FFPanel );

	CHudTeamScores( const char *pElementName ) : vgui::FFPanel( NULL, "HudTeamScores" ), CHudElement( pElementName )
	{
		SetParent( g_pClientMode->GetViewport() );
		SetHiddenBits( HIDEHUD_UNASSIGNED );
	}

	virtual ~CHudTeamScores( void )
	{

	}

	virtual void Paint( void );
	virtual void Init( void );
	virtual void VidInit( void );
	
protected:

	void CHudTeamScores::PaintNumbers(HFont font, int xpos, int ypos, int value);
	void CHudTeamScores::PaintNumbersRightAligned(HFont font, int xpos, int ypos, int value, int maxchars);

private:
	// Stuff we need to know	
	CPanelAnimationVar( vgui::HFont, m_hTeamScoreBlueFont, "TeamScoreBlueFont", "Default" );
	CPanelAnimationVar( vgui::HFont, m_hTeamScoreRedFont, "TeamScoreRedFont", "Default" );
	CPanelAnimationVar( vgui::HFont, m_hTeamScoreGreenFont, "TeamScoreGreenFont", "Default" );
	CPanelAnimationVar( vgui::HFont, m_hTeamScoreYellowFont, "TeamScoreYellowFont", "Default" );

	CPanelAnimationVarAliasType( float, TeamScoreBlue_xpos, "TeamScoreBlue_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, TeamScoreBlue_ypos, "TeamScoreBlue_ypos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, TeamScoreRed_xpos, "TeamScoreRed_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, TeamScoreRed_ypos, "TeamScoreRed_ypos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, TeamScoreGreen_xpos, "TeamScoreGreen_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, TeamScoreGreen_ypos, "TeamScoreGreen_ypos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, TeamScoreYellow_xpos, "TeamScoreYellow_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, TeamScoreYellow_ypos, "TeamScoreYellow_ypos", "0", "proportional_float" );
};

DECLARE_HUDELEMENT( CHudTeamScores );

//-----------------------------------------------------------------------------
// Purpose: Done on loading game?
//-----------------------------------------------------------------------------
void CHudTeamScores::Init( void )
{

}

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudTeamScores::VidInit( void )
{
	SetPaintBackgroundEnabled( false );
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudTeamScores::Paint() 
{ 
	FFPanel::Paint(); // Draws the background glyphs 

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer(); 
	if ( !pPlayer ) 
		return; 

	IGameResources *pGR = GameResources();
	if( !pGR )
		return;
	
	if(!hud_teamscores.GetBool())
		return;

	Color cColor;

	int iTeam = TEAM_BLUE;
	C_FFTeam *pTeam = GetGlobalFFTeam( iTeam );
	if (pTeam->Get_Teams() > -1)
	{
		SetColorByTeam( iTeam, cColor );		
		surface()->DrawSetTextColor( cColor.r(), cColor.g(), cColor.b(), 255 );
		PaintNumbersRightAligned(m_hTeamScoreBlueFont, TeamScoreBlue_xpos, TeamScoreBlue_ypos, pGR->GetTeamScore( iTeam ), 5 );
	}

	iTeam = TEAM_RED;
	pTeam = GetGlobalFFTeam( iTeam );
	if (pTeam->Get_Teams() > -1)
	{
		SetColorByTeam( iTeam, cColor );		
		surface()->DrawSetTextColor( cColor.r(), cColor.g(), cColor.b(), 255 );
		PaintNumbers(m_hTeamScoreRedFont, TeamScoreRed_xpos, TeamScoreRed_ypos, pGR->GetTeamScore( iTeam ) );
	}

	iTeam = TEAM_YELLOW;
	pTeam = GetGlobalFFTeam( iTeam );
	if (pTeam->Get_Teams() > -1)
	{
		SetColorByTeam( iTeam, cColor );		
		surface()->DrawSetTextColor( cColor.r(), cColor.g(), cColor.b(), 255 );
		PaintNumbersRightAligned(m_hTeamScoreYellowFont, TeamScoreYellow_xpos, TeamScoreYellow_ypos, pGR->GetTeamScore( iTeam ), 5 );
	}

	iTeam = TEAM_GREEN;
	pTeam = GetGlobalFFTeam( iTeam );
	if (pTeam->Get_Teams() > -1)
	{
		SetColorByTeam( iTeam, cColor );		
		surface()->DrawSetTextColor( cColor.r(), cColor.g(), cColor.b(), 255 );
		PaintNumbers(m_hTeamScoreGreenFont, TeamScoreGreen_xpos, TeamScoreGreen_ypos, pGR->GetTeamScore( iTeam ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: paints a number right aligned, so the digits column is always in the same place
//-----------------------------------------------------------------------------
void CHudTeamScores::PaintNumbersRightAligned(HFont font, int xpos, int ypos, int value, int maxchars)
{	
	int charWidth = surface()->GetCharacterWidth(font, '0');
	int iTempxpos = xpos + charWidth * maxchars; // allow for X characters of score

	wchar_t unicode[6];
	swprintf(unicode, L"%d", value);

	surface()->DrawSetTextFont( font );
	
	// for each character, measure its width and subtract that from the xpos
	// so the bottom right corner is always in the same place - shok
	for( wchar_t *wch = unicode; *wch != 0; wch++ )
		iTempxpos -= surface()->GetCharacterWidth(font, *wch);

	surface()->DrawSetTextPos( iTempxpos, ypos );

	for( wchar_t *wch = unicode; *wch != 0; wch++ )
		surface()->DrawUnicodeChar( *wch );
}

//-----------------------------------------------------------------------------
// Purpose: paints a number at the specified position
//-----------------------------------------------------------------------------
void CHudTeamScores::PaintNumbers(HFont font, int xpos, int ypos, int value)
{
	surface()->DrawSetTextFont(font);
	wchar_t unicode[6];
	swprintf(unicode, L"%d", value);
	surface()->DrawSetTextPos(xpos, ypos);
	surface()->DrawUnicodeString( unicode );
}