//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_HintCenter.cpp
//	@author Christopher Boylan (Jiggles)
//	@date 2/8/2007
//	@brief HUD Hint/Message display center
//
//	REVISIONS
//	---------
//	2/08/2007, Jiggles:
//		First created - based significantly off of Mulch's ff_hud_hint.

#ifndef FF_HUD_HINTCENTER_H
#define FF_HUD_HINTCENTER_H

#include "hudelement.h"
#include "iclientmode.h"

#include "ff_panel.h"

using namespace vgui;

#include <vgui_controls/Panel.h>
#include <vgui_controls/AnimationController.h>
#include "ff_hud_menu.h"


#define SELECTION_TIMEOUT_THRESHOLD		5.0f	// Seconds
#define SELECTION_FADEOUT_TIME			1.5f

class CHudHintCenter : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CHudHintCenter, vgui::Panel );

public:
	CHudHintCenter( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudHintCenter" ) 
	{
		SetParent( g_pClientMode->GetViewport() );
		// Hide when player is dead
		SetHiddenBits( HIDEHUD_PLAYERDEAD );
		
		m_pRichText = NULL;
	}

	~CHudHintCenter( void );	

public:
	void	KeyDown( void );
	void	KeyUp( void );

	//void	MouseMove( float *x, float *y );
	//void	SetMenu( void );
	//void	DoCommand( const char *pszCmd );

	// Stuff we need to know
	CPanelAnimationVar( vgui::HFont, m_hDisguiseFont, "DisguiseFont", "ClassGlyphs" );
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "HUD_TextSmall" );

	CPanelAnimationVarAliasType( float, text1_xpos, "text1_xpos", "34", "proportional_float" );
	CPanelAnimationVarAliasType( float, text1_ypos, "text1_ypos", "10", "proportional_float" );
	CPanelAnimationVarAliasType( float, text1_wide, "text1_wide", "200", "proportional_float" );
	CPanelAnimationVarAliasType( float, text1_tall, "text1_tall", "50", "proportional_float" );

	CPanelAnimationVarAliasType( float, image1_xpos, "image1_xpos", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, image1_ypos, "image1_ypos", "4", "proportional_float" );

public:
	virtual void Init( void );
	virtual void VidInit( void );	
	virtual bool ShouldDraw( void );
	virtual void Paint( void );
	virtual bool IsVisible( void )		{ return ShouldDraw(); }
	virtual void OnThink();

	virtual void OpenSelection( void );
	virtual void HideSelection( void );


	// Callback function for the "FF_SendHint" user message
	void	MsgFunc_FF_SendHint( bf_read &msg );

	// Manually add a hud hint
	void	AddHudHint( unsigned short hintID, const char *pszMessage );

private:
	bool			m_bHintCenterVisible;
	CHudTexture		*m_pHudElementTexture;

	//wchar_t		m_pText[ 4096 ];
	float		m_flStartTime;	// When the message was recevied
	float		m_flDuration;	// Duration of the message

	RichText	*m_pRichText;	// Stores the hint text for display

	CUtlVector<unsigned short> m_HintVector;  // Stores whether a hint has been shown yet

	//CPanelAnimationVar( vgui::HFont, m_hNumberFont, "NumberFont", "HudSelectionNumbers" );
	//CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "HudSelectionText" );

	CPanelAnimationVarAliasType( float, m_flSmallBoxSize, "SmallBoxSize", "32", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flLargeBoxWide, "LargeBoxWide", "108", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flLargeBoxTall, "LargeBoxTall", "72", "proportional_float" );

	CPanelAnimationVarAliasType( float, m_flBoxGap, "BoxGap", "12", "proportional_float" );

	CPanelAnimationVarAliasType( float, m_flSelectionNumberXPos, "SelectionNumberXPos", "4", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flSelectionNumberYPos, "SelectionNumberYPos", "4", "proportional_float" );

	CPanelAnimationVarAliasType( float, m_flTextYPos, "TextYPos", "54", "proportional_float" );

	CPanelAnimationVar( float, m_flAlphaOverride, "Alpha", "255" );
	CPanelAnimationVar( float, m_flSelectionAlphaOverride, "SelectionAlpha", "255" );

	CPanelAnimationVar( Color, m_TextColor, "TextColor", "SelectionTextFg" );
	CPanelAnimationVar( Color, m_NumberColor, "NumberColor", "SelectionNumberFg" );
	CPanelAnimationVar( Color, m_EmptyBoxColor, "EmptyBoxColor", "SelectionEmptyBoxBg" );
	CPanelAnimationVar( Color, m_BoxColor, "BoxColor", "SelectionBoxBg" );
	CPanelAnimationVar( Color, m_SelectedBoxColor, "SelectedBoxClor", "SelectionSelectedBoxBg" );

	CPanelAnimationVar( float, m_flWeaponPickupGrowTime, "SelectionGrowTime", "0.1" );

	CPanelAnimationVar( float, m_flTextScan, "TextScan", "1.0" );

	bool m_bFadingOut;

	float	m_flSelectionTime;

	bool m_bHintKeyHeld;

};

// Global
extern CHudHintCenter *g_pHintCenter;


//// Keeps track of whether we've shown the hint or not
//typedef struct HUD_HintInfo
//{
//	unsigned short		HintIndex;			// Each hint has a unique enumerated index (see ff_utils.cpp)
//	bool				bBeenShown = 0;
//};



#endif
