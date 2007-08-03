//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_menu_medengy.h
//	@author Patrick O'Leary (Mulchman)
//	@date 12/04/2006
//	@brief client side Hud Menu for Medic & Engy options
//
//	REVISIONS
//	---------
//	12/04/2006, Mulchman:
//		First created - mainly a copy/paste of CHudContextMenu

#ifndef FF_HUD_MENU_MEDENGY_H
#define FF_HUD_MENU_MEDENGY_H

#include "hudelement.h"
#include "iclientmode.h"

using namespace vgui;

#include <vgui_controls/Panel.h>
#include "ff_hud_menu.h"

class CHudContextMenu_MedEngy : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CHudContextMenu_MedEngy, vgui::Panel );

public:
	CHudContextMenu_MedEngy( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudRadialMedEngyMenu" ) 
	{
		SetParent( g_pClientMode->GetViewport() );
		SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION );		
	}

	~CHudContextMenu_MedEngy( void );	

public:
	void	KeyDown( void );
	void	KeyUp( void );

	void	MouseMove( float *x, float *y );
	void	SetMenu( void );
	void	DoCommand( const char *pszCmd );

	// Stuff we need to know
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );

public:
	virtual void Init( void );
	virtual void VidInit( void );	
	virtual bool ShouldDraw( void );
	virtual void Paint( void );
	virtual bool IsVisible( void )		{ return ShouldDraw(); }

protected:
	bool			m_bMenuVisible;
	CHudTexture		*m_pHudElementTexture;
	//const char		*m_pszPreviousCmd;

	// Which menu to show
	MenuOption *m_pMenu;

	// Progress
	float			m_flSelectStart;
	float			m_flDuration;

	int				m_nOptions;
	float			m_flPositions[5][2];
	int				m_iSelected;

	float			m_flPosX;
	float			m_flPosY;
};

// Global
extern CHudContextMenu_MedEngy *g_pMedEngyHudMenu;

#endif
