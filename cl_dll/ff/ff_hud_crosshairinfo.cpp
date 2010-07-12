//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_crosshairinfo.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 02/03/2006
//	@brief client side Hud crosshair info
//
//	REVISIONS
//	---------
//	02/03/2006, Mulchman: 
//		First created

#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "view.h"

using namespace vgui;

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>

//#include "debugoverlay_shared.h"

//#include "c_ff_player.h"
#include "ff_buildableobjects_shared.h"
#include <igameresources.h>
#include "c_ff_team.h"
#include "ff_gamerules.h"
#include "ff_utils.h"
#include "ff_shareddefs.h"

#include "ff_hud_quantityBar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


static ConVar hud_centerid( "hud_centerid", "0", FCVAR_ARCHIVE );
static ConVar hud_xhairinfo_newStyle( "hud_xhairinfo_newStyle", "1", FCVAR_NONE, "Crosshair info new style (0 Off, 1 On)");
static ConVar hud_xhairinfo_update( "hud_xhairinfo_update", "1", FCVAR_NONE, "Used to update the look (does not happen on value change as there are so many values to set)");
static ConVar hud_xhairinfo_updateAlways( "hud_xhairinfo_updateAlways", "1", FCVAR_NONE, "For perfromance reasons, only have this on while editing your settings.");

static ConVar hud_xhairinfo_x( "hud_xhairinfo_x", "200", FCVAR_NONE, "Crosshair info X Position on 640 480 Resolution");
static ConVar hud_xhairinfo_y( "hud_xhairinfo_y", "300", FCVAR_NONE, "Crosshair info Y Position on 640 480 Resolution");

static ConVar hud_xhairinfo_intensity_red( "hud_xhairinfo_intensity_red", "20", FCVAR_NONE, "When using colormode stepped and faded this is the value at which the bar is red");
static ConVar hud_xhairinfo_intensity_orange( "hud_xhairinfo_intensity_orange", "50", FCVAR_NONE, "When using colormode stepped and faded this is the value at which the bar is orange");
static ConVar hud_xhairinfo_intensity_yellow( "hud_xhairinfo_intensity_yellow", "70", FCVAR_NONE, "When using colormode stepped and faded this is the value at which the bar is yellow");
static ConVar hud_xhairinfo_intensity_green( "hud_xhairinfo_intensity_green", "100", FCVAR_NONE, "When using colormode stepped and faded this is the value at which the bar is green");

static ConVar hud_xhairinfo_showBar( "hud_xhairinfo_showBar", "1", FCVAR_NONE, "Show Bar");
static ConVar hud_xhairinfo_showBarBackground( "hud_xhairinfo_showBarBackground", "1", FCVAR_NONE, "Show Bar Background");
static ConVar hud_xhairinfo_showBarBorder( "hud_xhairinfo_showBarBorder", "1", FCVAR_NONE, "Show Bar Boarder");
static ConVar hud_xhairinfo_showIcon( "hud_xhairinfo_showIcon", "1", FCVAR_NONE, "Show Icon");
static ConVar hud_xhairinfo_showLabel( "hud_xhairinfo_showLabel", "0", FCVAR_NONE, "Show label");
static ConVar hud_xhairinfo_showAmount( "hud_xhairinfo_showAmount", "0", FCVAR_NONE, "Show amount");
static ConVar hud_xhairinfo_showIdent( "hud_xhairinfo_showIdent", "1", FCVAR_NONE, "Show Identifier (Name,Icon)");

static ConVar hud_xhairinfo_itemsPerRow( "hud_xhairinfo_itemsPerRow", "2", FCVAR_NONE, "Crosshair  height gap on 640 480 Resolution");
static ConVar hud_xhairinfo_itemOffsetX( "hud_xhairinfo_itemOffsetX", "40", FCVAR_NONE, "Crosshair  height gap on 640 480 Resolution");
static ConVar hud_xhairinfo_itemOffsetY( "hud_xhairinfo_itemOffsetY", "20", FCVAR_NONE, "Crosshair  height gap on 640 480 Resolution");

static ConVar hud_xhairinfo_barWidth( "hud_xhairinfo_barWidth", "25", FCVAR_NONE, "Bar width on 640 480 Resolution");
static ConVar hud_xhairinfo_barHeight( "hud_xhairinfo_barHeight", "4", FCVAR_NONE, "Bar height on 640 480 Resolution");
static ConVar hud_xhairinfo_barBorderWidth( "hud_xhairinfo_barBorderWidth", "1", FCVAR_NONE, "Bar border width (non-scaleable)");

static ConVar hud_xhairinfo_ColorBar_r( "hud_xhairinfo_ColorBar_r", "255", FCVAR_NONE, "Bar color red component");
static ConVar hud_xhairinfo_ColorBar_g( "hud_xhairinfo_ColorBar_g", "255", FCVAR_NONE, "Bar color green component");
static ConVar hud_xhairinfo_ColorBar_b( "hud_xhairinfo_ColorBar_b", "255", FCVAR_NONE, "Bar color blue component");
static ConVar hud_xhairinfo_ColorBar_a( "hud_xhairinfo_ColorBar_a", "200", FCVAR_NONE, "Bar color alpha component");
static ConVar hud_xhairinfo_ColorBarBackground_r( "hud_xhairinfo_ColorBarBackground_r", "255", FCVAR_NONE, "Bar Background  background color red component");
static ConVar hud_xhairinfo_ColorBarBackground_g( "hud_xhairinfo_ColorBarBackground_g", "255", FCVAR_NONE, "Bar Background color green component");
static ConVar hud_xhairinfo_ColorBarBackground_b( "hud_xhairinfo_ColorBarBackground_b", "255", FCVAR_NONE, "Bar Background color blue component");
static ConVar hud_xhairinfo_ColorBarBackground_a( "hud_xhairinfo_ColorBarBackground_a", "96", FCVAR_NONE, "Bar Background color alpha component");
static ConVar hud_xhairinfo_ColorBarBorder_r( "hud_xhairinfo_ColorBarBorder_r", "255", FCVAR_NONE, "Bar Border color red component");
static ConVar hud_xhairinfo_ColorBarBorder_g( "hud_xhairinfo_ColorBarBorder_g", "255", FCVAR_NONE, "Bar Border color green component");
static ConVar hud_xhairinfo_ColorBarBorder_b( "hud_xhairinfo_ColorBarBorder_b", "255", FCVAR_NONE, "Bar Border color blue component");
static ConVar hud_xhairinfo_ColorBarBorder_a( "hud_xhairinfo_ColorBarBorder_a", "255", FCVAR_NONE, "Bar Border color alpha component");
static ConVar hud_xhairinfo_ColorIcon_r( "hud_xhairinfo_ColorIcon_r", "0", FCVAR_NONE, "Icon color red component");
static ConVar hud_xhairinfo_ColorIcon_g( "hud_xhairinfo_ColorIcon_g", "0", FCVAR_NONE, "Icon color green component");
static ConVar hud_xhairinfo_ColorIcon_b( "hud_xhairinfo_ColorIcon_b", "0", FCVAR_NONE, "Icon color blue component");
static ConVar hud_xhairinfo_ColorIcon_a( "hud_xhairinfo_ColorIcon_a", "255", FCVAR_NONE, "Icon color alpha component");
static ConVar hud_xhairinfo_ColorLabel_r( "hud_xhairinfo_ColorLabel_r", "0", FCVAR_NONE, "Label color red component");
static ConVar hud_xhairinfo_ColorLabel_g( "hud_xhairinfo_ColorLabel_g", "0", FCVAR_NONE, "Label color green component");
static ConVar hud_xhairinfo_ColorLabel_b( "hud_xhairinfo_ColorLabel_b", "0", FCVAR_NONE, "Label color blue component");
static ConVar hud_xhairinfo_ColorLabel_a( "hud_xhairinfo_ColorLabel_a", "255", FCVAR_NONE, "Label color alpha component");
static ConVar hud_xhairinfo_ColorAmount_r( "hud_xhairinfo_ColorAmount_r", "255", FCVAR_NONE, "Amount color red component");
static ConVar hud_xhairinfo_ColorAmount_g( "hud_xhairinfo_ColorAmount_g", "255", FCVAR_NONE, "Amount color green component");
static ConVar hud_xhairinfo_ColorAmount_b( "hud_xhairinfo_ColorAmount_b", "255", FCVAR_NONE, "Amount color blue component");
static ConVar hud_xhairinfo_ColorAmount_a( "hud_xhairinfo_ColorAmount_a", "255", FCVAR_NONE, "Amount color alpha component");
static ConVar hud_xhairinfo_ColorIdent_r( "hud_xhairinfo_ColorIdent_r", "255", FCVAR_NONE, "Identifier (Class,Icon) color red component");
static ConVar hud_xhairinfo_ColorIdent_g( "hud_xhairinfo_ColorIdent_g", "255", FCVAR_NONE, "Identifier (Class,Icon) color green component");
static ConVar hud_xhairinfo_ColorIdent_b( "hud_xhairinfo_ColorIdent_b", "255", FCVAR_NONE, "Identifier (Class,Icon) color blue component");
static ConVar hud_xhairinfo_ColorIdent_a( "hud_xhairinfo_ColorIdent_a", "255", FCVAR_NONE, "Identifier (Class,Icon) color alpha component");

