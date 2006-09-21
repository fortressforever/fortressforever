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
#include <vgui/IVGui.h>

#include "cliententitylist.h"

#include "c_ff_player.h"
#include "ff_utils.h"
#include "ff_esp_shared.h"
#include "ff_glyph.h"
#include "c_playerresource.h"
#include "ff_radiotagdata.h"

class CHudRadioTag : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CHudRadioTag, vgui::Panel );

	CGlyphESP	m_hList[ MAX_PLAYERS ];
	int			m_nItems;

	int		m_iTextureWide;
	int		m_iTextureTall;
	int		m_iWidthOffset;
	int		m_iHeightOffset;

	float	m_flStartTime;
	float	m_flLastDraw;

	void	CacheTextures( void );

public:

	CHudRadioTag( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudRadioTag" ) 
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport() );

		// Hide when player is dead
		SetHiddenBits( HIDEHUD_PLAYERDEAD );

		//vgui::ivgui()->AddTickSignal( GetVPanel() );
	};

	void	Init( void );
	void	VidInit( void );
	void	Paint( void );
	//void	OnTick( void );

	// Callback function for the "RadioTagUpdate" user message
	//void	MsgFunc_RadioTagUpdate( bf_read &msg );
	
};

DECLARE_HUDELEMENT( CHudRadioTag );
//DECLARE_HUD_MESSAGE( CHudRadioTag, RadioTagUpdate );

void CHudRadioTag::VidInit( void )
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

	// Cache the glyphs!
	CacheTextures();

	// Reset!
	for( int i = 0; i < MAX_PLAYERS; i++  )
		m_hList[ i ].m_bActive = false;

	// Reset!
	m_nItems = 0;

	m_flStartTime = 0.0f;
	m_flLastDraw = 0.0f;
}

/*
void CHudRadioTag::OnTick( void )
{
	if( !engine->IsInGame() )
		return;

	float flTimeScale = gpGlobals->curtime - m_flLastDraw;

	for( int i = 0; i < MAX_PLAYERS; i++ )
	{
		if( m_hList[ i ].m_bActive )
		{
			// Do we need to interp?
			bool bHasVelocity = m_hList[ i ].m_vecVel != vec3_origin;

			Vector vecInterpPos = m_hList[ i ].m_vecOrigin;				
			if( bHasVelocity )
			{
				vecInterpPos = m_hList[ i ].m_vecOrigin + ( m_hList[ i ].m_vecVel * flTimeScale );
				m_hList[ i ].m_vecOrigin = vecInterpPos;
			}			
		}
	}

	m_flLastDraw = gpGlobals->curtime;
}
*/

void CHudRadioTag::Init( void )
{
	//HOOK_HUD_MESSAGE( CHudRadioTag, RadioTagUpdate );

	CacheGlyphs();
}

void CHudRadioTag::CacheTextures( void )
{
	m_iTextureWide = 256;
	m_iTextureTall = 512;

	m_iWidthOffset = 32;
	m_iHeightOffset = 160;
}

