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

static ConVar hud_centerid( "hud_centerid", "0", FCVAR_ARCHIVE );
static ConVar hud_ci_newStyle( "hud_ci_newStyle", "1", FCVAR_ARCHIVE, "Crosshair info new style (0 Off, 1 On)");
static ConVar hud_ci_update( "hud_ci_update", "1", FCVAR_ARCHIVE, "Used to update the look (does not happen on value change as there are so many values to set)");
static ConVar hud_ci_updateAlways( "hud_ci_updateAlways", "0", FCVAR_ARCHIVE, "For perfromance reasons, only have this on while editing your settings.");

static ConVar hud_ci_x( "hud_ci_x", "200", FCVAR_ARCHIVE, "Crosshair info X Position on 640 480 Resolution");
static ConVar hud_ci_y( "hud_ci_y", "300", FCVAR_ARCHIVE, "Crosshair info Y Position on 640 480 Resolution");

static ConVar hud_ci_intensity_red( "hud_ci_intensity_red", "20", FCVAR_ARCHIVE, "item bar color red component");
static ConVar hud_ci_intensity_orange( "hud_ci_intensity_orange", "50", FCVAR_ARCHIVE, "Crosshair color blue component");
static ConVar hud_ci_intensity_yellow( "hud_ci_intensity_yellow", "70", FCVAR_ARCHIVE, "Crosshair color alpha component");
static ConVar hud_ci_intensity_green( "hud_ci_intensity_green", "100", FCVAR_ARCHIVE, "Crosshair color green component");

static ConVar hud_ci_showBar( "hud_ci_showBar", "0", FCVAR_ARCHIVE, "Show Bar");
static ConVar hud_ci_showBarBackground( "hud_ci_showBarBackground", "1", FCVAR_ARCHIVE, "Show Bar Background");
static ConVar hud_ci_showBarBorder( "hud_ci_showBarBorder", "1", FCVAR_ARCHIVE, "Show Bar Boarder");
static ConVar hud_ci_showIcon( "hud_ci_showIcon", "1", FCVAR_ARCHIVE, "Show Icon");
static ConVar hud_ci_showLabel( "hud_ci_showLabel", "1", FCVAR_ARCHIVE, "Show label");
static ConVar hud_ci_showAmount( "hud_ci_showAmount", "1", FCVAR_ARCHIVE, "Show amount");
static ConVar hud_ci_showIdent( "hud_ci_showIdent", "1", FCVAR_ARCHIVE, "Show Identifier (Class,Icon)");

static ConVar hud_ci_itemsPerRow( "hud_ci_itemsPerRow", "2", FCVAR_ARCHIVE, "Crosshair  height gap on 640 480 Resolution");
static ConVar hud_ci_itemOffsetX( "hud_ci_itemOffsetX", "120", FCVAR_ARCHIVE, "Crosshair  height gap on 640 480 Resolution");
static ConVar hud_ci_itemOffsetY( "hud_ci_itemOffsetY", "20", FCVAR_ARCHIVE, "Crosshair  height gap on 640 480 Resolution");

static ConVar hud_ci_barWidth( "hud_ci_barWidth", "70", FCVAR_ARCHIVE, "Bar width on 640 480 Resolution");
static ConVar hud_ci_barHeight( "hud_ci_barHeight", "13", FCVAR_ARCHIVE, "Bar height on 640 480 Resolution");
static ConVar hud_ci_barBorderWidth( "hud_ci_barBorderWidth", "1", FCVAR_ARCHIVE, "Bar border width (non-scaleable)");