static ConVar hud_xhairinfo_ShadowIcon( "hud_xhairinfo_ShadowIcon", "0", FCVAR_NONE, "Icon Shadow (0 Off, 1 On)");
static ConVar hud_xhairinfo_ShadowLabel( "hud_xhairinfo_ShadowLabel", "0", FCVAR_NONE, "Label Shadow (0 Off, 1 On)");
static ConVar hud_xhairinfo_ShadowAmount( "hud_xhairinfo_ShadowAmount", "1", FCVAR_NONE, "Amount Shadow (0 Off, 1 On)");

static ConVar hud_xhairinfo_ColorModeBar( "hud_xhairinfo_ColorModeBar", "2", FCVAR_NONE, "Bar color mode");
static ConVar hud_xhairinfo_ColorModeBarBackground( "hud_xhairinfo_ColorModeBarBackground", "2", FCVAR_NONE, "Bar Background color mode");
static ConVar hud_xhairinfo_ColorModeBarBorder( "hud_xhairinfo_ColorModeBarBorder", "2", FCVAR_NONE, "Bar Border color mode");
static ConVar hud_xhairinfo_ColorModeIcon( "hud_xhairinfo_ColorModeIcon", "2", FCVAR_NONE, "Icon color mode");
static ConVar hud_xhairinfo_ColorModeLabel( "hud_xhairinfo_ColorModeLabel", "0", FCVAR_NONE, "Label color mode");
static ConVar hud_xhairinfo_ColorModeAmount( "hud_xhairinfo_ColorModeAmount", "0", FCVAR_NONE, "Amount color mode");
static ConVar hud_xhairinfo_ColorModeIdent( "hud_xhairinfo_ColorModeIdent", "3", FCVAR_NONE, "Identifier (Class,Icon) color mode");

static ConVar hud_xhairinfo_offsetXBar( "hud_xhairinfo_offsetXBar", "0", FCVAR_NONE, "Bar offset x");
static ConVar hud_xhairinfo_offsetYBar( "hud_xhairinfo_offsetYBar", "0", FCVAR_NONE, "Bar offset y");
static ConVar hud_xhairinfo_offsetXIcon( "hud_xhairinfo_offsetXIcon", "-5", FCVAR_NONE, "Icon offset x");
static ConVar hud_xhairinfo_offsetYIcon( "hud_xhairinfo_offsetYIcon", "-2", FCVAR_NONE, "Icon offset y");
static ConVar hud_xhairinfo_offsetXLabel( "hud_xhairinfo_offsetXLabel", "1", FCVAR_NONE, "label offset x");
static ConVar hud_xhairinfo_offsetYLabel( "hud_xhairinfo_offsetYLabel", "0", FCVAR_NONE, "label offset y");
static ConVar hud_xhairinfo_offsetXAmount( "hud_xhairinfo_offsetXAmount", "-25", FCVAR_NONE, "Amount offset x");
static ConVar hud_xhairinfo_offsetYAmount( "hud_xhairinfo_offsetYAmount", "0", FCVAR_NONE, "Amount offset y");
static ConVar hud_xhairinfo_offsetXIdent( "hud_xhairinfo_offsetXIdent", "0", FCVAR_NONE, "Identifier (Class,Name,Icon) offset x");
static ConVar hud_xhairinfo_offsetYIdent( "hud_xhairinfo_offsetYIdent", "-30", FCVAR_NONE, "Identifier (Class,Name,Icon) offset y");

#define CROSSHAIRTYPE_NORMAL 0
#define CROSSHAIRTYPE_DISPENSER 1
#define CROSSHAIRTYPE_SENTRYGUN 2
#define CROSSHAIRTYPE_DETPACK 3
#define CROSSHAIRTYPE_ENEMY_SENTRYGUN 4
#define CROSSHAIRTYPE_FRIENDLY_SENTRYGUN 5

//=============================================================================
//
//	class CHudCrosshairInfo
//
//=============================================================================
class CHudCrosshairInfo : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CHudCrosshairInfo, vgui::Panel );
	void UpdateQuantityBars( void );

public:
	CHudCrosshairInfo( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudCrosshairInfo" )
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport() );
		// Hide when player is dead
		SetHiddenBits( HIDEHUD_PLAYERDEAD );

		vgui::ivgui()->AddTickSignal( GetVPanel() );

		m_flDuration = 0.1f;
		m_flDrawDuration = 1.0f;
		/*
		m_qbIdent = new CHudQuantityBar(this, "HudCrosshairInfoIdent");
		m_qbHealth = new CHudQuantityBar(this, "HudCrosshairInfoHealth");
		m_qbArmor = new CHudQuantityBar(this, "HudCrosshairInfoArmour");
		m_qbCells = new CHudQuantityBar(this, "HudCrosshairInfoCells");
		m_qbShells = new CHudQuantityBar(this, "HudCrosshairInfoShells");
		m_qbRockets = new CHudQuantityBar(this, "HudCrosshairInfoRockets");
		m_qbNails = new CHudQuantityBar(this, "HudCrosshairInfoNails");
		m_qbLevel = new CHudQuantityBar(this, "HudCrosshairInfoLevel");
		
		m_qbIdent->ShowAmount(false);
		m_qbIdent->ShowBar(false);
		m_qbIdent->ShowBarBackground(false);
		m_qbIdent->ShowBarBorder(false);

		m_qbLevel->SetIntensityControl(1,2,2,3);

		m_qbHealth->ShowAmountMax(false);

		m_qbRockets->SetAmountMax(50);
		m_qbLevel->SetAmountMax(3);

		m_qbHealth->SetLabelText("#FF_ITEM_HEALTH");
		m_qbArmor->SetLabelText("#FF_ITEM_ARMOR");
		m_qbCells->SetLabelText("#FF_ITEM_CELLS");
		m_qbShells->SetLabelText("#FF_ITEM_SHELLS");
		m_qbRockets->SetLabelText("#FF_ITEM_ROCKETS");
		m_qbNails->SetLabelText("#FF_ITEM_NAILS");
		m_qbLevel->SetLabelText("FF_ITEM_LEVEL");

		m_qbHealth->SetIconChar(":");
		m_qbArmor->SetIconChar(";");
		m_qbCells->SetIconChar("6");
		m_qbShells->SetIconChar("2");
		m_qbRockets->SetIconChar("8");
		m_qbNails->SetIconChar("7");
		m_qbLevel->ShowIcon(false);
		*/
}

	~CHudCrosshairInfo( void ) {}

	void Init( void );
	void VidInit( void );
	void OnTick( void );
	void Paint( void );
	virtual	bool	ShouldDraw( void );

	void Reset( void )
	{ 
		if( m_flDrawTime != 0.0f )
			m_flDrawTime = 0.0f; 
	}

protected:
	float		m_flStartTime;
	float		m_flDuration;
	wchar_t		m_pText[ 256 ];	// Unicode text buffer
	float		m_flDrawTime;
	float		m_flDrawDuration;

	float m_flXOffset;
	float m_flYOffset;

	// For color
	int			m_iTeam;
	int			m_iClass;
	/*
	CHudQuantityBar *m_qbIdent;
	CHudQuantityBar *m_qbHealth;
	CHudQuantityBar *m_qbArmor;
	CHudQuantityBar *m_qbRockets;
	CHudQuantityBar *m_qbCells;
	CHudQuantityBar *m_qbShells;
	CHudQuantityBar *m_qbNails;
	CHudQuantityBar *m_qbLevel;
*/
private:
	// Stuff we need to know 
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "ChatFont" );
	CPanelAnimationVar( vgui::HFont, m_hCrosshairInfo, "TextFont", "CrosshairInfo" );
	CPanelAnimationVar( vgui::HFont, m_hCrosshairInfoShadow, "TextFont", "CrosshairInfoShadow" );
	CPanelAnimationVar( vgui::HFont, m_hCrosshairInfoIcon, "IconFont", "CrosshairInfoIcon" );
	CPanelAnimationVar( vgui::HFont, m_hCrosshairInfoIconShadow, "IconFont", "CrosshairInfoIconShadow" );
	CPanelAnimationVar( vgui::HFont, m_hCrosshairInfoPlayerName, "TextFont", "CrosshairInfoPlayerName" );
	CPanelAnimationVar( vgui::HFont, m_hCrosshairInfoPlayerGlyph, "IconFont", "CrosshairInfoPlayerGlyph" );
	CPanelAnimationVarAliasType( float, text1_xpos, "text1_xpos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, text1_ypos, "text1_ypos", "20", "proportional_float" );
};