/*
void CHudRadioTag::MsgFunc_RadioTagUpdate( bf_read &msg )
{
	// Set all to non-updated
	for( int i = 0; i < MAX_PLAYERS; i++ )
		m_hList[ i ].m_bUpdated = false;

	bool bRecvMessage = false;
	int iCount = msg.ReadShort();

	// Store off how many active guys
	m_nItems = iCount;

	for( int i = 0; i < iCount; i++ )
	{
		// Get player index (entindex() - 1)
		int iIndex = msg.ReadShort();
		
		m_hList[ iIndex ].m_iEntIndex = iIndex; // redundant
		m_hList[ iIndex ].m_bUpdated = true;

		int iInfo = msg.ReadWord();

		m_hList[ iIndex ].m_iTeam = ( iInfo & 0x0000000F );
		m_hList[ iIndex ].m_iClass = ( ( iInfo & 0xFFFFFFF0 ) >> 4 );
		m_hList[ iIndex ].m_bDucked = ( msg.ReadByte() == 1 );
		
		if( m_hList[ iIndex ].m_bActive )
		{
			// Don't overwrite if this existed last frame
			Vector vecOrigin, vecVel;

			msg.ReadBitVec3Coord( vecOrigin );
			msg.ReadBitVec3Coord( vecVel );

			// If the velocity has changed, update position and velocity
			if( m_hList[ iIndex ].m_vecVel.DistTo( vecVel ) > 2.0f )
			{
				m_hList[ iIndex ].m_vecOrigin = vecOrigin;
				m_hList[ iIndex ].m_vecVel = vecVel;
			}
		}
		else
		{
			msg.ReadBitVec3Coord( m_hList[ iIndex ].m_vecOrigin );
			msg.ReadBitVec3Coord( m_hList[ iIndex ].m_vecVel );
		}

		//if( m_hList[ iIndex ].m_vecVel.Length() < 1.0f )
		//	m_hList[ iIndex ].m_vecVel = vec3_origin;

		m_hList[ iIndex ].m_bActive = true;

		// Received at least one valid message
		bRecvMessage = true;
	}

	if( bRecvMessage )
	{
		// Setup our time trackers
		m_flStartTime = gpGlobals->curtime;		
	}

	// Clear out old entries
	for( int i = 0; i < MAX_PLAYERS; i++ )
	{
		// Make sure only guys we updated are active
		if( !m_hList[ i ].m_bUpdated )
			m_hList[ i ].m_bActive = false;
	}
}
*/