static ConVar hud_ci_ColorBar_r( "hud_ci_ColorBar_r", "255", FCVAR_ARCHIVE, "Bar color red component");
static ConVar hud_ci_ColorBar_g( "hud_ci_ColorBar_g", "255", FCVAR_ARCHIVE, "Bar color green component");
static ConVar hud_ci_ColorBar_b( "hud_ci_ColorBar_b", "255", FCVAR_ARCHIVE, "Bar color blue component");
static ConVar hud_ci_ColorBar_a( "hud_ci_ColorBar_a", "96", FCVAR_ARCHIVE, "Bar color alpha component");
static ConVar hud_ci_ColorBarBackground_r( "hud_ci_ColorBarBackground_r", "255", FCVAR_ARCHIVE, "Bar Background  background color red component");
static ConVar hud_ci_ColorBarBackground_g( "hud_ci_ColorBarBackground_g", "255", FCVAR_ARCHIVE, "Bar Background color green component");
static ConVar hud_ci_ColorBarBackground_b( "hud_ci_ColorBarBackground_b", "255", FCVAR_ARCHIVE, "Bar Background color blue component");
static ConVar hud_ci_ColorBarBackground_a( "hud_ci_ColorBarBackground_a", "96", FCVAR_ARCHIVE, "Bar Background color alpha component");
static ConVar hud_ci_ColorBarBorder_r( "hud_ci_ColorBarBorder_r", "255", FCVAR_ARCHIVE, "Bar Border color red component");
static ConVar hud_ci_ColorBarBorder_g( "hud_ci_ColorBarBorder_g", "255", FCVAR_ARCHIVE, "Bar Border color green component");
static ConVar hud_ci_ColorBarBorder_b( "hud_ci_ColorBarBorder_b", "255", FCVAR_ARCHIVE, "Bar Border color blue component");
static ConVar hud_ci_ColorBarBorder_a( "hud_ci_ColorBarBorder_a", "255", FCVAR_ARCHIVE, "Bar Border color alpha component");
static ConVar hud_ci_ColorIcon_r( "hud_ci_ColorIcon_r", "0", FCVAR_ARCHIVE, "Icon color red component");
static ConVar hud_ci_ColorIcon_g( "hud_ci_ColorIcon_g", "0", FCVAR_ARCHIVE, "Icon color green component");
static ConVar hud_ci_ColorIcon_b( "hud_ci_ColorIcon_b", "0", FCVAR_ARCHIVE, "Icon color blue component");
static ConVar hud_ci_ColorIcon_a( "hud_ci_ColorIcon_a", "255", FCVAR_ARCHIVE, "Icon color alpha component");
static ConVar hud_ci_ColorLabel_r( "hud_ci_ColorLabel_r", "0", FCVAR_ARCHIVE, "Label color red component");
static ConVar hud_ci_ColorLabel_g( "hud_ci_ColorLabel_g", "0", FCVAR_ARCHIVE, "Label color green component");
static ConVar hud_ci_ColorLabel_b( "hud_ci_ColorLabel_b", "0", FCVAR_ARCHIVE, "Label color blue component");
static ConVar hud_ci_ColorLabel_a( "hud_ci_ColorLabel_a", "255", FCVAR_ARCHIVE, "Label color alpha component");
static ConVar hud_ci_ColorAmount_r( "hud_ci_ColorAmount_r", "255", FCVAR_ARCHIVE, "Amount color red component");
static ConVar hud_ci_ColorAmount_g( "hud_ci_ColorAmount_g", "255", FCVAR_ARCHIVE, "Amount color green component");
static ConVar hud_ci_ColorAmount_b( "hud_ci_ColorAmount_b", "255", FCVAR_ARCHIVE, "Amount color blue component");
static ConVar hud_ci_ColorAmount_a( "hud_ci_ColorAmount_a", "255", FCVAR_ARCHIVE, "Amount color alpha component");
static ConVar hud_ci_ColorIdent_r( "hud_ci_ColorIdent_r", "255", FCVAR_ARCHIVE, "Identifier (Class,Icon) color red component");
static ConVar hud_ci_ColorIdent_g( "hud_ci_ColorIdent_g", "255", FCVAR_ARCHIVE, "Identifier (Class,Icon) color green component");
static ConVar hud_ci_ColorIdent_b( "hud_ci_ColorIdent_b", "255", FCVAR_ARCHIVE, "Identifier (Class,Icon) color blue component");
static ConVar hud_ci_ColorIdent_a( "hud_ci_ColorIdent_a", "255", FCVAR_ARCHIVE, "Identifier (Class,Icon) color alpha component");

static ConVar hud_ci_ShadowIcon( "hud_ci_ShadowIcon", "0", FCVAR_ARCHIVE, "Icon Shadow (0 Off, 1 On)");
static ConVar hud_ci_ShadowLabel( "hud_ci_ShadowLabel", "0", FCVAR_ARCHIVE, "Label Shadow (0 Off, 1 On)");
static ConVar hud_ci_ShadowAmount( "hud_ci_ShadowAmount", "1", FCVAR_ARCHIVE, "Amount Shadow (0 Off, 1 On)");

static ConVar hud_ci_ColorModeBar( "hud_ci_ColorModeBar", "2", FCVAR_ARCHIVE, "Bar color mode");
static ConVar hud_ci_ColorModeBarBackground( "hud_ci_ColorModeBarBackground", "2", FCVAR_ARCHIVE, "Bar Background color mode");
static ConVar hud_ci_ColorModeBarBorder( "hud_ci_ColorModeBarBorder", "3", FCVAR_ARCHIVE, "Bar Border color mode");
static ConVar hud_ci_ColorModeIcon( "hud_ci_ColorModeIcon", "0", FCVAR_ARCHIVE, "Icon color mode");
static ConVar hud_ci_ColorModeLabel( "hud_ci_ColorModeLabel", "0", FCVAR_ARCHIVE, "Label color mode");
static ConVar hud_ci_ColorModeAmount( "hud_ci_ColorModeAmount", "0", FCVAR_ARCHIVE, "Amount color mode");
static ConVar hud_ci_ColorModeIdent( "hud_ci_ColorModeIdent", "0", FCVAR_ARCHIVE, "Identifier (Class,Icon) color mode");

