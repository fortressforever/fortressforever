//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_radiotag.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 07/25/2005
//	@brief client side Hud Radio Tag class
//
//	REVISIONS
//	---------
//	07/25/2005, Mulchman: 
//		First created
//
//	08/01/2005, Mulchman:
//		Worked on this for the past couple of days. It
//		now gets updates from the server and draws some
//		text on the screen at the player's position.
//
//	09/19/2005,	Mulchman:
//		Got glyphs and radio tower animation working
//		fully. Just need to tweak the speed of the
//		"animation" to make is consistant no matter
//		what your FPS is at. Also set it up so the
//		glyph code is shared so scout radar and radio
//		tag can use the same stuff.

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

#include "cliententitylist.h"

#include "c_ff_player.h"
#include "ff_utils.h"
#include "ff_esp_shared.h"
#include "ff_glyph.h"
#include "utlvector.h"

class CHudRadioTag : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CHudRadioTag, vgui::Panel );

	CUtlVector< CGlyphESP >	m_hRadioTaggedList;
	float	m_flStartTime;

	int		m_iTextureWide;
	int		m_iTextureTall;
	int		m_iWidthOffset;
	int		m_iHeightOffset;

	void	CacheTextures( void );

public:

	CHudRadioTag( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudRadioTag" ) 
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport( ) );

		// Hide when player is dead
		SetHiddenBits( HIDEHUD_PLAYERDEAD );		
	};

	void	Init( void );
	void	VidInit( void );
	void	Paint( void );

	// Callback function for the "RadioTagUpdate" user message
	void	MsgFunc_RadioTagUpdate( bf_read &msg );
	
};

DECLARE_HUDELEMENT( CHudRadioTag );
DECLARE_HUD_MESSAGE( CHudRadioTag, RadioTagUpdate );

void CHudRadioTag::VidInit( void )
{	
	// So we don't have a big "haze" over the entire screen
	// (ie. the background from the panel showing)
	SetPaintBackgroundEnabled( false );

	// Set up our screen position and stuff before drawing
	int iWide, iTall;
	surface( )->GetScreenSize( iWide, iTall );

	// Set up the panel to take up the WHOLE screen
	SetPos( 0, 0 );
	SetWide( iWide );
	SetTall( iTall );

	// Hide the panel for now
	SetVisible( false );

	// Cache the glyphs!
	CacheTextures( );
}

void CHudRadioTag::Init( void )
{
	HOOK_HUD_MESSAGE( CHudRadioTag, RadioTagUpdate );

	CacheGlyphs();
}

void CHudRadioTag::CacheTextures( void )
{
	m_iTextureWide = 256;
	m_iTextureTall = 512;

	m_iWidthOffset = 32;
	m_iHeightOffset = 160;
}

void CHudRadioTag::MsgFunc_RadioTagUpdate( bf_read &msg )
{
	// Initialize
	m_hRadioTaggedList.RemoveAll();
	
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

		// Received at least one valid message
		bRecvMessage = true;

		m_hRadioTaggedList.AddToTail( hObject );

		iInfo = msg.ReadWord();
	}

	if( bRecvMessage )
	{
		// Setup our time trackers
		m_flStartTime = gpGlobals->curtime;
	}
}

void CHudRadioTag::Paint( void )
{
	if( engine->IsInGame() )
	{
		if( m_hRadioTaggedList.Count() )
		{
			// Get us
			C_FFPlayer *pPlayer = ToFFPlayer( C_BasePlayer::GetLocalPlayer() );
			if( !pPlayer )
			{
				Warning( "[Radio Tag] No local player!\n" );
				return;
			}

			// Get our origin
			Vector vecOrigin = pPlayer->GetAbsOrigin();

			// Find our fade based on our time shown
			float dt = ( m_flStartTime - gpGlobals->curtime );
			float flAlpha = SimpleSplineRemapVal( dt, 0.0f, FF_RADIOTAG_TIMETOFORGET, 255, 0 );
			flAlpha = clamp( flAlpha, 0.0f, 255.0f );

			// Loop through all our dudes
			for( int i = 0; i < m_hRadioTaggedList.Count( ); i++ )
			{				
				// Draw a box around the guy if they're on our screen
				int iScreenX, iScreenY;
				if( GetVectorInScreenSpace( m_hRadioTaggedList[ i ].m_vecOrigin, iScreenX, iScreenY ) )
				{
					int iTopScreenX, iTopScreenY;
					/*bool bGotTopScreenY =*/ GetVectorInScreenSpace( m_hRadioTaggedList[ i ].m_vecOrigin + ( m_hRadioTaggedList[ i ].m_bDucked ? Vector( 0, 0, 60 ) : Vector( 0, 0, 80 ) ), iTopScreenX, iTopScreenY );

					Color cColor;
					SetColorByTeam( m_hRadioTaggedList[ i ].m_iTeam, cColor );

					// Get distance from us to them
					float flDist = vecOrigin.DistTo( m_hRadioTaggedList[ i ].m_vecOrigin );

					int iIndex = m_hRadioTaggedList[ i ].m_iClass - 1;

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
					int iFrame = m_hRadioTaggedList[ i ].UpdateFrame();

					// Draw the radio tower thing
					surface()->DrawSetTextureFile( g_RadioTowerGlyphs[ iFrame ].m_pTexture->textureId, g_RadioTowerGlyphs[ iFrame ].m_szMaterial, true, false );
					surface()->DrawSetTexture( g_RadioTowerGlyphs[ iFrame ].m_pTexture->textureId );
					surface()->DrawSetColor( 255, 255, 255, flAlpha );
					surface()->DrawTexturedRect( iScreenX, iYTop, iScreenX + iAdjX, iYTop + iAdjX );
				}
			}
		}

		// Stop drawing since we haven't gotten another update recently
		if( ( m_flStartTime + FF_RADIOTAG_TIMETOFORGET ) <= gpGlobals->curtime )
			m_hRadioTaggedList.RemoveAll();
	}
}