void CHudRadioTag::Paint( void )
{
	if( engine->IsInGame() )
	{
		C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
		if( !pPlayer )
		{
			Assert( 0 );
			return;
		}

		// Get radio tag data
		C_FFRadioTagData *pRadioTagData = pPlayer->GetRadioTagData();
		if( !pRadioTagData )
		{
			//Assert( 0 );
			return;
		}

		// Get feet origin
		Vector vecOrigin = pPlayer->GetFeetOrigin();

		for( int i = 0; i < MAX_PLAYERS + 1; i++ )
		{
			// If the player isn't visible... visible
			// meaning he's tagged and we're supposed
			// to draw him
			if( !pRadioTagData->GetVisible( i ) )
				continue;
			else
			{
				// C_FFPlayer::DrawModel handles drawing when the player
				// isn't dormant. When the player is dormant, we draw here!
				C_BaseEntity *pEntity = ClientEntityList().GetBaseEntity( i );
				if( pEntity )
				{
					if( !pEntity->IsDormant() )
						continue;
				}
			}

			// Draw a box around the guy if they're on our screen
			int iScreenX, iScreenY;
			if( GetVectorInScreenSpace( pRadioTagData->GetOrigin( i ), iScreenX, iScreenY ) )
			{
				int iTopScreenX, iTopScreenY;
				/*bool bGotTopScreenY =*/ GetVectorInScreenSpace( pRadioTagData->GetOrigin( i ) + ( pRadioTagData->GetDucking( i ) ? Vector( 0, 0, 60 ) : Vector( 0, 0, 80 ) ), iTopScreenX, iTopScreenY );

				Color cColor = Color( 255, 255, 255, 255 );
				if( g_PR )
					cColor = g_PR->GetTeamColor( pRadioTagData->GetTeam( i ) );

				// Get distance from us to them
				float flDist = vecOrigin.DistTo( pRadioTagData->GetOrigin( i ) );

				// Store an index into our glyph array
				int iIndex = pRadioTagData->GetClass( i ) - 1;

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
					surface()->DrawSetColor( cColor.r(), cColor.g(), cColor.b(), 255 );
					surface()->DrawTexturedRect( iScreenX - iAdjX, iYTop, iScreenX + iAdjX, iYBot );
				}
				else
				{
					surface()->DrawSetColor( cColor.r(), cColor.g(), cColor.b(), 255 );
					surface()->DrawOutlinedRect( iScreenX - iAdjX, iYTop, iScreenX + iAdjX, iYBot );
				}

				// Get the current frame we're supposed to draw
				//int iFrame = m_hList[ i ].UpdateFrame();

				// Draw the radio tower thing
				//surface()->DrawSetTextureFile( g_RadioTowerGlyphs[ iFrame ].m_pTexture->textureId, g_RadioTowerGlyphs[ iFrame ].m_szMaterial, true, false );
				//surface()->DrawSetTexture( g_RadioTowerGlyphs[ iFrame ].m_pTexture->textureId );
				//surface()->DrawSetColor( 255, 255, 255, flAlpha );
				//surface()->DrawTexturedRect( iScreenX, iYTop, iScreenX + iAdjX, iYTop + iAdjX );
			}
		}
	}

	return;

	if( engine->IsInGame() )
	{
		if( m_nItems )
		{
			// Get us
			C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
			if( !pPlayer )
			{
				Warning( "[Radio Tag] No local player!\n" );
				return;
			}

			// Get our origin
			Vector vecOrigin = pPlayer->GetFeetOrigin();

			// Find our fade based on our time shown
			float dt = ( m_flStartTime - gpGlobals->curtime );
			float flAlpha = SimpleSplineRemapVal( dt, 0.0f, FF_RADIOTAG_TIMETOFORGET, 255, 0 );
			flAlpha = clamp( flAlpha, 0.0f, 255.0f );

			float flTimeScale = gpGlobals->curtime - m_flLastDraw;

			// Loop through all our dudes
			for( int i = 0; i < MAX_PLAYERS; i++ )
			{
				if( !m_hList[ i ].m_bActive )
					continue;

				Vector vecInterpPos = m_hList[ i ].m_vecOrigin;

				// Draw a box around the guy if they're on our screen
				int iScreenX, iScreenY;
				if( GetVectorInScreenSpace( vecInterpPos, iScreenX, iScreenY ) )
				{
					int iTopScreenX, iTopScreenY;
					/*bool bGotTopScreenY =*/ GetVectorInScreenSpace( vecInterpPos + ( m_hList[ i ].m_bDucked ? Vector( 0, 0, 60 ) : Vector( 0, 0, 80 ) ), iTopScreenX, iTopScreenY );

					Color cColor = Color( 255, 255, 255, 255 );
					if( g_PR )
						cColor = g_PR->GetTeamColor( m_hList[ i ].m_iTeam );

					// Get distance from us to them
					float flDist = vecOrigin.DistTo( vecInterpPos );

					int iIndex = m_hList[ i ].m_iClass - 1;

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
					int iFrame = m_hList[ i ].UpdateFrame();

					// Draw the radio tower thing
					surface()->DrawSetTextureFile( g_RadioTowerGlyphs[ iFrame ].m_pTexture->textureId, g_RadioTowerGlyphs[ iFrame ].m_szMaterial, true, false );
					surface()->DrawSetTexture( g_RadioTowerGlyphs[ iFrame ].m_pTexture->textureId );
					surface()->DrawSetColor( 255, 255, 255, flAlpha );
					surface()->DrawTexturedRect( iScreenX, iYTop, iScreenX + iAdjX, iYTop + iAdjX );
				}

				// Do we need to interp?
				bool bHasVelocity = m_hList[ i ].m_vecVel != vec3_origin;

				if( bHasVelocity )
					vecInterpPos = m_hList[ i ].m_vecOrigin + ( m_hList[ i ].m_vecVel * flTimeScale );

				// Update because we don't know when we'll get a server update next
				if( bHasVelocity )
					m_hList[ i ].m_vecOrigin = vecInterpPos;
			}
		}

		// Stop drawing since we haven't gotten another update recently
		if( ( m_flStartTime + FF_RADIOTAG_TIMETOFORGET ) <= gpGlobals->curtime )
		{
			for( int i = 0; i < MAX_PLAYERS; i++ )
				m_hList[ i ].m_bActive = false;

			m_nItems = 0;
		}
	}

	m_flLastDraw = gpGlobals->curtime;
}