DECLARE_HUDELEMENT( CHudCrosshairInfo );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCrosshairInfo::Init( void )
{
	m_pText[ 0 ] = '\0';
}
void CHudCrosshairInfo::UpdateQuantityBars( void )
{
//QBAR 
/*
	int barWidth = hud_xhairinfo_barWidth.GetInt();
	int barHeight = hud_xhairinfo_barHeight.GetInt();
	int barBorderWidth = hud_xhairinfo_barBorderWidth.GetInt();

	int barColorMode = hud_xhairinfo_ColorModeBar.GetInt();
	int barBackgroundColorMode = hud_xhairinfo_ColorModeBarBackground.GetInt();
	int barBorderColorMode =  hud_xhairinfo_ColorModeBarBorder.GetInt();
	int iconColorMode = hud_xhairinfo_ColorModeIcon.GetInt();
	int labelColorMode = hud_xhairinfo_ColorModeLabel.GetInt();
	int amountColorMode = hud_xhairinfo_ColorModeAmount.GetInt();
	int identColorMode = hud_xhairinfo_ColorModeIdent.GetInt();
	
	Color barColor = *new Color(
		hud_xhairinfo_ColorBar_r.GetInt(),
		hud_xhairinfo_ColorBar_g.GetInt(),
		hud_xhairinfo_ColorBar_b.GetInt(),
		hud_xhairinfo_ColorBar_a.GetInt());
	Color barBackgroundColor = *new Color(
		hud_xhairinfo_ColorBarBackground_r.GetInt(),
		hud_xhairinfo_ColorBarBackground_g.GetInt(),
		hud_xhairinfo_ColorBarBackground_b.GetInt(),
		hud_xhairinfo_ColorBarBackground_a.GetInt());
	Color barBorderColor = *new Color(
		hud_xhairinfo_ColorBarBorder_r.GetInt(),
		hud_xhairinfo_ColorBarBorder_g.GetInt(),
		hud_xhairinfo_ColorBarBorder_b.GetInt(),
		hud_xhairinfo_ColorBarBorder_a.GetInt());
	Color iconColor = *new Color(
		hud_xhairinfo_ColorIcon_r.GetInt(),
		hud_xhairinfo_ColorIcon_g.GetInt(),
		hud_xhairinfo_ColorIcon_b.GetInt(),
		hud_xhairinfo_ColorIcon_a.GetInt());
	Color labelColor = *new Color(
		hud_xhairinfo_ColorLabel_r.GetInt(),
		hud_xhairinfo_ColorLabel_g.GetInt(),
		hud_xhairinfo_ColorLabel_b.GetInt(),
		hud_xhairinfo_ColorLabel_a.GetInt());
	Color amountColor = *new Color(
		hud_xhairinfo_ColorAmount_r.GetInt(),
		hud_xhairinfo_ColorAmount_g.GetInt(),
		hud_xhairinfo_ColorAmount_b.GetInt(),
		hud_xhairinfo_ColorAmount_a.GetInt());
	Color identColor = *new Color(
		hud_xhairinfo_ColorIdent_r.GetInt(),
		hud_xhairinfo_ColorIdent_g.GetInt(),
		hud_xhairinfo_ColorIdent_b.GetInt(),
		hud_xhairinfo_ColorIdent_a.GetInt());
	
	m_qbIdent->SetIconColor(identColor);
	m_qbIdent->SetLabelColor(identColor);
	m_qbIdent->SetIconColorMode(identColorMode);
	m_qbIdent->SetLabelColorMode(identColorMode);

	int red = hud_xhairinfo_intensity_red.GetInt();
	int orange = hud_xhairinfo_intensity_orange.GetInt();
	int yellow = hud_xhairinfo_intensity_yellow.GetInt();
	int green = hud_xhairinfo_intensity_green.GetInt();

	bool showBar = hud_xhairinfo_showBar.GetBool();
	bool showBarBackground = hud_xhairinfo_showBarBackground.GetBool();
	bool showBarBorder = hud_xhairinfo_showBarBorder.GetBool();
	bool showIcon = hud_xhairinfo_showIcon.GetBool();
	bool showLabel = hud_xhairinfo_showLabel.GetBool();
	bool showAmount = hud_xhairinfo_showAmount.GetBool();
	
	int barOffsetX = hud_xhairinfo_offsetXBar.GetInt();
	int barOffsetY = hud_xhairinfo_offsetYBar.GetInt();
	int iconOffsetX = hud_xhairinfo_offsetXIcon.GetInt();
	int iconOffsetY = hud_xhairinfo_offsetYIcon.GetInt();
	int labelOffsetX = hud_xhairinfo_offsetXLabel.GetInt();
	int labelOffsetY = hud_xhairinfo_offsetYLabel.GetInt();
	int amountOffsetX = hud_xhairinfo_offsetXAmount.GetInt();
	int amountOffsetY = hud_xhairinfo_offsetYAmount.GetInt();
	
	vgui::HFont amountFont,iconFont,labelFont;

	if(hud_xhairinfo_ShadowIcon.GetBool())
		iconFont = m_hCrosshairInfoIconShadow;
	else
		iconFont = m_hCrosshairInfoIcon;

	if(hud_xhairinfo_ShadowLabel.GetBool())
		labelFont = m_hCrosshairInfoShadow;
	else
		labelFont = m_hCrosshairInfo;

	if(hud_xhairinfo_ShadowAmount.GetBool())
		amountFont = m_hCrosshairInfoShadow;
	else
		amountFont = m_hCrosshairInfo;

	m_qbHealth->SetBarOffsetX(barOffsetX);
	m_qbArmor->SetBarOffsetX(barOffsetX);
	m_qbLevel->SetBarOffsetX(barOffsetX);
	m_qbCells->SetBarOffsetX(barOffsetX);
	m_qbShells->SetBarOffsetX(barOffsetX);
	m_qbRockets->SetBarOffsetX(barOffsetX);
	m_qbNails->SetBarOffsetX(barOffsetX);

	m_qbHealth->SetBarOffsetY(barOffsetY);
	m_qbArmor->SetBarOffsetY(barOffsetY);
	m_qbLevel->SetBarOffsetY(barOffsetY);
	m_qbCells->SetBarOffsetY(barOffsetY);
	m_qbShells->SetBarOffsetY(barOffsetY);
	m_qbRockets->SetBarOffsetY(barOffsetY);
	m_qbNails->SetBarOffsetY(barOffsetY);

	m_qbHealth->SetIconOffsetX(iconOffsetX);
	m_qbArmor->SetIconOffsetX(iconOffsetX);
	m_qbLevel->SetIconOffsetX(iconOffsetX);
	m_qbCells->SetIconOffsetX(iconOffsetX);
	m_qbShells->SetIconOffsetX(iconOffsetX);
	m_qbRockets->SetIconOffsetX(iconOffsetX);
	m_qbNails->SetIconOffsetX(iconOffsetX);

	m_qbHealth->SetIconOffsetY(iconOffsetY);
	m_qbArmor->SetIconOffsetY(iconOffsetY);
	m_qbLevel->SetIconOffsetY(iconOffsetY);
	m_qbCells->SetIconOffsetY(iconOffsetY);
	m_qbShells->SetIconOffsetY(iconOffsetY);
	m_qbRockets->SetIconOffsetY(iconOffsetY);
	m_qbNails->SetIconOffsetY(iconOffsetY);

	m_qbHealth->SetLabelOffsetX(labelOffsetX);
	m_qbArmor->SetLabelOffsetX(labelOffsetX);
	m_qbLevel->SetLabelOffsetX(labelOffsetX);
	m_qbCells->SetLabelOffsetX(labelOffsetX);
	m_qbShells->SetLabelOffsetX(labelOffsetX);
	m_qbRockets->SetLabelOffsetX(labelOffsetX);
	m_qbNails->SetLabelOffsetX(labelOffsetX);

	m_qbHealth->SetLabelOffsetY(labelOffsetY);
	m_qbArmor->SetLabelOffsetY(labelOffsetY);
	m_qbLevel->SetLabelOffsetY(labelOffsetY);
	m_qbCells->SetLabelOffsetY(labelOffsetY);
	m_qbShells->SetLabelOffsetY(labelOffsetY);
	m_qbRockets->SetLabelOffsetY(labelOffsetY);
	m_qbNails->SetLabelOffsetY(labelOffsetY);

	m_qbHealth->SetAmountOffsetX(amountOffsetX);
	m_qbArmor->SetAmountOffsetX(amountOffsetX);
	m_qbLevel->SetAmountOffsetX(amountOffsetX);
	m_qbCells->SetAmountOffsetX(amountOffsetX);
	m_qbShells->SetAmountOffsetX(amountOffsetX);
	m_qbRockets->SetAmountOffsetX(amountOffsetX);
	m_qbNails->SetAmountOffsetX(amountOffsetX);

	m_qbHealth->SetAmountOffsetY(amountOffsetY);
	m_qbArmor->SetAmountOffsetY(amountOffsetY);
	m_qbLevel->SetAmountOffsetY(amountOffsetY);
	m_qbCells->SetAmountOffsetY(amountOffsetY);
	m_qbShells->SetAmountOffsetY(amountOffsetY);
	m_qbRockets->SetAmountOffsetY(amountOffsetY);
	m_qbNails->SetAmountOffsetY(amountOffsetY);

	m_qbHealth->SetBarWidth(barWidth);
	m_qbArmor->SetBarWidth(barWidth);
	m_qbLevel->SetBarWidth(barWidth);
	m_qbCells->SetBarWidth(barWidth);
	m_qbShells->SetBarWidth(barWidth);
	m_qbRockets->SetBarWidth(barWidth);
	m_qbNails->SetBarWidth(barWidth);

	m_qbHealth->SetBarHeight(barHeight);
	m_qbArmor->SetBarHeight(barHeight);
	m_qbLevel->SetBarHeight(barHeight);
	m_qbCells->SetBarHeight(barHeight);
	m_qbShells->SetBarHeight(barHeight);
	m_qbRockets->SetBarHeight(barHeight);
	m_qbNails->SetBarHeight(barHeight);

	m_qbHealth->SetBarBorderWidth(barBorderWidth);
	m_qbArmor->SetBarBorderWidth(barBorderWidth);
	m_qbLevel->SetBarBorderWidth(barBorderWidth);
	m_qbCells->SetBarBorderWidth(barBorderWidth);
	m_qbShells->SetBarBorderWidth(barBorderWidth);
	m_qbRockets->SetBarBorderWidth(barBorderWidth);
	m_qbNails->SetBarBorderWidth(barBorderWidth);

	m_qbHealth->SetIntensityControl(red,orange,yellow,green);
	m_qbArmor->SetIntensityControl(red,orange,yellow,green);
	m_qbCells->SetIntensityControl(red,orange,yellow,green);
	m_qbShells->SetIntensityControl(red,orange,yellow,green);
	m_qbNails->SetIntensityControl(red,orange,yellow,green);
	m_qbRockets->SetIntensityControl((int)red/2,(int)orange/2,(int)yellow/2,(int)green/2);
	m_qbIdent->SetIntensityControl(red,orange,yellow,green);

	m_qbArmor->SetBarColor(barColor);
	m_qbHealth->SetBarColor(barColor);
	m_qbLevel->SetBarColor(barColor);
	m_qbCells->SetBarColor(barColor);
	m_qbShells->SetBarColor(barColor);
	m_qbRockets->SetBarColor(barColor);
	m_qbNails->SetBarColor(barColor);
	m_qbHealth->SetBarBackgroundColor(barBackgroundColor);
	m_qbArmor->SetBarBackgroundColor(barBackgroundColor);
	m_qbLevel->SetBarBackgroundColor(barBackgroundColor);
	m_qbCells->SetBarBackgroundColor(barBackgroundColor);
	m_qbShells->SetBarBackgroundColor(barBackgroundColor);
	m_qbRockets->SetBarBackgroundColor(barBackgroundColor);
	m_qbNails->SetBarBackgroundColor(barBackgroundColor);
	m_qbHealth->SetBarBorderColor(barBorderColor);
	m_qbArmor->SetBarBorderColor(barBorderColor);
	m_qbLevel->SetBarBorderColor(barBorderColor);
	m_qbCells->SetBarBorderColor(barBorderColor);
	m_qbShells->SetBarBorderColor(barBorderColor);
	m_qbRockets->SetBarBorderColor(barBorderColor);
	m_qbNails->SetBarBorderColor(barBorderColor);

	m_qbHealth->SetIconColor(iconColor);
	m_qbArmor->SetIconColor(iconColor);
	m_qbLevel->SetIconColor(iconColor);
	m_qbCells->SetIconColor(iconColor);
	m_qbShells->SetIconColor(iconColor);
	m_qbRockets->SetIconColor(iconColor);
	m_qbNails->SetIconColor(iconColor);
	m_qbHealth->SetLabelColor(labelColor);
	m_qbArmor->SetLabelColor(labelColor);
	m_qbLevel->SetLabelColor(labelColor);
	m_qbCells->SetLabelColor(labelColor);
	m_qbShells->SetLabelColor(labelColor);
	m_qbRockets->SetLabelColor(labelColor);
	m_qbNails->SetLabelColor(labelColor);
	m_qbHealth->SetAmountColor(amountColor);
	m_qbArmor->SetAmountColor(amountColor);
	m_qbLevel->SetAmountColor(amountColor);
	m_qbCells->SetAmountColor(amountColor);
	m_qbShells->SetAmountColor(amountColor);
	m_qbRockets->SetAmountColor(amountColor);
	m_qbNails->SetAmountColor(amountColor);

	m_qbArmor->SetBarColorMode(barColorMode);
	m_qbHealth->SetBarColorMode(barColorMode);
	m_qbLevel->SetBarColorMode(barColorMode);
	m_qbCells->SetBarColorMode(barColorMode);
	m_qbShells->SetBarColorMode(barColorMode);
	m_qbRockets->SetBarColorMode(barColorMode);
	m_qbNails->SetBarColorMode(barColorMode);
	m_qbHealth->SetBarBackgroundColorMode(barBackgroundColorMode);
	m_qbArmor->SetBarBackgroundColorMode(barBackgroundColorMode);
	m_qbLevel->SetBarBackgroundColorMode(barBackgroundColorMode);
	m_qbCells->SetBarBackgroundColorMode(barBackgroundColorMode);
	m_qbShells->SetBarBackgroundColorMode(barBackgroundColorMode);
	m_qbRockets->SetBarBackgroundColorMode(barBackgroundColorMode);
	m_qbNails->SetBarBackgroundColorMode(barBackgroundColorMode);
	m_qbHealth->SetBarBorderColorMode(barBorderColorMode);
	m_qbArmor->SetBarBorderColorMode(barBorderColorMode);
	m_qbLevel->SetBarBorderColorMode(barBorderColorMode);
	m_qbCells->SetBarBorderColorMode(barBorderColorMode);
	m_qbShells->SetBarBorderColorMode(barBorderColorMode);
	m_qbRockets->SetBarBorderColorMode(barBorderColorMode);
	m_qbNails->SetBarBorderColorMode(barBorderColorMode);

	m_qbHealth->SetIconColorMode(iconColorMode);
	m_qbArmor->SetIconColorMode(iconColorMode);
	m_qbLevel->SetIconColorMode(iconColorMode);
	m_qbCells->SetIconColorMode(iconColorMode);
	m_qbShells->SetIconColorMode(iconColorMode);
	m_qbRockets->SetIconColorMode(iconColorMode);
	m_qbNails->SetIconColorMode(iconColorMode);
	m_qbHealth->SetLabelColorMode(labelColorMode);
	m_qbArmor->SetLabelColorMode(labelColorMode);
	m_qbLevel->SetLabelColorMode(labelColorMode);
	m_qbCells->SetLabelColorMode(labelColorMode);
	m_qbShells->SetLabelColorMode(labelColorMode);
	m_qbRockets->SetLabelColorMode(labelColorMode);
	m_qbNails->SetLabelColorMode(labelColorMode);
	m_qbHealth->SetAmountColorMode(amountColorMode);
	m_qbArmor->SetAmountColorMode(amountColorMode);
	m_qbLevel->SetAmountColorMode(amountColorMode);
	m_qbCells->SetAmountColorMode(amountColorMode);
	m_qbShells->SetAmountColorMode(amountColorMode);
	m_qbRockets->SetAmountColorMode(amountColorMode);
	m_qbNails->SetAmountColorMode(amountColorMode);

	m_qbHealth->ShowBar(showBar);
	m_qbArmor->ShowBar(showBar);
	m_qbLevel->ShowBar(showBar);
	m_qbCells->ShowBar(showBar);
	m_qbShells->ShowBar(showBar);
	m_qbRockets->ShowBar(showBar);
	m_qbNails->ShowBar(showBar);
	m_qbHealth->ShowBarBackground(showBarBackground);
	m_qbArmor->ShowBarBackground(showBarBackground);
	m_qbLevel->ShowBarBackground(showBarBackground);
	m_qbCells->ShowBarBackground(showBarBackground);
	m_qbShells->ShowBarBackground(showBarBackground);
	m_qbRockets->ShowBarBackground(showBarBackground);
	m_qbNails->ShowBarBackground(showBarBackground);
	m_qbHealth->ShowBarBorder(showBarBorder);
	m_qbArmor->ShowBarBorder(showBarBorder);
	m_qbLevel->ShowBarBorder(showBarBorder);
	m_qbCells->ShowBarBorder(showBarBorder);
	m_qbShells->ShowBarBorder(showBarBorder);
	m_qbRockets->ShowBarBorder(showBarBorder);
	m_qbNails->ShowBarBorder(showBarBorder);

	m_qbHealth->ShowIcon(showIcon);
	m_qbArmor->ShowIcon(showIcon);
	m_qbLevel->ShowIcon(showIcon);
	m_qbCells->ShowIcon(showIcon);
	m_qbShells->ShowIcon(showIcon);
	m_qbRockets->ShowIcon(showIcon);
	m_qbNails->ShowIcon(showIcon);
	m_qbHealth->ShowLabel(showLabel);
	m_qbArmor->ShowLabel(showLabel);
	m_qbLevel->ShowLabel(showLabel);
	m_qbCells->ShowLabel(showLabel);
	m_qbShells->ShowLabel(showLabel);
	m_qbRockets->ShowLabel(showLabel);
	m_qbNails->ShowLabel(showLabel);
	m_qbHealth->ShowAmount(showAmount);
	m_qbArmor->ShowAmount(showAmount);
	m_qbLevel->ShowAmount(showAmount);
	m_qbCells->ShowAmount(showAmount);
	m_qbShells->ShowAmount(showAmount);
	m_qbRockets->ShowAmount(showAmount);
	m_qbNails->ShowAmount(showAmount);

	m_qbHealth->SetAmountFont(amountFont);
	m_qbArmor->SetAmountFont(amountFont);
	m_qbLevel->SetAmountFont(amountFont);
	m_qbCells->SetAmountFont(amountFont);
	m_qbShells->SetAmountFont(amountFont);
	m_qbRockets->SetAmountFont(amountFont);
	m_qbNails->SetAmountFont(amountFont);

	m_qbHealth->SetLabelFont(labelFont);
	m_qbArmor->SetLabelFont(labelFont);
	m_qbLevel->SetLabelFont(labelFont);
	m_qbCells->SetLabelFont(labelFont);
	m_qbShells->SetLabelFont(labelFont);
	m_qbRockets->SetLabelFont(labelFont);
	m_qbNails->SetLabelFont(labelFont);

	m_qbHealth->SetIconFont(iconFont);
	m_qbArmor->SetIconFont(iconFont);
	m_qbLevel->SetIconFont(iconFont);
	m_qbCells->SetIconFont(iconFont);
	m_qbShells->SetIconFont(iconFont);
	m_qbRockets->SetIconFont(iconFont);
	m_qbNails->SetIconFont(iconFont);
*/
}
//-----------------------------------------------------------------------------
// Purpose: Reset on map/vgui load
//-----------------------------------------------------------------------------
void CHudCrosshairInfo::VidInit( void )
{	
	SetPaintBackgroundEnabled( false );
	m_pText[ 0 ] = '\0';
	m_flStartTime = 0.0f;		// |-- Mirv: Fix messages reappearing next map
	m_flDrawTime = 0.0f;
	m_iTeam = 0;
	m_iClass = 0;
	
	hud_xhairinfo_update.SetValue(1);
	/*
	m_qbIdent->SetLabelOffsetX(-20);
	m_qbIdent->SetLabelOffsetY(0);
	*/
}

