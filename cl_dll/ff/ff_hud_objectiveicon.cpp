//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_objectiveicon.cpp
//	@author Christopher "Jiggles" Boylan
//	@date 2/13/2008
//	@brief client side Hud Objective Icon class

#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"

#include "text_message.h"
#include "view.h"

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
#include "ff_hud_chat.h"

#define OBJECTIVE_ICON_TEXTURE			"glyphs/objective_icon"
#define OBJECTIVE_ICON_TEXTURE_OBSCURED	"glyphs/objective_icon_obscured"

//ConVar cl_objectiveicon_minsize( "cl_objectiveicon_minsize", "350.0f", FCVAR_ARCHIVE, "Minimum size of the objective icon");
#define OBJECTIVE_ICON_MINIMUM_SIZE		350.0f/*cl_objectiveicon_minsize.GetFloat()*/

ConVar cl_objectiveicon("cl_objectiveicon", "0", FCVAR_ARCHIVE, "Displays an icon showing the current objective");

class CHudObjectiveIcon : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CHudObjectiveIcon, vgui::Panel );

	int		m_iTextureWide;
	int		m_iTextureTall;
	int		m_iWidthOffset;
	int		m_iHeightOffset;

	CHudTexture	*m_pIconTexture;
	CHudTexture *m_pObscuredIconTexture;

	void	CacheTextures( void );
	float	GetObjectiveAngle( const Vector &vecDelta );

public:

	CHudObjectiveIcon( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudObjectiveIcon" ) 
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport() );

		// Hide when player is dead
		SetHiddenBits( HIDEHUD_PLAYERDEAD );
		
		m_pIconTexture = NULL;
		m_pObscuredIconTexture = NULL;

		m_iTextureWide = 256;
		m_iTextureTall = 512;

		m_iWidthOffset = 32;
		m_iHeightOffset = 160;

	};

	~CHudObjectiveIcon( void );

	void	Init( void );
	void	VidInit( void );
	void	Paint( void );
	
};

DECLARE_HUDELEMENT( CHudObjectiveIcon );

void CHudObjectiveIcon::VidInit( void )
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

void CHudObjectiveIcon::Init( void )
{
	CacheTextures();
}

void CHudObjectiveIcon::CacheTextures( void )
{
	if ( !m_pIconTexture )
	{
		m_pIconTexture = new CHudTexture( );
		m_pIconTexture->textureId = vgui::surface( )->CreateNewTextureID( );
		PrecacheMaterial( OBJECTIVE_ICON_TEXTURE );
	}

	if ( !m_pObscuredIconTexture )
	{
		m_pObscuredIconTexture = new CHudTexture( );
		m_pObscuredIconTexture->textureId = vgui::surface( )->CreateNewTextureID( );
		PrecacheMaterial( OBJECTIVE_ICON_TEXTURE_OBSCURED );
	}
}

CHudObjectiveIcon::~CHudObjectiveIcon( void )
{
	if( m_pIconTexture )
	{
		delete m_pIconTexture;
		m_pIconTexture = NULL;
	}
	if( m_pObscuredIconTexture )
	{
		delete m_pObscuredIconTexture;
		m_pObscuredIconTexture = NULL;
	}
}


