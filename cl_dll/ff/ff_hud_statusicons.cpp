//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_statusicons.cpp
//	@author Kevin Hjelden (FryGuy)
//	@date 05/18/2005
//	@brief Client-side Status Icons (conc, gassed, etc)
//
//	REVISIONS
//	---------
//	05/18/2005, FryGuy:
//		Initial copy.

#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "engine/IVDebugOverlay.h"

using namespace vgui;

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>

#include "c_ff_player.h"
#include "ff_utils.h"

#define MAX_STATUSICONS 4

// |-- Mirv: Store all the references to the textures up here now
const char *szIcons[FF_NUMICONS] = {	"vgui/statusicon_conc",		//FF_ICON_CONCUSSION
										"vgui/statusicon_tranq",	//FF_ICON_ONFIRE
										"vgui/statusicon_tranq",	//FF_ICON_TRANQ
										"vgui/statusicon_caltrop",	//FF_ICON_CALTROP
										"vgui/statusicon_legshot",	//FF_ICON_LEGSHOT
										"vgui/statusicon_gas",		//FF_ICON_GAS
									};

struct statusicon_t {
	int icontype;
	float start;
	float duration;
	bool enabled;
};

class CStatusIcons : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CStatusIcons, vgui::Panel );

	statusicon_t m_hIcons[MAX_STATUSICONS];
	CHudTexture	*m_pIconTextures[FF_NUMICONS];

protected:
	void	CacheTextures( void );

public:

	CStatusIcons( const char *pElementName );

	void	Init( void );
	void	VidInit( void );
	void	Paint( void );
	void	OnTick( void );

	// Callback function for the "StatusIconUpdate" user message
	void	MsgFunc_StatusIconUpdate( bf_read &msg );
	
};

DECLARE_HUDELEMENT( CStatusIcons );
DECLARE_HUD_MESSAGE( CStatusIcons, StatusIconUpdate );


