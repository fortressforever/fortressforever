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
//
//	9/21/2005,	Mulchman:
//		Implemented a better drawing method for when
//		the player is actually visible or not dormant

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

	int		m_iTextureWide;
	int		m_iTextureTall;
	int		m_iWidthOffset;
	int		m_iHeightOffset;

	void	CacheTextures( void );

public:

	CHudRadioTag( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudRadioTag" ) 
	{
		SetParent( g_pClientMode->GetViewport() );
		SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_UNASSIGNED );
	};

	void	Init( void );
	void	VidInit( void );
	void	Paint( void );
	
};

DECLARE_HUDELEMENT( CHudRadioTag );

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
}

void CHudRadioTag::Init( void )
{
	CacheGlyphs();
}

void CHudRadioTag::CacheTextures( void )
{
	m_iTextureWide = 256;
	m_iTextureTall = 512;

	m_iWidthOffset = 32;
	m_iHeightOffset = 160;

	CacheGlyphs();
}

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
			return;

		// Get feet origin
		Vector vecOrigin = pPlayer->GetFeetOrigin();

		for( int i = 0; i < MAX_PLAYERS + 1; i++ )
		{
			// Feet origin of tagged player
			Vector vecPlayerOrigin;

			// Skip this guy if he's not visible. Visible means
			// he's tagged and we're supposed to be drawing him!
			if( !pRadioTagData->GetVisible( i ) )
				continue;
			else
			{
				// See if player is dormant or not
				C_BaseEntity *pEntity = ClientEntityList().GetBaseEntity( i );
				if( pEntity )
				{
					// If not dormant, we can get real origin
					if( !pEntity->IsDormant() )
						vecPlayerOrigin = ToFFPlayer( pEntity )->GetFeetOrigin();
					else
					{
						//DevMsg( "[Radio Tag] Drawing from network position (%f)!\n", gpGlobals->curtime );
						vecPlayerOrigin = pRadioTagData->GetOrigin( i );
					}
				}
			}

			// Draw a box around the guy if they're on our screen
			int iScreenX, iScreenY;
			if( GetVectorInScreenSpace( vecPlayerOrigin, iScreenX, iScreenY ) )
			{
				int iTopScreenX, iTopScreenY;
				GetVectorInScreenSpace( vecPlayerOrigin + ( pRadioTagData->GetDucking( i ) ? Vector( 0, 0, 60 ) : Vector( 0, 0, 80 ) ), iTopScreenX, iTopScreenY );

				Color cColor = Color( 255, 255, 255, 255 );
				if( g_PR )
					cColor = g_PR->GetTeamColor( pRadioTagData->GetTeam( i ) );

				// Get distance from us to them
				float flDist = vecOrigin.DistTo( vecPlayerOrigin );

				// Store an index into our glyph array
				int iIndex = pRadioTagData->GetClass( i ) - 1;

				// Modify based on FOV
				flDist *= ( pPlayer->GetFOVDistanceAdjustFactor() );

				int iWidthAdj = 30;
				int iAdjX = ( ( ( m_iTextureWide - iWidthAdj ) / 2 ) * ( ( ( m_iTextureWide - iWidthAdj ) / 2 ) / flDist ) );
				int iYTop = iTopScreenY;
				int iYBot = iScreenY + ( m_iWidthOffset * ( ( m_iTextureTall / 2 ) / flDist ) );

				// Let's see if the player is visible (ha ha!)
				trace_t		tr;
				UTIL_TraceLine ( vecOrigin + Vector( 0, 0, 80 ), vecPlayerOrigin + Vector( 0, 0, 80 ), MASK_VISIBLE, pPlayer, COLLISION_GROUP_NONE, & tr);
				
				if( tr.fraction != 1.0 )  // Trace hit something -- the player is not visible (hopefully!)
				{
					surface()->DrawSetTextureFile( g_ClassGlyphs[ iIndex ].m_pDistTexture->textureId, g_ClassGlyphs[ iIndex ].m_szDistMaterial, true, false );
					surface()->DrawSetTexture( g_ClassGlyphs[ iIndex ].m_pDistTexture->textureId );
					surface()->DrawSetColor( cColor.r(), cColor.g(), cColor.b(), 255 );
					surface()->DrawTexturedRect( iScreenX - iAdjX, iYTop, iScreenX + iAdjX, iYTop + iAdjX + iAdjX );
				}
				else
				{
					surface()->DrawSetTextureFile( g_ClassGlyphs[ iIndex ].m_pTexture->textureId, g_ClassGlyphs[ iIndex ].m_szMaterial, true, false );
					surface()->DrawSetTexture( g_ClassGlyphs[ iIndex ].m_pTexture->textureId );
					surface()->DrawSetColor( cColor.r(), cColor.g(), cColor.b(), 255 );
					surface()->DrawTexturedRect( iScreenX - iAdjX, iYTop, iScreenX + iAdjX, iYBot );
				}
			}
		}
	}
}