//-----------------------------------------------------------------------------
// Purpose: Trace out and touch someone
//-----------------------------------------------------------------------------
void CHudCrosshairInfo::OnTick( void )
{
	if( !engine->IsInGame() )
		return;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return;

	if( !pPlayer->IsAlive() )
		Reset();

	// Check for crosshair info every x seconds
	if( m_flStartTime < gpGlobals->curtime )
	{
		// Store off when to trace next
		m_flStartTime = gpGlobals->curtime + m_flDuration;

		// Get our forward vector
		Vector vecForward;
		pPlayer->EyeVectors( &vecForward );

		VectorNormalize( vecForward );

		// Get eye position
		Vector vecOrigin = pPlayer->EyePosition();

		//debugoverlay->AddLineOverlay( vecOrigin + ( vecForward * 64.f ), vecOrigin + ( vecForward * 1024.f ), 0, 0, 255, false, 3.0f );
		//debugoverlay->AddLineOverlay( vecOrigin + ( vecForward * 64.f ), vecOrigin + ( vecForward * 64.f ) + Vector( 0, 0, 8 ), 255, 0, 0, false, 3.0f );
		//debugoverlay->AddLineOverlay( vecOrigin + ( vecForward * 64.f ), vecOrigin + ( vecForward * 64.f ) + Vector( 0, 0, -8 ), 255, 0, 0, false, 3.0f );

		trace_t tr;
		UTIL_TraceLine( vecOrigin, vecOrigin + ( vecForward * 1024.f ), MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_PLAYER, &tr );

		// If we hit something...
		if( tr.DidHit() )
		{
			// Some defaults...
			bool bBuildable = false;
			C_FFPlayer *pHitPlayer = NULL;

			// If we hit a player
			if( tr.m_pEnt->IsPlayer() )
			{					
				if( tr.m_pEnt->IsAlive() )
					pHitPlayer = ToFFPlayer( tr.m_pEnt );
			}
			// If we hit a sentrygun
			else if( tr.m_pEnt->Classify() == CLASS_SENTRYGUN )
			{
				C_FFSentryGun *pSentryGun = ( C_FFSentryGun * )tr.m_pEnt;
				if( !pSentryGun->IsBuilt() )
					return;					

				if( pSentryGun->IsAlive() )
				{
					bBuildable = true;
					pHitPlayer = ToFFPlayer( pSentryGun->m_hOwner.Get() );
				}
			}
			// If we hit a dispenser
			else if( tr.m_pEnt->Classify() == CLASS_DISPENSER )
			{
				C_FFDispenser *pDispenser = ( C_FFDispenser * )tr.m_pEnt;
				if( !pDispenser->IsBuilt() )
					return;

				if( pDispenser->IsAlive() )
				{
					bBuildable = true;
					pHitPlayer = ToFFPlayer( pDispenser->m_hOwner.Get() );
				}					
			}
			// If we hit a man cannon
			else if( tr.m_pEnt->Classify() == CLASS_MANCANNON )
			{
				C_FFManCannon *pManCannon = (C_FFManCannon *)tr.m_pEnt;
				if( !pManCannon->IsBuilt() )
					return;

				if( pManCannon->IsAlive() )
				{
					bBuildable = true;
					pHitPlayer = ToFFPlayer( pManCannon->m_hOwner.Get() );
				}
			}

			// If the players/objects aren't "alive" pHitPlayer will still be NULL
			// and we'll bail here...

			// If we got a player/owner
			if( pHitPlayer )
			{
				// Get at the game resources
				IGameResources *pGR = GameResources();
				if( !pGR )
				{
					Warning( "[Crosshair Info] Failed to get game resources!\n" );
					return;
				}

				// Are we a medic?
				//bool bWeMedic = ( pPlayer->GetClassSlot() == CLASS_MEDIC );
				// Are we an engineer?
				//bool bWeEngy = ( pPlayer->GetClassSlot() == 9 );
				// Are we looking at a spy?
				bool bTheySpy = ( pHitPlayer->GetClassSlot() == CLASS_SPY );
				
				// For the player/owner name
				char szName[ MAX_PLAYER_NAME_LENGTH ];
				// For the class
				char szClass[ MAX_PLAYER_NAME_LENGTH ];
					
				// Get their real name now (deal w/ non teammate/ally spies later)
				Q_strcpy( szName, pGR->GetPlayerName( pHitPlayer->index ) );

				//QBAR m_qbIdent->SetLabelText(szName);

				if( bBuildable )
				{
					switch( tr.m_pEnt->Classify() )
					{
						case CLASS_SENTRYGUN:
							Q_strcpy( szClass, "#FF_PLAYER_SENTRYGUN" );
							break;
						case CLASS_DISPENSER:
							Q_strcpy( szClass, "#FF_PLAYER_DISPENSER" );
							break;
						case CLASS_MANCANNON:
							Q_strcpy( szClass, "#FF_PLAYER_MANCANNON" );
							break;
					}
				}
				else // Get the players' class always
					Q_strcpy( szClass, Class_IntToResourceString( pGR->GetClass( pHitPlayer->index ) ) );
								
				// Default
				int iHealth = -1, iArmor = -1, iCells = -1, iRockets = -1, iNails = -1, iShells = -1, iLevel = -1, iFuseTime = -1;

				int CROSSHAIRTYPE = CROSSHAIRTYPE_NORMAL;
				// Default
				m_iTeam = pHitPlayer->GetTeamNumber();
				m_iClass = pHitPlayer->GetTeamNumber();

				if ( (pPlayer == pHitPlayer) && (bBuildable) ) // looking at our own buildable
				{
					C_FFBuildableObject *pBuildable = ( C_FFBuildableObject * )tr.m_pEnt;
					iHealth = pBuildable->GetHealthPercent();

					if( pBuildable->Classify() == CLASS_DISPENSER )
					{
						CROSSHAIRTYPE = CROSSHAIRTYPE_DISPENSER;
						iRockets = ( ( C_FFDispenser * )pBuildable )->GetRockets();
						iShells = ( ( C_FFDispenser * )pBuildable )->GetShells();
						iCells = ( ( C_FFDispenser * )pBuildable )->GetCells();
						iNails = ( ( C_FFDispenser* )pBuildable )->GetNails();
						iArmor = ( ( C_FFDispenser * )pBuildable )->GetArmor();
					}
					else if( pBuildable->Classify() == CLASS_SENTRYGUN )
					{
						CROSSHAIRTYPE = CROSSHAIRTYPE_SENTRYGUN;
						iLevel = ( ( C_FFSentryGun * )pBuildable )->GetLevel();
						//iRockets = ( ( C_FFSentryGun * )pBuildable )->GetRocketsPercent();
						//iShells = ( ( C_FFSentryGun * )pBuildable )->GetShellsPercent();
						//iArmor = ( ( C_FFSentryGun * )pBuildable )->GetSGArmorPercent();

						//if (iArmor >= 128) //VOOGRU: when the sg has no rockets it would show ammopercent+128.
						//	iArmor -= 128;
					}
					else if( pBuildable->Classify() == CLASS_DETPACK )
					{
						// Doesnt work atm - aftershock
						//CROSSHAIRTYPE = CROSSHAIRTYPE_DETPACK;
						//iFuseTime = ( ( C_FFDetpack * )pBuildable )->GetFuseTime();
						iArmor = -1;
					}
					else if( pBuildable->Classify() == CLASS_MANCANNON )
					{
						// TODO: Do whatever other stuff is doing w/ the crosshair
						iArmor = -1;
					}
					else
					{
						iArmor = -1;
					}
				}
				else if( FFGameRules()->PlayerRelationship( pPlayer, pHitPlayer ) == GR_TEAMMATE )
				{
					// We're looking at a teammate/ally

					if( bBuildable )
					{
						// Now on teammates/allies can see teammate/allies
						// buildable info according to:
						// Bug #0000463: Hud Crosshair Info - douched
						C_FFBuildableObject *pBuildable = (C_FFBuildableObject *)tr.m_pEnt;

						iHealth = pBuildable->GetHealthPercent();
							
						if( pBuildable->Classify() == CLASS_DISPENSER )
						{
							iArmor = ((C_FFDispenser *)pBuildable)->GetAmmoPercent();
							//QBAR m_qbArmor->ShowAmountMax(true);
						}
						else if( pBuildable->Classify() == CLASS_SENTRYGUN )
						{
							//You see a friendly sentrygun -GreenMushy
							CROSSHAIRTYPE = CROSSHAIRTYPE_FRIENDLY_SENTRYGUN;
						}
						else if( pBuildable->Classify() == CLASS_MANCANNON )
						{
							// TODO: Maybe man cannon's have armor? or some other property we want to show?
							iArmor = -1;
						}
						else
						{
							iArmor = -1;
						}
					}
					else
					{						
						iHealth = pHitPlayer->GetHealthPercentage();
						iArmor = pHitPlayer->GetArmorPercentage();
						//QBAR m_qbArmor->ShowAmountMax(false);
					}
				}
				else
				{
					if( bBuildable )
					{
						C_FFBuildableObject *pBuildable = (C_FFBuildableObject *)tr.m_pEnt;

						iHealth = pBuildable->GetHealthPercent();
							
						if( pBuildable->Classify() == CLASS_DISPENSER )
						{
							iArmor = ((C_FFDispenser *)pBuildable)->GetAmmoPercent();
						}
						else if( pBuildable->Classify() == CLASS_SENTRYGUN )
						{
							//Enemy sentrygun -GreenMushy
							CROSSHAIRTYPE = CROSSHAIRTYPE_ENEMY_SENTRYGUN;
						}
						else if( pBuildable->Classify() == CLASS_MANCANNON )
						{
							// TODO: Maybe man cannon's have armor? or some other property we want to show?
							iArmor = -1;
						}
						else
						{
							iArmor = -1;
						}
					}
					else 
					{
						// We're looking at a player

						//if( bWeMedic ) //AfterShock: Testing every class having this ability
						//{
							// Grab the real health/armor of this player
							iHealth = pHitPlayer->GetHealthPercentage();
							iArmor = pHitPlayer->GetArmorPercentage();
							//QBAR m_qbArmor->ShowAmountMax(false);
						//}
							
						if( bTheySpy )
						{
							// We're looking at an enemy/non-allied spy

							if( pHitPlayer->IsDisguised() )
							{
								// The spy is disguised so we do some special stuff
								// to try and fake out the player - like show the class
								// we're disguised as and try to steal a name from a
								// a player on whatever team we are disguised as playing
								// as whatever class we are disguised as. If that fails
								// we use a name from the team we're disguised as. If that
								// fails we use the real name.

								// Get the disguised class
								/*int iClassSlot*/ m_iClass = pHitPlayer->GetDisguisedClass();
								Q_strcpy( szClass, Class_IntToResourceString( m_iClass ) );

								// Get the disguised team
								m_iTeam = pHitPlayer->GetDisguisedTeam();

								// If this spy is disguised as our team we need to show his
								// health/armor
								if( m_iTeam == pPlayer->GetTeamNumber() )
								{
									iHealth = pHitPlayer->GetHealthPercentage();
									iArmor = pHitPlayer->GetArmorPercentage();
								}

								// Or, if this spy is disguised as an ally of our team we
								// need to show his health/armor
								if( FFGameRules()->IsTeam1AlliedToTeam2( pPlayer->GetTeamNumber(), m_iTeam ) == GR_TEAMMATE )
								{
									iHealth = pHitPlayer->GetHealthPercentage();
									iArmor = pHitPlayer->GetArmorPercentage();
								}

								// TODO: Could be bugs with this spy tracking thing in that
								// a player who's name is being used as a spy ID drops
								// and that name is still being used because the spy hasn't
								// changed disguises...

								// Check to see if we've ID'd this spy before as the 
								// disguise he's currently disguised as
								if( pPlayer->m_hSpyTracking[ pHitPlayer->index ].SameGuy( m_iTeam, m_iClass ) )
									Q_strcpy( szName, pPlayer->m_hSpyTracking[ pHitPlayer->index ].m_szName );
								else
								{
									// Change name (if we can) to someone on the team iTeam
									// that is playing the class this guy is disguised as

									// Gonna generate an array of people on the team we're disguised as
									// in case we have to randomly pick a name later
									int iPlayers[ 128 ], iCount = 0;

									bool bDone = false;
									for( int i = 1; ( i < gpGlobals->maxClients ) && ( !bDone ); i++ )
									{
										// Skip this spy - kind of useless if it tells us
										// our real name, eh? Using our real name is a last resort
										if( i == pHitPlayer->index )
											continue;

										if( pGR->IsConnected( i ) )
										{
											// If the guy's on the team we're disguised as...
											if( pGR->GetTeam( i ) == m_iTeam )
											{
												// Store off the player index since we found
												// someone on the team we're disguised as
												iPlayers[ iCount++ ] = i;

												// If the guy's playing as the class we're disguised as...
												if( pGR->GetClass( i ) == m_iClass )
												{
													// We're stealing this guys name
													Q_strcpy( szName, pGR->GetPlayerName( i ) ) ;
													bDone = true; // bail
												}
											}
										}
									}

									// If no one was on the other team, add the real name
									// to the array of possible choices
									if( iCount == 0 )
										iPlayers[ iCount++ ] = pHitPlayer->index;
	
									// We iterated around and found no one on the team we're disguised as
									// playing as the class we're disguised as so just pick a guy from
									// the team we're disguised as (or use real name if iCount was 0)
									if( !bDone )
									{
										// So we got an array of indexes to players of whom we can steal
										// their name, so randomly steal one
										Q_strcpy( szName, pGR->GetPlayerName( iPlayers[ random->RandomInt( 0, iCount - 1 ) ] ) );
									}

									// Store off the spies name, class & team in case we ID him again
									// and he hasn't changed disguise
									pPlayer->m_hSpyTracking[ pHitPlayer->index ].Set( szName, m_iTeam, m_iClass );
								}
							}
							// Jiggles: Don't draw anything if we're looking at a cloaked enemy spy
							if( pHitPlayer->IsCloaked() )
								return;  
						}
					}					
				}

				// Set up local crosshair info struct
				pPlayer->m_hCrosshairInfo.Set( szName, m_iTeam, m_iClass );

				// Get the screen width/height
				int iScreenWide, iScreenTall;
				surface()->GetScreenSize( iScreenWide, iScreenTall );

				// "map" screen res to 640/480
				float flXScale = 640.0f / iScreenWide;
				float flYScale = 480.0f / iScreenTall;
				
				const char *pszOldName = szName;
				int iBufSize = ( int )strlen( pszOldName ) * 2;
				char *pszNewName = ( char * )_alloca( iBufSize );

				UTIL_MakeSafeName( pszOldName, pszNewName, iBufSize );

				wchar_t wszName[ 256 ];
				vgui::localize()->ConvertANSIToUnicode( pszNewName, wszName, sizeof( wszName ) );

				wchar_t wszClass[ 256 ];					
				wchar_t *pszTemp = vgui::localize()->Find( szClass );
				if( pszTemp )
					wcscpy( wszClass, pszTemp );
				else
				{
					wcscpy( wszClass, L"CLASS" );	// TODO: fix to show English version of class name :/
				}

				if(!hud_xhairinfo_newStyle.GetBool())
				{
					// NOW! Remember team is 1 higher than the actual team
					// If health/armor are -1 then we don't show it

					// Convert to unicode & localize stuff
					
					if (CROSSHAIRTYPE == CROSSHAIRTYPE_DISPENSER)
					{
						char szHealth[ 5 ], szArmor[ 5 ], szRockets[ 5 ], szShells[ 5 ], szCells[ 5 ], szNails[ 5 ];
						Q_snprintf( szHealth, 5, "%i%%", iHealth );
						Q_snprintf( szArmor, 5, "%i", iArmor );
						Q_snprintf( szRockets, 5, "%i", iRockets );
						Q_snprintf( szShells, 5, "%i", iShells );
						Q_snprintf( szCells, 5, "%i", iCells );
						Q_snprintf( szNails, 5, "%i", iNails );
						
						wchar_t wszHealth[ 10 ], wszArmor[ 10 ], wszRockets[ 10 ], wszCells[ 10 ], wszShells[ 10 ], wszNails[ 10 ];

						vgui::localize()->ConvertANSIToUnicode( szHealth, wszHealth, sizeof( wszHealth ) );
						vgui::localize()->ConvertANSIToUnicode( szArmor, wszArmor, sizeof( wszArmor ) );
						vgui::localize()->ConvertANSIToUnicode( szRockets, wszRockets, sizeof( wszRockets ) );
						vgui::localize()->ConvertANSIToUnicode( szCells, wszCells, sizeof( wszCells ) );
						vgui::localize()->ConvertANSIToUnicode( szShells, wszShells, sizeof( wszShells ) );
						vgui::localize()->ConvertANSIToUnicode( szNails, wszNails, sizeof( wszNails ) );
						
						_snwprintf( m_pText, 255, L"Your Dispenser - Cells(%s) Rkts(%s) Nls(%s) Shls(%s) Armr(%s)", wszCells, wszRockets, wszNails, wszShells, wszArmor );
					}
					else if (CROSSHAIRTYPE == CROSSHAIRTYPE_SENTRYGUN)
					{
						char szHealth[ 5 ], /*szRockets[ 5 ], szShells[ 5 ],*/ szLevel[ 5 ];//, szArmor[ 5 ];
						Q_snprintf( szHealth, 5, "%i%%", iHealth );
						Q_snprintf( szLevel, 5, "%i", iLevel );
						//Q_snprintf( szRockets, 5, "%i%%", iRockets );
						//Q_snprintf( szShells, 5, "%i%%", iShells );
						//Q_snprintf( szArmor, 5, "%i%%", iArmor );

						
						wchar_t wszHealth[ 10 ], /*wszRockets[ 10 ], wszShells[ 10 ],*/ wszLevel[ 10 ];//, wszArmor[ 10 ];

						vgui::localize()->ConvertANSIToUnicode( szHealth, wszHealth, sizeof( wszHealth ) );
						//vgui::localize()->ConvertANSIToUnicode( szRockets, wszRockets, sizeof( wszRockets ) );
						vgui::localize()->ConvertANSIToUnicode( szLevel, wszLevel, sizeof( wszLevel ) );
						//vgui::localize()->ConvertANSIToUnicode( szShells, wszShells, sizeof( wszShells ) );
						//vgui::localize()->ConvertANSIToUnicode( szArmor, wszArmor, sizeof( wszArmor ) );
						
						_snwprintf( m_pText, 255, L"Your Sentry Gun: Level %s - Health: %s ", wszLevel, wszHealth );
					
					}
					else if (CROSSHAIRTYPE == CROSSHAIRTYPE_DETPACK)
					{
						char szFuseTime[ 5 ];
						Q_snprintf( szFuseTime, 5, "%i", iFuseTime );

						
						wchar_t wszFuseTime[ 10 ];

						vgui::localize()->ConvertANSIToUnicode( szFuseTime, wszFuseTime, sizeof( wszFuseTime ) );
						
						_snwprintf( m_pText, 255, L"Your %s Second Detpack", wszFuseTime );
					}
					else if( CROSSHAIRTYPE == CROSSHAIRTYPE_ENEMY_SENTRYGUN )
					{
						char szHealth[ 5 ];
						Q_snprintf( szHealth, 5, "%i%%", iHealth );
						
						wchar_t wszHealth[ 10 ];
						vgui::localize()->ConvertANSIToUnicode( szHealth, wszHealth, sizeof( wszHealth ) );

						_snwprintf( m_pText, 255, L"(%s) %s - H: %s", wszClass, wszName, wszHealth );
					}
					else if( CROSSHAIRTYPE == CROSSHAIRTYPE_FRIENDLY_SENTRYGUN )
					{
						char szHealth[ 5 ];
						Q_snprintf( szHealth, 5, "%i%%", iHealth );
						
						wchar_t wszHealth[ 10 ];
						vgui::localize()->ConvertANSIToUnicode( szHealth, wszHealth, sizeof( wszHealth ) );

						_snwprintf( m_pText, 255, L"(%s) %s - H: %s", wszClass, wszName, wszHealth );
					}
					// else CROSSHAIRTYPE_NORMAL
					else if( ( iHealth != -1 ) && ( iArmor != -1 ) )
					{
						char szHealth[ 5 ], szArmor[ 5 ];
						Q_snprintf( szHealth, 5, "%i%%", iHealth );
						Q_snprintf( szArmor, 5, "%i%%", iArmor );
						
						wchar_t wszHealth[ 10 ], wszArmor[ 10 ];

						   vgui::localize()->ConvertANSIToUnicode( szHealth, wszHealth, sizeof( wszHealth ) );
						vgui::localize()->ConvertANSIToUnicode( szArmor, wszArmor, sizeof( wszArmor ) );

						_snwprintf( m_pText, 255, L"(%s) %s - H: %s, A: %s", wszClass, wszName, wszHealth, wszArmor );
					}
					else
						_snwprintf( m_pText, 255, L"(%s) %s", wszClass, wszName );

					if( hud_centerid.GetInt() )
					{
						int iWide;
						int iTall;
						
						surface()->GetTextSize(m_hTextFont, m_pText, iWide, iTall);

						// Adjust values to get below the crosshair and offset correctly
						m_flXOffset = ( float )( iScreenWide / 2 ) - ( iWide / 2 );
						m_flYOffset = ( float )( iScreenTall / 2 ) + ( iTall / 2 ) + 75; // 75 to get it below the crosshair and not right on it

						// Scale by "map" scale values
						m_flXOffset *= flXScale;
						m_flYOffset *= flYScale;

						// Scale to screen co-ords
						m_flXOffset = scheme()->GetProportionalScaledValue( m_flXOffset );
						m_flYOffset = scheme()->GetProportionalScaledValue( m_flYOffset );
					}
				}
				else
				{
					wchar_t wsz_Label[256];
					if(Q_stricmp(szClass,"#FF_PLAYER_SCOUT") == 0)
					{
						//QBAR m_qbIdent->SetIconChar("!");
						_snwprintf( wsz_Label, 255, L"%s", wszName, wszClass );
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_SNIPER") == 0)
					{
						//QBAR m_qbIdent->SetIconChar("@");
						_snwprintf( wsz_Label, 255, L"%s", wszName, wszClass );
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_SOLDIER") == 0)
					{
						//QBAR m_qbIdent->SetIconChar("#");
						_snwprintf( wsz_Label, 255, L"%s", wszName, wszClass );
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_DEMOMAN") == 0)
					{
						//QBAR m_qbIdent->SetIconChar("$");
						_snwprintf( wsz_Label, 255, L"%s", wszName, wszClass );
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_MEDIC") == 0)
					{
						//QBAR m_qbIdent->SetIconChar("%");
						_snwprintf( wsz_Label, 255, L"%s", wszName, wszClass );
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_HWGUY") == 0)
					{
						//QBAR m_qbIdent->SetIconChar("^");
						_snwprintf( wsz_Label, 255, L"%s", wszName, wszClass );
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_SPY") == 0)
					{
						//QBAR m_qbIdent->SetIconChar("*");
						_snwprintf( wsz_Label, 255, L"%s", wszName, wszClass );
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_PYRO") == 0)
					{
						//QBAR m_qbIdent->SetIconChar("?");
						_snwprintf( wsz_Label, 255, L"%s", wszName, wszClass );
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_ENGINEER") == 0)
					{
						//QBAR m_qbIdent->SetIconChar("(");
						_snwprintf( wsz_Label, 255, L"%s", wszName, wszClass );
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_CIVILIAN") == 0)
					{
						//QBAR m_qbIdent->SetIconChar(")");
						_snwprintf( wsz_Label, 255, L"%s", wszName, wszClass );
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_SENTRYGUN") == 0)

					{
						//QBAR m_qbIdent->SetIconChar("R");
						if (CROSSHAIRTYPE == CROSSHAIRTYPE_SENTRYGUN)
							_snwprintf( wsz_Label, 255, L"Your Sentry Gun" );
						else
							_snwprintf( wsz_Label, 255, L"%s's Sentry Gun", wszName );
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_DISPENSER") == 0)

					{
						//QBAR m_qbIdent->SetIconChar("Q");
						if (CROSSHAIRTYPE == CROSSHAIRTYPE_DISPENSER)
							_snwprintf( wsz_Label, 255, L"Your Dispenser" );
						else
							_snwprintf( wsz_Label, 255, L"%s's Dispenser", wszName );
					}
					else

					{
						//QBAR m_qbIdent->SetIconChar("_");
						_snwprintf( wsz_Label, 255, L"" );
					}
					//QBAR m_qbIdent->SetLabelText(wsz_Label);

					if(hud_xhairinfo_update.GetBool() || hud_xhairinfo_updateAlways.GetBool())
					{
						//reset it so that it does not update again
						hud_xhairinfo_update.SetValue(0);
						UpdateQuantityBars();
					}

					//QBAR 
					/*

					Color teamColor;
					SetColorByTeam( m_iTeam, teamColor );

					int iLeft = hud_xhairinfo_x.GetInt(), iTop = hud_xhairinfo_y.GetInt();
					int itemOffsetY = hud_xhairinfo_itemOffsetY.GetInt();
					int itemOffsetX = hud_xhairinfo_itemOffsetX.GetInt();
					int itemsPerRow = hud_xhairinfo_itemsPerRow.GetInt();
					int iRow = 0,iOffsetY = 0,iOffsetX = 0;

					// Get the screen width/height
					int iScreenWide, iScreenTall;
					surface()->GetScreenSize( iScreenWide, iScreenTall );

					// "map" screen res to 640/480
					float flXScale = 640.0f / iScreenWide;
					float flYScale = 480.0f / iScreenTall;

					if(hud_xhairinfo_showIdent.GetBool())
					{
						m_qbIdent->SetAmount(iHealth);
						m_qbIdent->SetTeamColor(teamColor);
						m_qbIdent->SetIconFont(m_hCrosshairInfoPlayerGlyph);
						m_qbIdent->SetLabelFont(m_hCrosshairInfoPlayerName);
						m_qbIdent->SetScaleX(flXScale);
						m_qbIdent->SetScaleY(flYScale);
						m_qbIdent->SetPosition(iLeft + hud_xhairinfo_offsetXIdent.GetInt(), iTop + hud_xhairinfo_offsetYIdent.GetInt());

					}
					if(iHealth > -1)
					{
						m_qbHealth->SetAmount(iHealth);
						m_qbHealth->SetTeamColor(teamColor);
						m_qbHealth->SetScaleX(flXScale);
						m_qbHealth->SetScaleY(flYScale);
						m_qbHealth->SetPosition(iLeft + iOffsetX, iTop + iOffsetY);
						m_qbHealth->SetVisible(true);
						++iRow;
						if(iRow >= itemsPerRow)	
						{
							iRow = 0;
							iOffsetY += itemOffsetY;
						}
						else
							iOffsetX += itemOffsetX;
					}
					else
						m_qbHealth->SetVisible(false);

					if(iArmor > -1)
					{
						m_qbArmor->SetAmount(iArmor);
						m_qbArmor->SetTeamColor(teamColor);
						m_qbArmor->SetScaleX(flXScale);
						m_qbArmor->SetScaleY(flYScale);
						m_qbArmor->SetPosition(iLeft + iOffsetX, iTop + iOffsetY);
						m_qbArmor->SetVisible(true);
						++iRow;
						if(iRow >= itemsPerRow)	
						{
							iRow = 0;
							iOffsetX = 0;
							iOffsetY += itemOffsetY;
						}
						else
							iOffsetX += itemOffsetX;
					}
					else
						m_qbArmor->SetVisible(false);

					if(iLevel > -1)
					{
						m_qbLevel->SetAmount(iLevel);
						m_qbLevel->SetTeamColor(teamColor);
						m_qbLevel->SetScaleX(flXScale);
						m_qbLevel->SetScaleY(flYScale);
						m_qbLevel->SetPosition(iLeft + iOffsetX, iTop + iOffsetY);
						m_qbLevel->SetVisible(true);
						++iRow;
						if(iRow >= itemsPerRow)	
						{
							iRow = 0;
							iOffsetX = 0;
							iOffsetY += itemOffsetY;
						}
						else
							iOffsetX += itemOffsetX;
					}
					else
						m_qbLevel->SetVisible(false);

					if(iCells > -1)
					{
						m_qbCells->SetAmount(iCells);
						m_qbCells->SetTeamColor(teamColor);
						m_qbCells->SetScaleX(flXScale);
						m_qbCells->SetScaleY(flYScale);
						m_qbCells->SetPosition(iLeft + iOffsetX, iTop + iOffsetY);
						m_qbCells->SetVisible(true);
						++iRow;
						if(iRow >= itemsPerRow)	
						{
							iRow = 0;
							iOffsetX = 0;
							iOffsetY += itemOffsetY;
						}
						else
							iOffsetX += itemOffsetX;
					}
					else
						m_qbCells->SetVisible(false);

					if(iShells > -1)
					{
						m_qbShells->SetAmount(iShells);
						m_qbShells->SetTeamColor(teamColor);
						m_qbShells->SetScaleX(flXScale);
						m_qbShells->SetScaleY(flYScale);
						m_qbShells->SetPosition(iLeft + iOffsetX, iTop + iOffsetY);
						m_qbShells->SetVisible(true);
						++iRow;
						if(iRow >= itemsPerRow)	
						{
							iRow = 0;
							iOffsetX = 0;
							iOffsetY += itemOffsetY;
						}
						else
							iOffsetX += itemOffsetX;
					}
					else
						m_qbShells->SetVisible(false);

					if(iRockets > -1)
					{
						m_qbRockets->SetAmount(iRockets);
						m_qbRockets->SetTeamColor(teamColor);
						m_qbRockets->SetScaleX(flXScale);
						m_qbRockets->SetScaleY(flYScale);
						m_qbRockets->SetPosition(iLeft + iOffsetX, iTop + iOffsetY);
						m_qbRockets->SetVisible(true);
						++iRow;
						if(iRow >= itemsPerRow)	
						{
							iRow = 0;
							iOffsetX = 0;
							iOffsetY += itemOffsetY;
						}
						else
							iOffsetX += itemOffsetX;
					}
					else
						m_qbRockets->SetVisible(false);

					if(iNails > -1)
					{
						m_qbNails->SetAmount(iNails);
						m_qbNails->SetTeamColor(teamColor);
						m_qbNails->SetScaleX(flXScale);
						m_qbNails->SetScaleY(flYScale);
						m_qbNails->SetPosition(iLeft + iOffsetX, iTop + iOffsetY);
						m_qbNails->SetVisible(true);
						++iRow;
						if(iRow >= itemsPerRow)	
						{
							iRow = 0;
							iOffsetX = 0;
							iOffsetY += itemOffsetY;
						}
						else
							iOffsetX += itemOffsetX;
					}
					else
						m_qbNails->SetVisible(false);
					*/
				}
				// Start drawing
				m_flDrawTime = gpGlobals->curtime;
			}
			else
			{
				// Hit something but not a player/dispenser/sentrygun
				pPlayer->m_hCrosshairInfo.Set( "", 0, 0 );
			}
		}
		else
		{
			// Didn't hit anything!
			pPlayer->m_hCrosshairInfo.Set( "", 0, 0 );
		}
	}

	SetPos( 0, 0);
	SetWide( scheme()->GetProportionalScaledValue( 640 ) );
	SetTall( scheme()->GetProportionalScaledValue( 480 ) );
}

//-----------------------------------------------------------------------------
// Purpose: See if we should draw stuff or not
//-----------------------------------------------------------------------------
bool CHudCrosshairInfo::ShouldDraw( void )
{
	if( !engine->IsInGame() )
		return false;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return false;

	return pPlayer->IsAlive();
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff
//-----------------------------------------------------------------------------
void CHudCrosshairInfo::Paint( void )
{
	if( ( m_flDrawTime + m_flDrawDuration ) > gpGlobals->curtime )
	{

		if(!hud_xhairinfo_newStyle.GetBool())
		{
			// draw xhair info
			if( hud_centerid.GetInt() )
				surface()->DrawSetTextPos( m_flXOffset, m_flYOffset );
			else
				surface()->DrawSetTextPos( text1_xpos, text1_ypos );

			// Bug #0000686: defrag wants team colored hud_crosshair names
			surface()->DrawSetTextFont( m_hTextFont );
			Color cColor;
			SetColorByTeam( m_iTeam, cColor );		
			surface()->DrawSetTextColor( cColor.r(), cColor.g(), cColor.b(), 255 );

			for( wchar_t *wch = m_pText; *wch != 0; wch++ )
				surface()->DrawUnicodeChar( *wch );
		}
	}
	else
	{
		//set the visible children panels to invis
	}
}