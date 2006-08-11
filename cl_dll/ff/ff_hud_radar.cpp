//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_radar.cpo
//	@author Patrick O'Leary (Mulchman)
//	@date 04/06/2005
//	@brief client side Hud Radar class
//
//	REVISIONS
//	---------
//	04/06/2005, Mulchman: 
//		First created
//
//	04/21/2005, Mulchman:
//		Lots of changes, pretty much done
//
//	09/19/2005,	Mulchman:
//		Completely overhauled. Is now 99.9% done.
//		Used shared glyph stuff now (shares with
//		the radio tagging).

#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"

using namespace vgui;

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

#include "ff_hud_boxes.h"
#include "c_ff_player.h"
#include "ff_utils.h"
#include "ff_esp_shared.h"
#include "ff_glyph.h"
#include "utlvector.h"

// [integer] Duration [in seconds] that the radar information
// is drawn on the screen
static ConVar radar_duration( "ffdev_radar_duration", "5" );

class CHudRadar : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CHudRadar, vgui::Panel );

	CUtlVector< CGlyphESP >	m_hRadarList;
	float	m_flStartTime;

	int		m_iTextureWide;
	int		m_iTextureTall;
	int		m_iWidthOffset;
	int		m_iHeightOffset;

	void	CacheTextures( void );

public:

	CHudRadar( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudRadar" ) 
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport( ) );

		// Hide when player is dead
		SetHiddenBits( HIDEHUD_PLAYERDEAD );
	};

	void	Init( void );
	void	VidInit( void );
	void	Paint( void );

	// Callback function for the "RadarUpdate" user message
	void	MsgFunc_RadarUpdate( bf_read &msg );

};

DECLARE_HUDELEMENT( CHudRadar );
DECLARE_HUD_MESSAGE( CHudRadar, RadarUpdate );

void CHudRadar::VidInit( void )
{	
	// So we don't have a big "haze" over the entire screen
	// (ie. the background from the panel showing)
	SetPaintBackgroundEnabled( false );

	// Set up our screen position and stuff before drawing
	int iWide, iTall;
	surface()->GetScreenSize( iWide, iTall );

	// Set up the panel to take up the WHOLE screen
	SetPos( 0, 0 );
	SetWide( iWide );
	SetTall( iTall );

	// Hide the panel for now
	SetVisible( false );

	// Cache textures
	CacheTextures();
}

void CHudRadar::Init( void )
{
	HOOK_HUD_MESSAGE( CHudRadar, RadarUpdate );

	CacheGlyphs();
}

void CHudRadar::CacheTextures( void )
{
	m_iTextureWide = 256;
	m_iTextureTall = 512;

	m_iWidthOffset = 32;
	m_iHeightOffset = 160;
}

void CHudRadar::MsgFunc_RadarUpdate( bf_read &msg )
{
	bool bRecvMessage = false;

	// Initialize
	//m_iNumPlayers = 0;
	
	// send block	
	// - team (int 1-4) [to color the silhouettes elitely]
	// - class (int)
	// - origin (float[3])
	// team = 99 terminates

	int iInfo = msg.ReadWord();
	while( iInfo )
	{
		CGlyphESP	hObject;

		/*
		// Do stuff here - build internal vector
		// for when we "paint" later
		hObject.m_iTeam = iTeam;

		// Read class and do stuff
		hObject.m_iClass = msg.ReadShort( );

		// Read origin and do stuff
		msg.ReadBitVec3Coord( hObject.m_vecOrigin );
		*/

		// Get team
		hObject.m_iTeam = iInfo & 0x0000000F;
		// Get class
		hObject.m_iClass = ( ( iInfo & 0xFFFFFFF0 ) >> 4 );
		// Get ducked state
		hObject.m_bDucked = ( msg.ReadByte() == 1 );
		// Get origin
		msg.ReadBitVec3Coord( hObject.m_vecOrigin );

		DevMsg( "[Radar] Team: %i, Class: %i, Ducked: %s, Origin: %f, %f, %f\n", hObject.m_iTeam, hObject.m_iClass, hObject.m_bDucked ? "yes" : "no", hObject.m_vecOrigin.x, hObject.m_vecOrigin.y, hObject.m_vecOrigin.z );

		// Received at least one valid message
		bRecvMessage = true;

		m_hRadarList.AddToTail( hObject );

		iInfo = msg.ReadWord();
	}

	if( bRecvMessage )
	{
		// Setup our time trackers
		m_flStartTime = gpGlobals->curtime;
	}
}