static ConVar hud_ci_offsetXBar( "hud_ci_offsetXBar", "0", FCVAR_ARCHIVE, "Bar offset x");
static ConVar hud_ci_offsetYBar( "hud_ci_offsetYBar", "0", FCVAR_ARCHIVE, "Bar offset y");
static ConVar hud_ci_offsetXIcon( "hud_ci_offsetXIcon", "65", FCVAR_ARCHIVE, "Icon offset x");
static ConVar hud_ci_offsetYIcon( "hud_ci_offsetYIcon", "0", FCVAR_ARCHIVE, "Icon offset y");
static ConVar hud_ci_offsetXLabel( "hud_ci_offsetXLabel", "1", FCVAR_ARCHIVE, "label offset x");
static ConVar hud_ci_offsetYLabel( "hud_ci_offsetYLabel", "0", FCVAR_ARCHIVE, "label offset y");
static ConVar hud_ci_offsetXAmount( "hud_ci_offsetXAmount", "-25", FCVAR_ARCHIVE, "Amount offset x");
static ConVar hud_ci_offsetYAmount( "hud_ci_offsetYAmount", "0", FCVAR_ARCHIVE, "Amount offset y");
static ConVar hud_ci_offsetXIdent( "hud_ci_offsetXIdent", "0", FCVAR_ARCHIVE, "Identifier (Class,Name,Icon) offset x");
static ConVar hud_ci_offsetYIdent( "hud_ci_offsetYIdent", "-30", FCVAR_ARCHIVE, "Identifier (Class,Name,Icon) offset y");

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

public:
	CHudCrosshairInfo( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudCrosshairInfo" )
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport() );
		// Hide when player is dead
		SetHiddenBits( HIDEHUD_PLAYERDEAD );

		vgui::ivgui()->AddTickSignal( GetVPanel() );

		m_flDuration = 0.2f;
		m_flDrawDuration = 2.0f;
		
		m_qbIdent = new CHudQuantityBar();
		m_qbHealth = new CHudQuantityBar();
		m_qbArmor = new CHudQuantityBar();
		m_qbCells = new CHudQuantityBar();
		m_qbShells = new CHudQuantityBar();
		m_qbRockets = new CHudQuantityBar();
		m_qbNails = new CHudQuantityBar();
		m_qbLevel = new CHudQuantityBar();
		
		m_qbIdent->showAmount(false);
		m_qbIdent->showBar(false);
		m_qbIdent->showBarBackground(false);
		m_qbIdent->showBarBorder(false);		
		m_qbLevel->setIntensityControl(1,2,2,3);

		m_qbRockets->showAmountMax(true);
		m_qbCells->showAmountMax(true);
		m_qbShells->showAmountMax(true);
		m_qbNails->showAmountMax(true);
		m_qbLevel->showAmountMax(true);

		m_qbRockets->setAmountMax(50);
		m_qbLevel->setAmountMax(3);

		m_qbHealth->setLabelText("#FF_ITEM_HEALTH");
		m_qbArmor->setLabelText("#FF_ITEM_ARMOR");
		m_qbCells->setLabelText("#FF_ITEM_CELLS");
		m_qbShells->setLabelText("#FF_ITEM_SHELLS");
		m_qbRockets->setLabelText("#FF_ITEM_ROCKETS");
		m_qbNails->setLabelText("#FF_ITEM_NAILS");
		m_qbLevel->setLabelText("FF_ITEM_LEVEL");

		m_qbHealth->setIconChar(":");
		m_qbArmor->setIconChar(";");
		m_qbCells->setIconChar("6");
		m_qbShells->setIconChar("2");
		m_qbRockets->setIconChar("8");
		m_qbNails->setIconChar("7");
		m_qbLevel->showIcon(false);
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

	// For center printing
	float		m_flXOffset;
	float		m_flYOffset;

	// For color
	int			m_iTeam;
	int			m_iClass;
	
	CHudQuantityBar *m_qbIdent;
	CHudQuantityBar *m_qbHealth;
	CHudQuantityBar *m_qbArmor;
	CHudQuantityBar *m_qbRockets;
	CHudQuantityBar *m_qbCells;
	CHudQuantityBar *m_qbShells;
	CHudQuantityBar *m_qbNails;
	CHudQuantityBar *m_qbLevel;