CStatusIcons::CStatusIcons( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudStatusIcons" )
{
	// Set our parent window
	SetParent( g_pClientMode->GetViewport( ) );

	// Hide when player is dead
	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	// initialize the status icons
	for (int i=0; i<MAX_STATUSICONS; i++) {
		m_hIcons[i].icontype = 0;
		m_hIcons[i].start = m_hIcons[i].duration = 0.0;
		m_hIcons[i].enabled = false;
	}

	vgui::ivgui()->AddTickSignal( GetVPanel() );
};

void CStatusIcons::CacheTextures( void )
{
	// |-- Mirv: Okay here's a more sane way to do it.
	for (int i = 0; i < FF_NUMICONS; i++)
	{
		m_pIconTextures[i] = new CHudTexture();
		m_pIconTextures[i]->textureId = surface()->CreateNewTextureID();
		surface()->DrawSetTextureFile(m_pIconTextures[i]->textureId, szIcons[i], true, false);
	}

	//m_pIconTextures[1] = new CHudTexture();
	//m_pIconTextures[1]->textureId = surface()->CreateNewTextureID();
	//surface( )->DrawSetTextureFile( m_pIconTextures[1]->textureId, "vgui/statusicon_tranq", true, false );

	//m_pIconTextures[2] = new CHudTexture();
	//m_pIconTextures[2]->textureId = surface()->CreateNewTextureID();
	//surface( )->DrawSetTextureFile( m_pIconTextures[2]->textureId, "vgui/statusicon_tranq", true, false );

	//m_pIconTextures[3] = new CHudTexture();
	//m_pIconTextures[3]->textureId = surface()->CreateNewTextureID();
	//surface( )->DrawSetTextureFile( m_pIconTextures[3]->textureId, "vgui/statusicon_tranq", true, false );

	//m_pIconTextures[4] = new CHudTexture();
	//m_pIconTextures[4]->textureId = surface()->CreateNewTextureID();
	//surface( )->DrawSetTextureFile( m_pIconTextures[4]->textureId, "vgui/statusicon_tranq", true, false );
}

void CStatusIcons::VidInit( void )
{	
	// Cache textures
	CacheTextures( );

	// Bug #0000388: Status HUD icons persist don't go away
	// Reset all icons
	for( int i = 0; i < MAX_STATUSICONS; i++ )
	{
		m_hIcons[ i ].icontype = 0;
		m_hIcons[ i ].start = m_hIcons[ i ].duration = 0.0;
		m_hIcons[ i ].enabled = false;
	}
}

void CStatusIcons::Init( void )
{
	HOOK_HUD_MESSAGE( CStatusIcons, StatusIconUpdate );
}

void CStatusIcons::OnTick( void )
{
	int iWide, iTall;
	surface( )->GetScreenSize( iWide, iTall );

	int displayed = 0;
	for (int i=0; i<MAX_STATUSICONS; i++) {
		if (m_hIcons[i].enabled) {
			if (gpGlobals->curtime - m_hIcons[i].start <= m_hIcons[i].duration) {
				displayed++;
			} else {
				// no time left.. remove it
				m_hIcons[i].enabled = false;
			}
		}
	}

	// resize the window
	if (displayed) {
		SetVisible( true );
		SetPos(iWide/2 - displayed*20, iTall - 50);
		SetWide( displayed*40 );
		SetTall( 40 );
	} else {
		SetVisible( false );
	}

}

void CStatusIcons::MsgFunc_StatusIconUpdate( bf_read &msg )
{
	int icontype = msg.ReadByte();
	float duration = msg.ReadFloat();

	// Mirv: if they select NUM_STATUSICONS, set all
	if (icontype == FF_NUMICONS)
	{
		for (int i = 0; i < MAX_STATUSICONS; i++)
			m_hIcons[i].duration = duration;

		return;
	}

	// find an unused status icon
	// check for an existing icon first
	int id = 0;
	while (m_hIcons[id].icontype != icontype && id != MAX_STATUSICONS)
		id++;

	// if it's not found, then find any usable one
	if (id == MAX_STATUSICONS)
		id = 0;
	while (m_hIcons[id].enabled && m_hIcons[id].icontype != icontype && id != MAX_STATUSICONS)
		id++;

	// if there's no more room, break
	// probably should throw an assertion or something
	if (id == MAX_STATUSICONS)
		return;
	
	// save this icon in the list
	m_hIcons[id].icontype = icontype;
	m_hIcons[id].duration = duration;
	m_hIcons[id].start = gpGlobals->curtime;
	m_hIcons[id].enabled = true;
}

void CStatusIcons::Paint( void )
{
	int displayed = 0;

	// display all the icons
	for (int i=0; i<MAX_STATUSICONS; i++) {
		if (m_hIcons[i].enabled)
		{
			float timeleft = m_hIcons[i].duration - (gpGlobals->curtime - m_hIcons[i].start);
			float alpha;
			if (timeleft >= 0)
			{
				// this is an icon that needs to be displayed
				// draw it..
				if (timeleft <= 2.5)
				{
					alpha = fmod(timeleft*2, 2);
					if (alpha >= 1)
						alpha = 2 - alpha;
				}
				else
				{
					alpha = 1.0f;
				}
				
				if (m_hIcons[i].icontype >= 0 && m_hIcons[i].icontype < FF_NUMICONS)
				{
					//surface( )->DrawSetTexture( m_pIconTextures[ m_hIcons[i].icontype ]->textureId );
					surface( )->DrawSetTexture( m_pIconTextures[ 0 ]->textureId );
					surface()->DrawSetColor( m_hIcons[i].icontype*255, 255, 0, (int)(alpha*255) ); //RGBA
					//surface( )->DrawSetAlpha( alpha );
					//surface()->DrawFilledRect( displayed*40+4, 4, displayed*40+36, 36 ); //x0,y0,x1,y1
					surface( )->DrawTexturedRect( displayed*40+4, 4, displayed*40+36, 36 );
				}
				displayed++;
			}
		}
	}
}