void CHudRadar::Paint( void )
{
	if( engine->IsInGame() )
	{
		/*
		C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
		if( pPlayer )
		{
			Color fgCol = Color( pPlayer->GetTeamColor().r(), pPlayer->GetTeamColor().g(), pPlayer->GetTeamColor().b(), 215 ) ;
			Color bgCol = Color( pPlayer->GetTeamColor().r(), pPlayer->GetTeamColor().g(), pPlayer->GetTeamColor().b(), 175 ) ;
			
			DrawHudBox( "ammoCarriedBoxBG", 300, 100, bgCol );
			DrawHudBox( "ammoCarriedBoxFG", 300, 100, fgCol );

			DrawHudBox( "healthBoxBG", 5, 100, bgCol );
			DrawHudBox( "healthBoxFG", 5, 100, fgCol );

			DrawHudBox( "locationBoxBG", 5, 170, bgCol );
			DrawHudBox( "locationBoxFG", 5, 170, fgCol );
		}
		*/

		if( m_hRadarList.Count() )
		{
			// Get us
			C_FFPlayer *pPlayer = ToFFPlayer( C_BasePlayer::GetLocalPlayer() );
			if( !pPlayer )
			{
				Warning( "[Scout Radar] No local player!\n" );
				return;
			}

			// Get our origin
			Vector vecOrigin = pPlayer->GetAbsOrigin();

			// Find our fade based on our time shown
			float dt = ( m_flStartTime - gpGlobals->curtime );
			float flAlpha = SimpleSplineRemapVal( dt, 0.0f, radar_duration.GetInt(), 255, 0 );
			flAlpha = clamp( flAlpha, 0.0f, 255.0f );

			// Loop through all our dudes
			for( int i = 0; i < m_hRadarList.Count(); i++ )
			{				
				// Draw a box around the guy if they're on our screen
				int iScreenX, iScreenY;
				if( GetVectorInScreenSpace( m_hRadarList[ i ].m_vecOrigin, iScreenX, iScreenY ) )
				{
					int iTopScreenX, iTopScreenY;
					/*bool bGotTopScreenY =*/ GetVectorInScreenSpace( m_hRadarList[ i ].m_vecOrigin + ( m_hRadarList[ i ].m_bDucked ? Vector( 0, 0, 60 ) : Vector( 0, 0, 80 ) ), iTopScreenX, iTopScreenY );

					Color cColor;
					SetColorByTeam( m_hRadarList[ i ].m_iTeam, cColor );

					// Get distance from us to them
					float flDist = vecOrigin.DistTo( m_hRadarList[ i ].m_vecOrigin );

					int iIndex = m_hRadarList[ i ].m_iClass - 1;

					// Modify based on FOV
					flDist *= ( pPlayer->GetFOVDistanceAdjustFactor() );

					int iWidthAdj = 30;
					int iAdjX = ( ( ( m_iTextureWide - iWidthAdj ) / 2 ) * ( ( ( m_iTextureWide - iWidthAdj ) / 2 ) / flDist ) );
					//int iYTop = ( iScreenY - ( m_iHeightOffset * ( ( m_iTextureTall / 2 ) / flDist ) ) );
					int iYTop = /*( bGotTopScreenY ?*/ iTopScreenY /*: ( ( iScreenY - ( m_iHeightOffset * ( ( m_iTextureTall / 2 ) / flDist ) ) ) ) )*/;
					int iYBot = iScreenY + ( m_iWidthOffset * ( ( m_iTextureTall / 2 ) / flDist ) );

					if( flDist <= 300 )
					{
						surface()->DrawSetTextureFile( g_ClassGlyphs[ iIndex ].m_pTexture->textureId, g_ClassGlyphs[ iIndex ].m_szMaterial, true, false );
						surface()->DrawSetTexture( g_ClassGlyphs[ iIndex ].m_pTexture->textureId );
						surface()->DrawSetColor( cColor.r(), cColor.g(), cColor.b(), flAlpha );
						surface()->DrawTexturedRect( iScreenX - iAdjX, iYTop, iScreenX + iAdjX, iYBot );
					}
					else
					{
						surface()->DrawSetColor( cColor.r(), cColor.g(), cColor.b(), flAlpha );
						surface()->DrawOutlinedRect( iScreenX - iAdjX, iYTop, iScreenX + iAdjX, iYBot );
					}

					// Get the current frame we're supposed to draw
					int iFrame = m_hRadarList[ i ].UpdateFrame();

					// Draw the radio tower thing
					surface()->DrawSetTextureFile( g_RadioTowerGlyphs[ iFrame ].m_pTexture->textureId, g_RadioTowerGlyphs[ iFrame ].m_szMaterial, true, false );
					surface()->DrawSetTexture( g_RadioTowerGlyphs[ iFrame ].m_pTexture->textureId );
					surface()->DrawSetColor( 255, 255, 255, flAlpha );
					surface()->DrawTexturedRect( iScreenX, iYTop, iScreenX + iAdjX, iYTop + iAdjX );
				}
			}
		}

		// Stop drawing since we haven't gotten another update recently
		if( ( m_flStartTime + radar_duration.GetInt() ) <= gpGlobals->curtime )
			m_hRadarList.RemoveAll();
	}
}