private:
	// Stuff we need to know 
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "ChatFont" );
	CPanelAnimationVar( vgui::HFont, m_hQuantityBarHUD, "TextFont", "QuantityBarHUD" );
	CPanelAnimationVar( vgui::HFont, m_hQuantityBarShadowHUD, "TextFont", "QuantityBarShadowHUD" );
	CPanelAnimationVar( vgui::HFont, m_hQuantityBarIconHUD, "IconFont", "QuantityBarIconHUD" );
	CPanelAnimationVar( vgui::HFont, m_hQuantityBarIconShadowHUD, "IconFont", "QuantityBarIconShadowHUD" );
	CPanelAnimationVar( vgui::HFont, m_hQuantityBarClassNameHUD, "TextFont", "QuantityBarClassNameHUD" );
	CPanelAnimationVar( vgui::HFont, m_hQuantityBarClassGlyphHUD, "IconFont", "QuantityBarClassGlyphHUD" );
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
	
	hud_ci_update.SetValue(1);

	m_qbIdent->setIconFont(m_hQuantityBarClassGlyphHUD);
	m_qbIdent->setLabelFont(m_hQuantityBarClassNameHUD);
	m_qbIdent->setLabelOffsetX(12);
	m_qbIdent->setLabelOffsetY(3);

	// Make the panel as big as the screen
	SetPos( 0, 0 );
	SetWide( scheme()->GetProportionalScaledValue( 640 ) );
	SetTall( scheme()->GetProportionalScaledValue( 480 ) );
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
								
				m_qbIdent->setLabelText(szClass);
				// Default
				int iHealth = -1, iArmor = -1, iCells = -1, iRockets = -1, iNails = -1, iShells = -1, iLevel = -1, iFuseTime = -1;

				int CROSSHAIRTYPE = CROSSHAIRTYPE_NORMAL;
				// Default
				m_iTeam = pHitPlayer->GetTeamNumber();
				m_iClass = pHitPlayer->GetTeamNumber();
				m_qbArmor->showAmountMax(false);

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
						
						m_qbArmor->showAmountMax(true);
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
							m_qbArmor->showAmountMax(true);
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
							m_qbArmor->showAmountMax(true);
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

				if(!hud_ci_newStyle.GetBool())
				{
					// NOW! Remember team is 1 higher than the actual team
					// If health/armor are -1 then we don't show it

					// Convert to unicode & localize stuff

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
						int iWide = UTIL_ComputeStringWidth( m_hTextFont, m_pText );
						int iTall = surface()->GetFontTall( m_hTextFont );

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
					if(Q_stricmp(szClass,"#FF_PLAYER_SCOUT") == 0)
					{
						m_qbIdent->setIconChar("!");
						m_qbIdent->setLabelText(szClass);
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_SNIPER") == 0)
					{
						m_qbIdent->setIconChar("@");
						m_qbIdent->setLabelText(szClass);
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_SOLDIER") == 0)
					{
						m_qbIdent->setIconChar("#");
						m_qbIdent->setLabelText(szClass);
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_DEMOMAN") == 0)
					{
						m_qbIdent->setIconChar("$");
						m_qbIdent->setLabelText(szClass);
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_MEDIC") == 0)
					{
						m_qbIdent->setIconChar("%");
						m_qbIdent->setLabelText(szClass);
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_HWGUY") == 0)
					{
						m_qbIdent->setIconChar("^");
						m_qbIdent->setLabelText(szClass);
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_SPY") == 0)
					{
						m_qbIdent->setIconChar("*");
						m_qbIdent->setLabelText(szClass);
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_PYRO") == 0)
					{
						m_qbIdent->setIconChar("?");
						m_qbIdent->setLabelText(szClass);
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_ENGINEER") == 0)
					{
						m_qbIdent->setIconChar("(");
						m_qbIdent->setLabelText(szClass);
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_CIVILIAN") == 0)
					{
						m_qbIdent->setIconChar(")");
						m_qbIdent->setLabelText(szClass);
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_SENTRYGUN") == 0)
					{
						m_qbIdent->setIconChar("R");
						m_qbIdent->setLabelText(szClass);
					}
					else if(Q_stricmp(szClass, "#FF_PLAYER_DISPENSER") == 0)
					{
						m_qbIdent->setIconChar("Q");
						m_qbIdent->setLabelText(szClass);
					}
					else
					{
						m_qbIdent->setIconChar("_");
						m_qbIdent->setLabelText("#FF_PLAYER_INVALID");
					}

					if(hud_ci_update.GetBool() || hud_ci_updateAlways.GetBool())
					{
						//reset it so that it does not update again
						hud_ci_update.SetValue(0);

						int barWidth = hud_ci_barWidth.GetInt();
						int barHeight = hud_ci_barHeight.GetInt();
						int barBorderWidth = hud_ci_barBorderWidth.GetInt();

						int barColorMode = hud_ci_ColorModeBar.GetInt();
						int barBackgroundColorMode = hud_ci_ColorModeBarBackground.GetInt();
						int barBorderColorMode =  hud_ci_ColorModeBarBorder.GetInt();
						int iconColorMode = hud_ci_ColorModeIcon.GetInt();
						int labelColorMode = hud_ci_ColorModeLabel.GetInt();
						int amountColorMode = hud_ci_ColorModeAmount.GetInt();
						int identColorMode = hud_ci_ColorModeIdent.GetInt();
						
						Color barColor = *new Color(
							hud_ci_ColorBar_r.GetInt(),
							hud_ci_ColorBar_g.GetInt(),
							hud_ci_ColorBar_b.GetInt(),
							hud_ci_ColorBar_a.GetInt());
						Color barBackgroundColor = *new Color(
							hud_ci_ColorBarBackground_r.GetInt(),
							hud_ci_ColorBarBackground_g.GetInt(),
							hud_ci_ColorBarBackground_b.GetInt(),
							hud_ci_ColorBarBackground_a.GetInt());
						Color barBorderColor = *new Color(
							hud_ci_ColorBarBorder_r.GetInt(),
							hud_ci_ColorBarBorder_g.GetInt(),
							hud_ci_ColorBarBorder_b.GetInt(),
							hud_ci_ColorBarBorder_a.GetInt());
						Color iconColor = *new Color(
							hud_ci_ColorIcon_r.GetInt(),
							hud_ci_ColorIcon_g.GetInt(),
							hud_ci_ColorIcon_b.GetInt(),
							hud_ci_ColorIcon_a.GetInt());
						Color labelColor = *new Color(
							hud_ci_ColorLabel_r.GetInt(),
							hud_ci_ColorLabel_g.GetInt(),
							hud_ci_ColorLabel_b.GetInt(),
							hud_ci_ColorLabel_a.GetInt());
						Color amountColor = *new Color(
							hud_ci_ColorAmount_r.GetInt(),
							hud_ci_ColorAmount_g.GetInt(),
							hud_ci_ColorAmount_b.GetInt(),
							hud_ci_ColorAmount_a.GetInt());
						Color identColor = *new Color(
							hud_ci_ColorIdent_r.GetInt(),
							hud_ci_ColorIdent_g.GetInt(),
							hud_ci_ColorIdent_b.GetInt(),
							hud_ci_ColorIdent_a.GetInt());
						
						m_qbIdent->setIconColor(identColor);
						m_qbIdent->setLabelColor(identColor);
						m_qbIdent->setIconColorMode(identColorMode);
						m_qbIdent->setLabelColorMode(identColorMode);
			
						int red = hud_ci_intensity_red.GetInt();
						int orange = hud_ci_intensity_orange.GetInt();
						int yellow = hud_ci_intensity_yellow.GetInt();
						int green = hud_ci_intensity_green.GetInt();

						bool showBar = hud_ci_showBar.GetBool();
						bool showBarBackground = hud_ci_showBarBackground.GetBool();
						bool showBarBorder = hud_ci_showBarBorder.GetBool();
						bool showIcon = hud_ci_showIcon.GetBool();
						bool showLabel = hud_ci_showLabel.GetBool();
						bool showAmount = hud_ci_showAmount.GetBool();
						
						int barOffsetX = hud_ci_offsetXBar.GetInt();
						int barOffsetY = hud_ci_offsetYBar.GetInt();
						int iconOffsetX = hud_ci_offsetXIcon.GetInt();
						int iconOffsetY = hud_ci_offsetYIcon.GetInt();
						int labelOffsetX = hud_ci_offsetXLabel.GetInt();
						int labelOffsetY = hud_ci_offsetYLabel.GetInt();
						int amountOffsetX = hud_ci_offsetXAmount.GetInt();
						int amountOffsetY = hud_ci_offsetYAmount.GetInt();
						
						vgui::HFont amountFont,iconFont,labelFont;

						if(hud_ci_ShadowIcon.GetBool())
							iconFont = m_hQuantityBarIconShadowHUD;
						else
							iconFont = m_hQuantityBarIconHUD;

						if(hud_ci_ShadowLabel.GetBool())
							labelFont = m_hQuantityBarShadowHUD;
						else
							labelFont = m_hQuantityBarHUD;

						if(hud_ci_ShadowAmount.GetBool())
							amountFont = m_hQuantityBarShadowHUD;
						else
							amountFont = m_hQuantityBarHUD;

						m_qbHealth->setBarOffsetX(barOffsetX);
						m_qbArmor->setBarOffsetX(barOffsetX);
						m_qbLevel->setBarOffsetX(barOffsetX);
						m_qbCells->setBarOffsetX(barOffsetX);
						m_qbShells->setBarOffsetX(barOffsetX);
						m_qbRockets->setBarOffsetX(barOffsetX);
						m_qbNails->setBarOffsetX(barOffsetX);

						m_qbHealth->setBarOffsetY(barOffsetY);
						m_qbArmor->setBarOffsetY(barOffsetY);
						m_qbLevel->setBarOffsetY(barOffsetY);
						m_qbCells->setBarOffsetY(barOffsetY);
						m_qbShells->setBarOffsetY(barOffsetY);
						m_qbRockets->setBarOffsetY(barOffsetY);
						m_qbNails->setBarOffsetY(barOffsetY);

						m_qbHealth->setIconOffsetX(iconOffsetX);
						m_qbArmor->setIconOffsetX(iconOffsetX);
						m_qbLevel->setIconOffsetX(iconOffsetX);
						m_qbCells->setIconOffsetX(iconOffsetX);
						m_qbShells->setIconOffsetX(iconOffsetX);
						m_qbRockets->setIconOffsetX(iconOffsetX);
						m_qbNails->setIconOffsetX(iconOffsetX);

						m_qbHealth->setIconOffsetY(iconOffsetY);
						m_qbArmor->setIconOffsetY(iconOffsetY);
						m_qbLevel->setIconOffsetY(iconOffsetY);
						m_qbCells->setIconOffsetY(iconOffsetY);
						m_qbShells->setIconOffsetY(iconOffsetY);
						m_qbRockets->setIconOffsetY(iconOffsetY);
						m_qbNails->setIconOffsetY(iconOffsetY);

						m_qbHealth->setLabelOffsetX(labelOffsetX);
						m_qbArmor->setLabelOffsetX(labelOffsetX);
						m_qbLevel->setLabelOffsetX(labelOffsetX);
						m_qbCells->setLabelOffsetX(labelOffsetX);
						m_qbShells->setLabelOffsetX(labelOffsetX);
						m_qbRockets->setLabelOffsetX(labelOffsetX);
						m_qbNails->setLabelOffsetX(labelOffsetX);

						m_qbHealth->setLabelOffsetY(labelOffsetY);
						m_qbArmor->setLabelOffsetY(labelOffsetY);
						m_qbLevel->setLabelOffsetY(labelOffsetY);
						m_qbCells->setLabelOffsetY(labelOffsetY);
						m_qbShells->setLabelOffsetY(labelOffsetY);
						m_qbRockets->setLabelOffsetY(labelOffsetY);
						m_qbNails->setLabelOffsetY(labelOffsetY);

						m_qbHealth->setAmountOffsetX(amountOffsetX);
						m_qbArmor->setAmountOffsetX(amountOffsetX);
						m_qbLevel->setAmountOffsetX(amountOffsetX);
						m_qbCells->setAmountOffsetX(amountOffsetX);
						m_qbShells->setAmountOffsetX(amountOffsetX);
						m_qbRockets->setAmountOffsetX(amountOffsetX);
						m_qbNails->setAmountOffsetX(amountOffsetX);

						m_qbHealth->setAmountOffsetY(amountOffsetY);
						m_qbArmor->setAmountOffsetY(amountOffsetY);
						m_qbLevel->setAmountOffsetY(amountOffsetY);
						m_qbCells->setAmountOffsetY(amountOffsetY);
						m_qbShells->setAmountOffsetY(amountOffsetY);
						m_qbRockets->setAmountOffsetY(amountOffsetY);
						m_qbNails->setAmountOffsetY(amountOffsetY);

						m_qbHealth->setBarWidth(barWidth);
						m_qbArmor->setBarWidth(barWidth);
						m_qbLevel->setBarWidth(barWidth);
						m_qbCells->setBarWidth(barWidth);
						m_qbShells->setBarWidth(barWidth);
						m_qbRockets->setBarWidth(barWidth);
						m_qbNails->setBarWidth(barWidth);

						m_qbHealth->setBarHeight(barHeight);
						m_qbArmor->setBarHeight(barHeight);
						m_qbLevel->setBarHeight(barHeight);
						m_qbCells->setBarHeight(barHeight);
						m_qbShells->setBarHeight(barHeight);
						m_qbRockets->setBarHeight(barHeight);
						m_qbNails->setBarHeight(barHeight);

						m_qbHealth->setBarBorderWidth(barBorderWidth);
						m_qbArmor->setBarBorderWidth(barBorderWidth);
						m_qbLevel->setBarBorderWidth(barBorderWidth);
						m_qbCells->setBarBorderWidth(barBorderWidth);
						m_qbShells->setBarBorderWidth(barBorderWidth);
						m_qbRockets->setBarBorderWidth(barBorderWidth);
						m_qbNails->setBarBorderWidth(barBorderWidth);

						m_qbHealth->setIntensityControl(red,orange,yellow,green);
						m_qbArmor->setIntensityControl(red,orange,yellow,green);
						m_qbCells->setIntensityControl(red,orange,yellow,green);
						m_qbShells->setIntensityControl(red,orange,yellow,green);
						m_qbNails->setIntensityControl(red,orange,yellow,green);
						m_qbRockets->setIntensityControl((int)red/2,(int)orange/2,(int)yellow/2,(int)green/2);
						m_qbIdent->setIntensityControl(red,orange,yellow,green);

						m_qbArmor->setBarColor(barColor);
						m_qbHealth->setBarColor(barColor);
						m_qbLevel->setBarColor(barColor);
						m_qbCells->setBarColor(barColor);
						m_qbShells->setBarColor(barColor);
						m_qbRockets->setBarColor(barColor);
						m_qbNails->setBarColor(barColor);
						m_qbHealth->setBarBackgroundColor(barBackgroundColor);
						m_qbArmor->setBarBackgroundColor(barBackgroundColor);
						m_qbLevel->setBarBackgroundColor(barBackgroundColor);
						m_qbCells->setBarBackgroundColor(barBackgroundColor);
						m_qbShells->setBarBackgroundColor(barBackgroundColor);
						m_qbRockets->setBarBackgroundColor(barBackgroundColor);
						m_qbNails->setBarBackgroundColor(barBackgroundColor);
						m_qbHealth->setBarBorderColor(barBorderColor);
						m_qbArmor->setBarBorderColor(barBorderColor);
						m_qbLevel->setBarBorderColor(barBorderColor);
						m_qbCells->setBarBorderColor(barBorderColor);
						m_qbShells->setBarBorderColor(barBorderColor);
						m_qbRockets->setBarBorderColor(barBorderColor);
						m_qbNails->setBarBorderColor(barBorderColor);

						m_qbHealth->setIconColor(iconColor);
						m_qbArmor->setIconColor(iconColor);
						m_qbLevel->setIconColor(iconColor);
						m_qbCells->setIconColor(iconColor);
						m_qbShells->setIconColor(iconColor);
						m_qbRockets->setIconColor(iconColor);
						m_qbNails->setIconColor(iconColor);
						m_qbHealth->setLabelColor(labelColor);
						m_qbArmor->setLabelColor(labelColor);
						m_qbLevel->setLabelColor(labelColor);
						m_qbCells->setLabelColor(labelColor);
						m_qbShells->setLabelColor(labelColor);
						m_qbRockets->setLabelColor(labelColor);
						m_qbNails->setLabelColor(labelColor);
						m_qbHealth->setAmountColor(amountColor);
						m_qbArmor->setAmountColor(amountColor);
						m_qbLevel->setAmountColor(amountColor);
						m_qbCells->setAmountColor(amountColor);
						m_qbShells->setAmountColor(amountColor);
						m_qbRockets->setAmountColor(amountColor);
						m_qbNails->setAmountColor(amountColor);

						m_qbArmor->setBarColorMode(barColorMode);
						m_qbHealth->setBarColorMode(barColorMode);
						m_qbLevel->setBarColorMode(barColorMode);
						m_qbCells->setBarColorMode(barColorMode);
						m_qbShells->setBarColorMode(barColorMode);
						m_qbRockets->setBarColorMode(barColorMode);
						m_qbNails->setBarColorMode(barColorMode);
						m_qbHealth->setBarBackgroundColorMode(barBackgroundColorMode);
						m_qbArmor->setBarBackgroundColorMode(barBackgroundColorMode);
						m_qbLevel->setBarBackgroundColorMode(barBackgroundColorMode);
						m_qbCells->setBarBackgroundColorMode(barBackgroundColorMode);
						m_qbShells->setBarBackgroundColorMode(barBackgroundColorMode);
						m_qbRockets->setBarBackgroundColorMode(barBackgroundColorMode);
						m_qbNails->setBarBackgroundColorMode(barBackgroundColorMode);
						m_qbHealth->setBarBorderColorMode(barBorderColorMode);
						m_qbArmor->setBarBorderColorMode(barBorderColorMode);
						m_qbLevel->setBarBorderColorMode(barBorderColorMode);
						m_qbCells->setBarBorderColorMode(barBorderColorMode);
						m_qbShells->setBarBorderColorMode(barBorderColorMode);
						m_qbRockets->setBarBorderColorMode(barBorderColorMode);
						m_qbNails->setBarBorderColorMode(barBorderColorMode);

						m_qbHealth->setIconColorMode(iconColorMode);
						m_qbArmor->setIconColorMode(iconColorMode);
						m_qbLevel->setIconColorMode(iconColorMode);
						m_qbCells->setIconColorMode(iconColorMode);
						m_qbShells->setIconColorMode(iconColorMode);
						m_qbRockets->setIconColorMode(iconColorMode);
						m_qbNails->setIconColorMode(iconColorMode);
						m_qbHealth->setLabelColorMode(labelColorMode);
						m_qbArmor->setLabelColorMode(labelColorMode);
						m_qbLevel->setLabelColorMode(labelColorMode);
						m_qbCells->setLabelColorMode(labelColorMode);
						m_qbShells->setLabelColorMode(labelColorMode);
						m_qbRockets->setLabelColorMode(labelColorMode);
						m_qbNails->setLabelColorMode(labelColorMode);
						m_qbHealth->setAmountColorMode(amountColorMode);
						m_qbArmor->setAmountColorMode(amountColorMode);
						m_qbLevel->setAmountColorMode(amountColorMode);
						m_qbCells->setAmountColorMode(amountColorMode);
						m_qbShells->setAmountColorMode(amountColorMode);
						m_qbRockets->setAmountColorMode(amountColorMode);
						m_qbNails->setAmountColorMode(amountColorMode);

						m_qbHealth->showBar(showBar);
						m_qbArmor->showBar(showBar);
						m_qbLevel->showBar(showBar);
						m_qbCells->showBar(showBar);
						m_qbShells->showBar(showBar);
						m_qbRockets->showBar(showBar);
						m_qbNails->showBar(showBar);
						m_qbHealth->showBarBackground(showBarBackground);
						m_qbArmor->showBarBackground(showBarBackground);
						m_qbLevel->showBarBackground(showBarBackground);
						m_qbCells->showBarBackground(showBarBackground);
						m_qbShells->showBarBackground(showBarBackground);
						m_qbRockets->showBarBackground(showBarBackground);
						m_qbNails->showBarBackground(showBarBackground);
						m_qbHealth->showBarBorder(showBarBorder);
						m_qbArmor->showBarBorder(showBarBorder);
						m_qbLevel->showBarBorder(showBarBorder);
						m_qbCells->showBarBorder(showBarBorder);
						m_qbShells->showBarBorder(showBarBorder);
						m_qbRockets->showBarBorder(showBarBorder);
						m_qbNails->showBarBorder(showBarBorder);

						m_qbHealth->showIcon(showIcon);
						m_qbArmor->showIcon(showIcon);
						m_qbLevel->showIcon(showIcon);
						m_qbCells->showIcon(showIcon);
						m_qbShells->showIcon(showIcon);
						m_qbRockets->showIcon(showIcon);
						m_qbNails->showIcon(showIcon);
						m_qbHealth->showLabel(showLabel);
						m_qbArmor->showLabel(showLabel);
						m_qbLevel->showLabel(showLabel);
						m_qbCells->showLabel(showLabel);
						m_qbShells->showLabel(showLabel);
						m_qbRockets->showLabel(showLabel);
						m_qbNails->showLabel(showLabel);
						m_qbHealth->showAmount(showAmount);
						m_qbArmor->showAmount(showAmount);
						m_qbLevel->showAmount(showAmount);
						m_qbCells->showAmount(showAmount);
						m_qbShells->showAmount(showAmount);
						m_qbRockets->showAmount(showAmount);
						m_qbNails->showAmount(showAmount);

						m_qbHealth->setAmountFont(amountFont);
						m_qbArmor->setAmountFont(amountFont);
						m_qbLevel->setAmountFont(amountFont);
						m_qbCells->setAmountFont(amountFont);
						m_qbShells->setAmountFont(amountFont);
						m_qbRockets->setAmountFont(amountFont);
						m_qbNails->setAmountFont(amountFont);

						m_qbHealth->setLabelFont(labelFont);
						m_qbArmor->setLabelFont(labelFont);
						m_qbLevel->setLabelFont(labelFont);
						m_qbCells->setLabelFont(labelFont);
						m_qbShells->setLabelFont(labelFont);
						m_qbRockets->setLabelFont(labelFont);
						m_qbNails->setLabelFont(labelFont);

						m_qbHealth->setIconFont(iconFont);
						m_qbArmor->setIconFont(iconFont);
						m_qbLevel->setIconFont(iconFont);
						m_qbCells->setIconFont(iconFont);
						m_qbShells->setIconFont(iconFont);
						m_qbRockets->setIconFont(iconFont);
						m_qbNails->setIconFont(iconFont);
					}

					Color teamColor;
					SetColorByTeam( m_iTeam, teamColor );

					int iLeft = hud_ci_x.GetInt(), iTop = hud_ci_y.GetInt();
					int itemOffsetY = hud_ci_itemOffsetY.GetInt();
					int itemOffsetX = hud_ci_itemOffsetX.GetInt();
					int itemsPerRow = hud_ci_itemsPerRow.GetInt();
					int iRow = 0,iOffsetY = 0,iOffsetX = 0;
					
					if(hud_ci_showIdent.GetBool())
					{
						m_qbIdent->setAmount(iHealth);
						m_qbIdent->setTeamColor(teamColor);
						m_qbIdent->setScaleX(flXScale);
						m_qbIdent->setScaleY(flYScale);
						m_qbIdent->setPosition(iLeft + hud_ci_offsetXIdent.GetInt(), iTop + hud_ci_offsetYIdent.GetInt());
					}
					if(iHealth > -1)
					{
						m_qbHealth->setAmount(iHealth);
						m_qbHealth->setTeamColor(teamColor);
						m_qbHealth->setScaleX(flXScale);
						m_qbHealth->setScaleY(flYScale);
						m_qbHealth->setPosition(iLeft + iOffsetX, iTop + iOffsetY);
						m_qbHealth->setVisible(true);
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
						m_qbHealth->setVisible(false);

					if(iArmor > -1)
					{
						m_qbArmor->setAmount(iArmor);
						m_qbArmor->setTeamColor(teamColor);
						m_qbArmor->setScaleX(flXScale);
						m_qbArmor->setScaleY(flYScale);
						m_qbArmor->setPosition(iLeft + iOffsetX, iTop + iOffsetY);
						m_qbArmor->setVisible(true);
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
						m_qbArmor->setVisible(false);

					if(iLevel > -1)
					{
						m_qbLevel->setAmount(iLevel);
						m_qbLevel->setTeamColor(teamColor);
						m_qbLevel->setScaleX(flXScale);
						m_qbLevel->setScaleY(flYScale);
						m_qbLevel->setPosition(iLeft + iOffsetX, iTop + iOffsetY);
						m_qbLevel->setVisible(true);
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
						m_qbLevel->setVisible(false);

					if(iCells > -1)
					{
						m_qbCells->setAmount(iCells);
						m_qbCells->setTeamColor(teamColor);
						m_qbCells->setScaleX(flXScale);
						m_qbCells->setScaleY(flYScale);
						m_qbCells->setPosition(iLeft + iOffsetX, iTop + iOffsetY);
						m_qbCells->setVisible(true);
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
						m_qbCells->setVisible(false);

					if(iShells > -1)
					{
						m_qbShells->setAmount(iShells);
						m_qbShells->setTeamColor(teamColor);
						m_qbShells->setScaleX(flXScale);
						m_qbShells->setScaleY(flYScale);
						m_qbShells->setPosition(iLeft + iOffsetX, iTop + iOffsetY);
						m_qbShells->setVisible(true);
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
						m_qbShells->setVisible(false);

					if(iRockets > -1)
					{
						m_qbRockets->setAmount(iRockets);
						m_qbRockets->setTeamColor(teamColor);
						m_qbRockets->setScaleX(flXScale);
						m_qbRockets->setScaleY(flYScale);
						m_qbRockets->setPosition(iLeft + iOffsetX, iTop + iOffsetY);
						m_qbRockets->setVisible(true);
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
						m_qbRockets->setVisible(false);

					if(iNails > -1)
					{
						m_qbNails->setAmount(iNails);
						m_qbNails->setTeamColor(teamColor);
						m_qbNails->setScaleX(flXScale);
						m_qbNails->setScaleY(flYScale);
						m_qbNails->setPosition(iLeft + iOffsetX, iTop + iOffsetY);
						m_qbNails->setVisible(true);
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
						m_qbNails->setVisible(false);
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

		if(!hud_ci_newStyle.GetBool())
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
		else
		{
			//background then foreground so borders can be large and not cover over the next item.
			m_qbHealth->paintBackground();
			m_qbArmor->paintBackground();
			m_qbLevel->paintBackground();
			m_qbCells->paintBackground();
			m_qbShells->paintBackground();
			m_qbRockets->paintBackground();
			m_qbNails->paintBackground();
			m_qbHealth->paint();
			m_qbArmor->paint();
			m_qbLevel->paint();
			m_qbCells->paint();
			m_qbShells->paint();
			m_qbRockets->paint();
			m_qbNails->paint();
			
			m_qbIdent->paint();
		}
	}
}