void CHudObjectiveIcon::Paint( void )
{
	if( engine->IsInGame() )
	{
		C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
		if( !pPlayer )
		{
			Assert( 0 );
			return;
		}

		if ( !cl_objectiveicon.GetBool() )
			return;
		
		// Get the current objective entity
		CBaseEntity *pObjectiveEntity = pPlayer->GetCurrObjectiveEntity();
		if ( !pObjectiveEntity )
			return;

		// Get the current objective's location
		Vector vecObjectiveOrigin = pObjectiveEntity->GetAbsOrigin();
		// Get the player's feet origin
		Vector vecOrigin = pPlayer->GetFeetOrigin();

		Color cColor = Color( 255, 255, 255, 255 );
		if( g_PR && ( pPlayer->GetTeamNumber() >= TEAM_BLUE ) && ( pPlayer->GetTeamNumber() <= TEAM_GREEN ) )
			cColor = g_PR->GetTeamColor( pPlayer->GetTeamNumber() );

		// Draw a box around the objective if it's on the player's screen
		int iScreenX, iScreenY;
		if( GetVectorInScreenSpace( vecObjectiveOrigin, iScreenX, iScreenY ) )
		{
			int iTopScreenX, iTopScreenY;
			GetVectorInScreenSpace( vecObjectiveOrigin + Vector( 0, 0, 80 ), iTopScreenX, iTopScreenY );

			// Get distance from the player to the objective
			float flDist = vecOrigin.DistTo( vecObjectiveOrigin );

			// Modify based on FOV
			flDist *= ( pPlayer->GetFOVDistanceAdjustFactor() );

			// We don't want the icon getting too small on the screen
			if ( flDist > OBJECTIVE_ICON_MINIMUM_SIZE )
				flDist = OBJECTIVE_ICON_MINIMUM_SIZE;

			int iWidthAdj = 30;
			int iAdjX = ( ( ( m_iTextureWide - iWidthAdj ) / 2 ) * ( ( ( m_iTextureWide - iWidthAdj ) / 2 ) / flDist ) );
			int iYTop = iTopScreenY;
			//int iYBot = iScreenY + ( m_iWidthOffset * ( ( m_iTextureTall / 2 ) / flDist ) );

			// Let's see if the objective is visible (ha ha!)
			trace_t		tr;
			UTIL_TraceLine ( vecOrigin + Vector( 0, 0, 80 ), vecObjectiveOrigin + Vector( 0, 0, 80 ), MASK_VISIBLE, pPlayer, COLLISION_GROUP_NONE, & tr);
			
			if( tr.fraction != 1.0 )  // Trace hit something -- the objective is not visible (hopefully!)
			{
				surface()->DrawSetTextureFile( m_pObscuredIconTexture->textureId, OBJECTIVE_ICON_TEXTURE_OBSCURED, true, false );
				surface()->DrawSetTexture( m_pObscuredIconTexture->textureId );
				surface()->DrawSetColor( cColor );
				surface()->DrawTexturedRect( iScreenX - iAdjX, iYTop, iScreenX + iAdjX, iYTop + iAdjX + iAdjX );
			}
			else
			{
				surface()->DrawSetTextureFile( m_pIconTexture->textureId, OBJECTIVE_ICON_TEXTURE, true, false );
				surface()->DrawSetTexture( m_pIconTexture->textureId );
				surface()->DrawSetColor( cColor );
				//surface()->DrawTexturedRect( iScreenX - iAdjX, iYTop, iScreenX + iAdjX, iYBot );
				surface()->DrawTexturedRect( iScreenX - iAdjX, iYTop, iScreenX + iAdjX, iYTop + iAdjX + iAdjX );
			}
		}
		
		// Calculate the angle the objective is from the player
		Vector vecDelta = ( vecObjectiveOrigin - MainViewOrigin() );
		VectorNormalize( vecDelta );	
		float angle = GetObjectiveAngle( vecDelta );
		//char szBuf[64];
		//Q_snprintf( szBuf, 64, "%f", angle );
		//ClientPrintMsg( pPlayer, HUD_PRINTCENTER, szBuf );

		// If the objective isn't on the screen, draw an arrow pointing to it
		if ( angle >= 45 && angle <= 315 )  
		{
			// Let's draw a triangle pointing toward the objective that is off the screen
			float yawRadians1 = - (angle) * M_PI / 180.0f;
			float yawRadians2 = - (angle - 5.0f) * M_PI / 180.0f;
			float yawRadians3 = - (angle + 5.0f) * M_PI / 180.0f;

			// Rotate it around the circle
			int iXCenter = ScreenWidth() / 2;
			int iYCenter = ScreenHeight() / 2;

			int xposLeft = (int) ( iXCenter + ( 360.0f * sin(yawRadians2) ) );
			int yposLeft = (int) ( iYCenter - ( 360.0f * cos(yawRadians2) ) );
			
			int xposRight = (int) ( iXCenter + ( 360.0f * sin(yawRadians3) ) );
			int yposRight = (int) ( iYCenter - ( 360.0f * cos(yawRadians3) ) );
			
			int xposMiddle = (int) ( iXCenter + ( (360.0f * 1.05f) * sin(yawRadians1) ) );
			int yposMiddle = (int) ( iYCenter - ( (360.0f * 1.05f) * cos(yawRadians1) ) );
			
			// Let's actually draw the triangle now...
			int xcoords[] = { xposLeft, xposRight, xposMiddle };
			int ycoords[] = { yposLeft, yposRight, yposMiddle };
			surface()->DrawSetColor( cColor );
			surface()->DrawPolyLine( xcoords, ycoords, 3 );

		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Convert an objective position in world units to the screen's units
//-----------------------------------------------------------------------------
float CHudObjectiveIcon::GetObjectiveAngle( const Vector &vecDelta )
{
	Vector playerPosition = MainViewOrigin();
	QAngle playerAngles = MainViewAngles();

	Vector forward, right, up(0, 0, 1);
	AngleVectors(playerAngles, &forward, NULL, NULL);
	forward.z = 0;
	VectorNormalize(forward);
	CrossProduct(up, forward, right);
	float front = DotProduct(vecDelta, forward);
	float side = DotProduct(vecDelta, right);
	float xpos = 360.0f * -side;
	float ypos = 360.0f * -front;

	// Get the rotation(yaw)
	float flRotation = atan2(xpos, ypos) + M_PI;
	flRotation *= 180 / M_PI;
	
	return flRotation;

}